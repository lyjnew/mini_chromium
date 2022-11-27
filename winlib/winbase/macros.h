// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file contains macros and macro-like constructs (e.g., templates) that
// are commonly used throughout Chromium source. (It may also contain things
// that are closely related to things that are commonly used that belong in this
// file.)

#ifndef WINLIB_WINBASE_BASIC_MACROS_H_
#define WINLIB_WINBASE_BASIC_MACROS_H_

#include <stddef.h>

// declare WinBaseArraySizeHelper as returning point to array.
// see: https://stackoverflow.com/questions/3473438/return-array-in-a-function
template <class T, size_t N>
char (*WinBaseArraySizeHelper(T(&array)[N]))[N] {};

#define array_size(array) (sizeof(*WinBaseArraySizeHelper(array)))

#endif  // WINLIB_WINBASE_BASIC_MACROS_H_