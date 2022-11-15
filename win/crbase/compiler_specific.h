// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_CRBASE_COMPILER_SPECIFIC_H_
#define MINI_CHROMIUM_CRBASE_COMPILER_SPECIFIC_H_

#include "crbuild/build_config.h"

#if defined(MINI_CHROMIUM_COMPILER_MSVC)

// For _Printf_format_string_.
#include <sal.h>

// Macros for suppressing and disabling warnings on MSVC.
//
// Warning numbers are enumerated at:
// http://msdn.microsoft.com/en-us/library/8x5x43k7(VS.80).aspx
//
// The warning pragma:
// http://msdn.microsoft.com/en-us/library/2c8f766e(VS.80).aspx
//
// Using __pragma instead of #pragma inside macros:
// http://msdn.microsoft.com/en-us/library/d9x1s805.aspx

// CR_MSVC_SUPPRESS_WARNING disables warning |n| for the remainder of the line 
// and for the next line of the source file.
#define CR_MSVC_SUPPRESS_WARNING(n) __pragma(warning(suppress:n))

// MSVC_PUSH_DISABLE_WARNING pushes |n| onto a stack of warnings to be disabled.
// The warning remains disabled until popped by MSVC_POP_WARNING.
#define CR_MSVC_PUSH_DISABLE_WARNING(n) __pragma(warning(push)) \
                                        __pragma(warning(disable:n))

// MSVC_PUSH_WARNING_LEVEL pushes |n| as the global warning level.  The level
// remains in effect until popped by MSVC_POP_WARNING().  Use 0 to disable all
// warnings.
#define CR_MSVC_PUSH_WARNING_LEVEL(n) __pragma(warning(push, n))

// Pop effects of innermost MSVC_PUSH_* macro.
#define CR_MSVC_POP_WARNING() __pragma(warning(pop))

#define CR_MSVC_DISABLE_OPTIMIZE() __pragma(optimize("", off))
#define CR_MSVC_ENABLE_OPTIMIZE() __pragma(optimize("", on))

// Allows exporting a class that inherits from a non-exported base class.
// This uses suppress instead of push/pop because the delimiter after the
// declaration (either "," or "{") has to be placed before the pop macro.
//
// Example usage:
// class EXPORT_API Foo : CR_NON_EXPORTED_BASE(public Bar) {
//
// MSVC Compiler warning C4275:
// non dll-interface class 'Bar' used as base for dll-interface class 'Foo'.
// Note that this is intended to be used only when no access to the base class'
// static data is done through derived classes or inline methods. For more info,
// see http://msdn.microsoft.com/en-us/library/3tdb471s(VS.80).aspx
#define CR_NON_EXPORTED_BASE(code) CR_MSVC_SUPPRESS_WARNING(4275) \
                                   code

#else  // Not MSVC

#define _Printf_format_string_
#define CR_MSVC_SUPPRESS_WARNING(n)
#define CR_MSVC_PUSH_DISABLE_WARNING(n)
#define CR_MSVC_PUSH_WARNING_LEVEL(n)
#define CR_MSVC_POP_WARNING()
#define CR_MSVC_DISABLE_OPTIMIZE()
#define CR_MSVC_ENABLE_OPTIMIZE()
#define CR_NON_EXPORTED_BASE(code) code

#endif  // COMPILER_MSVC


// Annotate a variable indicating it's ok if the variable is not used.
// (Typically used to silence a compiler warning when the assignment
// is important for some other reason.)
// Use like:
//   int x = ...;
//   CR_ALLOW_UNUSED_LOCAL(x);
#define CR_ALLOW_UNUSED_LOCAL(x) false ? (void)x : (void)0

// Annotate a typedef or function indicating it's ok if it's not used.
// Use like:
//   typedef Foo Bar ALLOW_UNUSED_TYPE;
#if defined(MINI_CHROMIUM_COMPILER_GCC) || defined(__clang__)
#define CR_ALLOW_UNUSED_TYPE __attribute__((unused))
#else
#define CR_ALLOW_UNUSED_TYPE
#endif

// Annotate a function indicating it should not be inlined.
// Use like:
//   NOINLINE void DoStuff() { ... }
#if defined(MINI_CHROMIUM_COMPILER_GCC)
#define CR_NOINLINE __attribute__((noinline))
#elif defined(MINI_CHROMIUM_COMPILER_MSVC)
#define CR_NOINLINE __declspec(noinline)
#else
#define CR_NOINLINE
#endif

// Specify memory alignment for structs, classes, etc.
// Use like:
//   class CR_ALIGNAS(16) MyClass { ... }
//   CR_ALIGNAS(16) int array[4];
#if defined(MINI_CHROMIUM_COMPILER_MSVC)
#define CR_ALIGNAS(byte_alignment) __declspec(align(byte_alignment))
#elif defined(MINI_CHROMIUM_COMPILER_GCC)
#define CR_ALIGNAS(byte_alignment) __attribute__((aligned(byte_alignment)))
#endif

// Return the byte alignment of the given type (available at compile time).
// Use like:
//   ALIGNOF(int32_t)  // this would be 4
#if defined(MINI_CHROMIUM_COMPILER_MSVC)
#define CR_ALIGNOF(type) __alignof(type)
#elif defined(MINI_CHROMIUM_COMPILER_GCC)
#define CR_ALIGNOF(type) __alignof__(type)
#endif

// Annotate a function indicating the caller must examine the return value.
// Use like:
//   int foo() CR_WARN_UNUSED_RESULT;
// To explicitly ignore a result, see |ignore_result()| in base/macros.h.
#undef CR_WARN_UNUSED_RESULT
#if defined(MINI_CHROMIUM_COMPILER_GCC) || defined(__clang__)
#define CR_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
#define CR_WARN_UNUSED_RESULT
#endif

// Tell the compiler a function is using a printf-style format string.
// |format_param| is the one-based index of the format string parameter;
// |dots_param| is the one-based index of the "..." parameter.
// For v*printf functions (which take a va_list), pass 0 for dots_param.
// (This is undocumented but matches what the system C headers do.)
#if defined(MINI_CHROMIUM_COMPILER_GCC)
#define CR_PRINTF_FORMAT(format_param, dots_param) \
    __attribute__((format(printf, format_param, dots_param)))
#else
#define CR_PRINTF_FORMAT(format_param, dots_param)
#endif

// CR_WPRINTF_FORMAT is the same, but for wide format strings.
// This doesn't appear to yet be implemented in any compiler.
// See http://gcc.gnu.org/bugzilla/show_bug.cgi?id=38308 .
#define CR_WPRINTF_FORMAT(format_param, dots_param)
// If available, it would look like:
//   __attribute__((format(wprintf, format_param, dots_param)))

// MemorySanitizer annotations.
#if defined(CR_MEMORY_SANITIZER)
#include <sanitizer/msan_interface.h>

// Mark a memory region fully initialized.
// Use this to annotate code that deliberately reads uninitialized data, for
// example a GC scavenging root set pointers from the stack.
#define CR_MSAN_UNPOISON(p, size)  __msan_unpoison(p, size)

// Check a memory region for initializedness, as if it was being used here.
// If any bits are uninitialized, crash with an MSan report.
// Use this to sanitize data which MSan won't be able to track, e.g. before
// passing data to another process via shared memory.
#define CR_MSAN_CHECK_MEM_IS_INITIALIZED(p, size) \
    __msan_check_mem_is_initialized(p, size)
#else  // CR_MEMORY_SANITIZER
#define CR_MSAN_UNPOISON(p, size)
#define CR_MSAN_CHECK_MEM_IS_INITIALIZED(p, size)
#endif  // CR_MEMORY_SANITIZER

// Macro useful for writing cross-platform function pointers.
#if !defined(CR_CDECL)
#define CR_CDECL __cdecl
#endif  // !defined(CDECL)

// Macro for hinting that an expression is likely to be false.
#if !defined(CR_UNLIKELY)
#if defined(MINI_CHROMIUM_COMPILER_GCC)
#define CR_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define CR_UNLIKELY(x) (x)
#endif  // defined(MINI_CHROMIUM_COMPILER_GCC)
#endif  // !defined(CR_UNLIKELY)

#endif  // MINI_CHROMIUM_CRBASE_COMPILER_SPECIFIC_H_