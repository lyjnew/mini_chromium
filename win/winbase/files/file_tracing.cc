// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "winbase\files\file_tracing.h"

#include "winbase\atomic\atomicops.h"
#include "winbase\files\file.h"

using winbase::subtle::AtomicWord;

namespace winbase {

namespace {
AtomicWord g_provider;
}

FileTracing::Provider* GetProvider() {
  AtomicWord provider = winbase::subtle::Acquire_Load(&g_provider);
  return reinterpret_cast<FileTracing::Provider*>(provider);
}

// static
bool FileTracing::IsCategoryEnabled() {
  FileTracing::Provider* provider = GetProvider();
  return provider && provider->FileTracingCategoryIsEnabled();
}

// static
void FileTracing::SetProvider(FileTracing::Provider* provider) {
  winbase::subtle::Release_Store(
      &g_provider, reinterpret_cast<AtomicWord>(provider));
}

FileTracing::ScopedEnabler::ScopedEnabler() {
  FileTracing::Provider* provider = GetProvider();
  if (provider)
    provider->FileTracingEnable(this);
}

FileTracing::ScopedEnabler::~ScopedEnabler() {
  FileTracing::Provider* provider = GetProvider();
  if (provider)
    provider->FileTracingDisable(this);
}

FileTracing::ScopedTrace::ScopedTrace() : id_(nullptr), name_(nullptr) {}

FileTracing::ScopedTrace::~ScopedTrace() {
  if (id_) {
    FileTracing::Provider* provider = GetProvider();
    if (provider)
      provider->FileTracingEventEnd(name_, id_);
  }
}

void FileTracing::ScopedTrace::Initialize(const char* name,
                                          const File* file,
                                          int64_t size) {
  id_ = &file->trace_enabler_;
  name_ = name;
  GetProvider()->FileTracingEventBegin(name_, id_, file->tracing_path_, size);
}

}  // namespace winbase