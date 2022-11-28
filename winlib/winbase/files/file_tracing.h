// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_FILES_FILE_TRACING_H_
#define WINLIB_WINBASE_FILES_FILE_TRACING_H_

#include <stdint.h>

#include "winbase\base_export.h"
#include "winbase\macros.h"

#define WINBASE_FILE_TRACING_PREFIX "File"

#define WINBASE_SCOPED_FILE_TRACE_WITH_SIZE(name, size)    \
  winbase::FileTracing::ScopedTrace scoped_file_trace;     \
  if (FileTracing::IsCategoryEnabled())                    \
    scoped_file_trace.Initialize(                          \
        WINBASE_FILE_TRACING_PREFIX "::" name, this, size)

#define WINBASE_SCOPED_FILE_TRACE(name) \
  WINBASE_SCOPED_FILE_TRACE_WITH_SIZE(name, 0)

namespace winbase {

class File;
class FilePath;

class WINBASE_EXPORT FileTracing {
 public:
  // Whether the file tracing category is enabled.
  static bool IsCategoryEnabled();

  FileTracing(const FileTracing&) = delete;
  FileTracing& operator=(const FileTracing&) = delete;

  class Provider {
   public:
    virtual ~Provider() = default;

    // Whether the file tracing category is currently enabled.
    virtual bool FileTracingCategoryIsEnabled() const = 0;

    // Enables file tracing for |id|. Must be called before recording events.
    virtual void FileTracingEnable(const void* id) = 0;

    // Disables file tracing for |id|.
    virtual void FileTracingDisable(const void* id) = 0;

    // Begins an event for |id| with |name|. |path| tells where in the directory
    // structure the event is happening (and may be blank). |size| is the number
    // of bytes involved in the event.
    virtual void FileTracingEventBegin(const char* name,
                                       const void* id,
                                       const FilePath& path,
                                       int64_t size) = 0;

    // Ends an event for |id| with |name|.
    virtual void FileTracingEventEnd(const char* name, const void* id) = 0;
  };

  // Sets a global file tracing provider to query categories and record events.
  static void SetProvider(Provider* provider);

  // Enables file tracing while in scope.
  class ScopedEnabler {
   public:
    ScopedEnabler();
    ~ScopedEnabler();
  };

  class ScopedTrace {
   public:
    ScopedTrace();
    ScopedTrace(const ScopedTrace&) = delete;
    ScopedTrace& operator=(const ScopedTrace&) = delete;
    ~ScopedTrace();

    // Called only if the tracing category is enabled. |name| is the name of the
    // event to trace (e.g. "Read", "Write") and must have an application
    // lifetime (e.g. static or literal). |file| is the file being traced; must
    // outlive this class. |size| is the size (in bytes) of this event.
    void Initialize(const char* name, const File* file, int64_t size);

   private:
    // The ID of this trace. Based on the |file| passed to |Initialize()|. Must
    // outlive this class.
    const void* id_;

    // The name of the event to trace (e.g. "Read", "Write"). Prefixed with
    // "File".
    const char* name_;
  };
};

}  // namespace base

#endif  // WINLIB_WINBASE_FILES_FILE_TRACING_H_