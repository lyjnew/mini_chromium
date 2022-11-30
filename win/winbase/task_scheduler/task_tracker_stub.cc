// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "winbase\task_scheduler\task_tracker.h"

namespace winbase {
namespace internal {

class TaskTracker::State {
 public:
  State() = default;
  State(const State&) = delete;
  State& operator=(const State&) = delete;

  // Sets a flag indicating that shutdown has started. Returns true if there are
  // tasks blocking shutdown. Can only be called once.
  bool StartShutdown() {
    const auto new_value =
        subtle::NoBarrier_AtomicIncrement(&bits_, kShutdownHasStartedMask);

    // Check that the "shutdown has started" bit isn't zero. This would happen
    // if it was incremented twice.
    WINBASE_DCHECK(new_value & kShutdownHasStartedMask);

    const auto num_tasks_blocking_shutdown =
        new_value >> kNumTasksBlockingShutdownBitOffset;
    return num_tasks_blocking_shutdown != 0;
  }

  // Returns true if shutdown has started.
  bool HasShutdownStarted() const {
    return subtle::NoBarrier_Load(&bits_) & kShutdownHasStartedMask;
  }

  // Returns true if there are tasks blocking shutdown.
  bool AreTasksBlockingShutdown() const {
    const auto num_tasks_blocking_shutdown =
        subtle::NoBarrier_Load(&bits_) >> kNumTasksBlockingShutdownBitOffset;
    WINBASE_DCHECK_GE(num_tasks_blocking_shutdown, 0);
    return num_tasks_blocking_shutdown != 0;
  }

  // Increments the number of tasks blocking shutdown. Returns true if shutdown
  // has started.
  bool IncrementNumTasksBlockingShutdown() {
#if WINBASE_DCHECK_IS_ON()
    // Verify that no overflow will occur.
    const auto num_tasks_blocking_shutdown =
        subtle::NoBarrier_Load(&bits_) >> kNumTasksBlockingShutdownBitOffset;
    WINBASE_DCHECK_LT(num_tasks_blocking_shutdown,
                      std::numeric_limits<subtle::Atomic32>::max() -
                          kNumTasksBlockingShutdownIncrement);
#endif

    const auto new_bits = subtle::NoBarrier_AtomicIncrement(
        &bits_, kNumTasksBlockingShutdownIncrement);
    return new_bits & kShutdownHasStartedMask;
  }

  // Decrements the number of tasks blocking shutdown. Returns true if shutdown
  // has started and the number of tasks blocking shutdown becomes zero.
  bool DecrementNumTasksBlockingShutdown() {
    const auto new_bits = subtle::NoBarrier_AtomicIncrement(
        &bits_, -kNumTasksBlockingShutdownIncrement);
    const bool shutdown_has_started = new_bits & kShutdownHasStartedMask;
    const auto num_tasks_blocking_shutdown =
        new_bits >> kNumTasksBlockingShutdownBitOffset;
    WINBASE_DCHECK_GE(num_tasks_blocking_shutdown, 0);
    return shutdown_has_started && num_tasks_blocking_shutdown == 0;
  }

 private:
  static constexpr subtle::Atomic32 kShutdownHasStartedMask = 1;
  static constexpr subtle::Atomic32 kNumTasksBlockingShutdownBitOffset = 1;
  static constexpr subtle::Atomic32 kNumTasksBlockingShutdownIncrement =
      1 << kNumTasksBlockingShutdownBitOffset;

  // The LSB indicates whether shutdown has started. The other bits count the
  // number of tasks blocking shutdown.
  // No barriers are required to read/write |bits_| as this class is only used
  // as an atomic state checker, it doesn't provide sequential consistency
  // guarantees w.r.t. external state. Sequencing of the TaskTracker::State
  // operations themselves is guaranteed by the AtomicIncrement RMW (read-
  // modify-write) semantics however. For example, if two threads are racing to
  // call IncrementNumTasksBlockingShutdown() and StartShutdown() respectively,
  // either the first thread will win and the StartShutdown() call will see the
  // blocking task or the second thread will win and
  // IncrementNumTasksBlockingShutdown() will know that shutdown has started.
  subtle::Atomic32 bits_ = 0;
};
  
struct TaskTracker::PreemptedBackgroundSequence {
  PreemptedBackgroundSequence() = default;
  PreemptedBackgroundSequence(scoped_refptr<Sequence> sequence_in,
                              TimeTicks next_task_sequenced_time_in,
                              CanScheduleSequenceObserver* observer_in)
      : sequence(std::move(sequence_in)),
        next_task_sequenced_time(next_task_sequenced_time_in),
        observer(observer_in) {}
  PreemptedBackgroundSequence(PreemptedBackgroundSequence&& other) = default;
  PreemptedBackgroundSequence(const PreemptedBackgroundSequence&) = delete;
  PreemptedBackgroundSequence& operator=(const PreemptedBackgroundSequence&) 
      = delete;
  ~PreemptedBackgroundSequence() = default;
  PreemptedBackgroundSequence& operator=(PreemptedBackgroundSequence&& other) =
      default;
  bool operator<(const PreemptedBackgroundSequence& other) const {
    return next_task_sequenced_time < other.next_task_sequenced_time;
  }
  bool operator>(const PreemptedBackgroundSequence& other) const {
    return next_task_sequenced_time > other.next_task_sequenced_time;
  }

  // A background sequence waiting to be scheduled.
  scoped_refptr<Sequence> sequence;

  // The sequenced time of the next task in |sequence|.
  TimeTicks next_task_sequenced_time;

  // An observer to notify when |sequence| can be scheduled.
  CanScheduleSequenceObserver* observer = nullptr;
};

TaskTracker::TaskTracker(StringPiece histogram_label) 
    : TaskTracker(histogram_label, 1) {}

TaskTracker::TaskTracker(
    StringPiece histogram_label,
    int max_num_scheduled_background_sequences) 
        : flush_cv_(flush_lock_.CreateConditionVariable()),
          shutdown_lock_(&flush_lock_),
          max_num_scheduled_background_sequences_(
              max_num_scheduled_background_sequences),
          tracked_ref_factory_(this){

}

TaskTracker::~TaskTracker() {

}

void TaskTracker::Shutdown() {

}

void TaskTracker::FlushForTesting() {

}

void TaskTracker::FlushAsyncForTesting(OnceClosure flush_callback) {

}

bool TaskTracker::WillPostTask(Task* task) {
  return true;
}

scoped_refptr<Sequence> TaskTracker::WillScheduleSequence(
    scoped_refptr<Sequence> sequence,
    CanScheduleSequenceObserver* observer) {
  return nullptr;
}

scoped_refptr<Sequence> TaskTracker::RunAndPopNextTask(
    scoped_refptr<Sequence> sequence,
    CanScheduleSequenceObserver* observer) {
  return nullptr;
}

bool TaskTracker::HasShutdownStarted() const {
  return true;
}

bool TaskTracker::IsShutdownComplete() const {
  return true;
}

void TaskTracker::SetHasShutdownStartedForTesting() {}

void TaskTracker::RecordLatencyHistogram(
    LatencyHistogramType latency_histogram_type,
    TaskTraits task_traits,
    TimeTicks posted_time) const {}

void TaskTracker::RunOrSkipTask(Task task,
                                Sequence* sequence,
                                bool can_run_task) {
}

#if WINBASE_DCHECK_IS_ON()
bool TaskTracker::IsPostingBlockShutdownTaskAfterShutdownAllowed() {

}
#endif

void TaskTracker::PerformShutdown() {}

void TaskTracker::SetMaxNumScheduledBackgroundSequences(
    int max_num_scheduled_background_sequences) {}

TaskTracker::PreemptedBackgroundSequence
TaskTracker::GetPreemptedBackgroundSequenceToScheduleLockRequired() {
  return TaskTracker::PreemptedBackgroundSequence();
}

void TaskTracker::SchedulePreemptedBackgroundSequence(
    PreemptedBackgroundSequence sequence_to_schedule) {

}

bool TaskTracker::BeforePostTask(TaskShutdownBehavior shutdown_behavior) {
  return true;
}

bool TaskTracker::BeforeRunTask(TaskShutdownBehavior shutdown_behavior) {
  return true;
}

void TaskTracker::AfterRunTask(TaskShutdownBehavior shutdown_behavior) {}

void TaskTracker::OnBlockingShutdownTasksComplete() {}

void TaskTracker::DecrementNumIncompleteUndelayedTasks() {}

scoped_refptr<Sequence> TaskTracker::ManageBackgroundSequencesAfterRunningTask(
    scoped_refptr<Sequence> just_ran_sequence,
    CanScheduleSequenceObserver* observer) {
  return nullptr;
}

void TaskTracker::CallFlushCallbackForTesting() {}

}  // namespace internal
}  // namespace winbase