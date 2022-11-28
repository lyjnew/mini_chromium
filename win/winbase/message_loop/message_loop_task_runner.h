// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_MESSAGE_LOOP_MESSAGE_LOOP_TASK_RUNNER_H_
#define WINLIB_WINBASE_MESSAGE_LOOP_MESSAGE_LOOP_TASK_RUNNER_H_

#include "winbase\base_export.h"
#include "winbase\functional\callback.h"
#include "winbase\macros.h"
#include "winbase\memory\ref_counted.h"
#include "winbase\pending_task.h"
#include "winbase\single_thread_task_runner.h"
#include "winbase\synchronization\lock.h"
#include "winbase\threading\platform_thread.h"

namespace winbase {
namespace internal {

class IncomingTaskQueue;

// A stock implementation of SingleThreadTaskRunner that is created and managed
// by a MessageLoop. For now a MessageLoopTaskRunner can only be created as
// part of a MessageLoop.
class WINBASE_EXPORT MessageLoopTaskRunner : public SingleThreadTaskRunner {
 public:
  explicit MessageLoopTaskRunner(
      scoped_refptr<IncomingTaskQueue> incoming_queue);
  MessageLoopTaskRunner(const MessageLoopTaskRunner&) = delete;
  MessageLoopTaskRunner& operator=(const MessageLoopTaskRunner&) = delete;
  // Initialize this message loop task runner on the current thread.
  void BindToCurrentThread();

  // SingleThreadTaskRunner implementation
  bool PostDelayedTask(const Location& from_here,
                       OnceClosure task,
                       TimeDelta delay) override;
  bool PostNonNestableDelayedTask(const Location& from_here,
                                  OnceClosure task,
                                  TimeDelta delay) override;
  bool RunsTasksInCurrentSequence() const override;

 private:
  friend class RefCountedThreadSafe<MessageLoopTaskRunner>;
  ~MessageLoopTaskRunner() override;

  // The incoming queue receiving all posted tasks.
  scoped_refptr<IncomingTaskQueue> incoming_queue_;

  // ID of the thread |this| was created on.  Could be accessed on multiple
  // threads, protected by |valid_thread_id_lock_|.
  PlatformThreadId valid_thread_id_;
  mutable Lock valid_thread_id_lock_;
};

}  // namespace internal
}  // namespace winbase

#endif  // WINLIB_WINBASE_MESSAGE_LOOP_MESSAGE_LOOP_TASK_RUNNER_H_