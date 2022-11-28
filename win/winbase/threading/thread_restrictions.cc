// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "winbase\threading\thread_restrictions.h"

#if WINBASE_DCHECK_IS_ON()

#include "winbase\lazy_instance.h"
#include "winbase\logging.h"
#include "winbase\threading\thread_local.h"

namespace winbase {

namespace {

LazyInstance<ThreadLocalBoolean>::Leaky g_blocking_disallowed =
    WINBASE_LAZY_INSTANCE_INITIALIZER;

LazyInstance<ThreadLocalBoolean>::Leaky g_singleton_disallowed =
    WINBASE_LAZY_INSTANCE_INITIALIZER;

LazyInstance<ThreadLocalBoolean>::Leaky g_base_sync_primitives_disallowed =
    WINBASE_LAZY_INSTANCE_INITIALIZER;

}  // namespace

void AssertBlockingAllowed() {
  WINBASE_DCHECK(!g_blocking_disallowed.Get().Get())
      << "Function marked as blocking was called from a scope that disallows "
         "blocking! If this task is running inside the TaskScheduler, it needs "
         "to have MayBlock() in its TaskTraits. Otherwise, consider making "
         "this blocking work asynchronous or, as a last resort, you may use "
         "ScopedAllowBlocking (see its documentation for best practices).";
}

void DisallowBlocking() {
  g_blocking_disallowed.Get().Set(true);
}

ScopedDisallowBlocking::ScopedDisallowBlocking()
    : was_disallowed_(g_blocking_disallowed.Get().Get()) {
  g_blocking_disallowed.Get().Set(true);
}

ScopedDisallowBlocking::~ScopedDisallowBlocking() {
  WINBASE_DCHECK(g_blocking_disallowed.Get().Get());
  g_blocking_disallowed.Get().Set(was_disallowed_);
}

ScopedAllowBlocking::ScopedAllowBlocking()
    : was_disallowed_(g_blocking_disallowed.Get().Get()) {
  g_blocking_disallowed.Get().Set(false);
}

ScopedAllowBlocking::~ScopedAllowBlocking() {
  WINBASE_DCHECK(!g_blocking_disallowed.Get().Get());
  g_blocking_disallowed.Get().Set(was_disallowed_);
}

void DisallowBaseSyncPrimitives() {
  g_base_sync_primitives_disallowed.Get().Set(true);
}

ScopedAllowBaseSyncPrimitives::ScopedAllowBaseSyncPrimitives()
    : was_disallowed_(g_base_sync_primitives_disallowed.Get().Get()) {
  WINBASE_DCHECK(!g_blocking_disallowed.Get().Get())
      << "To allow //base sync primitives in a scope where blocking is "
         "disallowed use ScopedAllowBaseSyncPrimitivesOutsideBlockingScope.";
  g_base_sync_primitives_disallowed.Get().Set(false);
}

ScopedAllowBaseSyncPrimitives::~ScopedAllowBaseSyncPrimitives() {
  WINBASE_DCHECK(!g_base_sync_primitives_disallowed.Get().Get());
  g_base_sync_primitives_disallowed.Get().Set(was_disallowed_);
}

ScopedAllowBaseSyncPrimitivesOutsideBlockingScope::
    ScopedAllowBaseSyncPrimitivesOutsideBlockingScope()
    : was_disallowed_(g_base_sync_primitives_disallowed.Get().Get()) {
  g_base_sync_primitives_disallowed.Get().Set(false);
}

ScopedAllowBaseSyncPrimitivesOutsideBlockingScope::
    ~ScopedAllowBaseSyncPrimitivesOutsideBlockingScope() {
  WINBASE_DCHECK(!g_base_sync_primitives_disallowed.Get().Get());
  g_base_sync_primitives_disallowed.Get().Set(was_disallowed_);
}

ScopedAllowBaseSyncPrimitivesForTesting::
    ScopedAllowBaseSyncPrimitivesForTesting()
    : was_disallowed_(g_base_sync_primitives_disallowed.Get().Get()) {
  g_base_sync_primitives_disallowed.Get().Set(false);
}

ScopedAllowBaseSyncPrimitivesForTesting::
    ~ScopedAllowBaseSyncPrimitivesForTesting() {
  WINBASE_DCHECK(!g_base_sync_primitives_disallowed.Get().Get());
  g_base_sync_primitives_disallowed.Get().Set(was_disallowed_);
}

namespace internal {

void AssertBaseSyncPrimitivesAllowed() {
  WINBASE_DCHECK(!g_base_sync_primitives_disallowed.Get().Get())
      << "Waiting on a //base sync primitive is not allowed on this thread to "
         "prevent jank and deadlock. If waiting on a //base sync primitive is "
         "unavoidable, do it within the scope of a "
         "ScopedAllowBaseSyncPrimitives. If in a test, "
         "use ScopedAllowBaseSyncPrimitivesForTesting.";
}

void ResetThreadRestrictionsForTesting() {
  g_blocking_disallowed.Get().Set(false);
  g_singleton_disallowed.Get().Set(false);
  g_base_sync_primitives_disallowed.Get().Set(false);
}

}  // namespace internal

ThreadRestrictions::ScopedAllowIO::ScopedAllowIO()
    : was_allowed_(SetIOAllowed(true)) {}

ThreadRestrictions::ScopedAllowIO::~ScopedAllowIO() {
  SetIOAllowed(was_allowed_);
}

// static
bool ThreadRestrictions::SetIOAllowed(bool allowed) {
  bool previous_disallowed = g_blocking_disallowed.Get().Get();
  g_blocking_disallowed.Get().Set(!allowed);
  return !previous_disallowed;
}

// static
bool ThreadRestrictions::SetSingletonAllowed(bool allowed) {
  bool previous_disallowed = g_singleton_disallowed.Get().Get();
  g_singleton_disallowed.Get().Set(!allowed);
  return !previous_disallowed;
}

// static
void ThreadRestrictions::AssertSingletonAllowed() {
  if (g_singleton_disallowed.Get().Get()) {
    WINBASE_NOTREACHED()
        << "LazyInstance/Singleton is not allowed to be used on this "
        << "thread.  Most likely it's because this thread is not "
        << "joinable (or the current task is running with "
        << "TaskShutdownBehavior::CONTINUE_ON_SHUTDOWN semantics), so "
        << "AtExitManager may have deleted the object on shutdown, "
        << "leading to a potential shutdown crash. If you need to use "
        << "the object from this context, it'll have to be updated to "
        << "use Leaky traits.";
  }
}

// static
void ThreadRestrictions::DisallowWaiting() {
  DisallowBaseSyncPrimitives();
}

bool ThreadRestrictions::SetWaitAllowed(bool allowed) {
  bool previous_disallowed = g_base_sync_primitives_disallowed.Get().Get();
  g_base_sync_primitives_disallowed.Get().Set(!allowed);
  return !previous_disallowed;
}

ThreadRestrictions::ScopedAllowWait::ScopedAllowWait()
    : was_allowed_(SetWaitAllowed(true)) {}

ThreadRestrictions::ScopedAllowWait::~ScopedAllowWait() {
  SetWaitAllowed(was_allowed_);
}

}  // namespace winbase

#endif  // WINBASE_DCHECK_IS_ON()