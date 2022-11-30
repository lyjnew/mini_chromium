// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "winbase\task_scheduler\scheduler_worker_stack.h"

#include <algorithm>

#include "winbase\logging.h"
#include "winbase\stl_util.h"
#include "winbase\task_scheduler\scheduler_worker.h"

namespace winbase {
namespace internal {

SchedulerWorkerStack::SchedulerWorkerStack() = default;

SchedulerWorkerStack::~SchedulerWorkerStack() = default;

void SchedulerWorkerStack::Push(SchedulerWorker* worker) {
  WINBASE_DCHECK(!Contains(worker)) << "SchedulerWorker already on stack";
  if (!IsEmpty())
    stack_.back()->BeginUnusedPeriod();
  stack_.push_back(worker);
}

SchedulerWorker* SchedulerWorkerStack::Pop() {
  if (IsEmpty())
    return nullptr;
  SchedulerWorker* const worker = stack_.back();
  stack_.pop_back();
  if (!IsEmpty())
    stack_.back()->EndUnusedPeriod();
  return worker;
}

SchedulerWorker* SchedulerWorkerStack::Peek() const {
  if (IsEmpty())
    return nullptr;
  return stack_.back();
}

bool SchedulerWorkerStack::Contains(const SchedulerWorker* worker) const {
  return ContainsValue(stack_, worker);
}

void SchedulerWorkerStack::Remove(const SchedulerWorker* worker) {
  WINBASE_DCHECK(!IsEmpty());
  WINBASE_DCHECK_NE(worker, stack_.back());
  auto it = std::find(stack_.begin(), stack_.end(), worker);
  WINBASE_DCHECK(it != stack_.end());
  WINBASE_DCHECK_NE(TimeTicks(), (*it)->GetLastUsedTime());
  stack_.erase(it);
}

}  // namespace internal
}  // namespace winbase