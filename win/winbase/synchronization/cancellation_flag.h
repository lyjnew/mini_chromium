// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_SYNCHRONIZATION_CANCELLATION_FLAG_H_
#define WINLIB_WINBASE_SYNCHRONIZATION_CANCELLATION_FLAG_H_

#include "winbase\synchronization\atomic_flag.h"

namespace winbase {

// Use inheritance instead of "using" to allow forward declaration of "class
// CancellationFlag".
// TODO(fdoray): Replace CancellationFlag with AtomicFlag throughout the
// codebase and delete this file. crbug.com/630251
class CancellationFlag : public AtomicFlag {};

}  // namespace winbase

#endif  // WINLIB_WINBASE_SYNCHRONIZATION_CANCELLATION_FLAG_H_