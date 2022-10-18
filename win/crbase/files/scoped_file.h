// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_CRBASE_FILES_SCOPED_FILE_H_
#define MINI_CHROMIUM_CRBASE_FILES_SCOPED_FILE_H_

#include <stdio.h>

#include "crbase/crbase_export.h"
#include "crbase/logging.h"
#include "crbase/memory/scoped_ptr.h"
///#include "crbase/scoped_generic.h"
#include "crbuild/build_config.h"

namespace crbase {

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
typedef scoped_ptr<FILE, internal::ScopedFILECloser> ScopedFILE;

}  // namespace crbase

#endif  // MINI_CHROMIUM_CRBASE_FILES_SCOPED_FILE_H_