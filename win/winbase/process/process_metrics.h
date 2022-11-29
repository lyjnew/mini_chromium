// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file contains routines for gathering resource statistics for processes
// running on the system.

#ifndef WINLIB_WINBASE_PROCESS_PROCESS_METRICS_H_
#define WINLIB_WINBASE_PROCESS_PROCESS_METRICS_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <string>

#include "winbase\base_export.h"
#include "winbase\macros.h"
#include "winbase\process\process_handle.h"
#include "winbase\time\time.h"
#include "winbase\values.h"

#include "winbase\win\scoped_handle.h"
#include "winbase\win\windows_types.h"

namespace winbase {

// Full declaration is in process_metrics_iocounters.h.
struct IoCounters;

// Convert a POSIX timeval to microseconds.
WINBASE_EXPORT int64_t TimeValToMicroseconds(const struct timeval& tv);

// Provides performance metrics for a specified process (CPU usage and IO
// counters). Use CreateCurrentProcessMetrics() to get an instance for the
// current process, or CreateProcessMetrics() to get an instance for an
// arbitrary process. Then, access the information with the different get
// methods.
//
// This class exposes a few platform-specific APIs for parsing memory usage, but
// these are not intended to generalize to other platforms, since the memory
// models differ substantially.
//
// To obtain consistent memory metrics, use the memory_instrumentation service.
//
// For further documentation on memory, see
// https://chromium.googlesource.com/chromium/src/+/HEAD/docs/README.md
class WINBASE_EXPORT ProcessMetrics {
 public:
  ProcessMetrics(const ProcessMetrics&) = delete;
  ProcessMetrics& operator=(const ProcessMetrics&) = delete;
  ~ProcessMetrics();

  // Creates a ProcessMetrics for the specified process.
  static std::unique_ptr<ProcessMetrics> CreateProcessMetrics(
      ProcessHandle process);

  // Creates a ProcessMetrics for the current process. This a cross-platform
  // convenience wrapper for CreateProcessMetrics().
  static std::unique_ptr<ProcessMetrics> CreateCurrentProcessMetrics();

  // Returns the percentage of time spent executing, across all threads of the
  // process, in the interval since the last time the method was called. Since
  // this considers the total execution time across all threads in a process,
  // the result can easily exceed 100% in multi-thread processes running on
  // multi-core systems. In general the result is therefore a value in the
  // range 0% to SysInfo::NumberOfProcessors() * 100%.
  //
  // To obtain the percentage of total available CPU resources consumed by this
  // process over the interval, the caller must divide by NumberOfProcessors().
  //
  // Since this API measures usage over an interval, it will return zero on the
  // first call, and an actual value only on the second and subsequent calls.
  double GetPlatformIndependentCPUUsage();

  // Returns the cumulative CPU usage across all threads of the process since
  // process start. In case of multi-core processors, a process can consume CPU
  // at a rate higher than wall-clock time, e.g. two cores at full utilization
  // will result in a time delta of 2 seconds/per 1 wall-clock second.
  TimeDelta GetCumulativeCPUUsage();

  // Returns the number of average idle cpu wakeups per second since the last
  // call.
  int GetIdleWakeupsPerSecond();

  // Retrieves accounting information for all I/O operations performed by the
  // process.
  // If IO information is retrieved successfully, the function returns true
  // and fills in the IO_COUNTERS passed in. The function returns false
  // otherwise.
  bool GetIOCounters(IoCounters* io_counters) const;

  // Returns total memory usage of malloc.
  size_t GetMallocUsage();

 private:
  explicit ProcessMetrics(ProcessHandle process);

  win::ScopedHandle process_;

  // Used to store the previous times and CPU usage counts so we can
  // compute the CPU usage between calls.
  TimeTicks last_cpu_time_;
  TimeDelta last_cumulative_cpu_;
};

// Returns the memory committed by the system in KBytes.
// Returns 0 if it can't compute the commit charge.
WINBASE_EXPORT size_t GetSystemCommitCharge();

// Returns the number of bytes in a memory page. Do not use this to compute
// the number of pages in a block of memory for calling mincore(). On some
// platforms, e.g. iOS, mincore() uses a different page size from what is
// returned by GetPageSize().
WINBASE_EXPORT size_t GetPageSize();

// Returns the maximum number of file descriptors that can be open by a process
// at once. If the number is unavailable, a conservative best guess is returned.
WINBASE_EXPORT size_t GetMaxFds();

// Data about system-wide memory consumption. Values are in KB. Available on
// Windows, Mac, Linux, Android and Chrome OS.
//
// Total memory are available on all platforms that implement
// GetSystemMemoryInfo(). Total/free swap memory are available on all platforms
// except on Mac. Buffers/cached/active_anon/inactive_anon/active_file/
// inactive_file/dirty/reclaimable/pswpin/pswpout/pgmajfault are available on
// Linux/Android/Chrome OS. Shmem/slab/gem_objects/gem_size are Chrome OS only.
// Speculative/file_backed/purgeable are Mac and iOS only.
// Free is absent on Windows (see "avail_phys" below).
struct WINBASE_EXPORT SystemMemoryInfoKB {
  SystemMemoryInfoKB();
  SystemMemoryInfoKB(const SystemMemoryInfoKB& other);

  // Serializes the platform specific fields to value.
  std::unique_ptr<DictionaryValue> ToValue() const;

  int total = 0;

  // "This is the amount of physical memory that can be immediately reused
  // without having to write its contents to disk first. It is the sum of the
  // size of the standby, free, and zero lists." (MSDN).
  // Standby: not modified pages of physical ram (file-backed memory) that are
  // not actively being used.
  int avail_phys = 0;

  int swap_total = 0;
  int swap_free = 0;
};

// On Linux/Android/Chrome OS, system-wide memory consumption data is parsed
// from /proc/meminfo and /proc/vmstat. On Windows/Mac, it is obtained using
// system API calls.
//
// Fills in the provided |meminfo| structure. Returns true on success.
// Exposed for memory debugging widget.
WINBASE_EXPORT bool GetSystemMemoryInfo(SystemMemoryInfoKB* meminfo);

// Collects and holds performance metrics for system memory and disk.
// Provides functionality to retrieve the data on various platforms and
// to serialize the stored data.
class SystemMetrics {
 public:
  SystemMetrics();

  static SystemMetrics Sample();

  // Serializes the system metrics to value.
  std::unique_ptr<Value> ToValue() const;

 private:

  size_t committed_memory_;
};

}  // namespace winbase

#endif  // WINLIB_WINBASE_PROCESS_PROCESS_METRICS_H_