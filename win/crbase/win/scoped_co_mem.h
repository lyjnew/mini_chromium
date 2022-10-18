// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_CRBASE_WIN_SCOPED_CO_MEM_H_
#define MINI_CHROMIUM_CRBASE_WIN_SCOPED_CO_MEM_H_

#include <objbase.h>

#include "crbase/logging.h"
#include "crbase/macros.h"

namespace crbase {
namespace win {

// Simple scoped memory releaser class for COM allocated memory.
// Example:
//   crbase::win::ScopedCoMem<ITEMIDLIST> file_item;
//   SHGetSomeInfo(&file_item, ...);
//   ...
//   return;  <-- memory released
template<typename T>
class ScopedCoMem {
 public:
  ScopedCoMem() : mem_ptr_(NULL) {}
  ~ScopedCoMem() {
    Reset(NULL);
  }

  T** operator&() {  // NOLINT
    CR_DCHECK(mem_ptr_ == NULL);  // To catch memory leaks.
    return &mem_ptr_;
  }

  operator T*() {
    return mem_ptr_;
  }

  T* operator->() {
    CR_DCHECK(mem_ptr_ != NULL);
    return mem_ptr_;
  }

  const T* operator->() const {
    CR_DCHECK(mem_ptr_ != NULL);
    return mem_ptr_;
  }

  void Reset(T* ptr) {
    if (mem_ptr_)
      CoTaskMemFree(mem_ptr_);
    mem_ptr_ = ptr;
  }

  T* get() const {
    return mem_ptr_;
  }

 private:
  T* mem_ptr_;

  CR_DISALLOW_COPY_AND_ASSIGN(ScopedCoMem);
};

}  // namespace win
}  // namespace crbase

#endif  // MINI_CHROMIUM_CRBASE_WIN_SCOPED_CO_MEM_H_