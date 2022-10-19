// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/synchronization/cancellation_flag.h"

#include "crbase/logging.h"

namespace crbase {

void CancellationFlag::Set() {
#if !defined(NDEBUG)
  CR_DCHECK_EQ(set_on_, PlatformThread::CurrentId());
#endif
  crbase::subtle::Release_Store(&flag_, 1);
}

bool CancellationFlag::IsSet() const {
  return crbase::subtle::Acquire_Load(&flag_) != 0;
}

void CancellationFlag::UnsafeResetForTesting() {
  crbase::subtle::Release_Store(&flag_, 0);
}

}  // namespace crbase