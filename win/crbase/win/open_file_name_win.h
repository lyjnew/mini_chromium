// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_CRBASE_WIN_OPEN_FILE_NAME_WIN_H_
#define MINI_CHROMIUM_CRBASE_WIN_OPEN_FILE_NAME_WIN_H_

#include <windows.h>
#include <commdlg.h>

#include <vector>

#include "crbase/macros.h"
#include "crbase/strings/string16.h"
#include "crbase/tuple.h"

namespace crbase {

class FilePath;

namespace win {

// Encapsulates an OPENFILENAME struct and related buffers. Also provides static
// methods for interpreting the properties of an OPENFILENAME.
class CRBASE_EXPORT OpenFileName {
 public:
  // Initializes the OPENFILENAME, which may be accessed using Get(). All fields
  // will be NULL except for |lStructSize|, |lpstrFile|, and |nMaxFile|. The
  // file buffer will initially contain a null-terminated empty string.
  OpenFileName(HWND parent_window, DWORD flags);
  ~OpenFileName();

  // Initializes |lpstrFilter| from the label/pattern pairs in |filters|.
  // e.g.: 
  //  std::vector<crbase::Tuple<crbase::string16, crbase::string16>> filters;
  //  filters.push_back(crbase::MakeTuple(crbase::string16(L"PNG File(*.png)"), 
  //                                      crbase::string16(L"*.png")));
  //  foo.SetFilters(filters);
  //  ::GetSaveFileName(foo.GetOPENFILENAME());
  void SetFilters(
      const std::vector<crbase::Tuple<crbase::string16, 
                                      crbase::string16>>& filters);

  // Sets |lpstrInitialDir| and |lpstrFile|.
  void SetInitialSelection(const crbase::FilePath& initial_directory,
                           const crbase::FilePath& initial_filename);

  // The save as dialog on Windows XP remembers its last position, and if the
  // screen resolution has changed it may be off screen. This method will check
  // if we are running on XP and if so install a hook to reposition the dialog
  // if necessary.
  void MaybeInstallWindowPositionHookForSaveAsOnXP();

  // Returns the single selected file, or an empty path if there are more or
  // less than one results.
  crbase::FilePath GetSingleResult();

  // Returns the selected file or files.
  void GetResult(crbase::FilePath* directory,
                 std::vector<crbase::FilePath>* filenames);

  // Returns the OPENFILENAME structure.
  OPENFILENAME* GetOPENFILENAME() { return &openfilename_; }

  // Returns the OPENFILENAME structure.
  const OPENFILENAME* GetOPENFILENAME() const { return &openfilename_; }

  // Stores directory and filenames in the buffer pointed to by
  // |openfilename->lpstrFile| and sized |openfilename->nMaxFile|.
  static void SetResult(const crbase::FilePath& directory,
                        const std::vector<crbase::FilePath>& filenames,
                        OPENFILENAME* openfilename);

  // Returns a vector of label/pattern pairs built from
  // |openfilename->lpstrFilter|.
  static std::vector<crbase::Tuple<crbase::string16, 
                                   crbase::string16>> GetFilters(
      const OPENFILENAME* openfilename);

 private:
  OPENFILENAME openfilename_;
  crbase::string16 initial_directory_buffer_;
  wchar_t filename_buffer_[UNICODE_STRING_MAX_CHARS];
  crbase::string16 filter_buffer_;

  CR_DISALLOW_COPY_AND_ASSIGN(OpenFileName);
};

}  // namespace win
}  // namespace crbase

#endif  // MINI_CHROMIUM_CRBASE_WIN_OPEN_FILE_NAME_WIN_H_