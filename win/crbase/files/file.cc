// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/files/file.h"
#include "crbase/files/file_path.h"
#include "crbase/files/file_tracing.h"
///#include "crbase/metrics/histogram.h"
///#include "crbase/timer/elapsed_timer.h"
#include "crbuild/build_config.h"

namespace crbase {

File::Info::Info()
    : size(0),
      is_directory(false),
      is_symbolic_link(false) {
}

File::Info::~Info() {
}

File::File()
    : error_details_(FILE_ERROR_FAILED),
      created_(false),
      async_(false) {
}

File::File(const FilePath& path, uint32_t flags)
    : error_details_(FILE_OK), created_(false), async_(false) {
  Initialize(path, flags);
}

File::File(PlatformFile platform_file)
    : file_(platform_file),
      error_details_(FILE_OK),
      created_(false),
      async_(false) {
}

File::File(Error error_details)
    : error_details_(error_details),
      created_(false),
      async_(false) {
}

File::File(File&& other)
    : file_(other.TakePlatformFile()),
      tracing_path_(other.tracing_path_),
      error_details_(other.error_details()),
      created_(other.created()),
      async_(other.async_) {}

File::~File() {
  // Go through the AssertIOAllowed logic.
  Close();
}

// static
File File::CreateForAsyncHandle(PlatformFile platform_file) {
  File file(platform_file);
  // It would be nice if we could validate that |platform_file| was opened with
  // FILE_FLAG_OVERLAPPED on Windows but this doesn't appear to be possible.
  file.async_ = true;
  return file;
}

File& File::operator=(File&& other) {
  CR_DCHECK_NE(this, &other);
  Close();
  SetPlatformFile(other.TakePlatformFile());
  tracing_path_ = other.tracing_path_;
  error_details_ = other.error_details();
  created_ = other.created();
  async_ = other.async_;
  return *this;
}

void File::Initialize(const FilePath& path, uint32_t flags) {
  if (path.ReferencesParent()) {
    error_details_ = FILE_ERROR_ACCESS_DENIED;
    return;
  }
  if (FileTracing::IsCategoryEnabled())
    tracing_path_ = path;
  CR_SCOPED_FILE_TRACE("Initialize");
  DoInitialize(path, flags);
}

std::string File::ErrorToString(Error error) {
  switch (error) {
    case FILE_OK:
      return "FILE_OK";
    case FILE_ERROR_FAILED:
      return "FILE_ERROR_FAILED";
    case FILE_ERROR_IN_USE:
      return "FILE_ERROR_IN_USE";
    case FILE_ERROR_EXISTS:
      return "FILE_ERROR_EXISTS";
    case FILE_ERROR_NOT_FOUND:
      return "FILE_ERROR_NOT_FOUND";
    case FILE_ERROR_ACCESS_DENIED:
      return "FILE_ERROR_ACCESS_DENIED";
    case FILE_ERROR_TOO_MANY_OPENED:
      return "FILE_ERROR_TOO_MANY_OPENED";
    case FILE_ERROR_NO_MEMORY:
      return "FILE_ERROR_NO_MEMORY";
    case FILE_ERROR_NO_SPACE:
      return "FILE_ERROR_NO_SPACE";
    case FILE_ERROR_NOT_A_DIRECTORY:
      return "FILE_ERROR_NOT_A_DIRECTORY";
    case FILE_ERROR_INVALID_OPERATION:
      return "FILE_ERROR_INVALID_OPERATION";
    case FILE_ERROR_SECURITY:
      return "FILE_ERROR_SECURITY";
    case FILE_ERROR_ABORT:
      return "FILE_ERROR_ABORT";
    case FILE_ERROR_NOT_A_FILE:
      return "FILE_ERROR_NOT_A_FILE";
    case FILE_ERROR_NOT_EMPTY:
      return "FILE_ERROR_NOT_EMPTY";
    case FILE_ERROR_INVALID_URL:
      return "FILE_ERROR_INVALID_URL";
    case FILE_ERROR_IO:
      return "FILE_ERROR_IO";
    case FILE_ERROR_MAX:
      break;
  }

  CR_NOTREACHED();
  return "";
}

bool File::Flush() {
  ///ElapsedTimer timer;
  CR_SCOPED_FILE_TRACE("Flush");
  bool return_value = DoFlush();
  ///UMA_HISTOGRAM_TIMES("PlatformFile.FlushTime", timer.Elapsed());
  return return_value;
}

}  // namespace crbase