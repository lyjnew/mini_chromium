// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_CRBASE_BIT_CAST_H_
#define MINI_CHROMIUM_CRBASE_BIT_CAST_H_

#include <string.h>

// bit_cast<Dest,Source> 是一个实现了与"*reinterpret_cast<Dest*>(&source)"等效
// 的模板函数. 我们需要这样像原始缓存库和快速数学支持的非常低级别函数
//
//   float f = 3.14159265358979;
//   int i = bit_cast<int32_t>(f);
//   // i = 0x40490fdb
//
// 比如经典的地址转换方法:
//
//   // 错误
//   float f = 3.14159265358979;            // 错误
//   int i = * reinterpret_cast<int*>(&f);  // 错误
//
// 依据ISO C++98 规范, 大约在第3.10节15段落(这在C++11里也没有基本改变), 此地址转换实
// 际上产生了未定义的行为. 段落讲到: 如果一个对象在内存拥有一个类型并且程序以另外一个不
// 同的类型访问它, 这结果对于大多数的值来说是未定义的表现
// 
// 这对于任何转换语法来说是真的, 无论是*(int*)&f 还是 *reinterpret_cast<int*>(&f),
// 尤其是在积分左值和浮点左值转换的时候.
// 
// 规范文章目的是去允许优化编译器去假设具有不用类型的表达式引用不同的内存, 编译器都知道
// 去利用这一点, 因此一个不合格的程序会悄悄粗鲁的产生错误输出
//
// 问题不在于reinterpret_cast的使用, 而在与类型双语关:
// 一个类型在内存里拥有一个对象, 然后以一个不同的类型读取他的位数据
//
// C++ 标准比这更微妙和复杂, 但是这是基础的想法
//
// 不管怎么说....
//
// bit_cast<>调用memcpy()也是来自标准库的福气, 特别是通过第3.9节的样例, 
// 当然了bit_cast<>也把令人厌恶的逻辑包装在一个地方里
//
// 幸运的是memcpy速度非常快, 在优化模式下, 编译器会以内联对象代码来替换memcpy, 当尺寸
// 参数是一个编译期常量的时候, 在32位系统下, memcpy(d,s,4) 会被编译成一个储存一个加载，
// memcpy(d,s,8) 会被编译成两个储存两个加载
//
// 警告: 如果目标(Dest)或目标(Source) 是一个 non-POD类型, memcpy的结果很可能会让你
// 惊讶
// POD: 普通旧版数据, 比如int, float等C基础类型, C++ 里的结构体,类 就是非普通旧版数据


// bit_cast<Dest,Source> is a template function that implements the equivalent
// of "*reinterpret_cast<Dest*>(&source)".  We need this in very low-level
// functions like the protobuf library and fast math support.
//
//   float f = 3.14159265358979;
//   int i = bit_cast<int32_t>(f);
//   // i = 0x40490fdb
//
// The classical address-casting method is:
//
//   // WRONG
//   float f = 3.14159265358979;            // WRONG
//   int i = * reinterpret_cast<int*>(&f);  // WRONG
//
// The address-casting method actually produces undefined behavior according to
// the ISO C++98 specification, section 3.10 ("basic.lval"), paragraph 15.
// (This did not substantially change in C++11.)  Roughly, this section says: if
// an object in memory has one type, and a program accesses it with a different
// type, then the result is undefined behavior for most values of "different
// type".
//
// This is true for any cast syntax, either *(int*)&f or
// *reinterpret_cast<int*>(&f).  And it is particularly true for conversions
// between integral lvalues and floating-point lvalues.
//// The purpose of this paragraph is to allow optimizing compilers to assume that
// expressions with different types refer to different memory.  Compilers are
// known to take advantage of this.  So a non-conforming program quietly
// produces wildly incorrect output.
//
// The problem is not the use of reinterpret_cast.  The problem is type punning:
// holding an object in memory of one type and reading its bits back using a
// different type.
//
// The C++ standard is more subtle and complex than this, but that is the basic
// idea.
//
// Anyways ...
//
// bit_cast<> calls memcpy() which is blessed by the standard, especially by the
// example in section 3.9 .  Also, of course, bit_cast<> wraps up the nasty
// logic in one place.
//
// Fortunately memcpy() is very fast.  In optimized mode, compilers replace
// calls to memcpy() with inline object code when the size argument is a
// compile-time constant.  On a 32-bit system, memcpy(d,s,4) compiles to one
// load and one store, and memcpy(d,s,8) compiles to two loads and two stores.
//
// WARNING: if Dest or Source is a non-POD type, the result of the memcpy
// is likely to surprise you.

namespace crbase {

template <class Dest, class Source>
inline Dest bit_cast(const Source& source) {
  static_assert(sizeof(Dest) == sizeof(Source),
                "bit_cast requires source and destination to be the same size");

  Dest dest;
  memcpy(&dest, &source, sizeof(dest));
  return dest;
}

}  // namespace crbase

#endif  // MINI_CHROMIUM_CRBASE_BIT_CAST_H_