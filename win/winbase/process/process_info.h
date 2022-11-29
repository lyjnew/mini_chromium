// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_PROCESS_PROCESS_INFO_H_
#define WINLIB_WINBASE_PROCESS_PROCESS_INFO_H_

#include "winbase\base_export.h"

namespace winbase {

class Time;

// Vends information about the current process.
class WINBASE_EXPORT CurrentProcessInfo {
 public:
  // Returns the time at which the process was launched. May be empty if an
  // error occurred retrieving the information.
  static const Time CreationTime();
};

enum IntegrityLevel {
  INTEGRITY_UNKNOWN,
  UNTRUSTED_INTEGRITY,
  LOW_INTEGRITY,
  MEDIUM_INTEGRITY,
  HIGH_INTEGRITY,
};

// Returns the integrity level of the process. Returns INTEGRITY_UNKNOWN in the
// case of an underlying system failure.
WINBASE_EXPORT IntegrityLevel GetCurrentProcessIntegrityLevel();

// Determines whether the current process is elevated.
WINBASE_EXPORT bool IsCurrentProcessElevated();

}  // namespace base

#endif  // WINLIB_WINBASE_PROCESS_PROCESS_INFO_H_