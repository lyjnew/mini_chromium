// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_TIME_DEFAULT_CLOCK_H_
#define WINLIB_WINBASE_TIME_DEFAULT_CLOCK_H_

#include "winbase\base_export.h"
#include "winbase\compiler_specific.h"
#include "winbase\time\clock.h"

namespace winbase {

// DefaultClock is a Clock implementation that uses Time::Now().
class WINBASE_EXPORT DefaultClock : public Clock {
 public:
  ~DefaultClock() override;

  // Simply returns Time::Now().
  Time Now() const override;

  // Returns a shared instance of DefaultClock. This is thread-safe.
  static DefaultClock* GetInstance();
};

}  // namespace base

#endif  // WINLIB_WINBASE_TIME_DEFAULT_CLOCK_H_