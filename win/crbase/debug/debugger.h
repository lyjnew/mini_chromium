// Copyright 2011 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This is a cross platform interface for helper functions related to
// debuggers.  You should use this to test if you're running under a debugger,
// and if you would like to yield (breakpoint) into the debugger.

#ifndef MINI_CHROMIUM_CRBASE_DEBUG_DEBUGGER_H_
#define MINI_CHROMIUM_CRBASE_DEBUG_DEBUGGER_H_

#include "crbase/base_export.h"

namespace crbase {
namespace debug {

// Waits wait_seconds seconds for a debugger to attach to the current process.
// When silent is false, an exception is thrown when a debugger is detected.
CRBASE_EXPORT bool WaitForDebugger(int wait_seconds, bool silent);

// Returns true if the given process is being run under a debugger.
//
// On OS X, the underlying mechanism doesn't work when the sandbox is enabled.
// To get around this, this function caches its value.
//
// WARNING: Because of this, on OS X, a call MUST be made to this function
// BEFORE the sandbox is enabled.
CRBASE_EXPORT bool BeingDebugged();

// Break into the debugger, assumes a debugger is present.
CRBASE_EXPORT void BreakDebugger();
// Async-safe version of BreakDebugger(). In particular, this does not allocate
// any memory. More broadly, must be safe to call from anywhere, including
// signal handlers.
CRBASE_EXPORT void BreakDebuggerAsyncSafe();

// Used in test code, this controls whether showing dialogs and breaking into
// the debugger is suppressed for debug errors, even in debug mode (normally
// release mode doesn't do this stuff --  this is controlled separately).
// Normally UI is not suppressed.  This is normally used when running automated
// tests where we want a crash rather than a dialog or a debugger.
CRBASE_EXPORT void SetSuppressDebugUI(bool suppress);
CRBASE_EXPORT bool IsDebugUISuppressed();

// If a debugger is present, verifies that it is properly set up, and DCHECK()s
// if misconfigured.  Currently only verifies that //tools/gdb/gdbinit has been
// sourced when using gdb on Linux and //tools/lldb/lldbinit.py has been sourced
// when using lldb on macOS.
CRBASE_EXPORT void VerifyDebugger();

}  // namespace debug
}  // namespace crbase

#endif  // MINI_CHROMIUM_CRBASE_DEBUG_DEBUGGER_H_