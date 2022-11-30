// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_TASK_SCHEDULER_SCHEDULER_WORKER_POOL_H_
#define WINLIB_WINBASE_TASK_SCHEDULER_SCHEDULER_WORKER_POOL_H_

#include "winbase\base_export.h"
#include "winbase\memory\ref_counted.h"
#include "winbase\sequenced_task_runner.h"
#include "winbase\task_runner.h"
#include "winbase\task_scheduler\can_schedule_sequence_observer.h"
#include "winbase\task_scheduler\sequence.h"
#include "winbase\task_scheduler\task.h"
#include "winbase\task_scheduler\task_traits.h"
#include "winbase\task_scheduler\tracked_ref.h"

namespace winbase {
namespace internal {

class DelayedTaskManager;
class TaskTracker;

// Interface for a worker pool.
class WINBASE_EXPORT SchedulerWorkerPool : public CanScheduleSequenceObserver {
 public:
  SchedulerWorkerPool(const SchedulerWorkerPool&) = delete;
  SchedulerWorkerPool& operator=(const SchedulerWorkerPool&) = delete;
  ~SchedulerWorkerPool() override;

  // Returns a TaskRunner whose PostTask invocations result in scheduling tasks
  // in this SchedulerWorkerPool using |traits|. Tasks may run in any order and
  // in parallel.
  scoped_refptr<TaskRunner> CreateTaskRunnerWithTraits(
      const TaskTraits& traits);

  // Returns a SequencedTaskRunner whose PostTask invocations result in
  // scheduling tasks in this SchedulerWorkerPool using |traits|. Tasks run one
  // at a time in posting order.
  scoped_refptr<SequencedTaskRunner> CreateSequencedTaskRunnerWithTraits(
      const TaskTraits& traits);

  // Posts |task| to be executed by this SchedulerWorkerPool as part of
  // |sequence|. |task| won't be executed before its delayed run time, if any.
  // Returns true if |task| is posted.
  bool PostTaskWithSequence(Task task, scoped_refptr<Sequence> sequence);

  // Registers the worker pool in TLS.
  void BindToCurrentThread();

  // Resets the worker pool in TLS.
  void UnbindFromCurrentThread();

  // Prevents new tasks from starting to run and waits for currently running
  // tasks to complete their execution. It is guaranteed that no thread will do
  // work on behalf of this SchedulerWorkerPool after this returns. It is
  // invalid to post a task once this is called. TaskTracker::Flush() can be
  // called before this to complete existing tasks, which might otherwise post a
  // task during JoinForTesting(). This can only be called once.
  virtual void JoinForTesting() = 0;

 protected:
  SchedulerWorkerPool(TrackedRef<TaskTracker> task_tracker,
                      DelayedTaskManager* delayed_task_manager);

  // Posts |task| to be executed by this SchedulerWorkerPool as part of
  // |sequence|. This must only be called after |task| has gone through
  // PostTaskWithSequence() and after |task|'s delayed run time.
  void PostTaskWithSequenceNow(Task task, scoped_refptr<Sequence> sequence);

  const TrackedRef<TaskTracker> task_tracker_;
  DelayedTaskManager* const delayed_task_manager_;
};

}  // namespace internal
}  // namespace winbase

#endif  // WINLIB_WINBASE_TASK_SCHEDULER_SCHEDULER_WORKER_POOL_H_