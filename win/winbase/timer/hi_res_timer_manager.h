// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_TIMER_HI_RES_TIMER_MANAGER_H_
#define WINLIB_WINBASE_TIMER_HI_RES_TIMER_MANAGER_H_

#include "winbase\base_export.h"
#include "winbase\macros.h"
#include "winbase\memory\ref_counted.h"
#include "winbase\power_monitor\power_observer.h"
#include "winbase\timer\timer.h"

namespace winbase {

// Ensures that the Windows high resolution timer is only used
// when not running on battery power.
class WINBASE_EXPORT HighResolutionTimerManager 
    : public winbase::PowerObserver {
 public:
  HighResolutionTimerManager();
  HighResolutionTimerManager(const HighResolutionTimerManager&) = delete;
  HighResolutionTimerManager& operator=(const HighResolutionTimerManager&) 
      = delete;
  ~HighResolutionTimerManager() override;

  // winbase::PowerObserver methods.
  void OnPowerStateChange(bool on_battery_power) override;
  void OnSuspend() override;
  void OnResume() override;

  // Returns true if the hi resolution clock could be used right now.
  bool hi_res_clock_available() const { return hi_res_clock_available_; }

 private:
  // Enable or disable the faster multimedia timer.
  void UseHiResClock(bool use);

  bool hi_res_clock_available_;

  // Timer for polling the high resolution timer usage.
  winbase::RepeatingTimer timer_;
};

}  // namespace winbase

#endif  // WINLIB_WINBASE_TIMER_HI_RES_TIMER_MANAGER_H_