// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_CRBASE_MEMORY_RAW_SCOPED_REFPTR_MISMATCH_CHECKER_H_
#define MINI_CHROMIUM_CRBASE_MEMORY_RAW_SCOPED_REFPTR_MISMATCH_CHECKER_H_

#include "crbase/memory/ref_counted.h"
#include "crbase/template_util.h"
#include "crbase/tuple.h"
#include "crbuild/build_config.h"

// It is dangerous to post a task with a T* argument where T is a subtype of
// RefCounted(Base|ThreadSafeBase), since by the time the parameter is used, the
// object may already have been deleted since it was not held with a
// scoped_refptr. Example: http://crbug.com/27191
// The following set of traits are designed to generate a compile error
// whenever this antipattern is attempted.

namespace crbase {

// This is a base internal implementation file used by task.h and callback.h.
// Not for public consumption, so we wrap it in namespace internal.
namespace internal {

template <typename T>
struct NeedsScopedRefptrButGetsRawPtr {
  enum {
    value = crbase::false_type::value
  };
};

template <typename Params>
struct ParamsUseScopedRefptrCorrectly {
  enum { value = 0 };
};

template <>
struct ParamsUseScopedRefptrCorrectly<Tuple<>> {
  enum { value = 1 };
};

template <typename Head, typename... Tail>
struct ParamsUseScopedRefptrCorrectly<Tuple<Head, Tail...>> {
  enum { value = !NeedsScopedRefptrButGetsRawPtr<Head>::value &&
                 ParamsUseScopedRefptrCorrectly<Tuple<Tail...>>::value };
};

}  // namespace internal

}  // namespace crbase

#endif  // MINI_CHROMIUM_CRBASE_MEMORY_RAW_SCOPED_REFPTR_MISMATCH_CHECKER_H_