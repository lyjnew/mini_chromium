// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "winbase\threading\sequenced_task_runner_handle.h"

#include <utility>

#include "winbase\lazy_instance.h"
#include "winbase\logging.h"
#include "winbase\threading\thread_local.h"
#include "winbase\threading\thread_task_runner_handle.h"

namespace winbase {

namespace {

LazyInstance<ThreadLocalPointer<SequencedTaskRunnerHandle>>::Leaky
    sequenced_task_runner_tls = WINBASE_LAZY_INSTANCE_INITIALIZER;

}  // namespace

// static
scoped_refptr<SequencedTaskRunner> SequencedTaskRunnerHandle::Get() {
  // Return the registered SequencedTaskRunner, if any.
  const SequencedTaskRunnerHandle* handle =
      sequenced_task_runner_tls.Pointer()->Get();
  if (handle)
    return handle->task_runner_;

  // Note if you hit this: the problem is the lack of a sequenced context. The
  // ThreadTaskRunnerHandle is just the last attempt at finding such a context.
  WINBASE_CHECK(ThreadTaskRunnerHandle::IsSet())
      << "Error: This caller requires a sequenced context (i.e. the "
         "current task needs to run from a SequencedTaskRunner).";
  return ThreadTaskRunnerHandle::Get();
}

// static
bool SequencedTaskRunnerHandle::IsSet() {
  return sequenced_task_runner_tls.Pointer()->Get() ||
         ThreadTaskRunnerHandle::IsSet();
}

SequencedTaskRunnerHandle::SequencedTaskRunnerHandle(
    scoped_refptr<SequencedTaskRunner> task_runner)
    : task_runner_(std::move(task_runner)) {
  WINBASE_DCHECK(task_runner_->RunsTasksInCurrentSequence());
  WINBASE_DCHECK(!SequencedTaskRunnerHandle::IsSet());
  sequenced_task_runner_tls.Pointer()->Set(this);
}

SequencedTaskRunnerHandle::~SequencedTaskRunnerHandle() {
  WINBASE_DCHECK(task_runner_->RunsTasksInCurrentSequence());
  WINBASE_DCHECK_EQ(sequenced_task_runner_tls.Pointer()->Get(), this);
  sequenced_task_runner_tls.Pointer()->Set(nullptr);
}

}  // namespace winbase