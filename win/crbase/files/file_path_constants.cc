// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>

#include "crbase/files/file_path.h"
#include "crbase/macros.h"

namespace crbase {

#if defined(CR_FILE_PATH_USES_WIN_SEPARATORS)
const FilePath::CharType FilePath::kSeparators[] = CR_FILE_PATH_LITERAL("\\/");
#else  // CR_FILE_PATH_USES_WIN_SEPARATORS
const FilePath::CharType FilePath::kSeparators[] = CR_FILE_PATH_LITERAL("/");
#endif  // CR_FILE_PATH_USES_WIN_SEPARATORS

const size_t FilePath::kSeparatorsLength = cr_arraysize(kSeparators);

const FilePath::CharType FilePath::kCurrentDirectory[] 
    = CR_FILE_PATH_LITERAL(".");
const FilePath::CharType FilePath::kParentDirectory[] 
    = CR_FILE_PATH_LITERAL("..");

const FilePath::CharType FilePath::kExtensionSeparator 
    = CR_FILE_PATH_LITERAL('.');

}  // namespace crbase