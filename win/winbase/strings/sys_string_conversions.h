// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_STRINGS_SYS_STRING_CONVERSIONS_H_
#define WINLIB_WINBASE_STRINGS_SYS_STRING_CONVERSIONS_H_

// Provides system-dependent string type conversions for cases where it's
// necessary to not use ICU. Generally, you should not need this in Chrome,
// but it is used in some shared code. Dependencies should be minimal.

#include <stdint.h>

#include <string>

#include "winbase\base_export.h"
#include "winbase\strings\string16.h"
#include "winbase\strings\string_piece.h"

namespace winbase {

// Converts between wide and UTF-8 representations of a string. On error, the
// result is system-dependent.
WINBASE_EXPORT std::string SysWideToUTF8(const std::wstring& wide);
WINBASE_EXPORT std::wstring SysUTF8ToWide(StringPiece utf8);

// Converts between wide and the system multi-byte representations of a string.
// DANGER: This will lose information and can change (on Windows, this can
// change between reboots).
WINBASE_EXPORT std::string SysWideToNativeMB(const std::wstring& wide);
WINBASE_EXPORT std::wstring SysNativeMBToWide(StringPiece native_mb);

// Windows-specific ------------------------------------------------------------

// Converts between 8-bit and wide strings, using the given code page. The
// code page identifier is one accepted by the Windows function
// MultiByteToWideChar().
WINBASE_EXPORT std::wstring SysMultiByteToWide(StringPiece mb, 
                                               uint32_t code_page);
WINBASE_EXPORT std::string SysWideToMultiByte(const std::wstring& wide,
                                               uint32_t code_page);

}  // namespace winbase

#endif  // WINLIB_WINBASE_STRINGS_SYS_STRING_CONVERSIONS_H_