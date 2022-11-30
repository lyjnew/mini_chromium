// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "winbase\task_scheduler\scoped_set_task_priority_for_current_thread.h"

#include "winbase\lazy_instance.h"
#include "winbase\logging.h"
#include "winbase\threading\thread_local.h"

namespace winbase {
namespace internal {

namespace {

LazyInstance<ThreadLocalPointer<const TaskPriority>>::Leaky
    tls_task_priority_for_current_thread = WINBASE_LAZY_INSTANCE_INITIALIZER;

}  // namespace

ScopedSetTaskPriorityForCurrentThread::ScopedSetTaskPriorityForCurrentThread(
    TaskPriority priority)
    : priority_(priority) {
  WINBASE_DCHECK(!tls_task_priority_for_current_thread.Get().Get());
  tls_task_priority_for_current_thread.Get().Set(&priority_);
}

ScopedSetTaskPriorityForCurrentThread::
    ~ScopedSetTaskPriorityForCurrentThread() {
  WINBASE_DCHECK_EQ(&priority_,
                    tls_task_priority_for_current_thread.Get().Get());
  tls_task_priority_for_current_thread.Get().Set(nullptr);
}

TaskPriority GetTaskPriorityForCurrentThread() {
  const TaskPriority* priority =
      tls_task_priority_for_current_thread.Get().Get();
  return priority ? *priority : TaskPriority::USER_VISIBLE;
}

}  // namespace internal
}  // namespace winbase