// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_STRINGS_STRING_SPLIT_H_
#define WINLIB_WINBASE_STRINGS_STRING_SPLIT_H_

#include <string>
#include <utility>
#include <vector>

#include "winbase\base_export.h"
#include "winbase\strings\string16.h"
#include "winbase\strings\string_piece.h"

namespace winbase {

enum WhitespaceHandling {
  KEEP_WHITESPACE,
  TRIM_WHITESPACE,
};

enum SplitResult {
  // Strictly return all results.
  //
  // If the input is ",," and the separator is ',' this will return a
  // vector of three empty strings.
  SPLIT_WANT_ALL,

  // Only nonempty results will be added to the results. Multiple separators
  // will be coalesced. Separators at the beginning and end of the input will
  // be ignored. With TRIM_WHITESPACE, whitespace-only results will be dropped.
  //
  // If the input is ",," and the separator is ',', this will return an empty
  // vector.
  SPLIT_WANT_NONEMPTY,
};

// Split the given string on ANY of the given separators, returning copies of
// the result.
//
// To split on either commas or semicolons, keeping all whitespace:
//
//   std::vector<std::string> tokens = winbase::SplitString(
//       input, ",;", winbase::KEEP_WHITESPACE, winbase::SPLIT_WANT_ALL);
WINBASE_EXPORT std::vector<std::string> SplitString(
    StringPiece input,
    StringPiece separators,
    WhitespaceHandling whitespace,
    SplitResult result_type);
WINBASE_EXPORT std::vector<string16> SplitString(
    StringPiece16 input,
    StringPiece16 separators,
    WhitespaceHandling whitespace,
    SplitResult result_type);

// Like SplitString above except it returns a vector of StringPieces which
// reference the original buffer without copying. Although you have to be
// careful to keep the original string unmodified, this provides an efficient
// way to iterate through tokens in a string.
//
// To iterate through all whitespace-separated tokens in an input string:
//
//   for (const auto& cur :
//        winbase::SplitStringPiece(input, winbase::kWhitespaceASCII,
//                                  winbase::KEEP_WHITESPACE,
//                                  winbase::SPLIT_WANT_NONEMPTY)) {
//     ...
WINBASE_EXPORT std::vector<StringPiece> SplitStringPiece(
    StringPiece input,
    StringPiece separators,
    WhitespaceHandling whitespace,
    SplitResult result_type);
WINBASE_EXPORT std::vector<StringPiece16> SplitStringPiece(
    StringPiece16 input,
    StringPiece16 separators,
    WhitespaceHandling whitespace,
    SplitResult result_type);

using StringPairs = std::vector<std::pair<std::string, std::string>>;

// Splits |line| into key value pairs according to the given delimiters and
// removes whitespace leading each key and trailing each value. Returns true
// only if each pair has a non-empty key and value. |key_value_pairs| will
// include ("","") pairs for entries without |key_value_delimiter|.
WINBASE_EXPORT bool SplitStringIntoKeyValuePairs(StringPiece input,
                                                 char key_value_delimiter,
                                                 char key_value_pair_delimiter,
                                                 StringPairs* key_value_pairs);

// Similar to SplitString, but use a substring delimiter instead of a list of
// characters that are all possible delimiters.
WINBASE_EXPORT std::vector<string16> SplitStringUsingSubstr(
    StringPiece16 input,
    StringPiece16 delimiter,
    WhitespaceHandling whitespace,
    SplitResult result_type);
WINBASE_EXPORT std::vector<std::string> SplitStringUsingSubstr(
    StringPiece input,
    StringPiece delimiter,
    WhitespaceHandling whitespace,
    SplitResult result_type);

// Like SplitStringUsingSubstr above except it returns a vector of StringPieces
// which reference the original buffer without copying. Although you have to be
// careful to keep the original string unmodified, this provides an efficient
// way to iterate through tokens in a string.
//
// To iterate through all newline-separated tokens in an input string:
//
//   for (const auto& cur :
//        winbase::SplitStringUsingSubstr(input, "\r\n",
//                                        winbase::KEEP_WHITESPACE,
//                                        winbase::SPLIT_WANT_NONEMPTY)) {
//     ...
WINBASE_EXPORT std::vector<StringPiece16> SplitStringPieceUsingSubstr(
    StringPiece16 input,
    StringPiece16 delimiter,
    WhitespaceHandling whitespace,
    SplitResult result_type);
WINBASE_EXPORT std::vector<StringPiece> SplitStringPieceUsingSubstr(
    StringPiece input,
    StringPiece delimiter,
    WhitespaceHandling whitespace,
    SplitResult result_type);

}  // namespace winbase

#endif  // WINLIB_WINBASE_STRINGS_STRING_SPLIT_H_