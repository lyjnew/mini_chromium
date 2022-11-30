// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_CONTAINERS_QUEUE_H_
#define WINLIB_WINBASE_CONTAINERS_QUEUE_H_

#include <queue>

#include "winbase\containers\circular_deque.h"

namespace winbase {

// Provides a definition of winbase::queue that's like std::queue but uses a
// winbase::circular_deque instead of std::deque. Since std::queue is just a
// wrapper for an underlying type, we can just provide a typedef for it that
// defaults to the base circular_deque.
template <class T, class Container = circular_deque<T>>
using queue = std::queue<T, Container>;

}  // namespace winbase

#endif  // WINLIB_WINBASE_CONTAINERS_QUEUE_H_