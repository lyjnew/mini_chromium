// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_THREADING_SEQUENCED_TASK_RUNNER_HANDLE_H_
#define WINBASE_THREADING_SEQUENCED_TASK_RUNNER_HANDLE_H_

#include "winbase\compiler_specific.h"
#include "winbase\macros.h"
#include "winbase\memory\ref_counted.h"
#include "winbase\sequenced_task_runner.h"

namespace winbase {

class WINBASE_EXPORT SequencedTaskRunnerHandle {
 public:
  // Returns a SequencedTaskRunner which guarantees that posted tasks will only
  // run after the current task is finished and will satisfy a SequenceChecker.
  // It should only be called if IsSet() returns true (see the comment there for
  // the requirements).
  static scoped_refptr<SequencedTaskRunner> Get();

  // Returns true if one of the following conditions is fulfilled:
  // a) A SequencedTaskRunner has been assigned to the current thread by
  //    instantiating a SequencedTaskRunnerHandle.
  // b) The current thread has a ThreadTaskRunnerHandle (which includes any
  //    thread that has a MessageLoop associated with it).
  static bool IsSet();

  // Binds |task_runner| to the current thread.
  explicit SequencedTaskRunnerHandle(
      scoped_refptr<SequencedTaskRunner> task_runner);
  SequencedTaskRunnerHandle(const SequencedTaskRunnerHandle&) = delete;
  SequencedTaskRunnerHandle& operator=(const SequencedTaskRunnerHandle&) 
      = delete;
  ~SequencedTaskRunnerHandle();
  
 private:
  scoped_refptr<SequencedTaskRunner> task_runner_;
};

}  // namespace winbase

#endif  // WINLIB_WINBASE_THREADING_SEQUENCED_TASK_RUNNER_HANDLE_H_