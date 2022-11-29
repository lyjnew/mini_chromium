// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_FUNCTIONAL_BIND_HELPERS_H_
#define WINLIB_WINBASE_FUNCTIONAL_BIND_HELPERS_H_

#include <stddef.h>

#include <type_traits>
#include <utility>

#include "winbase\functional\bind.h"
#include "winbase\functional\callback.h"
#include "winbase\memory\weak_ptr.h"
#include "winlib\build_config.h"

// This defines a set of simple functions and utilities that people want when
// using Callback<> and Bind().

namespace winbase {

// Creates a null callback.
class WINBASE_EXPORT NullCallback {
 public:
  template <typename R, typename... Args>
  operator RepeatingCallback<R(Args...)>() const {
    return RepeatingCallback<R(Args...)>();
  }
  template <typename R, typename... Args>
  operator OnceCallback<R(Args...)>() const {
    return OnceCallback<R(Args...)>();
  }
};

// Creates a callback that does nothing when called.
class WINBASE_EXPORT DoNothing {
 public:
  template <typename... Args>
  operator RepeatingCallback<void(Args...)>() const {
    return Repeatedly<Args...>();
  }
  template <typename... Args>
  operator OnceCallback<void(Args...)>() const {
    return Once<Args...>();
  }
  // Explicit way of specifying a specific callback type when the compiler can't
  // deduce it.
  template <typename... Args>
  static RepeatingCallback<void(Args...)> Repeatedly() {
    return BindRepeating([](Args... args) {});
  }
  template <typename... Args>
  static OnceCallback<void(Args...)> Once() {
    return BindOnce([](Args... args) {});
  }
};

// Useful for creating a Closure that will delete a pointer when invoked. Only
// use this when necessary. In most cases MessageLoop::DeleteSoon() is a better
// fit.
template <typename T>
void DeletePointer(T* obj) {
  delete obj;
}

}  // namespace winbase

#endif  // WINLIB_WINBASE_FUNCTIONAL_BIND_HELPERS_H_