// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/tracking_info.h"

#include <stddef.h>
#include "crbase/tracked_objects.h"

namespace crbase {

TrackingInfo::TrackingInfo()
    : birth_tally(NULL) {
}

TrackingInfo::TrackingInfo(
    const crtracked_objects::Location& posted_from,
    crbase::TimeTicks delayed_run_time)
    : birth_tally(
          crtracked_objects::ThreadData::TallyABirthIfActive(posted_from)),
      time_posted(crtracked_objects::ThreadData::Now()),
      delayed_run_time(delayed_run_time) {
}

TrackingInfo::~TrackingInfo() {}

}  // namespace crbase
