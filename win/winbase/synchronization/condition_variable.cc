// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "winbase\synchronization\condition_variable.h"

#include "winbase\synchronization\lock.h"
#include "winbase\threading\scoped_blocking_call.h"
#include "winbase\threading\thread_restrictions.h"
#include "winbase\time\time.h"

namespace winbase {

ConditionVariable::ConditionVariable(Lock* user_lock)
    : srwlock_(user_lock->lock_.native_handle())
#if WINBASE_DCHECK_IS_ON()
    , user_lock_(user_lock)
#endif
{
  WINBASE_DCHECK(user_lock);
  InitializeConditionVariable(reinterpret_cast<PCONDITION_VARIABLE>(&cv_));
}

ConditionVariable::~ConditionVariable() = default;

void ConditionVariable::Wait() {
  TimedWait(TimeDelta::FromMilliseconds(INFINITE));
}

void ConditionVariable::TimedWait(const TimeDelta& max_time) {
  internal::AssertBaseSyncPrimitivesAllowed();
  ScopedBlockingCall scoped_blocking_call(BlockingType::MAY_BLOCK);
  DWORD timeout = static_cast<DWORD>(max_time.InMilliseconds());

#if WINBASE_DCHECK_IS_ON()
  user_lock_->CheckHeldAndUnmark();
#endif

  if (!SleepConditionVariableSRW(reinterpret_cast<PCONDITION_VARIABLE>(&cv_),
                                 reinterpret_cast<PSRWLOCK>(srwlock_), timeout,
                                 0)) {
    // On failure, we only expect the CV to timeout. Any other error value means
    // that we've unexpectedly woken up.
    // Note that WAIT_TIMEOUT != ERROR_TIMEOUT. WAIT_TIMEOUT is used with the
    // WaitFor* family of functions as a direct return value. ERROR_TIMEOUT is
    // used with GetLastError().
    WINBASE_DCHECK_EQ(static_cast<DWORD>(ERROR_TIMEOUT), GetLastError());
  }

#if WINBASE_DCHECK_IS_ON()
  user_lock_->CheckUnheldAndMark();
#endif
}

void ConditionVariable::Broadcast() {
  WakeAllConditionVariable(reinterpret_cast<PCONDITION_VARIABLE>(&cv_));
}

void ConditionVariable::Signal() {
  WakeConditionVariable(reinterpret_cast<PCONDITION_VARIABLE>(&cv_));
}

}  // namespace winbase