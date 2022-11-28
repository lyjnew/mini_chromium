// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_STRINGS_UTF_STRING_CONVERSIONS_H_
#define WINLIB_WINBASE_STRINGS_UTF_STRING_CONVERSIONS_H_

#include <stddef.h>

#include <string>

#include "winbase\base_export.h"
#include "winbase\strings\string16.h"
#include "winbase\strings\string_piece.h"

namespace winbase {

// These convert between UTF-8, -16 strings. They are potentially slow,
// so avoid unnecessary conversions. The low-level versions return a boolean
// indicating whether the conversion was 100% valid. In this case, it will still
// do the best it can and put the result in the output buffer. The versions that
// return strings ignore this error and just return the best conversion
// possible.

WINBASE_EXPORT bool UTF8ToUTF16(const char* src, size_t src_len, string16* output);
WINBASE_EXPORT string16 UTF8ToUTF16(StringPiece utf8);
WINBASE_EXPORT bool UTF16ToUTF8(const char16* src, size_t src_len,
                                std::string* output);
WINBASE_EXPORT std::string UTF16ToUTF8(StringPiece16 utf16);

inline bool UTF8ToWide(const char* src, size_t src_len, string16* output) {
  return UTF8ToUTF16(src, src_len, output);
}
inline string16 UTF8ToWide(StringPiece utf8) {
  return UTF8ToUTF16(utf8);
}
inline bool WideToUTF8(const char16* src, size_t src_len, std::string* output) {
  return UTF16ToUTF8(src, src_len, output);
}
inline std::string WideToUTF8(StringPiece16 utf16) {
  return UTF16ToUTF8(utf16);
}

// This converts an ASCII string, typically a hardcoded constant, to a UTF16
// string.
WINBASE_EXPORT string16 ASCIIToUTF16(StringPiece ascii);

// Converts to 7-bit ASCII by truncating. The result must be known to be ASCII
// beforehand.
WINBASE_EXPORT std::string UTF16ToASCII(StringPiece16 utf16);

}  // namespace winbase

#endif  // WINLIB_WINBASE_STRINGS_UTF_STRING_CONVERSIONS_H_