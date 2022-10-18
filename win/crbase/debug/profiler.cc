// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/debug/profiler.h"

#include <string>

///#include "crbase/debug/debugging_flags.h"
#include "crbase/process/process_handle.h"
#include "crbase/strings/string_number_conversions.h"
#include "crbase/strings/string_util.h"
#include "crbase/logging.h"
#include "crbuild/build_config.h"

#include "crbase/win/pe_image.h"

namespace crbase {
namespace debug {

void StartProfiling(const std::string& name) {
}

void StopProfiling() {
}

void FlushProfiling() {
}

bool BeingProfiled() {
  return false;
}

void RestartProfilingAfterFork() {
}

// http://blogs.msdn.com/oldnewthing/archive/2004/10/25/247180.aspx
extern "C" IMAGE_DOS_HEADER __ImageBase;

bool IsBinaryInstrumented() {
  enum InstrumentationCheckState {
    UNINITIALIZED,
    INSTRUMENTED_IMAGE,
    NON_INSTRUMENTED_IMAGE,
  };

  static InstrumentationCheckState state = UNINITIALIZED;

  if (state == UNINITIALIZED) {
    HMODULE this_module = reinterpret_cast<HMODULE>(&__ImageBase);
    crbase::win::PEImage image(this_module);

    // Check to be sure our image is structured as we'd expect.
    CR_DCHECK(image.VerifyMagic());

    // Syzygy-instrumented binaries contain a PE image section named ".thunks",
    // and all Syzygy-modified binaries contain the ".syzygy" image section.
    // This is a very fast check, as it only looks at the image header.
    if ((image.GetImageSectionHeaderByName(".thunks") != NULL) &&
        (image.GetImageSectionHeaderByName(".syzygy") != NULL)) {
      state = INSTRUMENTED_IMAGE;
    } else {
      state = NON_INSTRUMENTED_IMAGE;
    }
  }
  CR_DCHECK(state != UNINITIALIZED);

  return state == INSTRUMENTED_IMAGE;
}

namespace {

struct FunctionSearchContext {
  const char* name;
  FARPROC function;
};

// Callback function to PEImage::EnumImportChunks.
bool FindResolutionFunctionInImports(
    const crbase::win::PEImage &image, const char* module_name,
    PIMAGE_THUNK_DATA unused_name_table, PIMAGE_THUNK_DATA import_address_table,
    PVOID cookie) {
  FunctionSearchContext* context =
      reinterpret_cast<FunctionSearchContext*>(cookie);

  CR_DCHECK_NE(static_cast<FunctionSearchContext*>(NULL), context);
  CR_DCHECK_EQ(static_cast<FARPROC>(NULL), context->function);

  // Our import address table contains pointers to the functions we import
  // at this point. Let's retrieve the first such function and use it to
  // find the module this import was resolved to by the loader.
  const wchar_t* function_in_module =
      reinterpret_cast<const wchar_t*>(import_address_table->u1.Function);

  // Retrieve the module by a function in the module.
  const DWORD kFlags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                       GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
  HMODULE module = NULL;
  if (!::GetModuleHandleEx(kFlags, function_in_module, &module)) {
    // This can happen if someone IAT patches us to a thunk.
    return true;
  }

  // See whether this module exports the function we're looking for.
  FARPROC exported_func = ::GetProcAddress(module, context->name);
  if (exported_func != NULL) {
    // We found it, return the function and terminate the enumeration.
    context->function = exported_func;
    return false;
  }

  // Keep going.
  return true;
}

template <typename FunctionType>
FunctionType FindFunctionInImports(const char* function_name) {
  if (!IsBinaryInstrumented())
    return NULL;

  HMODULE this_module = reinterpret_cast<HMODULE>(&__ImageBase);
  crbase::win::PEImage image(this_module);

  FunctionSearchContext ctx = { function_name, NULL };
  image.EnumImportChunks(FindResolutionFunctionInImports, &ctx);

  return reinterpret_cast<FunctionType>(ctx.function);
}

}  // namespace

ReturnAddressLocationResolver GetProfilerReturnAddrResolutionFunc() {
  return FindFunctionInImports<ReturnAddressLocationResolver>(
      "ResolveReturnAddressLocation");
}

DynamicFunctionEntryHook GetProfilerDynamicFunctionEntryHookFunc() {
  return FindFunctionInImports<DynamicFunctionEntryHook>(
      "OnDynamicFunctionEntry");
}

AddDynamicSymbol GetProfilerAddDynamicSymbolFunc() {
  return FindFunctionInImports<AddDynamicSymbol>(
      "AddDynamicSymbol");
}

MoveDynamicSymbol GetProfilerMoveDynamicSymbolFunc() {
  return FindFunctionInImports<MoveDynamicSymbol>(
      "MoveDynamicSymbol");
}

}  // namespace debug
}  // namespace crbase