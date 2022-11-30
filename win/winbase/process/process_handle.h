// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_PROCESS_PROCESS_HANDLE_H_
#define WINLIB_WINBASE_PROCESS_PROCESS_HANDLE_H_

#include <stdint.h>
#include <sys/types.h>

#include "winbase\base_export.h"
#include "winbase\files\file_path.h"
#include "winbase\win\windows_types.h"

namespace winbase {

// ProcessHandle is a platform specific type which represents the underlying OS
// handle to a process.
// ProcessId is a number which identifies the process in the OS.
typedef HANDLE ProcessHandle;
typedef DWORD ProcessId;
typedef HANDLE UserTokenHandle;
const ProcessHandle kNullProcessHandle = NULL;
const ProcessId kNullProcessId = 0;

// To print ProcessIds portably use WinBasePRIdPid (based on PRIuS and friends 
// from C99 and format_macros.h) like this:
// winbase::StringPrintf("PID is %" WinBasePRIdPid ".\n", pid);
#define WinBasePRIdPid "ld"

// Returns the id of the current process.
// Note that on some platforms, this is not guaranteed to be unique across
// processes (use GetUniqueIdForProcess if uniqueness is required).
WINBASE_EXPORT ProcessId GetCurrentProcId();

// Returns a unique ID for the current process. The ID will be unique across all
// currently running processes within the chrome session, but IDs of terminated
// processes may be reused. This returns an opaque value that is different from
// a process's PID.
WINBASE_EXPORT uint32_t GetUniqueIdForProcess();

// Returns the ProcessHandle of the current process.
WINBASE_EXPORT ProcessHandle GetCurrentProcessHandle();

// Returns the process ID for the specified process. This is functionally the
// same as Windows' GetProcessId(), but works on versions of Windows before Win
// XP SP1 as well.
// DEPRECATED. New code should be using Process::Pid() instead.
// Note that on some platforms, this is not guaranteed to be unique across
// processes.
WINBASE_EXPORT ProcessId GetProcId(ProcessHandle process);

// Returns the ID for the parent of the given process. Not available on Fuchsia.
// Returning a negative value indicates an error, such as if the |process| does
// not exist. Returns 0 when |process| has no parent process.
WINBASE_EXPORT ProcessId GetParentProcessId(ProcessHandle process);

}  // namespace winbase

#endif  // WINLIB_WINBASE_PROCESS_PROCESS_HANDLE_H_