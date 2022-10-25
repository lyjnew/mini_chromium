// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This is a simple struct with tracking information that is stored
// with a PendingTask (when message_loop is handling the task).
// Only the information that is shared with the profiler in tracked_objects
// are included in this structure.


#ifndef MINI_CHROMIUM_CRBASE_TRACKING_INFO_H_
#define MINI_CHROMIUM_CRBASE_TRACKING_INFO_H_

#include "crbase/base_export.h"
#include "crbase/profiler/tracked_time.h"
#include "crbase/time/time.h"

namespace crtracked_objects {
class Location;
class Births;
}

namespace crbase {

// This structure is copied around by value.
struct CRBASE_EXPORT TrackingInfo {
  TrackingInfo();
  TrackingInfo(const crtracked_objects::Location& posted_from,
               crbase::TimeTicks delayed_run_time);
  ~TrackingInfo();

  // To avoid conflating our stats with the delay duration in a PostDelayedTask,
  // we identify such tasks, and replace their post_time with the time they
  // were scheduled (requested?) to emerge from the delayed task queue. This
  // means that queuing delay for such tasks will show how long they went
  // unserviced, after they *could* be serviced.  This is the same stat as we
  // have for non-delayed tasks, and we consistently call it queuing delay.
  crtracked_objects::TrackedTime EffectiveTimePosted() const {
    return delayed_run_time.is_null()
               ? time_posted
               : crtracked_objects::TrackedTime(delayed_run_time);
  }

  // Record of location and thread that the task came from.
  crtracked_objects::Births* birth_tally;

  // Time when the related task was posted. Note that this value may be empty
  // if task profiling is disabled, and should only be used in conjunction with
  // profiling-related reporting.
  crtracked_objects::TrackedTime time_posted;

  // The time when the task should be run.
  crbase::TimeTicks delayed_run_time;
};

}  // namespace crbase

#endif  // MINI_CHROMIUM_CRBASE_TRACKING_INFO_H_