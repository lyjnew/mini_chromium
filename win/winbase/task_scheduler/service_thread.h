// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_TASK_SCHEDULER_SERVICE_THREAD_H_
#define WINLIB_WINBASE_TASK_SCHEDULER_SERVICE_THREAD_H_

#include "winbase\base_export.h"
#include "winbase\macros.h"
#include "winbase\threading\thread.h"
#include "winbase\time\time.h"
#include "winbase\timer\timer.h"

namespace winbase {
namespace internal {

class TaskTracker;

// The TaskScheduler's ServiceThread is a mostly idle thread that is responsible
// for handling async events (e.g. delayed tasks and async I/O). Its role is to
// merely forward such events to their destination (hence staying mostly idle
// and highly responsive).
// It aliases Thread::Run() to enforce that ServiceThread::Run() be on the stack
// and make it easier to identify the service thread in stack traces.
class WINBASE_EXPORT ServiceThread : public Thread {
 public:
  // Constructs a ServiceThread which will report latency metrics through
  // |task_tracker| if non-null. In that case, this ServiceThread will assume a
  // registered TaskScheduler instance and that |task_tracker| will outlive this
  // ServiceThread.
  explicit ServiceThread(const TaskTracker* task_tracker);
  ServiceThread(const ServiceThread&) = delete;
  ServiceThread& operator=(const ServiceThread&) = delete;

  // Overrides the default interval at which |heartbeat_latency_timer_| fires.
  // Call this with a |heartbeat| of zero to undo the override.
  // Must not be called while the ServiceThread is running.
  static void SetHeartbeatIntervalForTesting(TimeDelta heartbeat);

 private:
  // Thread:
  void Init() override;
  void Run(RunLoop* run_loop) override;

  // Kicks off a single async task which will record a histogram on the latency
  // of a randomly chosen set of TaskTraits.
  ///void PerformHeartbeatLatencyReport() const;

  const TaskTracker* const task_tracker_;

  // Fires a recurring heartbeat task to record latency histograms which are
  // independent from any execution sequence. This is done on the service thread
  // to avoid all external dependencies (even main thread).
  ///winbase::RepeatingTimer heartbeat_latency_timer_;
};

}  // namespace internal
}  // namespace winbase

#endif  // WINLIB_WINBASE_TASK_SCHEDULER_SERVICE_THREAD_H_