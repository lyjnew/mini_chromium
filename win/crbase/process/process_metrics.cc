// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/process/process_metrics.h"

#include <utility>

#include "crbase/logging.h"
#include "crbase/values.h"
#include "crbuild/build_config.h"

namespace crbase {

SystemMetrics::SystemMetrics() {
  committed_memory_ = 0;
}

SystemMetrics SystemMetrics::Sample() {
  SystemMetrics system_metrics;

  system_metrics.committed_memory_ = GetSystemCommitCharge();
  return system_metrics;
}

scoped_ptr<Value> SystemMetrics::ToValue() const {
  scoped_ptr<DictionaryValue> res(new DictionaryValue());

  res->SetInteger("committed_memory", static_cast<int>(committed_memory_));
  return std::move(res);
}

ProcessMetrics* ProcessMetrics::CreateCurrentProcessMetrics() {
  return CreateProcessMetrics(crbase::GetCurrentProcessHandle());
}

double ProcessMetrics::GetPlatformIndependentCPUUsage() {
  return GetCPUUsage() * processor_count_;
}

int ProcessMetrics::GetIdleWakeupsPerSecond() {
  CR_NOTIMPLEMENTED();  // http://crbug.com/120488
  return 0;
}

}  // namespace crbase