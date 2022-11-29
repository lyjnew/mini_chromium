// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_MESSAGE_LOOP_MESSAGE_PUMP_DEFAULT_H_
#define WINLIB_WINBASE_MESSAGE_LOOP_MESSAGE_PUMP_DEFAULT_H_

#include "winbase\base_export.h"
#include "winbase\macros.h"
#include "winbase\message_loop\message_pump.h"
#include "winbase\synchronization\waitable_event.h"
#include "winbase\time\time.h"

namespace winbase {

class WINBASE_EXPORT MessagePumpDefault : public MessagePump {
 public:
  MessagePumpDefault();
  MessagePumpDefault(const MessagePumpDefault&) = delete;
  MessagePumpDefault& operator=(const MessagePumpDefault&) = delete;
  ~MessagePumpDefault() override;

  // MessagePump methods:
  void Run(Delegate* delegate) override;
  void Quit() override;
  void ScheduleWork() override;
  void ScheduleDelayedWork(const TimeTicks& delayed_work_time) override;

 private:
  // This flag is set to false when Run should return.
  bool keep_running_;

  // Used to sleep until there is more work to do.
  WaitableEvent event_;

  // The time at which we should call DoDelayedWork.
  TimeTicks delayed_work_time_;
};

}  // namespace base

#endif  // WINLIB_WINBASE_MESSAGE_LOOP_MESSAGE_PUMP_DEFAULT_H_