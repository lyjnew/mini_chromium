// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "winbase\time\default_tick_clock.h"

#include "winbase\no_destructor.h"

namespace winbase {

DefaultTickClock::~DefaultTickClock() = default;

TimeTicks DefaultTickClock::NowTicks() const {
  return TimeTicks::Now();
}

// static
const DefaultTickClock* DefaultTickClock::GetInstance() {
  static const winbase::NoDestructor<DefaultTickClock> default_tick_clock;
  return default_tick_clock.get();
}

}  // namespace winbase