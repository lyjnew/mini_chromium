// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_FILE_VERSION_INFO_WIN_H_
#define WINLIB_WINBASE_FILE_VERSION_INFO_WIN_H_

#include <windows.h>

#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

#include "winbase\base_export.h"
#include "winbase\file_version_info.h"
#include "winbase\macros.h"

struct tagVS_FIXEDFILEINFO;
typedef tagVS_FIXEDFILEINFO VS_FIXEDFILEINFO;

namespace winbase {

class WINBASE_EXPORT FileVersionInfoWin : public FileVersionInfo {
 public:
  FileVersionInfoWin(const FileVersionInfoWin&) = delete;
  FileVersionInfoWin& operator=(const FileVersionInfoWin&) = delete;
  ~FileVersionInfoWin() override;

  // Accessors to the different version properties.
  // Returns an empty string if the property is not found.
  winbase::string16 company_name() override;
  winbase::string16 company_short_name() override;
  winbase::string16 product_name() override;
  winbase::string16 product_short_name() override;
  winbase::string16 internal_name() override;
  winbase::string16 product_version() override;
  winbase::string16 private_build() override;
  winbase::string16 special_build() override;
  winbase::string16 comments() override;
  winbase::string16 original_filename() override;
  winbase::string16 file_description() override;
  winbase::string16 file_version() override;
  winbase::string16 legal_copyright() override;
  winbase::string16 legal_trademarks() override;
  winbase::string16 last_change() override;
  bool is_official_build() override;

  // Lets you access other properties not covered above.
  bool GetValue(const wchar_t* name, std::wstring* value);

  // Similar to GetValue but returns a wstring (empty string if the property
  // does not exist).
  std::wstring GetStringValue(const wchar_t* name);

  // Get the fixed file info if it exists. Otherwise NULL
  const VS_FIXEDFILEINFO* fixed_file_info() const { return fixed_file_info_; }

 private:
  friend FileVersionInfo;

  // |data| is a VS_VERSION_INFO resource. |language| and |code_page| are
  // extracted from the \VarFileInfo\Translation value of |data|.
  FileVersionInfoWin(std::vector<uint8_t>&& data,
                     WORD language,
                     WORD code_page);
  FileVersionInfoWin(void* data, WORD language, WORD code_page);

  const std::vector<uint8_t> owned_data_;
  const void* const data_;
  const WORD language_;
  const WORD code_page_;

  // This is a pointer into |data_| if it exists. Otherwise nullptr.
  const VS_FIXEDFILEINFO* const fixed_file_info_;
};

}  // namespace winbase

#endif  // WINLIB_WINBASE_FILE_VERSION_INFO_WIN_H_