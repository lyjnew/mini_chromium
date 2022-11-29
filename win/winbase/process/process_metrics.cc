// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "winbase\process\process_metrics.h"

#include <utility>

#include "winbase\logging.h"
#include "winbase\values.h"

namespace winbase {

SystemMemoryInfoKB::SystemMemoryInfoKB() = default;

SystemMemoryInfoKB::SystemMemoryInfoKB(const SystemMemoryInfoKB& other) =
    default;

SystemMetrics::SystemMetrics() {
  committed_memory_ = 0;
}

SystemMetrics SystemMetrics::Sample() {
  SystemMetrics system_metrics;

  system_metrics.committed_memory_ = GetSystemCommitCharge();
  return system_metrics;
}

std::unique_ptr<Value> SystemMetrics::ToValue() const {
  std::unique_ptr<DictionaryValue> res(new DictionaryValue());

  res->SetInteger("committed_memory", static_cast<int>(committed_memory_));
  return std::move(res);
}

std::unique_ptr<ProcessMetrics> ProcessMetrics::CreateCurrentProcessMetrics() {
  return CreateProcessMetrics(winbase::GetCurrentProcessHandle());
}

double ProcessMetrics::GetPlatformIndependentCPUUsage() {
  TimeDelta cumulative_cpu = GetCumulativeCPUUsage();
  TimeTicks time = TimeTicks::Now();

  if (last_cumulative_cpu_.is_zero()) {
    // First call, just set the last values.
    last_cumulative_cpu_ = cumulative_cpu;
    last_cpu_time_ = time;
    return 0;
  }

  TimeDelta system_time_delta = cumulative_cpu - last_cumulative_cpu_;
  TimeDelta time_delta = time - last_cpu_time_;
  WINBASE_DCHECK(!time_delta.is_zero());
  if (time_delta.is_zero())
    return 0;

  last_cumulative_cpu_ = cumulative_cpu;
  last_cpu_time_ = time;

  return 100.0 * system_time_delta.InMicrosecondsF() /
         time_delta.InMicrosecondsF();
}

int ProcessMetrics::GetIdleWakeupsPerSecond() {
  WINBASE_NOTIMPLEMENTED();  // http://crbug.com/120488
  return 0;
}

}  // namespace winbase