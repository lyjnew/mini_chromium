// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_WIN_WINDOWS_VERSION_H_
#define WINLIB_WINBASE_WIN_WINDOWS_VERSION_H_

#include <stddef.h>

#include <string>

#include "winbase\base_export.h"
#include "winbase\macros.h"

typedef void* HANDLE;

namespace winbase {
namespace win {

// The running version of Windows.  This is declared outside OSInfo for
// syntactic sugar reasons; see the declaration of GetVersion() below.
// NOTE: Keep these in order so callers can do things like
// "if (winbase::win::GetVersion() >= 
//          winbase::win::OSVersion::VISTA) ...".
//
// This enum is used in metrics histograms, so they shouldn't be reordered or
// removed. New values can be added before VERSION_WIN_LAST.
enum class OSVersion {
  PRE_XP = 0,       // Not supported.
  XP = 1,
  SERVER_2003 = 2,  // Also includes XP Pro x64 and Server 2003 R2.
  VISTA = 3,        // Also includes Windows Server 2008.
  WIN7 = 4,         // Also includes Windows Server 2008 R2.
  WIN8 = 5,         // Also includes Windows Server 2012.
  WIN8_1 = 6,       // Also includes Windows Server 2012 R2.
  WIN10 = 7,        // Threshold 1: Version 1507, Build 10240.
  WIN10_TH2 = 8,    // Threshold 2: Version 1511, Build 10586.
  WIN10_RS1 = 9,    // Redstone 1: Version 1607, Build 14393.
  WIN10_RS2 = 10,   // Redstone 2: Version 1703, Build 15063.
  WIN10_RS3 = 11,   // Redstone 3: Version 1709, Build 16299.
  WIN10_RS4 = 12,   // Redstone 4: Version 1803, Build 17134.
  WIN10_RS5 = 13,   // Redstone 5: Version 1809, Build 17763.
                    // Also includes Windows Server 2019
  WIN10_19H1 = 14,  // 19H1: Version 1903, Build 18362.
  WIN10_19H2 = 15,  // 19H2: Version 1909, Build 18363.
  WIN10_20H1 = 16,  // 20H1: Build 19041.
  WIN10_20H2 = 17,  // 20H2: Build 19042.
  WIN10_21H1 = 18,  // 21H1: Build 19043.
  WIN10_21H2 = 19,  // Win10 21H2: Build 19044.
  SERVER_2022 = 20, // Server 2022: Build 20348.
  WIN11 = 21,       // Win11 21H2: Build 22000.
                    // On edit, update tools\metrics\histograms\enums.xml "WindowsVersion" and
                    // "GpuBlacklistFeatureTestResultsWindows2".
  WIN_LAST,         // Indicates error condition.
};

// A rough bucketing of the available types of versions of Windows. This is used
// to distinguish enterprise enabled versions from home versions and potentially
// server versions. Keep these values in the same order, since they are used as
// is for metrics histogram ids.
enum class VersionType {
  SUITE_HOME = 0,
  SUITE_PROFESSIONAL,
  SUITE_SERVER,
  SUITE_ENTERPRISE,
  SUITE_EDUCATION,
  SUITE_LAST,
};

// A singleton that can be used to query various pieces of information about the
// OS and process state. Note that this doesn't use the base Singleton class, so
// it can be used without an AtExitManager.
class WINBASE_EXPORT OSInfo {
 public:
  struct VersionNumber {
    int major;
    int minor;
    int build;
    int patch;
  };

  struct ServicePack {
    int major;
    int minor;
  };

  // The processor architecture this copy of Windows natively uses.  For
  // example, given an x64-capable processor, we have three possibilities:
  //   32-bit Chrome running on 32-bit Windows:           X86_ARCHITECTURE
  //   32-bit Chrome running on 64-bit Windows via WOW64: X64_ARCHITECTURE
  //   64-bit Chrome running on 64-bit Windows:           X64_ARCHITECTURE
  enum class WindowsArchitecture {
    X86_ARCHITECTURE,
    X64_ARCHITECTURE,
    IA64_ARCHITECTURE,
    OTHER_ARCHITECTURE,
  };

  // Whether a process is running under WOW64 (the wrapper that allows 32-bit
  // processes to run on 64-bit versions of Windows).  This will return
  // WOW64_DISABLED for both "32-bit Chrome on 32-bit Windows" and "64-bit
  // Chrome on 64-bit Windows".  WOW64_UNKNOWN means "an error occurred", e.g.
  // the process does not have sufficient access rights to determine this.
  enum class WOW64Status {
    WOW64_DISABLED,
    WOW64_ENABLED,
    WOW64_UNKNOWN,
  };

  static OSInfo* GetInstance();

  OSInfo(const OSInfo&) = delete;
  OSInfo& operator=(const OSInfo&) = delete;

  OSVersion version() const { return version_; }
  OSVersion Kernel32Version() const;
  // The next two functions return arrays of values, [major, minor(, build)].
  VersionNumber version_number() const { return version_number_; }
  VersionType version_type() const { return version_type_; }
  ServicePack service_pack() const { return service_pack_; }
  std::string service_pack_str() const { return service_pack_str_; }
  WindowsArchitecture architecture() const { return architecture_; }
  int processors() const { return processors_; }
  size_t allocation_granularity() const { return allocation_granularity_; }
  WOW64Status wow64_status() const { return wow64_status_; }
  std::string processor_model_name();

  // Like wow64_status(), but for the supplied handle instead of the current
  // process.  This doesn't touch member state, so you can bypass the singleton.
  static WOW64Status GetWOW64StatusForProcess(HANDLE process_handle);

 private:
  OSInfo();
  ~OSInfo();

  OSVersion version_;
  mutable OSVersion kernel32_version_;
  mutable bool got_kernel32_version_;
  VersionNumber version_number_;
  VersionType version_type_;
  ServicePack service_pack_;

  // A string, such as "Service Pack 3", that indicates the latest Service Pack
  // installed on the system. If no Service Pack has been installed, the string
  // is empty.
  std::string service_pack_str_;
  WindowsArchitecture architecture_;
  int processors_;
  size_t allocation_granularity_;
  WOW64Status wow64_status_;
  std::string processor_model_name_;
};

// Because this is by far the most commonly-requested value from the above
// singleton, we add a global-scope accessor here as syntactic sugar.
WINBASE_EXPORT OSVersion GetVersion();

}  // namespace win
}  // namespace winbase

#endif  // WINLIB_WINBASE_WIN_WINDOWS_VERSION_H_