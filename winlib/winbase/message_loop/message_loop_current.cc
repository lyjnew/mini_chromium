// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "winbase\message_loop\message_loop_current.h"

#include "winbase\functional\bind.h"
#include "winbase\message_loop\message_loop.h"
#include "winbase\message_loop\message_pump_for_io.h"
#include "winbase\message_loop\message_pump_for_ui.h"
#include "winbase\no_destructor.h"
#include "winbase\threading\thread_local.h"

namespace winbase {

namespace {

winbase::ThreadLocalPointer<MessageLoop>* GetTLSMessageLoop() {
  static NoDestructor<ThreadLocalPointer<MessageLoop>> lazy_tls_ptr;
  return lazy_tls_ptr.get();
}

}  // namespace

//------------------------------------------------------------------------------
// MessageLoopCurrent

// static
MessageLoopCurrent MessageLoopCurrent::Get() {
  return MessageLoopCurrent(GetTLSMessageLoop()->Get());
}

// static
bool MessageLoopCurrent::IsSet() {
  return !!GetTLSMessageLoop()->Get();
}

void MessageLoopCurrent::AddDestructionObserver(
    DestructionObserver* destruction_observer) {
  WINBASE_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  current_->destruction_observers_.AddObserver(destruction_observer);
}

void MessageLoopCurrent::RemoveDestructionObserver(
    DestructionObserver* destruction_observer) {
  WINBASE_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  current_->destruction_observers_.RemoveObserver(destruction_observer);
}

const scoped_refptr<SingleThreadTaskRunner>& MessageLoopCurrent::task_runner()
    const {
  WINBASE_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  return current_->task_runner();
}

void MessageLoopCurrent::SetTaskRunner(
    scoped_refptr<SingleThreadTaskRunner> task_runner) {
  WINBASE_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  current_->SetTaskRunner(std::move(task_runner));
}

bool MessageLoopCurrent::IsIdleForTesting() {
  WINBASE_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  return current_->IsIdleForTesting();
}

void MessageLoopCurrent::AddTaskObserver(TaskObserver* task_observer) {
  WINBASE_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  current_->AddTaskObserver(task_observer);
}

void MessageLoopCurrent::RemoveTaskObserver(TaskObserver* task_observer) {
  WINBASE_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  current_->RemoveTaskObserver(task_observer);
}

void MessageLoopCurrent::SetNestableTasksAllowed(bool allowed) {
  WINBASE_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  if (allowed) {
    // Kick the native pump just in case we enter a OS-driven nested message
    // loop that does not go through RunLoop::Run().
    current_->pump_->ScheduleWork();
  }
  current_->task_execution_allowed_ = allowed;
}

bool MessageLoopCurrent::NestableTasksAllowed() const {
  WINBASE_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  return current_->task_execution_allowed_;
}

MessageLoopCurrent::ScopedNestableTaskAllower::ScopedNestableTaskAllower()
    : loop_(GetTLSMessageLoop()->Get()),
      old_state_(loop_->NestableTasksAllowed()) {
  loop_->SetNestableTasksAllowed(true);
}

MessageLoopCurrent::ScopedNestableTaskAllower::~ScopedNestableTaskAllower() {
  loop_->SetNestableTasksAllowed(old_state_);
}

// static
void MessageLoopCurrent::BindToCurrentThreadInternal(MessageLoop* current) {
  WINBASE_DCHECK(!GetTLSMessageLoop()->Get())
      << "Can't register a second MessageLoop on the same thread.";
  GetTLSMessageLoop()->Set(current);
}

// static
void MessageLoopCurrent::UnbindFromCurrentThreadInternal(MessageLoop* current) {
  WINBASE_DCHECK_EQ(current, GetTLSMessageLoop()->Get());
  GetTLSMessageLoop()->Set(nullptr);
}

bool MessageLoopCurrent::IsBoundToCurrentThreadInternal(
    MessageLoop* message_loop) {
  return GetTLSMessageLoop()->Get() == message_loop;
}

//------------------------------------------------------------------------------
// MessageLoopCurrentForUI

// static
MessageLoopCurrentForUI MessageLoopCurrentForUI::Get() {
  MessageLoop* loop = GetTLSMessageLoop()->Get();
  WINBASE_DCHECK(loop);
  WINBASE_DCHECK(loop->IsType(MessageLoop::TYPE_UI));
  auto* loop_for_ui = static_cast<MessageLoopForUI*>(loop);
  return MessageLoopCurrentForUI(
      loop_for_ui, static_cast<MessagePumpForUI*>(loop_for_ui->pump_.get()));
}

// static
bool MessageLoopCurrentForUI::IsSet() {
  MessageLoop* loop = GetTLSMessageLoop()->Get();
  return loop && loop->IsType(MessageLoop::TYPE_UI);
}

//------------------------------------------------------------------------------
// MessageLoopCurrentForIO

// static
MessageLoopCurrentForIO MessageLoopCurrentForIO::Get() {
  MessageLoop* loop = GetTLSMessageLoop()->Get();
  WINBASE_DCHECK(loop);
  WINBASE_DCHECK_EQ(MessageLoop::TYPE_IO, loop->type());
  auto* loop_for_io = static_cast<MessageLoopForIO*>(loop);
  return MessageLoopCurrentForIO(
      loop_for_io, static_cast<MessagePumpForIO*>(loop_for_io->pump_.get()));
}

// static
bool MessageLoopCurrentForIO::IsSet() {
  MessageLoop* loop = GetTLSMessageLoop()->Get();
  return loop && loop->IsType(MessageLoop::TYPE_IO);
}

HRESULT MessageLoopCurrentForIO::RegisterIOHandler(
    HANDLE file,
    MessagePumpForIO::IOHandler* handler) {
  WINBASE_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  return pump_->RegisterIOHandler(file, handler);
}

bool MessageLoopCurrentForIO::RegisterJobObject(
    HANDLE job,
    MessagePumpForIO::IOHandler* handler) {
  WINBASE_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  return pump_->RegisterJobObject(job, handler);
}

bool MessageLoopCurrentForIO::WaitForIOCompletion(
    DWORD timeout,
    MessagePumpForIO::IOHandler* filter) {
  WINBASE_DCHECK_CALLED_ON_VALID_THREAD(current_->bound_thread_checker_);
  return pump_->WaitForIOCompletion(timeout, filter);
}

}  // namespace winbase