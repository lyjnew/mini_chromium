// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_SCOPED_NATIVE_LIBRARY_H_
#define WINLIB_WINBASE_SCOPED_NATIVE_LIBRARY_H_

#include "winbase\base_export.h"
#include "winbase\macros.h"
#include "winbase\win\native_library.h"

namespace winbase {

class FilePath;

// A class which encapsulates a base::NativeLibrary object available only in a
// scope.
// This class automatically unloads the loaded library in its destructor.
class WINBASE_EXPORT ScopedNativeLibrary {
 public:
  // Initializes with a NULL library.
  ScopedNativeLibrary();
  ScopedNativeLibrary(const ScopedNativeLibrary&) = delete;
  ScopedNativeLibrary& operator=(const ScopedNativeLibrary&) = delete;

  // Takes ownership of the given library handle.
  explicit ScopedNativeLibrary(NativeLibrary library);

  // Opens the given library and manages its lifetime.
  explicit ScopedNativeLibrary(const FilePath& library_path);

  ~ScopedNativeLibrary();

  // Returns true if there's a valid library loaded.
  bool is_valid() const { return !!library_; }

  NativeLibrary get() const { return library_; }

  void* GetFunctionPointer(const char* function_name) const;

  // Takes ownership of the given library handle. Any existing handle will
  // be freed.
  void Reset(NativeLibrary library);

  // Returns the native library handle and removes it from this object. The
  // caller must manage the lifetime of the handle.
  NativeLibrary Release();

 private:
  NativeLibrary library_;
};

}  // namespace winbase

#endif  // WINLIB_WINBASE_SCOPED_NATIVE_LIBRARY_H_