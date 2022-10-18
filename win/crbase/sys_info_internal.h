// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_CRBASE_SYS_INFO_INTERNAL_H_
#define MINI_CHROMIUM_CRBASE_SYS_INFO_INTERNAL_H_

#include "crbase/macros.h"

namespace crbase {

namespace internal {

template<typename T, T (*F)(void)>
class LazySysInfoValue {
 public:
  LazySysInfoValue()
      : value_(F()) { }

  ~LazySysInfoValue() { }

  T value() { return value_; }

 private:
  const T value_;

  CR_DISALLOW_COPY_AND_ASSIGN(LazySysInfoValue);
};

}  // namespace internal

}  // namespace crbase

#endif  // MINI_CHROMIUM_CRBASE_SYS_INFO_INTERNAL_H_