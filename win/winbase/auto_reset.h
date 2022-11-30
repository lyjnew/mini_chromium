// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_AUTO_RESET_H_
#define WINLIB_WINBASE_AUTO_RESET_H_

#include <utility>

#include "winbase\macros.h"

// winbase::AutoReset<> is useful for setting a variable to a new value only 
// within a particular scope. An winbase::AutoReset<> object resets a variable 
// to its original value upon destruction, making it an alternative to writing
// "var = false;" or "var = old_val;" at all of a block's exit points.
//
// This should be obvious, but note that an winbase::AutoReset<> instance should
// have a shorter lifetime than its scoped_variable, to prevent invalid memory
// writes when the winbase::AutoReset<> object is destroyed.

namespace winbase {

template<typename T>
class AutoReset {
 public:
  AutoReset(T* scoped_variable, T new_value)
      : scoped_variable_(scoped_variable),
        original_value_(std::move(*scoped_variable)) {
    *scoped_variable_ = std::move(new_value);
  }

  AutoReset(const AutoReset&) = delete;
  AutoReset& operator=(const AutoReset&) = delete;

  ~AutoReset() { *scoped_variable_ = std::move(original_value_); }

 private:
  T* scoped_variable_;
  T original_value_;
};

}  // namespace winbase

#endif  // WINLIB_WINBASE_AUTO_RESET_H_