// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "winbase\task_runner.h"

#include <utility>

#include "winbase\compiler_specific.h"
#include "winbase\logging.h"
#include "winbase\threading\post_task_and_reply_impl.h"
          
namespace winbase {

namespace {

// TODO(akalin): There's only one other implementation of
// PostTaskAndReplyImpl in WorkerPool.  Investigate whether it'll be
// possible to merge the two.
class PostTaskAndReplyTaskRunner : public internal::PostTaskAndReplyImpl {
 public:
  explicit PostTaskAndReplyTaskRunner(TaskRunner* destination);

 private:
  bool PostTask(const Location& from_here, OnceClosure task) override;

  // Non-owning.
  TaskRunner* destination_;
};

PostTaskAndReplyTaskRunner::PostTaskAndReplyTaskRunner(
    TaskRunner* destination) : destination_(destination) {
  WINBASE_DCHECK(destination_);
}

bool PostTaskAndReplyTaskRunner::PostTask(const Location& from_here,
                                          OnceClosure task) {
  return destination_->PostTask(from_here, std::move(task));
}

}  // namespace

bool TaskRunner::PostTask(const Location& from_here, OnceClosure task) {
  return PostDelayedTask(from_here, std::move(task), winbase::TimeDelta());
}

bool TaskRunner::PostTaskAndReply(const Location& from_here,
                                  OnceClosure task,
                                  OnceClosure reply) {
  return PostTaskAndReplyTaskRunner(this).PostTaskAndReply(
      from_here, std::move(task), std::move(reply));
}

TaskRunner::TaskRunner() = default;

TaskRunner::~TaskRunner() = default;

void TaskRunner::OnDestruct() const {
  delete this;
}

void TaskRunnerTraits::Destruct(const TaskRunner* task_runner) {
  task_runner->OnDestruct();
}

}  // namespace winbase