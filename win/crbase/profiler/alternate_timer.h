// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This is a glue file, which allows third party code to call into our profiler
// without having to include most any functions from base.

#ifndef MINI_CHROMIUM_CRBASE_PROFILER_ALTERNATE_TIMER_H_
#define MINI_CHROMIUM_CRBASE_PROFILER_ALTERNATE_TIMER_H_

#include "crbase/crbase_export.h"

namespace crtracked_objects {

enum TimeSourceType {
  TIME_SOURCE_TYPE_WALL_TIME,
  TIME_SOURCE_TYPE_TCMALLOC
};

// Provide type for an alternate timer function.
typedef unsigned int NowFunction();

// Environment variable name that is used to activate alternate timer profiling
// (such as using TCMalloc allocations to provide a pseudo-timer) for tasks
// instead of wall clock profiling.
CRBASE_EXPORT extern const char kAlternateProfilerTime[];

// Set an alternate timer function to replace the OS time function when
// profiling.  Typically this is called by an allocator that is providing a
// function that indicates how much memory has been allocated on any given
// thread.
CRBASE_EXPORT void SetAlternateTimeSource(NowFunction* now_function,
                                          TimeSourceType type);

// Gets the pointer to a function that was set via SetAlternateTimeSource().
// Returns NULL if no set was done prior to calling GetAlternateTimeSource.
NowFunction* GetAlternateTimeSource();

// Returns the type of the currently set time source.
CRBASE_EXPORT TimeSourceType GetTimeSourceType();

}  // namespace crtracked_objects

#endif  // MINI_CHROMIUM_CRBASE_PROFILER_ALTERNATE_TIMER_H_