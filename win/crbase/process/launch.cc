// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/process/launch.h"
#include "crbuild/build_config.h"

namespace crbase {

LaunchOptions::LaunchOptions()
    : wait(false),
      start_hidden(false),
      handles_to_inherit(NULL),
      inherit_handles(false),
      as_user(NULL),
      empty_desktop_name(false),
      job_handle(NULL),
      stdin_handle(NULL),
      stdout_handle(NULL),
      stderr_handle(NULL),
      force_breakaway_from_job_(false)
    {
}

LaunchOptions::~LaunchOptions() {
}

}  // namespace crbase