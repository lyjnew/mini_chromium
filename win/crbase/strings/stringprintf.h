// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_CRBASE_STRINGS_STRINGPRINTF_H_
#define MINI_CHROMIUM_CRBASE_STRINGS_STRINGPRINTF_H_

#include <stdarg.h>   // va_list

#include <string>

#include "crbase/base_export.h"
#include "crbase/compiler_specific.h"
#include "crbuild/build_config.h"

namespace crbase {

// Return a C++ string given printf-like input.
CRBASE_EXPORT std::string StringPrintf(_Printf_format_string_ const char* format,
                                     ...)
    CR_PRINTF_FORMAT(1, 2) CR_WARN_UNUSED_RESULT;
CRBASE_EXPORT std::wstring StringPrintf(
    _Printf_format_string_ const wchar_t* format,
    ...) CR_WPRINTF_FORMAT(1, 2) CR_WARN_UNUSED_RESULT;

// Return a C++ string given vprintf-like input.
CRBASE_EXPORT std::string StringPrintV(const char* format, va_list ap)
    CR_PRINTF_FORMAT(1, 0) CR_WARN_UNUSED_RESULT;

// Store result into a supplied string and return it.
CRBASE_EXPORT const std::string& SStringPrintf(
    std::string* dst,
    _Printf_format_string_ const char* format,
    ...) CR_PRINTF_FORMAT(2, 3);

CRBASE_EXPORT const std::wstring& SStringPrintf(
    std::wstring* dst,
    _Printf_format_string_ const wchar_t* format,
    ...) CR_WPRINTF_FORMAT(2, 3);

// Append result to a supplied string.
CRBASE_EXPORT void StringAppendF(std::string* dst,
                                 _Printf_format_string_ const char* format,
                                 ...) CR_PRINTF_FORMAT(2, 3);

CRBASE_EXPORT void StringAppendF(std::wstring* dst,
                                 _Printf_format_string_ const wchar_t* format,
                                 ...) CR_WPRINTF_FORMAT(2, 3);

// Lower-level routine that takes a va_list and appends to a specified
// string.  All other routines are just convenience wrappers around it.
CRBASE_EXPORT void StringAppendV(std::string* dst, const char* format, va_list ap)
    CR_PRINTF_FORMAT(2, 0);

CRBASE_EXPORT void StringAppendV(std::wstring* dst,
                                 const wchar_t* format, va_list ap)
    CR_WPRINTF_FORMAT(2, 0);

}  // namespace crbase

#endif  // MINI_CHROMIUM_CRBASE_STRINGS_STRINGPRINTF_H_