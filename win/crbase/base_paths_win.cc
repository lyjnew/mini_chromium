// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windows.h>
#include <shlobj.h>

#include "crbase/base_paths.h"
#include "crbase/environment.h"
#include "crbase/files/file_path.h"
#include "crbase/path_service.h"
#include "crbase/strings/utf_string_conversions.h"
#include "crbase/win/scoped_co_mem.h"
#include "crbase/win/windows_version.h"

// http://blogs.msdn.com/oldnewthing/archive/2004/10/25/247180.aspx
extern "C" IMAGE_DOS_HEADER __ImageBase;

using crbase::FilePath;

namespace crbase {

bool PathProviderWin(int key, FilePath* result) {
  // We need to go compute the value. It would be nice to support paths with
  // names longer than MAX_PATH, but the system functions don't seem to be
  // designed for it either, with the exception of GetTempPath (but other
  // things will surely break if the temp path is too long, so we don't bother
  // handling it.
  wchar_t system_buffer[MAX_PATH];
  system_buffer[0] = 0;

  FilePath cur;
  switch (key) {
    case crbase::FILE_EXE:
      if (GetModuleFileName(NULL, system_buffer, MAX_PATH) == 0)
        return false;
      cur = FilePath(system_buffer);
      break;
    case crbase::FILE_MODULE: {
      // the resource containing module is assumed to be the one that
      // this code lives in, whether that's a dll or exe
      HMODULE this_module = reinterpret_cast<HMODULE>(&__ImageBase);
      if (GetModuleFileName(this_module, system_buffer, MAX_PATH) == 0)
        return false;
      cur = FilePath(system_buffer);
      break;
    }
    case crbase::DIR_WINDOWS:
      GetWindowsDirectory(system_buffer, MAX_PATH);
      cur = FilePath(system_buffer);
      break;
    case crbase::DIR_SYSTEM:
      GetSystemDirectory(system_buffer, MAX_PATH);
      cur = FilePath(system_buffer);
      break;
    case crbase::DIR_PROGRAM_FILESX86:
      if (crbase::win::OSInfo::GetInstance()->architecture() !=
          crbase::win::OSInfo::X86_ARCHITECTURE) {
        if (FAILED(SHGetFolderPath(NULL, CSIDL_PROGRAM_FILESX86, NULL,
                                   SHGFP_TYPE_CURRENT, system_buffer)))
          return false;
        cur = FilePath(system_buffer);
        break;
      }
      // Fall through to base::DIR_PROGRAM_FILES if we're on an X86 machine.
    case crbase::DIR_PROGRAM_FILES:
      if (FAILED(SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL,
                                 SHGFP_TYPE_CURRENT, system_buffer)))
        return false;
      cur = FilePath(system_buffer);
      break;
    case crbase::DIR_PROGRAM_FILES6432:
#if !defined(_WIN64)
      if (crbase::win::OSInfo::GetInstance()->wow64_status() ==
          crbase::win::OSInfo::WOW64_ENABLED) {
        crbase::scoped_ptr<crbase::Environment> env(
            crbase::Environment::Create());
        std::string programfiles_w6432;
        // 32-bit process running in WOW64 sets ProgramW6432 environment
        // variable. See
        // https://msdn.microsoft.com/library/windows/desktop/aa384274.aspx.
        if (!env->GetVar("ProgramW6432", &programfiles_w6432))
          return false;
        // GetVar returns UTF8 - convert back to Wide.
        cur = FilePath(UTF8ToWide(programfiles_w6432));
        break;
      }
#endif
      if (FAILED(SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL,
                                 SHGFP_TYPE_CURRENT, system_buffer)))
        return false;
      cur = FilePath(system_buffer);
      break;
    case crbase::DIR_IE_INTERNET_CACHE:
      if (FAILED(SHGetFolderPath(NULL, CSIDL_INTERNET_CACHE, NULL,
                                 SHGFP_TYPE_CURRENT, system_buffer)))
        return false;
      cur = FilePath(system_buffer);
      break;
    case crbase::DIR_COMMON_START_MENU:
      if (FAILED(SHGetFolderPath(NULL, CSIDL_COMMON_PROGRAMS, NULL,
                                 SHGFP_TYPE_CURRENT, system_buffer)))
        return false;
      cur = FilePath(system_buffer);
      break;
    case crbase::DIR_START_MENU:
      if (FAILED(SHGetFolderPath(NULL, CSIDL_PROGRAMS, NULL,
                                 SHGFP_TYPE_CURRENT, system_buffer)))
        return false;
      cur = FilePath(system_buffer);
      break;
    case crbase::DIR_APP_DATA:
      if (FAILED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT,
                                 system_buffer)))
        return false;
      cur = FilePath(system_buffer);
      break;
    case crbase::DIR_COMMON_APP_DATA:
      if (FAILED(SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL,
                                 SHGFP_TYPE_CURRENT, system_buffer)))
        return false;
      cur = FilePath(system_buffer);
      break;
    case crbase::DIR_LOCAL_APP_DATA:
      if (FAILED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL,
                                 SHGFP_TYPE_CURRENT, system_buffer)))
        return false;
      cur = FilePath(system_buffer);
      break;
    case crbase::DIR_SOURCE_ROOT: {
      FilePath executableDir;
      // On Windows, unit tests execute two levels deep from the source root.
      // For example:  chrome/{Debug|Release}/ui_tests.exe
      PathService::Get(crbase::DIR_EXE, &executableDir);
      cur = executableDir.DirName().DirName();
      break;
    }
    case crbase::DIR_APP_SHORTCUTS: {
      if (win::GetVersion() < win::VERSION_WIN8)
        return false;

      crbase::win::ScopedCoMem<wchar_t> path_buf;
      if (FAILED(SHGetKnownFolderPath(FOLDERID_ApplicationShortcuts, 0, NULL,
                                      &path_buf)))
        return false;

      cur = FilePath(string16(path_buf));
      break;
    }
    case crbase::DIR_USER_DESKTOP:
      if (FAILED(SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL,
                                 SHGFP_TYPE_CURRENT, system_buffer))) {
        return false;
      }
      cur = FilePath(system_buffer);
      break;
    case crbase::DIR_COMMON_DESKTOP:
      if (FAILED(SHGetFolderPath(NULL, CSIDL_COMMON_DESKTOPDIRECTORY, NULL,
                                 SHGFP_TYPE_CURRENT, system_buffer))) {
        return false;
      }
      cur = FilePath(system_buffer);
      break;
    case crbase::DIR_USER_QUICK_LAUNCH:
      if (!PathService::Get(crbase::DIR_APP_DATA, &cur))
        return false;
      // According to various sources, appending
      // "Microsoft\Internet Explorer\Quick Launch" to %appdata% is the only
      // reliable way to get the quick launch folder across all versions of
      // Windows.
      // http://stackoverflow.com/questions/76080/how-do-you-reliably-get-the-quick-
      // http://www.microsoft.com/technet/scriptcenter/resources/qanda/sept05/hey0901.mspx
      cur = cur.AppendASCII("Microsoft")
                .AppendASCII("Internet Explorer")
                .AppendASCII("Quick Launch");
      break;
    case crbase::DIR_TASKBAR_PINS:
      if (!PathService::Get(crbase::DIR_USER_QUICK_LAUNCH, &cur))
        return false;
      cur = cur.AppendASCII("User Pinned");
      cur = cur.AppendASCII("TaskBar");
      break;
    case crbase::DIR_WINDOWS_FONTS:
      if (FAILED(SHGetFolderPath(
              NULL, CSIDL_FONTS, NULL, SHGFP_TYPE_CURRENT, system_buffer))) {
        return false;
      }
      cur = FilePath(system_buffer);
      break;
    default:
      return false;
  }

  *result = cur;
  return true;
}

}  // namespace crbase