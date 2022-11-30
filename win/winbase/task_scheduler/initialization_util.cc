// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "winbase\task_scheduler\initialization_util.h"

#include <algorithm>

#include "winbase\system\sys_info.h"

namespace winbase {

int RecommendedMaxNumberOfThreadsInPool(int min,
                                        int max,
                                        double cores_multiplier,
                                        int offset) {
  const int num_of_cores = SysInfo::NumberOfProcessors();
  const int threads = static_cast<int>(std::ceil<int>(
      static_cast<int>(num_of_cores * cores_multiplier)) + offset);
  return std::min(max, std::max(min, threads));
}

}  // namespace winbase