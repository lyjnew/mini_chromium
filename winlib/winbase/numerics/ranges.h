// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_NUMERICS_RANGES_H_
#define WINLIB_WINBASE_NUMERICS_RANGES_H_

#include <algorithm>
#include <cmath>

#include "winbase\win\nominmax.h"

namespace winbase {

// To be replaced with std::clamp() from C++17, someday.
template <class T>
constexpr const T& ClampToRange(const T& value, const T& min, const T& max) {
  return std::min(std::max(value, min), max);
}

template <typename T>
constexpr bool IsApproximatelyEqual(T lhs, T rhs, T tolerance) {
  static_assert(std::is_arithmetic<T>::value, "Argument must be arithmetic");
  return std::abs(rhs - lhs) <= tolerance;
}

}  // namespace base

#endif  // WINLIB_WINBASE_NUMERICS_RANGES_H_