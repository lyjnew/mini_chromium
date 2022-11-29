// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "winbase\process\launch.h"

namespace winbase {

LaunchOptions::LaunchOptions() = default;

LaunchOptions::LaunchOptions(const LaunchOptions& other) = default;

LaunchOptions::~LaunchOptions() = default;

}  // namespace winbase