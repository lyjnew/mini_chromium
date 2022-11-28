// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "winbase\debug\debugger.h"

#include <stdlib.h>
#include <windows.h>

///#include "winbase\logging.h"
///#include "winbase\threading\platform_thread.h"

namespace winbase {
namespace debug {

static bool is_debug_ui_suppressed = false;

bool WaitForDebugger(int wait_seconds, bool silent) {
  for (int i = 0; i < wait_seconds * 10; ++i) {
    if (BeingDebugged()) {
      if (!silent)
        BreakDebugger();
      return true;
    }
    ///PlatformThread::Sleep(TimeDelta::FromMilliseconds(100));
  }
  return false;
}

bool BeingDebugged() {
  return ::IsDebuggerPresent() != 0;
}

void BreakDebugger() {
  if (IsDebugUISuppressed())
    _exit(1);

  __debugbreak();
}

void SetSuppressDebugUI(bool suppress) {
  is_debug_ui_suppressed = suppress;
}

bool IsDebugUISuppressed() {
  return is_debug_ui_suppressed;
}

}  // namespace debug
}  // namespace winbase