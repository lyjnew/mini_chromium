// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINBASE_WINBASE_WIN_SCOPED_CO_MEM_H_
#define WINBASE_WINBASE_WIN_SCOPED_CO_MEM_H_

#include <objbase.h>

#include "winbase\logging.h"
#include "winbase\macros.h"

namespace winbase {
namespace win {

// Simple scoped memory releaser class for COM allocated memory.
// Example:
//   winbase::win::ScopedCoMem<ITEMIDLIST> file_item;
//   SHGetSomeInfo(&file_item, ...);
//   ...
//   return;  <-- memory released
template <typename T>
class ScopedCoMem {
 public:
  ScopedCoMem() : mem_ptr_(NULL) {}
  ScopedCoMem(const ScopedCoMem&) = delete;
  ScopedCoMem& operator=(const ScopedCoMem&) = delete;
  ~ScopedCoMem() { Reset(NULL); }

  T** operator&() {            // NOLINT
    WINBASE_DCHECK(mem_ptr_ == NULL);  // To catch memory leaks.
    return &mem_ptr_;
  }

  operator T*() { return mem_ptr_; }

  T* operator->() {
    WINBASE_DCHECK(mem_ptr_ != NULL);
    return mem_ptr_;
  }

  const T* operator->() const {
    WINBASE_DCHECK(mem_ptr_ != NULL);
    return mem_ptr_;
  }

  void Reset(T* ptr) {
    if (mem_ptr_)
      CoTaskMemFree(mem_ptr_);
    mem_ptr_ = ptr;
  }

  T* get() const { return mem_ptr_; }

 private:
  T* mem_ptr_;
};

}  // namespace win
}  // namespace winbase

#endif  // WINBASE_WINBASE_WIN_SCOPED_CO_MEM_H_