// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_PROCESS_MEMORY_H_
#define WINLIB_WINBASE_PROCESS_MEMORY_H_

#include <stddef.h>

#include "winbase\base_export.h"
#include "winbase\process\process_handle.h"

#define WINBASE_PVALLOC_AVAILABLE 0

namespace winbase {

// Enables 'terminate on heap corruption' flag. Helps protect against heap
// overflow. Has no effect if the OS doesn't provide the necessary facility.
WINBASE_EXPORT void EnableTerminationOnHeapCorruption();

// Turns on process termination if memory runs out.
WINBASE_EXPORT void EnableTerminationOnOutOfMemory();

// Terminates process. Should be called only for out of memory errors.
// Crash reporting classifies such crashes as OOM.
WINBASE_EXPORT void TerminateBecauseOutOfMemory(size_t size);


namespace win {

// Custom Windows exception code chosen to indicate an out of memory error.
// See https://msdn.microsoft.com/en-us/library/het71c37.aspx.
// "To make sure that you do not define a code that conflicts with an existing
// exception code" ... "The resulting error code should therefore have the
// highest four bits set to hexadecimal E."
// 0xe0000008 was chosen arbitrarily, as 0x00000008 is ERROR_NOT_ENOUGH_MEMORY.
const DWORD kOomExceptionCode = 0xe0000008;

}  // namespace win

// Special allocator functions for callers that want to check for OOM.
// These will not abort if the allocation fails even if
// EnableTerminationOnOutOfMemory has been called.
// This can be useful for huge and/or unpredictable size memory allocations.
// Please only use this if you really handle the case when the allocation
// fails. Doing otherwise would risk security.
// These functions may still crash on OOM when running under memory tools,
// specifically ASan and other sanitizers.
// Return value tells whether the allocation succeeded. If it fails |result| is
// set to NULL, otherwise it holds the memory address.
WINBASE_EXPORT WARN_UNUSED_RESULT bool UncheckedMalloc(size_t size,
                                                       void** result);
WINBASE_EXPORT WARN_UNUSED_RESULT bool UncheckedCalloc(size_t num_items,
                                                       size_t size,
                                                       void** result);

}  // namespace base

#endif  // WINLIB_WINBASE_PROCESS_MEMORY_H_