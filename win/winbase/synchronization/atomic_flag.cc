// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "winbase\synchronization\atomic_flag.h"

#include "winbase\logging.h"

namespace winbase {

AtomicFlag::AtomicFlag() {
  // It doesn't matter where the AtomicFlag is built so long as it's always
  // Set() from the same sequence after. Note: the sequencing requirements are
  // necessary for IsSet()'s callers to know which sequence's memory operations
  // they are synchronized with.
  WINBASE_DETACH_FROM_SEQUENCE(set_sequence_checker_);
}

void AtomicFlag::Set() {
  WINBASE_DCHECK_CALLED_ON_VALID_SEQUENCE(set_sequence_checker_);
  winbase::subtle::Release_Store(&flag_, 1);
}

bool AtomicFlag::IsSet() const {
  return winbase::subtle::Acquire_Load(&flag_) != 0;
}

void AtomicFlag::UnsafeResetForTesting() {
  winbase::subtle::Release_Store(&flag_, 0);
}

}  // namespace winbase