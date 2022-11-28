// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_FILES_SCOPED_FILE_H_
#define WINLIB_WINBASE_FILES_SCOPED_FILE_H_

#include <stdio.h>

#include <memory>

#include "winbase\base_export.h"
#include "winbase\logging.h"
#include "winbase\scoped_generic.h"

namespace winbase {

namespace internal {

// Functor for |ScopedFILE| (below).
struct ScopedFILECloser {
  inline void operator()(FILE* x) const {
    if (x)
      fclose(x);
  }
};

}  // namespace internal

// -----------------------------------------------------------------------------

// Automatically closes |FILE*|s.
typedef std::unique_ptr<FILE, internal::ScopedFILECloser> ScopedFILE;

}  // namespace winbase

#endif  // WINLIB_WINBASE_FILES_SCOPED_FILE_H_