// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_AT_EXIT_H_
#define WINLIB_WINBASE_AT_EXIT_H_

#include "winbase\base_export.h"
#include "winbase\functional\callback.h"
#include "winbase\containers\stack.h"
#include "winbase\macros.h"
#include "winbase\synchronization\lock.h"

namespace winbase {

// This class provides a facility similar to the CRT atexit(), except that
// we control when the callbacks are executed. Under Windows for a DLL they
// happen at a really bad time and under the loader lock. This facility is
// mostly used by base::Singleton.
//
// The usage is simple. Early in the main() or WinMain() scope create an
// AtExitManager object on the stack:
// int main(...) {
//    winbase::AtExitManager exit_manager;
//
// }
// When the exit_manager object goes out of scope, all the registered
// callbacks and singleton destructors will be called.

class WINBASE_EXPORT AtExitManager {
 public:
  typedef void (*AtExitCallbackType)(void*);

  AtExitManager();

  // The dtor calls all the registered callbacks. Do not try to register more
  // callbacks after this point.
  ~AtExitManager();

  AtExitManager(const AtExitManager&) = delete;
  AtExitManager& operator=(const AtExitManager&) = delete;

  // Registers the specified function to be called at exit. The prototype of
  // the callback function is void func(void*).
  static void RegisterCallback(AtExitCallbackType func, void* param);

  // Registers the specified task to be called at exit.
  static void RegisterTask(winbase::Closure task);

  // Calls the functions registered with RegisterCallback in LIFO order. It
  // is possible to register new callbacks after calling this function.
  static void ProcessCallbacksNow();

  // Disable all registered at-exit callbacks. This is used only in a single-
  // process mode.
  static void DisableAllAtExitManagers();

 protected:
  // This constructor will allow this instance of AtExitManager to be created
  // even if one already exists.  This should only be used for testing!
  // AtExitManagers are kept on a global stack, and it will be removed during
  // destruction.  This allows you to shadow another AtExitManager.
  explicit AtExitManager(bool shadow);

 private:
  winbase::Lock lock_;
  winbase::stack<base::Closure> stack_;
  bool processing_callbacks_;
  AtExitManager* next_manager_;  // Stack of managers to allow shadowing.
};

}  // namespace base

#endif  // WINLIB_WINBASE_AT_EXIT_H_