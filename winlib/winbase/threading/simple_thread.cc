// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "winbase\threading\simple_thread.h"

#include "winbase\logging.h"
#include "winbase\strings\string_number_conversions.h"
#include "winbase\threading\platform_thread.h"
#include "winbase\threading\thread_restrictions.h"

namespace winbase {

SimpleThread::SimpleThread(const std::string& name_prefix)
    : SimpleThread(name_prefix, Options()) {}

SimpleThread::SimpleThread(const std::string& name_prefix,
                           const Options& options)
    : name_prefix_(name_prefix),
      options_(options),
      event_(WaitableEvent::ResetPolicy::MANUAL,
             WaitableEvent::InitialState::NOT_SIGNALED) {}

SimpleThread::~SimpleThread() {
  WINBASE_DCHECK(HasBeenStarted()) << "SimpleThread was never started.";
  WINBASE_DCHECK(!options_.joinable || HasBeenJoined())
      << "Joinable SimpleThread destroyed without being Join()ed.";
}

void SimpleThread::Start() {
  StartAsync();
  ThreadRestrictions::ScopedAllowWait allow_wait;
  event_.Wait();  // Wait for the thread to complete initialization.
}

void SimpleThread::Join() {
  WINBASE_DCHECK(options_.joinable) << "A non-joinable thread can't be joined.";
  WINBASE_DCHECK(HasStartBeenAttempted())
      << "Tried to Join a never-started thread.";
  WINBASE_DCHECK(!HasBeenJoined()) << "Tried to Join a thread multiple times.";
  BeforeJoin();
  PlatformThread::Join(thread_);
  thread_ = PlatformThreadHandle();
  joined_ = true;
}

void SimpleThread::StartAsync() {
  WINBASE_DCHECK(!HasStartBeenAttempted())
      << "Tried to Start a thread multiple times.";
  start_called_ = true;
  BeforeStart();
  bool success =
      options_.joinable
          ? PlatformThread::CreateWithPriority(options_.stack_size, this,
                                               &thread_, options_.priority)
          : PlatformThread::CreateNonJoinableWithPriority(
                options_.stack_size, this, options_.priority);
  WINBASE_CHECK(success);
}

PlatformThreadId SimpleThread::tid() {
  WINBASE_DCHECK(HasBeenStarted());
  return tid_;
}

bool SimpleThread::HasBeenStarted() {
  ThreadRestrictions::ScopedAllowWait allow_wait;
  return event_.IsSignaled();
}

void SimpleThread::ThreadMain() {
  tid_ = PlatformThread::CurrentId();
  // Construct our full name of the form "name_prefix_/TID".
  std::string name(name_prefix_);
  name.push_back('/');
  name.append(IntToString(tid_));
  PlatformThread::SetName(name);

  // We've initialized our new thread, signal that we're done to Start().
  event_.Signal();

  BeforeRun();
  Run();
}

DelegateSimpleThread::DelegateSimpleThread(Delegate* delegate,
                                           const std::string& name_prefix)
    : DelegateSimpleThread(delegate, name_prefix, Options()) {}

DelegateSimpleThread::DelegateSimpleThread(Delegate* delegate,
                                           const std::string& name_prefix,
                                           const Options& options)
    : SimpleThread(name_prefix, options),
      delegate_(delegate) {
  WINBASE_DCHECK(delegate_);
}

DelegateSimpleThread::~DelegateSimpleThread() = default;

void DelegateSimpleThread::Run() {
  WINBASE_DCHECK(delegate_)
      << "Tried to call Run without a delegate (called twice?)";

  // Non-joinable DelegateSimpleThreads are allowed to be deleted during Run().
  // Member state must not be accessed after invoking Run().
  Delegate* delegate = delegate_;
  delegate_ = nullptr;
  delegate->Run();
}

DelegateSimpleThreadPool::DelegateSimpleThreadPool(
    const std::string& name_prefix,
    int num_threads)
    : name_prefix_(name_prefix),
      num_threads_(num_threads),
      dry_(WaitableEvent::ResetPolicy::MANUAL,
           WaitableEvent::InitialState::NOT_SIGNALED) {}

DelegateSimpleThreadPool::~DelegateSimpleThreadPool() {
  WINBASE_DCHECK(threads_.empty());
  WINBASE_DCHECK(delegates_.empty());
  WINBASE_DCHECK(!dry_.IsSignaled());
}

void DelegateSimpleThreadPool::Start() {
  WINBASE_DCHECK(threads_.empty())
      << "Start() called with outstanding threads.";
  for (int i = 0; i < num_threads_; ++i) {
    DelegateSimpleThread* thread = new DelegateSimpleThread(this, name_prefix_);
    thread->Start();
    threads_.push_back(thread);
  }
}

void DelegateSimpleThreadPool::JoinAll() {
  WINBASE_DCHECK(!threads_.empty())
      << "JoinAll() called with no outstanding threads.";

  // Tell all our threads to quit their worker loop.
  AddWork(nullptr, num_threads_);

  // Join and destroy all the worker threads.
  for (int i = 0; i < num_threads_; ++i) {
    threads_[i]->Join();
    delete threads_[i];
  }
  threads_.clear();
  WINBASE_DCHECK(delegates_.empty());
}

void DelegateSimpleThreadPool::AddWork(Delegate* delegate, int repeat_count) {
  AutoLock locked(lock_);
  for (int i = 0; i < repeat_count; ++i)
    delegates_.push(delegate);
  // If we were empty, signal that we have work now.
  if (!dry_.IsSignaled())
    dry_.Signal();
}

void DelegateSimpleThreadPool::Run() {
  Delegate* work = nullptr;

  while (true) {
    dry_.Wait();
    {
      AutoLock locked(lock_);
      if (!dry_.IsSignaled())
        continue;

      WINBASE_DCHECK(!delegates_.empty());
      work = delegates_.front();
      delegates_.pop();

      // Signal to any other threads that we're currently out of work.
      if (delegates_.empty())
        dry_.Reset();
    }

    // A NULL delegate pointer signals us to quit.
    if (!work)
      break;

    work->Run();
  }
}

}  // namespace winbase