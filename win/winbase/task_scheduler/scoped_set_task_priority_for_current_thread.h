// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_TASK_SCHEDULER_SCOPED_SET_TASK_PRIORITY_FOR_CURRENT_THREAD_H_
#define WINLIB_WINBASE_TASK_SCHEDULER_SCOPED_SET_TASK_PRIORITY_FOR_CURRENT_THREAD_H_

#include "winbase\base_export.h"
#include "winbase\macros.h"
#include "winbase\task_scheduler\task_traits.h"

namespace winbase {
namespace internal {

class WINBASE_EXPORT ScopedSetTaskPriorityForCurrentThread {
 public:
  // Within the scope of this object, GetTaskPriorityForCurrentThread() will
  // return |priority|.
  ScopedSetTaskPriorityForCurrentThread(TaskPriority priority);
  ScopedSetTaskPriorityForCurrentThread(
      const ScopedSetTaskPriorityForCurrentThread&) = delete;
  ScopedSetTaskPriorityForCurrentThread& operator=(
      const ScopedSetTaskPriorityForCurrentThread&) = delete;
  ~ScopedSetTaskPriorityForCurrentThread();

 private:
  const TaskPriority priority_;
};

// Returns the priority of the TaskScheduler task running on the current thread,
// or TaskPriority::USER_VISIBLE if no TaskScheduler task is running on the
// current thread.
WINBASE_EXPORT TaskPriority GetTaskPriorityForCurrentThread();

}  // namespace internal
}  // namespace winbase

#endif  // WINLIB_WINBASE_TASK_SCHEDULER_SCOPED_SET_TASK_PRIORITY_FOR_CURRENT_THREAD_H_