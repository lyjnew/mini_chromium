// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_POWER_MONITOR_POWER_OBSERVER_H_
#define WINLIB_WINBASE_POWER_MONITOR_POWER_OBSERVER_H_

#include "winbase\base_export.h"
#include "winbase\compiler_specific.h"

namespace winbase {

class WINBASE_EXPORT PowerObserver {
 public:
  // Notification of a change in power status of the computer, such
  // as from switching between battery and A/C power.
  virtual void OnPowerStateChange(bool on_battery_power){};

  // Notification that the system is suspending.
  virtual void OnSuspend() {}

  // Notification that the system is resuming.
  virtual void OnResume() {}

 protected:
  virtual ~PowerObserver() = default;
};

}  // namespace winbase

#endif  // WINLIB_WINBASE_POWER_MONITOR_POWER_OBSERVER_H_