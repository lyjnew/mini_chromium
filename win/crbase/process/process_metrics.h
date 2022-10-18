// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file contains routines for gathering resource statistics for processes
// running on the system.

#ifndef MINI_CHROMIUM_CRBASE_PROCESS_PROCESS_METRICS_H_
#define MINI_CHROMIUM_CRBASE_PROCESS_PROCESS_METRICS_H_

#include <stddef.h>
#include <stdint.h>

#include <string>

#include "crbase/crbase_export.h"
///#include "crbase/gtest_prod_util.h"
#include "crbase/macros.h"
#include "crbase/process/process_handle.h"
#include "crbase/time/time.h"
#include "crbase/values.h"
#include "crbuild/build_config.h"

namespace crbase {

struct IoCounters : public IO_COUNTERS {
};

// Working Set (resident) memory usage broken down by
//
// On Windows:
// priv (private): These pages (kbytes) cannot be shared with any other process.
// shareable:      These pages (kbytes) can be shared with other processes under
//                 the right circumstances.
// shared :        These pages (kbytes) are currently shared with at least one
//                 other process.
//
// On Linux:
// priv:           Pages mapped only by this process.
// shared:         PSS or 0 if the kernel doesn't support this.
// shareable:      0

// On ChromeOS:
// priv:           Pages mapped only by this process.
// shared:         PSS or 0 if the kernel doesn't support this.
// shareable:      0
// swapped         Pages swapped out to zram.
//
// On OS X: TODO(thakis): Revise.
// priv:           Memory.
// shared:         0
// shareable:      0
//
struct WorkingSetKBytes {
  WorkingSetKBytes() : priv(0), shareable(0), shared(0) {}
  size_t priv;
  size_t shareable;
  size_t shared;
};

// Committed (resident + paged) memory usage broken down by
// private: These pages cannot be shared with any other process.
// mapped:  These pages are mapped into the view of a section (backed by
//          pagefile.sys)
// image:   These pages are mapped into the view of an image section (backed by
//          file system)
struct CommittedKBytes {
  CommittedKBytes() : priv(0), mapped(0), image(0) {}
  size_t priv;
  size_t mapped;
  size_t image;
};

// Convert a POSIX timeval to microseconds.
CRBASE_EXPORT int64_t TimeValToMicroseconds(const struct timeval& tv);

// Provides performance metrics for a specified process (CPU usage, memory and
// IO counters). Use CreateCurrentProcessMetrics() to get an instance for the
// current process, or CreateProcessMetrics() to get an instance for an
// arbitrary process. Then, access the information with the different get
// methods.
class CRBASE_EXPORT ProcessMetrics {
 public:
  ~ProcessMetrics();

  // Creates a ProcessMetrics for the specified process.
  // The caller owns the returned object.
  static ProcessMetrics* CreateProcessMetrics(ProcessHandle process);

  // Creates a ProcessMetrics for the current process. This a cross-platform
  // convenience wrapper for CreateProcessMetrics().
  // The caller owns the returned object.
  static ProcessMetrics* CreateCurrentProcessMetrics();

  // Returns the current space allocated for the pagefile, in bytes (these pages
  // may or may not be in memory).  On Linux, this returns the total virtual
  // memory size.
  size_t GetPagefileUsage() const;
  // Returns the peak space allocated for the pagefile, in bytes.
  size_t GetPeakPagefileUsage() const;
  // Returns the current working set size, in bytes.  On Linux, this returns
  // the resident set size.
  size_t GetWorkingSetSize() const;
  // Returns the peak working set size, in bytes.
  size_t GetPeakWorkingSetSize() const;
  // Returns private and sharedusage, in bytes. Private bytes is the amount of
  // memory currently allocated to a process that cannot be shared. Returns
  // false on platform specific error conditions.  Note: |private_bytes|
  // returns 0 on unsupported OSes: prior to XP SP2.
  bool GetMemoryBytes(size_t* private_bytes,
                      size_t* shared_bytes);
  // Fills a CommittedKBytes with both resident and paged
  // memory usage as per definition of CommittedBytes.
  void GetCommittedKBytes(CommittedKBytes* usage) const;
  // Fills a WorkingSetKBytes containing resident private and shared memory
  // usage in bytes, as per definition of WorkingSetBytes. Note that this
  // function is somewhat expensive on Windows (a few ms per process).
  bool GetWorkingSetKBytes(WorkingSetKBytes* ws_usage) const;

  // Returns the CPU usage in percent since the last time this method or
  // GetPlatformIndependentCPUUsage() was called. The first time this method
  // is called it returns 0 and will return the actual CPU info on subsequent
  // calls. On Windows, the CPU usage value is for all CPUs. So if you have
  // 2 CPUs and your process is using all the cycles of 1 CPU and not the other
  // CPU, this method returns 50.
  double GetCPUUsage();

  // Returns the number of average idle cpu wakeups per second since the last
  // call.
  int GetIdleWakeupsPerSecond();

  // Same as GetCPUUsage(), but will return consistent values on all platforms
  // (cancelling the Windows exception mentioned above) by returning a value in
  // the range of 0 to (100 * numCPUCores) everywhere.
  double GetPlatformIndependentCPUUsage();

  // Retrieves accounting information for all I/O operations performed by the
  // process.
  // If IO information is retrieved successfully, the function returns true
  // and fills in the IO_COUNTERS passed in. The function returns false
  // otherwise.
  bool GetIOCounters(IoCounters* io_counters) const;

 private:
  explicit ProcessMetrics(ProcessHandle process);

  ProcessHandle process_;

  int processor_count_;

  // Used to store the previous times and CPU usage counts so we can
  // compute the CPU usage between calls.
  TimeTicks last_cpu_time_;
  int64_t last_system_time_;

  CR_DISALLOW_COPY_AND_ASSIGN(ProcessMetrics);
};

// Returns the memory committed by the system in KBytes.
// Returns 0 if it can't compute the commit charge.
CRBASE_EXPORT size_t GetSystemCommitCharge();

// Returns the number of bytes in a memory page.
CRBASE_EXPORT size_t GetPageSize();

// Data about system-wide memory consumption. Values are in KB. Available on
// Windows, Mac, Linux, Android and Chrome OS.
//
// Total/free memory are available on all platforms that implement
// GetSystemMemoryInfo(). Total/free swap memory are available on all platforms
// except on Mac. Buffers/cached/active_anon/inactive_anon/active_file/
// inactive_file/dirty/pswpin/pswpout/pgmajfault are available on
// Linux/Android/Chrome OS. Shmem/slab/gem_objects/gem_size are Chrome OS only.
struct CRBASE_EXPORT SystemMemoryInfoKB {
  SystemMemoryInfoKB();

  // Serializes the platform specific fields to value.
  scoped_ptr<Value> ToValue() const;

  int total;
  int free;

  int swap_total;
  int swap_free;

};

// On Linux/Android/Chrome OS, system-wide memory consumption data is parsed
// from /proc/meminfo and /proc/vmstat. On Windows/Mac, it is obtained using
// system API calls.
//
// Fills in the provided |meminfo| structure. Returns true on success.
// Exposed for memory debugging widget.
CRBASE_EXPORT bool GetSystemMemoryInfo(SystemMemoryInfoKB* meminfo);

// Collects and holds performance metrics for system memory and disk.
// Provides functionality to retrieve the data on various platforms and
// to serialize the stored data.
class SystemMetrics {
 public:
  SystemMetrics();

  static SystemMetrics Sample();

  // Serializes the system metrics to value.
  scoped_ptr<Value> ToValue() const;

 private:
  ///FRIEND_TEST_ALL_PREFIXES(SystemMetricsTest, SystemMetrics);

  size_t committed_memory_;
};

}  // namespace crbase

#endif  // MINI_CHROMIUM_CRBASE_PROCESS_PROCESS_METRICS_H_