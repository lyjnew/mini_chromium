// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINLIB_WINBASE_WIN_MESSAGE_WINDOW_H_
#define WINLIB_WINBASE_WIN_MESSAGE_WINDOW_H_

#include <windows.h>

#include "winbase\base_export.h"
#include "winbase\functional\callback.h"
#include "winbase\compiler_specific.h"
#include "winbase\macros.h"
#include "winbase\strings\string16.h"
#include "winbase\threading\thread_checker.h"

namespace winbase {
namespace win {

// Implements a message-only window.
class WINBASE_EXPORT MessageWindow {
 public:
  // Used to register a process-wide message window class.
  class WindowClass;

  // Implement this callback to handle messages received by the message window.
  // If the callback returns |false|, the first four parameters are passed to
  // DefWindowProc(). Otherwise, |*result| is returned by the window procedure.
  using MessageCallback = winbase::RepeatingCallback<
      bool(UINT message, WPARAM wparam, LPARAM lparam, LRESULT* result)>;

  MessageWindow();
  MessageWindow(const MessageWindow&) = delete;
  MessageWindow& operator=(const MessageWindow&) = delete;

  ~MessageWindow();

  // Creates a message-only window. The incoming messages will be passed by
  // |message_callback|. |message_callback| must outlive |this|.
  bool Create(MessageCallback message_callback);

  // Same as Create() but assigns the name to the created window.
  bool CreateNamed(MessageCallback message_callback,
                   const string16& window_name);

  HWND hwnd() const { return window_; }

  // Retrieves a handle of the first message-only window with matching
  // |window_name|.
  static HWND FindWindow(const string16& window_name);

 private:
  // Give |WindowClass| access to WindowProc().
  friend class WindowClass;

  // Contains the actual window creation code.
  bool DoCreate(MessageCallback message_callback, const wchar_t* window_name);

  // Invoked by the OS to process incoming window messages.
  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wparam,
                                     LPARAM lparam);

  // Invoked to handle messages received by the window.
  MessageCallback message_callback_;

  // Handle of the input window.
  HWND window_;

  WINBASE_THREAD_CHECKER(thread_checker_);
};

}  // namespace win
}  // namespace winbase

#endif  // WINLIB_WINBASE_WIN_MESSAGE_WINDOW_H_