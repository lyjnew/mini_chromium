// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_MESSAGE_LOOP_MESSAGE_PUMP_FOR_IO_H_
#define WINLIB_WINBASE_MESSAGE_LOOP_MESSAGE_PUMP_FOR_IO_H_

// This header is a forwarding header to coalesce the various platform specific
// types representing MessagePumpForIO.

#include "winbase\message_loop\message_pump_win.h"

namespace winbase {

// Windows defines it as-is.
using MessagePumpForIO = MessagePumpForIO;

}  // namespace winbase

#endif  // WINLIB_WINBASE_MESSAGE_LOOP_MESSAGE_PUMP_FOR_IO_H_