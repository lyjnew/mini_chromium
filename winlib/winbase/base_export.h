// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_BASE_EXPORT_H_
#define WINLIB_WINBASE_BASE_EXPORT_H_

#if defined(WINLIB_COMPONENT_BUILD)
#if defined(_MSC_VER)

#if defined(WINBASE_IMPLEMENTATION)
#define WINBASE_EXPORT __declspec(dllexport)
#else
#define WINBASE_EXPORT __declspec(dllimport)
#endif  // defined(WINBASE_IMPLEMENTATION)

#else  // defined(_MSC_VER)

#if defined(WINBASE_IMPLEMENTATION)
#define WINBASE_EXPORT __attribute__((visibility("default")))
#else
#define WINBASE_EXPORT
#endif  // defined(WINBASE_IMPLEMENTATION)

#endif

#else  // defined(WINLIB_COMPONENT_BUILD)
#define WINBASE_EXPORT
#endif

#endif  // WINLIB_WINBASE_BASE_EXPORT_H_