// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_WIN_SCOPED_WINDOWS_THREAD_ENVIRONMENT_H_
#define WINLIB_WINBASE_WIN_SCOPED_WINDOWS_THREAD_ENVIRONMENT_H_

#include "winbase\macros.h"

namespace winbase {
namespace win {

// Serves as a root class for ScopedCOMInitializer and ScopedWinrtInitializer.
class ScopedWindowsThreadEnvironment {
 public:
  ScopedWindowsThreadEnvironment() {}
  ScopedWindowsThreadEnvironment(
      const ScopedWindowsThreadEnvironment&) = delete;
  ScopedWindowsThreadEnvironment& operator=(
      const ScopedWindowsThreadEnvironment&) = delete;
  virtual ~ScopedWindowsThreadEnvironment() {}

  virtual bool Succeeded() const = 0;
};

}  // namespace win
}  // namespace winbase

#endif  // WINLIB_WINBASE_WIN_SCOPED_WINDOWS_THREAD_ENVIRONMENT_H_