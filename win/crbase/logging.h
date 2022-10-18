// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_CRBASE_LOGGING_H_
#define MINI_CHROMIUM_CRBASE_LOGGING_H_

#include <stddef.h>

#include <cassert>
#include <cstring>
#include <sstream>
#include <string>

#include "crbase/crbase_export.h"
#include "crbase/debug/debugger.h"
#include "crbase/macros.h"
#include "crbuild/build_config.h"

//
// Optional message capabilities
// -----------------------------
// Assertion failed messages and fatal errors are displayed in a dialog box
// before the application exits. However, running this UI creates a message
// loop, which causes application messages to be processed and potentially
// dispatched to existing application windows. Since the application is in a
// bad state when this assertion dialog is displayed, these messages may not
// get processed and hang the dialog, or the application might go crazy.
//
// Therefore, it can be beneficial to display the error dialog in a separate
// process from the main application. When the logging system needs to display
// a fatal error dialog box, it will look for a program called
// "DebugMessage.exe" in the same directory as the application executable. It
// will run this application with the message as the command line, and will
// not include the name of the application as is traditional for easier
// parsing.
//
// The code for DebugMessage.exe is only one line. In WinMain, do:
//   MessageBox(NULL, GetCommandLineW(), L"Fatal Error", 0);
//
// If DebugMessage.exe is not found, the logging code will use a normal
// MessageBox, potentially causing the problems discussed above.


// Instructions
// ------------
//
// Make a bunch of macros for logging.  The way to log things is to stream
// things to LOG(<a particular severity level>).  E.g.,
//
//   CR_LOG(INFO) << "Found " << num_cookies << " cookies";
//
// You can also do conditional logging:
//
//   CR_LOG_IF(INFO, num_cookies > 10) << "Got lots of cookies";
//
// The CHECK(condition) macro is active in both debug and release builds and
// effectively performs a LOG(FATAL) which terminates the process and
// generates a crashdump unless a debugger is attached.
//
// There are also "debug mode" logging macros like the ones above:
//
//   CR_DLOG(INFO) << "Found cookies";
//
//   CR_DLOG_IF(INFO, num_cookies > 10) << "Got lots of cookies";
//
// All "debug mode" logging is compiled away to nothing for non-debug mode
// compiles.  LOG_IF and development flags also work well together
// because the code can be compiled away sometimes.
//
// We also have
//
//   CR_LOG_ASSERT(assertion);
//   CR_DLOG_ASSERT(assertion);
//
// which is syntactic sugar for {,D}CR_LOG_IF(FATAL, assert fails) << assertion;
//
// There are "verbose level" logging macros.  They look like
//
//   CR_VLOG(1) << "I'm printed when you run the program with --v=1 or more";
//   CR_VLOG(2) << "I'm printed when you run the program with --v=2 or more";
//
// These always log at the INFO log level (when they log at all).
// The verbose logging can also be turned on module-by-module.  For instance,
//    --vmodule=profile=2,icon_loader=1,browser_*=3,*/chromeos/*=4 --v=0
// will cause:
//   a. CR_VLOG(2) and lower messages to be printed from profile.{h,cc}
//   b. CR_VLOG(1) and lower messages to be printed from icon_loader.{h,cc}
//   c. CR_VLOG(3) and lower messages to be printed from files prefixed with
//      "browser"
//   d. CR_VLOG(4) and lower messages to be printed from files under a
//     "chromeos" directory.
//   e. CR_VLOG(0) and lower messages to be printed from elsewhere
//
// The wildcarding functionality shown by (c) supports both '*' (match
// 0 or more characters) and '?' (match any single character)
// wildcards.  Any pattern containing a forward or backward slash will
// be tested against the whole pathname and not just the module.
// E.g., "*/foo/bar/*=2" would change the logging level for all code
// in source files under a "foo/bar" directory.
//
// There's also CR_VLOG_IS_ON(n) "verbose level" condition macro. To be used as
//
//   if (CR_VLOG_IS_ON(2)) {
//     // do some logging preparation and logging
//     // that can't be accomplished with just VLOG(2) << ...;
//   }
//
// There is also a CR_VLOG_IF "verbose level" condition macro for sample
// cases, when some extra computation and preparation for logs is not
// needed.
//
//   CR_VLOG_IF(1, (size > 1024))
//      << "I'm printed when size is more than 1024 and when you run the "
//         "program with --v=1 or more";
//
// We also override the standard 'assert' to use 'DLOG_ASSERT'.
//
// Lastly, there is:
//
//   CR_PLOG(ERROR) << "Couldn't do foo";
//   CR_DPLOG(ERROR) << "Couldn't do foo";
//   CR_PLOG_IF(ERROR, cond) << "Couldn't do foo";
//   CR_DPLOG_IF(ERROR, cond) << "Couldn't do foo";
//   CR_PCHECK(condition) << "Couldn't do foo";
//   CR_DPCHECK(condition) << "Couldn't do foo";
//
// which append the last system error to the message in string form (taken from
// GetLastError() on Windows and errno on POSIX).
//
// The supported severity levels for macros that allow you to specify one
// are (in increasing order of severity) INFO, WARNING, ERROR, and FATAL.
//
// Very important: logging a message at the FATAL severity level causes
// the program to terminate (after the message is logged).
//
// There is the special severity of DFATAL, which logs FATAL in debug mode,
// ERROR in normal mode.

namespace crlogging {

// TODO(avi): do we want to do a unification of character types here?
typedef wchar_t PathChar;

// Where to record logging output? A flat file and/or system debug log
// via OutputDebugString.
enum LoggingDestination {
  LOG_NONE                = 0,
  LOG_TO_FILE             = 1 << 0,
  LOG_TO_SYSTEM_DEBUG_LOG = 1 << 1,

  LOG_TO_ALL = LOG_TO_FILE | LOG_TO_SYSTEM_DEBUG_LOG,

  // On Windows, use a file next to the exe; on POSIX platforms, where
  // it may not even be possible to locate the executable on disk, use
  // stderr.
  LOG_DEFAULT = LOG_TO_FILE,
};

// Indicates that the log file should be locked when being written to.
// Unless there is only one single-threaded process that is logging to
// the log file, the file should be locked during writes to make each
// log output atomic. Other writers will block.
//
// All processes writing to the log file must have their locking set for it to
// work properly. Defaults to LOCK_LOG_FILE.
enum LogLockingState { LOCK_LOG_FILE, DONT_LOCK_LOG_FILE };

// On startup, should we delete or append to an existing log file (if any)?
// Defaults to APPEND_TO_OLD_LOG_FILE.
enum OldFileDeletionState { DELETE_OLD_LOG_FILE, APPEND_TO_OLD_LOG_FILE };

struct CRBASE_EXPORT LoggingSettings {
  // The defaults values are:
  //
  //  logging_dest: LOG_DEFAULT
  //  log_file:     NULL
  //  lock_log:     LOCK_LOG_FILE
  //  delete_old:   APPEND_TO_OLD_LOG_FILE
  LoggingSettings();

  LoggingDestination logging_dest;

  // The three settings below have an effect only when LOG_TO_FILE is
  // set in |logging_dest|.
  const PathChar* log_file;
  LogLockingState lock_log;
  OldFileDeletionState delete_old;
};

// Define different names for the BaseInitLoggingImpl() function depending on
// whether NDEBUG is defined or not so that we'll fail to link if someone tries
// to compile logging.cc with NDEBUG but includes logging.h without defining it,
// or vice versa.
#if NDEBUG
#define CrBaseInitLoggingImpl BaseInitLoggingImpl_built_with_NDEBUG
#else
#define CrBaseInitLoggingImpl BaseInitLoggingImpl_built_without_NDEBUG
#endif

// Implementation of the InitLogging() method declared below.  We use a
// more-specific name so we can #define it above without affecting other code
// that has named stuff "InitLogging".
CRBASE_EXPORT bool CrBaseInitLoggingImpl(const LoggingSettings& settings);

// Sets the log file name and other global logging state. Calling this function
// is recommended, and is normally done at the beginning of application init.
// If you don't call it, all the flags will be initialized to their default
// values, and there is a race condition that may leak a critical section
// object if two threads try to do the first log at the same time.
// See the definition of the enums above for descriptions and default values.
//
// The default log file is initialized to "debug.log" in the application
// directory. You probably don't want this, especially since the program
// directory may not be writable on an enduser's system.
//
// This function may be called a second time to re-direct logging (e.g after
// loging in to a user partition), however it should never be called more than
// twice.
inline bool InitLogging(const LoggingSettings& settings) {
  return CrBaseInitLoggingImpl(settings);
}

// Sets the log level. Anything at or above this level will be written to the
// log file/displayed to the user (if applicable). Anything below this level
// will be silently ignored. The log level defaults to 0 (everything is logged
// up to level INFO) if this function is not called.
// Note that log messages for VLOG(x) are logged at level -x, so setting
// the min log level to negative values enables verbose logging.
CRBASE_EXPORT void SetMinLogLevel(int level);

// Gets the current log level.
CRBASE_EXPORT int GetMinLogLevel();

// Used by CR_LOG_IS_ON to lazy-evaluate stream arguments.
CRBASE_EXPORT bool ShouldCreateLogMessage(int severity);

// Gets the CR_VLOG default verbosity level.
CRBASE_EXPORT int GetVlogVerbosity();

// Gets the current vlog level for the given file (usually taken from
// __FILE__).

// Note that |N| is the size *with* the null terminator.
CRBASE_EXPORT int GetVlogLevelHelper(const char* file_start, size_t N);

template <size_t N>
int GetVlogLevel(const char (&file)[N]) {
  return GetVlogLevelHelper(file, N);
}

// Sets the common items you want to be prepended to each log message.
// process and thread IDs default to off, the timestamp defaults to on.
// If this function is not called, logging defaults to writing the timestamp
// only.
CRBASE_EXPORT void SetLogItems(bool enable_process_id, bool enable_thread_id,
                               bool enable_timestamp, bool enable_tickcount);

// Sets whether or not you'd like to see fatal debug messages popped up in
// a dialog box or not.
// Dialogs are not shown by default.
CRBASE_EXPORT void SetShowErrorDialogs(bool enable_dialogs);

// Sets the Log Assert Handler that will be used to notify of check failures.
// The default handler shows a dialog box and then terminate the process,
// however clients can use this function to override with their own handling
// (e.g. a silent one for Unit Tests)
typedef void (*LogAssertHandlerFunction)(const std::string& str);
CRBASE_EXPORT void SetLogAssertHandler(LogAssertHandlerFunction handler);

// Sets the Log Message Handler that gets passed every log message before
// it's sent to other log destinations (if any).
// Returns true to signal that it handled the message and the message
// should not be sent to other log destinations.
typedef bool (*LogMessageHandlerFunction)(int severity,
    const char* file, int line, size_t message_start, const std::string& str);
CRBASE_EXPORT void SetLogMessageHandler(LogMessageHandlerFunction handler);
CRBASE_EXPORT LogMessageHandlerFunction GetLogMessageHandler();

typedef int LogSeverity;
const LogSeverity LOG_VERBOSE = -1;  // This is level 1 verbosity
// Note: the log severities are used to index into the array of names,
// see log_severity_names.
const LogSeverity LOG_INFO = 0;
const LogSeverity LOG_WARNING = 1;
const LogSeverity LOG_ERROR = 2;
const LogSeverity LOG_FATAL = 3;
const LogSeverity LOG_NUM_SEVERITIES = 4;

// LOG_DFATAL is LOG_FATAL in debug mode, ERROR in normal mode
#ifdef NDEBUG
const LogSeverity LOG_DFATAL = LOG_ERROR;
#else
const LogSeverity LOG_DFATAL = LOG_FATAL;
#endif

// A few definitions of macros that don't generate much code. These are used
// by LOG() and LOG_IF, etc. Since these are used all over our code, it's
// better to have compact code for these operations.
#define COMPACT_CHROMIUM_LOG_EX_INFO(ClassName, ...)                  \
  crlogging::ClassName(__FILE__, __LINE__, crlogging::LOG_INFO ,    \
                        ##__VA_ARGS__)
#define COMPACT_CHROMIUM_LOG_EX_WARNING(ClassName, ...)               \
  crlogging::ClassName(__FILE__, __LINE__, crlogging::LOG_WARNING , \
                        ##__VA_ARGS__)
#define COMPACT_CHROMIUM_LOG_EX_ERROR(ClassName, ...)                 \
  crlogging::ClassName(__FILE__, __LINE__, crlogging::LOG_ERROR ,   \
                        ##__VA_ARGS__)
#define COMPACT_CHROMIUM_LOG_EX_FATAL(ClassName, ...)                 \
  crlogging::ClassName(__FILE__, __LINE__, crlogging::LOG_FATAL ,   \
                        ##__VA_ARGS__)
#define COMPACT_CHROMIUM_LOG_EX_DFATAL(ClassName, ...)                \
  crlogging::ClassName(__FILE__, __LINE__, crlogging::LOG_DFATAL ,  \
                        ##__VA_ARGS__)

#define COMPACT_CHROMIUM_LOG_INFO \
  COMPACT_CHROMIUM_LOG_EX_INFO(LogMessage)
#define COMPACT_CHROMIUM_LOG_WARNING \
  COMPACT_CHROMIUM_LOG_EX_WARNING(LogMessage)
#define COMPACT_CHROMIUM_LOG_ERROR \
  COMPACT_CHROMIUM_LOG_EX_ERROR(LogMessage)
#define COMPACT_CHROMIUM_LOG_FATAL \
  COMPACT_CHROMIUM_LOG_EX_FATAL(LogMessage)
#define COMPACT_CHROMIUM_LOG_DFATAL \
  COMPACT_CHROMIUM_LOG_EX_DFATAL(LogMessage)

// wingdi.h defines ERROR to be 0. When we call DR_LOG(ERROR), it gets
// substituted with 0, and it expands to COMPACT_CHROMIUM_LOG_0. To allow us
// to keep using this syntax, we define this macro to do the same thing
// as COMPACT_CHROMIUM_LOG_ERROR, and also define ERROR the same way that
// the Windows SDK does for consistency.
#define ERROR 0
#define COMPACT_CHROMIUM_LOG_EX_0(ClassName, ...) \
  COMPACT_CHROMIUM_LOG_EX_ERROR(ClassName , ##__VA_ARGS__)
#define COMPACT_CHROMIUM_LOG_0 COMPACT_CHROMIUM_LOG_ERROR
// Needed for LOG_IS_ON(ERROR).
const LogSeverity LOG_0 = LOG_ERROR;

// As special cases, we can assume that CR_LOG_IS_ON(FATAL) always holds. Also,
// CR_LOG_IS_ON(DFATAL) always holds in debug mode. In particular, 
// CR_CHECK()s will always fire if they fail.
#define CR_LOG_IS_ON(severity) \
  (::crlogging::ShouldCreateLogMessage(::crlogging::LOG_##severity))

// We can't do any caching tricks with CR_VLOG_IS_ON() like the
// google-glog version since it requires GCC extensions.  This means
// that using the v-logging functions in conjunction with --vmodule
// may be slow.
#define CR_VLOG_IS_ON(verboselevel) \
  ((verboselevel) <= ::crlogging::GetVlogLevel(__FILE__))

// Helper macro which avoids evaluating the arguments to a stream if
// the condition doesn't hold. Condition is evaluated once and only once.
#define CR_LAZY_STREAM(stream, condition)                                  \
  !(condition) ? (void) 0 : ::crlogging::LogMessageVoidify() & (stream)

// We use the preprocessor's merging operator, "##", so that, e.g.,
// LOG(INFO) becomes the token COMPACT_CHROMIUM_LOG_INFO.  There's some funny
// subtle difference between ostream member streaming functions (e.g.,
// ostream::operator<<(int) and ostream non-member streaming functions
// (e.g., ::operator<<(ostream&, string&): it turns out that it's
// impossible to stream something like a string directly to an unnamed
// ostream. We employ a neat hack by calling the stream() member
// function of LogMessage which seems to avoid the problem.
#define CR_LOG_STREAM(severity) COMPACT_CHROMIUM_LOG_ ## severity.stream()

#define CR_LOG(severity) \
  CR_LAZY_STREAM(CR_LOG_STREAM(severity), CR_LOG_IS_ON(severity))
#define CR_LOG_IF(severity, condition) \
  CR_LAZY_STREAM(CR_LOG_STREAM(severity), CR_LOG_IS_ON(severity) && (condition))

#define CR_SYSLOG(severity) CR_LOG(severity)
#define CR_SYSLOG_IF(severity, condition) CR_LOG_IF(severity, condition)

// The VLOG macros log with negative verbosities.
#define CR_VLOG_STREAM(verbose_level) \
  crlogging::LogMessage(__FILE__, __LINE__, -verbose_level).stream()

#define CR_VLOG(verbose_level) \
  CR_LAZY_STREAM(CR_VLOG_STREAM(verbose_level), CR_VLOG_IS_ON(verbose_level))

#define CR_VLOG_IF(verbose_level, condition) \
  CR_LAZY_STREAM(CR_VLOG_STREAM(verbose_level), \
      CR_VLOG_IS_ON(verbose_level) && (condition))

#define CR_VPLOG_STREAM(verbose_level) \
  crlogging::Win32ErrorLogMessage(__FILE__, __LINE__, -verbose_level, \
    ::crlogging::GetLastSystemErrorCode()).stream()

#define CR_VPLOG(verbose_level) \
  CR_LAZY_STREAM(CR_VPLOG_STREAM(verbose_level), CR_VLOG_IS_ON(verbose_level))

#define CR_VPLOG_IF(verbose_level, condition) \
  CR_LAZY_STREAM(VPLOG_STREAM(verbose_level), \
    CR_VLOG_IS_ON(verbose_level) && (condition))

// TODO(akalin): Add more VLOG variants, e.g. VPLOG.

#define CR_LOG_ASSERT(condition)  \
  CR_LOG_IF(FATAL, !(condition)) << "Assert failed: " #condition ". "
#define CR_SYSLOG_ASSERT(condition) \
  CR_SYSLOG_IF(FATAL, !(condition)) << "Assert failed: " #condition ". "

#define CR_PLOG_STREAM(severity) \
  COMPACT_CHROMIUM_LOG_EX_ ## severity(Win32ErrorLogMessage, \
      ::crlogging::GetLastSystemErrorCode()).stream()

#define CR_PLOG(severity)                                          \
  CR_LAZY_STREAM(CR_PLOG_STREAM(severity), CR_LOG_IS_ON(severity))

#define CR_PLOG_IF(severity, condition) \
  CR_LAZY_STREAM(CR_PLOG_STREAM(severity), \
                 CR_LOG_IS_ON(severity) && (condition))

// The actual stream used isn't important.
#define CR_EAT_STREAM_PARAMETERS                                        \
  true ? (void) 0 : ::crlogging::LogMessageVoidify() & CR_LOG_STREAM(FATAL)

// Captures the result of a CR_CHECK_EQ (for example) and facilitates testing
// as a boolean.
class CheckOpResult {
 public:
  // |message| must be null if and only if the check failed.
  CheckOpResult(std::string* message) : message_(message) {}
  // Returns true if the check succeeded.
  operator bool() const { return !message_; }
  // Returns the message.
  std::string* message() { return message_; }

 private:
  std::string* message_;
};

// CR_CHECK dies with a fatal error if condition is not true.  It is *not*
// controlled by NDEBUG, so the check will be executed regardless of
// compilation mode.
//
// We make sure CR_CHECK et al. always evaluates their arguments, as
// doing CR_CHECK(FunctionWithSideEffect()) is a common idiom.

// Do as much work as possible out of line to reduce inline code size.
#define CR_CHECK(condition)                                                    \
  CR_LAZY_STREAM(crlogging::LogMessage(                                       \
                   __FILE__, __LINE__, #condition).stream(),                   \
                 !(condition))

#define CR_PCHECK(condition)                       \
  CR_LAZY_STREAM(CR_PLOG_STREAM(FATAL), !(condition)) \
  << "Check failed: " #condition ". "

// Helper macro for binary operators.
// Don't use this macro directly in your code, use CR_CHECK_EQ et al below.
// The 'switch' is used to prevent the 'else' from being ambiguous when the
// macro is used in an 'if' clause such as:
// if (a == 1)
//   CR_CHECK_EQ(2, a);
#define CR_CHECK_OP(name, op, val1, val2)                                     \
  switch (0) case 0: default:                                                 \
  if (crlogging::CheckOpResult true_if_passed =                               \
      crlogging::Check##name##Impl((val1), (val2),                            \
                                 #val1 " " #op " " #val2))                    \
   ;                                                                          \
  else                                                                        \
    crlogging::LogMessage(__FILE__, __LINE__,                                 \
                           true_if_passed.message()).stream()

// Build the error message string.  This is separate from the "Impl"
// function template because it is not performance critical and so can
// be out of line, while the "Impl" code should be inline.  Caller
// takes ownership of the returned string.
template<class t1, class t2>
std::string* MakeCheckOpString(const t1& v1, const t2& v2, const char* names) {
  std::ostringstream ss;
  ss << names << " (" << v1 << " vs. " << v2 << ")";
  std::string* msg = new std::string(ss.str());
  return msg;
}

// Commonly used instantiations of MakeCheckOpString<>. Explicitly instantiated
// in logging.cc.
extern template CRBASE_EXPORT std::string* MakeCheckOpString<int, int>(
    const int&, const int&, const char* names);
extern template CRBASE_EXPORT
std::string* MakeCheckOpString<unsigned long, unsigned long>(
    const unsigned long&, const unsigned long&, const char* names);
extern template CRBASE_EXPORT
std::string* MakeCheckOpString<unsigned long, unsigned int>(
    const unsigned long&, const unsigned int&, const char* names);
extern template CRBASE_EXPORT
std::string* MakeCheckOpString<unsigned int, unsigned long>(
    const unsigned int&, const unsigned long&, const char* names);
extern template CRBASE_EXPORT
std::string* MakeCheckOpString<std::string, std::string>(
    const std::string&, const std::string&, const char* name);

// Helper functions for CR_CHECK_OP macro.
// The (int, int) specialization works around the issue that the compiler
// will not instantiate the template version of the function on values of
// unnamed enum type - see comment below.
#define DEFINE_CHECK_OP_IMPL(name, op) \
  template <class t1, class t2> \
  inline std::string* Check##name##Impl(const t1& v1, const t2& v2, \
                                        const char* names) { \
    if (v1 op v2) return NULL; \
    else return MakeCheckOpString(v1, v2, names); \
  } \
  inline std::string* Check##name##Impl(int v1, int v2, const char* names) { \
    if (v1 op v2) return NULL; \
    else return MakeCheckOpString(v1, v2, names); \
  }
DEFINE_CHECK_OP_IMPL(EQ, ==)
DEFINE_CHECK_OP_IMPL(NE, !=)
DEFINE_CHECK_OP_IMPL(LE, <=)
DEFINE_CHECK_OP_IMPL(LT, < )
DEFINE_CHECK_OP_IMPL(GE, >=)
DEFINE_CHECK_OP_IMPL(GT, > )
#undef DEFINE_CHECK_OP_IMPL

#define CR_CHECK_EQ(val1, val2) CR_CHECK_OP(EQ, ==, val1, val2)
#define CR_CHECK_NE(val1, val2) CR_CHECK_OP(NE, !=, val1, val2)
#define CR_CHECK_LE(val1, val2) CR_CHECK_OP(LE, <=, val1, val2)
#define CR_CHECK_LT(val1, val2) CR_CHECK_OP(LT, < , val1, val2)
#define CR_CHECK_GE(val1, val2) CR_CHECK_OP(GE, >=, val1, val2)
#define CR_CHECK_GT(val1, val2) CR_CHECK_OP(GT, > , val1, val2)

#if defined(NDEBUG)
#define ENABLE_CR_DLOG 0
#else
#define ENABLE_CR_DLOG 1
#endif

#if defined(NDEBUG) && !defined(CR_DCHECK_ALWAYS_ON)
#define CR_DCHECK_IS_ON() 0
#else
#define CR_DCHECK_IS_ON() 1
#endif

// Definitions for DLOG et al.

#if ENABLE_CR_DLOG

#define CR_DLOG_IS_ON(severity) CR_LOG_IS_ON(severity)
#define CR_DLOG_IF(severity, condition) CR_LOG_IF(severity, condition)
#define CR_DLOG_ASSERT(condition) CR_LOG_ASSERT(condition)
#define CR_DPLOG_IF(severity, condition) CR_PLOG_IF(severity, condition)
#define CR_DVLOG_IF(verboselevel, condition) CR_VLOG_IF(verboselevel, condition)
#define CR_DVPLOG_IF(verboselevel, condition) CR_VPLOG_IF(verboselevel, condition)

#else  // ENABLE_CR_DLOG

// If ENABLE_DLOG is off, we want to avoid emitting any references to
// |condition| (which may reference a variable defined only if NDEBUG
// is not defined).  Contrast this with DCHECK et al., which has
// different behavior.

#define CR_DLOG_IS_ON(severity) false
#define CR_DLOG_IF(severity, condition) CR_EAT_STREAM_PARAMETERS
#define CR_DLOG_ASSERT(condition) CR_EAT_STREAM_PARAMETERS
#define CR_DPLOG_IF(severity, condition) CR_EAT_STREAM_PARAMETERS
#define CR_DVLOG_IF(verboselevel, condition) CR_EAT_STREAM_PARAMETERS
#define CR_DVPLOG_IF(verboselevel, condition) CR_EAT_STREAM_PARAMETERS

#endif  // ENABLE_CR_DLOG

// DEBUG_MODE is for uses like
//   if (DEBUG_MODE) foo.CheckThatFoo();
// instead of
//   #ifndef NDEBUG
//     foo.CheckThatFoo();
//   #endif
//
// We tie its state to ENABLE_CR_DLOG.
enum { DEBUG_MODE = ENABLE_CR_DLOG};

#undef ENABLE_CR_DLOG

#define CR_DLOG(severity)                                          \
  CR_LAZY_STREAM(CR_LOG_STREAM(severity), CR_DLOG_IS_ON(severity))

#define CR_DPLOG(severity)                                          \
  CR_LAZY_STREAM(CR_PLOG_STREAM(severity), CR_DLOG_IS_ON(severity))

#define CR_DVLOG(verboselevel)                        \
  CR_DVLOG_IF(verboselevel, CR_VLOG_IS_ON(verboselevel))

#define CR_DVPLOG(verboselevel)                       \
  CR_DVPLOG_IF(verboselevel, CR_VLOG_IS_ON(verboselevel))

// Definitions for CR_DCHECK et al.

#if CR_DCHECK_IS_ON()

#define COMPACT_CHROMIUM_LOG_EX_DCHECK(ClassName, ...) \
  COMPACT_CHROMIUM_LOG_EX_FATAL(ClassName , ##__VA_ARGS__)
#define COMPACT_CHROMIUM_LOG_DCHECK COMPACT_CHROMIUM_LOG_FATAL
const LogSeverity LOG_DCHECK = LOG_FATAL;

#else  // CR_DCHECK_IS_ON()

// These are just dummy values.
#define COMPACT_CHROMIUM_LOG_EX_DCHECK(ClassName, ...) \
  COMPACT_CHROMIUM_LOG_EX_INFO(ClassName , ##__VA_ARGS__)
#define COMPACT_CHROMIUM_LOG_DCHECK COMPACT_CHROMIUM_LOG_INFO
const LogSeverity LOG_DCHECK = LOG_INFO;

#endif  // CR_DCHECK_IS_ON()

// CR_DCHECK et al. make sure to reference |condition| regardless of
// whether CR_DCHECKs are enabled; this is so that we don't get unused
// variable warnings if the only use of a variable is in a CR_DCHECK.
// This behavior is different from CR_DLOG_IF et al.
#define CR_DCHECK(condition)                                                \
  CR_LAZY_STREAM(CR_LOG_STREAM(DCHECK),                                     \
                 CR_DCHECK_IS_ON() ? !(condition) : false)                  \
      << "Check failed: " #condition ". "

#define CR_DPCHECK(condition)                                                \
  CR_LAZY_STREAM(CR_PLOG_STREAM(DCHECK),                                     \
                 CR_DCHECK_IS_ON() ? !(condition) : false)                   \
      << "Check failed: " #condition ". "

// Helper macro for binary operators.
// Don't use this macro directly in your code, use CR_DCHECK_EQ et al below.
// The 'switch' is used to prevent the 'else' from being ambiguous when the
// macro is used in an 'if' clause such as:
// if (a == 1)
//   CR_DCHECK_EQ(2, a);
#define CR_DCHECK_OP(name, op, val1, val2)                               \
  switch (0) case 0: default:                                            \
  if (crlogging::CheckOpResult true_if_passed =                          \
      CR_DCHECK_IS_ON() ?                                                \
      crlogging::Check##name##Impl((val1), (val2),                       \
                                    #val1 " " #op " " #val2) : nullptr)  \
   ;                                                                     \
  else                                                                   \
    crlogging::LogMessage(__FILE__, __LINE__, ::crlogging::LOG_DCHECK,   \
                           true_if_passed.message()).stream()

// Equality/Inequality checks - compare two values, and log a
// CR_LOG_DCHECK message including the two values when the result is not
// as expected.  The values must have operator<<(ostream, ...)
// defined.
//
// You may append to the error message like so:
//   CR_DCHECK_NE(1, 2) << ": The world must be ending!";
//
// We are very careful to ensure that each argument is evaluated exactly
// once, and that anything which is legal to pass as a function argument is
// legal here.  In particular, the arguments may be temporary expressions
// which will end up being destroyed at the end of the apparent statement,
// for example:
//   CR_DCHECK_EQ(string("abc")[1], 'b');
//
// WARNING: These may not compile correctly if one of the arguments is a pointer
// and the other is NULL. To work around this, simply static_cast NULL to the
// type of the desired pointer.

#define CR_DCHECK_EQ(val1, val2) CR_DCHECK_OP(EQ, ==, val1, val2)
#define CR_DCHECK_NE(val1, val2) CR_DCHECK_OP(NE, !=, val1, val2)
#define CR_DCHECK_LE(val1, val2) CR_DCHECK_OP(LE, <=, val1, val2)
#define CR_DCHECK_LT(val1, val2) CR_DCHECK_OP(LT, < , val1, val2)
#define CR_DCHECK_GE(val1, val2) CR_DCHECK_OP(GE, >=, val1, val2)
#define CR_DCHECK_GT(val1, val2) CR_DCHECK_OP(GT, > , val1, val2)

#define CR_NOTREACHED() CR_DCHECK(false)

// Redefine the standard assert to use our nice log files
#undef assert
#define assert(x) CR_DLOG_ASSERT(x)

// This class more or less represents a particular log message.  You
// create an instance of LogMessage and then stream stuff to it.
// When you finish streaming to it, ~LogMessage is called and the
// full message gets streamed to the appropriate destination.
//
// You shouldn't actually use LogMessage's constructor to log things,
// though.  You should use the LOG() macro (and variants thereof)
// above.
class CRBASE_EXPORT LogMessage {
 public:
  // Used for CR_LOG(severity).
  LogMessage(const char* file, int line, LogSeverity severity);

  // Used for CR_CHECK().  Implied severity = LOG_FATAL.
  LogMessage(const char* file, int line, const char* condition);

  // Used for CR_CHECK_EQ(), etc. Takes ownership of the given string.
  // Implied severity = LOG_FATAL.
  LogMessage(const char* file, int line, std::string* result);

  // Used for CR_DCHECK_EQ(), etc. Takes ownership of the given string.
  LogMessage(const char* file, int line, LogSeverity severity,
             std::string* result);

  ~LogMessage();

  std::ostream& stream() { return stream_; }

 private:
  void Init(const char* file, int line);

  LogSeverity severity_;
  std::ostringstream stream_;
  size_t message_start_;  // Offset of the start of the message (past prefix
                          // info).
  // The file and line information passed in to the constructor.
  const char* file_;
  const int line_;

  // Stores the current value of GetLastError in the constructor and restores
  // it in the destructor by calling SetLastError.
  // This is useful since the LogMessage class uses a lot of Win32 calls
  // that will lose the value of GLE and the code that called the log function
  // will have lost the thread error value when the log call returns.
  class SaveLastError {
   public:
    SaveLastError();
    ~SaveLastError();

    unsigned long get_error() const { return last_error_; }

   protected:
    unsigned long last_error_;
  };

  SaveLastError last_error_;

  CR_DISALLOW_COPY_AND_ASSIGN(LogMessage);
};

// A non-macro interface to the log facility; (useful
// when the logging level is not a compile-time constant).
inline void LogAtLevel(int log_level, const std::string& msg) {
  LogMessage(__FILE__, __LINE__, log_level).stream() << msg;
}

// This class is used to explicitly ignore values in the conditional
// logging macros.  This avoids compiler warnings like "value computed
// is not used" and "statement has no effect".
class LogMessageVoidify {
 public:
  LogMessageVoidify() { }
  // This has to be an operator with a precedence lower than << but
  // higher than ?:
  void operator&(std::ostream&) { }
};

typedef unsigned long SystemErrorCode;

// Alias for ::GetLastError() on Windows and errno on POSIX. Avoids having to
// pull in windows.h just for GetLastError() and DWORD.
CRBASE_EXPORT SystemErrorCode GetLastSystemErrorCode();
CRBASE_EXPORT std::string SystemErrorCodeToString(SystemErrorCode error_code);

// Appends a formatted system message of the GetLastError() type.
class CRBASE_EXPORT Win32ErrorLogMessage {
 public:
  Win32ErrorLogMessage(const char* file,
                       int line,
                       LogSeverity severity,
                       SystemErrorCode err);

  // Appends the error message before destructing the encapsulated class.
  ~Win32ErrorLogMessage();

  std::ostream& stream() { return log_message_.stream(); }

 private:
  SystemErrorCode err_;
  LogMessage log_message_;

  CR_DISALLOW_COPY_AND_ASSIGN(Win32ErrorLogMessage);
};

// Closes the log file explicitly if open.
// NOTE: Since the log file is opened as necessary by the action of logging
//       statements, there's no guarantee that it will stay closed
//       after this call.
CRBASE_EXPORT void CloseLogFile();

// Async signal safe logging mechanism.
CRBASE_EXPORT void RawLog(int level, const char* message);

#define CR_RAW_LOG(level, message) \
  crlogging::RawLog(logging::LOG_ ## level, message)

#define CR_RAW_CHECK(condition)                                                \
  do {                                                                         \
    if (!(condition))                                                          \
      crlogging::RawLog(crlogging::LOG_FATAL,                                  \
                         "Check failed: " #condition "\n");                    \
  } while (0)

// Returns true if logging to file is enabled.
CRBASE_EXPORT bool IsLoggingToFileEnabled();

// Returns the default log file path.
CRBASE_EXPORT std::wstring GetLogFileFullPath();

}  // namespace crlogging

// Note that "The behavior of a C++ program is undefined if it adds declarations
// or definitions to namespace std or to a namespace within namespace std unless
// otherwise specified." --C++11[namespace.std]
//
// We've checked that this particular definition has the intended behavior on
// our implementations, but it's prone to breaking in the future, and please
// don't imitate this in your own definitions without checking with some
// standard library experts.
namespace std {
// These functions are provided as a convenience for logging, which is where we
// use streams (it is against Google style to use streams in other places). It
// is designed to allow you to emit non-ASCII Unicode strings to the log file,
// which is normally ASCII. It is relatively slow, so try not to use it for
// common cases. Non-ASCII characters will be converted to UTF-8 by these
// operators.
CRBASE_EXPORT std::ostream& operator<<(std::ostream& out, const wchar_t* wstr);
inline std::ostream& operator<<(std::ostream& out, const std::wstring& wstr) {
  return out << wstr.c_str();
}
}  // namespace std

// The CR_NOTIMPLEMENTED() macro annotates codepaths which have
// not been implemented yet.
//
// The implementation of this macro is controlled by CR_NOTIMPLEMENTED_POLICY:
//   0 -- Do nothing (stripped by compiler)
//   1 -- Warn at compile time
//   2 -- Fail at compile time
//   3 -- Fail at runtime (CR_DCHECK)
//   4 -- [default] CR_LOG(ERROR) at runtime
//   5 -- CR_LOG(ERROR) at runtime, only once per call-site

#ifndef CR_NOTIMPLEMENTED_POLICY
// Select default policy: CR_LOG(ERROR)
#define CR_NOTIMPLEMENTED_POLICY 4
#endif

#if defined(MINI_CHROMIUM_COMPILER_GCC)
// On Linux, with GCC, we can use __PRETTY_FUNCTION__ to get the demangled name
// of the current function in the NOTIMPLEMENTED message.
#define CR_NOTIMPLEMENTED_MSG \
  "Not implemented reached in " << __PRETTY_FUNCTION__
#else
#define CR_NOTIMPLEMENTED_MSG "NOT IMPLEMENTED"
#endif

#if CR_NOTIMPLEMENTED_POLICY == 0
#define CR_NOTIMPLEMENTED() CR_EAT_STREAM_PARAMETERS
#elif CR_NOTIMPLEMENTED_POLICY == 1
// TODO, figure out how to generate a warning
#define NOTIMPLEMENTED() static_assert(false, "NOT_IMPLEMENTED")
#elif CR_NOTIMPLEMENTED_POLICY == 2
#define CR_NOTIMPLEMENTED() static_assert(false, "NOT_IMPLEMENTED")
#elif CR_NOTIMPLEMENTED_POLICY == 3
#define CR_NOTIMPLEMENTED() CR_NOTREACHED()
#elif CR_NOTIMPLEMENTED_POLICY == 4
#define CR_NOTIMPLEMENTED() CR_LOG(ERROR) << CR_NOTIMPLEMENTED_MSG
#elif CR_NOTIMPLEMENTED_POLICY == 5
#define CR_NOTIMPLEMENTED() do {\
  static bool logged_once = false;\
  CR_LOG_IF(ERROR, !logged_once) << CR_NOTIMPLEMENTED_MSG;\
  logged_once = true;\
} while(0);\
CR_EAT_STREAM_PARAMETERS
#endif

#endif  // MINI_CHROMIUM_CRBASE_LOGGING_H_