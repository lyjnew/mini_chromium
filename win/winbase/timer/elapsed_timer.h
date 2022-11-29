// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_TIMER_ELAPSED_TIMER_H_
#define WINLIB_WINBASE_TIMER_ELAPSED_TIMER_H_

#include "winbase\base_export.h"
#include "winbase\macros.h"
#include "winbase\time\time.h"

namespace winbase {

// A simple wrapper around TimeTicks::Now().
class WINBASE_EXPORT ElapsedTimer {
 public:
  ElapsedTimer();
  ElapsedTimer(ElapsedTimer&& other);
  ElapsedTimer(const ElapsedTimer&) = delete;
  ElapsedTimer& operator=(const ElapsedTimer&) = delete;

  void operator=(ElapsedTimer&& other);

  // Returns the time elapsed since object construction.
  TimeDelta Elapsed() const;

 private:
  TimeTicks begin_;
};

}  // namespace winbase

#endif  // WINLIB_WINBASE_TIMER_ELAPSED_TIMER_H_