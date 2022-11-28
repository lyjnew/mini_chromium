// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cross platform methods for FilePathWatcher. See the various platform
// specific implementation files, too.

#include "winbase\files\file_path_watcher.h"

#include "winbase\logging.h"
#include "winlib\build_config.h"

namespace winbase {

FilePathWatcher::~FilePathWatcher() {
  WINBASE_DCHECK(sequence_checker_.CalledOnValidSequence());
  impl_->Cancel();
}

// static
bool FilePathWatcher::RecursiveWatchAvailable() {
  return true;
}

FilePathWatcher::PlatformDelegate::PlatformDelegate(): cancelled_(false) {
}

FilePathWatcher::PlatformDelegate::~PlatformDelegate() {
  WINBASE_DCHECK(is_cancelled());
}

bool FilePathWatcher::Watch(const FilePath& path,
                            bool recursive,
                            const Callback& callback) {
  WINBASE_DCHECK(sequence_checker_.CalledOnValidSequence());
  WINBASE_DCHECK(path.IsAbsolute());
  return impl_->Watch(path, recursive, callback);
}

}  // namespace winbase