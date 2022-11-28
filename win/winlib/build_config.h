// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINLIB_BUILD_CONFIG_H_
#define WINLIB_WINLIB_BUILD_CONFIG_H_

#ifndef BUILD_BUILD_CONFIG_H_
#define BUILD_BUILD_CONFIG_H_

// compiler
#if defined(__GUNC__)
#define COMPILER_GCC  1
#elif defined(_MSC_VER)
#define COMPILER_MSVC 1
#else
#error Please add support for your compipler in winlib\build.h
#endif

// arch
#if defined(_M_X64) || defined(__x86_64__)
#define ARCH_CPU_X86_FAMILY 1
#define ARCH_CPU_X86_64 1
#define ARCH_CPU_64_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(_M_IX86) || defined(__i386__)
#define ARCH_CPU_X86_FAMILY 1
#define ARCH_CPU_X86 1
#define ARCH_CPU_32_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#else
#error Please add support for your architecture in winlib\build.h
#endif

#endif  // BUILD_BUILD_CONFIG_H_

#endif  // WINLIB_WINLIB_BUILD_CONFIG_H_