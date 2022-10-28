// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file contains utility functions and classes that help the
// implementation, and management of the Callback objects.

#ifndef MINI_CHROMIUM_CRBASE_CALLBACK_INTERNAL_H_
#define MINI_CHROMIUM_CRBASE_CALLBACK_INTERNAL_H_

#include <stddef.h>
#include <memory>
#include <type_traits>

#include "crbase/atomic_ref_count.h"
#include "crbase/base_export.h"
#include "crbase/macros.h"
#include "crbase/memory/ref_counted.h"
#include "crbase/memory/scoped_ptr.h"
#include "crbase/template_util.h"

namespace crbase {
namespace internal {
class CallbackBase;

// BindStateBase被Callback类用于提供一个不透明的句柄, Callback类可以去代表一个绑定了
// 参数的函数对象. 它表现的像一个存在的类型被用于对应的DoInvoke函数去执行函数. 这允许我
// 们可以通过"类型抹除"保护Callback类不受到已绑定参数的类型的影响.
// 在基础层, 唯一任务是增加引用计数数据, 自从RefCountedThreadSafe要求析构函数是一个虚
// 函数后就不要用它.
// 为每个BindState模板实例化创建一个虚函数表会导致膨胀, 它的唯一任务是调用析构函数, 这可
// 以用一个函数指针完成.

// BindStateBase is used to provide an opaque handle that the Callback
// class can use to represent a function object with bound arguments. It
// behaves as an existential type that is used by a corresponding
// DoInvoke function to perform the function execution. This allows
// us to shield the Callback class from the types of the bound argument via
// "type erasure."
// At the base level, the only task is to add reference counting data. Don't use
// RefCountedThreadSafe since it requires the destructor to be a virtual method.
// Creating a vtable for every BindState template instantiation results in a lot
// of bloat. Its only task is to call the destructor which can be done with a
// function pointer.
class BindStateBase {
 protected:
  explicit BindStateBase(void (*destructor)(BindStateBase*))
      : ref_count_(0), destructor_(destructor) {}
  ~BindStateBase() = default;

 private:
  friend class scoped_refptr<BindStateBase>;
  friend class CallbackBase;

  void AddRef();
  void Release();

  AtomicRefCount ref_count_;

  // Pointer to a function that will properly destroy |this|.
  void (*destructor_)(BindStateBase*);

  CR_DISALLOW_COPY_AND_ASSIGN(BindStateBase);
};

// 持有Callback方法, 不要求特殊化去减少模板膨胀.

// Holds the Callback methods that don't require specialization to reduce
// template bloat.
class CRBASE_EXPORT CallbackBase {
 public:
  CallbackBase(const CallbackBase& c);
  CallbackBase& operator=(const CallbackBase& c);

  // Returns true if Callback is null (doesn't refer to anything).
  bool is_null() const { return bind_state_.get() == NULL; }

  // Returns the Callback into an uninitialized state.
  void Reset();

 protected:
  // 在C++里, 把函数指针转到另外一种类型的函数指针是安全的. 使用void*来保存函数指针是
  // 不好的. 我们创建了一个调用函数存储(InvokeFuncStorage)来储存函数指针, 然后把它转
  // 回原始类型使用.

  // In C++, it is safe to cast function pointers to function pointers of
  // another type. It is not okay to use void*. We create a InvokeFuncStorage
  // that that can store our function pointer, and then cast it back to
  // the original type on usage.
  using InvokeFuncStorage = void(*)();

  // Returns true if this callback equals |other|. |other| may be null.
  bool Equals(const CallbackBase& other) const;

  // 允许通过构造函数来初始化类成员|bind_state_|, 以避免scoped_refptr的默认初始化调
  // 用. 我们也不用在这里初始化类成员|polymorphic_invoke_|, 因为在Callback模板派生里
  // 的一个正常赋值操作对于编译器错误来说更友好.

  // Allow initializing of |bind_state_| via the constructor to avoid default
  // initialization of the scoped_refptr.  We do not also initialize
  // |polymorphic_invoke_| here because doing a normal assignment in the
  // derived Callback templates makes for much nicer compiler errors.
  explicit CallbackBase(BindStateBase* bind_state);

  // 强制析构函数在此转换单元内部实例化, 因此我们的子类将将不会得到一个被内联的版本, 避免
  // 更多的模板膨胀.

  // Force the destructor to be instantiated inside this translation unit so
  // that our subclasses will not get inlined versions.  Avoids more template
  // bloat.
  ~CallbackBase();

  scoped_refptr<BindStateBase> bind_state_;
  InvokeFuncStorage polymorphic_invoke_;
};

// 一个模板辅助器来确定是否得到的类型为non-const还是move-only-type, 也就是说(i.e.)
// 一个给定类型的值是否应该通过传给std::move()来析构. 
// 如果类型拥有一个哨兵成员MoveOnlyTypeForCPP03则被考虑为仅移动的(move-only):
// 一个类通常会使用CR_DISALLOW_COPY_AND_ASSIGN_WITH_MOVE_FOR_BIND这个宏来实现此特性.
// 这样对所有的仅移动(move-only)的类型来说更容易些... 但是这会在VS2013里的某些类型混
// 淆模板推导, 像std::unique_ptr.
// 代办事项(dcheng): 在切换到VS2015的时候, 重新浏览此内容.

// A helper template to determine if given type is non-const move-only-type,
// i.e. if a value of the given type should be passed via std::move() in a
// destructive way. Types are considered to be move-only if they have a
// sentinel MoveOnlyTypeForCPP03 member: a class typically gets this from 
// using the CR_DISALLOW_COPY_AND_ASSIGN_WITH_MOVE_FOR_BIND macro.
// It would be easy to generalize this trait to all move-only types... but this
// confuses template deduction in VS2013 with certain types such as
// std::unique_ptr.
// TODO(dcheng): Revisit this when Windows switches to VS2015 by default.
template <typename T> struct IsMoveOnlyType {
  template <typename U>
  static YesType Test(const typename U::MoveOnlyTypeForCPP03*);

  template <typename U>
  static NoType Test(...);

  static const bool value = sizeof((Test<T>(0))) == sizeof(YesType) &&
                            !is_const<T>::value;
};

// 特殊处理IsMoveOnlyType, 让std::unique_ptr仍然被认为是仅移动的(move-only), 即使
// 没有哨兵成员MoveOnlyTypeForCPP03.

// Specialization of IsMoveOnlyType so that std::unique_ptr is still considered
// move-only, even without the sentinel member.
template <typename T>
struct IsMoveOnlyType<std::unique_ptr<T>> : std::true_type {};

template <typename>
struct CallbackParamTraitsForMoveOnlyType;

template <typename>
struct CallbackParamTraitsForNonMoveOnlyType;

// 代办事项(tzik): 一旦MSVS支持有默认值的可变参数模板, 请用一个默认的参数.
// http://connect.microsoft.com/VisualStudio/feedbackdetail/view/957801/compilation-error-with-variadic-templates
//
// 这是被用于带来一个参数类型的一个类型特征对象, 并且提取一个适合的类型来储存和转发参数.
//
// 特别的, 它可以除去引用并且把数组转换成指针来储存; 还能避免意外创建一个双引用参数当参数
// 是个引用类型时.
//
// 为数组类型存储将是一个问题, 因为我们是通过常量引用来传递参数的, 在这个案例上, 我们最终
// 通过C++不允许的初始化列表里传递一个真实数组类型. 这将损坏C字符串的传递.

// TODO(tzik): Use a default parameter once MSVS supports variadic templates
// with default values.
// http://connect.microsoft.com/VisualStudio/feedbackdetail/view/957801/compilation-error-with-variadic-templates
//
// This is a typetraits object that's used to take an argument type, and
// extract a suitable type for storing and forwarding arguments.
//
// In particular, it strips off references, and converts arrays to
// pointers for storage; and it avoids accidentally trying to create a
// "reference of a reference" if the argument is a reference type.
//
// This array type becomes an issue for storage because we are passing bound
// parameters by const reference. In this case, we end up passing an actual
// array type in the initializer list which C++ does not allow.  This will
// break passing of C-string literals.
template <typename T>
struct CallbackParamTraits
    : std::conditional<IsMoveOnlyType<T>::value,
         CallbackParamTraitsForMoveOnlyType<T>,
         CallbackParamTraitsForNonMoveOnlyType<T>>::type {
};

template <typename T>
struct CallbackParamTraitsForNonMoveOnlyType {
  using ForwardType = const T&;
  using StorageType = T;
};

// 除非我们手动的指定绑定参数的类型, 否则储存不应该被触发, 不管怎样, 万一发生了, 
// 这个将防止我们意外的储存一个引用参数.
//
// 转发类型(ForwardType)应该仅仅被用于解绑参数.

// The Storage should almost be impossible to trigger unless someone manually
// specifies type of the bind parameters.  However, in case they do,
// this will guard against us accidentally storing a reference parameter.
//
// The ForwardType should only be used for unbound arguments.
template <typename T>
struct CallbackParamTraitsForNonMoveOnlyType<T&> {
  using ForwardType = T&;
  using StorageType = T;
};

// 请注意数组类型, 我们在转换中暗中加入了const, 这意味着我们不可能给带有non-const指针的
// 函数绑定数组参数

// Note that for array types, we implicitly add a const in the conversion. This
// means that it is not possible to bind array arguments to functions that take
// a non-const pointer. Trying to specialize the template based on a "const
// T[n]" does not seem to match correctly, so we are stuck with this
// restriction.
template <typename T, size_t n>
struct CallbackParamTraitsForNonMoveOnlyType<T[n]> {
  using ForwardType = const T*;
  using StorageType = const T*;
};

// See comment for CallbackParamTraits<T[n]>.
template <typename T>
struct CallbackParamTraitsForNonMoveOnlyType<T[]> {
  using ForwardType = const T*;
  using StorageType = const T*;
};

// Parameter traits for movable-but-not-copyable scopers.
//
// Callback<>/Bind() understands movable-but-not-copyable semantics where
// the type cannot be copied but can still have its state destructively
// transferred (aka. moved) to another instance of the same type by calling a
// helper function.  When used with Bind(), this signifies transferal of the
// object's state to the target function.
//
// For these types, the ForwardType must not be a const reference, or a
// reference.  A const reference is inappropriate, and would break const
// correctness, because we are implementing a destructive move.  A non-const
// reference cannot be used with temporaries which means the result of a
// function or a cast would not be usable with Callback<> or Bind().
template <typename T>
struct CallbackParamTraitsForMoveOnlyType {
  using ForwardType = T;
  using StorageType = T;
};

// CallbackForward() is a very limited simulation of C++11's std::forward()
// used by the Callback/Bind system for a set of movable-but-not-copyable
// types.  It is needed because forwarding a movable-but-not-copyable
// argument to another function requires us to invoke the proper move
// operator to create a rvalue version of the type.  The supported types are
// whitelisted below as overloads of the CallbackForward() function. The
// default template compiles out to be a no-op.
//
// In C++11, std::forward would replace all uses of this function.  However, it
// is impossible to implement a general std::forward without C++11 due to a lack
// of rvalue references.
//
// In addition to Callback/Bind, this is used by PostTaskAndReplyWithResult to
// simulate std::forward() and forward the result of one Callback as a
// parameter to another callback. This is to support Callbacks that return
// the movable-but-not-copyable types whitelisted above.
template <typename T>
typename std::enable_if<!IsMoveOnlyType<T>::value, T>::type& CallbackForward(
    T& t) {
  return t;
}

template <typename T>
typename std::enable_if<IsMoveOnlyType<T>::value, T>::type CallbackForward(
    T& t) {
  return std::move(t);
}

}  // namespace internal
}  // namespace crbase

#endif  // MINI_CHROMIUM_CRBASE_CALLBACK_INTERNAL_H_