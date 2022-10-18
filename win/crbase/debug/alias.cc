// Copyright 2011 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/debug/alias.h"

#include "crbase/compiler_specific.h"

namespace crbase {
namespace debug {

// This file/function should be excluded from LTO/LTCG to ensure that the
// compiler can't see this function's implementation when compiling calls to it.
CR_NOINLINE void Alias(const void* var) {}

}  // namespace debug
}  // namespace CRbase