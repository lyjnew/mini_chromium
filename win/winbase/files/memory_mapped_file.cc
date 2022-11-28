// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "winbase\files\memory_mapped_file.h"

#include <utility>

#include "winbase\files\file_path.h"
#include "winbase\logging.h"
#include "winbase\numerics\safe_math.h"
#include "winbase\system\sys_info.h"

namespace winbase {

const MemoryMappedFile::Region MemoryMappedFile::Region::kWholeFile = {0, 0};

bool MemoryMappedFile::Region::operator==(
    const MemoryMappedFile::Region& other) const {
  return other.offset == offset && other.size == size;
}

bool MemoryMappedFile::Region::operator!=(
    const MemoryMappedFile::Region& other) const {
  return other.offset != offset || other.size != size;
}

MemoryMappedFile::~MemoryMappedFile() {
  CloseHandles();
}

bool MemoryMappedFile::Initialize(const FilePath& file_name, Access access) {
  if (IsValid())
    return false;

  uint32_t flags = 0;
  switch (access) {
    case READ_ONLY:
      flags = File::FLAG_OPEN | File::FLAG_READ;
      break;
    case READ_WRITE:
      flags = File::FLAG_OPEN | File::FLAG_READ | File::FLAG_WRITE;
      break;
    case READ_WRITE_EXTEND:
      // Can't open with "extend" because no maximum size is known.
      WINBASE_NOTREACHED();
  }
  file_.Initialize(file_name, flags);

  if (!file_.IsValid()) {
    WINBASE_DLOG(ERROR) << "Couldn't open " << file_name.AsUTF8Unsafe();
    return false;
  }

  if (!MapFileRegionToMemory(Region::kWholeFile, access)) {
    CloseHandles();
    return false;
  }

  return true;
}

bool MemoryMappedFile::Initialize(File file, Access access) {
  WINBASE_DCHECK_NE(READ_WRITE_EXTEND, access);
  return Initialize(std::move(file), Region::kWholeFile, access);
}

bool MemoryMappedFile::Initialize(File file,
                                  const Region& region,
                                  Access access) {
  switch (access) {
    case READ_WRITE_EXTEND:
      WINBASE_DCHECK(Region::kWholeFile != region);
      {
        CheckedNumeric<int64_t> region_end(region.offset);
        region_end += region.size;
        if (!region_end.IsValid()) {
          WINBASE_DLOG(ERROR) << "Region bounds exceed maximum for base::File.";
          return false;
        }
      }
      FALLTHROUGH;
    case READ_ONLY:
    case READ_WRITE:
      // Ensure that the region values are valid.
      if (region.offset < 0) {
        WINBASE_DLOG(ERROR) << "Region bounds are not valid.";
        return false;
      }
      break;
  }

  if (IsValid())
    return false;

  if (region != Region::kWholeFile)
    WINBASE_DCHECK_GE(region.offset, 0);

  file_ = std::move(file);

  if (!MapFileRegionToMemory(region, access)) {
    CloseHandles();
    return false;
  }

  return true;
}

bool MemoryMappedFile::IsValid() const {
  return data_ != nullptr;
}

// static
void MemoryMappedFile::CalculateVMAlignedBoundaries(int64_t start,
                                                    size_t size,
                                                    int64_t* aligned_start,
                                                    size_t* aligned_size,
                                                    int32_t* offset) {
  // Sadly, on Windows, the mmap alignment is not just equal to the page size.
  auto mask = SysInfo::VMAllocationGranularity() - 1;
  WINBASE_DCHECK(IsValueInRangeForNumericType<int32_t>(mask));
  *offset = start & mask;
  *aligned_start = start & ~mask;
  *aligned_size = (size + *offset + mask) & ~mask;
}

}  // namespace winbase