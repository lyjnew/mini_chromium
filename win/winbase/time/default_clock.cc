// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "winbase\time\default_clock.h"

#include "winbase\lazy_instance.h"

namespace winbase {

DefaultClock::~DefaultClock() = default;

Time DefaultClock::Now() const {
  return Time::Now();
}

// static
DefaultClock* DefaultClock::GetInstance() {
  static LazyInstance<DefaultClock>::Leaky instance =
      WINBASE_LAZY_INSTANCE_INITIALIZER;
  return instance.Pointer();
}

}  // namespace winbase