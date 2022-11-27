// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_CALLBACK_FORWARD_H_
#define WINLIB_WINBASE_CALLBACK_FORWARD_H_

namespace winbase {

template <typename Signature>
class OnceCallback;

template <typename Signature>
class RepeatingCallback;

template <typename Signature>
using Callback = RepeatingCallback<Signature>;

// Syntactic sugar to make Callback<void()> easier to declare since it
// will be used in a lot of APIs with delayed execution.
using OnceClosure = OnceCallback<void()>;
using RepeatingClosure = RepeatingCallback<void()>;
using Closure = Callback<void()>;

}  // namespace winbase

#endif  // WINLIB_WINBASE_CALLBACK_FORWARD_H_