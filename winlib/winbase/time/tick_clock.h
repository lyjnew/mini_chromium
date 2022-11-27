// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_TIME_TICK_CLOCK_H_
#define WINLIB_WINBASE_TIME_TICK_CLOCK_H_

#include "winbase\base_export.h"
#include "winbase\time\time.h"

namespace winbase {

// A TickClock is an interface for objects that vend TimeTicks.  It is
// intended to be able to test the behavior of classes with respect to
// non-decreasing time.
//
// See DefaultTickClock (winbase\time\default_tick_clock.h) for the default
// implementation that simply uses TimeTicks::Now().
//
// (Other implementations that use TimeTicks::NowFromSystemTime() should
// be added as needed.)
//
//
// See Clock (winbase\time\clock.h) for the equivalent interface for Times.
class WINBASE_EXPORT TickClock {
 public:
  virtual ~TickClock();

  // NowTicks() must be safe to call from any thread.  The caller may
  // assume that NowTicks() is monotonic (but not strictly monotonic).
  // In other words, the returned TimeTicks will never decrease with
  // time, although they might "stand still".
  virtual TimeTicks NowTicks() const = 0;
};

}  // namespace winbase

#endif  // WINLIB_WINBASE_TIME_TICK_CLOCK_H_