// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_CRBASE_BUILD_TIME_H_
#define MINI_CHROMIUM_CRBASE_BUILD_TIME_H_

#include "crbase/base_export.h"
#include "crbase/time/time.h"

namespace crbase {

// GetBuildTime returns the time at which the current binary was built.
//
// This uses the __DATE__ and __TIME__ macros, which don't trigger a rebuild
// when they change. However, official builds will always be rebuilt from
// scratch.
//
// Also, since __TIME__ doesn't include a timezone, this value should only be
// considered accurate to a day.
//
Time CRBASE_EXPORT GetBuildTime();

}  // namespace crbase

#endif  // MINI_CHROMIUM_CRBASE_BUILD_TIME_H_