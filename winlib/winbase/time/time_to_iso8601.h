// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_TIME_TIME_TO_ISO8601_H_
#define WINLIB_WINBASE_TIME_TIME_TO_ISO8601_H_

#include <string>

#include "winbase\base_export.h"

namespace winbase {

class Time;

WINBASE_EXPORT std::string TimeToISO8601(const winbase::Time& t);

}  // namespace base

#endif  // WINLIB_WINBASE_TIME_TIME_TO_ISO8601_H_