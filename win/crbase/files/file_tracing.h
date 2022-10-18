// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_CRBASE_FILES_FILE_TRACING_H_
#define MINI_CHROMIUM_CRBASE_FILES_FILE_TRACING_H_

#include <stdint.h>

#include "crbase/crbase_export.h"
#include "crbase/macros.h"

#define CR_FILE_TRACING_PREFIX "File"

#define CR_SCOPED_FILE_TRACE_WITH_SIZE(name, size) \
    FileTracing::ScopedTrace scoped_file_trace; \
    if (FileTracing::IsCategoryEnabled()) \
      scoped_file_trace.Initialize(CR_FILE_TRACING_PREFIX "::" name, this, size)

#define CR_SCOPED_FILE_TRACE(name) CR_SCOPED_FILE_TRACE_WITH_SIZE(name, 0)

namespace crbase {

class File;
class FilePath;

class CRBASE_EXPORT FileTracing {
 public:
  // Whether the file tracing category is enabled.
  static bool IsCategoryEnabled();

  class Provider {
   public:
    virtual ~Provider() = default;

    // Whether the file tracing category is currently enabled.
    virtual bool FileTracingCategoryIsEnabled() const = 0;

    // Enables file tracing for |id|. Must be called before recording events.
    virtual void FileTracingEnable(void* id) = 0;

    // Disables file tracing for |id|.
    virtual void FileTracingDisable(void* id) = 0;

    // Begins an event for |id| with |name|. |path| tells where in the directory
    // structure the event is happening (and may be blank). |size| is the number
    // of bytes involved in the event.
    virtual void FileTracingEventBegin(const char* name,
                                       void* id,
                                       const FilePath& path,
                                       int64_t size) = 0;

    // Ends an event for |id| with |name|.
    virtual void FileTracingEventEnd(const char* name, void* id) = 0;
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
    ~ScopedTrace();

    // Called only if the tracing category is enabled. |name| is the name of the
    // event to trace (e.g. "Read", "Write") and must have an application
    // lifetime (e.g. static or literal). |file| is the file being traced; must
    // outlive this class. |size| is the size (in bytes) of this event.
    void Initialize(const char* name, File* file, int64_t size);

   private:
    // The ID of this trace. Based on the |file| passed to |Initialize()|. Must
    // outlive this class.
    void* id_;

    // The name of the event to trace (e.g. "Read", "Write"). Prefixed with
    // "File".
    const char* name_;

    CR_DISALLOW_COPY_AND_ASSIGN(ScopedTrace);
  };

  CR_DISALLOW_COPY_AND_ASSIGN(FileTracing);
};

}  // namespace crbase

#endif  // MINI_CHROMIUM_CRBASE_FILES_FILE_TRACING_H_