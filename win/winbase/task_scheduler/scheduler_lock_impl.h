// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_TASK_SCHEDULER_SCHEDULER_LOCK_IMPL_H
#define WINLIB_WINBASE_TASK_SCHEDULER_SCHEDULER_LOCK_IMPL_H

#include <memory>

#include "winbase\base_export.h"
#include "winbase\macros.h"
#include "winbase\synchronization\lock.h"

namespace winbase {

class ConditionVariable;

namespace internal {

// A regular lock with simple deadlock correctness checking.
// This lock tracks all of the available locks to make sure that any locks are
// acquired in an expected order.
// See scheduler_lock.h for details.
class WINBASE_EXPORT SchedulerLockImpl {
 public:
  SchedulerLockImpl();
  SchedulerLockImpl(const SchedulerLockImpl&) = delete;
  SchedulerLockImpl& operator=(const SchedulerLockImpl&) = delete;
  explicit SchedulerLockImpl(const SchedulerLockImpl* predecessor);
  ~SchedulerLockImpl();

  void Acquire();
  void Release();

  void AssertAcquired() const;

  std::unique_ptr<ConditionVariable> CreateConditionVariable();

 private:
  Lock lock_;
};

}  // namespace internal
}  // namespace winbase

#endif  // WINLIB_WINBASE_TASK_SCHEDULER_SCHEDULER_LOCK_IMPL_H