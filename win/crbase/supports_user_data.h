// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_CRBASE_SUPPORTS_USER_DATA_H_
#define MINI_CHROMIUM_CRBASE_SUPPORTS_USER_DATA_H_

#include <map>

#include "crbase/base_export.h"
#include "crbase/macros.h"
#include "crbase/memory/linked_ptr.h"
#include "crbase/memory/ref_counted.h"
#include "crbase/threading/thread_checker.h"

namespace crbase {

// This is a helper for classes that want to allow users to stash random data by
// key. At destruction all the objects will be destructed.
class CRBASE_EXPORT SupportsUserData {
 public:
  SupportsUserData();

  // Derive from this class and add your own data members to associate extra
  // information with this object. Alternatively, add this as a public base
  // class to any class with a virtual destructor.
  class CRBASE_EXPORT Data {
   public:
    virtual ~Data() {}
  };

  // The user data allows the clients to associate data with this object.
  // Multiple user data values can be stored under different keys.
  // This object will TAKE OWNERSHIP of the given data pointer, and will
  // delete the object if it is changed or the object is destroyed.
  Data* GetUserData(const void* key) const;
  void SetUserData(const void* key, Data* data);
  void RemoveUserData(const void* key);

  // SupportsUserData is not thread-safe, and on debug build will assert it is
  // only used on one thread. Calling this method allows the caller to hand
  // the SupportsUserData instance across threads. Use only if you are taking
  // full control of the synchronization of that hand over.
  void DetachUserDataThread();

 protected:
  virtual ~SupportsUserData();

 private:
  typedef std::map<const void*, linked_ptr<Data> > DataMap;

  // Externally-defined data accessible by key.
  DataMap user_data_;
  // Guards usage of |user_data_|
  ThreadChecker thread_checker_;

  CR_DISALLOW_COPY_AND_ASSIGN(SupportsUserData);
};

// Adapter class that releases a refcounted object when the
// SupportsUserData::Data object is deleted.
template <typename T>
class UserDataAdapter : public crbase::SupportsUserData::Data {
 public:
  static T* Get(const SupportsUserData* supports_user_data, const void* key) {
    UserDataAdapter* data =
      static_cast<UserDataAdapter*>(supports_user_data->GetUserData(key));
    return data ? static_cast<T*>(data->object_.get()) : NULL;
  }

  UserDataAdapter(T* object) : object_(object) {}
  T* release() { return object_.release(); }

 private:
  scoped_refptr<T> object_;

  CR_DISALLOW_COPY_AND_ASSIGN(UserDataAdapter);
};

}  // namespace crbase

#endif  // MINI_CHROMIUM_CRBASE_SUPPORTS_USER_DATA_H_