// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This is a separate file so that users of process metrics don't need to
// include windows.h unless they need IoCounters.

#ifndef WINLIB_WINBASE_PROCESS_PROCESS_METRICS_IOCOUNTERS_H_
#define WINLIB_WINBASE_PROCESS_PROCESS_METRICS_IOCOUNTERS_H_

#include <stdint.h>

#include "winbase\process\process_metrics.h"
#include "winbase\win\windows_types.h"

namespace winbase {

struct IoCounters : public IO_COUNTERS {};

}  // namespace winbase

#endif  // WINLIB_WINBASE_PROCESS_PROCESS_METRICS_IOCOUNTERS_H_