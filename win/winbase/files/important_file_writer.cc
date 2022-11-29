// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "winbase\files\important_file_writer.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <utility>

#include "winbase\functional\bind.h"
#include "winbase\functional\critical_closure.h"
#include "winbase\functional\callback_helpers.h"
#include "winbase\debug\alias.h"
#include "winbase\files\file.h"
#include "winbase\files\file_path.h"
#include "winbase\files\file_util.h"
#include "winbase\logging.h"
#include "winbase\macros.h"
///#include "winbase\metrics\histogram_functions.h"
///#include "winbase\metrics\histogram_macros.h"
#include "winbase\numerics\safe_conversions.h"
#include "winbase\strings\string_number_conversions.h"
#include "winbase\strings\string_util.h"
#include "winbase\strings\string_piece.h"
#include "winbase\task_runner.h"
#include "winbase\task_runner_util.h"
#include "winbase\threading\thread.h"
#include "winbase\time\time.h"

namespace winbase {

namespace {

constexpr auto kDefaultCommitInterval = TimeDelta::FromSeconds(10);

// This enum is used to define the buckets for an enumerated UMA histogram.
// Hence,
//   (a) existing enumerated constants should never be deleted or reordered, and
//   (b) new constants should only be appended at the end of the enumeration.
enum TempFileFailure {
  FAILED_CREATING,
  FAILED_OPENING,
  FAILED_CLOSING,  // Unused.
  FAILED_WRITING,
  FAILED_RENAMING,
  FAILED_FLUSHING,
  TEMP_FILE_FAILURE_MAX
};

// Helper function to write samples to a histogram with a dynamically assigned
// histogram name.  Works with different error code types convertible to int
// which is the actual argument type of UmaHistogramExactLinear.
///template <typename SampleType>
///void UmaHistogramExactLinearWithSuffix(const char* histogram_name,
///                                       StringPiece histogram_suffix,
///                                       SampleType add_sample,
///                                       SampleType max_sample) {
///  static_assert(std::is_convertible<SampleType, int>::value,
///                "SampleType should be convertible to int");
///  WINBASE_DCHECK(histogram_name);
///  std::string histogram_full_name(histogram_name);
///  if (!histogram_suffix.empty()) {
///    histogram_full_name.append(".");
///    histogram_full_name.append(histogram_suffix.data(),
///                               histogram_suffix.length());
///  }
///  UmaHistogramExactLinear(histogram_full_name, static_cast<int>(add_sample),
///                          static_cast<int>(max_sample));
///}

// Helper function to write samples to a histogram with a dynamically assigned
// histogram name.  Works with short timings from 1 ms up to 10 seconds (50
// buckets) which is the actual argument type of UmaHistogramTimes.
///void UmaHistogramTimesWithSuffix(const char* histogram_name,
///                                 StringPiece histogram_suffix,
///                                 TimeDelta sample) {
///  WINBASE_DCHECK(histogram_name);
///  std::string histogram_full_name(histogram_name);
///  if (!histogram_suffix.empty()) {
///    histogram_full_name.append(".");
///    histogram_full_name.append(histogram_suffix.data(),
///                               histogram_suffix.length());
///  }
///  UmaHistogramTimes(histogram_full_name, sample);
///}

///void LogFailure(const FilePath& path,
///                StringPiece histogram_suffix,
///                TempFileFailure failure_code,
///                StringPiece message) {
///  UmaHistogramExactLinearWithSuffix("ImportantFile.TempFileFailures",
///                                    histogram_suffix, failure_code,
///                                    TEMP_FILE_FAILURE_MAX);
///  WINBASE_DPLOG(WARNING) << "temp file failure: " << path.value() << " : "
///                         << message;
///}

// Helper function to call WriteFileAtomically() with a
// std::unique_ptr<std::string>.
void WriteScopedStringToFileAtomically(
    const winbase::FilePath& path,
    std::unique_ptr<std::string> data,
    winbase::Closure before_write_callback,
    winbase::Callback<void(bool success)> after_write_callback,
    const std::string& histogram_suffix) {
  if (!before_write_callback.is_null())
    before_write_callback.Run();

  winbase::TimeTicks start_time = winbase::TimeTicks::Now();
  bool result = winbase::ImportantFileWriter::WriteFileAtomically(
      path, *data, histogram_suffix);
  ///if (result) {
  ///  UmaHistogramTimesWithSuffix("ImportantFile.TimeToWrite", histogram_suffix,
  ///                              TimeTicks::Now() - start_time);
  ///}

  if (!after_write_callback.is_null())
    after_write_callback.Run(result);
}

void DeleteTmpFile(const FilePath& tmp_file_path,
                   StringPiece histogram_suffix) {
  ///if (!DeleteFile(tmp_file_path, false)) {
  ///  UmaHistogramExactLinearWithSuffix(
  ///      "ImportantFile.FileDeleteError", histogram_suffix,
  ///      -base::File::GetLastFileError(), -base::File::FILE_ERROR_MAX);
  ///}
  winbase::DeleteFile(tmp_file_path, false);
}

}  // namespace

// static
bool ImportantFileWriter::WriteFileAtomically(const FilePath& path,
                                              StringPiece data,
                                              StringPiece histogram_suffix) {
  // Write the data to a temp file then rename to avoid data loss if we crash
  // while writing the file. Ensure that the temp file is on the same volume
  // as target file, so it can be moved in one step, and that the temp file
  // is securely created.
  FilePath tmp_file_path;
  if (!CreateTemporaryFileInDir(path.DirName(), &tmp_file_path)) {
    ///UmaHistogramExactLinearWithSuffix(
    ///    "ImportantFile.FileCreateError", histogram_suffix,
    ///    -base::File::GetLastFileError(), -base::File::FILE_ERROR_MAX);
    ///LogFailure(path, histogram_suffix, FAILED_CREATING,
    ///           "could not create temporary file");
    return false;
  }

  File tmp_file(tmp_file_path, File::FLAG_OPEN | File::FLAG_WRITE);
  if (!tmp_file.IsValid()) {
    ///UmaHistogramExactLinearWithSuffix(
    ///    "ImportantFile.FileOpenError", histogram_suffix,
    ///    -tmp_file.error_details(), -base::File::FILE_ERROR_MAX);
    ///LogFailure(path, histogram_suffix, FAILED_OPENING,
    ///           "could not open temporary file");
    DeleteFile(tmp_file_path, false);
    return false;
  }

  // If this fails in the wild, something really bad is going on.
  const int data_length = checked_cast<int32_t>(data.length());
  int bytes_written = tmp_file.Write(0, data.data(), data_length);
  ///if (bytes_written < data_length) {
  ///  UmaHistogramExactLinearWithSuffix(
  ///      "ImportantFile.FileWriteError", histogram_suffix,
  ///      -winbase::File::GetLastFileError(), -winbase::File::FILE_ERROR_MAX);
  ///}
  bool flush_success = tmp_file.Flush();
  tmp_file.Close();

  if (bytes_written < data_length) {
    ///LogFailure(path, histogram_suffix, FAILED_WRITING,
    ///           "error writing, bytes_written=" + IntToString(bytes_written));
    DeleteTmpFile(tmp_file_path, histogram_suffix);
    return false;
  }

  if (!flush_success) {
    ///LogFailure(path, histogram_suffix, FAILED_FLUSHING, "error flushing");
    DeleteTmpFile(tmp_file_path, histogram_suffix);
    return false;
  }

  winbase::File::Error replace_file_error = winbase::File::FILE_OK;
  if (!ReplaceFile(tmp_file_path, path, &replace_file_error)) {
    ///UmaHistogramExactLinearWithSuffix("ImportantFile.FileRenameError",
    ///                                  histogram_suffix, -replace_file_error,
    ///                                  -base::File::FILE_ERROR_MAX);
    ///LogFailure(path, histogram_suffix, FAILED_RENAMING,
    ///           "could not rename temporary file");
    DeleteTmpFile(tmp_file_path, histogram_suffix);
    return false;
  }

  return true;
}

ImportantFileWriter::ImportantFileWriter(
    const FilePath& path,
    scoped_refptr<SequencedTaskRunner> task_runner,
    const char* histogram_suffix)
    : ImportantFileWriter(path,
                          std::move(task_runner),
                          kDefaultCommitInterval,
                          histogram_suffix) {}

ImportantFileWriter::ImportantFileWriter(
    const FilePath& path,
    scoped_refptr<SequencedTaskRunner> task_runner,
    TimeDelta interval,
    const char* histogram_suffix)
    : path_(path),
      task_runner_(std::move(task_runner)),
      serializer_(nullptr),
      commit_interval_(interval),
      histogram_suffix_(histogram_suffix ? histogram_suffix : ""),
      weak_factory_(this) {
  WINBASE_DCHECK(task_runner_);
}

ImportantFileWriter::~ImportantFileWriter() {
  WINBASE_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  // We're usually a member variable of some other object, which also tends
  // to be our serializer. It may not be safe to call back to the parent object
  // being destructed.
  WINBASE_DCHECK(!HasPendingWrite());
}

bool ImportantFileWriter::HasPendingWrite() const {
  WINBASE_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  return timer().IsRunning();
}

void ImportantFileWriter::WriteNow(std::unique_ptr<std::string> data) {
  WINBASE_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  if (!IsValueInRangeForNumericType<int32_t>(data->length())) {
    WINBASE_NOTREACHED();
    return;
  }

  Closure task = AdaptCallbackForRepeating(
      BindOnce(&WriteScopedStringToFileAtomically, path_, std::move(data),
               std::move(before_next_write_callback_),
               std::move(after_next_write_callback_), histogram_suffix_));

  if (!task_runner_->PostTask(WINBASE_FROM_HERE, MakeCriticalClosure(task))) {
    // Posting the task to background message loop is not expected
    // to fail, but if it does, avoid losing data and just hit the disk
    // on the current thread.
    WINBASE_NOTREACHED();

    task.Run();
  }
  ClearPendingWrite();
}

void ImportantFileWriter::ScheduleWrite(DataSerializer* serializer) {
  WINBASE_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  WINBASE_DCHECK(serializer);
  serializer_ = serializer;

  if (!timer().IsRunning()) {
    timer().Start(
        WINBASE_FROM_HERE, commit_interval_,
        Bind(&ImportantFileWriter::DoScheduledWrite, Unretained(this)));
  }
}

void ImportantFileWriter::DoScheduledWrite() {
  WINBASE_DCHECK(serializer_);
  std::unique_ptr<std::string> data(new std::string);
  if (serializer_->SerializeData(data.get())) {
    WriteNow(std::move(data));
  } else {
    WINBASE_DLOG(WARNING) << "failed to serialize data to be saved in "
                          << path_.value();
  }
  ClearPendingWrite();
}

void ImportantFileWriter::RegisterOnNextWriteCallbacks(
    const Closure& before_next_write_callback,
    const Callback<void(bool success)>& after_next_write_callback) {
  before_next_write_callback_ = before_next_write_callback;
  after_next_write_callback_ = after_next_write_callback;
}

void ImportantFileWriter::ClearPendingWrite() {
  timer().Stop();
  serializer_ = nullptr;
}

void ImportantFileWriter::SetTimerForTesting(OneShotTimer* timer_override) {
  timer_override_ = timer_override;
}

}  // namespace winbase