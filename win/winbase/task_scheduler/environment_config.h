// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_TASK_SCHEDULER_ENVIRONMENT_CONFIG_H_
#define WINLIB_WINBASE_TASK_SCHEDULER_ENVIRONMENT_CONFIG_H_

#include <stddef.h>

#include "winbase\base_export.h"
#include "winbase\task_scheduler\task_traits.h"
#include "winbase\threading/thread.h"

namespace winbase {
namespace internal {

enum EnvironmentType {
  FOREGROUND = 0,
  FOREGROUND_BLOCKING,
  // Pools will only be created for the environment above on platforms that
  // don't support SchedulerWorkers running with a background priority.
  ENVIRONMENT_COUNT_WITHOUT_BACKGROUND_PRIORITY,
  BACKGROUND = ENVIRONMENT_COUNT_WITHOUT_BACKGROUND_PRIORITY,
  BACKGROUND_BLOCKING,
  ENVIRONMENT_COUNT  // Always last.
};

// Order must match the EnvironmentType enum.
constexpr struct {
  // The threads and histograms of this environment will be labeled with
  // the task scheduler name concatenated to this.
  const char* name_suffix;

  // Preferred priority for threads in this environment; the actual thread
  // priority depends on shutdown state and platform capabilities.
  ThreadPriority priority_hint;
} kEnvironmentParams[] = {
    {"Foreground", winbase::ThreadPriority::NORMAL},
    {"ForegroundBlocking", winbase::ThreadPriority::NORMAL},
    {"Background", winbase::ThreadPriority::BACKGROUND},
    {"BackgroundBlocking", winbase::ThreadPriority::BACKGROUND},
};

size_t WINBASE_EXPORT GetEnvironmentIndexForTraits(const TaskTraits& traits);

// Returns true if this platform supports having SchedulerWorkers running with a
// background priority.
bool WINBASE_EXPORT CanUseBackgroundPriorityForSchedulerWorker();

}  // namespace internal
}  // namespace winbase

#endif  // WINLIB_WINBASE_TASK_SCHEDULER_ENVIRONMENT_CONFIG_H_