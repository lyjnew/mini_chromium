// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_STRINGS_STRINGPRINTF_H_
#define WINLIB_WINBASE_STRINGS_STRINGPRINTF_H_

#include <stdarg.h>   // va_list

#include <string>

#include "winbase\base_export.h"
#include "winbase\compiler_specific.h"
#include "winbase\strings\string16.h"

namespace winbase {

// Return a C++ string given printf-like input.
WINBASE_EXPORT std::string StringPrintf(_Printf_format_string_ const char* format,
                                        ...)
    PRINTF_FORMAT(1, 2) WARN_UNUSED_RESULT;

WINBASE_EXPORT string16 StringPrintf(
    _Printf_format_string_ const char16* format,
    ...) WPRINTF_FORMAT(1, 2) WARN_UNUSED_RESULT;

// Return a C++ string given vprintf-like input.
WINBASE_EXPORT std::string StringPrintV(const char* format, va_list ap)
    PRINTF_FORMAT(1, 0) WARN_UNUSED_RESULT;

// Store result into a supplied string and return it.
WINBASE_EXPORT const std::string& SStringPrintf(
    std::string* dst,
    _Printf_format_string_ const char* format,
    ...) PRINTF_FORMAT(2, 3);

WINBASE_EXPORT const string16& SStringPrintf(
    string16* dst,
    _Printf_format_string_ const char16* format,
    ...) WPRINTF_FORMAT(2, 3);

// Append result to a supplied string.
WINBASE_EXPORT void StringAppendF(std::string* dst,
                                  _Printf_format_string_ const char* format,
                                  ...) PRINTF_FORMAT(2, 3);

WINBASE_EXPORT void StringAppendF(string16* dst,
                                  _Printf_format_string_ const wchar_t* format,
                                  ...) WPRINTF_FORMAT(2, 3);

// Lower-level routine that takes a va_list and appends to a specified
// string.  All other routines are just convenience wrappers around it.
WINBASE_EXPORT void StringAppendV(std::string* dst, 
                                  const char* format, va_list ap)
    PRINTF_FORMAT(2, 0);

WINBASE_EXPORT void StringAppendV(string16* dst,
                                  const char16* format, va_list ap)
    WPRINTF_FORMAT(2, 0);

}  // namespace winbase

#endif  // WINLIB_WINBASE_STRINGS_STRINGPRINTF_H_