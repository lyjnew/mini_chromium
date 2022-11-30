// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_TASK_SCHEDULER_SCHEDULER_LOCK_H
#define WINLIB_WINBASE_TASK_SCHEDULER_SCHEDULER_LOCK_H

#include <memory>

#include "winbase\base_export.h"
#include "winbase\macros.h"
#include "winbase\synchronization\condition_variable.h"
#include "winbase\synchronization\lock.h"
#include "winbase\task_scheduler\scheduler_lock_impl.h"

namespace winbase {
namespace internal {

// SchedulerLock should be used anywhere a lock would be used in the scheduler.
// When DCHECK_IS_ON(), lock checking occurs. Otherwise, SchedulerLock is
// equivalent to base::Lock.
//
// The shape of SchedulerLock is as follows:
// SchedulerLock()
//     Default constructor, no predecessor lock.
//     DCHECKs
//         On Acquisition if any scheduler lock is acquired on this thread.
//
// SchedulerLock(const SchedulerLock* predecessor)
//     Constructor that specifies an allowed predecessor for that lock.
//     DCHECKs
//         On Construction if |predecessor| forms a predecessor lock cycle.
//         On Acquisition if the previous lock acquired on the thread is not
//             |predecessor|. Okay if there was no previous lock acquired.
//
// void Acquire()
//     Acquires the lock.
//
// void Release()
//     Releases the lock.
//
// void AssertAcquired().
//     DCHECKs if the lock is not acquired.
//
// std::unique_ptr<ConditionVariable> CreateConditionVariable()
//     Creates a condition variable using this as a lock.

#if WINBASE_DCHECK_IS_ON()
class SchedulerLock : public SchedulerLockImpl {
 public:
  SchedulerLock() = default;
  explicit SchedulerLock(const SchedulerLock* predecessor)
      : SchedulerLockImpl(predecessor) {}
};
#else   // WINBASE_DCHECK_IS_ON()
class SchedulerLock : public Lock {
 public:
  SchedulerLock() = default;
  explicit SchedulerLock(const SchedulerLock*) {}

  std::unique_ptr<ConditionVariable> CreateConditionVariable() {
    return std::unique_ptr<ConditionVariable>(new ConditionVariable(this));
  }
};
#endif  // WINBASE_DCHECK_IS_ON()

// Provides the same functionality as base::AutoLock for SchedulerLock.
class AutoSchedulerLock {
 public:
  explicit AutoSchedulerLock(SchedulerLock& lock) : lock_(lock) {
    lock_.Acquire();
  }
  AutoSchedulerLock(const AutoSchedulerLock&) = delete;
  AutoSchedulerLock& operator=(const AutoSchedulerLock&) = delete;

  ~AutoSchedulerLock() {
    lock_.AssertAcquired();
    lock_.Release();
  }

 private:
  SchedulerLock& lock_;
};

}  // namespace internal
}  // namespace winbase

#endif  // WINLIB_WINBASE_TASK_SCHEDULER_SCHEDULER_LOCK_H