// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "winbase\debug\stack_trace.h"

#include <string.h>

#include <algorithm>
#include <sstream>

#include "winbase\logging.h"
#include "winbase\macros.h"
#include "winbase\win\nominmax.h"

namespace winbase {
namespace debug {

StackTrace::StackTrace() : StackTrace(array_size(trace_)) {}

StackTrace::StackTrace(const void* const* trace, size_t count) {
  count = std::min(count, array_size(trace_));
  if (count)
    memcpy(trace_, trace, count * sizeof(trace_[0]));
  count_ = count;
}

const void *const *StackTrace::Addresses(size_t* count) const {
  *count = count_;
  if (count_)
    return trace_;
  return nullptr;
}

std::string StackTrace::ToString() const {
  std::stringstream stream;
#if !defined(__UCLIBC__) && !defined(_AIX)
  OutputToStream(&stream);
#endif
  return stream.str();
}


}  // namespace debug
}  // namespace winbase