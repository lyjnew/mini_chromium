// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_POWER_MONITOR_POWER_MONITOR_DEVICE_SOURCE_H_
#define WINLIB_WINBASE_POWER_MONITOR_POWER_MONITOR_DEVICE_SOURCE_H_

#include <windows.h>

#include "winbase\base_export.h"
#include "winbase\macros.h"
#include "winbase\power_monitor\power_monitor_source.h"
#include "winbase\power_monitor\power_observer.h"

namespace winbase {

// A class used to monitor the power state change and notify the observers about
// the change event.
class WINBASE_EXPORT PowerMonitorDeviceSource : public PowerMonitorSource {
 public:
  PowerMonitorDeviceSource();
  PowerMonitorDeviceSource(const PowerMonitorDeviceSource&) = delete;
  PowerMonitorDeviceSource& operator=(const PowerMonitorDeviceSource&) = delete;
  ~PowerMonitorDeviceSource() override;

  void Shutdown() override;

 private:
  // Represents a message-only window for power message handling on Windows.
  // Only allow PowerMonitor to create it.
  class PowerMessageWindow {
   public:
    PowerMessageWindow();
    ~PowerMessageWindow();

   private:
    static LRESULT CALLBACK WndProcThunk(HWND hwnd,
                                         UINT message,
                                         WPARAM wparam,
                                         LPARAM lparam);
    // Instance of the module containing the window procedure.
    HMODULE instance_;
    // A hidden message-only window.
    HWND message_hwnd_;
  };

  // Platform-specific method to check whether the system is currently
  // running on battery power.  Returns true if running on batteries,
  // false otherwise.
  bool IsOnBatteryPowerImpl() override;

  PowerMessageWindow power_message_window_;
};

}  // namespace winbase

#endif  // WINLIB_WINBASE_POWER_MONITOR_POWER_MONITOR_DEVICE_SOURCE_H_