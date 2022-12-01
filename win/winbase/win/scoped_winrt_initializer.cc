// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "winbase\win\scoped_winrt_initializer.h"

#include "winbase\logging.h"
#include "winbase\win\com_init_util.h"
#include "winbase\win\core_winrt_util.h"
#include "winbase\win\windows_version.h"

namespace winbase {
namespace win {

ScopedWinrtInitializer::ScopedWinrtInitializer()
    : hr_(winbase::win::RoInitialize(RO_INIT_MULTITHREADED)) {
  WINBASE_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  WINBASE_DCHECK_GE(GetVersion(), OSVersion::WIN8);
#if WINBASE_DCHECK_IS_ON()
  if (SUCCEEDED(hr_))
    AssertComApartmentType(ComApartmentType::MTA);
  else
    WINBASE_DCHECK_NE(RPC_E_CHANGED_MODE, hr_)
        << "Invalid COM thread model change";
#endif
}

ScopedWinrtInitializer::~ScopedWinrtInitializer() {
  WINBASE_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  if (SUCCEEDED(hr_))
    winbase::win::RoUninitialize();
}

bool ScopedWinrtInitializer::Succeeded() const {
  return SUCCEEDED(hr_);
}

}  // namespace win
}  // namespace winbase