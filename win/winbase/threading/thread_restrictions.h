// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_THREADING_THREAD_RESTRICTIONS_H_
#define WINLIB_WINBASE_THREADING_THREAD_RESTRICTIONS_H_

#include "winbase\base_export.h"
#include "winbase\logging.h"
#include "winbase\macros.h"

///class BrowserProcessImpl;
///class HistogramSynchronizer;
///class NativeBackendKWallet;
///class KeyStorageLinux;

namespace winbase {

namespace internal {
class TaskTracker;
}

class GetAppOutputScopedAllowBaseSyncPrimitives;
class SimpleThread;
class StackSamplingProfiler;
class Thread;

#if WINBASE_DCHECK_IS_ON()
#define INLINE_IF_DCHECK_IS_OFF WINBASE_EXPORT
#define EMPTY_BODY_IF_DCHECK_IS_OFF
#else
#define INLINE_IF_DCHECK_IS_OFF inline
#define EMPTY_BODY_IF_DCHECK_IS_OFF \
  {}
#endif

// A "blocking call" refers to any call that causes the calling thread to wait
// off-CPU. It includes but is not limited to calls that wait on synchronous
// file I/O operations: read or write a file from disk, interact with a pipe or
// a socket, rename or delete a file, enumerate files in a directory, etc.
// Acquiring a low contention lock is not considered a blocking call.

// Asserts that blocking calls are allowed in the current scope.
//
// Style tip: It's best if you put AssertBlockingAllowed() checks as close to
// the blocking call as possible. For example:
//
// void ReadFile() {
//   PreWork();
//
//   winbase::AssertBlockingAllowed();
//   fopen(...);
//
//   PostWork();
// }
//
// void Bar() {
//   ReadFile();
// }
//
// void Foo() {
//   Bar();
// }
INLINE_IF_DCHECK_IS_OFF void AssertBlockingAllowed()
    EMPTY_BODY_IF_DCHECK_IS_OFF;

// Disallows blocking on the current thread.
INLINE_IF_DCHECK_IS_OFF void DisallowBlocking() EMPTY_BODY_IF_DCHECK_IS_OFF;

// Disallows blocking calls within its scope.
class WINBASE_EXPORT ScopedDisallowBlocking {
 public:
  ScopedDisallowBlocking() EMPTY_BODY_IF_DCHECK_IS_OFF;
  ~ScopedDisallowBlocking() EMPTY_BODY_IF_DCHECK_IS_OFF;

  ScopedDisallowBlocking(const ScopedDisallowBlocking&) = delete;
  ScopedDisallowBlocking& operator=(const ScopedDisallowBlocking&) = delete;

 private:
#if WINBASE_DCHECK_IS_ON()
  const bool was_disallowed_;
#endif
};

// ScopedAllowBlocking(ForTesting) allow blocking calls within a scope where
// they are normally disallowed.
//
// Avoid using this. Prefer making blocking calls from tasks posted to
// winbase::TaskScheduler with winbase::MayBlock().
//
// Where unavoidable, put ScopedAllow* instances in the narrowest scope possible
// in the caller making the blocking call but no further down. That is: if a
// Cleanup() method needs to do a blocking call, document Cleanup() as blocking
// and add a ScopedAllowBlocking instance in callers that can't avoid making
// this call from a context where blocking is banned, as such:
//   void Client::MyMethod() {
//     (...)
//     {
//       // Blocking is okay here because XYZ.
//       ScopedAllowBlocking allow_blocking;
//       my_foo_->Cleanup();
//     }
//     (...)
//   }
//
//   // This method can block.
//   void Foo::Cleanup() {
//     // Do NOT add the ScopedAllowBlocking in Cleanup() directly as that hides
//     // its blocking nature from unknowing callers and defeats the purpose of
//     // these checks.
//     FlushStateToDisk();
//   }
//
// Note: In rare situations where the blocking call is an implementation detail
// (i.e. the impl makes a call that invokes AssertBlockingAllowed() but it
// somehow knows that in practice this will not block), it might be okay to hide
// the ScopedAllowBlocking instance in the impl with a comment explaining why
// that's okay.
class WINBASE_EXPORT ScopedAllowBlocking {
 private:
  // This can only be instantiated by friends. Use ScopedAllowBlockingForTesting
  // in unit tests to avoid the friend requirement.
  friend class StackSamplingProfiler;

  ScopedAllowBlocking() EMPTY_BODY_IF_DCHECK_IS_OFF;
  ~ScopedAllowBlocking() EMPTY_BODY_IF_DCHECK_IS_OFF;

  ScopedAllowBlocking(const ScopedAllowBlocking&) = delete;
  ScopedAllowBlocking& operator=(const ScopedAllowBlocking&) = delete;

#if WINBASE_DCHECK_IS_ON()
  const bool was_disallowed_;
#endif
};

// "Waiting on a //base sync primitive" refers to calling one of these methods:
// - winbase::WaitableEvent::*Wait*
// - winbase::ConditionVariable::*Wait*
// - winbase::Process::WaitForExit*

// Disallows waiting on a //base sync primitive on the current thread.
INLINE_IF_DCHECK_IS_OFF void DisallowBaseSyncPrimitives()
    EMPTY_BODY_IF_DCHECK_IS_OFF;

// ScopedAllowBaseSyncPrimitives(ForTesting)(OutsideBlockingScope) allow waiting
// on a //base sync primitive within a scope where this is normally disallowed.
//
// Avoid using this.
//
// Instead of waiting on a WaitableEvent or a ConditionVariable, put the work
// that should happen after the wait in a callback and post that callback from
// where the WaitableEvent or ConditionVariable would have been signaled. If
// something needs to be scheduled after many tasks have executed, use
// winbase::BarrierClosure.
//
// On Windows, join processes asynchronously using winbase::win::ObjectWatcher.

// This can only be used in a scope where blocking is allowed.
class WINBASE_EXPORT ScopedAllowBaseSyncPrimitives {
 private:
  // This can only be instantiated by friends. Use
  // ScopedAllowBaseSyncPrimitivesForTesting in unit tests to avoid the friend
  // requirement.
  friend class winbase::GetAppOutputScopedAllowBaseSyncPrimitives;

  ScopedAllowBaseSyncPrimitives() EMPTY_BODY_IF_DCHECK_IS_OFF;
  ~ScopedAllowBaseSyncPrimitives() EMPTY_BODY_IF_DCHECK_IS_OFF;

  ScopedAllowBaseSyncPrimitives(const ScopedAllowBaseSyncPrimitives&) = delete;
  ScopedAllowBaseSyncPrimitives& operator=(
      const ScopedAllowBaseSyncPrimitives&) = delete;

#if WINBASE_DCHECK_IS_ON()
  const bool was_disallowed_;
#endif
};

// This can be used in a scope where blocking is disallowed.
class WINBASE_EXPORT ScopedAllowBaseSyncPrimitivesOutsideBlockingScope {
 private:
  // This can only be instantiated by friends. Use
  // ScopedAllowBaseSyncPrimitivesForTesting in unit tests to avoid the friend
  // requirement.

  ScopedAllowBaseSyncPrimitivesOutsideBlockingScope()
      EMPTY_BODY_IF_DCHECK_IS_OFF;
  ~ScopedAllowBaseSyncPrimitivesOutsideBlockingScope()
      EMPTY_BODY_IF_DCHECK_IS_OFF;

  ScopedAllowBaseSyncPrimitivesOutsideBlockingScope(
      const ScopedAllowBaseSyncPrimitivesOutsideBlockingScope&) = delete;
  ScopedAllowBaseSyncPrimitivesOutsideBlockingScope& operator=(
      const ScopedAllowBaseSyncPrimitivesOutsideBlockingScope&) = delete;

#if WINBASE_DCHECK_IS_ON()
  const bool was_disallowed_;
#endif
};

namespace internal {

// Asserts that waiting on a //base sync primitive is allowed in the current
// scope.
INLINE_IF_DCHECK_IS_OFF void AssertBaseSyncPrimitivesAllowed()
    EMPTY_BODY_IF_DCHECK_IS_OFF;

// Resets all thread restrictions on the current thread.
INLINE_IF_DCHECK_IS_OFF void ResetThreadRestrictionsForTesting()
    EMPTY_BODY_IF_DCHECK_IS_OFF;

}  // namespace internal

class WINBASE_EXPORT ThreadRestrictions {
 public:
  ThreadRestrictions() = delete;
  ThreadRestrictions(const ThreadRestrictions&) = delete;
  ThreadRestrictions& operator=(const ThreadRestrictions&) = delete;

  // Constructing a ScopedAllowIO temporarily allows IO for the current
  // thread.  Doing this is almost certainly always incorrect.
  //
  // DEPRECATED. Use ScopedAllowBlocking(ForTesting).
  class WINBASE_EXPORT ScopedAllowIO {
   public:
    ScopedAllowIO() EMPTY_BODY_IF_DCHECK_IS_OFF;
    ~ScopedAllowIO() EMPTY_BODY_IF_DCHECK_IS_OFF;

    ScopedAllowIO(const ScopedAllowIO&) = delete;
    ScopedAllowIO& operator=(const ScopedAllowIO&) = delete;

   private:
#if WINBASE_DCHECK_IS_ON()
    const bool was_allowed_;
#endif
  };

#if WINBASE_DCHECK_IS_ON()
  // Set whether the current thread to make IO calls.
  // Threads start out in the *allowed* state.
  // Returns the previous value.
  //
  // DEPRECATED. Use ScopedAllowBlocking(ForTesting) or ScopedDisallowBlocking.
  static bool SetIOAllowed(bool allowed);

  // Set whether the current thread can use singletons.  Returns the previous
  // value.
  static bool SetSingletonAllowed(bool allowed);

  // Check whether the current thread is allowed to use singletons (Singleton /
  // LazyInstance).  DCHECKs if not.
  static void AssertSingletonAllowed();

  // Disable waiting on the current thread. Threads start out in the *allowed*
  // state. Returns the previous value.
  //
  // DEPRECATED. Use DisallowBaseSyncPrimitives.
  static void DisallowWaiting();
#else
  // Inline the empty definitions of these functions so that they can be
  // compiled out.
  static bool SetIOAllowed(bool allowed) { return true; }
  static bool SetSingletonAllowed(bool allowed) { return true; }
  static void AssertSingletonAllowed() {}
  static void DisallowWaiting() {}
#endif

 private:
  // DO NOT ADD ANY OTHER FRIEND STATEMENTS.
  // BEGIN ALLOWED USAGE.
  friend class winbase::StackSamplingProfiler;
  friend class internal::TaskTracker;
  friend class MessagePumpDefault;
  friend class SimpleThread;
  friend class Thread;
  friend class ThreadTestHelper;
  friend class PlatformThread;
  
  // END ALLOWED USAGE.
  // BEGIN USAGE THAT NEEDS TO BE FIXED.
  // ....
  // END USAGE THAT NEEDS TO BE FIXED.

#if WINBASE_DCHECK_IS_ON()
  // DEPRECATED. Use ScopedAllowBaseSyncPrimitives.
  static bool SetWaitAllowed(bool allowed);
#else
  static bool SetWaitAllowed(bool allowed) { return true; }
#endif

  // Constructing a ScopedAllowWait temporarily allows waiting on the current
  // thread.  Doing this is almost always incorrect, which is why we limit who
  // can use this through friend.
  //
  // DEPRECATED. Use ScopedAllowBaseSyncPrimitives.
  class WINBASE_EXPORT ScopedAllowWait {
   public:
    ScopedAllowWait() EMPTY_BODY_IF_DCHECK_IS_OFF;
    ~ScopedAllowWait() EMPTY_BODY_IF_DCHECK_IS_OFF;

    ScopedAllowWait(const ScopedAllowWait&) = delete;
    ScopedAllowWait& operator=(const ScopedAllowWait&) = delete;

   private:
#if WINBASE_DCHECK_IS_ON()
    const bool was_allowed_;
#endif
  };
};

}  // namespace winbase

#undef INLINE_IF_DCHECK_IS_OFF
#undef EMPTY_BODY_IF_DCHECK_IS_OFF

#endif  // WINLIB_WINBASE_THREADING_THREAD_RESTRICTIONS_H_