// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "winbase\sequence_token.h"

#include "winbase\atomic\atomic_sequence_num.h"
#include "winbase\lazy_instance.h"
#include "winbase\logging.h"
#include "winbase\threading\thread_local.h"

namespace winbase {

namespace {

winbase::AtomicSequenceNumber g_sequence_token_generator;

winbase::AtomicSequenceNumber g_task_token_generator;

LazyInstance<ThreadLocalPointer<const SequenceToken>>::Leaky
    tls_current_sequence_token = WINBASE_LAZY_INSTANCE_INITIALIZER;

LazyInstance<ThreadLocalPointer<const TaskToken>>::Leaky
    tls_current_task_token = WINBASE_LAZY_INSTANCE_INITIALIZER;

}  // namespace

bool SequenceToken::operator==(const SequenceToken& other) const {
  return token_ == other.token_ && IsValid();
}

bool SequenceToken::operator!=(const SequenceToken& other) const {
  return !(*this == other);
}

bool SequenceToken::IsValid() const {
  return token_ != kInvalidSequenceToken;
}

int SequenceToken::ToInternalValue() const {
  return token_;
}

SequenceToken SequenceToken::Create() {
  return SequenceToken(g_sequence_token_generator.GetNext());
}

SequenceToken SequenceToken::GetForCurrentThread() {
  const SequenceToken* current_sequence_token =
      tls_current_sequence_token.Get().Get();
  return current_sequence_token ? *current_sequence_token : SequenceToken();
}

bool TaskToken::operator==(const TaskToken& other) const {
  return token_ == other.token_ && IsValid();
}

bool TaskToken::operator!=(const TaskToken& other) const {
  return !(*this == other);
}

bool TaskToken::IsValid() const {
  return token_ != kInvalidTaskToken;
}

TaskToken TaskToken::Create() {
  return TaskToken(g_task_token_generator.GetNext());
}

TaskToken TaskToken::GetForCurrentThread() {
  const TaskToken* current_task_token = tls_current_task_token.Get().Get();
  return current_task_token ? *current_task_token : TaskToken();
}

ScopedSetSequenceTokenForCurrentThread::ScopedSetSequenceTokenForCurrentThread(
    const SequenceToken& sequence_token)
    : sequence_token_(sequence_token), task_token_(TaskToken::Create()) {
  WINBASE_DCHECK(!tls_current_sequence_token.Get().Get());
  WINBASE_DCHECK(!tls_current_task_token.Get().Get());
  tls_current_sequence_token.Get().Set(&sequence_token_);
  tls_current_task_token.Get().Set(&task_token_);
}

ScopedSetSequenceTokenForCurrentThread::
    ~ScopedSetSequenceTokenForCurrentThread() {
  WINBASE_DCHECK_EQ(tls_current_sequence_token.Get().Get(), &sequence_token_);
  WINBASE_DCHECK_EQ(tls_current_task_token.Get().Get(), &task_token_);
  tls_current_sequence_token.Get().Set(nullptr);
  tls_current_task_token.Get().Set(nullptr);
}

}  // namespace winbase