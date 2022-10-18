// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_CRBASE_THREADING_THREAD_LOCAL_STORAGE_H_
#define MINI_CHROMIUM_CRBASE_THREADING_THREAD_LOCAL_STORAGE_H_

#include <windows.h>

#include "crbase/atomicops.h"
#include "crbase/crbase_export.h"
#include "crbase/macros.h"
#include "crbuild/build_config.h"

namespace crbase {

namespace internal {

// WARNING: You should *NOT* be using this class directly.
// PlatformThreadLocalStorage is low-level abstraction to the OS's TLS
// interface, you should instead be using ThreadLocalStorage::StaticSlot/Slot.
class CRBASE_EXPORT PlatformThreadLocalStorage {
 public:

  typedef unsigned long TLSKey;
  enum : unsigned { TLS_KEY_OUT_OF_INDEXES = TLS_OUT_OF_INDEXES };

  // The following methods need to be supported on each OS platform, so that
  // the Chromium ThreadLocalStore functionality can be constructed.
  // Chromium will use these methods to acquire a single OS slot, and then use
  // that to support a much larger number of Chromium slots (independent of the
  // OS restrictions).
  // The following returns true if it successfully is able to return an OS
  // key in |key|.
  static bool AllocTLS(TLSKey* key);
  // Note: FreeTLS() doesn't have to be called, it is fine with this leak, OS
  // might not reuse released slot, you might just reset the TLS value with
  // SetTLSValue().
  static void FreeTLS(TLSKey key);
  static void SetTLSValue(TLSKey key, void* value);
  static void* GetTLSValue(TLSKey key);

  // Each platform (OS implementation) is required to call this method on each
  // terminating thread when the thread is about to terminate.  This method
  // will then call all registered destructors for slots in Chromium
  // ThreadLocalStorage, until there are no slot values remaining as having
  // been set on this thread.
  // Destructors may end up being called multiple times on a terminating
  // thread, as other destructors may re-set slots that were previously
  // destroyed.
  // Since Windows which doesn't support TLS destructor, the implementation
  // should use GetTLSValue() to retrieve the value of TLS slot.
  static void OnThreadExit();
};

}  // namespace internal

// Wrapper for thread local storage.  This class doesn't do much except provide
// an API for portability.
class CRBASE_EXPORT ThreadLocalStorage {
 public:

  // Prototype for the TLS destructor function, which can be optionally used to
  // cleanup thread local storage on thread exit.  'value' is the data that is
  // stored in thread local storage.
  typedef void (*TLSDestructorFunc)(void* value);

  // StaticSlot uses its own struct initializer-list style static
  // initialization, as base's LINKER_INITIALIZED requires a constructor and on
  // some compilers (notably gcc 4.4) this still ends up needing runtime
  // initialization.
  #define TLS_INITIALIZER {0}

  // A key representing one value stored in TLS.
  // Initialize like
  //   ThreadLocalStorage::StaticSlot my_slot = TLS_INITIALIZER;
  // If you're not using a static variable, use the convenience class
  // ThreadLocalStorage::Slot (below) instead.
  struct CRBASE_EXPORT StaticSlot {
    // Set up the TLS slot.  Called by the constructor.
    // 'destructor' is a pointer to a function to perform per-thread cleanup of
    // this object.  If set to NULL, no cleanup is done for this TLS slot.
    void Initialize(TLSDestructorFunc destructor);

    // Free a previously allocated TLS 'slot'.
    // If a destructor was set for this slot, removes
    // the destructor so that remaining threads exiting
    // will not free data.
    void Free();

    // Get the thread-local value stored in slot 'slot'.
    // Values are guaranteed to initially be zero.
    void* Get() const;

    // Set the thread-local value stored in slot 'slot' to
    // value 'value'.
    void Set(void* value);

    bool initialized() const {
      return crbase::subtle::Acquire_Load(&initialized_) != 0;
    }

    // The internals of this struct should be considered private.
    crbase::subtle::Atomic32 initialized_;
    int slot_;
  };

  // A convenience wrapper around StaticSlot with a constructor. Can be used
  // as a member variable.
  class CRBASE_EXPORT Slot : public StaticSlot {
   public:
    // Calls StaticSlot::Initialize().
    explicit Slot(TLSDestructorFunc destructor = NULL);

   private:
    using StaticSlot::initialized_;
    using StaticSlot::slot_;

    CR_DISALLOW_COPY_AND_ASSIGN(Slot);
  };

 private:
  CR_DISALLOW_COPY_AND_ASSIGN(ThreadLocalStorage);
};

}  // namespace crbase

#endif  // MINI_CHROMIUM_CRBASE_THREADING_THREAD_LOCAL_STORAGE_H_