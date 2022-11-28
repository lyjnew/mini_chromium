// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_PENDING_TASK_H_
#define WINLIB_WINBASE_PENDING_TASK_H_

#include <array>

#include "winbase\base_export.h"
#include "winbase\functional\callback.h"
#include "winbase\containers\queue.h"
#include "winbase\location.h"
#include "winbase\time\time.h"

namespace winbase {

enum class Nestable {
  kNonNestable,
  kNestable,
};

// Contains data about a pending task. Stored in TaskQueue and DelayedTaskQueue
// for use by classes that queue and execute tasks.
struct WINBASE_EXPORT PendingTask {
  PendingTask(const Location& posted_from,
              OnceClosure task,
              TimeTicks delayed_run_time = TimeTicks(),
              Nestable nestable = Nestable::kNestable);
  PendingTask(PendingTask&& other);
  ~PendingTask();

  PendingTask& operator=(PendingTask&& other);

  // Used to support sorting.
  bool operator<(const PendingTask& other) const;

  // The task to run.
  OnceClosure task;

  // The site this PendingTask was posted from.
  Location posted_from;

  // The time when the task should be run.
  winbase::TimeTicks delayed_run_time;

  // Chain of up-to-four symbols of the parent tasks which led to this one being
  // posted.
  std::array<const void*, 4> task_backtrace = {};

  // Secondary sort key for run time.
  int sequence_num = 0;

  // OK to dispatch from a nested loop.
  Nestable nestable;

  // Needs high resolution timers.
  bool is_high_res = false;
};

using TaskQueue = winbase::queue<PendingTask>;

// PendingTasks are sorted by their |delayed_run_time| property.
using DelayedTaskQueue = std::priority_queue<winbase::PendingTask>;

}  // namespace winbase

#endif  // WINLIB_WINBASE_PENDING_TASK_H_