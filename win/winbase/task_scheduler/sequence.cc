// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "winbase\task_scheduler\sequence.h"

#include <utility>

#include "winbase\logging.h"
#include "winbase\time\time.h"

namespace winbase {
namespace internal {

Sequence::Sequence() = default;

bool Sequence::PushTask(Task task) {
  // Use CHECK instead of DCHECK to crash earlier. See http://crbug.com/711167
  // for details.
  WINBASE_CHECK(task.task);
  WINBASE_DCHECK(task.sequenced_time.is_null());
  task.sequenced_time = winbase::TimeTicks::Now();

  AutoSchedulerLock auto_lock(lock_);
  ++num_tasks_per_priority_[static_cast<int>(task.traits.priority())];
  queue_.push(std::move(task));

  // Return true if the sequence was empty before the push.
  return queue_.size() == 1;
}

Optional<Task> Sequence::TakeTask() {
  AutoSchedulerLock auto_lock(lock_);
  WINBASE_DCHECK(!queue_.empty());
  WINBASE_DCHECK(queue_.front().task);

  const int priority_index = static_cast<int>(queue_.front().traits.priority());
  WINBASE_DCHECK_GT(num_tasks_per_priority_[priority_index], 0U);
  --num_tasks_per_priority_[priority_index];

  return std::move(queue_.front());
}

bool Sequence::Pop() {
  AutoSchedulerLock auto_lock(lock_);
  WINBASE_DCHECK(!queue_.empty());
  WINBASE_DCHECK(!queue_.front().task);
  queue_.pop();
  return queue_.empty();
}

SequenceSortKey Sequence::GetSortKey() const {
  TaskPriority priority = TaskPriority::LOWEST;
  winbase::TimeTicks next_task_sequenced_time;

  {
    AutoSchedulerLock auto_lock(lock_);
    WINBASE_DCHECK(!queue_.empty());

    // Find the highest task priority in the sequence.
    const int highest_priority_index = static_cast<int>(TaskPriority::HIGHEST);
    const int lowest_priority_index = static_cast<int>(TaskPriority::LOWEST);
    for (int i = highest_priority_index; i > lowest_priority_index; --i) {
      if (num_tasks_per_priority_[i] > 0) {
        priority = static_cast<TaskPriority>(i);
        break;
      }
    }

    // Save the sequenced time of the next task in the sequence.
    next_task_sequenced_time = queue_.front().sequenced_time;
  }

  return SequenceSortKey(priority, next_task_sequenced_time);
}

Sequence::~Sequence() = default;

}  // namespace internal
}  // namespace winbase