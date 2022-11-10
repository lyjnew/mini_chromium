// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_CRBASE_BASE_EXPORT_H_
#define MINI_CHROMIUM_CRBASE_BASE_EXPORT_H_

#if defined(MINI_CHROMIUM_COMPONENT_BUILD)
#if defined(WIN32)

#if defined(CRBASE_IMPLEMENTATION)
#define CRBASE_EXPORT __declspec(dllexport)
#else
#define CRBASE_EXPORT __declspec(dllimport)
#endif  // defined(CRBASE_IMPLEMENTATION)

#else  // defined(WIN32)
#if defined(CRBASE_IMPLEMENTATION)
#define CRBASE_EXPORT __attribute__((visibility("default")))
#else
#define CRBASE_EXPORT
#endif  // defined(CRBASE_IMPLEMENTATION)
#endif

#else  // defined(CRBASE_IMPLEMENTATION)
#define CRBASE_EXPORT
#endif

#endif  // MINI_CHROMIUM_CRBASE_BASE_EXPORT_H_