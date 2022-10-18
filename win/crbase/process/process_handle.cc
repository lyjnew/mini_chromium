// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include "crbase/logging.h"
#include "crbase/process/process_handle.h"
#include "crbuild/build_config.h"

namespace crbase {

namespace {
bool g_have_unique_id = false;
uint32_t g_unique_id;

// The process which set |g_unique_id|.
ProcessId g_procid;

// Mangle IDs so that they are not accidentally used as PIDs, e.g. as an
// argument to kill or waitpid.
uint32_t MangleProcessId(ProcessId process_id) {
  // Add a large power of 10 so that the pid is still the pid is still readable
  // inside the mangled id.
  return static_cast<uint32_t>(process_id) + 1000000000U;
}

}  // namespace

uint32_t GetUniqueIdForProcess() {
  if (!g_have_unique_id) {
    return MangleProcessId(GetCurrentProcId());
  }

  // Make sure we are the same process that set |g_procid|. This check may have
  // false negatives (if a process ID was reused) but should have no false
  // positives.
  CR_DCHECK_EQ(GetCurrentProcId(), g_procid);
  return g_unique_id;
}

}  // namespace crbase