// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_WIN_SCOPED_HANDLE_VERIFIER_H_
#define WINLIB_WINBASE_WIN_SCOPED_HANDLE_VERIFIER_H_

#include "winbase\win\windows_types.h"

#include <unordered_map>

#include "winbase\base_export.h"
#include "winbase\debug\stack_trace.h"
#include "winbase\hash.h"
#include "winbase\synchronization\lock_impl.h"
#include "winbase\threading\thread_local.h"

namespace winbase {
namespace win {
namespace internal {

struct HandleHash {
  size_t operator()(const HANDLE& handle) const {
    char buffer[sizeof(handle)];
    memcpy(buffer, &handle, sizeof(handle));
    return winbase::Hash(buffer, sizeof(buffer));
  }
};

struct ScopedHandleVerifierInfo {
  const void* owner;
  const void* pc1;
  const void* pc2;
  winbase::debug::StackTrace stack;
  DWORD thread_id;
};

// Implements the actual object that is verifying handles for this process.
// The active instance is shared across the module boundary but there is no
// way to delete this object from the wrong side of it (or any side, actually).
// We need [[clang::lto_visibility_public]] because instances of this class are
// passed across module boundaries. This means different modules must have
// compatible definitions of the class even when whole program optimization is
// enabled - which is what this attribute accomplishes. The pragma stops MSVC
// from emitting an unrecognized attribute warning.
#pragma warning(push)
#pragma warning(disable : 5030)
class [[clang::lto_visibility_public]] ScopedHandleVerifier {
#pragma warning(pop)
 public:
  explicit ScopedHandleVerifier(bool enabled);

  ScopedHandleVerifier(const ScopedHandleVerifier&) = delete;
  ScopedHandleVerifier& operator=(const ScopedHandleVerifier&) = delete;

  // Retrieves the current verifier.
  static ScopedHandleVerifier* Get();

  // The methods required by HandleTraits. They are virtual because we need to
  // forward the call execution to another module, instead of letting the
  // compiler call the version that is linked in the current module.
  virtual bool CloseHandle(HANDLE handle);
  virtual void StartTracking(HANDLE handle, const void* owner, const void* pc1,
                             const void* pc2);
  virtual void StopTracking(HANDLE handle, const void* owner, const void* pc1,
                            const void* pc2);
  virtual void Disable();
  virtual void OnHandleBeingClosed(HANDLE handle);
  virtual HMODULE GetModule() const;

 private:
  ~ScopedHandleVerifier();  // Not implemented.

  static winbase::internal::LockImpl* GetLock();
  static void InstallVerifier();

  winbase::debug::StackTrace creation_stack_;
  bool enabled_;
  winbase::ThreadLocalBoolean closing_;
  winbase::internal::LockImpl* lock_;
  std::unordered_map<HANDLE, ScopedHandleVerifierInfo, HandleHash> map_;
};

}  // namespace internal
}  // namespace win
}  // namespace winbase

#endif  // WINLIB_WINBASE_WIN_SCOPED_HANDLE_VERIFIER_H_