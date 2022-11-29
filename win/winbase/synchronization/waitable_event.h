// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_SYNCHRONIZATION_WAITABLE_EVENT_H_
#define WINLIB_WINBASE_SYNCHRONIZATION_WAITABLE_EVENT_H_

#include <stddef.h>

#include <list>
#include <memory>

#include "winbase\base_export.h"
#include "winbase\macros.h"
#include "winbase\win\scoped_handle.h"
#include "winbase\functional\callback_forward.h"
#include "winbase\memory\ref_counted.h"
#include "winbase\synchronization\lock.h"

namespace winbase {

class TimeDelta;
class TimeTicks;

// A WaitableEvent can be a useful thread synchronization tool when you want to
// allow one thread to wait for another thread to finish some work. For
// non-Windows systems, this can only be used from within a single address
// space.
//
// Use a WaitableEvent when you would otherwise use a Lock+ConditionVariable to
// protect a simple boolean value.  However, if you find yourself using a
// WaitableEvent in conjunction with a Lock to wait for a more complex state
// change (e.g., for an item to be added to a queue), then you should probably
// be using a ConditionVariable instead of a WaitableEvent.
//
// NOTE: On Windows, this class provides a subset of the functionality afforded
// by a Windows event object.  This is intentional.  If you are writing Windows
// specific code and you need other features of a Windows event, then you might
// be better off just using an Windows event directly.
class WINBASE_EXPORT WaitableEvent {
 public:
  // Indicates whether a WaitableEvent should automatically reset the event
  // state after a single waiting thread has been released or remain signaled
  // until Reset() is manually invoked.
  enum class ResetPolicy { MANUAL, AUTOMATIC };

  // Indicates whether a new WaitableEvent should start in a signaled state or
  // not.
  enum class InitialState { SIGNALED, NOT_SIGNALED };

  // Constructs a WaitableEvent with policy and initial state as detailed in
  // the above enums.
  WaitableEvent(ResetPolicy reset_policy = ResetPolicy::MANUAL,
                InitialState initial_state = InitialState::NOT_SIGNALED);

  // Create a WaitableEvent from an Event HANDLE which has already been
  // created. This objects takes ownership of the HANDLE and will close it when
  // deleted.
  explicit WaitableEvent(win::ScopedHandle event_handle);

  WaitableEvent(const WaitableEvent&) = delete;
  WaitableEvent& operator=(const WaitableEvent&) = delete;

  ~WaitableEvent();

  // Put the event in the un-signaled state.
  void Reset();

  // Put the event in the signaled state.  Causing any thread blocked on Wait
  // to be woken up.
  void Signal();

  // Returns true if the event is in the signaled state, else false.  If this
  // is not a manual reset event, then this test will cause a reset.
  bool IsSignaled();

  // Wait indefinitely for the event to be signaled. Wait's return "happens
  // after" |Signal| has completed. This means that it's safe for a
  // WaitableEvent to synchronise its own destruction, like this:
  //
  //   WaitableEvent *e = new WaitableEvent;
  //   SendToOtherThread(e);
  //   e->Wait();
  //   delete e;
  void Wait();

  // Wait up until wait_delta has passed for the event to be signaled.  Returns
  // true if the event was signaled.
  //
  // TimedWait can synchronise its own destruction like |Wait|.
  bool TimedWait(const TimeDelta& wait_delta);

  // Wait up until end_time deadline has passed for the event to be signaled.
  // Return true if the event was signaled.
  //
  // TimedWaitUntil can synchronise its own destruction like |Wait|.
  bool TimedWaitUntil(const TimeTicks& end_time);

  HANDLE handle() const { return handle_.Get(); }

  // Wait, synchronously, on multiple events.
  //   waitables: an array of WaitableEvent pointers
  //   count: the number of elements in @waitables
  //
  // returns: the index of a WaitableEvent which has been signaled.
  //
  // You MUST NOT delete any of the WaitableEvent objects while this wait is
  // happening, however WaitMany's return "happens after" the |Signal| call
  // that caused it has completed, like |Wait|.
  //
  // If more than one WaitableEvent is signaled to unblock WaitMany, the lowest
  // index among them is returned.
  static size_t WaitMany(WaitableEvent** waitables, size_t count);

  // For asynchronous waiting, see WaitableEventWatcher

  // This is a private helper class. It's here because it's used by friends of
  // this class (such as WaitableEventWatcher) to be able to enqueue elements
  // of the wait-list
  class Waiter {
   public:
    // Signal the waiter to wake up.
    //
    // Consider the case of a Waiter which is in multiple WaitableEvent's
    // wait-lists. Each WaitableEvent is automatic-reset and two of them are
    // signaled at the same time. Now, each will wake only the first waiter in
    // the wake-list before resetting. However, if those two waiters happen to
    // be the same object (as can happen if another thread didn't have a chance
    // to dequeue the waiter from the other wait-list in time), two auto-resets
    // will have happened, but only one waiter has been signaled!
    //
    // Because of this, a Waiter may "reject" a wake by returning false. In
    // this case, the auto-reset WaitableEvent shouldn't act as if anything has
    // been notified.
    virtual bool Fire(WaitableEvent* signaling_event) = 0;

    // Waiters may implement this in order to provide an extra condition for
    // two Waiters to be considered equal. In WaitableEvent::Dequeue, if the
    // pointers match then this function is called as a final check. See the
    // comments in ~Handle for why.
    virtual bool Compare(void* tag) = 0;

   protected:
    virtual ~Waiter() = default;
  };

 private:
  friend class WaitableEventWatcher;

  win::ScopedHandle handle_;
};

}  // namespace winbase

#endif  // WINLIB_WINBASE_SYNCHRONIZATION_WAITABLE_EVENT_H_