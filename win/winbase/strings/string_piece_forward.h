// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Forward declaration of StringPiece types from base/strings/string_piece.h

#ifndef WINLIB_WINBASE_STRINGS_STRING_PIECE_FORWARD_H_
#define WINLIB_WINBASE_STRINGS_STRING_PIECE_FORWARD_H_

#include <string>

#include "winbase\strings\string16.h"

namespace winbase {

template <typename STRING_TYPE>
class BasicStringPiece;
using StringPiece = BasicStringPiece<std::string>;
using StringPiece16 = BasicStringPiece<string16>;

}  // namespace winbase

#endif  // WINLIB_WINBASE_STRINGS_STRING_PIECE_FORWARD_H_