// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_TASK_SCHEDULER_TASK_SCHEDULER_IMPL_H_
#define WINLIB_WINBASE_TASK_SCHEDULER_TASK_SCHEDULER_IMPL_H_

#include <memory>
#include <vector>

#include "winbase\base_export.h"
#include "winbase\functional\callback.h"
#include "winbase\logging.h"
#include "winbase\macros.h"
#include "winbase\memory\ptr_util.h"
#include "winbase\memory\ref_counted.h"
#include "winbase\strings\string_piece.h"
#include "winbase\synchronization\atomic_flag.h"
#include "winbase\task_scheduler\delayed_task_manager.h"
#include "winbase\task_scheduler\environment_config.h"
#include "winbase\task_scheduler\scheduler_single_thread_task_runner_manager.h"
#include "winbase\task_scheduler\scheduler_worker_pool_impl.h"
#include "winbase\task_scheduler\single_thread_task_runner_thread_mode.h"
#include "winbase\task_scheduler\task_scheduler.h"
#include "winbase\task_scheduler\task_tracker.h"
#include "winbase\task_scheduler\task_traits.h"

#include "winbase\win\com_init_check_hook.h"

namespace winbase {

///class HistogramBase;
class Thread;

namespace internal {

// Default TaskScheduler implementation. This class is thread-safe.
class WINBASE_EXPORT TaskSchedulerImpl : public TaskScheduler {
 public:
  using TaskTrackerImpl = TaskTracker;

  // Creates a TaskSchedulerImpl with a production TaskTracker.
  //|histogram_label| is used to label histograms, it must not be empty.
  explicit TaskSchedulerImpl(StringPiece histogram_label);

  // For testing only. Creates a TaskSchedulerImpl with a custom TaskTracker.
  TaskSchedulerImpl(StringPiece histogram_label,
                    std::unique_ptr<TaskTrackerImpl> task_tracker);
  
  TaskSchedulerImpl(const TaskSchedulerImpl&) = delete;
  TaskSchedulerImpl& operator=(const TaskSchedulerImpl&) = delete;

  ~TaskSchedulerImpl() override;

  // TaskScheduler:
  void Start(const TaskScheduler::InitParams& init_params,
             SchedulerWorkerObserver* scheduler_worker_observer) override;
  void PostDelayedTaskWithTraits(const Location& from_here,
                                 const TaskTraits& traits,
                                 OnceClosure task,
                                 TimeDelta delay) override;
  scoped_refptr<TaskRunner> CreateTaskRunnerWithTraits(
      const TaskTraits& traits) override;
  scoped_refptr<SequencedTaskRunner> CreateSequencedTaskRunnerWithTraits(
      const TaskTraits& traits) override;
  scoped_refptr<SingleThreadTaskRunner> CreateSingleThreadTaskRunnerWithTraits(
      const TaskTraits& traits,
      SingleThreadTaskRunnerThreadMode thread_mode) override;
  scoped_refptr<SingleThreadTaskRunner> CreateCOMSTATaskRunnerWithTraits(
      const TaskTraits& traits,
      SingleThreadTaskRunnerThreadMode thread_mode) override;
  ///std::vector<const HistogramBase*> GetHistograms() const override;
  int GetMaxConcurrentNonBlockedTasksWithTraitsDeprecated(
      const TaskTraits& traits) const override;
  void Shutdown() override;
  void FlushForTesting() override;
  void FlushAsyncForTesting(OnceClosure flush_callback) override;
  void JoinForTesting() override;

 private:
  // Returns the worker pool that runs Tasks with |traits|.
  SchedulerWorkerPoolImpl* GetWorkerPoolForTraits(
      const TaskTraits& traits) const;

  // Returns |traits|, with priority set to TaskPriority::USER_BLOCKING if
  // |all_tasks_user_blocking_| is set.
  TaskTraits SetUserBlockingPriorityIfNeeded(const TaskTraits& traits) const;

  const std::unique_ptr<TaskTrackerImpl> task_tracker_;
  std::unique_ptr<Thread> service_thread_;
  DelayedTaskManager delayed_task_manager_;
  SchedulerSingleThreadTaskRunnerManager single_thread_task_runner_manager_;

  // Indicates that all tasks are handled as if they had been posted with
  // TaskPriority::USER_BLOCKING. Since this is set in Start(), it doesn't apply
  // to tasks posted before Start() or to tasks posted to TaskRunners created
  // before Start().
  //
  // TODO(fdoray): Remove after experiment. https://crbug.com/757022
  AtomicFlag all_tasks_user_blocking_;

  // Owns all the pools managed by this TaskScheduler.
  std::vector<std::unique_ptr<SchedulerWorkerPoolImpl>> worker_pools_;

  // Maps an environment from EnvironmentType to a pool in |worker_pools_|.
  SchedulerWorkerPoolImpl* environment_to_worker_pool_[static_cast<int>(
      EnvironmentType::ENVIRONMENT_COUNT)];

#if WINBASE_DCHECK_IS_ON()
  // Set once JoinForTesting() has returned.
  AtomicFlag join_for_testing_returned_;
#endif

#if defined(COM_INIT_CHECK_HOOK_ENABLED)
  // Provides COM initialization verification for supported builds.
  winbase::win::ComInitCheckHook com_init_check_hook_;
#endif
};

}  // namespace internal
}  // namespace winbase

#endif  // WINLIB_WINBASE_TASK_SCHEDULER_TASK_SCHEDULER_IMPL_H_