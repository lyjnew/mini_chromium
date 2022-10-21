// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/at_exit.h"

#include <stddef.h>
#include <ostream>

#include "crbase/bind.h"
#include "crbase/callback.h"
#include "crbase/logging.h"

namespace crbase {

// Keep a stack of registered AtExitManagers.  We always operate on the most
// recent, and we should never have more than one outside of testing (for a
// statically linked version of this library).  Testing may use the shadow
// version of the constructor, and if we are building a dynamic library we may
// end up with multiple AtExitManagers on the same process.  We don't protect
// this for thread-safe access, since it will only be modified in testing.
static AtExitManager* g_top_manager = NULL;

AtExitManager::AtExitManager() : next_manager_(g_top_manager) {
// If multiple modules instantiate AtExitManagers they'll end up living in this
// module... they have to coexist.
#if !defined(MINI_CHROMIUM_COMPONENT_BUILD)
  CR_DCHECK(!g_top_manager);
#endif
  g_top_manager = this;
}

AtExitManager::~AtExitManager() {
  if (!g_top_manager) {
    CR_NOTREACHED() << "Tried to ~AtExitManager without an AtExitManager";
    return;
  }
  CR_DCHECK_EQ(this, g_top_manager);

  ProcessCallbacksNow();
  g_top_manager = next_manager_;
}

// static
void AtExitManager::RegisterCallback(AtExitCallbackType func, void* param) {
  CR_DCHECK(func);
  RegisterTask(crbase::Bind(func, param));
}

// static
void AtExitManager::RegisterTask(crbase::Closure task) {
  if (!g_top_manager) {
    CR_NOTREACHED() << "Tried to RegisterCallback without an AtExitManager";
    return;
  }

  AutoLock lock(g_top_manager->lock_);
  g_top_manager->stack_.push(task);
}

// static
void AtExitManager::ProcessCallbacksNow() {
  if (!g_top_manager) {
    CR_NOTREACHED() << "Tried to ProcessCallbacksNow without an AtExitManager";
    return;
  }

  AutoLock lock(g_top_manager->lock_);

  while (!g_top_manager->stack_.empty()) {
    crbase::Closure task = g_top_manager->stack_.top();
    task.Run();
    g_top_manager->stack_.pop();
  }
}

AtExitManager::AtExitManager(bool shadow) : next_manager_(g_top_manager) {
  CR_DCHECK(shadow || !g_top_manager);
  g_top_manager = this;
}

}  // namespace crbase