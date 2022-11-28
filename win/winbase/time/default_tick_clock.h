// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_TIME_DEFAULT_TICK_CLOCK_H_
#define WINLIB_WINBASE_TIME_DEFAULT_TICK_CLOCK_H_

#include "winbase\base_export.h"
#include "winbase\time\tick_clock.h"

namespace winbase {

// DefaultClock is a Clock implementation that uses TimeTicks::Now().
class WINBASE_EXPORT DefaultTickClock : public TickClock {
 public:
  ~DefaultTickClock() override;

  // Simply returns TimeTicks::Now().
  TimeTicks NowTicks() const override;

  // Returns a shared instance of DefaultTickClock. This is thread-safe.
  static const DefaultTickClock* GetInstance();
};

}  // namespace winbase

#endif  // WINLIB_WINBASE_TIME_DEFAULT_TICK_CLOCK_H_