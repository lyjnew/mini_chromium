// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_TIME_CLOCK_H_
#define WINLIB_WINBASE_TIME_CLOCK_H_

#include "winbase\base_export.h"
#include "winbase\time\time.h"

namespace winbase {

// A Clock is an interface for objects that vend Times.  It is
// intended to be able to test the behavior of classes with respect to
// time.
//
// See DefaultClock (winbase\time\default_clock.h) for the default
// implementation that simply uses Time::Now().
//
// (An implementation that uses Time::SystemTime() should be added as
// needed.)
//
// See TickClock (winbase\time\tick_clock.h) for the equivalent interface for
// TimeTicks.
class WINBASE_EXPORT Clock {
 public:
  virtual ~Clock();

  // Now() must be safe to call from any thread.  The caller cannot
  // make any ordering assumptions about the returned Time.  For
  // example, the system clock may change to an earlier time.
  virtual Time Now() const = 0;
};

}  // namespace winbase

#endif  // WINLIB_WINBASE_TIME_CLOCK_H_