// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file contains methods to iterate over processes on the system.

#ifndef WINLIB_WINBASE_PROCESS_PROCESS_ITERATOR_H_
#define WINLIB_WINBASE_PROCESS_PROCESS_ITERATOR_H_

#include <stddef.h>
#include <windows.h>
#include <tlhelp32.h>

#include <list>
#include <string>
#include <vector>

#include "winbase\base_export.h"
#include "winbase\files\file_path.h"
#include "winbase\macros.h"
#include "winbase\process/process.h"

namespace winbase {

struct ProcessEntry : public PROCESSENTRY32 {
  ProcessId pid() const { return th32ProcessID; }
  ProcessId parent_pid() const { return th32ParentProcessID; }
  const wchar_t* exe_file() const { return szExeFile; }
};

// Used to filter processes by process ID.
class ProcessFilter {
 public:
  // Returns true to indicate set-inclusion and false otherwise.  This method
  // should not have side-effects and should be idempotent.
  virtual bool Includes(const ProcessEntry& entry) const = 0;

 protected:
  virtual ~ProcessFilter() = default;
};

// This class provides a way to iterate through a list of processes on the
// current machine with a specified filter.
// To use, create an instance and then call NextProcessEntry() until it returns
// false.
class WINBASE_EXPORT ProcessIterator {
 public:
  typedef std::list<ProcessEntry> ProcessEntries;

  explicit ProcessIterator(const ProcessFilter* filter);
  ProcessIterator(const ProcessIterator&) = delete;
  ProcessIterator& operator=(const ProcessIterator&) = delete;
  virtual ~ProcessIterator();

  // If there's another process that matches the given executable name,
  // returns a const pointer to the corresponding PROCESSENTRY32.
  // If there are no more matching processes, returns NULL.
  // The returned pointer will remain valid until NextProcessEntry()
  // is called again or this NamedProcessIterator goes out of scope.
  const ProcessEntry* NextProcessEntry();

  // Takes a snapshot of all the ProcessEntry found.
  ProcessEntries Snapshot();

 protected:
  virtual bool IncludeEntry();
  const ProcessEntry& entry() { return entry_; }

 private:
  // Determines whether there's another process (regardless of executable)
  // left in the list of all processes.  Returns true and sets entry_ to
  // that process's info if there is one, false otherwise.
  bool CheckForNextProcess();

  // Initializes a PROCESSENTRY32 data structure so that it's ready for
  // use with Process32First/Process32Next.
  void InitProcessEntry(ProcessEntry* entry);

  HANDLE snapshot_;
  bool started_iteration_;
  ProcessEntry entry_;
  const ProcessFilter* filter_;
};

// This class provides a way to iterate through the list of processes
// on the current machine that were started from the given executable
// name.  To use, create an instance and then call NextProcessEntry()
// until it returns false.
class WINBASE_EXPORT NamedProcessIterator : public ProcessIterator {
 public:
  NamedProcessIterator(const FilePath::StringType& executable_name,
                       const ProcessFilter* filter);
  NamedProcessIterator(const NamedProcessIterator&) = delete;
  NamedProcessIterator& operator=(const NamedProcessIterator&) = delete;
  ~NamedProcessIterator() override;

 protected:
  bool IncludeEntry() override;

 private:
  FilePath::StringType executable_name_;
};

// Returns the number of processes on the machine that are running from the
// given executable name.  If filter is non-null, then only processes selected
// by the filter will be counted.
WINBASE_EXPORT int GetProcessCount(const FilePath::StringType& executable_name,
                                   const ProcessFilter* filter);

}  // namespace base

#endif  // WINLIB_WINBASE_PROCESS_PROCESS_ITERATOR_H_