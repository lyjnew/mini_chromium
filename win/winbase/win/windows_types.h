// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file contains defines and typedefs that allow popular Windows types to
// be used without the overhead of including windows.h.

#ifndef WINLIB_WINBASE_WIN_WINDOWS_TYPES_H
#define WINLIB_WINBASE_WIN_WINDOWS_TYPES_H

// Needed for function prototypes.

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif  // WIN32_LEAN_AND_MEAN

#include <windows.h>

struct WINBASE_SRWLOCK {
  PVOID Ptr;
};

struct WINBASE_CONDITION_VARIABLE {
  PVOID Ptr;
};

#endif  // WINLIB_WINBASE_WIN_WINDOWS_TYPES_H