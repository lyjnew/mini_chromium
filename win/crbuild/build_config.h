// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file adds defines about the platform we're currently building on.
//  Operating System:
//    WINDOWS 
//  Compiler:
//    MINI_CHROMIUM_COMPILER_MSVC / MINI_CHROMIUM_COMPILER_GCC
//  Processor:
//    MINI_CHROMIUM_ARCH_CPU_X86 / MINI_CHROMIUM_ARCH_CPU_X86_64 / 
//    MINI_CHROMIUM_ARCH_CPU_X86_FAMILY (X86 or X86_64)
//    MINI_CHROMIUM_ARCH_CPU_32_BITS / MINI_CHROMIUM_ARCH_CPU_64_BITS

#ifndef MINI_CHROMIUM_BUILD_CRBUILD_CONFIG_H_
#define MINI_CHROMIUM_BUILD_CRBUILD_CONFIG_H_

// Compiler detection.
#if defined(__GNUC__)
#define MINI_CHROMIUM_COMPILER_GCC 1
#elif defined(_MSC_VER)
#define MINI_CHROMIUM_COMPILER_MSVC 1
#else
#error Please add support for your compiler in crbuild/crbuild_config.h
#endif

// Processor architecture detection.  For more info on what's defined, see:
//   http://msdn.microsoft.com/en-us/library/b0084kay.aspx
//   http://www.agner.org/optimize/calling_conventions.pdf
//   or with gcc, run: "echo | gcc -E -dM -"
#if defined(_M_X64) || defined(__x86_64__)
#define MINI_CHROMIUM_ARCH_CPU_X86_FAMILY 1
#define MINI_CHROMIUM_ARCH_CPU_X86_64 1
#define MINI_CHROMIUM_ARCH_CPU_64_BITS 1
#define MINI_CHROMIUM_ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(_M_IX86) || defined(__i386__)
#define MINI_CHROMIUM_ARCH_CPU_X86_FAMILY 1
#define MINI_CHROMIUM_ARCH_CPU_X86 1
#define MINI_CHROMIUM_ARCH_CPU_32_BITS 1
#define MINI_CHROMIUM_ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(__ARMEL__)
#define MINI_CHROMIUM_ARCH_CPU_ARM_FAMILY 1
#define MINI_CHROMIUM_ARCH_CPU_ARMEL 1
#define MINI_CHROMIUM_ARCH_CPU_32_BITS 1
#define MINI_CHROMIUM_ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(__aarch64__)
#define MINI_CHROMIUM_ARCH_CPU_ARM_FAMILY 1
#define MINI_CHROMIUM_ARCH_CPU_ARM64 1
#define MINI_CHROMIUM_ARCH_CPU_64_BITS 1
#define MINI_CHROMIUM_ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(__pnacl__)
#define MINI_CHROMIUM_ARCH_CPU_32_BITS 1
#define MINI_CHROMIUM_ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(__MIPSEL__)
#if defined(__LP64__)
#define MINI_CHROMIUM_ARCH_CPU_MIPS64_FAMILY 1
#define MINI_CHROMIUM_ARCH_CPU_MIPS64EL 1
#define MINI_CHROMIUM_ARCH_CPU_64_BITS 1
#define MINI_CHROMIUM_ARCH_CPU_LITTLE_ENDIAN 1
#else
#define MINI_CHROMIUM_ARCH_CPU_MIPS_FAMILY 1
#define MINI_CHROMIUM_ARCH_CPU_MIPSEL 1
#define MINI_CHROMIUM_ARCH_CPU_32_BITS 1
#define MINI_CHROMIUM_ARCH_CPU_LITTLE_ENDIAN 1
#endif
#else
#error Please add support for your architecture in crbuild/crbuild_config.h
#endif

#endif  // MINI_CHROMIUM_BUILD_CRBUILD_CONFIG_H_