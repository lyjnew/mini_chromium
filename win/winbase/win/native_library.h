// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_WIN_NATIVE_LIBRARY_H_
#define WINLIB_WINBASE_WIN_NATIVE_LIBRARY_H_

// This file defines a cross-platform "NativeLibrary" type which represents
// a loadable module.

#include <windows.h>

#include <string>

#include "winbase\base_export.h"
#include "winbase\strings/string_piece.h"

namespace winbase {

class FilePath;

using NativeLibrary = HMODULE;

struct WINBASE_EXPORT NativeLibraryLoadError {
  NativeLibraryLoadError() : code(0) {}

  // Returns a string representation of the load error.
  std::string ToString() const;

  DWORD code;
};

struct WINBASE_EXPORT NativeLibraryOptions {
  NativeLibraryOptions() = default;
  NativeLibraryOptions(const NativeLibraryOptions& options) = default;

  // If |true|, a loaded library is required to prefer local symbol resolution
  // before considering global symbols. Note that this is already the default
  // behavior on most systems. Setting this to |false| does not guarantee the
  // inverse, i.e., it does not force a preference for global symbols over local
  // ones.
  bool prefer_own_symbols = false;
};

// Loads a native library from disk.  Release it with UnloadNativeLibrary when
// you're done.  Returns NULL on failure.
// If |error| is not NULL, it may be filled in on load error.
WINBASE_EXPORT NativeLibrary LoadNativeLibrary(const FilePath& library_path,
                                               NativeLibraryLoadError* error);

// Loads a native library from disk.  Release it with UnloadNativeLibrary when
// you're done.  Returns NULL on failure.
// If |error| is not NULL, it may be filled in on load error.
WINBASE_EXPORT NativeLibrary LoadNativeLibraryWithOptions(
    const FilePath& library_path,
    const NativeLibraryOptions& options,
    NativeLibraryLoadError* error);

// Unloads a native library.
WINBASE_EXPORT void UnloadNativeLibrary(NativeLibrary library);

// Gets a function pointer from a native library.
WINBASE_EXPORT void* GetFunctionPointerFromNativeLibrary(NativeLibrary library,
                                                         StringPiece name);

// Returns the full platform-specific name for a native library. |name| must be
// ASCII. This is also the default name for the output of a gn |shared_library|
// target. See tools/gn/docs/reference.md#shared_library.
// For example for "mylib", it returns:
// - "mylib.dll" on Windows
// - "libmylib.so" on Linux
// - "libmylib.dylib" on Mac
WINBASE_EXPORT std::string GetNativeLibraryName(StringPiece name);

// Returns the full platform-specific name for a gn |loadable_module| target.
// See tools/gn/docs/reference.md#loadable_module
// The returned name is the same as GetNativeLibraryName() on all platforms
// except for Mac where for "mylib" it returns "mylib.so".
WINBASE_EXPORT std::string GetLoadableModuleName(StringPiece name);

}  // namespace winbase

#endif  // WINLIB_WINBASE_WIN_NATIVE_LIBRARY_H_