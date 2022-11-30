// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_FILES_PLATFORM_FILE_H_
#define WINLIB_WINBASE_FILES_PLATFORM_FILE_H_

#include "winbase\files\scoped_file.h"
#include "winbase\win\scoped_handle.h"
#include "winbase\win\windows_types.h"

// This file defines platform-independent types for dealing with
// platform-dependent files. If possible, use the higher-level winbase::File 
// class rather than these primitives.

namespace winbase {

using PlatformFile = HANDLE;
using ScopedPlatformFile = ::winbase::win::ScopedHandle;

// It would be nice to make this constexpr but INVALID_HANDLE_VALUE is a
// ((void*)(-1)) which Clang rejects since reinterpret_cast is technically
// disallowed in constexpr. Visual Studio accepts this, however.
const PlatformFile kInvalidPlatformFile = INVALID_HANDLE_VALUE;

}  // namespace

#endif  // WINLIB_WINBASE_FILES_PLATFORM_FILE_H_