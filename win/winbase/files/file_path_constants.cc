// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>

#include "winbase\files\file_path.h"
#include "winbase\macros.h"

namespace winbase {

#if defined(WINBASE_FILE_PATH_USES_WIN_SEPARATORS)
const FilePath::CharType FilePath::kSeparators[] =
    WINBASE_FILE_PATH_LITERAL("\\/");
#else  // WINBASE_FILE_PATH_USES_WIN_SEPARATORS
const FilePath::CharType FilePath::kSeparators[] =
    WINBASE_FILE_PATH_LITERAL("/");
#endif  // WINBASE_FILE_PATH_USES_WIN_SEPARATORS

const size_t FilePath::kSeparatorsLength = array_size(kSeparators);

const FilePath::CharType FilePath::kCurrentDirectory[] =
    WINBASE_FILE_PATH_LITERAL(".");
const FilePath::CharType FilePath::kParentDirectory[] =
    WINBASE_FILE_PATH_LITERAL("..");

const FilePath::CharType FilePath::kExtensionSeparator =
    WINBASE_FILE_PATH_LITERAL('.');

}  // namespace winbase