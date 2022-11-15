// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_CRBASE_WIN_SCOPED_HGLOBAL_H_
#define MINI_CHROMIUM_CRBASE_WIN_SCOPED_HGLOBAL_H_

#include <windows.h>
#include <stddef.h>

#include "crbase/macros.h"

namespace crbase {
namespace win {

// Like ScopedHandle except for HGLOBAL.
template<class T>
class ScopedHGlobal {
 public:
  explicit ScopedHGlobal(HGLOBAL glob) : glob_(glob) {
    data_ = static_cast<T>(GlobalLock(glob_));
  }
  ~ScopedHGlobal() {
    GlobalUnlock(glob_);
  }

  T get() { return data_; }

  size_t Size() const { return GlobalSize(glob_); }

  T operator->() const {
    assert(data_ != 0);
    return data_;
  }

  T release() {
    T data = data_;
    data_ = NULL;
    return data;
  }

 private:
  HGLOBAL glob_;

  T data_;

  CR_DISALLOW_COPY_AND_ASSIGN(ScopedHGlobal);
};

}  // namespace win
}  // namespace crbase

#endif  // MINI_CHROMIUM_CRBASE_WIN_SCOPED_HGLOBAL_H_