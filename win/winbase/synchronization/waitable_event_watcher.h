// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_SYNCHRONIZATION_WAITABLE_EVENT_WATCHER_H_
#define WINLIB_WINBASE_SYNCHRONIZATION_WAITABLE_EVENT_WATCHER_H_

#include "winbase\base_export.h"
#include "winbase\macros.h"
#include "winbase\sequenced_task_runner.h"
#include "winbase\win\object_watcher.h"
#include "winbase\win\scoped_handle.h"
#include "winbase\functional\callback.h"

namespace winbase {

class Flag;
class AsyncWaiter;
class WaitableEvent;

// This class provides a way to wait on a WaitableEvent asynchronously.
//
// Each instance of this object can be waiting on a single WaitableEvent. When
// the waitable event is signaled, a callback is invoked on the sequence that
// called StartWatching(). This callback can be deleted by deleting the waiter.
//
// Typical usage:
//
//   class MyClass {
//    public:
//     void DoStuffWhenSignaled(WaitableEvent *waitable_event) {
//       watcher_.StartWatching(waitable_event,
//           winbase::BindOnce(&MyClass::OnWaitableEventSignaled, this);
//     }
//    private:
//     void OnWaitableEventSignaled(WaitableEvent* waitable_event) {
//       // OK, time to do stuff!
//     }
//     winbase::WaitableEventWatcher watcher_;
//   };
//
// In the above example, MyClass wants to "do stuff" when waitable_event
// becomes signaled. WaitableEventWatcher makes this task easy. When MyClass
// goes out of scope, the watcher_ will be destroyed, and there is no need to
// worry about OnWaitableEventSignaled being called on a deleted MyClass
// pointer.
//
// BEWARE: With automatically reset WaitableEvents, a signal may be lost if it
// occurs just before a WaitableEventWatcher is deleted. There is currently no
// safe way to stop watching an automatic reset WaitableEvent without possibly
// missing a signal.
//
// NOTE: you /are/ allowed to delete the WaitableEvent while still waiting on
// it with a Watcher. But pay attention: if the event was signaled and deleted
// right after, the callback may be called with deleted WaitableEvent pointer.

class WINBASE_EXPORT WaitableEventWatcher
    : public win::ObjectWatcher::Delegate
{
 public:
  using EventCallback = OnceCallback<void(WaitableEvent*)>;

  WaitableEventWatcher();
  WaitableEventWatcher(const WaitableEventWatcher&) = delete;
  WaitableEventWatcher& operator=(const WaitableEventWatcher&) = delete;

  ~WaitableEventWatcher() override;

  // When |event| is signaled, |callback| is called on the sequence that called
  // StartWatching().
  // |task_runner| is used for asynchronous executions of calling |callback|.
  bool StartWatching(WaitableEvent* event,
                     EventCallback callback,
                     scoped_refptr<SequencedTaskRunner> task_runner);

  // Cancel the current watch. Must be called from the same sequence which
  // started the watch.
  //
  // Does nothing if no event is being watched, nor if the watch has completed.
  // The callback will *not* be called for the current watch after this
  // function returns. Since the callback runs on the same sequence as this
  // function, it cannot be called during this function either.
  void StopWatching();

 private:
  void OnObjectSignaled(HANDLE h) override;

  // Duplicated handle of the event passed to StartWatching().
  win::ScopedHandle duplicated_event_handle_;

  // A watcher for |duplicated_event_handle_|. The handle MUST outlive
  // |watcher_|.
  win::ObjectWatcher watcher_;

  EventCallback callback_;
  WaitableEvent* event_ = nullptr;
};

}  // namespace winbase

#endif  // WINLIB_WIN1BASE_SYNCHRONIZATION_WAITABLE_EVENT_WATCHER_H_