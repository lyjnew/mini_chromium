// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_CRBASE_CRITICAL_CLOSURE_H_
#define MINI_CHROMIUM_CRBASE_CRITICAL_CLOSURE_H_

#include "crbase/callback.h"
#include "crbase/macros.h"
#include "crbuild/build_config.h"

namespace crbase {

// Returns a closure (which may return a result, but must not require any extra
// arguments) that will continue to run for a period of time when the
// application goes to the background if possible on platforms where
// applications don't execute while backgrounded, otherwise the original task is
// returned.
//
// Example:
//   file_task_runner_->PostTask(
//       CR_FROM_HERE,
//       MakeCriticalClosure(crbase::Bind(&WriteToDiskTask, path_, data)));
//
// Note new closures might be posted in this closure. If the new closures need
// background running time, |MakeCriticalClosure| should be applied on them
// before posting.

template <typename R>
inline Callback<R(void)> MakeCriticalClosure(const Callback<R(void)>& closure) {
  // No-op for platforms where the application does not need to acquire
  // background time for closures to finish when it goes into the background.
  return closure;
}

}  // namespace crbase

#endif  // MINI_CHROMIUM_CRBASE_CRITICAL_CLOSURE_H_