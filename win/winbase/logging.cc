// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "winbase\logging.h"

#include <limits.h>
#include <stdint.h>

#include <io.h>
#include <windows.h>

#include "winbase\macros.h"
#include "winlib\build_config.h"

typedef HANDLE FileHandle;
typedef HANDLE MutexHandle;
// Windows warns on using write().  It prefers _write().
#define write(fd, buf, count) _write(fd, buf, static_cast<unsigned int>(count))
// Windows doesn't define STDERR_FILENO.  Define it here.
#define STDERR_FILENO 2

#include <algorithm>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <ostream>
#include <string>
#include <utility>

///#include "winbase\base_switches.h"
#include "winbase\functional\callback.h"
///#include "winbase\command_line.h"
#include "winbase\containers\stack.h"
///#include "winbase\debug\activity_tracker.h"
#include "winbase\debug\alias.h"
#include "winbase\debug\debugger.h"
#include "winbase\debug\stack_trace.h"
#include "winbase\lazy_instance.h"
#include "winbase\strings\string_piece.h"
#include "winbase\strings\string_util.h"
#include "winbase\strings\stringprintf.h"
#include "winbase\strings\sys_string_conversions.h"
#include "winbase\strings\utf_string_conversions.h"
#include "winbase\synchronization\lock_impl.h"
#include "winbase\threading\platform_thread.h"
#include "winbase\win\nominmax.h"
///#include "winbase\vlog.h"


namespace winbase {
namespace logging {

namespace {

///VlogInfo* g_vlog_info = nullptr;
///VlogInfo* g_vlog_info_prev = nullptr;

const char* const log_severity_names[] = {"INFO", "WARNING", "ERROR", "FATAL"};
static_assert(LOG_NUM_SEVERITIES == array_size(log_severity_names),
              "Incorrect number of log_severity_names");

const char* log_severity_name(int severity) {
  if (severity >= 0 && severity < LOG_NUM_SEVERITIES)
    return log_severity_names[severity];
  return "UNKNOWN";
}

int g_min_log_level = 0;

LoggingDestination g_logging_destination = LOG_DEFAULT;

// For LOG_ERROR and above, always print to stderr.
const int kAlwaysPrintErrorLevel = LOG_ERROR;

// Which log file to use? This is initialized by InitLogging or
// will be lazily initialized to the default value when it is
// first needed.
typedef std::wstring PathString;
PathString* g_log_file_name = nullptr;

// This file is lazily opened and the handle may be nullptr
FileHandle g_log_file = nullptr;

// What should be prepended to each message?
bool g_log_process_id = false;
bool g_log_thread_id = false;
bool g_log_timestamp = true;
bool g_log_tickcount = false;

// Should we pop up fatal debug messages in a dialog?
bool show_error_dialogs = false;

// An assert handler override specified by the client to be called instead of
// the debug message dialog and process termination. Assert handlers are stored
// in stack to allow overriding and restoring.
winbase::LazyInstance<winbase::stack<LogAssertHandlerFunction>>::Leaky
    log_assert_handler_stack = WINBASE_LAZY_INSTANCE_INITIALIZER;

// A log message handler that gets notified of every log message we process.
LogMessageHandlerFunction log_message_handler = nullptr;

// Helper functions to wrap platform differences.

int32_t CurrentProcessId() {
  return ::GetCurrentProcessId();
}

uint64_t TickCount() {
  return ::GetTickCount();
}

void DeleteFilePath(const PathString& log_name) {
  ::DeleteFileW(log_name.c_str());
}

PathString GetDefaultLogFile() {
  // On Windows we use the same path as the exe.
  wchar_t module_name[MAX_PATH];
  GetModuleFileName(nullptr, module_name, MAX_PATH);

  PathString log_name = module_name;
  PathString::size_type last_backslash = log_name.rfind('\\', log_name.size());
  if (last_backslash != PathString::npos)
    log_name.erase(last_backslash + 1);
  log_name += L"debug.log";
  return log_name;
}

// Called by logging functions to ensure that |g_log_file| is initialized
// and can be used for writing. Returns false if the file could not be
// initialized. |g_log_file| will be nullptr in this case.
bool InitializeLogFileHandle() {
  if (g_log_file)
    return true;

  if (!g_log_file_name) {
    // Nobody has called InitLogging to specify a debug log file, so here we
    // initialize the log file name to a default.
    g_log_file_name = new PathString(GetDefaultLogFile());
  }

  if ((g_logging_destination & LOG_TO_FILE) != 0) {
    // The FILE_APPEND_DATA access mask ensures that the file is atomically
    // appended to across accesses from multiple threads.
    // https://msdn.microsoft.com/en-us/library/windows/desktop/aa364399(v=vs.85).aspx
    // https://msdn.microsoft.com/en-us/library/windows/desktop/aa363858(v=vs.85).aspx
    g_log_file = CreateFile(g_log_file_name->c_str(), FILE_APPEND_DATA,
                            FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                            OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (g_log_file == INVALID_HANDLE_VALUE || g_log_file == nullptr) {
      // We are intentionally not using FilePath or FileUtil here to reduce the
      // dependencies of the logging implementation. For e.g. FilePath and
      // FileUtil depend on shell32 and user32.dll. This is not acceptable for
      // some consumers of base logging like chrome_elf, etc.
      // Please don't change the code below to use FilePath.
      // try the current directory
      wchar_t system_buffer[MAX_PATH];
      system_buffer[0] = 0;
      DWORD len = ::GetCurrentDirectory(array_size(system_buffer),
                                        system_buffer);
      if (len == 0 || len > array_size(system_buffer))
        return false;

      *g_log_file_name = system_buffer;
      // Append a trailing backslash if needed.
      if (g_log_file_name->back() != L'\\')
        *g_log_file_name += L"\\";
      *g_log_file_name += L"debug.log";

      g_log_file = CreateFile(g_log_file_name->c_str(), FILE_APPEND_DATA,
                              FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                              OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
      if (g_log_file == INVALID_HANDLE_VALUE || g_log_file == nullptr) {
        g_log_file = nullptr;
        return false;
      }
    }
  }

  return true;
}

void CloseFile(FileHandle log) {
  ::CloseHandle(log);
}

void CloseLogFileUnlocked() {
  if (!g_log_file)
    return;

  CloseFile(g_log_file);
  g_log_file = nullptr;
}

}  // namespace

#if WINBASE_DCHECK_IS_CONFIGURABLE
// In DCHECK-enabled Chrome builds, allow the meaning of LOG_DCHECK to be
// determined at run-time. We default it to INFO, to avoid it triggering
// crashes before the run-time has explicitly chosen the behaviour.
WINBASE_EXPORT logging::LogSeverity LOG_DCHECK = LOG_INFO;
#endif  // WINBASE_DCHECK_IS_CONFIGURABLE

// This is never instantiated, it's just used for EAT_STREAM_PARAMETERS to have
// an object of the correct type on the LHS of the unused part of the ternary
// operator.
std::ostream* g_swallow_stream;

LoggingSettings::LoggingSettings()
    : logging_dest(LOG_DEFAULT),
      log_file(nullptr),
      lock_log(LOCK_LOG_FILE),
      delete_old(APPEND_TO_OLD_LOG_FILE) {}

bool WinBaseInitLoggingImpl(const LoggingSettings& settings) {
  ///base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  ///// Don't bother initializing |g_vlog_info| unless we use one of the
  ///// vlog switches.
  ///if (command_line->HasSwitch(switches::kV) ||
  ///    command_line->HasSwitch(switches::kVModule)) {
  ///  // NOTE: If |g_vlog_info| has already been initialized, it might be in use
  ///  // by another thread. Don't delete the old VLogInfo, just create a second
  ///  // one. We keep track of both to avoid memory leak warnings.
  ///  CHECK(!g_vlog_info_prev);
  ///  g_vlog_info_prev = g_vlog_info;
  ///
  ///  g_vlog_info =
  ///      new VlogInfo(command_line->GetSwitchValueASCII(switches::kV),
  ///                   command_line->GetSwitchValueASCII(switches::kVModule),
  ///                   &g_min_log_level);
  ///}

  g_logging_destination = settings.logging_dest;

  // ignore file options unless logging to file is set.
  if ((g_logging_destination & LOG_TO_FILE) == 0)
    return true;

  // Calling InitLogging twice or after some log call has already opened the
  // default log file will re-initialize to the new options.
  CloseLogFileUnlocked();

  if (!g_log_file_name)
    g_log_file_name = new PathString();
  *g_log_file_name = settings.log_file;
  if (settings.delete_old == DELETE_OLD_LOG_FILE)
    DeleteFilePath(*g_log_file_name);

  return InitializeLogFileHandle();
}

void SetMinLogLevel(int level) {
  g_min_log_level = std::min(LOG_FATAL, level);
}

int GetMinLogLevel() {
  return g_min_log_level;
}

bool ShouldCreateLogMessage(int severity) {
  if (severity < g_min_log_level)
    return false;

  // Return true here unless we know ~LogMessage won't do anything. Note that
  // ~LogMessage writes to stderr if severity_ >= kAlwaysPrintErrorLevel, even
  // when g_logging_destination is LOG_NONE.
  return g_logging_destination != LOG_NONE || log_message_handler ||
         severity >= kAlwaysPrintErrorLevel;
}

int GetVlogVerbosity() {
  return std::max(-1, LOG_INFO - GetMinLogLevel());
}

int GetVlogLevelHelper(const char* file, size_t N) {
  WINBASE_DCHECK_GT(N, 0U);
  // Note: |g_vlog_info| may change on a different thread during startup
  // (but will always be valid or nullptr).
  ///VlogInfo* vlog_info = g_vlog_info;
  ///return vlog_info ?
  ///    vlog_info->GetVlogLevel(winbase::StringPiece(file, N - 1)) :
  ///    GetVlogVerbosity();
  return GetVlogVerbosity();
}

void SetLogItems(bool enable_process_id, bool enable_thread_id,
                 bool enable_timestamp, bool enable_tickcount) {
  g_log_process_id = enable_process_id;
  g_log_thread_id = enable_thread_id;
  g_log_timestamp = enable_timestamp;
  g_log_tickcount = enable_tickcount;
}

void SetShowErrorDialogs(bool enable_dialogs) {
  show_error_dialogs = enable_dialogs;
}

ScopedLogAssertHandler::ScopedLogAssertHandler(
    LogAssertHandlerFunction handler) {
  log_assert_handler_stack.Get().push(std::move(handler));
}

ScopedLogAssertHandler::~ScopedLogAssertHandler() {
  log_assert_handler_stack.Get().pop();
}

void SetLogMessageHandler(LogMessageHandlerFunction handler) {
  log_message_handler = handler;
}

LogMessageHandlerFunction GetLogMessageHandler() {
  return log_message_handler;
}

// Explicit instantiations for commonly used comparisons.
template std::string* MakeCheckOpString<int, int>(
    const int&, const int&, const char* names);
template std::string* MakeCheckOpString<unsigned long, unsigned long>(
    const unsigned long&, const unsigned long&, const char* names);
template std::string* MakeCheckOpString<unsigned long, unsigned int>(
    const unsigned long&, const unsigned int&, const char* names);
template std::string* MakeCheckOpString<unsigned int, unsigned long>(
    const unsigned int&, const unsigned long&, const char* names);
template std::string* MakeCheckOpString<std::string, std::string>(
    const std::string&, const std::string&, const char* name);

void MakeCheckOpValueString(std::ostream* os, std::nullptr_t p) {
  (*os) << "nullptr";
}

#if !defined(NDEBUG)
// Displays a message box to the user with the error message in it.
// Used for fatal messages, where we close the app simultaneously.
// This is for developers only; we don't use this in circumstances
// (like release builds) where users could see it, since users don't
// understand these messages anyway.
void DisplayDebugMessageInDialog(const std::string& str) {
  if (str.empty())
    return;

  if (!show_error_dialogs)
    return;

  // We intentionally don't implement a dialog on other platforms.
  // You can just look at stderr.
  MessageBoxW(nullptr, base::UTF8ToUTF16(str).c_str(), L"Fatal error",
              MB_OK | MB_ICONHAND | MB_TOPMOST);
}
#endif  // !defined(NDEBUG)

LogMessage::SaveLastError::SaveLastError() : last_error_(::GetLastError()) {
}

LogMessage::SaveLastError::~SaveLastError() {
  ::SetLastError(last_error_);
}

LogMessage::LogMessage(const char* file, int line, LogSeverity severity)
    : severity_(severity), file_(file), line_(line) {
  Init(file, line);
}

LogMessage::LogMessage(const char* file, int line, const char* condition)
    : severity_(LOG_FATAL), file_(file), line_(line) {
  Init(file, line);
  stream_ << "Check failed: " << condition << ". ";
}

LogMessage::LogMessage(const char* file, int line, std::string* result)
    : severity_(LOG_FATAL), file_(file), line_(line) {
  Init(file, line);
  stream_ << "Check failed: " << *result;
  delete result;
}

LogMessage::LogMessage(const char* file, int line, LogSeverity severity,
                       std::string* result)
    : severity_(severity), file_(file), line_(line) {
  Init(file, line);
  stream_ << "Check failed: " << *result;
  delete result;
}

LogMessage::~LogMessage() {
  size_t stack_start = static_cast<size_t>(stream_.tellp());
#if !defined(OFFICIAL_BUILD) 
  if (severity_ == LOG_FATAL && !winbase::debug::BeingDebugged()) {
    // Include a stack trace on a fatal, unless a debugger is attached.
    winbase::debug::StackTrace trace;
    stream_ << std::endl;  // Newline to separate from log message.
    trace.OutputToStream(&stream_);
  }
#endif
  stream_ << std::endl;
  std::string str_newline(stream_.str());

  // Give any log message handler first dibs on the message.
  if (log_message_handler &&
      log_message_handler(severity_, file_, line_,
                          message_start_, str_newline)) {
    // The handler took care of it, no further processing.
    return;
  }

  if ((g_logging_destination & LOG_TO_SYSTEM_DEBUG_LOG) != 0) {
    OutputDebugStringA(str_newline.c_str());
    ignore_result(fwrite(str_newline.data(), str_newline.size(), 1, stderr));
    fflush(stderr);
  } else if (severity_ >= kAlwaysPrintErrorLevel) {
    // When we're only outputting to a log file, above a certain log level, we
    // should still output to stderr so that we can better detect and diagnose
    // problems with unit tests, especially on the buildbots.
    ignore_result(fwrite(str_newline.data(), str_newline.size(), 1, stderr));
    fflush(stderr);
  }

  // write to log file
  if ((g_logging_destination & LOG_TO_FILE) != 0) {
    // We can have multiple threads and/or processes, so try to prevent them
    // from clobbering each other's writes.
    // If the client app did not call InitLogging, and the lock has not
    // been created do it now. We do this on demand, but if two threads try
    // to do this at the same time, there will be a race condition to create
    // the lock. This is why InitLogging should be called from the main
    // thread at the beginning of execution.
    if (InitializeLogFileHandle()) {
      DWORD num_written;
      WriteFile(g_log_file,
                static_cast<const void*>(str_newline.c_str()),
                static_cast<DWORD>(str_newline.length()),
                &num_written,
                nullptr);
    }
  }

  if (severity_ == LOG_FATAL) {
    // Write the log message to the global activity tracker, if running.
    ///winbase::debug::GlobalActivityTracker* tracker =
    ///    winbase::debug::GlobalActivityTracker::Get();
    ///if (tracker)
    ///  tracker->RecordLogMessage(str_newline);

    // Ensure the first characters of the string are on the stack so they
    // are contained in minidumps for diagnostic purposes.
    WINBASE_DEBUG_ALIAS_FOR_CSTR(str_stack, str_newline.c_str(), 1024);

    if (log_assert_handler_stack.IsCreated() &&
        !log_assert_handler_stack.Get().empty()) {
      LogAssertHandlerFunction log_assert_handler =
          log_assert_handler_stack.Get().top();

      if (log_assert_handler) {
        log_assert_handler.Run(
            file_, line_,
            winbase::StringPiece(str_newline.c_str() + message_start_,
                                 stack_start - message_start_),
            winbase::StringPiece(str_newline.c_str() + stack_start));
      }
    } else {
      // Don't use the string with the newline, get a fresh version to send to
      // the debug message process. We also don't display assertions to the
      // user in release mode. The enduser can't do anything with this
      // information, and displaying message boxes when the application is
      // hosed can cause additional problems.
#ifndef NDEBUG
      if (!winbase::debug::BeingDebugged()) {
        // Displaying a dialog is unnecessary when debugging and can complicate
        // debugging.
        DisplayDebugMessageInDialog(stream_.str());
      }
#endif
      // Crash the process to generate a dump.
#if defined(OFFICIAL_BUILD) && defined(NDEBUG)
      WINBASE_IMMEDIATE_CRASH();
#else
      winbase::debug::BreakDebugger();
#endif
    }
  }
}

// writes the common header info to the stream
void LogMessage::Init(const char* file, int line) {
  winbase::StringPiece filename(file);
  size_t last_slash_pos = filename.find_last_of("\\/");
  if (last_slash_pos != winbase::StringPiece::npos)
    filename.remove_prefix(last_slash_pos + 1);

  // TODO(darin): It might be nice if the columns were fixed width.

  stream_ <<  '[';
  if (g_log_process_id)
    stream_ << CurrentProcessId() << ':';
  if (g_log_thread_id)
    stream_ << winbase::PlatformThread::CurrentId() << ':';
  if (g_log_timestamp) {
    SYSTEMTIME local_time;
    GetLocalTime(&local_time);
    stream_ << std::setfill('0')
            << std::setw(2) << local_time.wMonth
            << std::setw(2) << local_time.wDay
            << '/'
            << std::setw(2) << local_time.wHour
            << std::setw(2) << local_time.wMinute
            << std::setw(2) << local_time.wSecond
            << '.'
            << std::setw(3)
            << local_time.wMilliseconds
            << ':';
  }
  if (g_log_tickcount)
    stream_ << TickCount() << ':';
  if (severity_ >= 0)
    stream_ << log_severity_name(severity_);
  else
    stream_ << "VERBOSE" << -severity_;

  stream_ << ":" << filename << "(" << line << ")] ";

  message_start_ = stream_.str().length();
}

// This has already been defined in the header, but defining it again as DWORD
// ensures that the type used in the header is equivalent to DWORD. If not,
// the redefinition is a compile error.
typedef DWORD SystemErrorCode;

SystemErrorCode GetLastSystemErrorCode() {
  return ::GetLastError();
}

WINBASE_EXPORT std::string SystemErrorCodeToString(SystemErrorCode error_code) {
  const int kErrorMessageBufferSize = 256;
  char msgbuf[kErrorMessageBufferSize];
  DWORD flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
  DWORD len = FormatMessageA(flags, nullptr, error_code, 0, msgbuf,
                             array_size(msgbuf), nullptr);
  if (len) {
    // Messages returned by system end with line breaks.
    return winbase::CollapseWhitespaceASCII(msgbuf, true) +
           winbase::StringPrintf(" (0x%lX)", error_code);
  }
  return winbase::StringPrintf("Error (0x%lX) while retrieving error. (0x%lX)",
                               GetLastError(), error_code);
}


Win32ErrorLogMessage::Win32ErrorLogMessage(const char* file,
                                           int line,
                                           LogSeverity severity,
                                           SystemErrorCode err)
    : err_(err),
      log_message_(file, line, severity) {
}

Win32ErrorLogMessage::~Win32ErrorLogMessage() {
  stream() << ": " << SystemErrorCodeToString(err_);
  // We're about to crash (CHECK). Put |err_| on the stack (by placing it in a
  // field) and use Alias in hopes that it makes it into crash dumps.
  DWORD last_error = err_;
  winbase::debug::Alias(&last_error);
}

void CloseLogFile() {
  CloseLogFileUnlocked();
}

void RawLog(int level, const char* message) {
  if (level >= g_min_log_level && message) {
    size_t bytes_written = 0;
    const size_t message_len = strlen(message);
    int rv;
    while (bytes_written < message_len) {
      rv = write(STDERR_FILENO, message + bytes_written,
                 message_len - bytes_written);
      if (rv < 0) {
        // Give up, nothing we can do now.
        break;
      }
      bytes_written += rv;
    }

    if (message_len > 0 && message[message_len - 1] != '\n') {
      do {
        rv = write(STDERR_FILENO, "\n", 1);
        if (rv < 0) {
          // Give up, nothing we can do now.
          break;
        }
      } while (rv != 1);
    }
  }

  if (level == LOG_FATAL)
    winbase::debug::BreakDebugger();
}

// This was defined at the beginning of this file.
#undef write

bool IsLoggingToFileEnabled() {
  return g_logging_destination & LOG_TO_FILE;
}

std::wstring GetLogFileFullPath() {
  if (g_log_file_name)
    return *g_log_file_name;
  return std::wstring();
}

WINBASE_EXPORT void LogErrorNotReached(const char* file, int line) {
  LogMessage(file, line, LOG_ERROR).stream()
      << "NOTREACHED() hit.";
}

}  // namespace logging
}  // namespace winbase

std::ostream& std::operator<<(std::ostream& out, const wchar_t* wstr) {
  return out << (wstr ? winbase::WideToUTF8(wstr) : std::string());
}