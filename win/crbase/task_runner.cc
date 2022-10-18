// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/task_runner.h"

#include "crbase/compiler_specific.h"
#include "crbase/logging.h"
#include "crbase/threading/post_task_and_reply_impl.h"

namespace crbase {

namespace {

// TODO(akalin): There's only one other implementation of
// PostTaskAndReplyImpl in WorkerPool.  Investigate whether it'll be
// possible to merge the two.
class PostTaskAndReplyTaskRunner : public internal::PostTaskAndReplyImpl {
 public:
  explicit PostTaskAndReplyTaskRunner(TaskRunner* destination);

 private:
  bool PostTask(const crtracked_objects::Location& from_here,
                const Closure& task) override;

  // Non-owning.
  TaskRunner* destination_;
};

PostTaskAndReplyTaskRunner::PostTaskAndReplyTaskRunner(
    TaskRunner* destination) : destination_(destination) {
  CR_DCHECK(destination_);
}

bool PostTaskAndReplyTaskRunner::PostTask(
    const crtracked_objects::Location& from_here,
    const Closure& task) {
  return destination_->PostTask(from_here, task);
}

}  // namespace

bool TaskRunner::PostTask(const crtracked_objects::Location& from_here,
                          const Closure& task) {
  return PostDelayedTask(from_here, task, crbase::TimeDelta());
}

bool TaskRunner::PostTaskAndReply(
    const crtracked_objects::Location& from_here,
    const Closure& task,
    const Closure& reply) {
  return PostTaskAndReplyTaskRunner(this).PostTaskAndReply(
      from_here, task, reply);
}

TaskRunner::TaskRunner() {}

TaskRunner::~TaskRunner() {}

void TaskRunner::OnDestruct() const {
  delete this;
}

void TaskRunnerTraits::Destruct(const TaskRunner* task_runner) {
  task_runner->OnDestruct();
}

}  // namespace crbase