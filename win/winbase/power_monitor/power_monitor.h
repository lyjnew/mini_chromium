// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_BASE_POWER_MONITOR_POWER_MONITOR_H_
#define WINLIB_BASE_POWER_MONITOR_POWER_MONITOR_H_

#include "winbase\base_export.h"
#include "winbase\macros.h"
#include "winbase\memory\ref_counted.h"
#include "winbase\observer_list_threadsafe.h"
#include "winbase\power_monitor\power_observer.h"

namespace winbase {

class PowerMonitorSource;

// A class used to monitor the power state change and notify the observers about
// the change event.
class WINBASE_EXPORT PowerMonitor {
 public:
  // Takes ownership of |source|.
  explicit PowerMonitor(std::unique_ptr<PowerMonitorSource> source);
  PowerMonitor(const PowerMonitor&) = delete;
  PowerMonitor& operator=(const PowerMonitor&) = delete;
  ~PowerMonitor();

  // Get the process-wide PowerMonitor (if not present, returns NULL).
  static PowerMonitor* Get();

  // Add and remove an observer.
  // Can be called from any thread. |observer| is notified on the sequence
  // from which it was registered.
  // Must not be called from within a notification callback.
  void AddObserver(PowerObserver* observer);
  void RemoveObserver(PowerObserver* observer);

  // Is the computer currently on battery power.
  bool IsOnBatteryPower();

 private:
  friend class PowerMonitorSource;

  PowerMonitorSource* Source();

  void NotifyPowerStateChange(bool battery_in_use);
  void NotifySuspend();
  void NotifyResume();

  scoped_refptr<ObserverListThreadSafe<PowerObserver>> observers_;
  std::unique_ptr<PowerMonitorSource> source_;
};

}  // namespace base

#endif  // WINLIB_BASE_POWER_MONITOR_POWER_MONITOR_H_