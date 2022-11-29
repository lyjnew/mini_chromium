// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "winbase\power_monitor\power_monitor_device_source.h"

namespace winbase {

PowerMonitorDeviceSource::PowerMonitorDeviceSource() {
  // Provide the correct battery status if possible. Others platforms, such as
  // Android and ChromeOS, will update their status once their backends are
  // actually initialized.
  SetInitialOnBatteryPowerState(IsOnBatteryPowerImpl());
}

PowerMonitorDeviceSource::~PowerMonitorDeviceSource() {
}

// PowerMonitorDeviceSource does not need to take any special action to ensure
// that it doesn't callback into PowerMonitor after this phase of shutdown has
// completed.
void PowerMonitorDeviceSource::Shutdown() {}

}  // namespace winbase