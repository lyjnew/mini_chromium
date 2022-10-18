// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/build_time.h"

#include "crbase/logging.h"
#include "crbase/time/time.h"

namespace crbase {

Time GetBuildTime() {
  Time integral_build_time;
  // The format of __DATE__ and __TIME__ is specified by the ANSI C Standard,
  // section 6.8.8.
  //
  // __DATE__ is exactly "Mmm DD YYYY".
  // __TIME__ is exactly "hh:mm:ss".
  const char kDateTime[] = __DATE__ " " __TIME__ " PST";

  bool result = Time::FromString(kDateTime, &integral_build_time);
  CR_DCHECK(result);
  return integral_build_time;
}

}  // namespace crbase