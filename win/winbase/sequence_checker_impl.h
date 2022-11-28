// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_SEQUENCE_CHECKER_IMPL_H_
#define WINLIB_WINBASE_SEQUENCE_CHECKER_IMPL_H_

#include <memory>

#include "winbase\base_export.h"
#include "winbase\compiler_specific.h"
#include "winbase\macros.h"
#include "winbase\synchronization\lock.h"

namespace winbase {

// Real implementation of SequenceChecker for use in debug mode or for temporary
// use in release mode (e.g. to CHECK on a threading issue seen only in the
// wild).
//
// Note: You should almost always use the SequenceChecker class to get the right
// version for your build configuration.
class WINBASE_EXPORT SequenceCheckerImpl {
 public:
  SequenceCheckerImpl();
  SequenceCheckerImpl(const SequenceCheckerImpl&) = delete;
  SequenceCheckerImpl& operator=(const SequenceCheckerImpl&) = delete;
  ~SequenceCheckerImpl();

  // Returns true if called in sequence with previous calls to this method and
  // the constructor.
  bool CalledOnValidSequence() const WARN_UNUSED_RESULT;

  // Unbinds the checker from the currently associated sequence. The checker
  // will be re-bound on the next call to CalledOnValidSequence().
  void DetachFromSequence();

 private:
  class Core;

  // Guards all variables below.
  mutable Lock lock_;
  mutable std::unique_ptr<Core> core_;
};

}  // namespace winbase

#endif  // WINLIB_WINBASE_SEQUENCE_CHECKER_IMPL_H_