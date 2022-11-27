// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_SYNCHRONIZATION_ATOMIC_FLAG_H_
#define WINLIB_WINBASE_SYNCHRONIZATION_ATOMIC_FLAG_H_

#include "winbase\atomic\atomicops.h"
#include "winbase\base_export.h"
#include "winbase\macros.h"
///#include "winbase\sequence_checker.h"

namespace winbase {

// A flag that can safely be set from one thread and read from other threads.
//
// This class IS NOT intended for synchronization between threads.
class WINBASE_EXPORT AtomicFlag {
 public:
  AtomicFlag();
  ~AtomicFlag() = default;

  AtomicFlag(const AtomicFlag&) = delete;
  AtomicFlag& operator=(const AtomicFlag&) = delete;

  // Set the flag. Must always be called from the same sequence.
  void Set();

  // Returns true iff the flag was set. If this returns true, the current thread
  // is guaranteed to be synchronized with all memory operations on the sequence
  // which invoked Set() up until at least the first call to Set() on it.
  bool IsSet() const;

  // Resets the flag. Be careful when using this: callers might not expect
  // IsSet() to return false after returning true once.
  void UnsafeResetForTesting();

 private:
  winbase::subtle::Atomic32 flag_ = 0;
  ///SEQUENCE_CHECKER(set_sequence_checker_);
};

}  // namespace winbase

#endif  // WINLIB_WINBASE_SYNCHRONIZATION_ATOMIC_FLAG_H_