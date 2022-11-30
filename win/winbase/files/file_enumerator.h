// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_FILES_FILE_ENUMERATOR_H_
#define WINLIB_WINBASE_FILES_FILE_ENUMERATOR_H_

#include <stddef.h>
#include <stdint.h>

#include <vector>

#include "winbase\base_export.h"
#include "winbase\containers\stack.h"
#include "winbase\files\file_path.h"
#include "winbase\macros.h"
#include "winbase\time\time.h"
#include "winbase\win\windows_types.h"
#include "winlib\build_config.h"

namespace winbase {

// A class for enumerating the files in a provided path. The order of the
// results is not guaranteed.
//
// This is blocking. Do not use on critical threads.
//
// Example:
//
//   winbase::FileEnumerator enum(my_dir, false, winbase::FileEnumerator::FILES,
//                             WINBASE_FILE_PATH_LITERAL("*.txt"));
//   for (winbase::FilePath name = enum.Next(); !name.empty(); 
//        name = enum.Next())
//     ...
class WINBASE_EXPORT FileEnumerator {
 public:
  // Note: copy & assign supported.
  class WINBASE_EXPORT FileInfo {
   public:
    FileInfo();
    ~FileInfo();

    bool IsDirectory() const;

    // The name of the file. This will not include any path information. This
    // is in constrast to the value returned by FileEnumerator.Next() which
    // includes the |root_path| passed into the FileEnumerator constructor.
    FilePath GetName() const;

    int64_t GetSize() const;
    Time GetLastModifiedTime() const;

    // Note that the cAlternateFileName (used to hold the "short" 8.3 name)
    // of the WIN32_FIND_DATA will be empty. Since we don't use short file
    // names, we tell Windows to omit it which speeds up the query slightly.
    const WIN32_FIND_DATA& find_data() const { return find_data_; }

   private:
    friend class FileEnumerator;
    WIN32_FIND_DATA find_data_;
  };

  enum FileType {
    FILES = 1 << 0,
    DIRECTORIES = 1 << 1,
    INCLUDE_DOT_DOT = 1 << 2,
  };

  // Search policy for intermediate folders.
  enum class FolderSearchPolicy {
    // Recursive search will pass through folders whose names match the
    // pattern. Inside each one, all files will be returned. Folders with names
    // that do not match the pattern will be ignored within their interior.
    MATCH_ONLY,
    // Recursive search will pass through every folder and perform pattern
    // matching inside each one.
    ALL,
  };

  // |root_path| is the starting directory to search for. It may or may not end
  // in a slash.
  //
  // If |recursive| is true, this will enumerate all matches in any
  // subdirectories matched as well. It does a breadth-first search, so all
  // files in one directory will be returned before any files in a
  // subdirectory.
  //
  // |file_type|, a bit mask of FileType, specifies whether the enumerator
  // should match files, directories, or both.
  //
  // |pattern| is an optional pattern for which files to match. This
  // works like shell globbing. For example, "*.txt" or "Foo???.doc".
  // However, be careful in specifying patterns that aren't cross platform
  // since the underlying code uses OS-specific matching routines.  In general,
  // Windows matching is less featureful than others, so test there first.
  // If unspecified, this will match all files.
  FileEnumerator(const FilePath& root_path,
                 bool recursive,
                 int file_type);
  FileEnumerator(const FilePath& root_path,
                 bool recursive,
                 int file_type,
                 const FilePath::StringType& pattern);
  FileEnumerator(const FilePath& root_path,
                 bool recursive,
                 int file_type,
                 const FilePath::StringType& pattern,
                 FolderSearchPolicy folder_search_policy);
  FileEnumerator(const FileEnumerator&) = delete;
  FileEnumerator& operator=(const FileEnumerator&) = delete;

  ~FileEnumerator();

  // Returns the next file or an empty string if there are no more results.
  //
  // The returned path will incorporate the |root_path| passed in the
  // constructor: "<root_path>/file_name.txt". If the |root_path| is absolute,
  // then so will be the result of Next().
  FilePath Next();

  // Write the file info into |info|.
  FileInfo GetInfo() const;

 private:
  // Returns true if the given path should be skipped in enumeration.
  bool ShouldSkip(const FilePath& path);

  bool IsTypeMatched(bool is_dir) const;

  bool IsPatternMatched(const FilePath& src) const;

  // True when find_data_ is valid.
  bool has_find_data_ = false;
  WIN32_FIND_DATA find_data_;
  HANDLE find_handle_ = INVALID_HANDLE_VALUE;
  FilePath root_path_;
  const bool recursive_;
  const int file_type_;
  FilePath::StringType pattern_;
  const FolderSearchPolicy folder_search_policy_;

  // A stack that keeps track of which subdirectories we still need to
  // enumerate in the breadth-first search.
  winbase::stack<FilePath> pending_paths_;
};

}  // namespace winbase

#endif  // WINLIB_WINBASE_FILES_FILE_ENUMERATOR_H_