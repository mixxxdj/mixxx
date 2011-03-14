// Copyright 2007, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Author: wan@google.com (Zhanyong Wan)

// Google Mock - a framework for writing C++ mock classes.
//
// This is the main header file a user should include.

#ifndef GMOCK_INCLUDE_GMOCK_GMOCK_H_
#define GMOCK_INCLUDE_GMOCK_GMOCK_H_

// This file implements the following syntax:
//
//   ON_CALL(mock_object.Method(...))
//     .With(...) ?
//     .WillByDefault(...);
//
// where With() is optional and WillByDefault() must appear exactly
// once.
//
//   EXPECT_CALL(mock_object.Method(...))
//     .With(...) ?
//     .Times(...) ?
//     .InSequence(...) *
//     .WillOnce(...) *
//     .WillRepeatedly(...) ?
//     .RetiresOnSaturation() ? ;
//
// where all clauses are optional and WillOnce() can be repeated.

// Copyright 2007, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Author: wan@google.com (Zhanyong Wan)

// Google Mock - a framework for writing C++ mock classes.
//
// This file implements some commonly used actions.

#ifndef GMOCK_INCLUDE_GMOCK_GMOCK_ACTIONS_H_
#define GMOCK_INCLUDE_GMOCK_GMOCK_ACTIONS_H_

#include <algorithm>
#include <string>

#ifndef _WIN32_WCE
#include <errno.h>
#endif

// Copyright 2007, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Author: wan@google.com (Zhanyong Wan)

// Google Mock - a framework for writing C++ mock classes.
//
// This file implements a universal value printer that can print a
// value of any type T:
//
//   void ::testing::internal::UniversalPrinter<T>::Print(value, ostream_ptr);
//
// A user can teach this function how to print a class type T by
// defining either operator<<() or PrintTo() in the namespace that
// defines T.  More specifically, the FIRST defined function in the
// following list will be used (assuming T is defined in namespace
// foo):
//
//   1. foo::PrintTo(const T&, ostream*)
//   2. operator<<(ostream&, const T&) defined in either foo or the
//      global namespace.
//
// If none of the above is defined, it will print the debug string of
// the value if it is a protocol buffer, or print the raw bytes in the
// value otherwise.
//
// To aid debugging: when T is a reference type, the address of the
// value is also printed; when T is a (const) char pointer, both the
// pointer value and the NUL-terminated string it points to are
// printed.
//
// We also provide some convenient wrappers:
//
//   // Prints a value to a string.  For a (const or not) char
//   // pointer, the NUL-terminated string (but not the pointer) is
//   // printed.
//   std::string ::testing::PrintToString(const T& value);
//
//   // Prints a value tersely: for a reference type, the referenced
//   // value (but not the address) is printed; for a (const or not) char
//   // pointer, the NUL-terminated string (but not the pointer) is
//   // printed.
//   void ::testing::internal::UniversalTersePrint(const T& value, ostream*);
//
//   // Prints value using the type inferred by the compiler.  The difference
//   // from UniversalTersePrint() is that this function prints both the
//   // pointer and the NUL-terminated string for a (const or not) char pointer.
//   void ::testing::internal::UniversalPrint(const T& value, ostream*);
//
//   // Prints the fields of a tuple tersely to a string vector, one
//   // element for each field.
//   std::vector<string> UniversalTersePrintTupleFieldsToStrings(
//       const Tuple& value);
//
// Known limitation:
//
// The print primitives print the elements of an STL-style container
// using the compiler-inferred type of *iter where iter is a
// const_iterator of the container.  When const_iterator is an input
// iterator but not a forward iterator, this inferred type may not
// match value_type, and the print output may be incorrect.  In
// practice, this is rarely a problem as for most containers
// const_iterator is a forward iterator.  We'll fix this if there's an
// actual need for it.  Note that this fix cannot rely on value_type
// being defined as many user-defined container types don't have
// value_type.

#ifndef GMOCK_INCLUDE_GMOCK_GMOCK_PRINTERS_H_
#define GMOCK_INCLUDE_GMOCK_GMOCK_PRINTERS_H_

#include <ostream>  // NOLINT
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// Copyright 2007, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Author: wan@google.com (Zhanyong Wan)

// Google Mock - a framework for writing C++ mock classes.
//
// This file defines some utilities useful for implementing Google
// Mock.  They are subject to change without notice, so please DO NOT
// USE THEM IN USER CODE.

#ifndef GMOCK_INCLUDE_GMOCK_INTERNAL_GMOCK_INTERNAL_UTILS_H_
#define GMOCK_INCLUDE_GMOCK_INTERNAL_GMOCK_INTERNAL_UTILS_H_

#include <stdio.h>
#include <ostream>  // NOLINT
#include <string>

// This file was GENERATED by a script.  DO NOT EDIT BY HAND!!!

// Copyright 2007, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Author: wan@google.com (Zhanyong Wan)

// Google Mock - a framework for writing C++ mock classes.
//
// This file contains template meta-programming utility classes needed
// for implementing Google Mock.

#ifndef GMOCK_INCLUDE_GMOCK_INTERNAL_GMOCK_GENERATED_INTERNAL_UTILS_H_
#define GMOCK_INCLUDE_GMOCK_INTERNAL_GMOCK_GENERATED_INTERNAL_UTILS_H_

// Copyright 2008, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Author: vadimb@google.com (Vadim Berman)
//
// Low-level types and utilities for porting Google Mock to various
// platforms.  They are subject to change without notice.  DO NOT USE
// THEM IN USER CODE.

#ifndef GMOCK_INCLUDE_GMOCK_INTERNAL_GMOCK_PORT_H_
#define GMOCK_INCLUDE_GMOCK_INTERNAL_GMOCK_PORT_H_

#include <assert.h>
#include <stdlib.h>
#include <iostream>

// Most of the types needed for porting Google Mock are also required
// for Google Test and are defined in gtest-port.h.
#include <gtest/gtest.h>

// To avoid conditional compilation everywhere, we make it
// gmock-port.h's responsibility to #include the header implementing
// tr1/tuple.  gmock-port.h does this via gtest-port.h, which is
// guaranteed to pull in the tuple header.

#if GTEST_OS_LINUX

#endif  // GTEST_OS_LINUX

namespace testing {
namespace internal {

// For MS Visual C++, check the compiler version. At least VS 2003 is
// required to compile Google Mock.
#if defined(_MSC_VER) && _MSC_VER < 1310
#error "At least Visual C++ 2003 (7.1) is required to compile Google Mock."
#endif

// Use implicit_cast as a safe version of static_cast for upcasting in
// the type hierarchy (e.g. casting a Foo* to a SuperclassOfFoo* or a
// const Foo*).  When you use implicit_cast, the compiler checks that
// the cast is safe.  Such explicit implicit_casts are necessary in
// surprisingly many situations where C++ demands an exact type match
// instead of an argument type convertable to a target type.
//
// The syntax for using implicit_cast is the same as for static_cast:
//
//   implicit_cast<ToType>(expr)
//
// implicit_cast would have been part of the C++ standard library,
// but the proposal was submitted too late.  It will probably make
// its way into the language in the future.
template<typename To>
inline To implicit_cast(To x) { return x; }

// When you upcast (that is, cast a pointer from type Foo to type
// SuperclassOfFoo), it's fine to use implicit_cast<>, since upcasts
// always succeed.  When you downcast (that is, cast a pointer from
// type Foo to type SubclassOfFoo), static_cast<> isn't safe, because
// how do you know the pointer is really of type SubclassOfFoo?  It
// could be a bare Foo, or of type DifferentSubclassOfFoo.  Thus,
// when you downcast, you should use this macro.  In debug mode, we
// use dynamic_cast<> to double-check the downcast is legal (we die
// if it's not).  In normal mode, we do the efficient static_cast<>
// instead.  Thus, it's important to test in debug mode to make sure
// the cast is legal!
//    This is the only place in the code we should use dynamic_cast<>.
// In particular, you SHOULDN'T be using dynamic_cast<> in order to
// do RTTI (eg code like this:
//    if (dynamic_cast<Subclass1>(foo)) HandleASubclass1Object(foo);
//    if (dynamic_cast<Subclass2>(foo)) HandleASubclass2Object(foo);
// You should design the code some other way not to need this.
template<typename To, typename From>  // use like this: down_cast<T*>(foo);
inline To down_cast(From* f) {  // so we only accept pointers
  // Ensures that To is a sub-type of From *.  This test is here only
  // for compile-time type checking, and has no overhead in an
  // optimized build at run-time, as it will be optimized away
  // completely.
  if (false) {
    const To to = NULL;
    ::testing::internal::implicit_cast<From*>(to);
  }

#if GTEST_HAS_RTTI
  assert(f == NULL || dynamic_cast<To>(f) != NULL);  // RTTI: debug mode only!
#endif
  return static_cast<To>(f);
}

// The GMOCK_COMPILE_ASSERT_ macro can be used to verify that a compile time
// expression is true. For example, you could use it to verify the
// size of a static array:
//
//   GMOCK_COMPILE_ASSERT_(ARRAYSIZE(content_type_names) == CONTENT_NUM_TYPES,
//                         content_type_names_incorrect_size);
//
// or to make sure a struct is smaller than a certain size:
//
//   GMOCK_COMPILE_ASSERT_(sizeof(foo) < 128, foo_too_large);
//
// The second argument to the macro is the name of the variable. If
// the expression is false, most compilers will issue a warning/error
// containing the name of the variable.

template <bool>
struct CompileAssert {
};

#define GMOCK_COMPILE_ASSERT_(expr, msg) \
  typedef ::testing::internal::CompileAssert<(bool(expr))> \
      msg[bool(expr) ? 1 : -1]

// Implementation details of GMOCK_COMPILE_ASSERT_:
//
// - GMOCK_COMPILE_ASSERT_ works by defining an array type that has -1
//   elements (and thus is invalid) when the expression is false.
//
// - The simpler definition
//
//    #define GMOCK_COMPILE_ASSERT_(expr, msg) typedef char msg[(expr) ? 1 : -1]
//
//   does not work, as gcc supports variable-length arrays whose sizes
//   are determined at run-time (this is gcc's extension and not part
//   of the C++ standard).  As a result, gcc fails to reject the
//   following code with the simple definition:
//
//     int foo;
//     GMOCK_COMPILE_ASSERT_(foo, msg); // not supposed to compile as foo is
//                                      // not a compile-time constant.
//
// - By using the type CompileAssert<(bool(expr))>, we ensures that
//   expr is a compile-time constant.  (Template arguments must be
//   determined at compile-time.)
//
// - The outter parentheses in CompileAssert<(bool(expr))> are necessary
//   to work around a bug in gcc 3.4.4 and 4.0.1.  If we had written
//
//     CompileAssert<bool(expr)>
//
//   instead, these compilers will refuse to compile
//
//     GMOCK_COMPILE_ASSERT_(5 > 0, some_message);
//
//   (They seem to think the ">" in "5 > 0" marks the end of the
//   template argument list.)
//
// - The array size is (bool(expr) ? 1 : -1), instead of simply
//
//     ((expr) ? 1 : -1).
//
//   This is to avoid running into a bug in MS VC 7.1, which
//   causes ((0.0) ? 1 : -1) to incorrectly evaluate to 1.

#if GTEST_HAS_GLOBAL_STRING
typedef ::string string;
#else
typedef ::std::string string;
#endif  // GTEST_HAS_GLOBAL_STRING

#if GTEST_HAS_GLOBAL_WSTRING
typedef ::wstring wstring;
#elif GTEST_HAS_STD_WSTRING
typedef ::std::wstring wstring;
#endif  // GTEST_HAS_GLOBAL_WSTRING

}  // namespace internal
}  // namespace testing

// Macro for referencing flags.  This is public as we want the user to
// use this syntax to reference Google Mock flags.
#define GMOCK_FLAG(name) FLAGS_gmock_##name

// Macros for declaring flags.
#define GMOCK_DECLARE_bool_(name) extern bool GMOCK_FLAG(name)
#define GMOCK_DECLARE_int32_(name) \
    extern ::testing::internal::Int32 GMOCK_FLAG(name)
#define GMOCK_DECLARE_string_(name) \
    extern ::testing::internal::String GMOCK_FLAG(name)

// Macros for defining flags.
#define GMOCK_DEFINE_bool_(name, default_val, doc) \
    bool GMOCK_FLAG(name) = (default_val)
#define GMOCK_DEFINE_int32_(name, default_val, doc) \
    ::testing::internal::Int32 GMOCK_FLAG(name) = (default_val)
#define GMOCK_DEFINE_string_(name, default_val, doc) \
    ::testing::internal::String GMOCK_FLAG(name) = (default_val)

#endif  // GMOCK_INCLUDE_GMOCK_INTERNAL_GMOCK_PORT_H_

namespace testing {

template <typename T>
class Matcher;

namespace internal {

// An IgnoredValue object can be implicitly constructed from ANY value.
// This is used in implementing the IgnoreResult(a) action.
class IgnoredValue {
 public:
  // This constructor template allows any value to be implicitly
  // converted to IgnoredValue.  The object has no data member and
  // doesn't try to remember anything about the argument.  We
  // deliberately omit the 'explicit' keyword in order to allow the
  // conversion to be implicit.
  template <typename T>
  IgnoredValue(const T&) {}
};

// MatcherTuple<T>::type is a tuple type where each field is a Matcher
// for the corresponding field in tuple type T.
template <typename Tuple>
struct MatcherTuple;

template <>
struct MatcherTuple< ::std::tr1::tuple<> > {
  typedef ::std::tr1::tuple< > type;
};

template <typename A1>
struct MatcherTuple< ::std::tr1::tuple<A1> > {
  typedef ::std::tr1::tuple<Matcher<A1> > type;
};

template <typename A1, typename A2>
struct MatcherTuple< ::std::tr1::tuple<A1, A2> > {
  typedef ::std::tr1::tuple<Matcher<A1>, Matcher<A2> > type;
};

template <typename A1, typename A2, typename A3>
struct MatcherTuple< ::std::tr1::tuple<A1, A2, A3> > {
  typedef ::std::tr1::tuple<Matcher<A1>, Matcher<A2>, Matcher<A3> > type;
};

template <typename A1, typename A2, typename A3, typename A4>
struct MatcherTuple< ::std::tr1::tuple<A1, A2, A3, A4> > {
  typedef ::std::tr1::tuple<Matcher<A1>, Matcher<A2>, Matcher<A3>,
      Matcher<A4> > type;
};

template <typename A1, typename A2, typename A3, typename A4, typename A5>
struct MatcherTuple< ::std::tr1::tuple<A1, A2, A3, A4, A5> > {
  typedef ::std::tr1::tuple<Matcher<A1>, Matcher<A2>, Matcher<A3>, Matcher<A4>,
      Matcher<A5> > type;
};

template <typename A1, typename A2, typename A3, typename A4, typename A5,
    typename A6>
struct MatcherTuple< ::std::tr1::tuple<A1, A2, A3, A4, A5, A6> > {
  typedef ::std::tr1::tuple<Matcher<A1>, Matcher<A2>, Matcher<A3>, Matcher<A4>,
      Matcher<A5>, Matcher<A6> > type;
};

template <typename A1, typename A2, typename A3, typename A4, typename A5,
    typename A6, typename A7>
struct MatcherTuple< ::std::tr1::tuple<A1, A2, A3, A4, A5, A6, A7> > {
  typedef ::std::tr1::tuple<Matcher<A1>, Matcher<A2>, Matcher<A3>, Matcher<A4>,
      Matcher<A5>, Matcher<A6>, Matcher<A7> > type;
};

template <typename A1, typename A2, typename A3, typename A4, typename A5,
    typename A6, typename A7, typename A8>
struct MatcherTuple< ::std::tr1::tuple<A1, A2, A3, A4, A5, A6, A7, A8> > {
  typedef ::std::tr1::tuple<Matcher<A1>, Matcher<A2>, Matcher<A3>, Matcher<A4>,
      Matcher<A5>, Matcher<A6>, Matcher<A7>, Matcher<A8> > type;
};

template <typename A1, typename A2, typename A3, typename A4, typename A5,
    typename A6, typename A7, typename A8, typename A9>
struct MatcherTuple< ::std::tr1::tuple<A1, A2, A3, A4, A5, A6, A7, A8, A9> > {
  typedef ::std::tr1::tuple<Matcher<A1>, Matcher<A2>, Matcher<A3>, Matcher<A4>,
      Matcher<A5>, Matcher<A6>, Matcher<A7>, Matcher<A8>, Matcher<A9> > type;
};

template <typename A1, typename A2, typename A3, typename A4, typename A5,
    typename A6, typename A7, typename A8, typename A9, typename A10>
struct MatcherTuple< ::std::tr1::tuple<A1, A2, A3, A4, A5, A6, A7, A8, A9,
    A10> > {
  typedef ::std::tr1::tuple<Matcher<A1>, Matcher<A2>, Matcher<A3>, Matcher<A4>,
      Matcher<A5>, Matcher<A6>, Matcher<A7>, Matcher<A8>, Matcher<A9>,
      Matcher<A10> > type;
};

// Template struct Function<F>, where F must be a function type, contains
// the following typedefs:
//
//   Result:               the function's return type.
//   ArgumentN:            the type of the N-th argument, where N starts with 1.
//   ArgumentTuple:        the tuple type consisting of all parameters of F.
//   ArgumentMatcherTuple: the tuple type consisting of Matchers for all
//                         parameters of F.
//   MakeResultVoid:       the function type obtained by substituting void
//                         for the return type of F.
//   MakeResultIgnoredValue:
//                         the function type obtained by substituting Something
//                         for the return type of F.
template <typename F>
struct Function;

template <typename R>
struct Function<R()> {
  typedef R Result;
  typedef ::std::tr1::tuple<> ArgumentTuple;
  typedef typename MatcherTuple<ArgumentTuple>::type ArgumentMatcherTuple;
  typedef void MakeResultVoid();
  typedef IgnoredValue MakeResultIgnoredValue();
};

template <typename R, typename A1>
struct Function<R(A1)>
    : Function<R()> {
  typedef A1 Argument1;
  typedef ::std::tr1::tuple<A1> ArgumentTuple;
  typedef typename MatcherTuple<ArgumentTuple>::type ArgumentMatcherTuple;
  typedef void MakeResultVoid(A1);
  typedef IgnoredValue MakeResultIgnoredValue(A1);
};

template <typename R, typename A1, typename A2>
struct Function<R(A1, A2)>
    : Function<R(A1)> {
  typedef A2 Argument2;
  typedef ::std::tr1::tuple<A1, A2> ArgumentTuple;
  typedef typename MatcherTuple<ArgumentTuple>::type ArgumentMatcherTuple;
  typedef void MakeResultVoid(A1, A2);
  typedef IgnoredValue MakeResultIgnoredValue(A1, A2);
};

template <typename R, typename A1, typename A2, typename A3>
struct Function<R(A1, A2, A3)>
    : Function<R(A1, A2)> {
  typedef A3 Argument3;
  typedef ::std::tr1::tuple<A1, A2, A3> ArgumentTuple;
  typedef typename MatcherTuple<ArgumentTuple>::type ArgumentMatcherTuple;
  typedef void MakeResultVoid(A1, A2, A3);
  typedef IgnoredValue MakeResultIgnoredValue(A1, A2, A3);
};

template <typename R, typename A1, typename A2, typename A3, typename A4>
struct Function<R(A1, A2, A3, A4)>
    : Function<R(A1, A2, A3)> {
  typedef A4 Argument4;
  typedef ::std::tr1::tuple<A1, A2, A3, A4> ArgumentTuple;
  typedef typename MatcherTuple<ArgumentTuple>::type ArgumentMatcherTuple;
  typedef void MakeResultVoid(A1, A2, A3, A4);
  typedef IgnoredValue MakeResultIgnoredValue(A1, A2, A3, A4);
};

template <typename R, typename A1, typename A2, typename A3, typename A4,
    typename A5>
struct Function<R(A1, A2, A3, A4, A5)>
    : Function<R(A1, A2, A3, A4)> {
  typedef A5 Argument5;
  typedef ::std::tr1::tuple<A1, A2, A3, A4, A5> ArgumentTuple;
  typedef typename MatcherTuple<ArgumentTuple>::type ArgumentMatcherTuple;
  typedef void MakeResultVoid(A1, A2, A3, A4, A5);
  typedef IgnoredValue MakeResultIgnoredValue(A1, A2, A3, A4, A5);
};

template <typename R, typename A1, typename A2, typename A3, typename A4,
    typename A5, typename A6>
struct Function<R(A1, A2, A3, A4, A5, A6)>
    : Function<R(A1, A2, A3, A4, A5)> {
  typedef A6 Argument6;
  typedef ::std::tr1::tuple<A1, A2, A3, A4, A5, A6> ArgumentTuple;
  typedef typename MatcherTuple<ArgumentTuple>::type ArgumentMatcherTuple;
  typedef void MakeResultVoid(A1, A2, A3, A4, A5, A6);
  typedef IgnoredValue MakeResultIgnoredValue(A1, A2, A3, A4, A5, A6);
};

template <typename R, typename A1, typename A2, typename A3, typename A4,
    typename A5, typename A6, typename A7>
struct Function<R(A1, A2, A3, A4, A5, A6, A7)>
    : Function<R(A1, A2, A3, A4, A5, A6)> {
  typedef A7 Argument7;
  typedef ::std::tr1::tuple<A1, A2, A3, A4, A5, A6, A7> ArgumentTuple;
  typedef typename MatcherTuple<ArgumentTuple>::type ArgumentMatcherTuple;
  typedef void MakeResultVoid(A1, A2, A3, A4, A5, A6, A7);
  typedef IgnoredValue MakeResultIgnoredValue(A1, A2, A3, A4, A5, A6, A7);
};

template <typename R, typename A1, typename A2, typename A3, typename A4,
    typename A5, typename A6, typename A7, typename A8>
struct Function<R(A1, A2, A3, A4, A5, A6, A7, A8)>
    : Function<R(A1, A2, A3, A4, A5, A6, A7)> {
  typedef A8 Argument8;
  typedef ::std::tr1::tuple<A1, A2, A3, A4, A5, A6, A7, A8> ArgumentTuple;
  typedef typename MatcherTuple<ArgumentTuple>::type ArgumentMatcherTuple;
  typedef void MakeResultVoid(A1, A2, A3, A4, A5, A6, A7, A8);
  typedef IgnoredValue MakeResultIgnoredValue(A1, A2, A3, A4, A5, A6, A7, A8);
};

template <typename R, typename A1, typename A2, typename A3, typename A4,
    typename A5, typename A6, typename A7, typename A8, typename A9>
struct Function<R(A1, A2, A3, A4, A5, A6, A7, A8, A9)>
    : Function<R(A1, A2, A3, A4, A5, A6, A7, A8)> {
  typedef A9 Argument9;
  typedef ::std::tr1::tuple<A1, A2, A3, A4, A5, A6, A7, A8, A9> ArgumentTuple;
  typedef typename MatcherTuple<ArgumentTuple>::type ArgumentMatcherTuple;
  typedef void MakeResultVoid(A1, A2, A3, A4, A5, A6, A7, A8, A9);
  typedef IgnoredValue MakeResultIgnoredValue(A1, A2, A3, A4, A5, A6, A7, A8,
      A9);
};

template <typename R, typename A1, typename A2, typename A3, typename A4,
    typename A5, typename A6, typename A7, typename A8, typename A9,
    typename A10>
struct Function<R(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)>
    : Function<R(A1, A2, A3, A4, A5, A6, A7, A8, A9)> {
  typedef A10 Argument10;
  typedef ::std::tr1::tuple<A1, A2, A3, A4, A5, A6, A7, A8, A9,
      A10> ArgumentTuple;
  typedef typename MatcherTuple<ArgumentTuple>::type ArgumentMatcherTuple;
  typedef void MakeResultVoid(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);
  typedef IgnoredValue MakeResultIgnoredValue(A1, A2, A3, A4, A5, A6, A7, A8,
      A9, A10);
};

}  // namespace internal

}  // namespace testing

#endif  // GMOCK_INCLUDE_GMOCK_INTERNAL_GMOCK_GENERATED_INTERNAL_UTILS_H_

// Concatenates two pre-processor symbols; works for concatenating
// built-in macros like __FILE__ and __LINE__.
#define GMOCK_CONCAT_TOKEN_IMPL_(foo, bar) foo##bar
#define GMOCK_CONCAT_TOKEN_(foo, bar) GMOCK_CONCAT_TOKEN_IMPL_(foo, bar)

#ifdef __GNUC__
#define GMOCK_ATTRIBUTE_UNUSED_ __attribute__ ((unused))
#else
#define GMOCK_ATTRIBUTE_UNUSED_
#endif  // __GNUC__

class ProtocolMessage;
namespace proto2 { class Message; }

namespace testing {
namespace internal {

// Converts an identifier name to a space-separated list of lower-case
// words.  Each maximum substring of the form [A-Za-z][a-z]*|\d+ is
// treated as one word.  For example, both "FooBar123" and
// "foo_bar_123" are converted to "foo bar 123".
string ConvertIdentifierNameToWords(const char* id_name);

// Defining a variable of type CompileAssertTypesEqual<T1, T2> will cause a
// compiler error iff T1 and T2 are different types.
template <typename T1, typename T2>
struct CompileAssertTypesEqual;

template <typename T>
struct CompileAssertTypesEqual<T, T> {
};

// Removes the reference from a type if it is a reference type,
// otherwise leaves it unchanged.  This is the same as
// tr1::remove_reference, which is not widely available yet.
template <typename T>
struct RemoveReference { typedef T type; };  // NOLINT
template <typename T>
struct RemoveReference<T&> { typedef T type; };  // NOLINT

// A handy wrapper around RemoveReference that works when the argument
// T depends on template parameters.
#define GMOCK_REMOVE_REFERENCE_(T) \
    typename ::testing::internal::RemoveReference<T>::type

// Removes const from a type if it is a const type, otherwise leaves
// it unchanged.  This is the same as tr1::remove_const, which is not
// widely available yet.
template <typename T>
struct RemoveConst { typedef T type; };  // NOLINT
template <typename T>
struct RemoveConst<const T> { typedef T type; };  // NOLINT

// MSVC 8.0 has a bug which causes the above definition to fail to
// remove the const in 'const int[3]'.  The following specialization
// works around the bug.  However, it causes trouble with gcc and thus
// needs to be conditionally compiled.
#ifdef _MSC_VER
template <typename T, size_t N>
struct RemoveConst<T[N]> {
  typedef typename RemoveConst<T>::type type[N];
};
#endif  // _MSC_VER

// A handy wrapper around RemoveConst that works when the argument
// T depends on template parameters.
#define GMOCK_REMOVE_CONST_(T) \
    typename ::testing::internal::RemoveConst<T>::type

// Adds reference to a type if it is not a reference type,
// otherwise leaves it unchanged.  This is the same as
// tr1::add_reference, which is not widely available yet.
template <typename T>
struct AddReference { typedef T& type; };  // NOLINT
template <typename T>
struct AddReference<T&> { typedef T& type; };  // NOLINT

// A handy wrapper around AddReference that works when the argument T
// depends on template parameters.
#define GMOCK_ADD_REFERENCE_(T) \
    typename ::testing::internal::AddReference<T>::type

// Adds a reference to const on top of T as necessary.  For example,
// it transforms
//
//   char         ==> const char&
//   const char   ==> const char&
//   char&        ==> const char&
//   const char&  ==> const char&
//
// The argument T must depend on some template parameters.
#define GMOCK_REFERENCE_TO_CONST_(T) \
    GMOCK_ADD_REFERENCE_(const GMOCK_REMOVE_REFERENCE_(T))

// PointeeOf<Pointer>::type is the type of a value pointed to by a
// Pointer, which can be either a smart pointer or a raw pointer.  The
// following default implementation is for the case where Pointer is a
// smart pointer.
template <typename Pointer>
struct PointeeOf {
  // Smart pointer classes define type element_type as the type of
  // their pointees.
  typedef typename Pointer::element_type type;
};
// This specialization is for the raw pointer case.
template <typename T>
struct PointeeOf<T*> { typedef T type; };  // NOLINT

// GetRawPointer(p) returns the raw pointer underlying p when p is a
// smart pointer, or returns p itself when p is already a raw pointer.
// The following default implementation is for the smart pointer case.
template <typename Pointer>
inline typename Pointer::element_type* GetRawPointer(const Pointer& p) {
  return p.get();
}
// This overloaded version is for the raw pointer case.
template <typename Element>
inline Element* GetRawPointer(Element* p) { return p; }

// This comparator allows linked_ptr to be stored in sets.
template <typename T>
struct LinkedPtrLessThan {
  bool operator()(const ::testing::internal::linked_ptr<T>& lhs,
                  const ::testing::internal::linked_ptr<T>& rhs) const {
    return lhs.get() < rhs.get();
  }
};

// ImplicitlyConvertible<From, To>::value is a compile-time bool
// constant that's true iff type From can be implicitly converted to
// type To.
template <typename From, typename To>
class ImplicitlyConvertible {
 private:
  // We need the following helper functions only for their types.
  // They have no implementations.

  // MakeFrom() is an expression whose type is From.  We cannot simply
  // use From(), as the type From may not have a public default
  // constructor.
  static From MakeFrom();

  // These two functions are overloaded.  Given an expression
  // Helper(x), the compiler will pick the first version if x can be
  // implicitly converted to type To; otherwise it will pick the
  // second version.
  //
  // The first version returns a value of size 1, and the second
  // version returns a value of size 2.  Therefore, by checking the
  // size of Helper(x), which can be done at compile time, we can tell
  // which version of Helper() is used, and hence whether x can be
  // implicitly converted to type To.
  static char Helper(To);
  static char (&Helper(...))[2];  // NOLINT

  // We have to put the 'public' section after the 'private' section,
  // or MSVC refuses to compile the code.
 public:
  // MSVC warns about implicitly converting from double to int for
  // possible loss of data, so we need to temporarily disable the
  // warning.
#ifdef _MSC_VER
#pragma warning(push)          // Saves the current warning state.
#pragma warning(disable:4244)  // Temporarily disables warning 4244.
  static const bool value =
      sizeof(Helper(ImplicitlyConvertible::MakeFrom())) == 1;
#pragma warning(pop)           // Restores the warning state.
#else
  static const bool value =
      sizeof(Helper(ImplicitlyConvertible::MakeFrom())) == 1;
#endif  // _MSV_VER
};
template <typename From, typename To>
const bool ImplicitlyConvertible<From, To>::value;

// Symbian compilation can be done with wchar_t being either a native
// type or a typedef.  Using Google Mock with OpenC without wchar_t
// should require the definition of _STLP_NO_WCHAR_T.
//
// MSVC treats wchar_t as a native type usually, but treats it as the
// same as unsigned short when the compiler option /Zc:wchar_t- is
// specified.  It defines _NATIVE_WCHAR_T_DEFINED symbol when wchar_t
// is a native type.
#if (GTEST_OS_SYMBIAN && defined(_STLP_NO_WCHAR_T)) || \
    (defined(_MSC_VER) && !defined(_NATIVE_WCHAR_T_DEFINED))
// wchar_t is a typedef.
#else
#define GMOCK_WCHAR_T_IS_NATIVE_ 1
#endif

// signed wchar_t and unsigned wchar_t are NOT in the C++ standard.
// Using them is a bad practice and not portable.  So DON'T use them.
//
// Still, Google Mock is designed to work even if the user uses signed
// wchar_t or unsigned wchar_t (obviously, assuming the compiler
// supports them).
//
// To gcc,
//   wchar_t == signed wchar_t != unsigned wchar_t == unsigned int
#ifdef __GNUC__
#define GMOCK_HAS_SIGNED_WCHAR_T_ 1  // signed/unsigned wchar_t are valid types.
#endif

// In what follows, we use the term "kind" to indicate whether a type
// is bool, an integer type (excluding bool), a floating-point type,
// or none of them.  This categorization is useful for determining
// when a matcher argument type can be safely converted to another
// type in the implementation of SafeMatcherCast.
enum TypeKind {
  kBool, kInteger, kFloatingPoint, kOther
};

// KindOf<T>::value is the kind of type T.
template <typename T> struct KindOf {
  enum { value = kOther };  // The default kind.
};

// This macro declares that the kind of 'type' is 'kind'.
#define GMOCK_DECLARE_KIND_(type, kind) \
  template <> struct KindOf<type> { enum { value = kind }; }

GMOCK_DECLARE_KIND_(bool, kBool);

// All standard integer types.
GMOCK_DECLARE_KIND_(char, kInteger);
GMOCK_DECLARE_KIND_(signed char, kInteger);
GMOCK_DECLARE_KIND_(unsigned char, kInteger);
GMOCK_DECLARE_KIND_(short, kInteger);  // NOLINT
GMOCK_DECLARE_KIND_(unsigned short, kInteger);  // NOLINT
GMOCK_DECLARE_KIND_(int, kInteger);
GMOCK_DECLARE_KIND_(unsigned int, kInteger);
GMOCK_DECLARE_KIND_(long, kInteger);  // NOLINT
GMOCK_DECLARE_KIND_(unsigned long, kInteger);  // NOLINT

#if GMOCK_WCHAR_T_IS_NATIVE_
GMOCK_DECLARE_KIND_(wchar_t, kInteger);
#endif

// Non-standard integer types.
GMOCK_DECLARE_KIND_(Int64, kInteger);
GMOCK_DECLARE_KIND_(UInt64, kInteger);

// All standard floating-point types.
GMOCK_DECLARE_KIND_(float, kFloatingPoint);
GMOCK_DECLARE_KIND_(double, kFloatingPoint);
GMOCK_DECLARE_KIND_(long double, kFloatingPoint);

#undef GMOCK_DECLARE_KIND_

// Evaluates to the kind of 'type'.
#define GMOCK_KIND_OF_(type) \
  static_cast< ::testing::internal::TypeKind>( \
      ::testing::internal::KindOf<type>::value)

// Evaluates to true iff integer type T is signed.
#define GMOCK_IS_SIGNED_(T) (static_cast<T>(-1) < 0)

// LosslessArithmeticConvertibleImpl<kFromKind, From, kToKind, To>::value
// is true iff arithmetic type From can be losslessly converted to
// arithmetic type To.
//
// It's the user's responsibility to ensure that both From and To are
// raw (i.e. has no CV modifier, is not a pointer, and is not a
// reference) built-in arithmetic types, kFromKind is the kind of
// From, and kToKind is the kind of To; the value is
// implementation-defined when the above pre-condition is violated.
template <TypeKind kFromKind, typename From, TypeKind kToKind, typename To>
struct LosslessArithmeticConvertibleImpl : public false_type {};

// Converting bool to bool is lossless.
template <>
struct LosslessArithmeticConvertibleImpl<kBool, bool, kBool, bool>
    : public true_type {};  // NOLINT

// Converting bool to any integer type is lossless.
template <typename To>
struct LosslessArithmeticConvertibleImpl<kBool, bool, kInteger, To>
    : public true_type {};  // NOLINT

// Converting bool to any floating-point type is lossless.
template <typename To>
struct LosslessArithmeticConvertibleImpl<kBool, bool, kFloatingPoint, To>
    : public true_type {};  // NOLINT

// Converting an integer to bool is lossy.
template <typename From>
struct LosslessArithmeticConvertibleImpl<kInteger, From, kBool, bool>
    : public false_type {};  // NOLINT

// Converting an integer to another non-bool integer is lossless iff
// the target type's range encloses the source type's range.
template <typename From, typename To>
struct LosslessArithmeticConvertibleImpl<kInteger, From, kInteger, To>
    : public bool_constant<
      // When converting from a smaller size to a larger size, we are
      // fine as long as we are not converting from signed to unsigned.
      ((sizeof(From) < sizeof(To)) &&
       (!GMOCK_IS_SIGNED_(From) || GMOCK_IS_SIGNED_(To))) ||
      // When converting between the same size, the signedness must match.
      ((sizeof(From) == sizeof(To)) &&
       (GMOCK_IS_SIGNED_(From) == GMOCK_IS_SIGNED_(To)))> {};  // NOLINT

#undef GMOCK_IS_SIGNED_

// Converting an integer to a floating-point type may be lossy, since
// the format of a floating-point number is implementation-defined.
template <typename From, typename To>
struct LosslessArithmeticConvertibleImpl<kInteger, From, kFloatingPoint, To>
    : public false_type {};  // NOLINT

// Converting a floating-point to bool is lossy.
template <typename From>
struct LosslessArithmeticConvertibleImpl<kFloatingPoint, From, kBool, bool>
    : public false_type {};  // NOLINT

// Converting a floating-point to an integer is lossy.
template <typename From, typename To>
struct LosslessArithmeticConvertibleImpl<kFloatingPoint, From, kInteger, To>
    : public false_type {};  // NOLINT

// Converting a floating-point to another floating-point is lossless
// iff the target type is at least as big as the source type.
template <typename From, typename To>
struct LosslessArithmeticConvertibleImpl<
  kFloatingPoint, From, kFloatingPoint, To>
    : public bool_constant<sizeof(From) <= sizeof(To)> {};  // NOLINT

// LosslessArithmeticConvertible<From, To>::value is true iff arithmetic
// type From can be losslessly converted to arithmetic type To.
//
// It's the user's responsibility to ensure that both From and To are
// raw (i.e. has no CV modifier, is not a pointer, and is not a
// reference) built-in arithmetic types; the value is
// implementation-defined when the above pre-condition is violated.
template <typename From, typename To>
struct LosslessArithmeticConvertible
    : public LosslessArithmeticConvertibleImpl<
  GMOCK_KIND_OF_(From), From, GMOCK_KIND_OF_(To), To> {};  // NOLINT

// IsAProtocolMessage<T>::value is a compile-time bool constant that's
// true iff T is type ProtocolMessage, proto2::Message, or a subclass
// of those.
template <typename T>
struct IsAProtocolMessage
    : public bool_constant<
  ImplicitlyConvertible<const T*, const ::ProtocolMessage*>::value ||
  ImplicitlyConvertible<const T*, const ::proto2::Message*>::value> {
};

// When the compiler sees expression IsContainerTest<C>(0), the first
// overload of IsContainerTest will be picked if C is an STL-style
// container class (since C::const_iterator* is a valid type and 0 can
// be converted to it), while the second overload will be picked
// otherwise (since C::const_iterator will be an invalid type in this
// case).  Therefore, we can determine whether C is a container class
// by checking the type of IsContainerTest<C>(0).  The value of the
// expression is insignificant.
typedef int IsContainer;
template <class C>
IsContainer IsContainerTest(typename C::const_iterator*) { return 0; }

typedef char IsNotContainer;
template <class C>
IsNotContainer IsContainerTest(...) { return '\0'; }

// This interface knows how to report a Google Mock failure (either
// non-fatal or fatal).
class FailureReporterInterface {
 public:
  // The type of a failure (either non-fatal or fatal).
  enum FailureType {
    NONFATAL, FATAL
  };

  virtual ~FailureReporterInterface() {}

  // Reports a failure that occurred at the given source file location.
  virtual void ReportFailure(FailureType type, const char* file, int line,
                             const string& message) = 0;
};

// Returns the failure reporter used by Google Mock.
FailureReporterInterface* GetFailureReporter();

// Asserts that condition is true; aborts the process with the given
// message if condition is false.  We cannot use LOG(FATAL) or CHECK()
// as Google Mock might be used to mock the log sink itself.  We
// inline this function to prevent it from showing up in the stack
// trace.
inline void Assert(bool condition, const char* file, int line,
                   const string& msg) {
  if (!condition) {
    GetFailureReporter()->ReportFailure(FailureReporterInterface::FATAL,
                                        file, line, msg);
  }
}
inline void Assert(bool condition, const char* file, int line) {
  Assert(condition, file, line, "Assertion failed.");
}

// Verifies that condition is true; generates a non-fatal failure if
// condition is false.
inline void Expect(bool condition, const char* file, int line,
                   const string& msg) {
  if (!condition) {
    GetFailureReporter()->ReportFailure(FailureReporterInterface::NONFATAL,
                                        file, line, msg);
  }
}
inline void Expect(bool condition, const char* file, int line) {
  Expect(condition, file, line, "Expectation failed.");
}

// Severity level of a log.
enum LogSeverity {
  INFO = 0,
  WARNING = 1,
};

// Valid values for the --gmock_verbose flag.

// All logs (informational and warnings) are printed.
const char kInfoVerbosity[] = "info";
// Only warnings are printed.
const char kWarningVerbosity[] = "warning";
// No logs are printed.
const char kErrorVerbosity[] = "error";

// Returns true iff a log with the given severity is visible according
// to the --gmock_verbose flag.
bool LogIsVisible(LogSeverity severity);

// Prints the given message to stdout iff 'severity' >= the level
// specified by the --gmock_verbose flag.  If stack_frames_to_skip >=
// 0, also prints the stack trace excluding the top
// stack_frames_to_skip frames.  In opt mode, any positive
// stack_frames_to_skip is treated as 0, since we don't know which
// function calls will be inlined by the compiler and need to be
// conservative.
void Log(LogSeverity severity, const string& message, int stack_frames_to_skip);

// TODO(wan@google.com): group all type utilities together.

// Type traits.

// is_reference<T>::value is non-zero iff T is a reference type.
template <typename T> struct is_reference : public false_type {};
template <typename T> struct is_reference<T&> : public true_type {};

// type_equals<T1, T2>::value is non-zero iff T1 and T2 are the same type.
template <typename T1, typename T2> struct type_equals : public false_type {};
template <typename T> struct type_equals<T, T> : public true_type {};

// remove_reference<T>::type removes the reference from type T, if any.
template <typename T> struct remove_reference { typedef T type; };  // NOLINT
template <typename T> struct remove_reference<T&> { typedef T type; }; // NOLINT

// Invalid<T>() returns an invalid value of type T.  This is useful
// when a value of type T is needed for compilation, but the statement
// will not really be executed (or we don't care if the statement
// crashes).
template <typename T>
inline T Invalid() {
  return *static_cast<typename remove_reference<T>::type*>(NULL);
}
template <>
inline void Invalid<void>() {}

// Utilities for native arrays.

// ArrayEq() compares two k-dimensional native arrays using the
// elements' operator==, where k can be any integer >= 0.  When k is
// 0, ArrayEq() degenerates into comparing a single pair of values.

template <typename T, typename U>
bool ArrayEq(const T* lhs, size_t size, const U* rhs);

// This generic version is used when k is 0.
template <typename T, typename U>
inline bool ArrayEq(const T& lhs, const U& rhs) { return lhs == rhs; }

// This overload is used when k >= 1.
template <typename T, typename U, size_t N>
inline bool ArrayEq(const T(&lhs)[N], const U(&rhs)[N]) {
  return internal::ArrayEq(lhs, N, rhs);
}

// This helper reduces code bloat.  If we instead put its logic inside
// the previous ArrayEq() function, arrays with different sizes would
// lead to different copies of the template code.
template <typename T, typename U>
bool ArrayEq(const T* lhs, size_t size, const U* rhs) {
  for (size_t i = 0; i != size; i++) {
    if (!internal::ArrayEq(lhs[i], rhs[i]))
      return false;
  }
  return true;
}

// Finds the first element in the iterator range [begin, end) that
// equals elem.  Element may be a native array type itself.
template <typename Iter, typename Element>
Iter ArrayAwareFind(Iter begin, Iter end, const Element& elem) {
  for (Iter it = begin; it != end; ++it) {
    if (internal::ArrayEq(*it, elem))
      return it;
  }
  return end;
}

// CopyArray() copies a k-dimensional native array using the elements'
// operator=, where k can be any integer >= 0.  When k is 0,
// CopyArray() degenerates into copying a single value.

template <typename T, typename U>
void CopyArray(const T* from, size_t size, U* to);

// This generic version is used when k is 0.
template <typename T, typename U>
inline void CopyArray(const T& from, U* to) { *to = from; }

// This overload is used when k >= 1.
template <typename T, typename U, size_t N>
inline void CopyArray(const T(&from)[N], U(*to)[N]) {
  internal::CopyArray(from, N, *to);
}

// This helper reduces code bloat.  If we instead put its logic inside
// the previous CopyArray() function, arrays with different sizes
// would lead to different copies of the template code.
template <typename T, typename U>
void CopyArray(const T* from, size_t size, U* to) {
  for (size_t i = 0; i != size; i++) {
    internal::CopyArray(from[i], to + i);
  }
}

// The relation between an NativeArray object (see below) and the
// native array it represents.
enum RelationToSource {
  kReference,  // The NativeArray references the native array.
  kCopy        // The NativeArray makes a copy of the native array and
               // owns the copy.
};

// Adapts a native array to a read-only STL-style container.  Instead
// of the complete STL container concept, this adaptor only implements
// members useful for Google Mock's container matchers.  New members
// should be added as needed.  To simplify the implementation, we only
// support Element being a raw type (i.e. having no top-level const or
// reference modifier).  It's the client's responsibility to satisfy
// this requirement.  Element can be an array type itself (hence
// multi-dimensional arrays are supported).
template <typename Element>
class NativeArray {
 public:
  // STL-style container typedefs.
  typedef Element value_type;
  typedef const Element* const_iterator;

  // Constructs from a native array.
  NativeArray(const Element* array, size_t count, RelationToSource relation) {
    Init(array, count, relation);
  }

  // Copy constructor.
  NativeArray(const NativeArray& rhs) {
    Init(rhs.array_, rhs.size_, rhs.relation_to_source_);
  }

  ~NativeArray() {
    // Ensures that the user doesn't instantiate NativeArray with a
    // const or reference type.
    testing::StaticAssertTypeEq<Element,
        GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(Element))>();
    if (relation_to_source_ == kCopy)
      delete[] array_;
  }

  // STL-style container methods.
  size_t size() const { return size_; }
  const_iterator begin() const { return array_; }
  const_iterator end() const { return array_ + size_; }
  bool operator==(const NativeArray& rhs) const {
    return size() == rhs.size() &&
        ArrayEq(begin(), size(), rhs.begin());
  }

 private:
  // Not implemented as we don't want to support assignment.
  void operator=(const NativeArray& rhs);

  // Initializes this object; makes a copy of the input array if
  // 'relation' is kCopy.
  void Init(const Element* array, size_t a_size, RelationToSource relation) {
    if (relation == kReference) {
      array_ = array;
    } else {
      Element* const copy = new Element[a_size];
      CopyArray(array, a_size, copy);
      array_ = copy;
    }
    size_ = a_size;
    relation_to_source_ = relation;
  }

  const Element* array_;
  size_t size_;
  RelationToSource relation_to_source_;
};

// Given a raw type (i.e. having no top-level reference or const
// modifier) RawContainer that's either an STL-style container or a
// native array, class StlContainerView<RawContainer> has the
// following members:
//
//   - type is a type that provides an STL-style container view to
//     (i.e. implements the STL container concept for) RawContainer;
//   - const_reference is a type that provides a reference to a const
//     RawContainer;
//   - ConstReference(raw_container) returns a const reference to an STL-style
//     container view to raw_container, which is a RawContainer.
//   - Copy(raw_container) returns an STL-style container view of a
//     copy of raw_container, which is a RawContainer.
//
// This generic version is used when RawContainer itself is already an
// STL-style container.
template <class RawContainer>
class StlContainerView {
 public:
  typedef RawContainer type;
  typedef const type& const_reference;

  static const_reference ConstReference(const RawContainer& container) {
    // Ensures that RawContainer is not a const type.
    testing::StaticAssertTypeEq<RawContainer,
        GMOCK_REMOVE_CONST_(RawContainer)>();
    return container;
  }
  static type Copy(const RawContainer& container) { return container; }
};

// This specialization is used when RawContainer is a native array type.
template <typename Element, size_t N>
class StlContainerView<Element[N]> {
 public:
  typedef GMOCK_REMOVE_CONST_(Element) RawElement;
  typedef internal::NativeArray<RawElement> type;
  // NativeArray<T> can represent a native array either by value or by
  // reference (selected by a constructor argument), so 'const type'
  // can be used to reference a const native array.  We cannot
  // 'typedef const type& const_reference' here, as that would mean
  // ConstReference() has to return a reference to a local variable.
  typedef const type const_reference;

  static const_reference ConstReference(const Element (&array)[N]) {
    // Ensures that Element is not a const type.
    testing::StaticAssertTypeEq<Element, RawElement>();
#if GTEST_OS_SYMBIAN
    // The Nokia Symbian compiler confuses itself in template instantiation
    // for this call without the cast to Element*:
    // function call '[testing::internal::NativeArray<char *>].NativeArray(
    //     {lval} const char *[4], long, testing::internal::RelationToSource)'
    //     does not match
    // 'testing::internal::NativeArray<char *>::NativeArray(
    //     char *const *, unsigned int, testing::internal::RelationToSource)'
    // (instantiating: 'testing::internal::ContainsMatcherImpl
    //     <const char * (&)[4]>::Matches(const char * (&)[4]) const')
    // (instantiating: 'testing::internal::StlContainerView<char *[4]>::
    //     ConstReference(const char * (&)[4])')
    // (and though the N parameter type is mismatched in the above explicit
    // conversion of it doesn't help - only the conversion of the array).
    return type(const_cast<Element*>(&array[0]), N, kReference);
#else
    return type(array, N, kReference);
#endif  // GTEST_OS_SYMBIAN
  }
  static type Copy(const Element (&array)[N]) {
#if GTEST_OS_SYMBIAN
    return type(const_cast<Element*>(&array[0]), N, kCopy);
#else
    return type(array, N, kCopy);
#endif  // GTEST_OS_SYMBIAN
  }
};

// This specialization is used when RawContainer is a native array
// represented as a (pointer, size) tuple.
template <typename ElementPointer, typename Size>
class StlContainerView< ::std::tr1::tuple<ElementPointer, Size> > {
 public:
  typedef GMOCK_REMOVE_CONST_(
      typename internal::PointeeOf<ElementPointer>::type) RawElement;
  typedef internal::NativeArray<RawElement> type;
  typedef const type const_reference;

  static const_reference ConstReference(
      const ::std::tr1::tuple<ElementPointer, Size>& array) {
    using ::std::tr1::get;
    return type(get<0>(array), get<1>(array), kReference);
  }
  static type Copy(const ::std::tr1::tuple<ElementPointer, Size>& array) {
    using ::std::tr1::get;
    return type(get<0>(array), get<1>(array), kCopy);
  }
};

// The following specialization prevents the user from instantiating
// StlContainer with a reference type.
template <typename T> class StlContainerView<T&>;

}  // namespace internal
}  // namespace testing

#endif  // GMOCK_INCLUDE_GMOCK_INTERNAL_GMOCK_INTERNAL_UTILS_H_

namespace testing {

// Definitions in the 'internal' and 'internal2' name spaces are
// subject to change without notice.  DO NOT USE THEM IN USER CODE!
namespace internal2 {

// Prints the given number of bytes in the given object to the given
// ostream.
void PrintBytesInObjectTo(const unsigned char* obj_bytes,
                          size_t count,
                          ::std::ostream* os);

// TypeWithoutFormatter<T, kIsProto>::PrintValue(value, os) is called
// by the universal printer to print a value of type T when neither
// operator<< nor PrintTo() is defined for type T.  When T is
// ProtocolMessage, proto2::Message, or a subclass of those, kIsProto
// will be true and the short debug string of the protocol message
// value will be printed; otherwise kIsProto will be false and the
// bytes in the value will be printed.
template <typename T, bool kIsProto>
class TypeWithoutFormatter {
 public:
  static void PrintValue(const T& value, ::std::ostream* os) {
    PrintBytesInObjectTo(reinterpret_cast<const unsigned char*>(&value),
                         sizeof(value), os);
  }
};

// We print a protobuf using its ShortDebugString() when the string
// doesn't exceed this many characters; otherwise we print it using
// DebugString() for better readability.
const size_t kProtobufOneLinerMaxLength = 50;

template <typename T>
class TypeWithoutFormatter<T, true> {
 public:
  static void PrintValue(const T& value, ::std::ostream* os) {
    const ::testing::internal::string short_str = value.ShortDebugString();
    const ::testing::internal::string pretty_str =
        short_str.length() <= kProtobufOneLinerMaxLength ?
        short_str : ("\n" + value.DebugString());
    ::std::operator<<(*os, "<" + pretty_str + ">");
  }
};

// Prints the given value to the given ostream.  If the value is a
// protocol message, its short debug string is printed; otherwise the
// bytes in the value are printed.  This is what
// UniversalPrinter<T>::Print() does when it knows nothing about type
// T and T has no << operator.
//
// A user can override this behavior for a class type Foo by defining
// a << operator in the namespace where Foo is defined.
//
// We put this operator in namespace 'internal2' instead of 'internal'
// to simplify the implementation, as much code in 'internal' needs to
// use << in STL, which would conflict with our own << were it defined
// in 'internal'.
//
// Note that this operator<< takes a generic std::basic_ostream<Char,
// CharTraits> type instead of the more restricted std::ostream.  If
// we define it to take an std::ostream instead, we'll get an
// "ambiguous overloads" compiler error when trying to print a type
// Foo that supports streaming to std::basic_ostream<Char,
// CharTraits>, as the compiler cannot tell whether
// operator<<(std::ostream&, const T&) or
// operator<<(std::basic_stream<Char, CharTraits>, const Foo&) is more
// specific.
template <typename Char, typename CharTraits, typename T>
::std::basic_ostream<Char, CharTraits>& operator<<(
    ::std::basic_ostream<Char, CharTraits>& os, const T& x) {
  TypeWithoutFormatter<T, ::testing::internal::IsAProtocolMessage<T>::value>::
      PrintValue(x, &os);
  return os;
}

}  // namespace internal2
}  // namespace testing

// This namespace MUST NOT BE NESTED IN ::testing, or the name look-up
// magic needed for implementing UniversalPrinter won't work.
namespace testing_internal {

// Used to print a value that is not an STL-style container when the
// user doesn't define PrintTo() for it.
template <typename T>
void DefaultPrintNonContainerTo(const T& value, ::std::ostream* os) {
  // With the following statement, during unqualified name lookup,
  // testing::internal2::operator<< appears as if it was declared in
  // the nearest enclosing namespace that contains both
  // ::testing_internal and ::testing::internal2, i.e. the global
  // namespace.  For more details, refer to the C++ Standard section
  // 7.3.4-1 [namespace.udir].  This allows us to fall back onto
  // testing::internal2::operator<< in case T doesn't come with a <<
  // operator.
  //
  // We cannot write 'using ::testing::internal2::operator<<;', which
  // gcc 3.3 fails to compile due to a compiler bug.
  using namespace ::testing::internal2;  // NOLINT

  // Assuming T is defined in namespace foo, in the next statement,
  // the compiler will consider all of:
  //
  //   1. foo::operator<< (thanks to Koenig look-up),
  //   2. ::operator<< (as the current namespace is enclosed in ::),
  //   3. testing::internal2::operator<< (thanks to the using statement above).
  //
  // The operator<< whose type matches T best will be picked.
  //
  // We deliberately allow #2 to be a candidate, as sometimes it's
  // impossible to define #1 (e.g. when foo is ::std, defining
  // anything in it is undefined behavior unless you are a compiler
  // vendor.).
  *os << value;
}

}  // namespace testing_internal

namespace testing {
namespace internal {

// UniversalPrinter<T>::Print(value, ostream_ptr) prints the given
// value to the given ostream.  The caller must ensure that
// 'ostream_ptr' is not NULL, or the behavior is undefined.
//
// We define UniversalPrinter as a class template (as opposed to a
// function template), as we need to partially specialize it for
// reference types, which cannot be done with function templates.
template <typename T>
class UniversalPrinter;

template <typename T>
void UniversalPrint(const T& value, ::std::ostream* os);

// Used to print an STL-style container when the user doesn't define
// a PrintTo() for it.
template <typename C>
void DefaultPrintTo(IsContainer /* dummy */,
                    false_type /* is not a pointer */,
                    const C& container, ::std::ostream* os) {
  const size_t kMaxCount = 32;  // The maximum number of elements to print.
  *os << '{';
  size_t count = 0;
  for (typename C::const_iterator it = container.begin();
       it != container.end(); ++it, ++count) {
    if (count > 0) {
      *os << ',';
      if (count == kMaxCount) {  // Enough has been printed.
        *os << " ...";
        break;
      }
    }
    *os << ' ';
    // We cannot call PrintTo(*it, os) here as PrintTo() doesn't
    // handle *it being a native array.
    internal::UniversalPrint(*it, os);
  }

  if (count > 0) {
    *os << ' ';
  }
  *os << '}';
}

// Used to print a pointer that is neither a char pointer nor a member
// pointer, when the user doesn't define PrintTo() for it.  (A member
// variable pointer or member function pointer doesn't really point to
// a location in the address space.  Their representation is
// implementation-defined.  Therefore they will be printed as raw
// bytes.)
template <typename T>
void DefaultPrintTo(IsNotContainer /* dummy */,
                    true_type /* is a pointer */,
                    T* p, ::std::ostream* os) {
  if (p == NULL) {
    *os << "NULL";
  } else {
    // We want to print p as a const void*.  However, we cannot cast
    // it to const void* directly, even using reinterpret_cast, as
    // earlier versions of gcc (e.g. 3.4.5) cannot compile the cast
    // when p is a function pointer.  Casting to UInt64 first solves
    // the problem.
    *os << reinterpret_cast<const void*>(reinterpret_cast<internal::UInt64>(p));
  }
}

// Used to print a non-container, non-pointer value when the user
// doesn't define PrintTo() for it.
template <typename T>
void DefaultPrintTo(IsNotContainer /* dummy */,
                    false_type /* is not a pointer */,
                    const T& value, ::std::ostream* os) {
  ::testing_internal::DefaultPrintNonContainerTo(value, os);
}

// Prints the given value using the << operator if it has one;
// otherwise prints the bytes in it.  This is what
// UniversalPrinter<T>::Print() does when PrintTo() is not specialized
// or overloaded for type T.
//
// A user can override this behavior for a class type Foo by defining
// an overload of PrintTo() in the namespace where Foo is defined.  We
// give the user this option as sometimes defining a << operator for
// Foo is not desirable (e.g. the coding style may prevent doing it,
// or there is already a << operator but it doesn't do what the user
// wants).
template <typename T>
void PrintTo(const T& value, ::std::ostream* os) {
  // DefaultPrintTo() is overloaded.  The type of its first two
  // arguments determine which version will be picked.  If T is an
  // STL-style container, the version for container will be called; if
  // T is a pointer, the pointer version will be called; otherwise the
  // generic version will be called.
  //
  // Note that we check for container types here, prior to we check
  // for protocol message types in our operator<<.  The rationale is:
  //
  // For protocol messages, we want to give people a chance to
  // override Google Mock's format by defining a PrintTo() or
  // operator<<.  For STL containers, other formats can be
  // incompatible with Google Mock's format for the container
  // elements; therefore we check for container types here to ensure
  // that our format is used.
  //
  // The second argument of DefaultPrintTo() is needed to bypass a bug
  // in Symbian's C++ compiler that prevents it from picking the right
  // overload between:
  //
  //   PrintTo(const T& x, ...);
  //   PrintTo(T* x, ...);
  DefaultPrintTo(IsContainerTest<T>(0), is_pointer<T>(), value, os);
}

// The following list of PrintTo() overloads tells
// UniversalPrinter<T>::Print() how to print standard types (built-in
// types, strings, plain arrays, and pointers).

// Overloads for various char types.
void PrintCharTo(char c, int char_code, ::std::ostream* os);
inline void PrintTo(unsigned char c, ::std::ostream* os) {
  PrintCharTo(c, c, os);
}
inline void PrintTo(signed char c, ::std::ostream* os) {
  PrintCharTo(c, c, os);
}
inline void PrintTo(char c, ::std::ostream* os) {
  // When printing a plain char, we always treat it as unsigned.  This
  // way, the output won't be affected by whether the compiler thinks
  // char is signed or not.
  PrintTo(static_cast<unsigned char>(c), os);
}

// Overloads for other simple built-in types.
inline void PrintTo(bool x, ::std::ostream* os) {
  *os << (x ? "true" : "false");
}

// Overload for wchar_t type.
// Prints a wchar_t as a symbol if it is printable or as its internal
// code otherwise and also as its decimal code (except for L'\0').
// The L'\0' char is printed as "L'\\0'". The decimal code is printed
// as signed integer when wchar_t is implemented by the compiler
// as a signed type and is printed as an unsigned integer when wchar_t
// is implemented as an unsigned type.
void PrintTo(wchar_t wc, ::std::ostream* os);

// Overloads for C strings.
void PrintTo(const char* s, ::std::ostream* os);
inline void PrintTo(char* s, ::std::ostream* os) {
  PrintTo(implicit_cast<const char*>(s), os);
}

// MSVC can be configured to define wchar_t as a typedef of unsigned
// short.  It defines _NATIVE_WCHAR_T_DEFINED when wchar_t is a native
// type.  When wchar_t is a typedef, defining an overload for const
// wchar_t* would cause unsigned short* be printed as a wide string,
// possibly causing invalid memory accesses.
#if !defined(_MSC_VER) || defined(_NATIVE_WCHAR_T_DEFINED)
// Overloads for wide C strings
void PrintTo(const wchar_t* s, ::std::ostream* os);
inline void PrintTo(wchar_t* s, ::std::ostream* os) {
  PrintTo(implicit_cast<const wchar_t*>(s), os);
}
#endif

// Overload for C arrays.  Multi-dimensional arrays are printed
// properly.

// Prints the given number of elements in an array, without printing
// the curly braces.
template <typename T>
void PrintRawArrayTo(const T a[], size_t count, ::std::ostream* os) {
  UniversalPrinter<T>::Print(a[0], os);
  for (size_t i = 1; i != count; i++) {
    *os << ", ";
    UniversalPrinter<T>::Print(a[i], os);
  }
}

// Overloads for ::string and ::std::string.
#if GTEST_HAS_GLOBAL_STRING
void PrintStringTo(const ::string&s, ::std::ostream* os);
inline void PrintTo(const ::string& s, ::std::ostream* os) {
  PrintStringTo(s, os);
}
#endif  // GTEST_HAS_GLOBAL_STRING

void PrintStringTo(const ::std::string&s, ::std::ostream* os);
inline void PrintTo(const ::std::string& s, ::std::ostream* os) {
  PrintStringTo(s, os);
}

// Overloads for ::wstring and ::std::wstring.
#if GTEST_HAS_GLOBAL_WSTRING
void PrintWideStringTo(const ::wstring&s, ::std::ostream* os);
inline void PrintTo(const ::wstring& s, ::std::ostream* os) {
  PrintWideStringTo(s, os);
}
#endif  // GTEST_HAS_GLOBAL_WSTRING

#if GTEST_HAS_STD_WSTRING
void PrintWideStringTo(const ::std::wstring&s, ::std::ostream* os);
inline void PrintTo(const ::std::wstring& s, ::std::ostream* os) {
  PrintWideStringTo(s, os);
}
#endif  // GTEST_HAS_STD_WSTRING

// Overload for ::std::tr1::tuple.  Needed for printing function
// arguments, which are packed as tuples.

// Helper function for printing a tuple.  T must be instantiated with
// a tuple type.
template <typename T>
void PrintTupleTo(const T& t, ::std::ostream* os);

// Overloaded PrintTo() for tuples of various arities.  We support
// tuples of up-to 10 fields.  The following implementation works
// regardless of whether tr1::tuple is implemented using the
// non-standard variadic template feature or not.

inline void PrintTo(const ::std::tr1::tuple<>& t, ::std::ostream* os) {
  PrintTupleTo(t, os);
}

template <typename T1>
void PrintTo(const ::std::tr1::tuple<T1>& t, ::std::ostream* os) {
  PrintTupleTo(t, os);
}

template <typename T1, typename T2>
void PrintTo(const ::std::tr1::tuple<T1, T2>& t, ::std::ostream* os) {
  PrintTupleTo(t, os);
}

template <typename T1, typename T2, typename T3>
void PrintTo(const ::std::tr1::tuple<T1, T2, T3>& t, ::std::ostream* os) {
  PrintTupleTo(t, os);
}

template <typename T1, typename T2, typename T3, typename T4>
void PrintTo(const ::std::tr1::tuple<T1, T2, T3, T4>& t, ::std::ostream* os) {
  PrintTupleTo(t, os);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
void PrintTo(const ::std::tr1::tuple<T1, T2, T3, T4, T5>& t,
             ::std::ostream* os) {
  PrintTupleTo(t, os);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6>
void PrintTo(const ::std::tr1::tuple<T1, T2, T3, T4, T5, T6>& t,
             ::std::ostream* os) {
  PrintTupleTo(t, os);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7>
void PrintTo(const ::std::tr1::tuple<T1, T2, T3, T4, T5, T6, T7>& t,
             ::std::ostream* os) {
  PrintTupleTo(t, os);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8>
void PrintTo(const ::std::tr1::tuple<T1, T2, T3, T4, T5, T6, T7, T8>& t,
             ::std::ostream* os) {
  PrintTupleTo(t, os);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8, typename T9>
void PrintTo(const ::std::tr1::tuple<T1, T2, T3, T4, T5, T6, T7, T8, T9>& t,
             ::std::ostream* os) {
  PrintTupleTo(t, os);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8, typename T9, typename T10>
void PrintTo(
    const ::std::tr1::tuple<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10>& t,
    ::std::ostream* os) {
  PrintTupleTo(t, os);
}

// Overload for std::pair.
template <typename T1, typename T2>
void PrintTo(const ::std::pair<T1, T2>& value, ::std::ostream* os) {
  *os << '(';
  UniversalPrinter<T1>::Print(value.first, os);
  *os << ", ";
  UniversalPrinter<T2>::Print(value.second, os);
  *os << ')';
}

// Implements printing a non-reference type T by letting the compiler
// pick the right overload of PrintTo() for T.
template <typename T>
class UniversalPrinter {
 public:
  // MSVC warns about adding const to a function type, so we want to
  // disable the warning.
#ifdef _MSC_VER
#pragma warning(push)          // Saves the current warning state.
#pragma warning(disable:4180)  // Temporarily disables warning 4180.
#endif  // _MSC_VER

  // Note: we deliberately don't call this PrintTo(), as that name
  // conflicts with ::testing::internal::PrintTo in the body of the
  // function.
  static void Print(const T& value, ::std::ostream* os) {
    // By default, ::testing::internal::PrintTo() is used for printing
    // the value.
    //
    // Thanks to Koenig look-up, if T is a class and has its own
    // PrintTo() function defined in its namespace, that function will
    // be visible here.  Since it is more specific than the generic ones
    // in ::testing::internal, it will be picked by the compiler in the
    // following statement - exactly what we want.
    PrintTo(value, os);
  }

#ifdef _MSC_VER
#pragma warning(pop)           // Restores the warning state.
#endif  // _MSC_VER
};

// UniversalPrintArray(begin, len, os) prints an array of 'len'
// elements, starting at address 'begin'.
template <typename T>
void UniversalPrintArray(const T* begin, size_t len, ::std::ostream* os) {
  if (len == 0) {
    *os << "{}";
  } else {
    *os << "{ ";
    const size_t kThreshold = 18;
    const size_t kChunkSize = 8;
    // If the array has more than kThreshold elements, we'll have to
    // omit some details by printing only the first and the last
    // kChunkSize elements.
    // TODO(wan@google.com): let the user control the threshold using a flag.
    if (len <= kThreshold) {
      PrintRawArrayTo(begin, len, os);
    } else {
      PrintRawArrayTo(begin, kChunkSize, os);
      *os << ", ..., ";
      PrintRawArrayTo(begin + len - kChunkSize, kChunkSize, os);
    }
    *os << " }";
  }
}
// This overload prints a (const) char array compactly.
void UniversalPrintArray(const char* begin, size_t len, ::std::ostream* os);

// Implements printing an array type T[N].
template <typename T, size_t N>
class UniversalPrinter<T[N]> {
 public:
  // Prints the given array, omitting some elements when there are too
  // many.
  static void Print(const T (&a)[N], ::std::ostream* os) {
    UniversalPrintArray(a, N, os);
  }
};

// Implements printing a reference type T&.
template <typename T>
class UniversalPrinter<T&> {
 public:
  // MSVC warns about adding const to a function type, so we want to
  // disable the warning.
#ifdef _MSC_VER
#pragma warning(push)          // Saves the current warning state.
#pragma warning(disable:4180)  // Temporarily disables warning 4180.
#endif  // _MSC_VER

  static void Print(const T& value, ::std::ostream* os) {
    // Prints the address of the value.  We use reinterpret_cast here
    // as static_cast doesn't compile when T is a function type.
    *os << "@" << reinterpret_cast<const void*>(&value) << " ";

    // Then prints the value itself.
    UniversalPrinter<T>::Print(value, os);
  }

#ifdef _MSC_VER
#pragma warning(pop)           // Restores the warning state.
#endif  // _MSC_VER
};

// Prints a value tersely: for a reference type, the referenced value
// (but not the address) is printed; for a (const) char pointer, the
// NUL-terminated string (but not the pointer) is printed.
template <typename T>
void UniversalTersePrint(const T& value, ::std::ostream* os) {
  UniversalPrinter<T>::Print(value, os);
}
inline void UniversalTersePrint(const char* str, ::std::ostream* os) {
  if (str == NULL) {
    *os << "NULL";
  } else {
    UniversalPrinter<string>::Print(string(str), os);
  }
}
inline void UniversalTersePrint(char* str, ::std::ostream* os) {
  UniversalTersePrint(static_cast<const char*>(str), os);
}

// Prints a value using the type inferred by the compiler.  The
// difference between this and UniversalTersePrint() is that for a
// (const) char pointer, this prints both the pointer and the
// NUL-terminated string.
template <typename T>
void UniversalPrint(const T& value, ::std::ostream* os) {
  UniversalPrinter<T>::Print(value, os);
}

typedef ::std::vector<string> Strings;

// This helper template allows PrintTo() for tuples and
// UniversalTersePrintTupleFieldsToStrings() to be defined by
// induction on the number of tuple fields.  The idea is that
// TuplePrefixPrinter<N>::PrintPrefixTo(t, os) prints the first N
// fields in tuple t, and can be defined in terms of
// TuplePrefixPrinter<N - 1>.

// The inductive case.
template <size_t N>
struct TuplePrefixPrinter {
  // Prints the first N fields of a tuple.
  template <typename Tuple>
  static void PrintPrefixTo(const Tuple& t, ::std::ostream* os) {
    TuplePrefixPrinter<N - 1>::PrintPrefixTo(t, os);
    *os << ", ";
    UniversalPrinter<typename ::std::tr1::tuple_element<N - 1, Tuple>::type>
        ::Print(::std::tr1::get<N - 1>(t), os);
  }

  // Tersely prints the first N fields of a tuple to a string vector,
  // one element for each field.
  template <typename Tuple>
  static void TersePrintPrefixToStrings(const Tuple& t, Strings* strings) {
    TuplePrefixPrinter<N - 1>::TersePrintPrefixToStrings(t, strings);
    ::std::stringstream ss;
    UniversalTersePrint(::std::tr1::get<N - 1>(t), &ss);
    strings->push_back(ss.str());
  }
};

// Base cases.
template <>
struct TuplePrefixPrinter<0> {
  template <typename Tuple>
  static void PrintPrefixTo(const Tuple&, ::std::ostream*) {}

  template <typename Tuple>
  static void TersePrintPrefixToStrings(const Tuple&, Strings*) {}
};
template <>
template <typename Tuple>
void TuplePrefixPrinter<1>::PrintPrefixTo(const Tuple& t, ::std::ostream* os) {
  UniversalPrinter<typename ::std::tr1::tuple_element<0, Tuple>::type>::
      Print(::std::tr1::get<0>(t), os);
}

// Helper function for printing a tuple.  T must be instantiated with
// a tuple type.
template <typename T>
void PrintTupleTo(const T& t, ::std::ostream* os) {
  *os << "(";
  TuplePrefixPrinter< ::std::tr1::tuple_size<T>::value>::
      PrintPrefixTo(t, os);
  *os << ")";
}

// Prints the fields of a tuple tersely to a string vector, one
// element for each field.  See the comment before
// UniversalTersePrint() for how we define "tersely".
template <typename Tuple>
Strings UniversalTersePrintTupleFieldsToStrings(const Tuple& value) {
  Strings result;
  TuplePrefixPrinter< ::std::tr1::tuple_size<Tuple>::value>::
      TersePrintPrefixToStrings(value, &result);
  return result;
}

}  // namespace internal

template <typename T>
::std::string PrintToString(const T& value) {
  ::std::stringstream ss;
  internal::UniversalTersePrint(value, &ss);
  return ss.str();
}

}  // namespace testing

#endif  // GMOCK_INCLUDE_GMOCK_GMOCK_PRINTERS_H_

namespace testing {

// To implement an action Foo, define:
//   1. a class FooAction that implements the ActionInterface interface, and
//   2. a factory function that creates an Action object from a
//      const FooAction*.
//
// The two-level delegation design follows that of Matcher, providing
// consistency for extension developers.  It also eases ownership
// management as Action objects can now be copied like plain values.

namespace internal {

template <typename F>
class MonomorphicDoDefaultActionImpl;

template <typename F1, typename F2>
class ActionAdaptor;

// BuiltInDefaultValue<T>::Get() returns the "built-in" default
// value for type T, which is NULL when T is a pointer type, 0 when T
// is a numeric type, false when T is bool, or "" when T is string or
// std::string.  For any other type T, this value is undefined and the
// function will abort the process.
template <typename T>
class BuiltInDefaultValue {
 public:
  // This function returns true iff type T has a built-in default value.
  static bool Exists() { return false; }
  static T Get() {
    Assert(false, __FILE__, __LINE__,
           "Default action undefined for the function return type.");
    return internal::Invalid<T>();
    // The above statement will never be reached, but is required in
    // order for this function to compile.
  }
};

// This partial specialization says that we use the same built-in
// default value for T and const T.
template <typename T>
class BuiltInDefaultValue<const T> {
 public:
  static bool Exists() { return BuiltInDefaultValue<T>::Exists(); }
  static T Get() { return BuiltInDefaultValue<T>::Get(); }
};

// This partial specialization defines the default values for pointer
// types.
template <typename T>
class BuiltInDefaultValue<T*> {
 public:
  static bool Exists() { return true; }
  static T* Get() { return NULL; }
};

// The following specializations define the default values for
// specific types we care about.
#define GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(type, value) \
  template <> \
  class BuiltInDefaultValue<type> { \
   public: \
    static bool Exists() { return true; } \
    static type Get() { return value; } \
  }

GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(void, );  // NOLINT
#if GTEST_HAS_GLOBAL_STRING
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(::string, "");
#endif  // GTEST_HAS_GLOBAL_STRING
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(::std::string, "");
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(bool, false);
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(unsigned char, '\0');
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(signed char, '\0');
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(char, '\0');

// There's no need for a default action for signed wchar_t, as that
// type is the same as wchar_t for gcc, and invalid for MSVC.
//
// There's also no need for a default action for unsigned wchar_t, as
// that type is the same as unsigned int for gcc, and invalid for
// MSVC.
#if GMOCK_WCHAR_T_IS_NATIVE_
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(wchar_t, 0U);  // NOLINT
#endif

GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(unsigned short, 0U);  // NOLINT
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(signed short, 0);     // NOLINT
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(unsigned int, 0U);
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(signed int, 0);
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(unsigned long, 0UL);  // NOLINT
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(signed long, 0L);     // NOLINT
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(UInt64, 0);
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(Int64, 0);
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(float, 0);
GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_(double, 0);

#undef GMOCK_DEFINE_DEFAULT_ACTION_FOR_RETURN_TYPE_

}  // namespace internal

// When an unexpected function call is encountered, Google Mock will
// let it return a default value if the user has specified one for its
// return type, or if the return type has a built-in default value;
// otherwise Google Mock won't know what value to return and will have
// to abort the process.
//
// The DefaultValue<T> class allows a user to specify the
// default value for a type T that is both copyable and publicly
// destructible (i.e. anything that can be used as a function return
// type).  The usage is:
//
//   // Sets the default value for type T to be foo.
//   DefaultValue<T>::Set(foo);
template <typename T>
class DefaultValue {
 public:
  // Sets the default value for type T; requires T to be
  // copy-constructable and have a public destructor.
  static void Set(T x) {
    delete value_;
    value_ = new T(x);
  }

  // Unsets the default value for type T.
  static void Clear() {
    delete value_;
    value_ = NULL;
  }

  // Returns true iff the user has set the default value for type T.
  static bool IsSet() { return value_ != NULL; }

  // Returns true if T has a default return value set by the user or there
  // exists a built-in default value.
  static bool Exists() {
    return IsSet() || internal::BuiltInDefaultValue<T>::Exists();
  }

  // Returns the default value for type T if the user has set one;
  // otherwise returns the built-in default value if there is one;
  // otherwise aborts the process.
  static T Get() {
    return value_ == NULL ?
        internal::BuiltInDefaultValue<T>::Get() : *value_;
  }
 private:
  static const T* value_;
};

// This partial specialization allows a user to set default values for
// reference types.
template <typename T>
class DefaultValue<T&> {
 public:
  // Sets the default value for type T&.
  static void Set(T& x) {  // NOLINT
    address_ = &x;
  }

  // Unsets the default value for type T&.
  static void Clear() {
    address_ = NULL;
  }

  // Returns true iff the user has set the default value for type T&.
  static bool IsSet() { return address_ != NULL; }

  // Returns true if T has a default return value set by the user or there
  // exists a built-in default value.
  static bool Exists() {
    return IsSet() || internal::BuiltInDefaultValue<T&>::Exists();
  }

  // Returns the default value for type T& if the user has set one;
  // otherwise returns the built-in default value if there is one;
  // otherwise aborts the process.
  static T& Get() {
    return address_ == NULL ?
        internal::BuiltInDefaultValue<T&>::Get() : *address_;
  }
 private:
  static T* address_;
};

// This specialization allows DefaultValue<void>::Get() to
// compile.
template <>
class DefaultValue<void> {
 public:
  static bool Exists() { return true; }
  static void Get() {}
};

// Points to the user-set default value for type T.
template <typename T>
const T* DefaultValue<T>::value_ = NULL;

// Points to the user-set default value for type T&.
template <typename T>
T* DefaultValue<T&>::address_ = NULL;

// Implement this interface to define an action for function type F.
template <typename F>
class ActionInterface {
 public:
  typedef typename internal::Function<F>::Result Result;
  typedef typename internal::Function<F>::ArgumentTuple ArgumentTuple;

  ActionInterface() : is_do_default_(false) {}

  virtual ~ActionInterface() {}

  // Performs the action.  This method is not const, as in general an
  // action can have side effects and be stateful.  For example, a
  // get-the-next-element-from-the-collection action will need to
  // remember the current element.
  virtual Result Perform(const ArgumentTuple& args) = 0;

  // Returns true iff this is the DoDefault() action.
  bool IsDoDefault() const { return is_do_default_; }

 private:
  template <typename Function>
  friend class internal::MonomorphicDoDefaultActionImpl;

  // This private constructor is reserved for implementing
  // DoDefault(), the default action for a given mock function.
  explicit ActionInterface(bool is_do_default)
      : is_do_default_(is_do_default) {}

  // True iff this action is DoDefault().
  const bool is_do_default_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(ActionInterface);
};

// An Action<F> is a copyable and IMMUTABLE (except by assignment)
// object that represents an action to be taken when a mock function
// of type F is called.  The implementation of Action<T> is just a
// linked_ptr to const ActionInterface<T>, so copying is fairly cheap.
// Don't inherit from Action!
//
// You can view an object implementing ActionInterface<F> as a
// concrete action (including its current state), and an Action<F>
// object as a handle to it.
template <typename F>
class Action {
 public:
  typedef typename internal::Function<F>::Result Result;
  typedef typename internal::Function<F>::ArgumentTuple ArgumentTuple;

  // Constructs a null Action.  Needed for storing Action objects in
  // STL containers.
  Action() : impl_(NULL) {}

  // Constructs an Action from its implementation.
  explicit Action(ActionInterface<F>* impl) : impl_(impl) {}

  // Copy constructor.
  Action(const Action& action) : impl_(action.impl_) {}

  // This constructor allows us to turn an Action<Func> object into an
  // Action<F>, as long as F's arguments can be implicitly converted
  // to Func's and Func's return type can be implicitly converted to
  // F's.
  template <typename Func>
  explicit Action(const Action<Func>& action);

  // Returns true iff this is the DoDefault() action.
  bool IsDoDefault() const { return impl_->IsDoDefault(); }

  // Performs the action.  Note that this method is const even though
  // the corresponding method in ActionInterface is not.  The reason
  // is that a const Action<F> means that it cannot be re-bound to
  // another concrete action, not that the concrete action it binds to
  // cannot change state.  (Think of the difference between a const
  // pointer and a pointer to const.)
  Result Perform(const ArgumentTuple& args) const {
    return impl_->Perform(args);
  }

 private:
  template <typename F1, typename F2>
  friend class internal::ActionAdaptor;

  internal::linked_ptr<ActionInterface<F> > impl_;
};

// The PolymorphicAction class template makes it easy to implement a
// polymorphic action (i.e. an action that can be used in mock
// functions of than one type, e.g. Return()).
//
// To define a polymorphic action, a user first provides a COPYABLE
// implementation class that has a Perform() method template:
//
//   class FooAction {
//    public:
//     template <typename Result, typename ArgumentTuple>
//     Result Perform(const ArgumentTuple& args) const {
//       // Processes the arguments and returns a result, using
//       // tr1::get<N>(args) to get the N-th (0-based) argument in the tuple.
//     }
//     ...
//   };
//
// Then the user creates the polymorphic action using
// MakePolymorphicAction(object) where object has type FooAction.  See
// the definition of Return(void) and SetArgumentPointee<N>(value) for
// complete examples.
template <typename Impl>
class PolymorphicAction {
 public:
  explicit PolymorphicAction(const Impl& impl) : impl_(impl) {}

  template <typename F>
  operator Action<F>() const {
    return Action<F>(new MonomorphicImpl<F>(impl_));
  }

 private:
  template <typename F>
  class MonomorphicImpl : public ActionInterface<F> {
   public:
    typedef typename internal::Function<F>::Result Result;
    typedef typename internal::Function<F>::ArgumentTuple ArgumentTuple;

    explicit MonomorphicImpl(const Impl& impl) : impl_(impl) {}

    virtual Result Perform(const ArgumentTuple& args) {
      return impl_.template Perform<Result>(args);
    }

   private:
    Impl impl_;

    GTEST_DISALLOW_ASSIGN_(MonomorphicImpl);
  };

  Impl impl_;

  GTEST_DISALLOW_ASSIGN_(PolymorphicAction);
};

// Creates an Action from its implementation and returns it.  The
// created Action object owns the implementation.
template <typename F>
Action<F> MakeAction(ActionInterface<F>* impl) {
  return Action<F>(impl);
}

// Creates a polymorphic action from its implementation.  This is
// easier to use than the PolymorphicAction<Impl> constructor as it
// doesn't require you to explicitly write the template argument, e.g.
//
//   MakePolymorphicAction(foo);
// vs
//   PolymorphicAction<TypeOfFoo>(foo);
template <typename Impl>
inline PolymorphicAction<Impl> MakePolymorphicAction(const Impl& impl) {
  return PolymorphicAction<Impl>(impl);
}

namespace internal {

// Allows an Action<F2> object to pose as an Action<F1>, as long as F2
// and F1 are compatible.
template <typename F1, typename F2>
class ActionAdaptor : public ActionInterface<F1> {
 public:
  typedef typename internal::Function<F1>::Result Result;
  typedef typename internal::Function<F1>::ArgumentTuple ArgumentTuple;

  explicit ActionAdaptor(const Action<F2>& from) : impl_(from.impl_) {}

  virtual Result Perform(const ArgumentTuple& args) {
    return impl_->Perform(args);
  }

 private:
  const internal::linked_ptr<ActionInterface<F2> > impl_;

  GTEST_DISALLOW_ASSIGN_(ActionAdaptor);
};

// Implements the polymorphic Return(x) action, which can be used in
// any function that returns the type of x, regardless of the argument
// types.
//
// Note: The value passed into Return must be converted into
// Function<F>::Result when this action is cast to Action<F> rather than
// when that action is performed. This is important in scenarios like
//
// MOCK_METHOD1(Method, T(U));
// ...
// {
//   Foo foo;
//   X x(&foo);
//   EXPECT_CALL(mock, Method(_)).WillOnce(Return(x));
// }
//
// In the example above the variable x holds reference to foo which leaves
// scope and gets destroyed.  If copying X just copies a reference to foo,
// that copy will be left with a hanging reference.  If conversion to T
// makes a copy of foo, the above code is safe. To support that scenario, we
// need to make sure that the type conversion happens inside the EXPECT_CALL
// statement, and conversion of the result of Return to Action<T(U)> is a
// good place for that.
//
template <typename R>
class ReturnAction {
 public:
  // Constructs a ReturnAction object from the value to be returned.
  // 'value' is passed by value instead of by const reference in order
  // to allow Return("string literal") to compile.
  explicit ReturnAction(R value) : value_(value) {}

  // This template type conversion operator allows Return(x) to be
  // used in ANY function that returns x's type.
  template <typename F>
  operator Action<F>() const {
    // Assert statement belongs here because this is the best place to verify
    // conditions on F. It produces the clearest error messages
    // in most compilers.
    // Impl really belongs in this scope as a local class but can't
    // because MSVC produces duplicate symbols in different translation units
    // in this case. Until MS fixes that bug we put Impl into the class scope
    // and put the typedef both here (for use in assert statement) and
    // in the Impl class. But both definitions must be the same.
    typedef typename Function<F>::Result Result;
    GMOCK_COMPILE_ASSERT_(
        !internal::is_reference<Result>::value,
        use_ReturnRef_instead_of_Return_to_return_a_reference);
    return Action<F>(new Impl<F>(value_));
  }

 private:
  // Implements the Return(x) action for a particular function type F.
  template <typename F>
  class Impl : public ActionInterface<F> {
   public:
    typedef typename Function<F>::Result Result;
    typedef typename Function<F>::ArgumentTuple ArgumentTuple;

    // The implicit cast is necessary when Result has more than one
    // single-argument constructor (e.g. Result is std::vector<int>) and R
    // has a type conversion operator template.  In that case, value_(value)
    // won't compile as the compiler doesn't known which constructor of
    // Result to call.  implicit_cast forces the compiler to convert R to
    // Result without considering explicit constructors, thus resolving the
    // ambiguity. value_ is then initialized using its copy constructor.
    explicit Impl(R value)
        : value_(::testing::internal::implicit_cast<Result>(value)) {}

    virtual Result Perform(const ArgumentTuple&) { return value_; }

   private:
    GMOCK_COMPILE_ASSERT_(!internal::is_reference<Result>::value,
                          Result_cannot_be_a_reference_type);
    Result value_;

    GTEST_DISALLOW_ASSIGN_(Impl);
  };

  R value_;

  GTEST_DISALLOW_ASSIGN_(ReturnAction);
};

// Implements the ReturnNull() action.
class ReturnNullAction {
 public:
  // Allows ReturnNull() to be used in any pointer-returning function.
  template <typename Result, typename ArgumentTuple>
  static Result Perform(const ArgumentTuple&) {
    GMOCK_COMPILE_ASSERT_(internal::is_pointer<Result>::value,
                          ReturnNull_can_be_used_to_return_a_pointer_only);
    return NULL;
  }
};

// Implements the Return() action.
class ReturnVoidAction {
 public:
  // Allows Return() to be used in any void-returning function.
  template <typename Result, typename ArgumentTuple>
  static void Perform(const ArgumentTuple&) {
    CompileAssertTypesEqual<void, Result>();
  }
};

// Implements the polymorphic ReturnRef(x) action, which can be used
// in any function that returns a reference to the type of x,
// regardless of the argument types.
template <typename T>
class ReturnRefAction {
 public:
  // Constructs a ReturnRefAction object from the reference to be returned.
  explicit ReturnRefAction(T& ref) : ref_(ref) {}  // NOLINT

  // This template type conversion operator allows ReturnRef(x) to be
  // used in ANY function that returns a reference to x's type.
  template <typename F>
  operator Action<F>() const {
    typedef typename Function<F>::Result Result;
    // Asserts that the function return type is a reference.  This
    // catches the user error of using ReturnRef(x) when Return(x)
    // should be used, and generates some helpful error message.
    GMOCK_COMPILE_ASSERT_(internal::is_reference<Result>::value,
                          use_Return_instead_of_ReturnRef_to_return_a_value);
    return Action<F>(new Impl<F>(ref_));
  }

 private:
  // Implements the ReturnRef(x) action for a particular function type F.
  template <typename F>
  class Impl : public ActionInterface<F> {
   public:
    typedef typename Function<F>::Result Result;
    typedef typename Function<F>::ArgumentTuple ArgumentTuple;

    explicit Impl(T& ref) : ref_(ref) {}  // NOLINT

    virtual Result Perform(const ArgumentTuple&) {
      return ref_;
    }

   private:
    T& ref_;

    GTEST_DISALLOW_ASSIGN_(Impl);
  };

  T& ref_;

  GTEST_DISALLOW_ASSIGN_(ReturnRefAction);
};

// Implements the DoDefault() action for a particular function type F.
template <typename F>
class MonomorphicDoDefaultActionImpl : public ActionInterface<F> {
 public:
  typedef typename Function<F>::Result Result;
  typedef typename Function<F>::ArgumentTuple ArgumentTuple;

  MonomorphicDoDefaultActionImpl() : ActionInterface<F>(true) {}

  // For technical reasons, DoDefault() cannot be used inside a
  // composite action (e.g. DoAll(...)).  It can only be used at the
  // top level in an EXPECT_CALL().  If this function is called, the
  // user must be using DoDefault() inside a composite action, and we
  // have to generate a run-time error.
  virtual Result Perform(const ArgumentTuple&) {
    Assert(false, __FILE__, __LINE__,
           "You are using DoDefault() inside a composite action like "
           "DoAll() or WithArgs().  This is not supported for technical "
           "reasons.  Please instead spell out the default action, or "
           "assign the default action to an Action variable and use "
           "the variable in various places.");
    return internal::Invalid<Result>();
    // The above statement will never be reached, but is required in
    // order for this function to compile.
  }
};

// Implements the polymorphic DoDefault() action.
class DoDefaultAction {
 public:
  // This template type conversion operator allows DoDefault() to be
  // used in any function.
  template <typename F>
  operator Action<F>() const {
    return Action<F>(new MonomorphicDoDefaultActionImpl<F>);
  }
};

// Implements the Assign action to set a given pointer referent to a
// particular value.
template <typename T1, typename T2>
class AssignAction {
 public:
  AssignAction(T1* ptr, T2 value) : ptr_(ptr), value_(value) {}

  template <typename Result, typename ArgumentTuple>
  void Perform(const ArgumentTuple& /* args */) const {
    *ptr_ = value_;
  }

 private:
  T1* const ptr_;
  const T2 value_;

  GTEST_DISALLOW_ASSIGN_(AssignAction);
};

#if !GTEST_OS_WINDOWS_MOBILE

// Implements the SetErrnoAndReturn action to simulate return from
// various system calls and libc functions.
template <typename T>
class SetErrnoAndReturnAction {
 public:
  SetErrnoAndReturnAction(int errno_value, T result)
      : errno_(errno_value),
        result_(result) {}
  template <typename Result, typename ArgumentTuple>
  Result Perform(const ArgumentTuple& /* args */) const {
    errno = errno_;
    return result_;
  }

 private:
  const int errno_;
  const T result_;

  GTEST_DISALLOW_ASSIGN_(SetErrnoAndReturnAction);
};

#endif  // !GTEST_OS_WINDOWS_MOBILE

// Implements the SetArgumentPointee<N>(x) action for any function
// whose N-th argument (0-based) is a pointer to x's type.  The
// template parameter kIsProto is true iff type A is ProtocolMessage,
// proto2::Message, or a sub-class of those.
template <size_t N, typename A, bool kIsProto>
class SetArgumentPointeeAction {
 public:
  // Constructs an action that sets the variable pointed to by the
  // N-th function argument to 'value'.
  explicit SetArgumentPointeeAction(const A& value) : value_(value) {}

  template <typename Result, typename ArgumentTuple>
  void Perform(const ArgumentTuple& args) const {
    CompileAssertTypesEqual<void, Result>();
    *::std::tr1::get<N>(args) = value_;
  }

 private:
  const A value_;

  GTEST_DISALLOW_ASSIGN_(SetArgumentPointeeAction);
};

template <size_t N, typename Proto>
class SetArgumentPointeeAction<N, Proto, true> {
 public:
  // Constructs an action that sets the variable pointed to by the
  // N-th function argument to 'proto'.  Both ProtocolMessage and
  // proto2::Message have the CopyFrom() method, so the same
  // implementation works for both.
  explicit SetArgumentPointeeAction(const Proto& proto) : proto_(new Proto) {
    proto_->CopyFrom(proto);
  }

  template <typename Result, typename ArgumentTuple>
  void Perform(const ArgumentTuple& args) const {
    CompileAssertTypesEqual<void, Result>();
    ::std::tr1::get<N>(args)->CopyFrom(*proto_);
  }

 private:
  const internal::linked_ptr<Proto> proto_;

  GTEST_DISALLOW_ASSIGN_(SetArgumentPointeeAction);
};

// Implements the InvokeWithoutArgs(f) action.  The template argument
// FunctionImpl is the implementation type of f, which can be either a
// function pointer or a functor.  InvokeWithoutArgs(f) can be used as an
// Action<F> as long as f's type is compatible with F (i.e. f can be
// assigned to a tr1::function<F>).
template <typename FunctionImpl>
class InvokeWithoutArgsAction {
 public:
  // The c'tor makes a copy of function_impl (either a function
  // pointer or a functor).
  explicit InvokeWithoutArgsAction(FunctionImpl function_impl)
      : function_impl_(function_impl) {}

  // Allows InvokeWithoutArgs(f) to be used as any action whose type is
  // compatible with f.
  template <typename Result, typename ArgumentTuple>
  Result Perform(const ArgumentTuple&) { return function_impl_(); }

 private:
  FunctionImpl function_impl_;

  GTEST_DISALLOW_ASSIGN_(InvokeWithoutArgsAction);
};

// Implements the InvokeWithoutArgs(object_ptr, &Class::Method) action.
template <class Class, typename MethodPtr>
class InvokeMethodWithoutArgsAction {
 public:
  InvokeMethodWithoutArgsAction(Class* obj_ptr, MethodPtr method_ptr)
      : obj_ptr_(obj_ptr), method_ptr_(method_ptr) {}

  template <typename Result, typename ArgumentTuple>
  Result Perform(const ArgumentTuple&) const {
    return (obj_ptr_->*method_ptr_)();
  }

 private:
  Class* const obj_ptr_;
  const MethodPtr method_ptr_;

  GTEST_DISALLOW_ASSIGN_(InvokeMethodWithoutArgsAction);
};

// Implements the IgnoreResult(action) action.
template <typename A>
class IgnoreResultAction {
 public:
  explicit IgnoreResultAction(const A& action) : action_(action) {}

  template <typename F>
  operator Action<F>() const {
    // Assert statement belongs here because this is the best place to verify
    // conditions on F. It produces the clearest error messages
    // in most compilers.
    // Impl really belongs in this scope as a local class but can't
    // because MSVC produces duplicate symbols in different translation units
    // in this case. Until MS fixes that bug we put Impl into the class scope
    // and put the typedef both here (for use in assert statement) and
    // in the Impl class. But both definitions must be the same.
    typedef typename internal::Function<F>::Result Result;

    // Asserts at compile time that F returns void.
    CompileAssertTypesEqual<void, Result>();

    return Action<F>(new Impl<F>(action_));
  }

 private:
  template <typename F>
  class Impl : public ActionInterface<F> {
   public:
    typedef typename internal::Function<F>::Result Result;
    typedef typename internal::Function<F>::ArgumentTuple ArgumentTuple;

    explicit Impl(const A& action) : action_(action) {}

    virtual void Perform(const ArgumentTuple& args) {
      // Performs the action and ignores its result.
      action_.Perform(args);
    }

   private:
    // Type OriginalFunction is the same as F except that its return
    // type is IgnoredValue.
    typedef typename internal::Function<F>::MakeResultIgnoredValue
        OriginalFunction;

    const Action<OriginalFunction> action_;

    GTEST_DISALLOW_ASSIGN_(Impl);
  };

  const A action_;

  GTEST_DISALLOW_ASSIGN_(IgnoreResultAction);
};

// A ReferenceWrapper<T> object represents a reference to type T,
// which can be either const or not.  It can be explicitly converted
// from, and implicitly converted to, a T&.  Unlike a reference,
// ReferenceWrapper<T> can be copied and can survive template type
// inference.  This is used to support by-reference arguments in the
// InvokeArgument<N>(...) action.  The idea was from "reference
// wrappers" in tr1, which we don't have in our source tree yet.
template <typename T>
class ReferenceWrapper {
 public:
  // Constructs a ReferenceWrapper<T> object from a T&.
  explicit ReferenceWrapper(T& l_value) : pointer_(&l_value) {}  // NOLINT

  // Allows a ReferenceWrapper<T> object to be implicitly converted to
  // a T&.
  operator T&() const { return *pointer_; }
 private:
  T* pointer_;
};

// Allows the expression ByRef(x) to be printed as a reference to x.
template <typename T>
void PrintTo(const ReferenceWrapper<T>& ref, ::std::ostream* os) {
  T& value = ref;
  UniversalPrinter<T&>::Print(value, os);
}

// Does two actions sequentially.  Used for implementing the DoAll(a1,
// a2, ...) action.
template <typename Action1, typename Action2>
class DoBothAction {
 public:
  DoBothAction(Action1 action1, Action2 action2)
      : action1_(action1), action2_(action2) {}

  // This template type conversion operator allows DoAll(a1, ..., a_n)
  // to be used in ANY function of compatible type.
  template <typename F>
  operator Action<F>() const {
    return Action<F>(new Impl<F>(action1_, action2_));
  }

 private:
  // Implements the DoAll(...) action for a particular function type F.
  template <typename F>
  class Impl : public ActionInterface<F> {
   public:
    typedef typename Function<F>::Result Result;
    typedef typename Function<F>::ArgumentTuple ArgumentTuple;
    typedef typename Function<F>::MakeResultVoid VoidResult;

    Impl(const Action<VoidResult>& action1, const Action<F>& action2)
        : action1_(action1), action2_(action2) {}

    virtual Result Perform(const ArgumentTuple& args) {
      action1_.Perform(args);
      return action2_.Perform(args);
    }

   private:
    const Action<VoidResult> action1_;
    const Action<F> action2_;

    GTEST_DISALLOW_ASSIGN_(Impl);
  };

  Action1 action1_;
  Action2 action2_;

  GTEST_DISALLOW_ASSIGN_(DoBothAction);
};

}  // namespace internal

// An Unused object can be implicitly constructed from ANY value.
// This is handy when defining actions that ignore some or all of the
// mock function arguments.  For example, given
//
//   MOCK_METHOD3(Foo, double(const string& label, double x, double y));
//   MOCK_METHOD3(Bar, double(int index, double x, double y));
//
// instead of
//
//   double DistanceToOriginWithLabel(const string& label, double x, double y) {
//     return sqrt(x*x + y*y);
//   }
//   double DistanceToOriginWithIndex(int index, double x, double y) {
//     return sqrt(x*x + y*y);
//   }
//   ...
//   EXEPCT_CALL(mock, Foo("abc", _, _))
//       .WillOnce(Invoke(DistanceToOriginWithLabel));
//   EXEPCT_CALL(mock, Bar(5, _, _))
//       .WillOnce(Invoke(DistanceToOriginWithIndex));
//
// you could write
//
//   // We can declare any uninteresting argument as Unused.
//   double DistanceToOrigin(Unused, double x, double y) {
//     return sqrt(x*x + y*y);
//   }
//   ...
//   EXEPCT_CALL(mock, Foo("abc", _, _)).WillOnce(Invoke(DistanceToOrigin));
//   EXEPCT_CALL(mock, Bar(5, _, _)).WillOnce(Invoke(DistanceToOrigin));
typedef internal::IgnoredValue Unused;

// This constructor allows us to turn an Action<From> object into an
// Action<To>, as long as To's arguments can be implicitly converted
// to From's and From's return type cann be implicitly converted to
// To's.
template <typename To>
template <typename From>
Action<To>::Action(const Action<From>& from)
    : impl_(new internal::ActionAdaptor<To, From>(from)) {}

// Creates an action that returns 'value'.  'value' is passed by value
// instead of const reference - otherwise Return("string literal")
// will trigger a compiler error about using array as initializer.
template <typename R>
internal::ReturnAction<R> Return(R value) {
  return internal::ReturnAction<R>(value);
}

// Creates an action that returns NULL.
inline PolymorphicAction<internal::ReturnNullAction> ReturnNull() {
  return MakePolymorphicAction(internal::ReturnNullAction());
}

// Creates an action that returns from a void function.
inline PolymorphicAction<internal::ReturnVoidAction> Return() {
  return MakePolymorphicAction(internal::ReturnVoidAction());
}

// Creates an action that returns the reference to a variable.
template <typename R>
inline internal::ReturnRefAction<R> ReturnRef(R& x) {  // NOLINT
  return internal::ReturnRefAction<R>(x);
}

// Creates an action that does the default action for the give mock function.
inline internal::DoDefaultAction DoDefault() {
  return internal::DoDefaultAction();
}

// Creates an action that sets the variable pointed by the N-th
// (0-based) function argument to 'value'.
template <size_t N, typename T>
PolymorphicAction<
  internal::SetArgumentPointeeAction<
    N, T, internal::IsAProtocolMessage<T>::value> >
SetArgumentPointee(const T& x) {
  return MakePolymorphicAction(internal::SetArgumentPointeeAction<
      N, T, internal::IsAProtocolMessage<T>::value>(x));
}

// Creates an action that sets a pointer referent to a given value.
template <typename T1, typename T2>
PolymorphicAction<internal::AssignAction<T1, T2> > Assign(T1* ptr, T2 val) {
  return MakePolymorphicAction(internal::AssignAction<T1, T2>(ptr, val));
}

#if !GTEST_OS_WINDOWS_MOBILE

// Creates an action that sets errno and returns the appropriate error.
template <typename T>
PolymorphicAction<internal::SetErrnoAndReturnAction<T> >
SetErrnoAndReturn(int errval, T result) {
  return MakePolymorphicAction(
      internal::SetErrnoAndReturnAction<T>(errval, result));
}

#endif  // !GTEST_OS_WINDOWS_MOBILE

// Various overloads for InvokeWithoutArgs().

// Creates an action that invokes 'function_impl' with no argument.
template <typename FunctionImpl>
PolymorphicAction<internal::InvokeWithoutArgsAction<FunctionImpl> >
InvokeWithoutArgs(FunctionImpl function_impl) {
  return MakePolymorphicAction(
      internal::InvokeWithoutArgsAction<FunctionImpl>(function_impl));
}

// Creates an action that invokes the given method on the given object
// with no argument.
template <class Class, typename MethodPtr>
PolymorphicAction<internal::InvokeMethodWithoutArgsAction<Class, MethodPtr> >
InvokeWithoutArgs(Class* obj_ptr, MethodPtr method_ptr) {
  return MakePolymorphicAction(
      internal::InvokeMethodWithoutArgsAction<Class, MethodPtr>(
          obj_ptr, method_ptr));
}

// Creates an action that performs an_action and throws away its
// result.  In other words, it changes the return type of an_action to
// void.  an_action MUST NOT return void, or the code won't compile.
template <typename A>
inline internal::IgnoreResultAction<A> IgnoreResult(const A& an_action) {
  return internal::IgnoreResultAction<A>(an_action);
}

// Creates a reference wrapper for the given L-value.  If necessary,
// you can explicitly specify the type of the reference.  For example,
// suppose 'derived' is an object of type Derived, ByRef(derived)
// would wrap a Derived&.  If you want to wrap a const Base& instead,
// where Base is a base class of Derived, just write:
//
//   ByRef<const Base>(derived)
template <typename T>
inline internal::ReferenceWrapper<T> ByRef(T& l_value) {  // NOLINT
  return internal::ReferenceWrapper<T>(l_value);
}

}  // namespace testing

#endif  // GMOCK_INCLUDE_GMOCK_GMOCK_ACTIONS_H_
// Copyright 2007, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Author: wan@google.com (Zhanyong Wan)

// Google Mock - a framework for writing C++ mock classes.
//
// This file implements some commonly used cardinalities.  More
// cardinalities can be defined by the user implementing the
// CardinalityInterface interface if necessary.

#ifndef GMOCK_INCLUDE_GMOCK_GMOCK_CARDINALITIES_H_
#define GMOCK_INCLUDE_GMOCK_GMOCK_CARDINALITIES_H_

#include <limits.h>
#include <ostream>  // NOLINT

namespace testing {

// To implement a cardinality Foo, define:
//   1. a class FooCardinality that implements the
//      CardinalityInterface interface, and
//   2. a factory function that creates a Cardinality object from a
//      const FooCardinality*.
//
// The two-level delegation design follows that of Matcher, providing
// consistency for extension developers.  It also eases ownership
// management as Cardinality objects can now be copied like plain values.

// The implementation of a cardinality.
class CardinalityInterface {
 public:
  virtual ~CardinalityInterface() {}

  // Conservative estimate on the lower/upper bound of the number of
  // calls allowed.
  virtual int ConservativeLowerBound() const { return 0; }
  virtual int ConservativeUpperBound() const { return INT_MAX; }

  // Returns true iff call_count calls will satisfy this cardinality.
  virtual bool IsSatisfiedByCallCount(int call_count) const = 0;

  // Returns true iff call_count calls will saturate this cardinality.
  virtual bool IsSaturatedByCallCount(int call_count) const = 0;

  // Describes self to an ostream.
  virtual void DescribeTo(::std::ostream* os) const = 0;
};

// A Cardinality is a copyable and IMMUTABLE (except by assignment)
// object that specifies how many times a mock function is expected to
// be called.  The implementation of Cardinality is just a linked_ptr
// to const CardinalityInterface, so copying is fairly cheap.
// Don't inherit from Cardinality!
class Cardinality {
 public:
  // Constructs a null cardinality.  Needed for storing Cardinality
  // objects in STL containers.
  Cardinality() {}

  // Constructs a Cardinality from its implementation.
  explicit Cardinality(const CardinalityInterface* impl) : impl_(impl) {}

  // Conservative estimate on the lower/upper bound of the number of
  // calls allowed.
  int ConservativeLowerBound() const { return impl_->ConservativeLowerBound(); }
  int ConservativeUpperBound() const { return impl_->ConservativeUpperBound(); }

  // Returns true iff call_count calls will satisfy this cardinality.
  bool IsSatisfiedByCallCount(int call_count) const {
    return impl_->IsSatisfiedByCallCount(call_count);
  }

  // Returns true iff call_count calls will saturate this cardinality.
  bool IsSaturatedByCallCount(int call_count) const {
    return impl_->IsSaturatedByCallCount(call_count);
  }

  // Returns true iff call_count calls will over-saturate this
  // cardinality, i.e. exceed the maximum number of allowed calls.
  bool IsOverSaturatedByCallCount(int call_count) const {
    return impl_->IsSaturatedByCallCount(call_count) &&
        !impl_->IsSatisfiedByCallCount(call_count);
  }

  // Describes self to an ostream
  void DescribeTo(::std::ostream* os) const { impl_->DescribeTo(os); }

  // Describes the given actual call count to an ostream.
  static void DescribeActualCallCountTo(int actual_call_count,
                                        ::std::ostream* os);
 private:
  internal::linked_ptr<const CardinalityInterface> impl_;
};

// Creates a cardinality that allows at least n calls.
Cardinality AtLeast(int n);

// Creates a cardinality that allows at most n calls.
Cardinality AtMost(int n);

// Creates a cardinality that allows any number of calls.
Cardinality AnyNumber();

// Creates a cardinality that allows between min and max calls.
Cardinality Between(int min, int max);

// Creates a cardinality that allows exactly n calls.
Cardinality Exactly(int n);

// Creates a cardinality from its implementation.
inline Cardinality MakeCardinality(const CardinalityInterface* c) {
  return Cardinality(c);
}

}  // namespace testing

#endif  // GMOCK_INCLUDE_GMOCK_GMOCK_CARDINALITIES_H_
// This file was GENERATED by a script.  DO NOT EDIT BY HAND!!!

// Copyright 2007, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Author: wan@google.com (Zhanyong Wan)

// Google Mock - a framework for writing C++ mock classes.
//
// This file implements some commonly used variadic actions.

#ifndef GMOCK_INCLUDE_GMOCK_GMOCK_GENERATED_ACTIONS_H_
#define GMOCK_INCLUDE_GMOCK_GMOCK_GENERATED_ACTIONS_H_


namespace testing {
namespace internal {

// InvokeHelper<F> knows how to unpack an N-tuple and invoke an N-ary
// function or method with the unpacked values, where F is a function
// type that takes N arguments.
template <typename Result, typename ArgumentTuple>
class InvokeHelper;

template <typename R>
class InvokeHelper<R, ::std::tr1::tuple<> > {
 public:
  template <typename Function>
  static R Invoke(Function function, const ::std::tr1::tuple<>&) {
    return function();
  }

  template <class Class, typename MethodPtr>
  static R InvokeMethod(Class* obj_ptr,
                        MethodPtr method_ptr,
                        const ::std::tr1::tuple<>&) {
    return (obj_ptr->*method_ptr)();
  }
};

template <typename R, typename A1>
class InvokeHelper<R, ::std::tr1::tuple<A1> > {
 public:
  template <typename Function>
  static R Invoke(Function function, const ::std::tr1::tuple<A1>& args) {
    using ::std::tr1::get;
    return function(get<0>(args));
  }

  template <class Class, typename MethodPtr>
  static R InvokeMethod(Class* obj_ptr,
                        MethodPtr method_ptr,
                        const ::std::tr1::tuple<A1>& args) {
    using ::std::tr1::get;
    return (obj_ptr->*method_ptr)(get<0>(args));
  }
};

template <typename R, typename A1, typename A2>
class InvokeHelper<R, ::std::tr1::tuple<A1, A2> > {
 public:
  template <typename Function>
  static R Invoke(Function function, const ::std::tr1::tuple<A1, A2>& args) {
    using ::std::tr1::get;
    return function(get<0>(args), get<1>(args));
  }

  template <class Class, typename MethodPtr>
  static R InvokeMethod(Class* obj_ptr,
                        MethodPtr method_ptr,
                        const ::std::tr1::tuple<A1, A2>& args) {
    using ::std::tr1::get;
    return (obj_ptr->*method_ptr)(get<0>(args), get<1>(args));
  }
};

template <typename R, typename A1, typename A2, typename A3>
class InvokeHelper<R, ::std::tr1::tuple<A1, A2, A3> > {
 public:
  template <typename Function>
  static R Invoke(Function function, const ::std::tr1::tuple<A1, A2,
      A3>& args) {
    using ::std::tr1::get;
    return function(get<0>(args), get<1>(args), get<2>(args));
  }

  template <class Class, typename MethodPtr>
  static R InvokeMethod(Class* obj_ptr,
                        MethodPtr method_ptr,
                        const ::std::tr1::tuple<A1, A2, A3>& args) {
    using ::std::tr1::get;
    return (obj_ptr->*method_ptr)(get<0>(args), get<1>(args), get<2>(args));
  }
};

template <typename R, typename A1, typename A2, typename A3, typename A4>
class InvokeHelper<R, ::std::tr1::tuple<A1, A2, A3, A4> > {
 public:
  template <typename Function>
  static R Invoke(Function function, const ::std::tr1::tuple<A1, A2, A3,
      A4>& args) {
    using ::std::tr1::get;
    return function(get<0>(args), get<1>(args), get<2>(args), get<3>(args));
  }

  template <class Class, typename MethodPtr>
  static R InvokeMethod(Class* obj_ptr,
                        MethodPtr method_ptr,
                        const ::std::tr1::tuple<A1, A2, A3, A4>& args) {
    using ::std::tr1::get;
    return (obj_ptr->*method_ptr)(get<0>(args), get<1>(args), get<2>(args),
        get<3>(args));
  }
};

template <typename R, typename A1, typename A2, typename A3, typename A4,
    typename A5>
class InvokeHelper<R, ::std::tr1::tuple<A1, A2, A3, A4, A5> > {
 public:
  template <typename Function>
  static R Invoke(Function function, const ::std::tr1::tuple<A1, A2, A3, A4,
      A5>& args) {
    using ::std::tr1::get;
    return function(get<0>(args), get<1>(args), get<2>(args), get<3>(args),
        get<4>(args));
  }

  template <class Class, typename MethodPtr>
  static R InvokeMethod(Class* obj_ptr,
                        MethodPtr method_ptr,
                        const ::std::tr1::tuple<A1, A2, A3, A4, A5>& args) {
    using ::std::tr1::get;
    return (obj_ptr->*method_ptr)(get<0>(args), get<1>(args), get<2>(args),
        get<3>(args), get<4>(args));
  }
};

template <typename R, typename A1, typename A2, typename A3, typename A4,
    typename A5, typename A6>
class InvokeHelper<R, ::std::tr1::tuple<A1, A2, A3, A4, A5, A6> > {
 public:
  template <typename Function>
  static R Invoke(Function function, const ::std::tr1::tuple<A1, A2, A3, A4,
      A5, A6>& args) {
    using ::std::tr1::get;
    return function(get<0>(args), get<1>(args), get<2>(args), get<3>(args),
        get<4>(args), get<5>(args));
  }

  template <class Class, typename MethodPtr>
  static R InvokeMethod(Class* obj_ptr,
                        MethodPtr method_ptr,
                        const ::std::tr1::tuple<A1, A2, A3, A4, A5, A6>& args) {
    using ::std::tr1::get;
    return (obj_ptr->*method_ptr)(get<0>(args), get<1>(args), get<2>(args),
        get<3>(args), get<4>(args), get<5>(args));
  }
};

template <typename R, typename A1, typename A2, typename A3, typename A4,
    typename A5, typename A6, typename A7>
class InvokeHelper<R, ::std::tr1::tuple<A1, A2, A3, A4, A5, A6, A7> > {
 public:
  template <typename Function>
  static R Invoke(Function function, const ::std::tr1::tuple<A1, A2, A3, A4,
      A5, A6, A7>& args) {
    using ::std::tr1::get;
    return function(get<0>(args), get<1>(args), get<2>(args), get<3>(args),
        get<4>(args), get<5>(args), get<6>(args));
  }

  template <class Class, typename MethodPtr>
  static R InvokeMethod(Class* obj_ptr,
                        MethodPtr method_ptr,
                        const ::std::tr1::tuple<A1, A2, A3, A4, A5, A6,
                            A7>& args) {
    using ::std::tr1::get;
    return (obj_ptr->*method_ptr)(get<0>(args), get<1>(args), get<2>(args),
        get<3>(args), get<4>(args), get<5>(args), get<6>(args));
  }
};

template <typename R, typename A1, typename A2, typename A3, typename A4,
    typename A5, typename A6, typename A7, typename A8>
class InvokeHelper<R, ::std::tr1::tuple<A1, A2, A3, A4, A5, A6, A7, A8> > {
 public:
  template <typename Function>
  static R Invoke(Function function, const ::std::tr1::tuple<A1, A2, A3, A4,
      A5, A6, A7, A8>& args) {
    using ::std::tr1::get;
    return function(get<0>(args), get<1>(args), get<2>(args), get<3>(args),
        get<4>(args), get<5>(args), get<6>(args), get<7>(args));
  }

  template <class Class, typename MethodPtr>
  static R InvokeMethod(Class* obj_ptr,
                        MethodPtr method_ptr,
                        const ::std::tr1::tuple<A1, A2, A3, A4, A5, A6, A7,
                            A8>& args) {
    using ::std::tr1::get;
    return (obj_ptr->*method_ptr)(get<0>(args), get<1>(args), get<2>(args),
        get<3>(args), get<4>(args), get<5>(args), get<6>(args), get<7>(args));
  }
};

template <typename R, typename A1, typename A2, typename A3, typename A4,
    typename A5, typename A6, typename A7, typename A8, typename A9>
class InvokeHelper<R, ::std::tr1::tuple<A1, A2, A3, A4, A5, A6, A7, A8, A9> > {
 public:
  template <typename Function>
  static R Invoke(Function function, const ::std::tr1::tuple<A1, A2, A3, A4,
      A5, A6, A7, A8, A9>& args) {
    using ::std::tr1::get;
    return function(get<0>(args), get<1>(args), get<2>(args), get<3>(args),
        get<4>(args), get<5>(args), get<6>(args), get<7>(args), get<8>(args));
  }

  template <class Class, typename MethodPtr>
  static R InvokeMethod(Class* obj_ptr,
                        MethodPtr method_ptr,
                        const ::std::tr1::tuple<A1, A2, A3, A4, A5, A6, A7, A8,
                            A9>& args) {
    using ::std::tr1::get;
    return (obj_ptr->*method_ptr)(get<0>(args), get<1>(args), get<2>(args),
        get<3>(args), get<4>(args), get<5>(args), get<6>(args), get<7>(args),
        get<8>(args));
  }
};

template <typename R, typename A1, typename A2, typename A3, typename A4,
    typename A5, typename A6, typename A7, typename A8, typename A9,
    typename A10>
class InvokeHelper<R, ::std::tr1::tuple<A1, A2, A3, A4, A5, A6, A7, A8, A9,
    A10> > {
 public:
  template <typename Function>
  static R Invoke(Function function, const ::std::tr1::tuple<A1, A2, A3, A4,
      A5, A6, A7, A8, A9, A10>& args) {
    using ::std::tr1::get;
    return function(get<0>(args), get<1>(args), get<2>(args), get<3>(args),
        get<4>(args), get<5>(args), get<6>(args), get<7>(args), get<8>(args),
        get<9>(args));
  }

  template <class Class, typename MethodPtr>
  static R InvokeMethod(Class* obj_ptr,
                        MethodPtr method_ptr,
                        const ::std::tr1::tuple<A1, A2, A3, A4, A5, A6, A7, A8,
                            A9, A10>& args) {
    using ::std::tr1::get;
    return (obj_ptr->*method_ptr)(get<0>(args), get<1>(args), get<2>(args),
        get<3>(args), get<4>(args), get<5>(args), get<6>(args), get<7>(args),
        get<8>(args), get<9>(args));
  }
};

// CallableHelper has static methods for invoking "callables",
// i.e. function pointers and functors.  It uses overloading to
// provide a uniform interface for invoking different kinds of
// callables.  In particular, you can use:
//
//   CallableHelper<R>::Call(callable, a1, a2, ..., an)
//
// to invoke an n-ary callable, where R is its return type.  If an
// argument, say a2, needs to be passed by reference, you should write
// ByRef(a2) instead of a2 in the above expression.
template <typename R>
class CallableHelper {
 public:
  // Calls a nullary callable.
  template <typename Function>
  static R Call(Function function) { return function(); }

  // Calls a unary callable.

  // We deliberately pass a1 by value instead of const reference here
  // in case it is a C-string literal.  If we had declared the
  // parameter as 'const A1& a1' and write Call(function, "Hi"), the
  // compiler would've thought A1 is 'char[3]', which causes trouble
  // when you need to copy a value of type A1.  By declaring the
  // parameter as 'A1 a1', the compiler will correctly infer that A1
  // is 'const char*' when it sees Call(function, "Hi").
  //
  // Since this function is defined inline, the compiler can get rid
  // of the copying of the arguments.  Therefore the performance won't
  // be hurt.
  template <typename Function, typename A1>
  static R Call(Function function, A1 a1) { return function(a1); }

  // Calls a binary callable.
  template <typename Function, typename A1, typename A2>
  static R Call(Function function, A1 a1, A2 a2) {
    return function(a1, a2);
  }

  // Calls a ternary callable.
  template <typename Function, typename A1, typename A2, typename A3>
  static R Call(Function function, A1 a1, A2 a2, A3 a3) {
    return function(a1, a2, a3);
  }

  // Calls a 4-ary callable.
  template <typename Function, typename A1, typename A2, typename A3,
      typename A4>
  static R Call(Function function, A1 a1, A2 a2, A3 a3, A4 a4) {
    return function(a1, a2, a3, a4);
  }

  // Calls a 5-ary callable.
  template <typename Function, typename A1, typename A2, typename A3,
      typename A4, typename A5>
  static R Call(Function function, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    return function(a1, a2, a3, a4, a5);
  }

  // Calls a 6-ary callable.
  template <typename Function, typename A1, typename A2, typename A3,
      typename A4, typename A5, typename A6>
  static R Call(Function function, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) {
    return function(a1, a2, a3, a4, a5, a6);
  }

  // Calls a 7-ary callable.
  template <typename Function, typename A1, typename A2, typename A3,
      typename A4, typename A5, typename A6, typename A7>
  static R Call(Function function, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6,
      A7 a7) {
    return function(a1, a2, a3, a4, a5, a6, a7);
  }

  // Calls a 8-ary callable.
  template <typename Function, typename A1, typename A2, typename A3,
      typename A4, typename A5, typename A6, typename A7, typename A8>
  static R Call(Function function, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6,
      A7 a7, A8 a8) {
    return function(a1, a2, a3, a4, a5, a6, a7, a8);
  }

  // Calls a 9-ary callable.
  template <typename Function, typename A1, typename A2, typename A3,
      typename A4, typename A5, typename A6, typename A7, typename A8,
      typename A9>
  static R Call(Function function, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6,
      A7 a7, A8 a8, A9 a9) {
    return function(a1, a2, a3, a4, a5, a6, a7, a8, a9);
  }

  // Calls a 10-ary callable.
  template <typename Function, typename A1, typename A2, typename A3,
      typename A4, typename A5, typename A6, typename A7, typename A8,
      typename A9, typename A10>
  static R Call(Function function, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6,
      A7 a7, A8 a8, A9 a9, A10 a10) {
    return function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
  }

};  // class CallableHelper

// An INTERNAL macro for extracting the type of a tuple field.  It's
// subject to change without notice - DO NOT USE IN USER CODE!
#define GMOCK_FIELD_(Tuple, N) \
    typename ::std::tr1::tuple_element<N, Tuple>::type

// SelectArgs<Result, ArgumentTuple, k1, k2, ..., k_n>::type is the
// type of an n-ary function whose i-th (1-based) argument type is the
// k{i}-th (0-based) field of ArgumentTuple, which must be a tuple
// type, and whose return type is Result.  For example,
//   SelectArgs<int, ::std::tr1::tuple<bool, char, double, long>, 0, 3>::type
// is int(bool, long).
//
// SelectArgs<Result, ArgumentTuple, k1, k2, ..., k_n>::Select(args)
// returns the selected fields (k1, k2, ..., k_n) of args as a tuple.
// For example,
//   SelectArgs<int, ::std::tr1::tuple<bool, char, double>, 2, 0>::Select(
//       ::std::tr1::make_tuple(true, 'a', 2.5))
// returns ::std::tr1::tuple (2.5, true).
//
// The numbers in list k1, k2, ..., k_n must be >= 0, where n can be
// in the range [0, 10].  Duplicates are allowed and they don't have
// to be in an ascending or descending order.

template <typename Result, typename ArgumentTuple, int k1, int k2, int k3,
    int k4, int k5, int k6, int k7, int k8, int k9, int k10>
class SelectArgs {
 public:
  typedef Result type(GMOCK_FIELD_(ArgumentTuple, k1),
      GMOCK_FIELD_(ArgumentTuple, k2), GMOCK_FIELD_(ArgumentTuple, k3),
      GMOCK_FIELD_(ArgumentTuple, k4), GMOCK_FIELD_(ArgumentTuple, k5),
      GMOCK_FIELD_(ArgumentTuple, k6), GMOCK_FIELD_(ArgumentTuple, k7),
      GMOCK_FIELD_(ArgumentTuple, k8), GMOCK_FIELD_(ArgumentTuple, k9),
      GMOCK_FIELD_(ArgumentTuple, k10));
  typedef typename Function<type>::ArgumentTuple SelectedArgs;
  static SelectedArgs Select(const ArgumentTuple& args) {
    using ::std::tr1::get;
    return SelectedArgs(get<k1>(args), get<k2>(args), get<k3>(args),
        get<k4>(args), get<k5>(args), get<k6>(args), get<k7>(args),
        get<k8>(args), get<k9>(args), get<k10>(args));
  }
};

template <typename Result, typename ArgumentTuple>
class SelectArgs<Result, ArgumentTuple,
                 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1> {
 public:
  typedef Result type();
  typedef typename Function<type>::ArgumentTuple SelectedArgs;
  static SelectedArgs Select(const ArgumentTuple& /* args */) {
    using ::std::tr1::get;
    return SelectedArgs();
  }
};

template <typename Result, typename ArgumentTuple, int k1>
class SelectArgs<Result, ArgumentTuple,
                 k1, -1, -1, -1, -1, -1, -1, -1, -1, -1> {
 public:
  typedef Result type(GMOCK_FIELD_(ArgumentTuple, k1));
  typedef typename Function<type>::ArgumentTuple SelectedArgs;
  static SelectedArgs Select(const ArgumentTuple& args) {
    using ::std::tr1::get;
    return SelectedArgs(get<k1>(args));
  }
};

template <typename Result, typename ArgumentTuple, int k1, int k2>
class SelectArgs<Result, ArgumentTuple,
                 k1, k2, -1, -1, -1, -1, -1, -1, -1, -1> {
 public:
  typedef Result type(GMOCK_FIELD_(ArgumentTuple, k1),
      GMOCK_FIELD_(ArgumentTuple, k2));
  typedef typename Function<type>::ArgumentTuple SelectedArgs;
  static SelectedArgs Select(const ArgumentTuple& args) {
    using ::std::tr1::get;
    return SelectedArgs(get<k1>(args), get<k2>(args));
  }
};

template <typename Result, typename ArgumentTuple, int k1, int k2, int k3>
class SelectArgs<Result, ArgumentTuple,
                 k1, k2, k3, -1, -1, -1, -1, -1, -1, -1> {
 public:
  typedef Result type(GMOCK_FIELD_(ArgumentTuple, k1),
      GMOCK_FIELD_(ArgumentTuple, k2), GMOCK_FIELD_(ArgumentTuple, k3));
  typedef typename Function<type>::ArgumentTuple SelectedArgs;
  static SelectedArgs Select(const ArgumentTuple& args) {
    using ::std::tr1::get;
    return SelectedArgs(get<k1>(args), get<k2>(args), get<k3>(args));
  }
};

template <typename Result, typename ArgumentTuple, int k1, int k2, int k3,
    int k4>
class SelectArgs<Result, ArgumentTuple,
                 k1, k2, k3, k4, -1, -1, -1, -1, -1, -1> {
 public:
  typedef Result type(GMOCK_FIELD_(ArgumentTuple, k1),
      GMOCK_FIELD_(ArgumentTuple, k2), GMOCK_FIELD_(ArgumentTuple, k3),
      GMOCK_FIELD_(ArgumentTuple, k4));
  typedef typename Function<type>::ArgumentTuple SelectedArgs;
  static SelectedArgs Select(const ArgumentTuple& args) {
    using ::std::tr1::get;
    return SelectedArgs(get<k1>(args), get<k2>(args), get<k3>(args),
        get<k4>(args));
  }
};

template <typename Result, typename ArgumentTuple, int k1, int k2, int k3,
    int k4, int k5>
class SelectArgs<Result, ArgumentTuple,
                 k1, k2, k3, k4, k5, -1, -1, -1, -1, -1> {
 public:
  typedef Result type(GMOCK_FIELD_(ArgumentTuple, k1),
      GMOCK_FIELD_(ArgumentTuple, k2), GMOCK_FIELD_(ArgumentTuple, k3),
      GMOCK_FIELD_(ArgumentTuple, k4), GMOCK_FIELD_(ArgumentTuple, k5));
  typedef typename Function<type>::ArgumentTuple SelectedArgs;
  static SelectedArgs Select(const ArgumentTuple& args) {
    using ::std::tr1::get;
    return SelectedArgs(get<k1>(args), get<k2>(args), get<k3>(args),
        get<k4>(args), get<k5>(args));
  }
};

template <typename Result, typename ArgumentTuple, int k1, int k2, int k3,
    int k4, int k5, int k6>
class SelectArgs<Result, ArgumentTuple,
                 k1, k2, k3, k4, k5, k6, -1, -1, -1, -1> {
 public:
  typedef Result type(GMOCK_FIELD_(ArgumentTuple, k1),
      GMOCK_FIELD_(ArgumentTuple, k2), GMOCK_FIELD_(ArgumentTuple, k3),
      GMOCK_FIELD_(ArgumentTuple, k4), GMOCK_FIELD_(ArgumentTuple, k5),
      GMOCK_FIELD_(ArgumentTuple, k6));
  typedef typename Function<type>::ArgumentTuple SelectedArgs;
  static SelectedArgs Select(const ArgumentTuple& args) {
    using ::std::tr1::get;
    return SelectedArgs(get<k1>(args), get<k2>(args), get<k3>(args),
        get<k4>(args), get<k5>(args), get<k6>(args));
  }
};

template <typename Result, typename ArgumentTuple, int k1, int k2, int k3,
    int k4, int k5, int k6, int k7>
class SelectArgs<Result, ArgumentTuple,
                 k1, k2, k3, k4, k5, k6, k7, -1, -1, -1> {
 public:
  typedef Result type(GMOCK_FIELD_(ArgumentTuple, k1),
      GMOCK_FIELD_(ArgumentTuple, k2), GMOCK_FIELD_(ArgumentTuple, k3),
      GMOCK_FIELD_(ArgumentTuple, k4), GMOCK_FIELD_(ArgumentTuple, k5),
      GMOCK_FIELD_(ArgumentTuple, k6), GMOCK_FIELD_(ArgumentTuple, k7));
  typedef typename Function<type>::ArgumentTuple SelectedArgs;
  static SelectedArgs Select(const ArgumentTuple& args) {
    using ::std::tr1::get;
    return SelectedArgs(get<k1>(args), get<k2>(args), get<k3>(args),
        get<k4>(args), get<k5>(args), get<k6>(args), get<k7>(args));
  }
};

template <typename Result, typename ArgumentTuple, int k1, int k2, int k3,
    int k4, int k5, int k6, int k7, int k8>
class SelectArgs<Result, ArgumentTuple,
                 k1, k2, k3, k4, k5, k6, k7, k8, -1, -1> {
 public:
  typedef Result type(GMOCK_FIELD_(ArgumentTuple, k1),
      GMOCK_FIELD_(ArgumentTuple, k2), GMOCK_FIELD_(ArgumentTuple, k3),
      GMOCK_FIELD_(ArgumentTuple, k4), GMOCK_FIELD_(ArgumentTuple, k5),
      GMOCK_FIELD_(ArgumentTuple, k6), GMOCK_FIELD_(ArgumentTuple, k7),
      GMOCK_FIELD_(ArgumentTuple, k8));
  typedef typename Function<type>::ArgumentTuple SelectedArgs;
  static SelectedArgs Select(const ArgumentTuple& args) {
    using ::std::tr1::get;
    return SelectedArgs(get<k1>(args), get<k2>(args), get<k3>(args),
        get<k4>(args), get<k5>(args), get<k6>(args), get<k7>(args),
        get<k8>(args));
  }
};

template <typename Result, typename ArgumentTuple, int k1, int k2, int k3,
    int k4, int k5, int k6, int k7, int k8, int k9>
class SelectArgs<Result, ArgumentTuple,
                 k1, k2, k3, k4, k5, k6, k7, k8, k9, -1> {
 public:
  typedef Result type(GMOCK_FIELD_(ArgumentTuple, k1),
      GMOCK_FIELD_(ArgumentTuple, k2), GMOCK_FIELD_(ArgumentTuple, k3),
      GMOCK_FIELD_(ArgumentTuple, k4), GMOCK_FIELD_(ArgumentTuple, k5),
      GMOCK_FIELD_(ArgumentTuple, k6), GMOCK_FIELD_(ArgumentTuple, k7),
      GMOCK_FIELD_(ArgumentTuple, k8), GMOCK_FIELD_(ArgumentTuple, k9));
  typedef typename Function<type>::ArgumentTuple SelectedArgs;
  static SelectedArgs Select(const ArgumentTuple& args) {
    using ::std::tr1::get;
    return SelectedArgs(get<k1>(args), get<k2>(args), get<k3>(args),
        get<k4>(args), get<k5>(args), get<k6>(args), get<k7>(args),
        get<k8>(args), get<k9>(args));
  }
};

#undef GMOCK_FIELD_

// Implements the WithArgs action.
template <typename InnerAction, int k1 = -1, int k2 = -1, int k3 = -1,
    int k4 = -1, int k5 = -1, int k6 = -1, int k7 = -1, int k8 = -1,
    int k9 = -1, int k10 = -1>
class WithArgsAction {
 public:
  explicit WithArgsAction(const InnerAction& action) : action_(action) {}

  template <typename F>
  operator Action<F>() const { return MakeAction(new Impl<F>(action_)); }

 private:
  template <typename F>
  class Impl : public ActionInterface<F> {
   public:
    typedef typename Function<F>::Result Result;
    typedef typename Function<F>::ArgumentTuple ArgumentTuple;

    explicit Impl(const InnerAction& action) : action_(action) {}

    virtual Result Perform(const ArgumentTuple& args) {
      return action_.Perform(SelectArgs<Result, ArgumentTuple, k1, k2, k3, k4,
          k5, k6, k7, k8, k9, k10>::Select(args));
    }

   private:
    typedef typename SelectArgs<Result, ArgumentTuple,
        k1, k2, k3, k4, k5, k6, k7, k8, k9, k10>::type InnerFunctionType;

    Action<InnerFunctionType> action_;
  };

  const InnerAction action_;

  GTEST_DISALLOW_ASSIGN_(WithArgsAction);
};

// A macro from the ACTION* family (defined later in this file)
// defines an action that can be used in a mock function.  Typically,
// these actions only care about a subset of the arguments of the mock
// function.  For example, if such an action only uses the second
// argument, it can be used in any mock function that takes >= 2
// arguments where the type of the second argument is compatible.
//
// Therefore, the action implementation must be prepared to take more
// arguments than it needs.  The ExcessiveArg type is used to
// represent those excessive arguments.  In order to keep the compiler
// error messages tractable, we define it in the testing namespace
// instead of testing::internal.  However, this is an INTERNAL TYPE
// and subject to change without notice, so a user MUST NOT USE THIS
// TYPE DIRECTLY.
struct ExcessiveArg {};

// A helper class needed for implementing the ACTION* macros.
template <typename Result, class Impl>
class ActionHelper {
 public:
  static Result Perform(Impl* impl, const ::std::tr1::tuple<>& args) {
    using ::std::tr1::get;
    return impl->template gmock_PerformImpl<>(args, ExcessiveArg(),
        ExcessiveArg(), ExcessiveArg(), ExcessiveArg(), ExcessiveArg(),
        ExcessiveArg(), ExcessiveArg(), ExcessiveArg(), ExcessiveArg(),
        ExcessiveArg());
  }

  template <typename A0>
  static Result Perform(Impl* impl, const ::std::tr1::tuple<A0>& args) {
    using ::std::tr1::get;
    return impl->template gmock_PerformImpl<A0>(args, get<0>(args),
        ExcessiveArg(), ExcessiveArg(), ExcessiveArg(), ExcessiveArg(),
        ExcessiveArg(), ExcessiveArg(), ExcessiveArg(), ExcessiveArg(),
        ExcessiveArg());
  }

  template <typename A0, typename A1>
  static Result Perform(Impl* impl, const ::std::tr1::tuple<A0, A1>& args) {
    using ::std::tr1::get;
    return impl->template gmock_PerformImpl<A0, A1>(args, get<0>(args),
        get<1>(args), ExcessiveArg(), ExcessiveArg(), ExcessiveArg(),
        ExcessiveArg(), ExcessiveArg(), ExcessiveArg(), ExcessiveArg(),
        ExcessiveArg());
  }

  template <typename A0, typename A1, typename A2>
  static Result Perform(Impl* impl, const ::std::tr1::tuple<A0, A1, A2>& args) {
    using ::std::tr1::get;
    return impl->template gmock_PerformImpl<A0, A1, A2>(args, get<0>(args),
        get<1>(args), get<2>(args), ExcessiveArg(), ExcessiveArg(),
        ExcessiveArg(), ExcessiveArg(), ExcessiveArg(), ExcessiveArg(),
        ExcessiveArg());
  }

  template <typename A0, typename A1, typename A2, typename A3>
  static Result Perform(Impl* impl, const ::std::tr1::tuple<A0, A1, A2,
      A3>& args) {
    using ::std::tr1::get;
    return impl->template gmock_PerformImpl<A0, A1, A2, A3>(args, get<0>(args),
        get<1>(args), get<2>(args), get<3>(args), ExcessiveArg(),
        ExcessiveArg(), ExcessiveArg(), ExcessiveArg(), ExcessiveArg(),
        ExcessiveArg());
  }

  template <typename A0, typename A1, typename A2, typename A3, typename A4>
  static Result Perform(Impl* impl, const ::std::tr1::tuple<A0, A1, A2, A3,
      A4>& args) {
    using ::std::tr1::get;
    return impl->template gmock_PerformImpl<A0, A1, A2, A3, A4>(args,
        get<0>(args), get<1>(args), get<2>(args), get<3>(args), get<4>(args),
        ExcessiveArg(), ExcessiveArg(), ExcessiveArg(), ExcessiveArg(),
        ExcessiveArg());
  }

  template <typename A0, typename A1, typename A2, typename A3, typename A4,
      typename A5>
  static Result Perform(Impl* impl, const ::std::tr1::tuple<A0, A1, A2, A3, A4,
      A5>& args) {
    using ::std::tr1::get;
    return impl->template gmock_PerformImpl<A0, A1, A2, A3, A4, A5>(args,
        get<0>(args), get<1>(args), get<2>(args), get<3>(args), get<4>(args),
        get<5>(args), ExcessiveArg(), ExcessiveArg(), ExcessiveArg(),
        ExcessiveArg());
  }

  template <typename A0, typename A1, typename A2, typename A3, typename A4,
      typename A5, typename A6>
  static Result Perform(Impl* impl, const ::std::tr1::tuple<A0, A1, A2, A3, A4,
      A5, A6>& args) {
    using ::std::tr1::get;
    return impl->template gmock_PerformImpl<A0, A1, A2, A3, A4, A5, A6>(args,
        get<0>(args), get<1>(args), get<2>(args), get<3>(args), get<4>(args),
        get<5>(args), get<6>(args), ExcessiveArg(), ExcessiveArg(),
        ExcessiveArg());
  }

  template <typename A0, typename A1, typename A2, typename A3, typename A4,
      typename A5, typename A6, typename A7>
  static Result Perform(Impl* impl, const ::std::tr1::tuple<A0, A1, A2, A3, A4,
      A5, A6, A7>& args) {
    using ::std::tr1::get;
    return impl->template gmock_PerformImpl<A0, A1, A2, A3, A4, A5, A6,
        A7>(args, get<0>(args), get<1>(args), get<2>(args), get<3>(args),
        get<4>(args), get<5>(args), get<6>(args), get<7>(args), ExcessiveArg(),
        ExcessiveArg());
  }

  template <typename A0, typename A1, typename A2, typename A3, typename A4,
      typename A5, typename A6, typename A7, typename A8>
  static Result Perform(Impl* impl, const ::std::tr1::tuple<A0, A1, A2, A3, A4,
      A5, A6, A7, A8>& args) {
    using ::std::tr1::get;
    return impl->template gmock_PerformImpl<A0, A1, A2, A3, A4, A5, A6, A7,
        A8>(args, get<0>(args), get<1>(args), get<2>(args), get<3>(args),
        get<4>(args), get<5>(args), get<6>(args), get<7>(args), get<8>(args),
        ExcessiveArg());
  }

  template <typename A0, typename A1, typename A2, typename A3, typename A4,
      typename A5, typename A6, typename A7, typename A8, typename A9>
  static Result Perform(Impl* impl, const ::std::tr1::tuple<A0, A1, A2, A3, A4,
      A5, A6, A7, A8, A9>& args) {
    using ::std::tr1::get;
    return impl->template gmock_PerformImpl<A0, A1, A2, A3, A4, A5, A6, A7, A8,
        A9>(args, get<0>(args), get<1>(args), get<2>(args), get<3>(args),
        get<4>(args), get<5>(args), get<6>(args), get<7>(args), get<8>(args),
        get<9>(args));
  }
};

}  // namespace internal

// Various overloads for Invoke().

// WithArgs<N1, N2, ..., Nk>(an_action) creates an action that passes
// the selected arguments of the mock function to an_action and
// performs it.  It serves as an adaptor between actions with
// different argument lists.  C++ doesn't support default arguments for
// function templates, so we have to overload it.
template <int k1, typename InnerAction>
inline internal::WithArgsAction<InnerAction, k1>
WithArgs(const InnerAction& action) {
  return internal::WithArgsAction<InnerAction, k1>(action);
}

template <int k1, int k2, typename InnerAction>
inline internal::WithArgsAction<InnerAction, k1, k2>
WithArgs(const InnerAction& action) {
  return internal::WithArgsAction<InnerAction, k1, k2>(action);
}

template <int k1, int k2, int k3, typename InnerAction>
inline internal::WithArgsAction<InnerAction, k1, k2, k3>
WithArgs(const InnerAction& action) {
  return internal::WithArgsAction<InnerAction, k1, k2, k3>(action);
}

template <int k1, int k2, int k3, int k4, typename InnerAction>
inline internal::WithArgsAction<InnerAction, k1, k2, k3, k4>
WithArgs(const InnerAction& action) {
  return internal::WithArgsAction<InnerAction, k1, k2, k3, k4>(action);
}

template <int k1, int k2, int k3, int k4, int k5, typename InnerAction>
inline internal::WithArgsAction<InnerAction, k1, k2, k3, k4, k5>
WithArgs(const InnerAction& action) {
  return internal::WithArgsAction<InnerAction, k1, k2, k3, k4, k5>(action);
}

template <int k1, int k2, int k3, int k4, int k5, int k6, typename InnerAction>
inline internal::WithArgsAction<InnerAction, k1, k2, k3, k4, k5, k6>
WithArgs(const InnerAction& action) {
  return internal::WithArgsAction<InnerAction, k1, k2, k3, k4, k5, k6>(action);
}

template <int k1, int k2, int k3, int k4, int k5, int k6, int k7,
    typename InnerAction>
inline internal::WithArgsAction<InnerAction, k1, k2, k3, k4, k5, k6, k7>
WithArgs(const InnerAction& action) {
  return internal::WithArgsAction<InnerAction, k1, k2, k3, k4, k5, k6,
      k7>(action);
}

template <int k1, int k2, int k3, int k4, int k5, int k6, int k7, int k8,
    typename InnerAction>
inline internal::WithArgsAction<InnerAction, k1, k2, k3, k4, k5, k6, k7, k8>
WithArgs(const InnerAction& action) {
  return internal::WithArgsAction<InnerAction, k1, k2, k3, k4, k5, k6, k7,
      k8>(action);
}

template <int k1, int k2, int k3, int k4, int k5, int k6, int k7, int k8,
    int k9, typename InnerAction>
inline internal::WithArgsAction<InnerAction, k1, k2, k3, k4, k5, k6, k7, k8, k9>
WithArgs(const InnerAction& action) {
  return internal::WithArgsAction<InnerAction, k1, k2, k3, k4, k5, k6, k7, k8,
      k9>(action);
}

template <int k1, int k2, int k3, int k4, int k5, int k6, int k7, int k8,
    int k9, int k10, typename InnerAction>
inline internal::WithArgsAction<InnerAction, k1, k2, k3, k4, k5, k6, k7, k8,
    k9, k10>
WithArgs(const InnerAction& action) {
  return internal::WithArgsAction<InnerAction, k1, k2, k3, k4, k5, k6, k7, k8,
      k9, k10>(action);
}

// Creates an action that does actions a1, a2, ..., sequentially in
// each invocation.
template <typename Action1, typename Action2>
inline internal::DoBothAction<Action1, Action2>
DoAll(Action1 a1, Action2 a2) {
  return internal::DoBothAction<Action1, Action2>(a1, a2);
}

template <typename Action1, typename Action2, typename Action3>
inline internal::DoBothAction<Action1, internal::DoBothAction<Action2,
    Action3> >
DoAll(Action1 a1, Action2 a2, Action3 a3) {
  return DoAll(a1, DoAll(a2, a3));
}

template <typename Action1, typename Action2, typename Action3,
    typename Action4>
inline internal::DoBothAction<Action1, internal::DoBothAction<Action2,
    internal::DoBothAction<Action3, Action4> > >
DoAll(Action1 a1, Action2 a2, Action3 a3, Action4 a4) {
  return DoAll(a1, DoAll(a2, a3, a4));
}

template <typename Action1, typename Action2, typename Action3,
    typename Action4, typename Action5>
inline internal::DoBothAction<Action1, internal::DoBothAction<Action2,
    internal::DoBothAction<Action3, internal::DoBothAction<Action4,
    Action5> > > >
DoAll(Action1 a1, Action2 a2, Action3 a3, Action4 a4, Action5 a5) {
  return DoAll(a1, DoAll(a2, a3, a4, a5));
}

template <typename Action1, typename Action2, typename Action3,
    typename Action4, typename Action5, typename Action6>
inline internal::DoBothAction<Action1, internal::DoBothAction<Action2,
    internal::DoBothAction<Action3, internal::DoBothAction<Action4,
    internal::DoBothAction<Action5, Action6> > > > >
DoAll(Action1 a1, Action2 a2, Action3 a3, Action4 a4, Action5 a5, Action6 a6) {
  return DoAll(a1, DoAll(a2, a3, a4, a5, a6));
}

template <typename Action1, typename Action2, typename Action3,
    typename Action4, typename Action5, typename Action6, typename Action7>
inline internal::DoBothAction<Action1, internal::DoBothAction<Action2,
    internal::DoBothAction<Action3, internal::DoBothAction<Action4,
    internal::DoBothAction<Action5, internal::DoBothAction<Action6,
    Action7> > > > > >
DoAll(Action1 a1, Action2 a2, Action3 a3, Action4 a4, Action5 a5, Action6 a6,
    Action7 a7) {
  return DoAll(a1, DoAll(a2, a3, a4, a5, a6, a7));
}

template <typename Action1, typename Action2, typename Action3,
    typename Action4, typename Action5, typename Action6, typename Action7,
    typename Action8>
inline internal::DoBothAction<Action1, internal::DoBothAction<Action2,
    internal::DoBothAction<Action3, internal::DoBothAction<Action4,
    internal::DoBothAction<Action5, internal::DoBothAction<Action6,
    internal::DoBothAction<Action7, Action8> > > > > > >
DoAll(Action1 a1, Action2 a2, Action3 a3, Action4 a4, Action5 a5, Action6 a6,
    Action7 a7, Action8 a8) {
  return DoAll(a1, DoAll(a2, a3, a4, a5, a6, a7, a8));
}

template <typename Action1, typename Action2, typename Action3,
    typename Action4, typename Action5, typename Action6, typename Action7,
    typename Action8, typename Action9>
inline internal::DoBothAction<Action1, internal::DoBothAction<Action2,
    internal::DoBothAction<Action3, internal::DoBothAction<Action4,
    internal::DoBothAction<Action5, internal::DoBothAction<Action6,
    internal::DoBothAction<Action7, internal::DoBothAction<Action8,
    Action9> > > > > > > >
DoAll(Action1 a1, Action2 a2, Action3 a3, Action4 a4, Action5 a5, Action6 a6,
    Action7 a7, Action8 a8, Action9 a9) {
  return DoAll(a1, DoAll(a2, a3, a4, a5, a6, a7, a8, a9));
}

template <typename Action1, typename Action2, typename Action3,
    typename Action4, typename Action5, typename Action6, typename Action7,
    typename Action8, typename Action9, typename Action10>
inline internal::DoBothAction<Action1, internal::DoBothAction<Action2,
    internal::DoBothAction<Action3, internal::DoBothAction<Action4,
    internal::DoBothAction<Action5, internal::DoBothAction<Action6,
    internal::DoBothAction<Action7, internal::DoBothAction<Action8,
    internal::DoBothAction<Action9, Action10> > > > > > > > >
DoAll(Action1 a1, Action2 a2, Action3 a3, Action4 a4, Action5 a5, Action6 a6,
    Action7 a7, Action8 a8, Action9 a9, Action10 a10) {
  return DoAll(a1, DoAll(a2, a3, a4, a5, a6, a7, a8, a9, a10));
}

}  // namespace testing

// The ACTION* family of macros can be used in a namespace scope to
// define custom actions easily.  The syntax:
//
//   ACTION(name) { statements; }
//
// will define an action with the given name that executes the
// statements.  The value returned by the statements will be used as
// the return value of the action.  Inside the statements, you can
// refer to the K-th (0-based) argument of the mock function by
// 'argK', and refer to its type by 'argK_type'.  For example:
//
//   ACTION(IncrementArg1) {
//     arg1_type temp = arg1;
//     return ++(*temp);
//   }
//
// allows you to write
//
//   ...WillOnce(IncrementArg1());
//
// You can also refer to the entire argument tuple and its type by
// 'args' and 'args_type', and refer to the mock function type and its
// return type by 'function_type' and 'return_type'.
//
// Note that you don't need to specify the types of the mock function
// arguments.  However rest assured that your code is still type-safe:
// you'll get a compiler error if *arg1 doesn't support the ++
// operator, or if the type of ++(*arg1) isn't compatible with the
// mock function's return type, for example.
//
// Sometimes you'll want to parameterize the action.   For that you can use
// another macro:
//
//   ACTION_P(name, param_name) { statements; }
//
// For example:
//
//   ACTION_P(Add, n) { return arg0 + n; }
//
// will allow you to write:
//
//   ...WillOnce(Add(5));
//
// Note that you don't need to provide the type of the parameter
// either.  If you need to reference the type of a parameter named
// 'foo', you can write 'foo_type'.  For example, in the body of
// ACTION_P(Add, n) above, you can write 'n_type' to refer to the type
// of 'n'.
//
// We also provide ACTION_P2, ACTION_P3, ..., up to ACTION_P10 to support
// multi-parameter actions.
//
// For the purpose of typing, you can view
//
//   ACTION_Pk(Foo, p1, ..., pk) { ... }
//
// as shorthand for
//
//   template <typename p1_type, ..., typename pk_type>
//   FooActionPk<p1_type, ..., pk_type> Foo(p1_type p1, ..., pk_type pk) { ... }
//
// In particular, you can provide the template type arguments
// explicitly when invoking Foo(), as in Foo<long, bool>(5, false);
// although usually you can rely on the compiler to infer the types
// for you automatically.  You can assign the result of expression
// Foo(p1, ..., pk) to a variable of type FooActionPk<p1_type, ...,
// pk_type>.  This can be useful when composing actions.
//
// You can also overload actions with different numbers of parameters:
//
//   ACTION_P(Plus, a) { ... }
//   ACTION_P2(Plus, a, b) { ... }
//
// While it's tempting to always use the ACTION* macros when defining
// a new action, you should also consider implementing ActionInterface
// or using MakePolymorphicAction() instead, especially if you need to
// use the action a lot.  While these approaches require more work,
// they give you more control on the types of the mock function
// arguments and the action parameters, which in general leads to
// better compiler error messages that pay off in the long run.  They
// also allow overloading actions based on parameter types (as opposed
// to just based on the number of parameters).
//
// CAVEAT:
//
// ACTION*() can only be used in a namespace scope.  The reason is
// that C++ doesn't yet allow function-local types to be used to
// instantiate templates.  The up-coming C++0x standard will fix this.
// Once that's done, we'll consider supporting using ACTION*() inside
// a function.
//
// MORE INFORMATION:
//
// To learn more about using these macros, please search for 'ACTION'
// on http://code.google.com/p/googlemock/wiki/CookBook.

// An internal macro needed for implementing ACTION*().
#define GMOCK_ACTION_ARG_TYPES_AND_NAMES_UNUSED_\
    const args_type& args GTEST_ATTRIBUTE_UNUSED_,\
    arg0_type arg0 GTEST_ATTRIBUTE_UNUSED_,\
    arg1_type arg1 GTEST_ATTRIBUTE_UNUSED_,\
    arg2_type arg2 GTEST_ATTRIBUTE_UNUSED_,\
    arg3_type arg3 GTEST_ATTRIBUTE_UNUSED_,\
    arg4_type arg4 GTEST_ATTRIBUTE_UNUSED_,\
    arg5_type arg5 GTEST_ATTRIBUTE_UNUSED_,\
    arg6_type arg6 GTEST_ATTRIBUTE_UNUSED_,\
    arg7_type arg7 GTEST_ATTRIBUTE_UNUSED_,\
    arg8_type arg8 GTEST_ATTRIBUTE_UNUSED_,\
    arg9_type arg9 GTEST_ATTRIBUTE_UNUSED_

// Sometimes you want to give an action explicit template parameters
// that cannot be inferred from its value parameters.  ACTION() and
// ACTION_P*() don't support that.  ACTION_TEMPLATE() remedies that
// and can be viewed as an extension to ACTION() and ACTION_P*().
//
// The syntax:
//
//   ACTION_TEMPLATE(ActionName,
//                   HAS_m_TEMPLATE_PARAMS(kind1, name1, ..., kind_m, name_m),
//                   AND_n_VALUE_PARAMS(p1, ..., p_n)) { statements; }
//
// defines an action template that takes m explicit template
// parameters and n value parameters.  name_i is the name of the i-th
// template parameter, and kind_i specifies whether it's a typename,
// an integral constant, or a template.  p_i is the name of the i-th
// value parameter.
//
// Example:
//
//   // DuplicateArg<k, T>(output) converts the k-th argument of the mock
//   // function to type T and copies it to *output.
//   ACTION_TEMPLATE(DuplicateArg,
//                   HAS_2_TEMPLATE_PARAMS(int, k, typename, T),
//                   AND_1_VALUE_PARAMS(output)) {
//     *output = T(std::tr1::get<k>(args));
//   }
//   ...
//     int n;
//     EXPECT_CALL(mock, Foo(_, _))
//         .WillOnce(DuplicateArg<1, unsigned char>(&n));
//
// To create an instance of an action template, write:
//
//   ActionName<t1, ..., t_m>(v1, ..., v_n)
//
// where the ts are the template arguments and the vs are the value
// arguments.  The value argument types are inferred by the compiler.
// If you want to explicitly specify the value argument types, you can
// provide additional template arguments:
//
//   ActionName<t1, ..., t_m, u1, ..., u_k>(v1, ..., v_n)
//
// where u_i is the desired type of v_i.
//
// ACTION_TEMPLATE and ACTION/ACTION_P* can be overloaded on the
// number of value parameters, but not on the number of template
// parameters.  Without the restriction, the meaning of the following
// is unclear:
//
//   OverloadedAction<int, bool>(x);
//
// Are we using a single-template-parameter action where 'bool' refers
// to the type of x, or are we using a two-template-parameter action
// where the compiler is asked to infer the type of x?
//
// Implementation notes:
//
// GMOCK_INTERNAL_*_HAS_m_TEMPLATE_PARAMS and
// GMOCK_INTERNAL_*_AND_n_VALUE_PARAMS are internal macros for
// implementing ACTION_TEMPLATE.  The main trick we use is to create
// new macro invocations when expanding a macro.  For example, we have
//
//   #define ACTION_TEMPLATE(name, template_params, value_params)
//       ... GMOCK_INTERNAL_DECL_##template_params ...
//
// which causes ACTION_TEMPLATE(..., HAS_1_TEMPLATE_PARAMS(typename, T), ...)
// to expand to
//
//       ... GMOCK_INTERNAL_DECL_HAS_1_TEMPLATE_PARAMS(typename, T) ...
//
// Since GMOCK_INTERNAL_DECL_HAS_1_TEMPLATE_PARAMS is a macro, the
// preprocessor will continue to expand it to
//
//       ... typename T ...
//
// This technique conforms to the C++ standard and is portable.  It
// allows us to implement action templates using O(N) code, where N is
// the maximum number of template/value parameters supported.  Without
// using it, we'd have to devote O(N^2) amount of code to implement all
// combinations of m and n.

// Declares the template parameters.
#define GMOCK_INTERNAL_DECL_HAS_1_TEMPLATE_PARAMS(kind0, name0) kind0 name0
#define GMOCK_INTERNAL_DECL_HAS_2_TEMPLATE_PARAMS(kind0, name0, kind1, \
    name1) kind0 name0, kind1 name1
#define GMOCK_INTERNAL_DECL_HAS_3_TEMPLATE_PARAMS(kind0, name0, kind1, name1, \
    kind2, name2) kind0 name0, kind1 name1, kind2 name2
#define GMOCK_INTERNAL_DECL_HAS_4_TEMPLATE_PARAMS(kind0, name0, kind1, name1, \
    kind2, name2, kind3, name3) kind0 name0, kind1 name1, kind2 name2, \
    kind3 name3
#define GMOCK_INTERNAL_DECL_HAS_5_TEMPLATE_PARAMS(kind0, name0, kind1, name1, \
    kind2, name2, kind3, name3, kind4, name4) kind0 name0, kind1 name1, \
    kind2 name2, kind3 name3, kind4 name4
#define GMOCK_INTERNAL_DECL_HAS_6_TEMPLATE_PARAMS(kind0, name0, kind1, name1, \
    kind2, name2, kind3, name3, kind4, name4, kind5, name5) kind0 name0, \
    kind1 name1, kind2 name2, kind3 name3, kind4 name4, kind5 name5
#define GMOCK_INTERNAL_DECL_HAS_7_TEMPLATE_PARAMS(kind0, name0, kind1, name1, \
    kind2, name2, kind3, name3, kind4, name4, kind5, name5, kind6, \
    name6) kind0 name0, kind1 name1, kind2 name2, kind3 name3, kind4 name4, \
    kind5 name5, kind6 name6
#define GMOCK_INTERNAL_DECL_HAS_8_TEMPLATE_PARAMS(kind0, name0, kind1, name1, \
    kind2, name2, kind3, name3, kind4, name4, kind5, name5, kind6, name6, \
    kind7, name7) kind0 name0, kind1 name1, kind2 name2, kind3 name3, \
    kind4 name4, kind5 name5, kind6 name6, kind7 name7
#define GMOCK_INTERNAL_DECL_HAS_9_TEMPLATE_PARAMS(kind0, name0, kind1, name1, \
    kind2, name2, kind3, name3, kind4, name4, kind5, name5, kind6, name6, \
    kind7, name7, kind8, name8) kind0 name0, kind1 name1, kind2 name2, \
    kind3 name3, kind4 name4, kind5 name5, kind6 name6, kind7 name7, \
    kind8 name8
#define GMOCK_INTERNAL_DECL_HAS_10_TEMPLATE_PARAMS(kind0, name0, kind1, \
    name1, kind2, name2, kind3, name3, kind4, name4, kind5, name5, kind6, \
    name6, kind7, name7, kind8, name8, kind9, name9) kind0 name0, \
    kind1 name1, kind2 name2, kind3 name3, kind4 name4, kind5 name5, \
    kind6 name6, kind7 name7, kind8 name8, kind9 name9

// Lists the template parameters.
#define GMOCK_INTERNAL_LIST_HAS_1_TEMPLATE_PARAMS(kind0, name0) name0
#define GMOCK_INTERNAL_LIST_HAS_2_TEMPLATE_PARAMS(kind0, name0, kind1, \
    name1) name0, name1
#define GMOCK_INTERNAL_LIST_HAS_3_TEMPLATE_PARAMS(kind0, name0, kind1, name1, \
    kind2, name2) name0, name1, name2
#define GMOCK_INTERNAL_LIST_HAS_4_TEMPLATE_PARAMS(kind0, name0, kind1, name1, \
    kind2, name2, kind3, name3) name0, name1, name2, name3
#define GMOCK_INTERNAL_LIST_HAS_5_TEMPLATE_PARAMS(kind0, name0, kind1, name1, \
    kind2, name2, kind3, name3, kind4, name4) name0, name1, name2, name3, \
    name4
#define GMOCK_INTERNAL_LIST_HAS_6_TEMPLATE_PARAMS(kind0, name0, kind1, name1, \
    kind2, name2, kind3, name3, kind4, name4, kind5, name5) name0, name1, \
    name2, name3, name4, name5
#define GMOCK_INTERNAL_LIST_HAS_7_TEMPLATE_PARAMS(kind0, name0, kind1, name1, \
    kind2, name2, kind3, name3, kind4, name4, kind5, name5, kind6, \
    name6) name0, name1, name2, name3, name4, name5, name6
#define GMOCK_INTERNAL_LIST_HAS_8_TEMPLATE_PARAMS(kind0, name0, kind1, name1, \
    kind2, name2, kind3, name3, kind4, name4, kind5, name5, kind6, name6, \
    kind7, name7) name0, name1, name2, name3, name4, name5, name6, name7
#define GMOCK_INTERNAL_LIST_HAS_9_TEMPLATE_PARAMS(kind0, name0, kind1, name1, \
    kind2, name2, kind3, name3, kind4, name4, kind5, name5, kind6, name6, \
    kind7, name7, kind8, name8) name0, name1, name2, name3, name4, name5, \
    name6, name7, name8
#define GMOCK_INTERNAL_LIST_HAS_10_TEMPLATE_PARAMS(kind0, name0, kind1, \
    name1, kind2, name2, kind3, name3, kind4, name4, kind5, name5, kind6, \
    name6, kind7, name7, kind8, name8, kind9, name9) name0, name1, name2, \
    name3, name4, name5, name6, name7, name8, name9

// Declares the types of value parameters.
#define GMOCK_INTERNAL_DECL_TYPE_AND_0_VALUE_PARAMS()
#define GMOCK_INTERNAL_DECL_TYPE_AND_1_VALUE_PARAMS(p0) , typename p0##_type
#define GMOCK_INTERNAL_DECL_TYPE_AND_2_VALUE_PARAMS(p0, p1) , \
    typename p0##_type, typename p1##_type
#define GMOCK_INTERNAL_DECL_TYPE_AND_3_VALUE_PARAMS(p0, p1, p2) , \
    typename p0##_type, typename p1##_type, typename p2##_type
#define GMOCK_INTERNAL_DECL_TYPE_AND_4_VALUE_PARAMS(p0, p1, p2, p3) , \
    typename p0##_type, typename p1##_type, typename p2##_type, \
    typename p3##_type
#define GMOCK_INTERNAL_DECL_TYPE_AND_5_VALUE_PARAMS(p0, p1, p2, p3, p4) , \
    typename p0##_type, typename p1##_type, typename p2##_type, \
    typename p3##_type, typename p4##_type
#define GMOCK_INTERNAL_DECL_TYPE_AND_6_VALUE_PARAMS(p0, p1, p2, p3, p4, p5) , \
    typename p0##_type, typename p1##_type, typename p2##_type, \
    typename p3##_type, typename p4##_type, typename p5##_type
#define GMOCK_INTERNAL_DECL_TYPE_AND_7_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, \
    p6) , typename p0##_type, typename p1##_type, typename p2##_type, \
    typename p3##_type, typename p4##_type, typename p5##_type, \
    typename p6##_type
#define GMOCK_INTERNAL_DECL_TYPE_AND_8_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, \
    p6, p7) , typename p0##_type, typename p1##_type, typename p2##_type, \
    typename p3##_type, typename p4##_type, typename p5##_type, \
    typename p6##_type, typename p7##_type
#define GMOCK_INTERNAL_DECL_TYPE_AND_9_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, \
    p6, p7, p8) , typename p0##_type, typename p1##_type, typename p2##_type, \
    typename p3##_type, typename p4##_type, typename p5##_type, \
    typename p6##_type, typename p7##_type, typename p8##_type
#define GMOCK_INTERNAL_DECL_TYPE_AND_10_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, \
    p6, p7, p8, p9) , typename p0##_type, typename p1##_type, \
    typename p2##_type, typename p3##_type, typename p4##_type, \
    typename p5##_type, typename p6##_type, typename p7##_type, \
    typename p8##_type, typename p9##_type

// Initializes the value parameters.
#define GMOCK_INTERNAL_INIT_AND_0_VALUE_PARAMS()\
    ()
#define GMOCK_INTERNAL_INIT_AND_1_VALUE_PARAMS(p0)\
    (p0##_type gmock_p0) : p0(gmock_p0)
#define GMOCK_INTERNAL_INIT_AND_2_VALUE_PARAMS(p0, p1)\
    (p0##_type gmock_p0, p1##_type gmock_p1) : p0(gmock_p0), p1(gmock_p1)
#define GMOCK_INTERNAL_INIT_AND_3_VALUE_PARAMS(p0, p1, p2)\
    (p0##_type gmock_p0, p1##_type gmock_p1, \
        p2##_type gmock_p2) : p0(gmock_p0), p1(gmock_p1), p2(gmock_p2)
#define GMOCK_INTERNAL_INIT_AND_4_VALUE_PARAMS(p0, p1, p2, p3)\
    (p0##_type gmock_p0, p1##_type gmock_p1, p2##_type gmock_p2, \
        p3##_type gmock_p3) : p0(gmock_p0), p1(gmock_p1), p2(gmock_p2), \
        p3(gmock_p3)
#define GMOCK_INTERNAL_INIT_AND_5_VALUE_PARAMS(p0, p1, p2, p3, p4)\
    (p0##_type gmock_p0, p1##_type gmock_p1, p2##_type gmock_p2, \
        p3##_type gmock_p3, p4##_type gmock_p4) : p0(gmock_p0), p1(gmock_p1), \
        p2(gmock_p2), p3(gmock_p3), p4(gmock_p4)
#define GMOCK_INTERNAL_INIT_AND_6_VALUE_PARAMS(p0, p1, p2, p3, p4, p5)\
    (p0##_type gmock_p0, p1##_type gmock_p1, p2##_type gmock_p2, \
        p3##_type gmock_p3, p4##_type gmock_p4, \
        p5##_type gmock_p5) : p0(gmock_p0), p1(gmock_p1), p2(gmock_p2), \
        p3(gmock_p3), p4(gmock_p4), p5(gmock_p5)
#define GMOCK_INTERNAL_INIT_AND_7_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6)\
    (p0##_type gmock_p0, p1##_type gmock_p1, p2##_type gmock_p2, \
        p3##_type gmock_p3, p4##_type gmock_p4, p5##_type gmock_p5, \
        p6##_type gmock_p6) : p0(gmock_p0), p1(gmock_p1), p2(gmock_p2), \
        p3(gmock_p3), p4(gmock_p4), p5(gmock_p5), p6(gmock_p6)
#define GMOCK_INTERNAL_INIT_AND_8_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, p7)\
    (p0##_type gmock_p0, p1##_type gmock_p1, p2##_type gmock_p2, \
        p3##_type gmock_p3, p4##_type gmock_p4, p5##_type gmock_p5, \
        p6##_type gmock_p6, p7##_type gmock_p7) : p0(gmock_p0), p1(gmock_p1), \
        p2(gmock_p2), p3(gmock_p3), p4(gmock_p4), p5(gmock_p5), p6(gmock_p6), \
        p7(gmock_p7)
#define GMOCK_INTERNAL_INIT_AND_9_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, \
    p7, p8)\
    (p0##_type gmock_p0, p1##_type gmock_p1, p2##_type gmock_p2, \
        p3##_type gmock_p3, p4##_type gmock_p4, p5##_type gmock_p5, \
        p6##_type gmock_p6, p7##_type gmock_p7, \
        p8##_type gmock_p8) : p0(gmock_p0), p1(gmock_p1), p2(gmock_p2), \
        p3(gmock_p3), p4(gmock_p4), p5(gmock_p5), p6(gmock_p6), p7(gmock_p7), \
        p8(gmock_p8)
#define GMOCK_INTERNAL_INIT_AND_10_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, \
    p7, p8, p9)\
    (p0##_type gmock_p0, p1##_type gmock_p1, p2##_type gmock_p2, \
        p3##_type gmock_p3, p4##_type gmock_p4, p5##_type gmock_p5, \
        p6##_type gmock_p6, p7##_type gmock_p7, p8##_type gmock_p8, \
        p9##_type gmock_p9) : p0(gmock_p0), p1(gmock_p1), p2(gmock_p2), \
        p3(gmock_p3), p4(gmock_p4), p5(gmock_p5), p6(gmock_p6), p7(gmock_p7), \
        p8(gmock_p8), p9(gmock_p9)

// Declares the fields for storing the value parameters.
#define GMOCK_INTERNAL_DEFN_AND_0_VALUE_PARAMS()
#define GMOCK_INTERNAL_DEFN_AND_1_VALUE_PARAMS(p0) p0##_type p0;
#define GMOCK_INTERNAL_DEFN_AND_2_VALUE_PARAMS(p0, p1) p0##_type p0; \
    p1##_type p1;
#define GMOCK_INTERNAL_DEFN_AND_3_VALUE_PARAMS(p0, p1, p2) p0##_type p0; \
    p1##_type p1; p2##_type p2;
#define GMOCK_INTERNAL_DEFN_AND_4_VALUE_PARAMS(p0, p1, p2, p3) p0##_type p0; \
    p1##_type p1; p2##_type p2; p3##_type p3;
#define GMOCK_INTERNAL_DEFN_AND_5_VALUE_PARAMS(p0, p1, p2, p3, \
    p4) p0##_type p0; p1##_type p1; p2##_type p2; p3##_type p3; p4##_type p4;
#define GMOCK_INTERNAL_DEFN_AND_6_VALUE_PARAMS(p0, p1, p2, p3, p4, \
    p5) p0##_type p0; p1##_type p1; p2##_type p2; p3##_type p3; p4##_type p4; \
    p5##_type p5;
#define GMOCK_INTERNAL_DEFN_AND_7_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, \
    p6) p0##_type p0; p1##_type p1; p2##_type p2; p3##_type p3; p4##_type p4; \
    p5##_type p5; p6##_type p6;
#define GMOCK_INTERNAL_DEFN_AND_8_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, \
    p7) p0##_type p0; p1##_type p1; p2##_type p2; p3##_type p3; p4##_type p4; \
    p5##_type p5; p6##_type p6; p7##_type p7;
#define GMOCK_INTERNAL_DEFN_AND_9_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, \
    p7, p8) p0##_type p0; p1##_type p1; p2##_type p2; p3##_type p3; \
    p4##_type p4; p5##_type p5; p6##_type p6; p7##_type p7; p8##_type p8;
#define GMOCK_INTERNAL_DEFN_AND_10_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, \
    p7, p8, p9) p0##_type p0; p1##_type p1; p2##_type p2; p3##_type p3; \
    p4##_type p4; p5##_type p5; p6##_type p6; p7##_type p7; p8##_type p8; \
    p9##_type p9;

// Lists the value parameters.
#define GMOCK_INTERNAL_LIST_AND_0_VALUE_PARAMS()
#define GMOCK_INTERNAL_LIST_AND_1_VALUE_PARAMS(p0) p0
#define GMOCK_INTERNAL_LIST_AND_2_VALUE_PARAMS(p0, p1) p0, p1
#define GMOCK_INTERNAL_LIST_AND_3_VALUE_PARAMS(p0, p1, p2) p0, p1, p2
#define GMOCK_INTERNAL_LIST_AND_4_VALUE_PARAMS(p0, p1, p2, p3) p0, p1, p2, p3
#define GMOCK_INTERNAL_LIST_AND_5_VALUE_PARAMS(p0, p1, p2, p3, p4) p0, p1, \
    p2, p3, p4
#define GMOCK_INTERNAL_LIST_AND_6_VALUE_PARAMS(p0, p1, p2, p3, p4, p5) p0, \
    p1, p2, p3, p4, p5
#define GMOCK_INTERNAL_LIST_AND_7_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, \
    p6) p0, p1, p2, p3, p4, p5, p6
#define GMOCK_INTERNAL_LIST_AND_8_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, \
    p7) p0, p1, p2, p3, p4, p5, p6, p7
#define GMOCK_INTERNAL_LIST_AND_9_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, \
    p7, p8) p0, p1, p2, p3, p4, p5, p6, p7, p8
#define GMOCK_INTERNAL_LIST_AND_10_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, \
    p7, p8, p9) p0, p1, p2, p3, p4, p5, p6, p7, p8, p9

// Lists the value parameter types.
#define GMOCK_INTERNAL_LIST_TYPE_AND_0_VALUE_PARAMS()
#define GMOCK_INTERNAL_LIST_TYPE_AND_1_VALUE_PARAMS(p0) , p0##_type
#define GMOCK_INTERNAL_LIST_TYPE_AND_2_VALUE_PARAMS(p0, p1) , p0##_type, \
    p1##_type
#define GMOCK_INTERNAL_LIST_TYPE_AND_3_VALUE_PARAMS(p0, p1, p2) , p0##_type, \
    p1##_type, p2##_type
#define GMOCK_INTERNAL_LIST_TYPE_AND_4_VALUE_PARAMS(p0, p1, p2, p3) , \
    p0##_type, p1##_type, p2##_type, p3##_type
#define GMOCK_INTERNAL_LIST_TYPE_AND_5_VALUE_PARAMS(p0, p1, p2, p3, p4) , \
    p0##_type, p1##_type, p2##_type, p3##_type, p4##_type
#define GMOCK_INTERNAL_LIST_TYPE_AND_6_VALUE_PARAMS(p0, p1, p2, p3, p4, p5) , \
    p0##_type, p1##_type, p2##_type, p3##_type, p4##_type, p5##_type
#define GMOCK_INTERNAL_LIST_TYPE_AND_7_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, \
    p6) , p0##_type, p1##_type, p2##_type, p3##_type, p4##_type, p5##_type, \
    p6##_type
#define GMOCK_INTERNAL_LIST_TYPE_AND_8_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, \
    p6, p7) , p0##_type, p1##_type, p2##_type, p3##_type, p4##_type, \
    p5##_type, p6##_type, p7##_type
#define GMOCK_INTERNAL_LIST_TYPE_AND_9_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, \
    p6, p7, p8) , p0##_type, p1##_type, p2##_type, p3##_type, p4##_type, \
    p5##_type, p6##_type, p7##_type, p8##_type
#define GMOCK_INTERNAL_LIST_TYPE_AND_10_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, \
    p6, p7, p8, p9) , p0##_type, p1##_type, p2##_type, p3##_type, p4##_type, \
    p5##_type, p6##_type, p7##_type, p8##_type, p9##_type

// Declares the value parameters.
#define GMOCK_INTERNAL_DECL_AND_0_VALUE_PARAMS()
#define GMOCK_INTERNAL_DECL_AND_1_VALUE_PARAMS(p0) p0##_type p0
#define GMOCK_INTERNAL_DECL_AND_2_VALUE_PARAMS(p0, p1) p0##_type p0, \
    p1##_type p1
#define GMOCK_INTERNAL_DECL_AND_3_VALUE_PARAMS(p0, p1, p2) p0##_type p0, \
    p1##_type p1, p2##_type p2
#define GMOCK_INTERNAL_DECL_AND_4_VALUE_PARAMS(p0, p1, p2, p3) p0##_type p0, \
    p1##_type p1, p2##_type p2, p3##_type p3
#define GMOCK_INTERNAL_DECL_AND_5_VALUE_PARAMS(p0, p1, p2, p3, \
    p4) p0##_type p0, p1##_type p1, p2##_type p2, p3##_type p3, p4##_type p4
#define GMOCK_INTERNAL_DECL_AND_6_VALUE_PARAMS(p0, p1, p2, p3, p4, \
    p5) p0##_type p0, p1##_type p1, p2##_type p2, p3##_type p3, p4##_type p4, \
    p5##_type p5
#define GMOCK_INTERNAL_DECL_AND_7_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, \
    p6) p0##_type p0, p1##_type p1, p2##_type p2, p3##_type p3, p4##_type p4, \
    p5##_type p5, p6##_type p6
#define GMOCK_INTERNAL_DECL_AND_8_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, \
    p7) p0##_type p0, p1##_type p1, p2##_type p2, p3##_type p3, p4##_type p4, \
    p5##_type p5, p6##_type p6, p7##_type p7
#define GMOCK_INTERNAL_DECL_AND_9_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, \
    p7, p8) p0##_type p0, p1##_type p1, p2##_type p2, p3##_type p3, \
    p4##_type p4, p5##_type p5, p6##_type p6, p7##_type p7, p8##_type p8
#define GMOCK_INTERNAL_DECL_AND_10_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, \
    p7, p8, p9) p0##_type p0, p1##_type p1, p2##_type p2, p3##_type p3, \
    p4##_type p4, p5##_type p5, p6##_type p6, p7##_type p7, p8##_type p8, \
    p9##_type p9

// The suffix of the class template implementing the action template.
#define GMOCK_INTERNAL_COUNT_AND_0_VALUE_PARAMS()
#define GMOCK_INTERNAL_COUNT_AND_1_VALUE_PARAMS(p0) P
#define GMOCK_INTERNAL_COUNT_AND_2_VALUE_PARAMS(p0, p1) P2
#define GMOCK_INTERNAL_COUNT_AND_3_VALUE_PARAMS(p0, p1, p2) P3
#define GMOCK_INTERNAL_COUNT_AND_4_VALUE_PARAMS(p0, p1, p2, p3) P4
#define GMOCK_INTERNAL_COUNT_AND_5_VALUE_PARAMS(p0, p1, p2, p3, p4) P5
#define GMOCK_INTERNAL_COUNT_AND_6_VALUE_PARAMS(p0, p1, p2, p3, p4, p5) P6
#define GMOCK_INTERNAL_COUNT_AND_7_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6) P7
#define GMOCK_INTERNAL_COUNT_AND_8_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, \
    p7) P8
#define GMOCK_INTERNAL_COUNT_AND_9_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, \
    p7, p8) P9
#define GMOCK_INTERNAL_COUNT_AND_10_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, \
    p7, p8, p9) P10

// The name of the class template implementing the action template.
#define GMOCK_ACTION_CLASS_(name, value_params)\
    GMOCK_CONCAT_TOKEN_(name##Action, GMOCK_INTERNAL_COUNT_##value_params)

#define ACTION_TEMPLATE(name, template_params, value_params)\
  template <GMOCK_INTERNAL_DECL_##template_params\
            GMOCK_INTERNAL_DECL_TYPE_##value_params>\
  class GMOCK_ACTION_CLASS_(name, value_params) {\
   public:\
    GMOCK_ACTION_CLASS_(name, value_params)\
        GMOCK_INTERNAL_INIT_##value_params {}\
    template <typename F>\
    class gmock_Impl : public ::testing::ActionInterface<F> {\
     public:\
      typedef F function_type;\
      typedef typename ::testing::internal::Function<F>::Result return_type;\
      typedef typename ::testing::internal::Function<F>::ArgumentTuple\
          args_type;\
      explicit gmock_Impl GMOCK_INTERNAL_INIT_##value_params {}\
      virtual return_type Perform(const args_type& args) {\
        return ::testing::internal::ActionHelper<return_type, gmock_Impl>::\
            Perform(this, args);\
      }\
      template <typename arg0_type, typename arg1_type, typename arg2_type, \
          typename arg3_type, typename arg4_type, typename arg5_type, \
          typename arg6_type, typename arg7_type, typename arg8_type, \
          typename arg9_type>\
      return_type gmock_PerformImpl(const args_type& args, arg0_type arg0, \
          arg1_type arg1, arg2_type arg2, arg3_type arg3, arg4_type arg4, \
          arg5_type arg5, arg6_type arg6, arg7_type arg7, arg8_type arg8, \
          arg9_type arg9) const;\
      GMOCK_INTERNAL_DEFN_##value_params\
     private:\
      GTEST_DISALLOW_ASSIGN_(gmock_Impl);\
    };\
    template <typename F> operator ::testing::Action<F>() const {\
      return ::testing::Action<F>(\
          new gmock_Impl<F>(GMOCK_INTERNAL_LIST_##value_params));\
    }\
    GMOCK_INTERNAL_DEFN_##value_params\
   private:\
    GTEST_DISALLOW_ASSIGN_(GMOCK_ACTION_CLASS_(name, value_params));\
  };\
  template <GMOCK_INTERNAL_DECL_##template_params\
            GMOCK_INTERNAL_DECL_TYPE_##value_params>\
  inline GMOCK_ACTION_CLASS_(name, value_params)<\
      GMOCK_INTERNAL_LIST_##template_params\
      GMOCK_INTERNAL_LIST_TYPE_##value_params> name(\
          GMOCK_INTERNAL_DECL_##value_params) {\
    return GMOCK_ACTION_CLASS_(name, value_params)<\
        GMOCK_INTERNAL_LIST_##template_params\
        GMOCK_INTERNAL_LIST_TYPE_##value_params>(\
            GMOCK_INTERNAL_LIST_##value_params);\
  }\
  template <GMOCK_INTERNAL_DECL_##template_params\
            GMOCK_INTERNAL_DECL_TYPE_##value_params>\
  template <typename F>\
  template <typename arg0_type, typename arg1_type, typename arg2_type,\
      typename arg3_type, typename arg4_type, typename arg5_type,\
      typename arg6_type, typename arg7_type, typename arg8_type,\
      typename arg9_type>\
  typename ::testing::internal::Function<F>::Result\
      GMOCK_ACTION_CLASS_(name, value_params)<\
          GMOCK_INTERNAL_LIST_##template_params\
          GMOCK_INTERNAL_LIST_TYPE_##value_params>::gmock_Impl<F>::\
              gmock_PerformImpl(\
          GMOCK_ACTION_ARG_TYPES_AND_NAMES_UNUSED_) const

#define ACTION(name)\
  class name##Action {\
   public:\
    name##Action() {}\
    template <typename F>\
    class gmock_Impl : public ::testing::ActionInterface<F> {\
     public:\
      typedef F function_type;\
      typedef typename ::testing::internal::Function<F>::Result return_type;\
      typedef typename ::testing::internal::Function<F>::ArgumentTuple\
          args_type;\
      gmock_Impl() {}\
      virtual return_type Perform(const args_type& args) {\
        return ::testing::internal::ActionHelper<return_type, gmock_Impl>::\
            Perform(this, args);\
      }\
      template <typename arg0_type, typename arg1_type, typename arg2_type, \
          typename arg3_type, typename arg4_type, typename arg5_type, \
          typename arg6_type, typename arg7_type, typename arg8_type, \
          typename arg9_type>\
      return_type gmock_PerformImpl(const args_type& args, arg0_type arg0, \
          arg1_type arg1, arg2_type arg2, arg3_type arg3, arg4_type arg4, \
          arg5_type arg5, arg6_type arg6, arg7_type arg7, arg8_type arg8, \
          arg9_type arg9) const;\
     private:\
      GTEST_DISALLOW_ASSIGN_(gmock_Impl);\
    };\
    template <typename F> operator ::testing::Action<F>() const {\
      return ::testing::Action<F>(new gmock_Impl<F>());\
    }\
   private:\
    GTEST_DISALLOW_ASSIGN_(name##Action);\
  };\
  inline name##Action name() {\
    return name##Action();\
  }\
  template <typename F>\
  template <typename arg0_type, typename arg1_type, typename arg2_type, \
      typename arg3_type, typename arg4_type, typename arg5_type, \
      typename arg6_type, typename arg7_type, typename arg8_type, \
      typename arg9_type>\
  typename ::testing::internal::Function<F>::Result\
      name##Action::gmock_Impl<F>::gmock_PerformImpl(\
          GMOCK_ACTION_ARG_TYPES_AND_NAMES_UNUSED_) const

#define ACTION_P(name, p0)\
  template <typename p0##_type>\
  class name##ActionP {\
   public:\
    name##ActionP(p0##_type gmock_p0) : p0(gmock_p0) {}\
    template <typename F>\
    class gmock_Impl : public ::testing::ActionInterface<F> {\
     public:\
      typedef F function_type;\
      typedef typename ::testing::internal::Function<F>::Result return_type;\
      typedef typename ::testing::internal::Function<F>::ArgumentTuple\
          args_type;\
      explicit gmock_Impl(p0##_type gmock_p0) : p0(gmock_p0) {}\
      virtual return_type Perform(const args_type& args) {\
        return ::testing::internal::ActionHelper<return_type, gmock_Impl>::\
            Perform(this, args);\
      }\
      template <typename arg0_type, typename arg1_type, typename arg2_type, \
          typename arg3_type, typename arg4_type, typename arg5_type, \
          typename arg6_type, typename arg7_type, typename arg8_type, \
          typename arg9_type>\
      return_type gmock_PerformImpl(const args_type& args, arg0_type arg0, \
          arg1_type arg1, arg2_type arg2, arg3_type arg3, arg4_type arg4, \
          arg5_type arg5, arg6_type arg6, arg7_type arg7, arg8_type arg8, \
          arg9_type arg9) const;\
      p0##_type p0;\
     private:\
      GTEST_DISALLOW_ASSIGN_(gmock_Impl);\
    };\
    template <typename F> operator ::testing::Action<F>() const {\
      return ::testing::Action<F>(new gmock_Impl<F>(p0));\
    }\
    p0##_type p0;\
   private:\
    GTEST_DISALLOW_ASSIGN_(name##ActionP);\
  };\
  template <typename p0##_type>\
  inline name##ActionP<p0##_type> name(p0##_type p0) {\
    return name##ActionP<p0##_type>(p0);\
  }\
  template <typename p0##_type>\
  template <typename F>\
  template <typename arg0_type, typename arg1_type, typename arg2_type, \
      typename arg3_type, typename arg4_type, typename arg5_type, \
      typename arg6_type, typename arg7_type, typename arg8_type, \
      typename arg9_type>\
  typename ::testing::internal::Function<F>::Result\
      name##ActionP<p0##_type>::gmock_Impl<F>::gmock_PerformImpl(\
          GMOCK_ACTION_ARG_TYPES_AND_NAMES_UNUSED_) const

#define ACTION_P2(name, p0, p1)\
  template <typename p0##_type, typename p1##_type>\
  class name##ActionP2 {\
   public:\
    name##ActionP2(p0##_type gmock_p0, p1##_type gmock_p1) : p0(gmock_p0), \
        p1(gmock_p1) {}\
    template <typename F>\
    class gmock_Impl : public ::testing::ActionInterface<F> {\
     public:\
      typedef F function_type;\
      typedef typename ::testing::internal::Function<F>::Result return_type;\
      typedef typename ::testing::internal::Function<F>::ArgumentTuple\
          args_type;\
      gmock_Impl(p0##_type gmock_p0, p1##_type gmock_p1) : p0(gmock_p0), \
          p1(gmock_p1) {}\
      virtual return_type Perform(const args_type& args) {\
        return ::testing::internal::ActionHelper<return_type, gmock_Impl>::\
            Perform(this, args);\
      }\
      template <typename arg0_type, typename arg1_type, typename arg2_type, \
          typename arg3_type, typename arg4_type, typename arg5_type, \
          typename arg6_type, typename arg7_type, typename arg8_type, \
          typename arg9_type>\
      return_type gmock_PerformImpl(const args_type& args, arg0_type arg0, \
          arg1_type arg1, arg2_type arg2, arg3_type arg3, arg4_type arg4, \
          arg5_type arg5, arg6_type arg6, arg7_type arg7, arg8_type arg8, \
          arg9_type arg9) const;\
      p0##_type p0;\
      p1##_type p1;\
     private:\
      GTEST_DISALLOW_ASSIGN_(gmock_Impl);\
    };\
    template <typename F> operator ::testing::Action<F>() const {\
      return ::testing::Action<F>(new gmock_Impl<F>(p0, p1));\
    }\
    p0##_type p0;\
    p1##_type p1;\
   private:\
    GTEST_DISALLOW_ASSIGN_(name##ActionP2);\
  };\
  template <typename p0##_type, typename p1##_type>\
  inline name##ActionP2<p0##_type, p1##_type> name(p0##_type p0, \
      p1##_type p1) {\
    return name##ActionP2<p0##_type, p1##_type>(p0, p1);\
  }\
  template <typename p0##_type, typename p1##_type>\
  template <typename F>\
  template <typename arg0_type, typename arg1_type, typename arg2_type, \
      typename arg3_type, typename arg4_type, typename arg5_type, \
      typename arg6_type, typename arg7_type, typename arg8_type, \
      typename arg9_type>\
  typename ::testing::internal::Function<F>::Result\
      name##ActionP2<p0##_type, p1##_type>::gmock_Impl<F>::gmock_PerformImpl(\
          GMOCK_ACTION_ARG_TYPES_AND_NAMES_UNUSED_) const

#define ACTION_P3(name, p0, p1, p2)\
  template <typename p0##_type, typename p1##_type, typename p2##_type>\
  class name##ActionP3 {\
   public:\
    name##ActionP3(p0##_type gmock_p0, p1##_type gmock_p1, \
        p2##_type gmock_p2) : p0(gmock_p0), p1(gmock_p1), p2(gmock_p2) {}\
    template <typename F>\
    class gmock_Impl : public ::testing::ActionInterface<F> {\
     public:\
      typedef F function_type;\
      typedef typename ::testing::internal::Function<F>::Result return_type;\
      typedef typename ::testing::internal::Function<F>::ArgumentTuple\
          args_type;\
      gmock_Impl(p0##_type gmock_p0, p1##_type gmock_p1, \
          p2##_type gmock_p2) : p0(gmock_p0), p1(gmock_p1), p2(gmock_p2) {}\
      virtual return_type Perform(const args_type& args) {\
        return ::testing::internal::ActionHelper<return_type, gmock_Impl>::\
            Perform(this, args);\
      }\
      template <typename arg0_type, typename arg1_type, typename arg2_type, \
          typename arg3_type, typename arg4_type, typename arg5_type, \
          typename arg6_type, typename arg7_type, typename arg8_type, \
          typename arg9_type>\
      return_type gmock_PerformImpl(const args_type& args, arg0_type arg0, \
          arg1_type arg1, arg2_type arg2, arg3_type arg3, arg4_type arg4, \
          arg5_type arg5, arg6_type arg6, arg7_type arg7, arg8_type arg8, \
          arg9_type arg9) const;\
      p0##_type p0;\
      p1##_type p1;\
      p2##_type p2;\
     private:\
      GTEST_DISALLOW_ASSIGN_(gmock_Impl);\
    };\
    template <typename F> operator ::testing::Action<F>() const {\
      return ::testing::Action<F>(new gmock_Impl<F>(p0, p1, p2));\
    }\
    p0##_type p0;\
    p1##_type p1;\
    p2##_type p2;\
   private:\
    GTEST_DISALLOW_ASSIGN_(name##ActionP3);\
  };\
  template <typename p0##_type, typename p1##_type, typename p2##_type>\
  inline name##ActionP3<p0##_type, p1##_type, p2##_type> name(p0##_type p0, \
      p1##_type p1, p2##_type p2) {\
    return name##ActionP3<p0##_type, p1##_type, p2##_type>(p0, p1, p2);\
  }\
  template <typename p0##_type, typename p1##_type, typename p2##_type>\
  template <typename F>\
  template <typename arg0_type, typename arg1_type, typename arg2_type, \
      typename arg3_type, typename arg4_type, typename arg5_type, \
      typename arg6_type, typename arg7_type, typename arg8_type, \
      typename arg9_type>\
  typename ::testing::internal::Function<F>::Result\
      name##ActionP3<p0##_type, p1##_type, \
          p2##_type>::gmock_Impl<F>::gmock_PerformImpl(\
          GMOCK_ACTION_ARG_TYPES_AND_NAMES_UNUSED_) const

#define ACTION_P4(name, p0, p1, p2, p3)\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type>\
  class name##ActionP4 {\
   public:\
    name##ActionP4(p0##_type gmock_p0, p1##_type gmock_p1, \
        p2##_type gmock_p2, p3##_type gmock_p3) : p0(gmock_p0), p1(gmock_p1), \
        p2(gmock_p2), p3(gmock_p3) {}\
    template <typename F>\
    class gmock_Impl : public ::testing::ActionInterface<F> {\
     public:\
      typedef F function_type;\
      typedef typename ::testing::internal::Function<F>::Result return_type;\
      typedef typename ::testing::internal::Function<F>::ArgumentTuple\
          args_type;\
      gmock_Impl(p0##_type gmock_p0, p1##_type gmock_p1, p2##_type gmock_p2, \
          p3##_type gmock_p3) : p0(gmock_p0), p1(gmock_p1), p2(gmock_p2), \
          p3(gmock_p3) {}\
      virtual return_type Perform(const args_type& args) {\
        return ::testing::internal::ActionHelper<return_type, gmock_Impl>::\
            Perform(this, args);\
      }\
      template <typename arg0_type, typename arg1_type, typename arg2_type, \
          typename arg3_type, typename arg4_type, typename arg5_type, \
          typename arg6_type, typename arg7_type, typename arg8_type, \
          typename arg9_type>\
      return_type gmock_PerformImpl(const args_type& args, arg0_type arg0, \
          arg1_type arg1, arg2_type arg2, arg3_type arg3, arg4_type arg4, \
          arg5_type arg5, arg6_type arg6, arg7_type arg7, arg8_type arg8, \
          arg9_type arg9) const;\
      p0##_type p0;\
      p1##_type p1;\
      p2##_type p2;\
      p3##_type p3;\
     private:\
      GTEST_DISALLOW_ASSIGN_(gmock_Impl);\
    };\
    template <typename F> operator ::testing::Action<F>() const {\
      return ::testing::Action<F>(new gmock_Impl<F>(p0, p1, p2, p3));\
    }\
    p0##_type p0;\
    p1##_type p1;\
    p2##_type p2;\
    p3##_type p3;\
   private:\
    GTEST_DISALLOW_ASSIGN_(name##ActionP4);\
  };\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type>\
  inline name##ActionP4<p0##_type, p1##_type, p2##_type, \
      p3##_type> name(p0##_type p0, p1##_type p1, p2##_type p2, \
      p3##_type p3) {\
    return name##ActionP4<p0##_type, p1##_type, p2##_type, p3##_type>(p0, p1, \
        p2, p3);\
  }\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type>\
  template <typename F>\
  template <typename arg0_type, typename arg1_type, typename arg2_type, \
      typename arg3_type, typename arg4_type, typename arg5_type, \
      typename arg6_type, typename arg7_type, typename arg8_type, \
      typename arg9_type>\
  typename ::testing::internal::Function<F>::Result\
      name##ActionP4<p0##_type, p1##_type, p2##_type, \
          p3##_type>::gmock_Impl<F>::gmock_PerformImpl(\
          GMOCK_ACTION_ARG_TYPES_AND_NAMES_UNUSED_) const

#define ACTION_P5(name, p0, p1, p2, p3, p4)\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type>\
  class name##ActionP5 {\
   public:\
    name##ActionP5(p0##_type gmock_p0, p1##_type gmock_p1, \
        p2##_type gmock_p2, p3##_type gmock_p3, \
        p4##_type gmock_p4) : p0(gmock_p0), p1(gmock_p1), p2(gmock_p2), \
        p3(gmock_p3), p4(gmock_p4) {}\
    template <typename F>\
    class gmock_Impl : public ::testing::ActionInterface<F> {\
     public:\
      typedef F function_type;\
      typedef typename ::testing::internal::Function<F>::Result return_type;\
      typedef typename ::testing::internal::Function<F>::ArgumentTuple\
          args_type;\
      gmock_Impl(p0##_type gmock_p0, p1##_type gmock_p1, p2##_type gmock_p2, \
          p3##_type gmock_p3, p4##_type gmock_p4) : p0(gmock_p0), \
          p1(gmock_p1), p2(gmock_p2), p3(gmock_p3), p4(gmock_p4) {}\
      virtual return_type Perform(const args_type& args) {\
        return ::testing::internal::ActionHelper<return_type, gmock_Impl>::\
            Perform(this, args);\
      }\
      template <typename arg0_type, typename arg1_type, typename arg2_type, \
          typename arg3_type, typename arg4_type, typename arg5_type, \
          typename arg6_type, typename arg7_type, typename arg8_type, \
          typename arg9_type>\
      return_type gmock_PerformImpl(const args_type& args, arg0_type arg0, \
          arg1_type arg1, arg2_type arg2, arg3_type arg3, arg4_type arg4, \
          arg5_type arg5, arg6_type arg6, arg7_type arg7, arg8_type arg8, \
          arg9_type arg9) const;\
      p0##_type p0;\
      p1##_type p1;\
      p2##_type p2;\
      p3##_type p3;\
      p4##_type p4;\
     private:\
      GTEST_DISALLOW_ASSIGN_(gmock_Impl);\
    };\
    template <typename F> operator ::testing::Action<F>() const {\
      return ::testing::Action<F>(new gmock_Impl<F>(p0, p1, p2, p3, p4));\
    }\
    p0##_type p0;\
    p1##_type p1;\
    p2##_type p2;\
    p3##_type p3;\
    p4##_type p4;\
   private:\
    GTEST_DISALLOW_ASSIGN_(name##ActionP5);\
  };\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type>\
  inline name##ActionP5<p0##_type, p1##_type, p2##_type, p3##_type, \
      p4##_type> name(p0##_type p0, p1##_type p1, p2##_type p2, p3##_type p3, \
      p4##_type p4) {\
    return name##ActionP5<p0##_type, p1##_type, p2##_type, p3##_type, \
        p4##_type>(p0, p1, p2, p3, p4);\
  }\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type>\
  template <typename F>\
  template <typename arg0_type, typename arg1_type, typename arg2_type, \
      typename arg3_type, typename arg4_type, typename arg5_type, \
      typename arg6_type, typename arg7_type, typename arg8_type, \
      typename arg9_type>\
  typename ::testing::internal::Function<F>::Result\
      name##ActionP5<p0##_type, p1##_type, p2##_type, p3##_type, \
          p4##_type>::gmock_Impl<F>::gmock_PerformImpl(\
          GMOCK_ACTION_ARG_TYPES_AND_NAMES_UNUSED_) const

#define ACTION_P6(name, p0, p1, p2, p3, p4, p5)\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type, typename p5##_type>\
  class name##ActionP6 {\
   public:\
    name##ActionP6(p0##_type gmock_p0, p1##_type gmock_p1, \
        p2##_type gmock_p2, p3##_type gmock_p3, p4##_type gmock_p4, \
        p5##_type gmock_p5) : p0(gmock_p0), p1(gmock_p1), p2(gmock_p2), \
        p3(gmock_p3), p4(gmock_p4), p5(gmock_p5) {}\
    template <typename F>\
    class gmock_Impl : public ::testing::ActionInterface<F> {\
     public:\
      typedef F function_type;\
      typedef typename ::testing::internal::Function<F>::Result return_type;\
      typedef typename ::testing::internal::Function<F>::ArgumentTuple\
          args_type;\
      gmock_Impl(p0##_type gmock_p0, p1##_type gmock_p1, p2##_type gmock_p2, \
          p3##_type gmock_p3, p4##_type gmock_p4, \
          p5##_type gmock_p5) : p0(gmock_p0), p1(gmock_p1), p2(gmock_p2), \
          p3(gmock_p3), p4(gmock_p4), p5(gmock_p5) {}\
      virtual return_type Perform(const args_type& args) {\
        return ::testing::internal::ActionHelper<return_type, gmock_Impl>::\
            Perform(this, args);\
      }\
      template <typename arg0_type, typename arg1_type, typename arg2_type, \
          typename arg3_type, typename arg4_type, typename arg5_type, \
          typename arg6_type, typename arg7_type, typename arg8_type, \
          typename arg9_type>\
      return_type gmock_PerformImpl(const args_type& args, arg0_type arg0, \
          arg1_type arg1, arg2_type arg2, arg3_type arg3, arg4_type arg4, \
          arg5_type arg5, arg6_type arg6, arg7_type arg7, arg8_type arg8, \
          arg9_type arg9) const;\
      p0##_type p0;\
      p1##_type p1;\
      p2##_type p2;\
      p3##_type p3;\
      p4##_type p4;\
      p5##_type p5;\
     private:\
      GTEST_DISALLOW_ASSIGN_(gmock_Impl);\
    };\
    template <typename F> operator ::testing::Action<F>() const {\
      return ::testing::Action<F>(new gmock_Impl<F>(p0, p1, p2, p3, p4, p5));\
    }\
    p0##_type p0;\
    p1##_type p1;\
    p2##_type p2;\
    p3##_type p3;\
    p4##_type p4;\
    p5##_type p5;\
   private:\
    GTEST_DISALLOW_ASSIGN_(name##ActionP6);\
  };\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type, typename p5##_type>\
  inline name##ActionP6<p0##_type, p1##_type, p2##_type, p3##_type, \
      p4##_type, p5##_type> name(p0##_type p0, p1##_type p1, p2##_type p2, \
      p3##_type p3, p4##_type p4, p5##_type p5) {\
    return name##ActionP6<p0##_type, p1##_type, p2##_type, p3##_type, \
        p4##_type, p5##_type>(p0, p1, p2, p3, p4, p5);\
  }\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type, typename p5##_type>\
  template <typename F>\
  template <typename arg0_type, typename arg1_type, typename arg2_type, \
      typename arg3_type, typename arg4_type, typename arg5_type, \
      typename arg6_type, typename arg7_type, typename arg8_type, \
      typename arg9_type>\
  typename ::testing::internal::Function<F>::Result\
      name##ActionP6<p0##_type, p1##_type, p2##_type, p3##_type, p4##_type, \
          p5##_type>::gmock_Impl<F>::gmock_PerformImpl(\
          GMOCK_ACTION_ARG_TYPES_AND_NAMES_UNUSED_) const

#define ACTION_P7(name, p0, p1, p2, p3, p4, p5, p6)\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type, typename p5##_type, \
      typename p6##_type>\
  class name##ActionP7 {\
   public:\
    name##ActionP7(p0##_type gmock_p0, p1##_type gmock_p1, \
        p2##_type gmock_p2, p3##_type gmock_p3, p4##_type gmock_p4, \
        p5##_type gmock_p5, p6##_type gmock_p6) : p0(gmock_p0), p1(gmock_p1), \
        p2(gmock_p2), p3(gmock_p3), p4(gmock_p4), p5(gmock_p5), \
        p6(gmock_p6) {}\
    template <typename F>\
    class gmock_Impl : public ::testing::ActionInterface<F> {\
     public:\
      typedef F function_type;\
      typedef typename ::testing::internal::Function<F>::Result return_type;\
      typedef typename ::testing::internal::Function<F>::ArgumentTuple\
          args_type;\
      gmock_Impl(p0##_type gmock_p0, p1##_type gmock_p1, p2##_type gmock_p2, \
          p3##_type gmock_p3, p4##_type gmock_p4, p5##_type gmock_p5, \
          p6##_type gmock_p6) : p0(gmock_p0), p1(gmock_p1), p2(gmock_p2), \
          p3(gmock_p3), p4(gmock_p4), p5(gmock_p5), p6(gmock_p6) {}\
      virtual return_type Perform(const args_type& args) {\
        return ::testing::internal::ActionHelper<return_type, gmock_Impl>::\
            Perform(this, args);\
      }\
      template <typename arg0_type, typename arg1_type, typename arg2_type, \
          typename arg3_type, typename arg4_type, typename arg5_type, \
          typename arg6_type, typename arg7_type, typename arg8_type, \
          typename arg9_type>\
      return_type gmock_PerformImpl(const args_type& args, arg0_type arg0, \
          arg1_type arg1, arg2_type arg2, arg3_type arg3, arg4_type arg4, \
          arg5_type arg5, arg6_type arg6, arg7_type arg7, arg8_type arg8, \
          arg9_type arg9) const;\
      p0##_type p0;\
      p1##_type p1;\
      p2##_type p2;\
      p3##_type p3;\
      p4##_type p4;\
      p5##_type p5;\
      p6##_type p6;\
     private:\
      GTEST_DISALLOW_ASSIGN_(gmock_Impl);\
    };\
    template <typename F> operator ::testing::Action<F>() const {\
      return ::testing::Action<F>(new gmock_Impl<F>(p0, p1, p2, p3, p4, p5, \
          p6));\
    }\
    p0##_type p0;\
    p1##_type p1;\
    p2##_type p2;\
    p3##_type p3;\
    p4##_type p4;\
    p5##_type p5;\
    p6##_type p6;\
   private:\
    GTEST_DISALLOW_ASSIGN_(name##ActionP7);\
  };\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type, typename p5##_type, \
      typename p6##_type>\
  inline name##ActionP7<p0##_type, p1##_type, p2##_type, p3##_type, \
      p4##_type, p5##_type, p6##_type> name(p0##_type p0, p1##_type p1, \
      p2##_type p2, p3##_type p3, p4##_type p4, p5##_type p5, \
      p6##_type p6) {\
    return name##ActionP7<p0##_type, p1##_type, p2##_type, p3##_type, \
        p4##_type, p5##_type, p6##_type>(p0, p1, p2, p3, p4, p5, p6);\
  }\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type, typename p5##_type, \
      typename p6##_type>\
  template <typename F>\
  template <typename arg0_type, typename arg1_type, typename arg2_type, \
      typename arg3_type, typename arg4_type, typename arg5_type, \
      typename arg6_type, typename arg7_type, typename arg8_type, \
      typename arg9_type>\
  typename ::testing::internal::Function<F>::Result\
      name##ActionP7<p0##_type, p1##_type, p2##_type, p3##_type, p4##_type, \
          p5##_type, p6##_type>::gmock_Impl<F>::gmock_PerformImpl(\
          GMOCK_ACTION_ARG_TYPES_AND_NAMES_UNUSED_) const

#define ACTION_P8(name, p0, p1, p2, p3, p4, p5, p6, p7)\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type, typename p5##_type, \
      typename p6##_type, typename p7##_type>\
  class name##ActionP8 {\
   public:\
    name##ActionP8(p0##_type gmock_p0, p1##_type gmock_p1, \
        p2##_type gmock_p2, p3##_type gmock_p3, p4##_type gmock_p4, \
        p5##_type gmock_p5, p6##_type gmock_p6, \
        p7##_type gmock_p7) : p0(gmock_p0), p1(gmock_p1), p2(gmock_p2), \
        p3(gmock_p3), p4(gmock_p4), p5(gmock_p5), p6(gmock_p6), \
        p7(gmock_p7) {}\
    template <typename F>\
    class gmock_Impl : public ::testing::ActionInterface<F> {\
     public:\
      typedef F function_type;\
      typedef typename ::testing::internal::Function<F>::Result return_type;\
      typedef typename ::testing::internal::Function<F>::ArgumentTuple\
          args_type;\
      gmock_Impl(p0##_type gmock_p0, p1##_type gmock_p1, p2##_type gmock_p2, \
          p3##_type gmock_p3, p4##_type gmock_p4, p5##_type gmock_p5, \
          p6##_type gmock_p6, p7##_type gmock_p7) : p0(gmock_p0), \
          p1(gmock_p1), p2(gmock_p2), p3(gmock_p3), p4(gmock_p4), \
          p5(gmock_p5), p6(gmock_p6), p7(gmock_p7) {}\
      virtual return_type Perform(const args_type& args) {\
        return ::testing::internal::ActionHelper<return_type, gmock_Impl>::\
            Perform(this, args);\
      }\
      template <typename arg0_type, typename arg1_type, typename arg2_type, \
          typename arg3_type, typename arg4_type, typename arg5_type, \
          typename arg6_type, typename arg7_type, typename arg8_type, \
          typename arg9_type>\
      return_type gmock_PerformImpl(const args_type& args, arg0_type arg0, \
          arg1_type arg1, arg2_type arg2, arg3_type arg3, arg4_type arg4, \
          arg5_type arg5, arg6_type arg6, arg7_type arg7, arg8_type arg8, \
          arg9_type arg9) const;\
      p0##_type p0;\
      p1##_type p1;\
      p2##_type p2;\
      p3##_type p3;\
      p4##_type p4;\
      p5##_type p5;\
      p6##_type p6;\
      p7##_type p7;\
     private:\
      GTEST_DISALLOW_ASSIGN_(gmock_Impl);\
    };\
    template <typename F> operator ::testing::Action<F>() const {\
      return ::testing::Action<F>(new gmock_Impl<F>(p0, p1, p2, p3, p4, p5, \
          p6, p7));\
    }\
    p0##_type p0;\
    p1##_type p1;\
    p2##_type p2;\
    p3##_type p3;\
    p4##_type p4;\
    p5##_type p5;\
    p6##_type p6;\
    p7##_type p7;\
   private:\
    GTEST_DISALLOW_ASSIGN_(name##ActionP8);\
  };\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type, typename p5##_type, \
      typename p6##_type, typename p7##_type>\
  inline name##ActionP8<p0##_type, p1##_type, p2##_type, p3##_type, \
      p4##_type, p5##_type, p6##_type, p7##_type> name(p0##_type p0, \
      p1##_type p1, p2##_type p2, p3##_type p3, p4##_type p4, p5##_type p5, \
      p6##_type p6, p7##_type p7) {\
    return name##ActionP8<p0##_type, p1##_type, p2##_type, p3##_type, \
        p4##_type, p5##_type, p6##_type, p7##_type>(p0, p1, p2, p3, p4, p5, \
        p6, p7);\
  }\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type, typename p5##_type, \
      typename p6##_type, typename p7##_type>\
  template <typename F>\
  template <typename arg0_type, typename arg1_type, typename arg2_type, \
      typename arg3_type, typename arg4_type, typename arg5_type, \
      typename arg6_type, typename arg7_type, typename arg8_type, \
      typename arg9_type>\
  typename ::testing::internal::Function<F>::Result\
      name##ActionP8<p0##_type, p1##_type, p2##_type, p3##_type, p4##_type, \
          p5##_type, p6##_type, \
          p7##_type>::gmock_Impl<F>::gmock_PerformImpl(\
          GMOCK_ACTION_ARG_TYPES_AND_NAMES_UNUSED_) const

#define ACTION_P9(name, p0, p1, p2, p3, p4, p5, p6, p7, p8)\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type, typename p5##_type, \
      typename p6##_type, typename p7##_type, typename p8##_type>\
  class name##ActionP9 {\
   public:\
    name##ActionP9(p0##_type gmock_p0, p1##_type gmock_p1, \
        p2##_type gmock_p2, p3##_type gmock_p3, p4##_type gmock_p4, \
        p5##_type gmock_p5, p6##_type gmock_p6, p7##_type gmock_p7, \
        p8##_type gmock_p8) : p0(gmock_p0), p1(gmock_p1), p2(gmock_p2), \
        p3(gmock_p3), p4(gmock_p4), p5(gmock_p5), p6(gmock_p6), p7(gmock_p7), \
        p8(gmock_p8) {}\
    template <typename F>\
    class gmock_Impl : public ::testing::ActionInterface<F> {\
     public:\
      typedef F function_type;\
      typedef typename ::testing::internal::Function<F>::Result return_type;\
      typedef typename ::testing::internal::Function<F>::ArgumentTuple\
          args_type;\
      gmock_Impl(p0##_type gmock_p0, p1##_type gmock_p1, p2##_type gmock_p2, \
          p3##_type gmock_p3, p4##_type gmock_p4, p5##_type gmock_p5, \
          p6##_type gmock_p6, p7##_type gmock_p7, \
          p8##_type gmock_p8) : p0(gmock_p0), p1(gmock_p1), p2(gmock_p2), \
          p3(gmock_p3), p4(gmock_p4), p5(gmock_p5), p6(gmock_p6), \
          p7(gmock_p7), p8(gmock_p8) {}\
      virtual return_type Perform(const args_type& args) {\
        return ::testing::internal::ActionHelper<return_type, gmock_Impl>::\
            Perform(this, args);\
      }\
      template <typename arg0_type, typename arg1_type, typename arg2_type, \
          typename arg3_type, typename arg4_type, typename arg5_type, \
          typename arg6_type, typename arg7_type, typename arg8_type, \
          typename arg9_type>\
      return_type gmock_PerformImpl(const args_type& args, arg0_type arg0, \
          arg1_type arg1, arg2_type arg2, arg3_type arg3, arg4_type arg4, \
          arg5_type arg5, arg6_type arg6, arg7_type arg7, arg8_type arg8, \
          arg9_type arg9) const;\
      p0##_type p0;\
      p1##_type p1;\
      p2##_type p2;\
      p3##_type p3;\
      p4##_type p4;\
      p5##_type p5;\
      p6##_type p6;\
      p7##_type p7;\
      p8##_type p8;\
     private:\
      GTEST_DISALLOW_ASSIGN_(gmock_Impl);\
    };\
    template <typename F> operator ::testing::Action<F>() const {\
      return ::testing::Action<F>(new gmock_Impl<F>(p0, p1, p2, p3, p4, p5, \
          p6, p7, p8));\
    }\
    p0##_type p0;\
    p1##_type p1;\
    p2##_type p2;\
    p3##_type p3;\
    p4##_type p4;\
    p5##_type p5;\
    p6##_type p6;\
    p7##_type p7;\
    p8##_type p8;\
   private:\
    GTEST_DISALLOW_ASSIGN_(name##ActionP9);\
  };\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type, typename p5##_type, \
      typename p6##_type, typename p7##_type, typename p8##_type>\
  inline name##ActionP9<p0##_type, p1##_type, p2##_type, p3##_type, \
      p4##_type, p5##_type, p6##_type, p7##_type, \
      p8##_type> name(p0##_type p0, p1##_type p1, p2##_type p2, p3##_type p3, \
      p4##_type p4, p5##_type p5, p6##_type p6, p7##_type p7, \
      p8##_type p8) {\
    return name##ActionP9<p0##_type, p1##_type, p2##_type, p3##_type, \
        p4##_type, p5##_type, p6##_type, p7##_type, p8##_type>(p0, p1, p2, \
        p3, p4, p5, p6, p7, p8);\
  }\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type, typename p5##_type, \
      typename p6##_type, typename p7##_type, typename p8##_type>\
  template <typename F>\
  template <typename arg0_type, typename arg1_type, typename arg2_type, \
      typename arg3_type, typename arg4_type, typename arg5_type, \
      typename arg6_type, typename arg7_type, typename arg8_type, \
      typename arg9_type>\
  typename ::testing::internal::Function<F>::Result\
      name##ActionP9<p0##_type, p1##_type, p2##_type, p3##_type, p4##_type, \
          p5##_type, p6##_type, p7##_type, \
          p8##_type>::gmock_Impl<F>::gmock_PerformImpl(\
          GMOCK_ACTION_ARG_TYPES_AND_NAMES_UNUSED_) const

#define ACTION_P10(name, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9)\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type, typename p5##_type, \
      typename p6##_type, typename p7##_type, typename p8##_type, \
      typename p9##_type>\
  class name##ActionP10 {\
   public:\
    name##ActionP10(p0##_type gmock_p0, p1##_type gmock_p1, \
        p2##_type gmock_p2, p3##_type gmock_p3, p4##_type gmock_p4, \
        p5##_type gmock_p5, p6##_type gmock_p6, p7##_type gmock_p7, \
        p8##_type gmock_p8, p9##_type gmock_p9) : p0(gmock_p0), p1(gmock_p1), \
        p2(gmock_p2), p3(gmock_p3), p4(gmock_p4), p5(gmock_p5), p6(gmock_p6), \
        p7(gmock_p7), p8(gmock_p8), p9(gmock_p9) {}\
    template <typename F>\
    class gmock_Impl : public ::testing::ActionInterface<F> {\
     public:\
      typedef F function_type;\
      typedef typename ::testing::internal::Function<F>::Result return_type;\
      typedef typename ::testing::internal::Function<F>::ArgumentTuple\
          args_type;\
      gmock_Impl(p0##_type gmock_p0, p1##_type gmock_p1, p2##_type gmock_p2, \
          p3##_type gmock_p3, p4##_type gmock_p4, p5##_type gmock_p5, \
          p6##_type gmock_p6, p7##_type gmock_p7, p8##_type gmock_p8, \
          p9##_type gmock_p9) : p0(gmock_p0), p1(gmock_p1), p2(gmock_p2), \
          p3(gmock_p3), p4(gmock_p4), p5(gmock_p5), p6(gmock_p6), \
          p7(gmock_p7), p8(gmock_p8), p9(gmock_p9) {}\
      virtual return_type Perform(const args_type& args) {\
        return ::testing::internal::ActionHelper<return_type, gmock_Impl>::\
            Perform(this, args);\
      }\
      template <typename arg0_type, typename arg1_type, typename arg2_type, \
          typename arg3_type, typename arg4_type, typename arg5_type, \
          typename arg6_type, typename arg7_type, typename arg8_type, \
          typename arg9_type>\
      return_type gmock_PerformImpl(const args_type& args, arg0_type arg0, \
          arg1_type arg1, arg2_type arg2, arg3_type arg3, arg4_type arg4, \
          arg5_type arg5, arg6_type arg6, arg7_type arg7, arg8_type arg8, \
          arg9_type arg9) const;\
      p0##_type p0;\
      p1##_type p1;\
      p2##_type p2;\
      p3##_type p3;\
      p4##_type p4;\
      p5##_type p5;\
      p6##_type p6;\
      p7##_type p7;\
      p8##_type p8;\
      p9##_type p9;\
     private:\
      GTEST_DISALLOW_ASSIGN_(gmock_Impl);\
    };\
    template <typename F> operator ::testing::Action<F>() const {\
      return ::testing::Action<F>(new gmock_Impl<F>(p0, p1, p2, p3, p4, p5, \
          p6, p7, p8, p9));\
    }\
    p0##_type p0;\
    p1##_type p1;\
    p2##_type p2;\
    p3##_type p3;\
    p4##_type p4;\
    p5##_type p5;\
    p6##_type p6;\
    p7##_type p7;\
    p8##_type p8;\
    p9##_type p9;\
   private:\
    GTEST_DISALLOW_ASSIGN_(name##ActionP10);\
  };\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type, typename p5##_type, \
      typename p6##_type, typename p7##_type, typename p8##_type, \
      typename p9##_type>\
  inline name##ActionP10<p0##_type, p1##_type, p2##_type, p3##_type, \
      p4##_type, p5##_type, p6##_type, p7##_type, p8##_type, \
      p9##_type> name(p0##_type p0, p1##_type p1, p2##_type p2, p3##_type p3, \
      p4##_type p4, p5##_type p5, p6##_type p6, p7##_type p7, p8##_type p8, \
      p9##_type p9) {\
    return name##ActionP10<p0##_type, p1##_type, p2##_type, p3##_type, \
        p4##_type, p5##_type, p6##_type, p7##_type, p8##_type, p9##_type>(p0, \
        p1, p2, p3, p4, p5, p6, p7, p8, p9);\
  }\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type, typename p5##_type, \
      typename p6##_type, typename p7##_type, typename p8##_type, \
      typename p9##_type>\
  template <typename F>\
  template <typename arg0_type, typename arg1_type, typename arg2_type, \
      typename arg3_type, typename arg4_type, typename arg5_type, \
      typename arg6_type, typename arg7_type, typename arg8_type, \
      typename arg9_type>\
  typename ::testing::internal::Function<F>::Result\
      name##ActionP10<p0##_type, p1##_type, p2##_type, p3##_type, p4##_type, \
          p5##_type, p6##_type, p7##_type, p8##_type, \
          p9##_type>::gmock_Impl<F>::gmock_PerformImpl(\
          GMOCK_ACTION_ARG_TYPES_AND_NAMES_UNUSED_) const

// TODO(wan@google.com): move the following to a different .h file
// such that we don't have to run 'pump' every time the code is
// updated.
namespace testing {

// The ACTION*() macros trigger warning C4100 (unreferenced formal
// parameter) in MSVC with -W4.  Unfortunately they cannot be fixed in
// the macro definition, as the warnings are generated when the macro
// is expanded and macro expansion cannot contain #pragma.  Therefore
// we suppress them here.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4100)
#endif

// Various overloads for InvokeArgument<N>().
//
// The InvokeArgument<N>(a1, a2, ..., a_k) action invokes the N-th
// (0-based) argument, which must be a k-ary callable, of the mock
// function, with arguments a1, a2, ..., a_k.
//
// Notes:
//
//   1. The arguments are passed by value by default.  If you need to
//   pass an argument by reference, wrap it inside ByRef().  For
//   example,
//
//     InvokeArgument<1>(5, string("Hello"), ByRef(foo))
//
//   passes 5 and string("Hello") by value, and passes foo by
//   reference.
//
//   2. If the callable takes an argument by reference but ByRef() is
//   not used, it will receive the reference to a copy of the value,
//   instead of the original value.  For example, when the 0-th
//   argument of the mock function takes a const string&, the action
//
//     InvokeArgument<0>(string("Hello"))
//
//   makes a copy of the temporary string("Hello") object and passes a
//   reference of the copy, instead of the original temporary object,
//   to the callable.  This makes it easy for a user to define an
//   InvokeArgument action from temporary values and have it performed
//   later.

ACTION_TEMPLATE(InvokeArgument,
                HAS_1_TEMPLATE_PARAMS(int, k),
                AND_0_VALUE_PARAMS()) {
  return internal::CallableHelper<return_type>::Call(
      ::std::tr1::get<k>(args));
}

ACTION_TEMPLATE(InvokeArgument,
                HAS_1_TEMPLATE_PARAMS(int, k),
                AND_1_VALUE_PARAMS(p0)) {
  return internal::CallableHelper<return_type>::Call(
      ::std::tr1::get<k>(args), p0);
}

ACTION_TEMPLATE(InvokeArgument,
                HAS_1_TEMPLATE_PARAMS(int, k),
                AND_2_VALUE_PARAMS(p0, p1)) {
  return internal::CallableHelper<return_type>::Call(
      ::std::tr1::get<k>(args), p0, p1);
}

ACTION_TEMPLATE(InvokeArgument,
                HAS_1_TEMPLATE_PARAMS(int, k),
                AND_3_VALUE_PARAMS(p0, p1, p2)) {
  return internal::CallableHelper<return_type>::Call(
      ::std::tr1::get<k>(args), p0, p1, p2);
}

ACTION_TEMPLATE(InvokeArgument,
                HAS_1_TEMPLATE_PARAMS(int, k),
                AND_4_VALUE_PARAMS(p0, p1, p2, p3)) {
  return internal::CallableHelper<return_type>::Call(
      ::std::tr1::get<k>(args), p0, p1, p2, p3);
}

ACTION_TEMPLATE(InvokeArgument,
                HAS_1_TEMPLATE_PARAMS(int, k),
                AND_5_VALUE_PARAMS(p0, p1, p2, p3, p4)) {
  return internal::CallableHelper<return_type>::Call(
      ::std::tr1::get<k>(args), p0, p1, p2, p3, p4);
}

ACTION_TEMPLATE(InvokeArgument,
                HAS_1_TEMPLATE_PARAMS(int, k),
                AND_6_VALUE_PARAMS(p0, p1, p2, p3, p4, p5)) {
  return internal::CallableHelper<return_type>::Call(
      ::std::tr1::get<k>(args), p0, p1, p2, p3, p4, p5);
}

ACTION_TEMPLATE(InvokeArgument,
                HAS_1_TEMPLATE_PARAMS(int, k),
                AND_7_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6)) {
  return internal::CallableHelper<return_type>::Call(
      ::std::tr1::get<k>(args), p0, p1, p2, p3, p4, p5, p6);
}

ACTION_TEMPLATE(InvokeArgument,
                HAS_1_TEMPLATE_PARAMS(int, k),
                AND_8_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, p7)) {
  return internal::CallableHelper<return_type>::Call(
      ::std::tr1::get<k>(args), p0, p1, p2, p3, p4, p5, p6, p7);
}

ACTION_TEMPLATE(InvokeArgument,
                HAS_1_TEMPLATE_PARAMS(int, k),
                AND_9_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, p7, p8)) {
  return internal::CallableHelper<return_type>::Call(
      ::std::tr1::get<k>(args), p0, p1, p2, p3, p4, p5, p6, p7, p8);
}

ACTION_TEMPLATE(InvokeArgument,
                HAS_1_TEMPLATE_PARAMS(int, k),
                AND_10_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9)) {
  return internal::CallableHelper<return_type>::Call(
      ::std::tr1::get<k>(args), p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
}

// Various overloads for ReturnNew<T>().
//
// The ReturnNew<T>(a1, a2, ..., a_k) action returns a pointer to a new
// instance of type T, constructed on the heap with constructor arguments
// a1, a2, ..., and a_k. The caller assumes ownership of the returned value.
ACTION_TEMPLATE(ReturnNew,
                HAS_1_TEMPLATE_PARAMS(typename, T),
                AND_0_VALUE_PARAMS()) {
  return new T();
}

ACTION_TEMPLATE(ReturnNew,
                HAS_1_TEMPLATE_PARAMS(typename, T),
                AND_1_VALUE_PARAMS(p0)) {
  return new T(p0);
}

ACTION_TEMPLATE(ReturnNew,
                HAS_1_TEMPLATE_PARAMS(typename, T),
                AND_2_VALUE_PARAMS(p0, p1)) {
  return new T(p0, p1);
}

ACTION_TEMPLATE(ReturnNew,
                HAS_1_TEMPLATE_PARAMS(typename, T),
                AND_3_VALUE_PARAMS(p0, p1, p2)) {
  return new T(p0, p1, p2);
}

ACTION_TEMPLATE(ReturnNew,
                HAS_1_TEMPLATE_PARAMS(typename, T),
                AND_4_VALUE_PARAMS(p0, p1, p2, p3)) {
  return new T(p0, p1, p2, p3);
}

ACTION_TEMPLATE(ReturnNew,
                HAS_1_TEMPLATE_PARAMS(typename, T),
                AND_5_VALUE_PARAMS(p0, p1, p2, p3, p4)) {
  return new T(p0, p1, p2, p3, p4);
}

ACTION_TEMPLATE(ReturnNew,
                HAS_1_TEMPLATE_PARAMS(typename, T),
                AND_6_VALUE_PARAMS(p0, p1, p2, p3, p4, p5)) {
  return new T(p0, p1, p2, p3, p4, p5);
}

ACTION_TEMPLATE(ReturnNew,
                HAS_1_TEMPLATE_PARAMS(typename, T),
                AND_7_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6)) {
  return new T(p0, p1, p2, p3, p4, p5, p6);
}

ACTION_TEMPLATE(ReturnNew,
                HAS_1_TEMPLATE_PARAMS(typename, T),
                AND_8_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, p7)) {
  return new T(p0, p1, p2, p3, p4, p5, p6, p7);
}

ACTION_TEMPLATE(ReturnNew,
                HAS_1_TEMPLATE_PARAMS(typename, T),
                AND_9_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, p7, p8)) {
  return new T(p0, p1, p2, p3, p4, p5, p6, p7, p8);
}

ACTION_TEMPLATE(ReturnNew,
                HAS_1_TEMPLATE_PARAMS(typename, T),
                AND_10_VALUE_PARAMS(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9)) {
  return new T(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

}  // namespace testing

#endif  // GMOCK_INCLUDE_GMOCK_GMOCK_GENERATED_ACTIONS_H_
// This file was GENERATED by a script.  DO NOT EDIT BY HAND!!!

// Copyright 2007, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Author: wan@google.com (Zhanyong Wan)

// Google Mock - a framework for writing C++ mock classes.
//
// This file implements function mockers of various arities.

#ifndef GMOCK_INCLUDE_GMOCK_GMOCK_GENERATED_FUNCTION_MOCKERS_H_
#define GMOCK_INCLUDE_GMOCK_GMOCK_GENERATED_FUNCTION_MOCKERS_H_

// Copyright 2007, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Author: wan@google.com (Zhanyong Wan)

// Google Mock - a framework for writing C++ mock classes.
//
// This file implements the ON_CALL() and EXPECT_CALL() macros.
//
// A user can use the ON_CALL() macro to specify the default action of
// a mock method.  The syntax is:
//
//   ON_CALL(mock_object, Method(argument-matchers))
//       .With(multi-argument-matcher)
//       .WillByDefault(action);
//
//  where the .With() clause is optional.
//
// A user can use the EXPECT_CALL() macro to specify an expectation on
// a mock method.  The syntax is:
//
//   EXPECT_CALL(mock_object, Method(argument-matchers))
//       .With(multi-argument-matchers)
//       .Times(cardinality)
//       .InSequence(sequences)
//       .After(expectations)
//       .WillOnce(action)
//       .WillRepeatedly(action)
//       .RetiresOnSaturation();
//
// where all clauses are optional, and .InSequence()/.After()/
// .WillOnce() can appear any number of times.

#ifndef GMOCK_INCLUDE_GMOCK_GMOCK_SPEC_BUILDERS_H_
#define GMOCK_INCLUDE_GMOCK_GMOCK_SPEC_BUILDERS_H_

#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

// Copyright 2007, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Author: wan@google.com (Zhanyong Wan)

// Google Mock - a framework for writing C++ mock classes.
//
// This file implements some commonly used argument matchers.  More
// matchers can be defined by the user implementing the
// MatcherInterface<T> interface if necessary.

#ifndef GMOCK_INCLUDE_GMOCK_GMOCK_MATCHERS_H_
#define GMOCK_INCLUDE_GMOCK_GMOCK_MATCHERS_H_

#include <algorithm>
#include <limits>
#include <ostream>  // NOLINT
#include <sstream>
#include <string>
#include <vector>


namespace testing {

// To implement a matcher Foo for type T, define:
//   1. a class FooMatcherImpl that implements the
//      MatcherInterface<T> interface, and
//   2. a factory function that creates a Matcher<T> object from a
//      FooMatcherImpl*.
//
// The two-level delegation design makes it possible to allow a user
// to write "v" instead of "Eq(v)" where a Matcher is expected, which
// is impossible if we pass matchers by pointers.  It also eases
// ownership management as Matcher objects can now be copied like
// plain values.

// MatchResultListener is an abstract class.  Its << operator can be
// used by a matcher to explain why a value matches or doesn't match.
//
// TODO(wan@google.com): add method
//   bool InterestedInWhy(bool result) const;
// to indicate whether the listener is interested in why the match
// result is 'result'.
class MatchResultListener {
 public:
  // Creates a listener object with the given underlying ostream.  The
  // listener does not own the ostream.
  explicit MatchResultListener(::std::ostream* os) : stream_(os) {}
  virtual ~MatchResultListener() = 0;  // Makes this class abstract.

  // Streams x to the underlying ostream; does nothing if the ostream
  // is NULL.
  template <typename T>
  MatchResultListener& operator<<(const T& x) {
    if (stream_ != NULL)
      *stream_ << x;
    return *this;
  }

  // Returns the underlying ostream.
  ::std::ostream* stream() { return stream_; }

  // Returns true iff the listener is interested in an explanation of
  // the match result.  A matcher's MatchAndExplain() method can use
  // this information to avoid generating the explanation when no one
  // intends to hear it.
  bool IsInterested() const { return stream_ != NULL; }

 private:
  ::std::ostream* const stream_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(MatchResultListener);
};

inline MatchResultListener::~MatchResultListener() {
}

// The implementation of a matcher.
template <typename T>
class MatcherInterface {
 public:
  virtual ~MatcherInterface() {}

  // Returns true iff the matcher matches x; also explains the match
  // result to 'listener', in the form of a non-restrictive relative
  // clause ("which ...", "whose ...", etc) that describes x.  For
  // example, the MatchAndExplain() method of the Pointee(...) matcher
  // should generate an explanation like "which points to ...".
  //
  // You should override this method when defining a new matcher.
  //
  // It's the responsibility of the caller (Google Mock) to guarantee
  // that 'listener' is not NULL.  This helps to simplify a matcher's
  // implementation when it doesn't care about the performance, as it
  // can talk to 'listener' without checking its validity first.
  // However, in order to implement dummy listeners efficiently,
  // listener->stream() may be NULL.
  virtual bool MatchAndExplain(T x, MatchResultListener* listener) const = 0;

  // Describes this matcher to an ostream.  The function should print
  // a verb phrase that describes the property a value matching this
  // matcher should have.  The subject of the verb phrase is the value
  // being matched.  For example, the DescribeTo() method of the Gt(7)
  // matcher prints "is greater than 7".
  virtual void DescribeTo(::std::ostream* os) const = 0;

  // Describes the negation of this matcher to an ostream.  For
  // example, if the description of this matcher is "is greater than
  // 7", the negated description could be "is not greater than 7".
  // You are not required to override this when implementing
  // MatcherInterface, but it is highly advised so that your matcher
  // can produce good error messages.
  virtual void DescribeNegationTo(::std::ostream* os) const {
    *os << "not (";
    DescribeTo(os);
    *os << ")";
  }
};

namespace internal {

// A match result listener that ignores the explanation.
class DummyMatchResultListener : public MatchResultListener {
 public:
  DummyMatchResultListener() : MatchResultListener(NULL) {}

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(DummyMatchResultListener);
};

// A match result listener that forwards the explanation to a given
// ostream.  The difference between this and MatchResultListener is
// that the former is concrete.
class StreamMatchResultListener : public MatchResultListener {
 public:
  explicit StreamMatchResultListener(::std::ostream* os)
      : MatchResultListener(os) {}

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(StreamMatchResultListener);
};

// A match result listener that stores the explanation in a string.
class StringMatchResultListener : public MatchResultListener {
 public:
  StringMatchResultListener() : MatchResultListener(&ss_) {}

  // Returns the explanation heard so far.
  internal::string str() const { return ss_.str(); }

 private:
  ::std::stringstream ss_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(StringMatchResultListener);
};

// An internal class for implementing Matcher<T>, which will derive
// from it.  We put functionalities common to all Matcher<T>
// specializations here to avoid code duplication.
template <typename T>
class MatcherBase {
 public:
  // Returns true iff the matcher matches x; also explains the match
  // result to 'listener'.
  bool MatchAndExplain(T x, MatchResultListener* listener) const {
    return impl_->MatchAndExplain(x, listener);
  }

  // Returns true iff this matcher matches x.
  bool Matches(T x) const {
    DummyMatchResultListener dummy;
    return MatchAndExplain(x, &dummy);
  }

  // Describes this matcher to an ostream.
  void DescribeTo(::std::ostream* os) const { impl_->DescribeTo(os); }

  // Describes the negation of this matcher to an ostream.
  void DescribeNegationTo(::std::ostream* os) const {
    impl_->DescribeNegationTo(os);
  }

  // Explains why x matches, or doesn't match, the matcher.
  void ExplainMatchResultTo(T x, ::std::ostream* os) const {
    StreamMatchResultListener listener(os);
    MatchAndExplain(x, &listener);
  }

 protected:
  MatcherBase() {}

  // Constructs a matcher from its implementation.
  explicit MatcherBase(const MatcherInterface<T>* impl)
      : impl_(impl) {}

  virtual ~MatcherBase() {}

 private:
  // shared_ptr (util/gtl/shared_ptr.h) and linked_ptr have similar
  // interfaces.  The former dynamically allocates a chunk of memory
  // to hold the reference count, while the latter tracks all
  // references using a circular linked list without allocating
  // memory.  It has been observed that linked_ptr performs better in
  // typical scenarios.  However, shared_ptr can out-perform
  // linked_ptr when there are many more uses of the copy constructor
  // than the default constructor.
  //
  // If performance becomes a problem, we should see if using
  // shared_ptr helps.
  ::testing::internal::linked_ptr<const MatcherInterface<T> > impl_;
};

}  // namespace internal

// A Matcher<T> is a copyable and IMMUTABLE (except by assignment)
// object that can check whether a value of type T matches.  The
// implementation of Matcher<T> is just a linked_ptr to const
// MatcherInterface<T>, so copying is fairly cheap.  Don't inherit
// from Matcher!
template <typename T>
class Matcher : public internal::MatcherBase<T> {
 public:
  // Constructs a null matcher.  Needed for storing Matcher objects in
  // STL containers.
  Matcher() {}

  // Constructs a matcher from its implementation.
  explicit Matcher(const MatcherInterface<T>* impl)
      : internal::MatcherBase<T>(impl) {}

  // Implicit constructor here allows people to write
  // EXPECT_CALL(foo, Bar(5)) instead of EXPECT_CALL(foo, Bar(Eq(5))) sometimes
  Matcher(T value);  // NOLINT
};

// The following two specializations allow the user to write str
// instead of Eq(str) and "foo" instead of Eq("foo") when a string
// matcher is expected.
template <>
class Matcher<const internal::string&>
    : public internal::MatcherBase<const internal::string&> {
 public:
  Matcher() {}

  explicit Matcher(const MatcherInterface<const internal::string&>* impl)
      : internal::MatcherBase<const internal::string&>(impl) {}

  // Allows the user to write str instead of Eq(str) sometimes, where
  // str is a string object.
  Matcher(const internal::string& s);  // NOLINT

  // Allows the user to write "foo" instead of Eq("foo") sometimes.
  Matcher(const char* s);  // NOLINT
};

template <>
class Matcher<internal::string>
    : public internal::MatcherBase<internal::string> {
 public:
  Matcher() {}

  explicit Matcher(const MatcherInterface<internal::string>* impl)
      : internal::MatcherBase<internal::string>(impl) {}

  // Allows the user to write str instead of Eq(str) sometimes, where
  // str is a string object.
  Matcher(const internal::string& s);  // NOLINT

  // Allows the user to write "foo" instead of Eq("foo") sometimes.
  Matcher(const char* s);  // NOLINT
};

// The PolymorphicMatcher class template makes it easy to implement a
// polymorphic matcher (i.e. a matcher that can match values of more
// than one type, e.g. Eq(n) and NotNull()).
//
// To define a polymorphic matcher, a user should provide an Impl
// class that has a DescribeTo() method and a DescribeNegationTo()
// method, and define a member function (or member function template)
//
//   bool MatchAndExplain(const Value& value,
//                        MatchResultListener* listener) const;
//
// See the definition of NotNull() for a complete example.
template <class Impl>
class PolymorphicMatcher {
 public:
  explicit PolymorphicMatcher(const Impl& an_impl) : impl_(an_impl) {}

  // Returns a mutable reference to the underlying matcher
  // implementation object.
  Impl& mutable_impl() { return impl_; }

  // Returns an immutable reference to the underlying matcher
  // implementation object.
  const Impl& impl() const { return impl_; }

  template <typename T>
  operator Matcher<T>() const {
    return Matcher<T>(new MonomorphicImpl<T>(impl_));
  }

 private:
  template <typename T>
  class MonomorphicImpl : public MatcherInterface<T> {
   public:
    explicit MonomorphicImpl(const Impl& impl) : impl_(impl) {}

    virtual void DescribeTo(::std::ostream* os) const {
      impl_.DescribeTo(os);
    }

    virtual void DescribeNegationTo(::std::ostream* os) const {
      impl_.DescribeNegationTo(os);
    }

    virtual bool MatchAndExplain(T x, MatchResultListener* listener) const {
      return impl_.MatchAndExplain(x, listener);
    }

   private:
    const Impl impl_;

    GTEST_DISALLOW_ASSIGN_(MonomorphicImpl);
  };

  Impl impl_;

  GTEST_DISALLOW_ASSIGN_(PolymorphicMatcher);
};

// Creates a matcher from its implementation.  This is easier to use
// than the Matcher<T> constructor as it doesn't require you to
// explicitly write the template argument, e.g.
//
//   MakeMatcher(foo);
// vs
//   Matcher<const string&>(foo);
template <typename T>
inline Matcher<T> MakeMatcher(const MatcherInterface<T>* impl) {
  return Matcher<T>(impl);
};

// Creates a polymorphic matcher from its implementation.  This is
// easier to use than the PolymorphicMatcher<Impl> constructor as it
// doesn't require you to explicitly write the template argument, e.g.
//
//   MakePolymorphicMatcher(foo);
// vs
//   PolymorphicMatcher<TypeOfFoo>(foo);
template <class Impl>
inline PolymorphicMatcher<Impl> MakePolymorphicMatcher(const Impl& impl) {
  return PolymorphicMatcher<Impl>(impl);
}

// In order to be safe and clear, casting between different matcher
// types is done explicitly via MatcherCast<T>(m), which takes a
// matcher m and returns a Matcher<T>.  It compiles only when T can be
// statically converted to the argument type of m.
template <typename T, typename M>
Matcher<T> MatcherCast(M m);

// Implements SafeMatcherCast().
//
// We use an intermediate class to do the actual safe casting as Nokia's
// Symbian compiler cannot decide between
// template <T, M> ... (M) and
// template <T, U> ... (const Matcher<U>&)
// for function templates but can for member function templates.
template <typename T>
class SafeMatcherCastImpl {
 public:
  // This overload handles polymorphic matchers only since monomorphic
  // matchers are handled by the next one.
  template <typename M>
  static inline Matcher<T> Cast(M polymorphic_matcher) {
    return Matcher<T>(polymorphic_matcher);
  }

  // This overload handles monomorphic matchers.
  //
  // In general, if type T can be implicitly converted to type U, we can
  // safely convert a Matcher<U> to a Matcher<T> (i.e. Matcher is
  // contravariant): just keep a copy of the original Matcher<U>, convert the
  // argument from type T to U, and then pass it to the underlying Matcher<U>.
  // The only exception is when U is a reference and T is not, as the
  // underlying Matcher<U> may be interested in the argument's address, which
  // is not preserved in the conversion from T to U.
  template <typename U>
  static inline Matcher<T> Cast(const Matcher<U>& matcher) {
    // Enforce that T can be implicitly converted to U.
    GMOCK_COMPILE_ASSERT_((internal::ImplicitlyConvertible<T, U>::value),
                          T_must_be_implicitly_convertible_to_U);
    // Enforce that we are not converting a non-reference type T to a reference
    // type U.
    GMOCK_COMPILE_ASSERT_(
        internal::is_reference<T>::value || !internal::is_reference<U>::value,
        cannot_convert_non_referentce_arg_to_reference);
    // In case both T and U are arithmetic types, enforce that the
    // conversion is not lossy.
    typedef GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(T)) RawT;
    typedef GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(U)) RawU;
    const bool kTIsOther = GMOCK_KIND_OF_(RawT) == internal::kOther;
    const bool kUIsOther = GMOCK_KIND_OF_(RawU) == internal::kOther;
    GMOCK_COMPILE_ASSERT_(
        kTIsOther || kUIsOther ||
        (internal::LosslessArithmeticConvertible<RawT, RawU>::value),
        conversion_of_arithmetic_types_must_be_lossless);
    return MatcherCast<T>(matcher);
  }
};

template <typename T, typename M>
inline Matcher<T> SafeMatcherCast(const M& polymorphic_matcher) {
  return SafeMatcherCastImpl<T>::Cast(polymorphic_matcher);
}

// A<T>() returns a matcher that matches any value of type T.
template <typename T>
Matcher<T> A();

// Anything inside the 'internal' namespace IS INTERNAL IMPLEMENTATION
// and MUST NOT BE USED IN USER CODE!!!
namespace internal {

// If the explanation is not empty, prints it to the ostream.
inline void PrintIfNotEmpty(const internal::string& explanation,
                            std::ostream* os) {
  if (explanation != "" && os != NULL) {
    *os << ", " << explanation;
  }
}

// Matches the value against the given matcher, prints the value and explains
// the match result to the listener. Returns the match result.
// 'listener' must not be NULL.
// Value cannot be passed by const reference, because some matchers take a
// non-const argument.
template <typename Value, typename T>
bool MatchPrintAndExplain(Value& value, const Matcher<T>& matcher,
                          MatchResultListener* listener) {
  if (!listener->IsInterested()) {
    // If the listener is not interested, we do not need to construct the
    // inner explanation.
    return matcher.Matches(value);
  }

  StringMatchResultListener inner_listener;
  const bool match = matcher.MatchAndExplain(value, &inner_listener);

  UniversalPrint(value, listener->stream());
  PrintIfNotEmpty(inner_listener.str(), listener->stream());

  return match;
}

// An internal helper class for doing compile-time loop on a tuple's
// fields.
template <size_t N>
class TuplePrefix {
 public:
  // TuplePrefix<N>::Matches(matcher_tuple, value_tuple) returns true
  // iff the first N fields of matcher_tuple matches the first N
  // fields of value_tuple, respectively.
  template <typename MatcherTuple, typename ValueTuple>
  static bool Matches(const MatcherTuple& matcher_tuple,
                      const ValueTuple& value_tuple) {
    using ::std::tr1::get;
    return TuplePrefix<N - 1>::Matches(matcher_tuple, value_tuple)
        && get<N - 1>(matcher_tuple).Matches(get<N - 1>(value_tuple));
  }

  // TuplePrefix<N>::ExplainMatchFailuresTo(matchers, values, os)
  // describes failures in matching the first N fields of matchers
  // against the first N fields of values.  If there is no failure,
  // nothing will be streamed to os.
  template <typename MatcherTuple, typename ValueTuple>
  static void ExplainMatchFailuresTo(const MatcherTuple& matchers,
                                     const ValueTuple& values,
                                     ::std::ostream* os) {
    using ::std::tr1::tuple_element;
    using ::std::tr1::get;

    // First, describes failures in the first N - 1 fields.
    TuplePrefix<N - 1>::ExplainMatchFailuresTo(matchers, values, os);

    // Then describes the failure (if any) in the (N - 1)-th (0-based)
    // field.
    typename tuple_element<N - 1, MatcherTuple>::type matcher =
        get<N - 1>(matchers);
    typedef typename tuple_element<N - 1, ValueTuple>::type Value;
    Value value = get<N - 1>(values);
    StringMatchResultListener listener;
    if (!matcher.MatchAndExplain(value, &listener)) {
      // TODO(wan): include in the message the name of the parameter
      // as used in MOCK_METHOD*() when possible.
      *os << "  Expected arg #" << N - 1 << ": ";
      get<N - 1>(matchers).DescribeTo(os);
      *os << "\n           Actual: ";
      // We remove the reference in type Value to prevent the
      // universal printer from printing the address of value, which
      // isn't interesting to the user most of the time.  The
      // matcher's MatchAndExplain() method handles the case when
      // the address is interesting.
      internal::UniversalPrint(value, os);
      PrintIfNotEmpty(listener.str(), os);
      *os << "\n";
    }
  }
};

// The base case.
template <>
class TuplePrefix<0> {
 public:
  template <typename MatcherTuple, typename ValueTuple>
  static bool Matches(const MatcherTuple& /* matcher_tuple */,
                      const ValueTuple& /* value_tuple */) {
    return true;
  }

  template <typename MatcherTuple, typename ValueTuple>
  static void ExplainMatchFailuresTo(const MatcherTuple& /* matchers */,
                                     const ValueTuple& /* values */,
                                     ::std::ostream* /* os */) {}
};

// TupleMatches(matcher_tuple, value_tuple) returns true iff all
// matchers in matcher_tuple match the corresponding fields in
// value_tuple.  It is a compiler error if matcher_tuple and
// value_tuple have different number of fields or incompatible field
// types.
template <typename MatcherTuple, typename ValueTuple>
bool TupleMatches(const MatcherTuple& matcher_tuple,
                  const ValueTuple& value_tuple) {
  using ::std::tr1::tuple_size;
  // Makes sure that matcher_tuple and value_tuple have the same
  // number of fields.
  GMOCK_COMPILE_ASSERT_(tuple_size<MatcherTuple>::value ==
                        tuple_size<ValueTuple>::value,
                        matcher_and_value_have_different_numbers_of_fields);
  return TuplePrefix<tuple_size<ValueTuple>::value>::
      Matches(matcher_tuple, value_tuple);
}

// Describes failures in matching matchers against values.  If there
// is no failure, nothing will be streamed to os.
template <typename MatcherTuple, typename ValueTuple>
void ExplainMatchFailureTupleTo(const MatcherTuple& matchers,
                                const ValueTuple& values,
                                ::std::ostream* os) {
  using ::std::tr1::tuple_size;
  TuplePrefix<tuple_size<MatcherTuple>::value>::ExplainMatchFailuresTo(
      matchers, values, os);
}

// The MatcherCastImpl class template is a helper for implementing
// MatcherCast().  We need this helper in order to partially
// specialize the implementation of MatcherCast() (C++ allows
// class/struct templates to be partially specialized, but not
// function templates.).

// This general version is used when MatcherCast()'s argument is a
// polymorphic matcher (i.e. something that can be converted to a
// Matcher but is not one yet; for example, Eq(value)).
template <typename T, typename M>
class MatcherCastImpl {
 public:
  static Matcher<T> Cast(M polymorphic_matcher) {
    return Matcher<T>(polymorphic_matcher);
  }
};

// This more specialized version is used when MatcherCast()'s argument
// is already a Matcher.  This only compiles when type T can be
// statically converted to type U.
template <typename T, typename U>
class MatcherCastImpl<T, Matcher<U> > {
 public:
  static Matcher<T> Cast(const Matcher<U>& source_matcher) {
    return Matcher<T>(new Impl(source_matcher));
  }

 private:
  class Impl : public MatcherInterface<T> {
   public:
    explicit Impl(const Matcher<U>& source_matcher)
        : source_matcher_(source_matcher) {}

    // We delegate the matching logic to the source matcher.
    virtual bool MatchAndExplain(T x, MatchResultListener* listener) const {
      return source_matcher_.MatchAndExplain(static_cast<U>(x), listener);
    }

    virtual void DescribeTo(::std::ostream* os) const {
      source_matcher_.DescribeTo(os);
    }

    virtual void DescribeNegationTo(::std::ostream* os) const {
      source_matcher_.DescribeNegationTo(os);
    }

   private:
    const Matcher<U> source_matcher_;

    GTEST_DISALLOW_ASSIGN_(Impl);
  };
};

// This even more specialized version is used for efficiently casting
// a matcher to its own type.
template <typename T>
class MatcherCastImpl<T, Matcher<T> > {
 public:
  static Matcher<T> Cast(const Matcher<T>& matcher) { return matcher; }
};

// Implements A<T>().
template <typename T>
class AnyMatcherImpl : public MatcherInterface<T> {
 public:
  virtual bool MatchAndExplain(
      T /* x */, MatchResultListener* /* listener */) const { return true; }
  virtual void DescribeTo(::std::ostream* os) const { *os << "is anything"; }
  virtual void DescribeNegationTo(::std::ostream* os) const {
    // This is mostly for completeness' safe, as it's not very useful
    // to write Not(A<bool>()).  However we cannot completely rule out
    // such a possibility, and it doesn't hurt to be prepared.
    *os << "never matches";
  }
};

// Implements _, a matcher that matches any value of any
// type.  This is a polymorphic matcher, so we need a template type
// conversion operator to make it appearing as a Matcher<T> for any
// type T.
class AnythingMatcher {
 public:
  template <typename T>
  operator Matcher<T>() const { return A<T>(); }
};

// Implements a matcher that compares a given value with a
// pre-supplied value using one of the ==, <=, <, etc, operators.  The
// two values being compared don't have to have the same type.
//
// The matcher defined here is polymorphic (for example, Eq(5) can be
// used to match an int, a short, a double, etc).  Therefore we use
// a template type conversion operator in the implementation.
//
// We define this as a macro in order to eliminate duplicated source
// code.
//
// The following template definition assumes that the Rhs parameter is
// a "bare" type (i.e. neither 'const T' nor 'T&').
#define GMOCK_IMPLEMENT_COMPARISON_MATCHER_( \
    name, op, relation, negated_relation) \
  template <typename Rhs> class name##Matcher { \
   public: \
    explicit name##Matcher(const Rhs& rhs) : rhs_(rhs) {} \
    template <typename Lhs> \
    operator Matcher<Lhs>() const { \
      return MakeMatcher(new Impl<Lhs>(rhs_)); \
    } \
   private: \
    template <typename Lhs> \
    class Impl : public MatcherInterface<Lhs> { \
     public: \
      explicit Impl(const Rhs& rhs) : rhs_(rhs) {} \
      virtual bool MatchAndExplain(\
          Lhs lhs, MatchResultListener* /* listener */) const { \
        return lhs op rhs_; \
      } \
      virtual void DescribeTo(::std::ostream* os) const { \
        *os << relation  " "; \
        UniversalPrinter<Rhs>::Print(rhs_, os); \
      } \
      virtual void DescribeNegationTo(::std::ostream* os) const { \
        *os << negated_relation  " "; \
        UniversalPrinter<Rhs>::Print(rhs_, os); \
      } \
     private: \
      Rhs rhs_; \
      GTEST_DISALLOW_ASSIGN_(Impl); \
    }; \
    Rhs rhs_; \
    GTEST_DISALLOW_ASSIGN_(name##Matcher); \
  }

// Implements Eq(v), Ge(v), Gt(v), Le(v), Lt(v), and Ne(v)
// respectively.
GMOCK_IMPLEMENT_COMPARISON_MATCHER_(Eq, ==, "is equal to", "isn't equal to");
GMOCK_IMPLEMENT_COMPARISON_MATCHER_(Ge, >=, "is >=", "isn't >=");
GMOCK_IMPLEMENT_COMPARISON_MATCHER_(Gt, >, "is >", "isn't >");
GMOCK_IMPLEMENT_COMPARISON_MATCHER_(Le, <=, "is <=", "isn't <=");
GMOCK_IMPLEMENT_COMPARISON_MATCHER_(Lt, <, "is <", "isn't <");
GMOCK_IMPLEMENT_COMPARISON_MATCHER_(Ne, !=, "isn't equal to", "is equal to");

#undef GMOCK_IMPLEMENT_COMPARISON_MATCHER_

// Implements the polymorphic IsNull() matcher, which matches any raw or smart
// pointer that is NULL.
class IsNullMatcher {
 public:
  template <typename Pointer>
  bool MatchAndExplain(const Pointer& p,
                       MatchResultListener* /* listener */) const {
    return GetRawPointer(p) == NULL;
  }

  void DescribeTo(::std::ostream* os) const { *os << "is NULL"; }
  void DescribeNegationTo(::std::ostream* os) const {
    *os << "isn't NULL";
  }
};

// Implements the polymorphic NotNull() matcher, which matches any raw or smart
// pointer that is not NULL.
class NotNullMatcher {
 public:
  template <typename Pointer>
  bool MatchAndExplain(const Pointer& p,
                       MatchResultListener* /* listener */) const {
    return GetRawPointer(p) != NULL;
  }

  void DescribeTo(::std::ostream* os) const { *os << "isn't NULL"; }
  void DescribeNegationTo(::std::ostream* os) const {
    *os << "is NULL";
  }
};

// Ref(variable) matches any argument that is a reference to
// 'variable'.  This matcher is polymorphic as it can match any
// super type of the type of 'variable'.
//
// The RefMatcher template class implements Ref(variable).  It can
// only be instantiated with a reference type.  This prevents a user
// from mistakenly using Ref(x) to match a non-reference function
// argument.  For example, the following will righteously cause a
// compiler error:
//
//   int n;
//   Matcher<int> m1 = Ref(n);   // This won't compile.
//   Matcher<int&> m2 = Ref(n);  // This will compile.
template <typename T>
class RefMatcher;

template <typename T>
class RefMatcher<T&> {
  // Google Mock is a generic framework and thus needs to support
  // mocking any function types, including those that take non-const
  // reference arguments.  Therefore the template parameter T (and
  // Super below) can be instantiated to either a const type or a
  // non-const type.
 public:
  // RefMatcher() takes a T& instead of const T&, as we want the
  // compiler to catch using Ref(const_value) as a matcher for a
  // non-const reference.
  explicit RefMatcher(T& x) : object_(x) {}  // NOLINT

  template <typename Super>
  operator Matcher<Super&>() const {
    // By passing object_ (type T&) to Impl(), which expects a Super&,
    // we make sure that Super is a super type of T.  In particular,
    // this catches using Ref(const_value) as a matcher for a
    // non-const reference, as you cannot implicitly convert a const
    // reference to a non-const reference.
    return MakeMatcher(new Impl<Super>(object_));
  }

 private:
  template <typename Super>
  class Impl : public MatcherInterface<Super&> {
   public:
    explicit Impl(Super& x) : object_(x) {}  // NOLINT

    // MatchAndExplain() takes a Super& (as opposed to const Super&)
    // in order to match the interface MatcherInterface<Super&>.
    virtual bool MatchAndExplain(
        Super& x, MatchResultListener* listener) const {
      *listener << "which is located @" << static_cast<const void*>(&x);
      return &x == &object_;
    }

    virtual void DescribeTo(::std::ostream* os) const {
      *os << "references the variable ";
      UniversalPrinter<Super&>::Print(object_, os);
    }

    virtual void DescribeNegationTo(::std::ostream* os) const {
      *os << "does not reference the variable ";
      UniversalPrinter<Super&>::Print(object_, os);
    }

   private:
    const Super& object_;

    GTEST_DISALLOW_ASSIGN_(Impl);
  };

  T& object_;

  GTEST_DISALLOW_ASSIGN_(RefMatcher);
};

// Polymorphic helper functions for narrow and wide string matchers.
inline bool CaseInsensitiveCStringEquals(const char* lhs, const char* rhs) {
  return String::CaseInsensitiveCStringEquals(lhs, rhs);
}

inline bool CaseInsensitiveCStringEquals(const wchar_t* lhs,
                                         const wchar_t* rhs) {
  return String::CaseInsensitiveWideCStringEquals(lhs, rhs);
}

// String comparison for narrow or wide strings that can have embedded NUL
// characters.
template <typename StringType>
bool CaseInsensitiveStringEquals(const StringType& s1,
                                 const StringType& s2) {
  // Are the heads equal?
  if (!CaseInsensitiveCStringEquals(s1.c_str(), s2.c_str())) {
    return false;
  }

  // Skip the equal heads.
  const typename StringType::value_type nul = 0;
  const size_t i1 = s1.find(nul), i2 = s2.find(nul);

  // Are we at the end of either s1 or s2?
  if (i1 == StringType::npos || i2 == StringType::npos) {
    return i1 == i2;
  }

  // Are the tails equal?
  return CaseInsensitiveStringEquals(s1.substr(i1 + 1), s2.substr(i2 + 1));
}

// String matchers.

// Implements equality-based string matchers like StrEq, StrCaseNe, and etc.
template <typename StringType>
class StrEqualityMatcher {
 public:
  typedef typename StringType::const_pointer ConstCharPointer;

  StrEqualityMatcher(const StringType& str, bool expect_eq,
                     bool case_sensitive)
      : string_(str), expect_eq_(expect_eq), case_sensitive_(case_sensitive) {}

  // When expect_eq_ is true, returns true iff s is equal to string_;
  // otherwise returns true iff s is not equal to string_.
  bool MatchAndExplain(ConstCharPointer s,
                       MatchResultListener* listener) const {
    if (s == NULL) {
      return !expect_eq_;
    }
    return MatchAndExplain(StringType(s), listener);
  }

  bool MatchAndExplain(const StringType& s,
                       MatchResultListener* /* listener */) const {
    const bool eq = case_sensitive_ ? s == string_ :
        CaseInsensitiveStringEquals(s, string_);
    return expect_eq_ == eq;
  }

  void DescribeTo(::std::ostream* os) const {
    DescribeToHelper(expect_eq_, os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    DescribeToHelper(!expect_eq_, os);
  }

 private:
  void DescribeToHelper(bool expect_eq, ::std::ostream* os) const {
    *os << (expect_eq ? "is " : "isn't ");
    *os << "equal to ";
    if (!case_sensitive_) {
      *os << "(ignoring case) ";
    }
    UniversalPrinter<StringType>::Print(string_, os);
  }

  const StringType string_;
  const bool expect_eq_;
  const bool case_sensitive_;

  GTEST_DISALLOW_ASSIGN_(StrEqualityMatcher);
};

// Implements the polymorphic HasSubstr(substring) matcher, which
// can be used as a Matcher<T> as long as T can be converted to a
// string.
template <typename StringType>
class HasSubstrMatcher {
 public:
  typedef typename StringType::const_pointer ConstCharPointer;

  explicit HasSubstrMatcher(const StringType& substring)
      : substring_(substring) {}

  // These overloaded methods allow HasSubstr(substring) to be used as a
  // Matcher<T> as long as T can be converted to string.  Returns true
  // iff s contains substring_ as a substring.
  bool MatchAndExplain(ConstCharPointer s,
                       MatchResultListener* listener) const {
    return s != NULL && MatchAndExplain(StringType(s), listener);
  }

  bool MatchAndExplain(const StringType& s,
                       MatchResultListener* /* listener */) const {
    return s.find(substring_) != StringType::npos;
  }

  // Describes what this matcher matches.
  void DescribeTo(::std::ostream* os) const {
    *os << "has substring ";
    UniversalPrinter<StringType>::Print(substring_, os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "has no substring ";
    UniversalPrinter<StringType>::Print(substring_, os);
  }

 private:
  const StringType substring_;

  GTEST_DISALLOW_ASSIGN_(HasSubstrMatcher);
};

// Implements the polymorphic StartsWith(substring) matcher, which
// can be used as a Matcher<T> as long as T can be converted to a
// string.
template <typename StringType>
class StartsWithMatcher {
 public:
  typedef typename StringType::const_pointer ConstCharPointer;

  explicit StartsWithMatcher(const StringType& prefix) : prefix_(prefix) {
  }

  // These overloaded methods allow StartsWith(prefix) to be used as a
  // Matcher<T> as long as T can be converted to string.  Returns true
  // iff s starts with prefix_.
  bool MatchAndExplain(ConstCharPointer s,
                       MatchResultListener* listener) const {
    return s != NULL && MatchAndExplain(StringType(s), listener);
  }

  bool MatchAndExplain(const StringType& s,
                       MatchResultListener* /* listener */) const {
    return s.length() >= prefix_.length() &&
        s.substr(0, prefix_.length()) == prefix_;
  }

  void DescribeTo(::std::ostream* os) const {
    *os << "starts with ";
    UniversalPrinter<StringType>::Print(prefix_, os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "doesn't start with ";
    UniversalPrinter<StringType>::Print(prefix_, os);
  }

 private:
  const StringType prefix_;

  GTEST_DISALLOW_ASSIGN_(StartsWithMatcher);
};

// Implements the polymorphic EndsWith(substring) matcher, which
// can be used as a Matcher<T> as long as T can be converted to a
// string.
template <typename StringType>
class EndsWithMatcher {
 public:
  typedef typename StringType::const_pointer ConstCharPointer;

  explicit EndsWithMatcher(const StringType& suffix) : suffix_(suffix) {}

  // These overloaded methods allow EndsWith(suffix) to be used as a
  // Matcher<T> as long as T can be converted to string.  Returns true
  // iff s ends with suffix_.
  bool MatchAndExplain(ConstCharPointer s,
                       MatchResultListener* listener) const {
    return s != NULL && MatchAndExplain(StringType(s), listener);
  }

  bool MatchAndExplain(const StringType& s,
                       MatchResultListener* /* listener */) const {
    return s.length() >= suffix_.length() &&
        s.substr(s.length() - suffix_.length()) == suffix_;
  }

  void DescribeTo(::std::ostream* os) const {
    *os << "ends with ";
    UniversalPrinter<StringType>::Print(suffix_, os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "doesn't end with ";
    UniversalPrinter<StringType>::Print(suffix_, os);
  }

 private:
  const StringType suffix_;

  GTEST_DISALLOW_ASSIGN_(EndsWithMatcher);
};

// Implements polymorphic matchers MatchesRegex(regex) and
// ContainsRegex(regex), which can be used as a Matcher<T> as long as
// T can be converted to a string.
class MatchesRegexMatcher {
 public:
  MatchesRegexMatcher(const RE* regex, bool full_match)
      : regex_(regex), full_match_(full_match) {}

  // These overloaded methods allow MatchesRegex(regex) to be used as
  // a Matcher<T> as long as T can be converted to string.  Returns
  // true iff s matches regular expression regex.  When full_match_ is
  // true, a full match is done; otherwise a partial match is done.
  bool MatchAndExplain(const char* s,
                       MatchResultListener* listener) const {
    return s != NULL && MatchAndExplain(internal::string(s), listener);
  }

  bool MatchAndExplain(const internal::string& s,
                       MatchResultListener* /* listener */) const {
    return full_match_ ? RE::FullMatch(s, *regex_) :
        RE::PartialMatch(s, *regex_);
  }

  void DescribeTo(::std::ostream* os) const {
    *os << (full_match_ ? "matches" : "contains")
        << " regular expression ";
    UniversalPrinter<internal::string>::Print(regex_->pattern(), os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "doesn't " << (full_match_ ? "match" : "contain")
        << " regular expression ";
    UniversalPrinter<internal::string>::Print(regex_->pattern(), os);
  }

 private:
  const internal::linked_ptr<const RE> regex_;
  const bool full_match_;

  GTEST_DISALLOW_ASSIGN_(MatchesRegexMatcher);
};

// Implements a matcher that compares the two fields of a 2-tuple
// using one of the ==, <=, <, etc, operators.  The two fields being
// compared don't have to have the same type.
//
// The matcher defined here is polymorphic (for example, Eq() can be
// used to match a tuple<int, short>, a tuple<const long&, double>,
// etc).  Therefore we use a template type conversion operator in the
// implementation.
//
// We define this as a macro in order to eliminate duplicated source
// code.
#define GMOCK_IMPLEMENT_COMPARISON2_MATCHER_(name, op) \
  class name##2Matcher { \
   public: \
    template <typename T1, typename T2> \
    operator Matcher<const ::std::tr1::tuple<T1, T2>&>() const { \
      return MakeMatcher(new Impl<T1, T2>); \
    } \
   private: \
    template <typename T1, typename T2> \
    class Impl : public MatcherInterface<const ::std::tr1::tuple<T1, T2>&> { \
     public: \
      virtual bool MatchAndExplain( \
          const ::std::tr1::tuple<T1, T2>& args, \
          MatchResultListener* /* listener */) const { \
        return ::std::tr1::get<0>(args) op ::std::tr1::get<1>(args); \
      } \
      virtual void DescribeTo(::std::ostream* os) const { \
        *os << "are a pair (x, y) where x " #op " y"; \
      } \
      virtual void DescribeNegationTo(::std::ostream* os) const { \
        *os << "are a pair (x, y) where x " #op " y is false"; \
      } \
    }; \
  }

// Implements Eq(), Ge(), Gt(), Le(), Lt(), and Ne() respectively.
GMOCK_IMPLEMENT_COMPARISON2_MATCHER_(Eq, ==);
GMOCK_IMPLEMENT_COMPARISON2_MATCHER_(Ge, >=);
GMOCK_IMPLEMENT_COMPARISON2_MATCHER_(Gt, >);
GMOCK_IMPLEMENT_COMPARISON2_MATCHER_(Le, <=);
GMOCK_IMPLEMENT_COMPARISON2_MATCHER_(Lt, <);
GMOCK_IMPLEMENT_COMPARISON2_MATCHER_(Ne, !=);

#undef GMOCK_IMPLEMENT_COMPARISON2_MATCHER_

// Implements the Not(...) matcher for a particular argument type T.
// We do not nest it inside the NotMatcher class template, as that
// will prevent different instantiations of NotMatcher from sharing
// the same NotMatcherImpl<T> class.
template <typename T>
class NotMatcherImpl : public MatcherInterface<T> {
 public:
  explicit NotMatcherImpl(const Matcher<T>& matcher)
      : matcher_(matcher) {}

  virtual bool MatchAndExplain(T x, MatchResultListener* listener) const {
    return !matcher_.MatchAndExplain(x, listener);
  }

  virtual void DescribeTo(::std::ostream* os) const {
    matcher_.DescribeNegationTo(os);
  }

  virtual void DescribeNegationTo(::std::ostream* os) const {
    matcher_.DescribeTo(os);
  }

 private:
  const Matcher<T> matcher_;

  GTEST_DISALLOW_ASSIGN_(NotMatcherImpl);
};

// Implements the Not(m) matcher, which matches a value that doesn't
// match matcher m.
template <typename InnerMatcher>
class NotMatcher {
 public:
  explicit NotMatcher(InnerMatcher matcher) : matcher_(matcher) {}

  // This template type conversion operator allows Not(m) to be used
  // to match any type m can match.
  template <typename T>
  operator Matcher<T>() const {
    return Matcher<T>(new NotMatcherImpl<T>(SafeMatcherCast<T>(matcher_)));
  }

 private:
  InnerMatcher matcher_;

  GTEST_DISALLOW_ASSIGN_(NotMatcher);
};

// Implements the AllOf(m1, m2) matcher for a particular argument type
// T. We do not nest it inside the BothOfMatcher class template, as
// that will prevent different instantiations of BothOfMatcher from
// sharing the same BothOfMatcherImpl<T> class.
template <typename T>
class BothOfMatcherImpl : public MatcherInterface<T> {
 public:
  BothOfMatcherImpl(const Matcher<T>& matcher1, const Matcher<T>& matcher2)
      : matcher1_(matcher1), matcher2_(matcher2) {}

  virtual void DescribeTo(::std::ostream* os) const {
    *os << "(";
    matcher1_.DescribeTo(os);
    *os << ") and (";
    matcher2_.DescribeTo(os);
    *os << ")";
  }

  virtual void DescribeNegationTo(::std::ostream* os) const {
    *os << "(";
    matcher1_.DescribeNegationTo(os);
    *os << ") or (";
    matcher2_.DescribeNegationTo(os);
    *os << ")";
  }

  virtual bool MatchAndExplain(T x, MatchResultListener* listener) const {
    // If either matcher1_ or matcher2_ doesn't match x, we only need
    // to explain why one of them fails.
    StringMatchResultListener listener1;
    if (!matcher1_.MatchAndExplain(x, &listener1)) {
      *listener << listener1.str();
      return false;
    }

    StringMatchResultListener listener2;
    if (!matcher2_.MatchAndExplain(x, &listener2)) {
      *listener << listener2.str();
      return false;
    }

    // Otherwise we need to explain why *both* of them match.
    const internal::string s1 = listener1.str();
    const internal::string s2 = listener2.str();

    if (s1 == "") {
      *listener << s2;
    } else {
      *listener << s1;
      if (s2 != "") {
        *listener << ", and " << s2;
      }
    }
    return true;
  }

 private:
  const Matcher<T> matcher1_;
  const Matcher<T> matcher2_;

  GTEST_DISALLOW_ASSIGN_(BothOfMatcherImpl);
};

// Used for implementing the AllOf(m_1, ..., m_n) matcher, which
// matches a value that matches all of the matchers m_1, ..., and m_n.
template <typename Matcher1, typename Matcher2>
class BothOfMatcher {
 public:
  BothOfMatcher(Matcher1 matcher1, Matcher2 matcher2)
      : matcher1_(matcher1), matcher2_(matcher2) {}

  // This template type conversion operator allows a
  // BothOfMatcher<Matcher1, Matcher2> object to match any type that
  // both Matcher1 and Matcher2 can match.
  template <typename T>
  operator Matcher<T>() const {
    return Matcher<T>(new BothOfMatcherImpl<T>(SafeMatcherCast<T>(matcher1_),
                                               SafeMatcherCast<T>(matcher2_)));
  }

 private:
  Matcher1 matcher1_;
  Matcher2 matcher2_;

  GTEST_DISALLOW_ASSIGN_(BothOfMatcher);
};

// Implements the AnyOf(m1, m2) matcher for a particular argument type
// T.  We do not nest it inside the AnyOfMatcher class template, as
// that will prevent different instantiations of AnyOfMatcher from
// sharing the same EitherOfMatcherImpl<T> class.
template <typename T>
class EitherOfMatcherImpl : public MatcherInterface<T> {
 public:
  EitherOfMatcherImpl(const Matcher<T>& matcher1, const Matcher<T>& matcher2)
      : matcher1_(matcher1), matcher2_(matcher2) {}

  virtual void DescribeTo(::std::ostream* os) const {
    *os << "(";
    matcher1_.DescribeTo(os);
    *os << ") or (";
    matcher2_.DescribeTo(os);
    *os << ")";
  }

  virtual void DescribeNegationTo(::std::ostream* os) const {
    *os << "(";
    matcher1_.DescribeNegationTo(os);
    *os << ") and (";
    matcher2_.DescribeNegationTo(os);
    *os << ")";
  }

  virtual bool MatchAndExplain(T x, MatchResultListener* listener) const {
    // If either matcher1_ or matcher2_ matches x, we just need to
    // explain why *one* of them matches.
    StringMatchResultListener listener1;
    if (matcher1_.MatchAndExplain(x, &listener1)) {
      *listener << listener1.str();
      return true;
    }

    StringMatchResultListener listener2;
    if (matcher2_.MatchAndExplain(x, &listener2)) {
      *listener << listener2.str();
      return true;
    }

    // Otherwise we need to explain why *both* of them fail.
    const internal::string s1 = listener1.str();
    const internal::string s2 = listener2.str();

    if (s1 == "") {
      *listener << s2;
    } else {
      *listener << s1;
      if (s2 != "") {
        *listener << ", and " << s2;
      }
    }
    return false;
  }

 private:
  const Matcher<T> matcher1_;
  const Matcher<T> matcher2_;

  GTEST_DISALLOW_ASSIGN_(EitherOfMatcherImpl);
};

// Used for implementing the AnyOf(m_1, ..., m_n) matcher, which
// matches a value that matches at least one of the matchers m_1, ...,
// and m_n.
template <typename Matcher1, typename Matcher2>
class EitherOfMatcher {
 public:
  EitherOfMatcher(Matcher1 matcher1, Matcher2 matcher2)
      : matcher1_(matcher1), matcher2_(matcher2) {}

  // This template type conversion operator allows a
  // EitherOfMatcher<Matcher1, Matcher2> object to match any type that
  // both Matcher1 and Matcher2 can match.
  template <typename T>
  operator Matcher<T>() const {
    return Matcher<T>(new EitherOfMatcherImpl<T>(
        SafeMatcherCast<T>(matcher1_), SafeMatcherCast<T>(matcher2_)));
  }

 private:
  Matcher1 matcher1_;
  Matcher2 matcher2_;

  GTEST_DISALLOW_ASSIGN_(EitherOfMatcher);
};

// Used for implementing Truly(pred), which turns a predicate into a
// matcher.
template <typename Predicate>
class TrulyMatcher {
 public:
  explicit TrulyMatcher(Predicate pred) : predicate_(pred) {}

  // This method template allows Truly(pred) to be used as a matcher
  // for type T where T is the argument type of predicate 'pred'.  The
  // argument is passed by reference as the predicate may be
  // interested in the address of the argument.
  template <typename T>
  bool MatchAndExplain(T& x,  // NOLINT
                       MatchResultListener* /* listener */) const {
#if GTEST_OS_WINDOWS
    // MSVC warns about converting a value into bool (warning 4800).
#pragma warning(push)          // Saves the current warning state.
#pragma warning(disable:4800)  // Temporarily disables warning 4800.
#endif  // GTEST_OS_WINDOWS
    return predicate_(x);
#if GTEST_OS_WINDOWS
#pragma warning(pop)           // Restores the warning state.
#endif  // GTEST_OS_WINDOWS
  }

  void DescribeTo(::std::ostream* os) const {
    *os << "satisfies the given predicate";
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "doesn't satisfy the given predicate";
  }

 private:
  Predicate predicate_;

  GTEST_DISALLOW_ASSIGN_(TrulyMatcher);
};

// Used for implementing Matches(matcher), which turns a matcher into
// a predicate.
template <typename M>
class MatcherAsPredicate {
 public:
  explicit MatcherAsPredicate(M matcher) : matcher_(matcher) {}

  // This template operator() allows Matches(m) to be used as a
  // predicate on type T where m is a matcher on type T.
  //
  // The argument x is passed by reference instead of by value, as
  // some matcher may be interested in its address (e.g. as in
  // Matches(Ref(n))(x)).
  template <typename T>
  bool operator()(const T& x) const {
    // We let matcher_ commit to a particular type here instead of
    // when the MatcherAsPredicate object was constructed.  This
    // allows us to write Matches(m) where m is a polymorphic matcher
    // (e.g. Eq(5)).
    //
    // If we write Matcher<T>(matcher_).Matches(x) here, it won't
    // compile when matcher_ has type Matcher<const T&>; if we write
    // Matcher<const T&>(matcher_).Matches(x) here, it won't compile
    // when matcher_ has type Matcher<T>; if we just write
    // matcher_.Matches(x), it won't compile when matcher_ is
    // polymorphic, e.g. Eq(5).
    //
    // MatcherCast<const T&>() is necessary for making the code work
    // in all of the above situations.
    return MatcherCast<const T&>(matcher_).Matches(x);
  }

 private:
  M matcher_;

  GTEST_DISALLOW_ASSIGN_(MatcherAsPredicate);
};

// For implementing ASSERT_THAT() and EXPECT_THAT().  The template
// argument M must be a type that can be converted to a matcher.
template <typename M>
class PredicateFormatterFromMatcher {
 public:
  explicit PredicateFormatterFromMatcher(const M& m) : matcher_(m) {}

  // This template () operator allows a PredicateFormatterFromMatcher
  // object to act as a predicate-formatter suitable for using with
  // Google Test's EXPECT_PRED_FORMAT1() macro.
  template <typename T>
  AssertionResult operator()(const char* value_text, const T& x) const {
    // We convert matcher_ to a Matcher<const T&> *now* instead of
    // when the PredicateFormatterFromMatcher object was constructed,
    // as matcher_ may be polymorphic (e.g. NotNull()) and we won't
    // know which type to instantiate it to until we actually see the
    // type of x here.
    //
    // We write MatcherCast<const T&>(matcher_) instead of
    // Matcher<const T&>(matcher_), as the latter won't compile when
    // matcher_ has type Matcher<T> (e.g. An<int>()).
    const Matcher<const T&> matcher = MatcherCast<const T&>(matcher_);
    StringMatchResultListener listener;
    if (MatchPrintAndExplain(x, matcher, &listener))
      return AssertionSuccess();

    ::std::stringstream ss;
    ss << "Value of: " << value_text << "\n"
       << "Expected: ";
    matcher.DescribeTo(&ss);
    ss << "\n  Actual: " << listener.str();
    return AssertionFailure() << ss.str();
  }

 private:
  const M matcher_;

  GTEST_DISALLOW_ASSIGN_(PredicateFormatterFromMatcher);
};

// A helper function for converting a matcher to a predicate-formatter
// without the user needing to explicitly write the type.  This is
// used for implementing ASSERT_THAT() and EXPECT_THAT().
template <typename M>
inline PredicateFormatterFromMatcher<M>
MakePredicateFormatterFromMatcher(const M& matcher) {
  return PredicateFormatterFromMatcher<M>(matcher);
}

// Implements the polymorphic floating point equality matcher, which
// matches two float values using ULP-based approximation.  The
// template is meant to be instantiated with FloatType being either
// float or double.
template <typename FloatType>
class FloatingEqMatcher {
 public:
  // Constructor for FloatingEqMatcher.
  // The matcher's input will be compared with rhs.  The matcher treats two
  // NANs as equal if nan_eq_nan is true.  Otherwise, under IEEE standards,
  // equality comparisons between NANs will always return false.
  FloatingEqMatcher(FloatType rhs, bool nan_eq_nan) :
    rhs_(rhs), nan_eq_nan_(nan_eq_nan) {}

  // Implements floating point equality matcher as a Matcher<T>.
  template <typename T>
  class Impl : public MatcherInterface<T> {
   public:
    Impl(FloatType rhs, bool nan_eq_nan) :
      rhs_(rhs), nan_eq_nan_(nan_eq_nan) {}

    virtual bool MatchAndExplain(T value,
                                 MatchResultListener* /* listener */) const {
      const FloatingPoint<FloatType> lhs(value), rhs(rhs_);

      // Compares NaNs first, if nan_eq_nan_ is true.
      if (nan_eq_nan_ && lhs.is_nan()) {
        return rhs.is_nan();
      }

      return lhs.AlmostEquals(rhs);
    }

    virtual void DescribeTo(::std::ostream* os) const {
      // os->precision() returns the previously set precision, which we
      // store to restore the ostream to its original configuration
      // after outputting.
      const ::std::streamsize old_precision = os->precision(
          ::std::numeric_limits<FloatType>::digits10 + 2);
      if (FloatingPoint<FloatType>(rhs_).is_nan()) {
        if (nan_eq_nan_) {
          *os << "is NaN";
        } else {
          *os << "never matches";
        }
      } else {
        *os << "is approximately " << rhs_;
      }
      os->precision(old_precision);
    }

    virtual void DescribeNegationTo(::std::ostream* os) const {
      // As before, get original precision.
      const ::std::streamsize old_precision = os->precision(
          ::std::numeric_limits<FloatType>::digits10 + 2);
      if (FloatingPoint<FloatType>(rhs_).is_nan()) {
        if (nan_eq_nan_) {
          *os << "isn't NaN";
        } else {
          *os << "is anything";
        }
      } else {
        *os << "isn't approximately " << rhs_;
      }
      // Restore original precision.
      os->precision(old_precision);
    }

   private:
    const FloatType rhs_;
    const bool nan_eq_nan_;

    GTEST_DISALLOW_ASSIGN_(Impl);
  };

  // The following 3 type conversion operators allow FloatEq(rhs) and
  // NanSensitiveFloatEq(rhs) to be used as a Matcher<float>, a
  // Matcher<const float&>, or a Matcher<float&>, but nothing else.
  // (While Google's C++ coding style doesn't allow arguments passed
  // by non-const reference, we may see them in code not conforming to
  // the style.  Therefore Google Mock needs to support them.)
  operator Matcher<FloatType>() const {
    return MakeMatcher(new Impl<FloatType>(rhs_, nan_eq_nan_));
  }

  operator Matcher<const FloatType&>() const {
    return MakeMatcher(new Impl<const FloatType&>(rhs_, nan_eq_nan_));
  }

  operator Matcher<FloatType&>() const {
    return MakeMatcher(new Impl<FloatType&>(rhs_, nan_eq_nan_));
  }
 private:
  const FloatType rhs_;
  const bool nan_eq_nan_;

  GTEST_DISALLOW_ASSIGN_(FloatingEqMatcher);
};

// Implements the Pointee(m) matcher for matching a pointer whose
// pointee matches matcher m.  The pointer can be either raw or smart.
template <typename InnerMatcher>
class PointeeMatcher {
 public:
  explicit PointeeMatcher(const InnerMatcher& matcher) : matcher_(matcher) {}

  // This type conversion operator template allows Pointee(m) to be
  // used as a matcher for any pointer type whose pointee type is
  // compatible with the inner matcher, where type Pointer can be
  // either a raw pointer or a smart pointer.
  //
  // The reason we do this instead of relying on
  // MakePolymorphicMatcher() is that the latter is not flexible
  // enough for implementing the DescribeTo() method of Pointee().
  template <typename Pointer>
  operator Matcher<Pointer>() const {
    return MakeMatcher(new Impl<Pointer>(matcher_));
  }

 private:
  // The monomorphic implementation that works for a particular pointer type.
  template <typename Pointer>
  class Impl : public MatcherInterface<Pointer> {
   public:
    typedef typename PointeeOf<GMOCK_REMOVE_CONST_(  // NOLINT
        GMOCK_REMOVE_REFERENCE_(Pointer))>::type Pointee;

    explicit Impl(const InnerMatcher& matcher)
        : matcher_(MatcherCast<const Pointee&>(matcher)) {}

    virtual void DescribeTo(::std::ostream* os) const {
      *os << "points to a value that ";
      matcher_.DescribeTo(os);
    }

    virtual void DescribeNegationTo(::std::ostream* os) const {
      *os << "does not point to a value that ";
      matcher_.DescribeTo(os);
    }

    virtual bool MatchAndExplain(Pointer pointer,
                                 MatchResultListener* listener) const {
      if (GetRawPointer(pointer) == NULL)
        return false;

      *listener << "which points to ";
      return MatchPrintAndExplain(*pointer, matcher_, listener);
    }

   private:
    const Matcher<const Pointee&> matcher_;

    GTEST_DISALLOW_ASSIGN_(Impl);
  };

  const InnerMatcher matcher_;

  GTEST_DISALLOW_ASSIGN_(PointeeMatcher);
};

// Implements the Field() matcher for matching a field (i.e. member
// variable) of an object.
template <typename Class, typename FieldType>
class FieldMatcher {
 public:
  FieldMatcher(FieldType Class::*field,
               const Matcher<const FieldType&>& matcher)
      : field_(field), matcher_(matcher) {}

  void DescribeTo(::std::ostream* os) const {
    *os << "is an object whose given field ";
    matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "is an object whose given field ";
    matcher_.DescribeNegationTo(os);
  }

  template <typename T>
  bool MatchAndExplain(const T& value, MatchResultListener* listener) const {
    return MatchAndExplainImpl(
        typename ::testing::internal::
            is_pointer<GMOCK_REMOVE_CONST_(T)>::type(),
        value, listener);
  }

 private:
  // The first argument of MatchAndExplainImpl() is needed to help
  // Symbian's C++ compiler choose which overload to use.  Its type is
  // true_type iff the Field() matcher is used to match a pointer.
  bool MatchAndExplainImpl(false_type /* is_not_pointer */, const Class& obj,
                           MatchResultListener* listener) const {
    *listener << "whose given field is ";
    return MatchPrintAndExplain(obj.*field_, matcher_, listener);
  }

  bool MatchAndExplainImpl(true_type /* is_pointer */, const Class* p,
                           MatchResultListener* listener) const {
    if (p == NULL)
      return false;

    *listener << "which points to an object ";
    // Since *p has a field, it must be a class/struct/union type and
    // thus cannot be a pointer.  Therefore we pass false_type() as
    // the first argument.
    return MatchAndExplainImpl(false_type(), *p, listener);
  }

  const FieldType Class::*field_;
  const Matcher<const FieldType&> matcher_;

  GTEST_DISALLOW_ASSIGN_(FieldMatcher);
};

// Implements the Property() matcher for matching a property
// (i.e. return value of a getter method) of an object.
template <typename Class, typename PropertyType>
class PropertyMatcher {
 public:
  // The property may have a reference type, so 'const PropertyType&'
  // may cause double references and fail to compile.  That's why we
  // need GMOCK_REFERENCE_TO_CONST, which works regardless of
  // PropertyType being a reference or not.
  typedef GMOCK_REFERENCE_TO_CONST_(PropertyType) RefToConstProperty;

  PropertyMatcher(PropertyType (Class::*property)() const,
                  const Matcher<RefToConstProperty>& matcher)
      : property_(property), matcher_(matcher) {}

  void DescribeTo(::std::ostream* os) const {
    *os << "is an object whose given property ";
    matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "is an object whose given property ";
    matcher_.DescribeNegationTo(os);
  }

  template <typename T>
  bool MatchAndExplain(const T&value, MatchResultListener* listener) const {
    return MatchAndExplainImpl(
        typename ::testing::internal::
            is_pointer<GMOCK_REMOVE_CONST_(T)>::type(),
        value, listener);
  }

 private:
  // The first argument of MatchAndExplainImpl() is needed to help
  // Symbian's C++ compiler choose which overload to use.  Its type is
  // true_type iff the Property() matcher is used to match a pointer.
  bool MatchAndExplainImpl(false_type /* is_not_pointer */, const Class& obj,
                           MatchResultListener* listener) const {
    *listener << "whose given property is ";
    // Cannot pass the return value (for example, int) to MatchPrintAndExplain,
    // which takes a non-const reference as argument.
    RefToConstProperty result = (obj.*property_)();
    return MatchPrintAndExplain(result, matcher_, listener);
  }

  bool MatchAndExplainImpl(true_type /* is_pointer */, const Class* p,
                           MatchResultListener* listener) const {
    if (p == NULL)
      return false;

    *listener << "which points to an object ";
    // Since *p has a property method, it must be a class/struct/union
    // type and thus cannot be a pointer.  Therefore we pass
    // false_type() as the first argument.
    return MatchAndExplainImpl(false_type(), *p, listener);
  }

  PropertyType (Class::*property_)() const;
  const Matcher<RefToConstProperty> matcher_;

  GTEST_DISALLOW_ASSIGN_(PropertyMatcher);
};

// Type traits specifying various features of different functors for ResultOf.
// The default template specifies features for functor objects.
// Functor classes have to typedef argument_type and result_type
// to be compatible with ResultOf.
template <typename Functor>
struct CallableTraits {
  typedef typename Functor::result_type ResultType;
  typedef Functor StorageType;

  static void CheckIsValid(Functor /* functor */) {}
  template <typename T>
  static ResultType Invoke(Functor f, T arg) { return f(arg); }
};

// Specialization for function pointers.
template <typename ArgType, typename ResType>
struct CallableTraits<ResType(*)(ArgType)> {
  typedef ResType ResultType;
  typedef ResType(*StorageType)(ArgType);

  static void CheckIsValid(ResType(*f)(ArgType)) {
    GTEST_CHECK_(f != NULL)
        << "NULL function pointer is passed into ResultOf().";
  }
  template <typename T>
  static ResType Invoke(ResType(*f)(ArgType), T arg) {
    return (*f)(arg);
  }
};

// Implements the ResultOf() matcher for matching a return value of a
// unary function of an object.
template <typename Callable>
class ResultOfMatcher {
 public:
  typedef typename CallableTraits<Callable>::ResultType ResultType;

  ResultOfMatcher(Callable callable, const Matcher<ResultType>& matcher)
      : callable_(callable), matcher_(matcher) {
    CallableTraits<Callable>::CheckIsValid(callable_);
  }

  template <typename T>
  operator Matcher<T>() const {
    return Matcher<T>(new Impl<T>(callable_, matcher_));
  }

 private:
  typedef typename CallableTraits<Callable>::StorageType CallableStorageType;

  template <typename T>
  class Impl : public MatcherInterface<T> {
   public:
    Impl(CallableStorageType callable, const Matcher<ResultType>& matcher)
        : callable_(callable), matcher_(matcher) {}

    virtual void DescribeTo(::std::ostream* os) const {
      *os << "is mapped by the given callable to a value that ";
      matcher_.DescribeTo(os);
    }

    virtual void DescribeNegationTo(::std::ostream* os) const {
      *os << "is mapped by the given callable to a value that ";
      matcher_.DescribeNegationTo(os);
    }

    virtual bool MatchAndExplain(T obj, MatchResultListener* listener) const {
      *listener << "which is mapped by the given callable to ";
      // Cannot pass the return value (for example, int) to
      // MatchPrintAndExplain, which takes a non-const reference as argument.
      ResultType result =
          CallableTraits<Callable>::template Invoke<T>(callable_, obj);
      return MatchPrintAndExplain(result, matcher_, listener);
    }

   private:
    // Functors often define operator() as non-const method even though
    // they are actualy stateless. But we need to use them even when
    // 'this' is a const pointer. It's the user's responsibility not to
    // use stateful callables with ResultOf(), which does't guarantee
    // how many times the callable will be invoked.
    mutable CallableStorageType callable_;
    const Matcher<ResultType> matcher_;

    GTEST_DISALLOW_ASSIGN_(Impl);
  };  // class Impl

  const CallableStorageType callable_;
  const Matcher<ResultType> matcher_;

  GTEST_DISALLOW_ASSIGN_(ResultOfMatcher);
};

// Implements an equality matcher for any STL-style container whose elements
// support ==. This matcher is like Eq(), but its failure explanations provide
// more detailed information that is useful when the container is used as a set.
// The failure message reports elements that are in one of the operands but not
// the other. The failure messages do not report duplicate or out-of-order
// elements in the containers (which don't properly matter to sets, but can
// occur if the containers are vectors or lists, for example).
//
// Uses the container's const_iterator, value_type, operator ==,
// begin(), and end().
template <typename Container>
class ContainerEqMatcher {
 public:
  typedef internal::StlContainerView<Container> View;
  typedef typename View::type StlContainer;
  typedef typename View::const_reference StlContainerReference;

  // We make a copy of rhs in case the elements in it are modified
  // after this matcher is created.
  explicit ContainerEqMatcher(const Container& rhs) : rhs_(View::Copy(rhs)) {
    // Makes sure the user doesn't instantiate this class template
    // with a const or reference type.
    testing::StaticAssertTypeEq<Container,
        GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(Container))>();
  }

  void DescribeTo(::std::ostream* os) const {
    *os << "equals ";
    UniversalPrinter<StlContainer>::Print(rhs_, os);
  }
  void DescribeNegationTo(::std::ostream* os) const {
    *os << "does not equal ";
    UniversalPrinter<StlContainer>::Print(rhs_, os);
  }

  template <typename LhsContainer>
  bool MatchAndExplain(const LhsContainer& lhs,
                       MatchResultListener* listener) const {
    // GMOCK_REMOVE_CONST_() is needed to work around an MSVC 8.0 bug
    // that causes LhsContainer to be a const type sometimes.
    typedef internal::StlContainerView<GMOCK_REMOVE_CONST_(LhsContainer)>
        LhsView;
    typedef typename LhsView::type LhsStlContainer;
    StlContainerReference lhs_stl_container = LhsView::ConstReference(lhs);
    if (lhs_stl_container == rhs_)
      return true;

    ::std::ostream* const os = listener->stream();
    if (os != NULL) {
      // Something is different. Check for extra values first.
      bool printed_header = false;
      for (typename LhsStlContainer::const_iterator it =
               lhs_stl_container.begin();
           it != lhs_stl_container.end(); ++it) {
        if (internal::ArrayAwareFind(rhs_.begin(), rhs_.end(), *it) ==
            rhs_.end()) {
          if (printed_header) {
            *os << ", ";
          } else {
            *os << "which has these unexpected elements: ";
            printed_header = true;
          }
          UniversalPrinter<typename LhsStlContainer::value_type>::
              Print(*it, os);
        }
      }

      // Now check for missing values.
      bool printed_header2 = false;
      for (typename StlContainer::const_iterator it = rhs_.begin();
           it != rhs_.end(); ++it) {
        if (internal::ArrayAwareFind(
                lhs_stl_container.begin(), lhs_stl_container.end(), *it) ==
            lhs_stl_container.end()) {
          if (printed_header2) {
            *os << ", ";
          } else {
            *os << (printed_header ? ",\nand" : "which")
                << " doesn't have these expected elements: ";
            printed_header2 = true;
          }
          UniversalPrinter<typename StlContainer::value_type>::Print(*it, os);
        }
      }
    }

    return false;
  }

 private:
  const StlContainer rhs_;

  GTEST_DISALLOW_ASSIGN_(ContainerEqMatcher);
};

// Implements Contains(element_matcher) for the given argument type Container.
template <typename Container>
class ContainsMatcherImpl : public MatcherInterface<Container> {
 public:
  typedef GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(Container)) RawContainer;
  typedef StlContainerView<RawContainer> View;
  typedef typename View::type StlContainer;
  typedef typename View::const_reference StlContainerReference;
  typedef typename StlContainer::value_type Element;

  template <typename InnerMatcher>
  explicit ContainsMatcherImpl(InnerMatcher inner_matcher)
      : inner_matcher_(
          testing::SafeMatcherCast<const Element&>(inner_matcher)) {}

  // Describes what this matcher does.
  virtual void DescribeTo(::std::ostream* os) const {
    *os << "contains at least one element that ";
    inner_matcher_.DescribeTo(os);
  }

  // Describes what the negation of this matcher does.
  virtual void DescribeNegationTo(::std::ostream* os) const {
    *os << "doesn't contain any element that ";
    inner_matcher_.DescribeTo(os);
  }

  virtual bool MatchAndExplain(Container container,
                               MatchResultListener* listener) const {
    StlContainerReference stl_container = View::ConstReference(container);
    size_t i = 0;
    for (typename StlContainer::const_iterator it = stl_container.begin();
         it != stl_container.end(); ++it, ++i) {
      StringMatchResultListener inner_listener;
      if (inner_matcher_.MatchAndExplain(*it, &inner_listener)) {
        *listener << "whose element #" << i << " matches";
        PrintIfNotEmpty(inner_listener.str(), listener->stream());
        return true;
      }
    }
    return false;
  }

 private:
  const Matcher<const Element&> inner_matcher_;

  GTEST_DISALLOW_ASSIGN_(ContainsMatcherImpl);
};

// Implements polymorphic Contains(element_matcher).
template <typename M>
class ContainsMatcher {
 public:
  explicit ContainsMatcher(M m) : inner_matcher_(m) {}

  template <typename Container>
  operator Matcher<Container>() const {
    return MakeMatcher(new ContainsMatcherImpl<Container>(inner_matcher_));
  }

 private:
  const M inner_matcher_;

  GTEST_DISALLOW_ASSIGN_(ContainsMatcher);
};

// Implements Key(inner_matcher) for the given argument pair type.
// Key(inner_matcher) matches an std::pair whose 'first' field matches
// inner_matcher.  For example, Contains(Key(Ge(5))) can be used to match an
// std::map that contains at least one element whose key is >= 5.
template <typename PairType>
class KeyMatcherImpl : public MatcherInterface<PairType> {
 public:
  typedef GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(PairType)) RawPairType;
  typedef typename RawPairType::first_type KeyType;

  template <typename InnerMatcher>
  explicit KeyMatcherImpl(InnerMatcher inner_matcher)
      : inner_matcher_(
          testing::SafeMatcherCast<const KeyType&>(inner_matcher)) {
  }

  // Returns true iff 'key_value.first' (the key) matches the inner matcher.
  virtual bool MatchAndExplain(PairType key_value,
                               MatchResultListener* listener) const {
    StringMatchResultListener inner_listener;
    const bool match = inner_matcher_.MatchAndExplain(key_value.first,
                                                      &inner_listener);
    const internal::string explanation = inner_listener.str();
    if (explanation != "") {
      *listener << "whose first field is a value " << explanation;
    }
    return match;
  }

  // Describes what this matcher does.
  virtual void DescribeTo(::std::ostream* os) const {
    *os << "has a key that ";
    inner_matcher_.DescribeTo(os);
  }

  // Describes what the negation of this matcher does.
  virtual void DescribeNegationTo(::std::ostream* os) const {
    *os << "doesn't have a key that ";
    inner_matcher_.DescribeTo(os);
  }

 private:
  const Matcher<const KeyType&> inner_matcher_;

  GTEST_DISALLOW_ASSIGN_(KeyMatcherImpl);
};

// Implements polymorphic Key(matcher_for_key).
template <typename M>
class KeyMatcher {
 public:
  explicit KeyMatcher(M m) : matcher_for_key_(m) {}

  template <typename PairType>
  operator Matcher<PairType>() const {
    return MakeMatcher(new KeyMatcherImpl<PairType>(matcher_for_key_));
  }

 private:
  const M matcher_for_key_;

  GTEST_DISALLOW_ASSIGN_(KeyMatcher);
};

// Implements Pair(first_matcher, second_matcher) for the given argument pair
// type with its two matchers. See Pair() function below.
template <typename PairType>
class PairMatcherImpl : public MatcherInterface<PairType> {
 public:
  typedef GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(PairType)) RawPairType;
  typedef typename RawPairType::first_type FirstType;
  typedef typename RawPairType::second_type SecondType;

  template <typename FirstMatcher, typename SecondMatcher>
  PairMatcherImpl(FirstMatcher first_matcher, SecondMatcher second_matcher)
      : first_matcher_(
            testing::SafeMatcherCast<const FirstType&>(first_matcher)),
        second_matcher_(
            testing::SafeMatcherCast<const SecondType&>(second_matcher)) {
  }

  // Describes what this matcher does.
  virtual void DescribeTo(::std::ostream* os) const {
    *os << "has a first field that ";
    first_matcher_.DescribeTo(os);
    *os << ", and has a second field that ";
    second_matcher_.DescribeTo(os);
  }

  // Describes what the negation of this matcher does.
  virtual void DescribeNegationTo(::std::ostream* os) const {
    *os << "has a first field that ";
    first_matcher_.DescribeNegationTo(os);
    *os << ", or has a second field that ";
    second_matcher_.DescribeNegationTo(os);
  }

  // Returns true iff 'a_pair.first' matches first_matcher and 'a_pair.second'
  // matches second_matcher.
  virtual bool MatchAndExplain(PairType a_pair,
                               MatchResultListener* listener) const {
    if (!listener->IsInterested()) {
      // If the listener is not interested, we don't need to construct the
      // explanation.
      return first_matcher_.Matches(a_pair.first) &&
             second_matcher_.Matches(a_pair.second);
    }
    StringMatchResultListener first_inner_listener;
    if (!first_matcher_.MatchAndExplain(a_pair.first,
                                        &first_inner_listener)) {
      *listener << "whose first field does not match";
      PrintIfNotEmpty(first_inner_listener.str(), listener->stream());
      return false;
    }
    StringMatchResultListener second_inner_listener;
    if (!second_matcher_.MatchAndExplain(a_pair.second,
                                         &second_inner_listener)) {
      *listener << "whose second field does not match";
      PrintIfNotEmpty(second_inner_listener.str(), listener->stream());
      return false;
    }
    ExplainSuccess(first_inner_listener.str(), second_inner_listener.str(),
                   listener);
    return true;
  }

 private:
  void ExplainSuccess(const internal::string& first_explanation,
                      const internal::string& second_explanation,
                      MatchResultListener* listener) const {
    *listener << "whose both fields match";
    if (first_explanation != "") {
      *listener << ", where the first field is a value " << first_explanation;
    }
    if (second_explanation != "") {
      *listener << ", ";
      if (first_explanation != "") {
        *listener << "and ";
      } else {
        *listener << "where ";
      }
      *listener << "the second field is a value " << second_explanation;
    }
  }

  const Matcher<const FirstType&> first_matcher_;
  const Matcher<const SecondType&> second_matcher_;

  GTEST_DISALLOW_ASSIGN_(PairMatcherImpl);
};

// Implements polymorphic Pair(first_matcher, second_matcher).
template <typename FirstMatcher, typename SecondMatcher>
class PairMatcher {
 public:
  PairMatcher(FirstMatcher first_matcher, SecondMatcher second_matcher)
      : first_matcher_(first_matcher), second_matcher_(second_matcher) {}

  template <typename PairType>
  operator Matcher<PairType> () const {
    return MakeMatcher(
        new PairMatcherImpl<PairType>(
            first_matcher_, second_matcher_));
  }

 private:
  const FirstMatcher first_matcher_;
  const SecondMatcher second_matcher_;

  GTEST_DISALLOW_ASSIGN_(PairMatcher);
};

// Implements ElementsAre() and ElementsAreArray().
template <typename Container>
class ElementsAreMatcherImpl : public MatcherInterface<Container> {
 public:
  typedef GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(Container)) RawContainer;
  typedef internal::StlContainerView<RawContainer> View;
  typedef typename View::type StlContainer;
  typedef typename View::const_reference StlContainerReference;
  typedef typename StlContainer::value_type Element;

  // Constructs the matcher from a sequence of element values or
  // element matchers.
  template <typename InputIter>
  ElementsAreMatcherImpl(InputIter first, size_t a_count) {
    matchers_.reserve(a_count);
    InputIter it = first;
    for (size_t i = 0; i != a_count; ++i, ++it) {
      matchers_.push_back(MatcherCast<const Element&>(*it));
    }
  }

  // Describes what this matcher does.
  virtual void DescribeTo(::std::ostream* os) const {
    if (count() == 0) {
      *os << "is empty";
    } else if (count() == 1) {
      *os << "has 1 element that ";
      matchers_[0].DescribeTo(os);
    } else {
      *os << "has " << Elements(count()) << " where\n";
      for (size_t i = 0; i != count(); ++i) {
        *os << "element #" << i << " ";
        matchers_[i].DescribeTo(os);
        if (i + 1 < count()) {
          *os << ",\n";
        }
      }
    }
  }

  // Describes what the negation of this matcher does.
  virtual void DescribeNegationTo(::std::ostream* os) const {
    if (count() == 0) {
      *os << "isn't empty";
      return;
    }

    *os << "doesn't have " << Elements(count()) << ", or\n";
    for (size_t i = 0; i != count(); ++i) {
      *os << "element #" << i << " ";
      matchers_[i].DescribeNegationTo(os);
      if (i + 1 < count()) {
        *os << ", or\n";
      }
    }
  }

  virtual bool MatchAndExplain(Container container,
                               MatchResultListener* listener) const {
    StlContainerReference stl_container = View::ConstReference(container);
    const size_t actual_count = stl_container.size();
    if (actual_count != count()) {
      // The element count doesn't match.  If the container is empty,
      // there's no need to explain anything as Google Mock already
      // prints the empty container.  Otherwise we just need to show
      // how many elements there actually are.
      if (actual_count != 0) {
        *listener << "which has " << Elements(actual_count);
      }
      return false;
    }

    typename StlContainer::const_iterator it = stl_container.begin();
    // explanations[i] is the explanation of the element at index i.
    std::vector<internal::string> explanations(count());
    for (size_t i = 0; i != count();  ++it, ++i) {
      StringMatchResultListener s;
      if (matchers_[i].MatchAndExplain(*it, &s)) {
        explanations[i] = s.str();
      } else {
        // The container has the right size but the i-th element
        // doesn't match its expectation.
        *listener << "whose element #" << i << " doesn't match";
        PrintIfNotEmpty(s.str(), listener->stream());
        return false;
      }
    }

    // Every element matches its expectation.  We need to explain why
    // (the obvious ones can be skipped).
    bool reason_printed = false;
    for (size_t i = 0; i != count(); ++i) {
      const internal::string& s = explanations[i];
      if (!s.empty()) {
        if (reason_printed) {
          *listener << ",\nand ";
        }
        *listener << "whose element #" << i << " matches, " << s;
        reason_printed = true;
      }
    }

    return true;
  }

 private:
  static Message Elements(size_t count) {
    return Message() << count << (count == 1 ? " element" : " elements");
  }

  size_t count() const { return matchers_.size(); }
  std::vector<Matcher<const Element&> > matchers_;

  GTEST_DISALLOW_ASSIGN_(ElementsAreMatcherImpl);
};

// Implements ElementsAre() of 0 arguments.
class ElementsAreMatcher0 {
 public:
  ElementsAreMatcher0() {}

  template <typename Container>
  operator Matcher<Container>() const {
    typedef GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(Container))
        RawContainer;
    typedef typename internal::StlContainerView<RawContainer>::type::value_type
        Element;

    const Matcher<const Element&>* const matchers = NULL;
    return MakeMatcher(new ElementsAreMatcherImpl<Container>(matchers, 0));
  }
};

// Implements ElementsAreArray().
template <typename T>
class ElementsAreArrayMatcher {
 public:
  ElementsAreArrayMatcher(const T* first, size_t count) :
      first_(first), count_(count) {}

  template <typename Container>
  operator Matcher<Container>() const {
    typedef GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(Container))
        RawContainer;
    typedef typename internal::StlContainerView<RawContainer>::type::value_type
        Element;

    return MakeMatcher(new ElementsAreMatcherImpl<Container>(first_, count_));
  }

 private:
  const T* const first_;
  const size_t count_;

  GTEST_DISALLOW_ASSIGN_(ElementsAreArrayMatcher);
};

// Constants denoting interpolations in a matcher description string.
const int kTupleInterpolation = -1;    // "%(*)s"
const int kPercentInterpolation = -2;  // "%%"
const int kInvalidInterpolation = -3;  // "%" followed by invalid text

// Records the location and content of an interpolation.
struct Interpolation {
  Interpolation(const char* start, const char* end, int param)
      : start_pos(start), end_pos(end), param_index(param) {}

  // Points to the start of the interpolation (the '%' character).
  const char* start_pos;
  // Points to the first character after the interpolation.
  const char* end_pos;
  // 0-based index of the interpolated matcher parameter;
  // kTupleInterpolation for "%(*)s"; kPercentInterpolation for "%%".
  int param_index;
};

typedef ::std::vector<Interpolation> Interpolations;

// Parses a matcher description string and returns a vector of
// interpolations that appear in the string; generates non-fatal
// failures iff 'description' is an invalid matcher description.
// 'param_names' is a NULL-terminated array of parameter names in the
// order they appear in the MATCHER_P*() parameter list.
Interpolations ValidateMatcherDescription(
    const char* param_names[], const char* description);

// Returns the actual matcher description, given the matcher name,
// user-supplied description template string, interpolations in the
// string, and the printed values of the matcher parameters.
string FormatMatcherDescription(
    const char* matcher_name, const char* description,
    const Interpolations& interp, const Strings& param_values);

}  // namespace internal

// Implements MatcherCast().
template <typename T, typename M>
inline Matcher<T> MatcherCast(M matcher) {
  return internal::MatcherCastImpl<T, M>::Cast(matcher);
}

// _ is a matcher that matches anything of any type.
//
// This definition is fine as:
//
//   1. The C++ standard permits using the name _ in a namespace that
//      is not the global namespace or ::std.
//   2. The AnythingMatcher class has no data member or constructor,
//      so it's OK to create global variables of this type.
//   3. c-style has approved of using _ in this case.
const internal::AnythingMatcher _ = {};
// Creates a matcher that matches any value of the given type T.
template <typename T>
inline Matcher<T> A() { return MakeMatcher(new internal::AnyMatcherImpl<T>()); }

// Creates a matcher that matches any value of the given type T.
template <typename T>
inline Matcher<T> An() { return A<T>(); }

// Creates a polymorphic matcher that matches anything equal to x.
// Note: if the parameter of Eq() were declared as const T&, Eq("foo")
// wouldn't compile.
template <typename T>
inline internal::EqMatcher<T> Eq(T x) { return internal::EqMatcher<T>(x); }

// Constructs a Matcher<T> from a 'value' of type T.  The constructed
// matcher matches any value that's equal to 'value'.
template <typename T>
Matcher<T>::Matcher(T value) { *this = Eq(value); }

// Creates a monomorphic matcher that matches anything with type Lhs
// and equal to rhs.  A user may need to use this instead of Eq(...)
// in order to resolve an overloading ambiguity.
//
// TypedEq<T>(x) is just a convenient short-hand for Matcher<T>(Eq(x))
// or Matcher<T>(x), but more readable than the latter.
//
// We could define similar monomorphic matchers for other comparison
// operations (e.g. TypedLt, TypedGe, and etc), but decided not to do
// it yet as those are used much less than Eq() in practice.  A user
// can always write Matcher<T>(Lt(5)) to be explicit about the type,
// for example.
template <typename Lhs, typename Rhs>
inline Matcher<Lhs> TypedEq(const Rhs& rhs) { return Eq(rhs); }

// Creates a polymorphic matcher that matches anything >= x.
template <typename Rhs>
inline internal::GeMatcher<Rhs> Ge(Rhs x) {
  return internal::GeMatcher<Rhs>(x);
}

// Creates a polymorphic matcher that matches anything > x.
template <typename Rhs>
inline internal::GtMatcher<Rhs> Gt(Rhs x) {
  return internal::GtMatcher<Rhs>(x);
}

// Creates a polymorphic matcher that matches anything <= x.
template <typename Rhs>
inline internal::LeMatcher<Rhs> Le(Rhs x) {
  return internal::LeMatcher<Rhs>(x);
}

// Creates a polymorphic matcher that matches anything < x.
template <typename Rhs>
inline internal::LtMatcher<Rhs> Lt(Rhs x) {
  return internal::LtMatcher<Rhs>(x);
}

// Creates a polymorphic matcher that matches anything != x.
template <typename Rhs>
inline internal::NeMatcher<Rhs> Ne(Rhs x) {
  return internal::NeMatcher<Rhs>(x);
}

// Creates a polymorphic matcher that matches any NULL pointer.
inline PolymorphicMatcher<internal::IsNullMatcher > IsNull() {
  return MakePolymorphicMatcher(internal::IsNullMatcher());
}

// Creates a polymorphic matcher that matches any non-NULL pointer.
// This is convenient as Not(NULL) doesn't compile (the compiler
// thinks that that expression is comparing a pointer with an integer).
inline PolymorphicMatcher<internal::NotNullMatcher > NotNull() {
  return MakePolymorphicMatcher(internal::NotNullMatcher());
}

// Creates a polymorphic matcher that matches any argument that
// references variable x.
template <typename T>
inline internal::RefMatcher<T&> Ref(T& x) {  // NOLINT
  return internal::RefMatcher<T&>(x);
}

// Creates a matcher that matches any double argument approximately
// equal to rhs, where two NANs are considered unequal.
inline internal::FloatingEqMatcher<double> DoubleEq(double rhs) {
  return internal::FloatingEqMatcher<double>(rhs, false);
}

// Creates a matcher that matches any double argument approximately
// equal to rhs, including NaN values when rhs is NaN.
inline internal::FloatingEqMatcher<double> NanSensitiveDoubleEq(double rhs) {
  return internal::FloatingEqMatcher<double>(rhs, true);
}

// Creates a matcher that matches any float argument approximately
// equal to rhs, where two NANs are considered unequal.
inline internal::FloatingEqMatcher<float> FloatEq(float rhs) {
  return internal::FloatingEqMatcher<float>(rhs, false);
}

// Creates a matcher that matches any double argument approximately
// equal to rhs, including NaN values when rhs is NaN.
inline internal::FloatingEqMatcher<float> NanSensitiveFloatEq(float rhs) {
  return internal::FloatingEqMatcher<float>(rhs, true);
}

// Creates a matcher that matches a pointer (raw or smart) that points
// to a value that matches inner_matcher.
template <typename InnerMatcher>
inline internal::PointeeMatcher<InnerMatcher> Pointee(
    const InnerMatcher& inner_matcher) {
  return internal::PointeeMatcher<InnerMatcher>(inner_matcher);
}

// Creates a matcher that matches an object whose given field matches
// 'matcher'.  For example,
//   Field(&Foo::number, Ge(5))
// matches a Foo object x iff x.number >= 5.
template <typename Class, typename FieldType, typename FieldMatcher>
inline PolymorphicMatcher<
  internal::FieldMatcher<Class, FieldType> > Field(
    FieldType Class::*field, const FieldMatcher& matcher) {
  return MakePolymorphicMatcher(
      internal::FieldMatcher<Class, FieldType>(
          field, MatcherCast<const FieldType&>(matcher)));
  // The call to MatcherCast() is required for supporting inner
  // matchers of compatible types.  For example, it allows
  //   Field(&Foo::bar, m)
  // to compile where bar is an int32 and m is a matcher for int64.
}

// Creates a matcher that matches an object whose given property
// matches 'matcher'.  For example,
//   Property(&Foo::str, StartsWith("hi"))
// matches a Foo object x iff x.str() starts with "hi".
template <typename Class, typename PropertyType, typename PropertyMatcher>
inline PolymorphicMatcher<
  internal::PropertyMatcher<Class, PropertyType> > Property(
    PropertyType (Class::*property)() const, const PropertyMatcher& matcher) {
  return MakePolymorphicMatcher(
      internal::PropertyMatcher<Class, PropertyType>(
          property,
          MatcherCast<GMOCK_REFERENCE_TO_CONST_(PropertyType)>(matcher)));
  // The call to MatcherCast() is required for supporting inner
  // matchers of compatible types.  For example, it allows
  //   Property(&Foo::bar, m)
  // to compile where bar() returns an int32 and m is a matcher for int64.
}

// Creates a matcher that matches an object iff the result of applying
// a callable to x matches 'matcher'.
// For example,
//   ResultOf(f, StartsWith("hi"))
// matches a Foo object x iff f(x) starts with "hi".
// callable parameter can be a function, function pointer, or a functor.
// Callable has to satisfy the following conditions:
//   * It is required to keep no state affecting the results of
//     the calls on it and make no assumptions about how many calls
//     will be made. Any state it keeps must be protected from the
//     concurrent access.
//   * If it is a function object, it has to define type result_type.
//     We recommend deriving your functor classes from std::unary_function.
template <typename Callable, typename ResultOfMatcher>
internal::ResultOfMatcher<Callable> ResultOf(
    Callable callable, const ResultOfMatcher& matcher) {
  return internal::ResultOfMatcher<Callable>(
          callable,
          MatcherCast<typename internal::CallableTraits<Callable>::ResultType>(
              matcher));
  // The call to MatcherCast() is required for supporting inner
  // matchers of compatible types.  For example, it allows
  //   ResultOf(Function, m)
  // to compile where Function() returns an int32 and m is a matcher for int64.
}

// String matchers.

// Matches a string equal to str.
inline PolymorphicMatcher<internal::StrEqualityMatcher<internal::string> >
    StrEq(const internal::string& str) {
  return MakePolymorphicMatcher(internal::StrEqualityMatcher<internal::string>(
      str, true, true));
}

// Matches a string not equal to str.
inline PolymorphicMatcher<internal::StrEqualityMatcher<internal::string> >
    StrNe(const internal::string& str) {
  return MakePolymorphicMatcher(internal::StrEqualityMatcher<internal::string>(
      str, false, true));
}

// Matches a string equal to str, ignoring case.
inline PolymorphicMatcher<internal::StrEqualityMatcher<internal::string> >
    StrCaseEq(const internal::string& str) {
  return MakePolymorphicMatcher(internal::StrEqualityMatcher<internal::string>(
      str, true, false));
}

// Matches a string not equal to str, ignoring case.
inline PolymorphicMatcher<internal::StrEqualityMatcher<internal::string> >
    StrCaseNe(const internal::string& str) {
  return MakePolymorphicMatcher(internal::StrEqualityMatcher<internal::string>(
      str, false, false));
}

// Creates a matcher that matches any string, std::string, or C string
// that contains the given substring.
inline PolymorphicMatcher<internal::HasSubstrMatcher<internal::string> >
    HasSubstr(const internal::string& substring) {
  return MakePolymorphicMatcher(internal::HasSubstrMatcher<internal::string>(
      substring));
}

// Matches a string that starts with 'prefix' (case-sensitive).
inline PolymorphicMatcher<internal::StartsWithMatcher<internal::string> >
    StartsWith(const internal::string& prefix) {
  return MakePolymorphicMatcher(internal::StartsWithMatcher<internal::string>(
      prefix));
}

// Matches a string that ends with 'suffix' (case-sensitive).
inline PolymorphicMatcher<internal::EndsWithMatcher<internal::string> >
    EndsWith(const internal::string& suffix) {
  return MakePolymorphicMatcher(internal::EndsWithMatcher<internal::string>(
      suffix));
}

// Matches a string that fully matches regular expression 'regex'.
// The matcher takes ownership of 'regex'.
inline PolymorphicMatcher<internal::MatchesRegexMatcher> MatchesRegex(
    const internal::RE* regex) {
  return MakePolymorphicMatcher(internal::MatchesRegexMatcher(regex, true));
}
inline PolymorphicMatcher<internal::MatchesRegexMatcher> MatchesRegex(
    const internal::string& regex) {
  return MatchesRegex(new internal::RE(regex));
}

// Matches a string that contains regular expression 'regex'.
// The matcher takes ownership of 'regex'.
inline PolymorphicMatcher<internal::MatchesRegexMatcher> ContainsRegex(
    const internal::RE* regex) {
  return MakePolymorphicMatcher(internal::MatchesRegexMatcher(regex, false));
}
inline PolymorphicMatcher<internal::MatchesRegexMatcher> ContainsRegex(
    const internal::string& regex) {
  return ContainsRegex(new internal::RE(regex));
}

#if GTEST_HAS_GLOBAL_WSTRING || GTEST_HAS_STD_WSTRING
// Wide string matchers.

// Matches a string equal to str.
inline PolymorphicMatcher<internal::StrEqualityMatcher<internal::wstring> >
    StrEq(const internal::wstring& str) {
  return MakePolymorphicMatcher(internal::StrEqualityMatcher<internal::wstring>(
      str, true, true));
}

// Matches a string not equal to str.
inline PolymorphicMatcher<internal::StrEqualityMatcher<internal::wstring> >
    StrNe(const internal::wstring& str) {
  return MakePolymorphicMatcher(internal::StrEqualityMatcher<internal::wstring>(
      str, false, true));
}

// Matches a string equal to str, ignoring case.
inline PolymorphicMatcher<internal::StrEqualityMatcher<internal::wstring> >
    StrCaseEq(const internal::wstring& str) {
  return MakePolymorphicMatcher(internal::StrEqualityMatcher<internal::wstring>(
      str, true, false));
}

// Matches a string not equal to str, ignoring case.
inline PolymorphicMatcher<internal::StrEqualityMatcher<internal::wstring> >
    StrCaseNe(const internal::wstring& str) {
  return MakePolymorphicMatcher(internal::StrEqualityMatcher<internal::wstring>(
      str, false, false));
}

// Creates a matcher that matches any wstring, std::wstring, or C wide string
// that contains the given substring.
inline PolymorphicMatcher<internal::HasSubstrMatcher<internal::wstring> >
    HasSubstr(const internal::wstring& substring) {
  return MakePolymorphicMatcher(internal::HasSubstrMatcher<internal::wstring>(
      substring));
}

// Matches a string that starts with 'prefix' (case-sensitive).
inline PolymorphicMatcher<internal::StartsWithMatcher<internal::wstring> >
    StartsWith(const internal::wstring& prefix) {
  return MakePolymorphicMatcher(internal::StartsWithMatcher<internal::wstring>(
      prefix));
}

// Matches a string that ends with 'suffix' (case-sensitive).
inline PolymorphicMatcher<internal::EndsWithMatcher<internal::wstring> >
    EndsWith(const internal::wstring& suffix) {
  return MakePolymorphicMatcher(internal::EndsWithMatcher<internal::wstring>(
      suffix));
}

#endif  // GTEST_HAS_GLOBAL_WSTRING || GTEST_HAS_STD_WSTRING

// Creates a polymorphic matcher that matches a 2-tuple where the
// first field == the second field.
inline internal::Eq2Matcher Eq() { return internal::Eq2Matcher(); }

// Creates a polymorphic matcher that matches a 2-tuple where the
// first field >= the second field.
inline internal::Ge2Matcher Ge() { return internal::Ge2Matcher(); }

// Creates a polymorphic matcher that matches a 2-tuple where the
// first field > the second field.
inline internal::Gt2Matcher Gt() { return internal::Gt2Matcher(); }

// Creates a polymorphic matcher that matches a 2-tuple where the
// first field <= the second field.
inline internal::Le2Matcher Le() { return internal::Le2Matcher(); }

// Creates a polymorphic matcher that matches a 2-tuple where the
// first field < the second field.
inline internal::Lt2Matcher Lt() { return internal::Lt2Matcher(); }

// Creates a polymorphic matcher that matches a 2-tuple where the
// first field != the second field.
inline internal::Ne2Matcher Ne() { return internal::Ne2Matcher(); }

// Creates a matcher that matches any value of type T that m doesn't
// match.
template <typename InnerMatcher>
inline internal::NotMatcher<InnerMatcher> Not(InnerMatcher m) {
  return internal::NotMatcher<InnerMatcher>(m);
}

// Creates a matcher that matches any value that matches all of the
// given matchers.
//
// For now we only support up to 5 matchers.  Support for more
// matchers can be added as needed, or the user can use nested
// AllOf()s.
template <typename Matcher1, typename Matcher2>
inline internal::BothOfMatcher<Matcher1, Matcher2>
AllOf(Matcher1 m1, Matcher2 m2) {
  return internal::BothOfMatcher<Matcher1, Matcher2>(m1, m2);
}

template <typename Matcher1, typename Matcher2, typename Matcher3>
inline internal::BothOfMatcher<Matcher1,
           internal::BothOfMatcher<Matcher2, Matcher3> >
AllOf(Matcher1 m1, Matcher2 m2, Matcher3 m3) {
  return AllOf(m1, AllOf(m2, m3));
}

template <typename Matcher1, typename Matcher2, typename Matcher3,
          typename Matcher4>
inline internal::BothOfMatcher<Matcher1,
           internal::BothOfMatcher<Matcher2,
               internal::BothOfMatcher<Matcher3, Matcher4> > >
AllOf(Matcher1 m1, Matcher2 m2, Matcher3 m3, Matcher4 m4) {
  return AllOf(m1, AllOf(m2, m3, m4));
}

template <typename Matcher1, typename Matcher2, typename Matcher3,
          typename Matcher4, typename Matcher5>
inline internal::BothOfMatcher<Matcher1,
           internal::BothOfMatcher<Matcher2,
               internal::BothOfMatcher<Matcher3,
                   internal::BothOfMatcher<Matcher4, Matcher5> > > >
AllOf(Matcher1 m1, Matcher2 m2, Matcher3 m3, Matcher4 m4, Matcher5 m5) {
  return AllOf(m1, AllOf(m2, m3, m4, m5));
}

// Creates a matcher that matches any value that matches at least one
// of the given matchers.
//
// For now we only support up to 5 matchers.  Support for more
// matchers can be added as needed, or the user can use nested
// AnyOf()s.
template <typename Matcher1, typename Matcher2>
inline internal::EitherOfMatcher<Matcher1, Matcher2>
AnyOf(Matcher1 m1, Matcher2 m2) {
  return internal::EitherOfMatcher<Matcher1, Matcher2>(m1, m2);
}

template <typename Matcher1, typename Matcher2, typename Matcher3>
inline internal::EitherOfMatcher<Matcher1,
           internal::EitherOfMatcher<Matcher2, Matcher3> >
AnyOf(Matcher1 m1, Matcher2 m2, Matcher3 m3) {
  return AnyOf(m1, AnyOf(m2, m3));
}

template <typename Matcher1, typename Matcher2, typename Matcher3,
          typename Matcher4>
inline internal::EitherOfMatcher<Matcher1,
           internal::EitherOfMatcher<Matcher2,
               internal::EitherOfMatcher<Matcher3, Matcher4> > >
AnyOf(Matcher1 m1, Matcher2 m2, Matcher3 m3, Matcher4 m4) {
  return AnyOf(m1, AnyOf(m2, m3, m4));
}

template <typename Matcher1, typename Matcher2, typename Matcher3,
          typename Matcher4, typename Matcher5>
inline internal::EitherOfMatcher<Matcher1,
           internal::EitherOfMatcher<Matcher2,
               internal::EitherOfMatcher<Matcher3,
                   internal::EitherOfMatcher<Matcher4, Matcher5> > > >
AnyOf(Matcher1 m1, Matcher2 m2, Matcher3 m3, Matcher4 m4, Matcher5 m5) {
  return AnyOf(m1, AnyOf(m2, m3, m4, m5));
}

// Returns a matcher that matches anything that satisfies the given
// predicate.  The predicate can be any unary function or functor
// whose return type can be implicitly converted to bool.
template <typename Predicate>
inline PolymorphicMatcher<internal::TrulyMatcher<Predicate> >
Truly(Predicate pred) {
  return MakePolymorphicMatcher(internal::TrulyMatcher<Predicate>(pred));
}

// Returns a matcher that matches an equal container.
// This matcher behaves like Eq(), but in the event of mismatch lists the
// values that are included in one container but not the other. (Duplicate
// values and order differences are not explained.)
template <typename Container>
inline PolymorphicMatcher<internal::ContainerEqMatcher<  // NOLINT
                            GMOCK_REMOVE_CONST_(Container)> >
    ContainerEq(const Container& rhs) {
  // This following line is for working around a bug in MSVC 8.0,
  // which causes Container to be a const type sometimes.
  typedef GMOCK_REMOVE_CONST_(Container) RawContainer;
  return MakePolymorphicMatcher(
      internal::ContainerEqMatcher<RawContainer>(rhs));
}

// Matches an STL-style container or a native array that contains at
// least one element matching the given value or matcher.
//
// Examples:
//   ::std::set<int> page_ids;
//   page_ids.insert(3);
//   page_ids.insert(1);
//   EXPECT_THAT(page_ids, Contains(1));
//   EXPECT_THAT(page_ids, Contains(Gt(2)));
//   EXPECT_THAT(page_ids, Not(Contains(4)));
//
//   ::std::map<int, size_t> page_lengths;
//   page_lengths[1] = 100;
//   EXPECT_THAT(page_lengths,
//               Contains(::std::pair<const int, size_t>(1, 100)));
//
//   const char* user_ids[] = { "joe", "mike", "tom" };
//   EXPECT_THAT(user_ids, Contains(Eq(::std::string("tom"))));
template <typename M>
inline internal::ContainsMatcher<M> Contains(M matcher) {
  return internal::ContainsMatcher<M>(matcher);
}

// Key(inner_matcher) matches an std::pair whose 'first' field matches
// inner_matcher.  For example, Contains(Key(Ge(5))) can be used to match an
// std::map that contains at least one element whose key is >= 5.
template <typename M>
inline internal::KeyMatcher<M> Key(M inner_matcher) {
  return internal::KeyMatcher<M>(inner_matcher);
}

// Pair(first_matcher, second_matcher) matches a std::pair whose 'first' field
// matches first_matcher and whose 'second' field matches second_matcher.  For
// example, EXPECT_THAT(map_type, ElementsAre(Pair(Ge(5), "foo"))) can be used
// to match a std::map<int, string> that contains exactly one element whose key
// is >= 5 and whose value equals "foo".
template <typename FirstMatcher, typename SecondMatcher>
inline internal::PairMatcher<FirstMatcher, SecondMatcher>
Pair(FirstMatcher first_matcher, SecondMatcher second_matcher) {
  return internal::PairMatcher<FirstMatcher, SecondMatcher>(
      first_matcher, second_matcher);
}

// Returns a predicate that is satisfied by anything that matches the
// given matcher.
template <typename M>
inline internal::MatcherAsPredicate<M> Matches(M matcher) {
  return internal::MatcherAsPredicate<M>(matcher);
}

// Returns true iff the value matches the matcher.
template <typename T, typename M>
inline bool Value(const T& value, M matcher) {
  return testing::Matches(matcher)(value);
}

// Matches the value against the given matcher and explains the match
// result to listener.
template <typename T, typename M>
inline bool ExplainMatchResult(
    M matcher, const T& value, MatchResultListener* listener) {
  return SafeMatcherCast<const T&>(matcher).MatchAndExplain(value, listener);
}

// AllArgs(m) is a synonym of m.  This is useful in
//
//   EXPECT_CALL(foo, Bar(_, _)).With(AllArgs(Eq()));
//
// which is easier to read than
//
//   EXPECT_CALL(foo, Bar(_, _)).With(Eq());
template <typename InnerMatcher>
inline InnerMatcher AllArgs(const InnerMatcher& matcher) { return matcher; }

// These macros allow using matchers to check values in Google Test
// tests.  ASSERT_THAT(value, matcher) and EXPECT_THAT(value, matcher)
// succeed iff the value matches the matcher.  If the assertion fails,
// the value and the description of the matcher will be printed.
#define ASSERT_THAT(value, matcher) ASSERT_PRED_FORMAT1(\
    ::testing::internal::MakePredicateFormatterFromMatcher(matcher), value)
#define EXPECT_THAT(value, matcher) EXPECT_PRED_FORMAT1(\
    ::testing::internal::MakePredicateFormatterFromMatcher(matcher), value)

}  // namespace testing

#endif  // GMOCK_INCLUDE_GMOCK_GMOCK_MATCHERS_H_

namespace testing {

// An abstract handle of an expectation.
class Expectation;

// A set of expectation handles.
class ExpectationSet;

// Anything inside the 'internal' namespace IS INTERNAL IMPLEMENTATION
// and MUST NOT BE USED IN USER CODE!!!
namespace internal {

// Implements a mock function.
template <typename F> class FunctionMocker;

// Base class for expectations.
class ExpectationBase;

// Implements an expectation.
template <typename F> class TypedExpectation;

// Helper class for testing the Expectation class template.
class ExpectationTester;

// Base class for function mockers.
template <typename F> class FunctionMockerBase;

// Protects the mock object registry (in class Mock), all function
// mockers, and all expectations.
//
// The reason we don't use more fine-grained protection is: when a
// mock function Foo() is called, it needs to consult its expectations
// to see which one should be picked.  If another thread is allowed to
// call a mock function (either Foo() or a different one) at the same
// time, it could affect the "retired" attributes of Foo()'s
// expectations when InSequence() is used, and thus affect which
// expectation gets picked.  Therefore, we sequence all mock function
// calls to ensure the integrity of the mock objects' states.
GTEST_DECLARE_STATIC_MUTEX_(g_gmock_mutex);

// Abstract base class of FunctionMockerBase.  This is the
// type-agnostic part of the function mocker interface.  Its pure
// virtual methods are implemented by FunctionMockerBase.
class UntypedFunctionMockerBase {
 public:
  virtual ~UntypedFunctionMockerBase() {}

  // Verifies that all expectations on this mock function have been
  // satisfied.  Reports one or more Google Test non-fatal failures
  // and returns false if not.
  // L >= g_gmock_mutex
  virtual bool VerifyAndClearExpectationsLocked() = 0;

  // Clears the ON_CALL()s set on this mock function.
  // L >= g_gmock_mutex
  virtual void ClearDefaultActionsLocked() = 0;
};  // class UntypedFunctionMockerBase

// This template class implements a default action spec (i.e. an
// ON_CALL() statement).
template <typename F>
class DefaultActionSpec {
 public:
  typedef typename Function<F>::ArgumentTuple ArgumentTuple;
  typedef typename Function<F>::ArgumentMatcherTuple ArgumentMatcherTuple;

  // Constructs a DefaultActionSpec object from the information inside
  // the parenthesis of an ON_CALL() statement.
  DefaultActionSpec(const char* a_file, int a_line,
                    const ArgumentMatcherTuple& matchers)
      : file_(a_file),
        line_(a_line),
        matchers_(matchers),
        // By default, extra_matcher_ should match anything.  However,
        // we cannot initialize it with _ as that triggers a compiler
        // bug in Symbian's C++ compiler (cannot decide between two
        // overloaded constructors of Matcher<const ArgumentTuple&>).
        extra_matcher_(A<const ArgumentTuple&>()),
        last_clause_(kNone) {
  }

  // Where in the source file was the default action spec defined?
  const char* file() const { return file_; }
  int line() const { return line_; }

  // Implements the .With() clause.
  DefaultActionSpec& With(const Matcher<const ArgumentTuple&>& m) {
    // Makes sure this is called at most once.
    ExpectSpecProperty(last_clause_ < kWith,
                       ".With() cannot appear "
                       "more than once in an ON_CALL().");
    last_clause_ = kWith;

    extra_matcher_ = m;
    return *this;
  }

  // Implements the .WillByDefault() clause.
  DefaultActionSpec& WillByDefault(const Action<F>& action) {
    ExpectSpecProperty(last_clause_ < kWillByDefault,
                       ".WillByDefault() must appear "
                       "exactly once in an ON_CALL().");
    last_clause_ = kWillByDefault;

    ExpectSpecProperty(!action.IsDoDefault(),
                       "DoDefault() cannot be used in ON_CALL().");
    action_ = action;
    return *this;
  }

  // Returns true iff the given arguments match the matchers.
  bool Matches(const ArgumentTuple& args) const {
    return TupleMatches(matchers_, args) && extra_matcher_.Matches(args);
  }

  // Returns the action specified by the user.
  const Action<F>& GetAction() const {
    AssertSpecProperty(last_clause_ == kWillByDefault,
                       ".WillByDefault() must appear exactly "
                       "once in an ON_CALL().");
    return action_;
  }

 private:
  // Gives each clause in the ON_CALL() statement a name.
  enum Clause {
    // Do not change the order of the enum members!  The run-time
    // syntax checking relies on it.
    kNone,
    kWith,
    kWillByDefault,
  };

  // Asserts that the ON_CALL() statement has a certain property.
  void AssertSpecProperty(bool property, const string& failure_message) const {
    Assert(property, file_, line_, failure_message);
  }

  // Expects that the ON_CALL() statement has a certain property.
  void ExpectSpecProperty(bool property, const string& failure_message) const {
    Expect(property, file_, line_, failure_message);
  }

  // The information in statement
  //
  //   ON_CALL(mock_object, Method(matchers))
  //       .With(multi-argument-matcher)
  //       .WillByDefault(action);
  //
  // is recorded in the data members like this:
  //
  //   source file that contains the statement => file_
  //   line number of the statement            => line_
  //   matchers                                => matchers_
  //   multi-argument-matcher                  => extra_matcher_
  //   action                                  => action_
  const char* file_;
  int line_;
  ArgumentMatcherTuple matchers_;
  Matcher<const ArgumentTuple&> extra_matcher_;
  Action<F> action_;

  // The last clause in the ON_CALL() statement as seen so far.
  // Initially kNone and changes as the statement is parsed.
  Clause last_clause_;
};  // class DefaultActionSpec

// Possible reactions on uninteresting calls.  TODO(wan@google.com):
// rename the enum values to the kFoo style.
enum CallReaction {
  ALLOW,
  WARN,
  FAIL,
};

}  // namespace internal

// Utilities for manipulating mock objects.
class Mock {
 public:
  // The following public methods can be called concurrently.

  // Tells Google Mock to ignore mock_obj when checking for leaked
  // mock objects.
  static void AllowLeak(const void* mock_obj);

  // Verifies and clears all expectations on the given mock object.
  // If the expectations aren't satisfied, generates one or more
  // Google Test non-fatal failures and returns false.
  static bool VerifyAndClearExpectations(void* mock_obj);

  // Verifies all expectations on the given mock object and clears its
  // default actions and expectations.  Returns true iff the
  // verification was successful.
  static bool VerifyAndClear(void* mock_obj);
 private:
  // Needed for a function mocker to register itself (so that we know
  // how to clear a mock object).
  template <typename F>
  friend class internal::FunctionMockerBase;

  template <typename M>
  friend class NiceMock;

  template <typename M>
  friend class StrictMock;

  // Tells Google Mock to allow uninteresting calls on the given mock
  // object.
  // L < g_gmock_mutex
  static void AllowUninterestingCalls(const void* mock_obj);

  // Tells Google Mock to warn the user about uninteresting calls on
  // the given mock object.
  // L < g_gmock_mutex
  static void WarnUninterestingCalls(const void* mock_obj);

  // Tells Google Mock to fail uninteresting calls on the given mock
  // object.
  // L < g_gmock_mutex
  static void FailUninterestingCalls(const void* mock_obj);

  // Tells Google Mock the given mock object is being destroyed and
  // its entry in the call-reaction table should be removed.
  // L < g_gmock_mutex
  static void UnregisterCallReaction(const void* mock_obj);

  // Returns the reaction Google Mock will have on uninteresting calls
  // made on the given mock object.
  // L < g_gmock_mutex
  static internal::CallReaction GetReactionOnUninterestingCalls(
      const void* mock_obj);

  // Verifies that all expectations on the given mock object have been
  // satisfied.  Reports one or more Google Test non-fatal failures
  // and returns false if not.
  // L >= g_gmock_mutex
  static bool VerifyAndClearExpectationsLocked(void* mock_obj);

  // Clears all ON_CALL()s set on the given mock object.
  // L >= g_gmock_mutex
  static void ClearDefaultActionsLocked(void* mock_obj);

  // Registers a mock object and a mock method it owns.
  // L < g_gmock_mutex
  static void Register(const void* mock_obj,
                       internal::UntypedFunctionMockerBase* mocker);

  // Tells Google Mock where in the source code mock_obj is used in an
  // ON_CALL or EXPECT_CALL.  In case mock_obj is leaked, this
  // information helps the user identify which object it is.
  // L < g_gmock_mutex
  static void RegisterUseByOnCallOrExpectCall(
      const void* mock_obj, const char* file, int line);

  // Unregisters a mock method; removes the owning mock object from
  // the registry when the last mock method associated with it has
  // been unregistered.  This is called only in the destructor of
  // FunctionMockerBase.
  // L >= g_gmock_mutex
  static void UnregisterLocked(internal::UntypedFunctionMockerBase* mocker);
};  // class Mock

// An abstract handle of an expectation.  Useful in the .After()
// clause of EXPECT_CALL() for setting the (partial) order of
// expectations.  The syntax:
//
//   Expectation e1 = EXPECT_CALL(...)...;
//   EXPECT_CALL(...).After(e1)...;
//
// sets two expectations where the latter can only be matched after
// the former has been satisfied.
//
// Notes:
//   - This class is copyable and has value semantics.
//   - Constness is shallow: a const Expectation object itself cannot
//     be modified, but the mutable methods of the ExpectationBase
//     object it references can be called via expectation_base().
//   - The constructors and destructor are defined out-of-line because
//     the Symbian WINSCW compiler wants to otherwise instantiate them
//     when it sees this class definition, at which point it doesn't have
//     ExpectationBase available yet, leading to incorrect destruction
//     in the linked_ptr (or compilation errors if using a checking
//     linked_ptr).
class Expectation {
 public:
  // Constructs a null object that doesn't reference any expectation.
  Expectation();

  ~Expectation();

  // This single-argument ctor must not be explicit, in order to support the
  //   Expectation e = EXPECT_CALL(...);
  // syntax.
  //
  // A TypedExpectation object stores its pre-requisites as
  // Expectation objects, and needs to call the non-const Retire()
  // method on the ExpectationBase objects they reference.  Therefore
  // Expectation must receive a *non-const* reference to the
  // ExpectationBase object.
  Expectation(internal::ExpectationBase& exp);  // NOLINT

  // The compiler-generated copy ctor and operator= work exactly as
  // intended, so we don't need to define our own.

  // Returns true iff rhs references the same expectation as this object does.
  bool operator==(const Expectation& rhs) const {
    return expectation_base_ == rhs.expectation_base_;
  }

  bool operator!=(const Expectation& rhs) const { return !(*this == rhs); }

 private:
  friend class ExpectationSet;
  friend class Sequence;
  friend class ::testing::internal::ExpectationBase;

  template <typename F>
  friend class ::testing::internal::FunctionMockerBase;

  template <typename F>
  friend class ::testing::internal::TypedExpectation;

  // This comparator is needed for putting Expectation objects into a set.
  class Less {
   public:
    bool operator()(const Expectation& lhs, const Expectation& rhs) const {
      return lhs.expectation_base_.get() < rhs.expectation_base_.get();
    }
  };

  typedef ::std::set<Expectation, Less> Set;

  Expectation(
      const internal::linked_ptr<internal::ExpectationBase>& expectation_base);

  // Returns the expectation this object references.
  const internal::linked_ptr<internal::ExpectationBase>&
  expectation_base() const {
    return expectation_base_;
  }

  // A linked_ptr that co-owns the expectation this handle references.
  internal::linked_ptr<internal::ExpectationBase> expectation_base_;
};

// A set of expectation handles.  Useful in the .After() clause of
// EXPECT_CALL() for setting the (partial) order of expectations.  The
// syntax:
//
//   ExpectationSet es;
//   es += EXPECT_CALL(...)...;
//   es += EXPECT_CALL(...)...;
//   EXPECT_CALL(...).After(es)...;
//
// sets three expectations where the last one can only be matched
// after the first two have both been satisfied.
//
// This class is copyable and has value semantics.
class ExpectationSet {
 public:
  // A bidirectional iterator that can read a const element in the set.
  typedef Expectation::Set::const_iterator const_iterator;

  // An object stored in the set.  This is an alias of Expectation.
  typedef Expectation::Set::value_type value_type;

  // Constructs an empty set.
  ExpectationSet() {}

  // This single-argument ctor must not be explicit, in order to support the
  //   ExpectationSet es = EXPECT_CALL(...);
  // syntax.
  ExpectationSet(internal::ExpectationBase& exp) {  // NOLINT
    *this += Expectation(exp);
  }

  // This single-argument ctor implements implicit conversion from
  // Expectation and thus must not be explicit.  This allows either an
  // Expectation or an ExpectationSet to be used in .After().
  ExpectationSet(const Expectation& e) {  // NOLINT
    *this += e;
  }

  // The compiler-generator ctor and operator= works exactly as
  // intended, so we don't need to define our own.

  // Returns true iff rhs contains the same set of Expectation objects
  // as this does.
  bool operator==(const ExpectationSet& rhs) const {
    return expectations_ == rhs.expectations_;
  }

  bool operator!=(const ExpectationSet& rhs) const { return !(*this == rhs); }

  // Implements the syntax
  //   expectation_set += EXPECT_CALL(...);
  ExpectationSet& operator+=(const Expectation& e) {
    expectations_.insert(e);
    return *this;
  }

  int size() const { return static_cast<int>(expectations_.size()); }

  const_iterator begin() const { return expectations_.begin(); }
  const_iterator end() const { return expectations_.end(); }

 private:
  Expectation::Set expectations_;
};


// Sequence objects are used by a user to specify the relative order
// in which the expectations should match.  They are copyable (we rely
// on the compiler-defined copy constructor and assignment operator).
class Sequence {
 public:
  // Constructs an empty sequence.
  Sequence() : last_expectation_(new Expectation) {}

  // Adds an expectation to this sequence.  The caller must ensure
  // that no other thread is accessing this Sequence object.
  void AddExpectation(const Expectation& expectation) const;

 private:
  // The last expectation in this sequence.  We use a linked_ptr here
  // because Sequence objects are copyable and we want the copies to
  // be aliases.  The linked_ptr allows the copies to co-own and share
  // the same Expectation object.
  internal::linked_ptr<Expectation> last_expectation_;
};  // class Sequence

// An object of this type causes all EXPECT_CALL() statements
// encountered in its scope to be put in an anonymous sequence.  The
// work is done in the constructor and destructor.  You should only
// create an InSequence object on the stack.
//
// The sole purpose for this class is to support easy definition of
// sequential expectations, e.g.
//
//   {
//     InSequence dummy;  // The name of the object doesn't matter.
//
//     // The following expectations must match in the order they appear.
//     EXPECT_CALL(a, Bar())...;
//     EXPECT_CALL(a, Baz())...;
//     ...
//     EXPECT_CALL(b, Xyz())...;
//   }
//
// You can create InSequence objects in multiple threads, as long as
// they are used to affect different mock objects.  The idea is that
// each thread can create and set up its own mocks as if it's the only
// thread.  However, for clarity of your tests we recommend you to set
// up mocks in the main thread unless you have a good reason not to do
// so.
class InSequence {
 public:
  InSequence();
  ~InSequence();
 private:
  bool sequence_created_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(InSequence);  // NOLINT
} GMOCK_ATTRIBUTE_UNUSED_;

namespace internal {

// Points to the implicit sequence introduced by a living InSequence
// object (if any) in the current thread or NULL.
extern ThreadLocal<Sequence*> g_gmock_implicit_sequence;

// Base class for implementing expectations.
//
// There are two reasons for having a type-agnostic base class for
// Expectation:
//
//   1. We need to store collections of expectations of different
//   types (e.g. all pre-requisites of a particular expectation, all
//   expectations in a sequence).  Therefore these expectation objects
//   must share a common base class.
//
//   2. We can avoid binary code bloat by moving methods not depending
//   on the template argument of Expectation to the base class.
//
// This class is internal and mustn't be used by user code directly.
class ExpectationBase {
 public:
  // source_text is the EXPECT_CALL(...) source that created this Expectation.
  ExpectationBase(const char* file, int line, const string& source_text);

  virtual ~ExpectationBase();

  // Where in the source file was the expectation spec defined?
  const char* file() const { return file_; }
  int line() const { return line_; }
  const char* source_text() const { return source_text_.c_str(); }
  // Returns the cardinality specified in the expectation spec.
  const Cardinality& cardinality() const { return cardinality_; }

  // Describes the source file location of this expectation.
  void DescribeLocationTo(::std::ostream* os) const {
    *os << file() << ":" << line() << ": ";
  }

  // Describes how many times a function call matching this
  // expectation has occurred.
  // L >= g_gmock_mutex
  virtual void DescribeCallCountTo(::std::ostream* os) const = 0;

 protected:
  friend class ::testing::Expectation;

  enum Clause {
    // Don't change the order of the enum members!
    kNone,
    kWith,
    kTimes,
    kInSequence,
    kAfter,
    kWillOnce,
    kWillRepeatedly,
    kRetiresOnSaturation,
  };

  // Returns an Expectation object that references and co-owns this
  // expectation.
  virtual Expectation GetHandle() = 0;

  // Asserts that the EXPECT_CALL() statement has the given property.
  void AssertSpecProperty(bool property, const string& failure_message) const {
    Assert(property, file_, line_, failure_message);
  }

  // Expects that the EXPECT_CALL() statement has the given property.
  void ExpectSpecProperty(bool property, const string& failure_message) const {
    Expect(property, file_, line_, failure_message);
  }

  // Explicitly specifies the cardinality of this expectation.  Used
  // by the subclasses to implement the .Times() clause.
  void SpecifyCardinality(const Cardinality& cardinality);

  // Returns true iff the user specified the cardinality explicitly
  // using a .Times().
  bool cardinality_specified() const { return cardinality_specified_; }

  // Sets the cardinality of this expectation spec.
  void set_cardinality(const Cardinality& a_cardinality) {
    cardinality_ = a_cardinality;
  }

  // The following group of methods should only be called after the
  // EXPECT_CALL() statement, and only when g_gmock_mutex is held by
  // the current thread.

  // Retires all pre-requisites of this expectation.
  // L >= g_gmock_mutex
  void RetireAllPreRequisites();

  // Returns true iff this expectation is retired.
  // L >= g_gmock_mutex
  bool is_retired() const {
    g_gmock_mutex.AssertHeld();
    return retired_;
  }

  // Retires this expectation.
  // L >= g_gmock_mutex
  void Retire() {
    g_gmock_mutex.AssertHeld();
    retired_ = true;
  }

  // Returns true iff this expectation is satisfied.
  // L >= g_gmock_mutex
  bool IsSatisfied() const {
    g_gmock_mutex.AssertHeld();
    return cardinality().IsSatisfiedByCallCount(call_count_);
  }

  // Returns true iff this expectation is saturated.
  // L >= g_gmock_mutex
  bool IsSaturated() const {
    g_gmock_mutex.AssertHeld();
    return cardinality().IsSaturatedByCallCount(call_count_);
  }

  // Returns true iff this expectation is over-saturated.
  // L >= g_gmock_mutex
  bool IsOverSaturated() const {
    g_gmock_mutex.AssertHeld();
    return cardinality().IsOverSaturatedByCallCount(call_count_);
  }

  // Returns true iff all pre-requisites of this expectation are satisfied.
  // L >= g_gmock_mutex
  bool AllPrerequisitesAreSatisfied() const;

  // Adds unsatisfied pre-requisites of this expectation to 'result'.
  // L >= g_gmock_mutex
  void FindUnsatisfiedPrerequisites(ExpectationSet* result) const;

  // Returns the number this expectation has been invoked.
  // L >= g_gmock_mutex
  int call_count() const {
    g_gmock_mutex.AssertHeld();
    return call_count_;
  }

  // Increments the number this expectation has been invoked.
  // L >= g_gmock_mutex
  void IncrementCallCount() {
    g_gmock_mutex.AssertHeld();
    call_count_++;
  }

 private:
  friend class ::testing::Sequence;
  friend class ::testing::internal::ExpectationTester;

  template <typename Function>
  friend class TypedExpectation;

  // This group of fields are part of the spec and won't change after
  // an EXPECT_CALL() statement finishes.
  const char* file_;          // The file that contains the expectation.
  int line_;                  // The line number of the expectation.
  const string source_text_;  // The EXPECT_CALL(...) source text.
  // True iff the cardinality is specified explicitly.
  bool cardinality_specified_;
  Cardinality cardinality_;            // The cardinality of the expectation.
  // The immediate pre-requisites (i.e. expectations that must be
  // satisfied before this expectation can be matched) of this
  // expectation.  We use linked_ptr in the set because we want an
  // Expectation object to be co-owned by its FunctionMocker and its
  // successors.  This allows multiple mock objects to be deleted at
  // different times.
  ExpectationSet immediate_prerequisites_;

  // This group of fields are the current state of the expectation,
  // and can change as the mock function is called.
  int call_count_;  // How many times this expectation has been invoked.
  bool retired_;    // True iff this expectation has retired.

  GTEST_DISALLOW_ASSIGN_(ExpectationBase);
};  // class ExpectationBase

// Impements an expectation for the given function type.
template <typename F>
class TypedExpectation : public ExpectationBase {
 public:
  typedef typename Function<F>::ArgumentTuple ArgumentTuple;
  typedef typename Function<F>::ArgumentMatcherTuple ArgumentMatcherTuple;
  typedef typename Function<F>::Result Result;

  TypedExpectation(FunctionMockerBase<F>* owner,
                   const char* a_file, int a_line, const string& a_source_text,
                   const ArgumentMatcherTuple& m)
      : ExpectationBase(a_file, a_line, a_source_text),
        owner_(owner),
        matchers_(m),
        extra_matcher_specified_(false),
        // By default, extra_matcher_ should match anything.  However,
        // we cannot initialize it with _ as that triggers a compiler
        // bug in Symbian's C++ compiler (cannot decide between two
        // overloaded constructors of Matcher<const ArgumentTuple&>).
        extra_matcher_(A<const ArgumentTuple&>()),
        repeated_action_specified_(false),
        repeated_action_(DoDefault()),
        retires_on_saturation_(false),
        last_clause_(kNone),
        action_count_checked_(false) {}

  virtual ~TypedExpectation() {
    // Check the validity of the action count if it hasn't been done
    // yet (for example, if the expectation was never used).
    CheckActionCountIfNotDone();
  }

  // Implements the .With() clause.
  TypedExpectation& With(const Matcher<const ArgumentTuple&>& m) {
    if (last_clause_ == kWith) {
      ExpectSpecProperty(false,
                         ".With() cannot appear "
                         "more than once in an EXPECT_CALL().");
    } else {
      ExpectSpecProperty(last_clause_ < kWith,
                         ".With() must be the first "
                         "clause in an EXPECT_CALL().");
    }
    last_clause_ = kWith;

    extra_matcher_ = m;
    extra_matcher_specified_ = true;
    return *this;
  }

  // Implements the .Times() clause.
  TypedExpectation& Times(const Cardinality& a_cardinality) {
    if (last_clause_ ==kTimes) {
      ExpectSpecProperty(false,
                         ".Times() cannot appear "
                         "more than once in an EXPECT_CALL().");
    } else {
      ExpectSpecProperty(last_clause_ < kTimes,
                         ".Times() cannot appear after "
                         ".InSequence(), .WillOnce(), .WillRepeatedly(), "
                         "or .RetiresOnSaturation().");
    }
    last_clause_ = kTimes;

    ExpectationBase::SpecifyCardinality(a_cardinality);
    return *this;
  }

  // Implements the .Times() clause.
  TypedExpectation& Times(int n) {
    return Times(Exactly(n));
  }

  // Implements the .InSequence() clause.
  TypedExpectation& InSequence(const Sequence& s) {
    ExpectSpecProperty(last_clause_ <= kInSequence,
                       ".InSequence() cannot appear after .After(),"
                       " .WillOnce(), .WillRepeatedly(), or "
                       ".RetiresOnSaturation().");
    last_clause_ = kInSequence;

    s.AddExpectation(GetHandle());
    return *this;
  }
  TypedExpectation& InSequence(const Sequence& s1, const Sequence& s2) {
    return InSequence(s1).InSequence(s2);
  }
  TypedExpectation& InSequence(const Sequence& s1, const Sequence& s2,
                               const Sequence& s3) {
    return InSequence(s1, s2).InSequence(s3);
  }
  TypedExpectation& InSequence(const Sequence& s1, const Sequence& s2,
                               const Sequence& s3, const Sequence& s4) {
    return InSequence(s1, s2, s3).InSequence(s4);
  }
  TypedExpectation& InSequence(const Sequence& s1, const Sequence& s2,
                               const Sequence& s3, const Sequence& s4,
                               const Sequence& s5) {
    return InSequence(s1, s2, s3, s4).InSequence(s5);
  }

  // Implements that .After() clause.
  TypedExpectation& After(const ExpectationSet& s) {
    ExpectSpecProperty(last_clause_ <= kAfter,
                       ".After() cannot appear after .WillOnce(),"
                       " .WillRepeatedly(), or "
                       ".RetiresOnSaturation().");
    last_clause_ = kAfter;

    for (ExpectationSet::const_iterator it = s.begin(); it != s.end(); ++it) {
      immediate_prerequisites_ += *it;
    }
    return *this;
  }
  TypedExpectation& After(const ExpectationSet& s1, const ExpectationSet& s2) {
    return After(s1).After(s2);
  }
  TypedExpectation& After(const ExpectationSet& s1, const ExpectationSet& s2,
                          const ExpectationSet& s3) {
    return After(s1, s2).After(s3);
  }
  TypedExpectation& After(const ExpectationSet& s1, const ExpectationSet& s2,
                          const ExpectationSet& s3, const ExpectationSet& s4) {
    return After(s1, s2, s3).After(s4);
  }
  TypedExpectation& After(const ExpectationSet& s1, const ExpectationSet& s2,
                          const ExpectationSet& s3, const ExpectationSet& s4,
                          const ExpectationSet& s5) {
    return After(s1, s2, s3, s4).After(s5);
  }

  // Implements the .WillOnce() clause.
  TypedExpectation& WillOnce(const Action<F>& action) {
    ExpectSpecProperty(last_clause_ <= kWillOnce,
                       ".WillOnce() cannot appear after "
                       ".WillRepeatedly() or .RetiresOnSaturation().");
    last_clause_ = kWillOnce;

    actions_.push_back(action);
    if (!cardinality_specified()) {
      set_cardinality(Exactly(static_cast<int>(actions_.size())));
    }
    return *this;
  }

  // Implements the .WillRepeatedly() clause.
  TypedExpectation& WillRepeatedly(const Action<F>& action) {
    if (last_clause_ == kWillRepeatedly) {
      ExpectSpecProperty(false,
                         ".WillRepeatedly() cannot appear "
                         "more than once in an EXPECT_CALL().");
    } else {
      ExpectSpecProperty(last_clause_ < kWillRepeatedly,
                         ".WillRepeatedly() cannot appear "
                         "after .RetiresOnSaturation().");
    }
    last_clause_ = kWillRepeatedly;
    repeated_action_specified_ = true;

    repeated_action_ = action;
    if (!cardinality_specified()) {
      set_cardinality(AtLeast(static_cast<int>(actions_.size())));
    }

    // Now that no more action clauses can be specified, we check
    // whether their count makes sense.
    CheckActionCountIfNotDone();
    return *this;
  }

  // Implements the .RetiresOnSaturation() clause.
  TypedExpectation& RetiresOnSaturation() {
    ExpectSpecProperty(last_clause_ < kRetiresOnSaturation,
                       ".RetiresOnSaturation() cannot appear "
                       "more than once.");
    last_clause_ = kRetiresOnSaturation;
    retires_on_saturation_ = true;

    // Now that no more action clauses can be specified, we check
    // whether their count makes sense.
    CheckActionCountIfNotDone();
    return *this;
  }

  // Returns the matchers for the arguments as specified inside the
  // EXPECT_CALL() macro.
  const ArgumentMatcherTuple& matchers() const {
    return matchers_;
  }

  // Returns the matcher specified by the .With() clause.
  const Matcher<const ArgumentTuple&>& extra_matcher() const {
    return extra_matcher_;
  }

  // Returns the sequence of actions specified by the .WillOnce() clause.
  const std::vector<Action<F> >& actions() const { return actions_; }

  // Returns the action specified by the .WillRepeatedly() clause.
  const Action<F>& repeated_action() const { return repeated_action_; }

  // Returns true iff the .RetiresOnSaturation() clause was specified.
  bool retires_on_saturation() const { return retires_on_saturation_; }

  // Describes how many times a function call matching this
  // expectation has occurred (implements
  // ExpectationBase::DescribeCallCountTo()).
  // L >= g_gmock_mutex
  virtual void DescribeCallCountTo(::std::ostream* os) const {
    g_gmock_mutex.AssertHeld();

    // Describes how many times the function is expected to be called.
    *os << "         Expected: to be ";
    cardinality().DescribeTo(os);
    *os << "\n           Actual: ";
    Cardinality::DescribeActualCallCountTo(call_count(), os);

    // Describes the state of the expectation (e.g. is it satisfied?
    // is it active?).
    *os << " - " << (IsOverSaturated() ? "over-saturated" :
                     IsSaturated() ? "saturated" :
                     IsSatisfied() ? "satisfied" : "unsatisfied")
        << " and "
        << (is_retired() ? "retired" : "active");
  }

  void MaybeDescribeExtraMatcherTo(::std::ostream* os) {
    if (extra_matcher_specified_) {
      *os << "    Expected args: ";
      extra_matcher_.DescribeTo(os);
      *os << "\n";
    }
  }

 private:
  template <typename Function>
  friend class FunctionMockerBase;

  // Returns an Expectation object that references and co-owns this
  // expectation.
  virtual Expectation GetHandle() {
    return owner_->GetHandleOf(this);
  }

  // The following methods will be called only after the EXPECT_CALL()
  // statement finishes and when the current thread holds
  // g_gmock_mutex.

  // Returns true iff this expectation matches the given arguments.
  // L >= g_gmock_mutex
  bool Matches(const ArgumentTuple& args) const {
    g_gmock_mutex.AssertHeld();
    return TupleMatches(matchers_, args) && extra_matcher_.Matches(args);
  }

  // Returns true iff this expectation should handle the given arguments.
  // L >= g_gmock_mutex
  bool ShouldHandleArguments(const ArgumentTuple& args) const {
    g_gmock_mutex.AssertHeld();

    // In case the action count wasn't checked when the expectation
    // was defined (e.g. if this expectation has no WillRepeatedly()
    // or RetiresOnSaturation() clause), we check it when the
    // expectation is used for the first time.
    CheckActionCountIfNotDone();
    return !is_retired() && AllPrerequisitesAreSatisfied() && Matches(args);
  }

  // Describes the result of matching the arguments against this
  // expectation to the given ostream.
  // L >= g_gmock_mutex
  void ExplainMatchResultTo(const ArgumentTuple& args,
                            ::std::ostream* os) const {
    g_gmock_mutex.AssertHeld();

    if (is_retired()) {
      *os << "         Expected: the expectation is active\n"
          << "           Actual: it is retired\n";
    } else if (!Matches(args)) {
      if (!TupleMatches(matchers_, args)) {
        ExplainMatchFailureTupleTo(matchers_, args, os);
      }
      StringMatchResultListener listener;
      if (!extra_matcher_.MatchAndExplain(args, &listener)) {
        *os << "    Expected args: ";
        extra_matcher_.DescribeTo(os);
        *os << "\n           Actual: don't match";

        internal::PrintIfNotEmpty(listener.str(), os);
        *os << "\n";
      }
    } else if (!AllPrerequisitesAreSatisfied()) {
      *os << "         Expected: all pre-requisites are satisfied\n"
          << "           Actual: the following immediate pre-requisites "
          << "are not satisfied:\n";
      ExpectationSet unsatisfied_prereqs;
      FindUnsatisfiedPrerequisites(&unsatisfied_prereqs);
      int i = 0;
      for (ExpectationSet::const_iterator it = unsatisfied_prereqs.begin();
           it != unsatisfied_prereqs.end(); ++it) {
        it->expectation_base()->DescribeLocationTo(os);
        *os << "pre-requisite #" << i++ << "\n";
      }
      *os << "                   (end of pre-requisites)\n";
    } else {
      // This line is here just for completeness' sake.  It will never
      // be executed as currently the ExplainMatchResultTo() function
      // is called only when the mock function call does NOT match the
      // expectation.
      *os << "The call matches the expectation.\n";
    }
  }

  // Returns the action that should be taken for the current invocation.
  // L >= g_gmock_mutex
  const Action<F>& GetCurrentAction(const FunctionMockerBase<F>* mocker,
                                    const ArgumentTuple& args) const {
    g_gmock_mutex.AssertHeld();
    const int count = call_count();
    Assert(count >= 1, __FILE__, __LINE__,
           "call_count() is <= 0 when GetCurrentAction() is "
           "called - this should never happen.");

    const int action_count = static_cast<int>(actions().size());
    if (action_count > 0 && !repeated_action_specified_ &&
        count > action_count) {
      // If there is at least one WillOnce() and no WillRepeatedly(),
      // we warn the user when the WillOnce() clauses ran out.
      ::std::stringstream ss;
      DescribeLocationTo(&ss);
      ss << "Actions ran out in " << source_text() << "...\n"
         << "Called " << count << " times, but only "
         << action_count << " WillOnce()"
         << (action_count == 1 ? " is" : "s are") << " specified - ";
      mocker->DescribeDefaultActionTo(args, &ss);
      Log(WARNING, ss.str(), 1);
    }

    return count <= action_count ? actions()[count - 1] : repeated_action();
  }

  // Given the arguments of a mock function call, if the call will
  // over-saturate this expectation, returns the default action;
  // otherwise, returns the next action in this expectation.  Also
  // describes *what* happened to 'what', and explains *why* Google
  // Mock does it to 'why'.  This method is not const as it calls
  // IncrementCallCount().
  // L >= g_gmock_mutex
  Action<F> GetActionForArguments(const FunctionMockerBase<F>* mocker,
                                  const ArgumentTuple& args,
                                  ::std::ostream* what,
                                  ::std::ostream* why) {
    g_gmock_mutex.AssertHeld();
    if (IsSaturated()) {
      // We have an excessive call.
      IncrementCallCount();
      *what << "Mock function called more times than expected - ";
      mocker->DescribeDefaultActionTo(args, what);
      DescribeCallCountTo(why);

      // TODO(wan): allow the user to control whether unexpected calls
      // should fail immediately or continue using a flag
      // --gmock_unexpected_calls_are_fatal.
      return DoDefault();
    }

    IncrementCallCount();
    RetireAllPreRequisites();

    if (retires_on_saturation() && IsSaturated()) {
      Retire();
    }

    // Must be done after IncrementCount()!
    *what << "Mock function call matches " << source_text() <<"...\n";
    return GetCurrentAction(mocker, args);
  }

  // Checks the action count (i.e. the number of WillOnce() and
  // WillRepeatedly() clauses) against the cardinality if this hasn't
  // been done before.  Prints a warning if there are too many or too
  // few actions.
  // L < mutex_
  void CheckActionCountIfNotDone() const {
    bool should_check = false;
    {
      MutexLock l(&mutex_);
      if (!action_count_checked_) {
        action_count_checked_ = true;
        should_check = true;
      }
    }

    if (should_check) {
      if (!cardinality_specified_) {
        // The cardinality was inferred - no need to check the action
        // count against it.
        return;
      }

      // The cardinality was explicitly specified.
      const int action_count = static_cast<int>(actions_.size());
      const int upper_bound = cardinality().ConservativeUpperBound();
      const int lower_bound = cardinality().ConservativeLowerBound();
      bool too_many;  // True if there are too many actions, or false
                      // if there are too few.
      if (action_count > upper_bound ||
          (action_count == upper_bound && repeated_action_specified_)) {
        too_many = true;
      } else if (0 < action_count && action_count < lower_bound &&
                 !repeated_action_specified_) {
        too_many = false;
      } else {
        return;
      }

      ::std::stringstream ss;
      DescribeLocationTo(&ss);
      ss << "Too " << (too_many ? "many" : "few")
         << " actions specified in " << source_text() << "...\n"
         << "Expected to be ";
      cardinality().DescribeTo(&ss);
      ss << ", but has " << (too_many ? "" : "only ")
         << action_count << " WillOnce()"
         << (action_count == 1 ? "" : "s");
      if (repeated_action_specified_) {
        ss << " and a WillRepeatedly()";
      }
      ss << ".";
      Log(WARNING, ss.str(), -1);  // -1 means "don't print stack trace".
    }
  }

  // All the fields below won't change once the EXPECT_CALL()
  // statement finishes.
  FunctionMockerBase<F>* const owner_;
  ArgumentMatcherTuple matchers_;
  bool extra_matcher_specified_;
  Matcher<const ArgumentTuple&> extra_matcher_;
  std::vector<Action<F> > actions_;
  bool repeated_action_specified_;  // True if a WillRepeatedly() was specified.
  Action<F> repeated_action_;
  bool retires_on_saturation_;
  Clause last_clause_;
  mutable bool action_count_checked_;  // Under mutex_.
  mutable Mutex mutex_;  // Protects action_count_checked_.

  GTEST_DISALLOW_COPY_AND_ASSIGN_(TypedExpectation);
};  // class TypedExpectation

// A MockSpec object is used by ON_CALL() or EXPECT_CALL() for
// specifying the default behavior of, or expectation on, a mock
// function.

// Note: class MockSpec really belongs to the ::testing namespace.
// However if we define it in ::testing, MSVC will complain when
// classes in ::testing::internal declare it as a friend class
// template.  To workaround this compiler bug, we define MockSpec in
// ::testing::internal and import it into ::testing.

template <typename F>
class MockSpec {
 public:
  typedef typename internal::Function<F>::ArgumentTuple ArgumentTuple;
  typedef typename internal::Function<F>::ArgumentMatcherTuple
      ArgumentMatcherTuple;

  // Constructs a MockSpec object, given the function mocker object
  // that the spec is associated with.
  explicit MockSpec(internal::FunctionMockerBase<F>* function_mocker)
      : function_mocker_(function_mocker) {}

  // Adds a new default action spec to the function mocker and returns
  // the newly created spec.
  internal::DefaultActionSpec<F>& InternalDefaultActionSetAt(
      const char* file, int line, const char* obj, const char* call) {
    LogWithLocation(internal::INFO, file, line,
        string("ON_CALL(") + obj + ", " + call + ") invoked");
    return function_mocker_->AddNewDefaultActionSpec(file, line, matchers_);
  }

  // Adds a new expectation spec to the function mocker and returns
  // the newly created spec.
  internal::TypedExpectation<F>& InternalExpectedAt(
      const char* file, int line, const char* obj, const char* call) {
    const string source_text(string("EXPECT_CALL(") + obj + ", " + call + ")");
    LogWithLocation(internal::INFO, file, line, source_text + " invoked");
    return function_mocker_->AddNewExpectation(
        file, line, source_text, matchers_);
  }

 private:
  template <typename Function>
  friend class internal::FunctionMocker;

  void SetMatchers(const ArgumentMatcherTuple& matchers) {
    matchers_ = matchers;
  }

  // Logs a message including file and line number information.
  void LogWithLocation(testing::internal::LogSeverity severity,
                       const char* file, int line,
                       const string& message) {
    ::std::ostringstream s;
    s << file << ":" << line << ": " << message << ::std::endl;
    Log(severity, s.str(), 0);
  }

  // The function mocker that owns this spec.
  internal::FunctionMockerBase<F>* const function_mocker_;
  // The argument matchers specified in the spec.
  ArgumentMatcherTuple matchers_;

  GTEST_DISALLOW_ASSIGN_(MockSpec);
};  // class MockSpec

// MSVC warns about using 'this' in base member initializer list, so
// we need to temporarily disable the warning.  We have to do it for
// the entire class to suppress the warning, even though it's about
// the constructor only.

#ifdef _MSC_VER
#pragma warning(push)          // Saves the current warning state.
#pragma warning(disable:4355)  // Temporarily disables warning 4355.
#endif  // _MSV_VER

// C++ treats the void type specially.  For example, you cannot define
// a void-typed variable or pass a void value to a function.
// ActionResultHolder<T> holds a value of type T, where T must be a
// copyable type or void (T doesn't need to be default-constructable).
// It hides the syntactic difference between void and other types, and
// is used to unify the code for invoking both void-returning and
// non-void-returning mock functions.  This generic definition is used
// when T is not void.
template <typename T>
class ActionResultHolder {
 public:
  explicit ActionResultHolder(T a_value) : value_(a_value) {}

  // The compiler-generated copy constructor and assignment operator
  // are exactly what we need, so we don't need to define them.

  T value() const { return value_; }

  // Prints the held value as an action's result to os.
  void PrintAsActionResult(::std::ostream* os) const {
    *os << "\n          Returns: ";
    UniversalPrinter<T>::Print(value_, os);
  }

  // Performs the given mock function's default action and returns the
  // result in a ActionResultHolder.
  template <typename Function, typename Arguments>
  static ActionResultHolder PerformDefaultAction(
      const FunctionMockerBase<Function>* func_mocker,
      const Arguments& args,
      const string& call_description) {
    return ActionResultHolder(
        func_mocker->PerformDefaultAction(args, call_description));
  }

  // Performs the given action and returns the result in a
  // ActionResultHolder.
  template <typename Function, typename Arguments>
  static ActionResultHolder PerformAction(const Action<Function>& action,
                                          const Arguments& args) {
    return ActionResultHolder(action.Perform(args));
  }

 private:
  T value_;

  // T could be a reference type, so = isn't supported.
  GTEST_DISALLOW_ASSIGN_(ActionResultHolder);
};

// Specialization for T = void.
template <>
class ActionResultHolder<void> {
 public:
  ActionResultHolder() {}
  void value() const {}
  void PrintAsActionResult(::std::ostream* /* os */) const {}

  template <typename Function, typename Arguments>
  static ActionResultHolder PerformDefaultAction(
      const FunctionMockerBase<Function>* func_mocker,
      const Arguments& args,
      const string& call_description) {
    func_mocker->PerformDefaultAction(args, call_description);
    return ActionResultHolder();
  }

  template <typename Function, typename Arguments>
  static ActionResultHolder PerformAction(const Action<Function>& action,
                                          const Arguments& args) {
    action.Perform(args);
    return ActionResultHolder();
  }
};

// The base of the function mocker class for the given function type.
// We put the methods in this class instead of its child to avoid code
// bloat.
template <typename F>
class FunctionMockerBase : public UntypedFunctionMockerBase {
 public:
  typedef typename Function<F>::Result Result;
  typedef typename Function<F>::ArgumentTuple ArgumentTuple;
  typedef typename Function<F>::ArgumentMatcherTuple ArgumentMatcherTuple;

  FunctionMockerBase() : mock_obj_(NULL), name_(""), current_spec_(this) {}

  // The destructor verifies that all expectations on this mock
  // function have been satisfied.  If not, it will report Google Test
  // non-fatal failures for the violations.
  // L < g_gmock_mutex
  virtual ~FunctionMockerBase() {
    MutexLock l(&g_gmock_mutex);
    VerifyAndClearExpectationsLocked();
    Mock::UnregisterLocked(this);
  }

  // Returns the ON_CALL spec that matches this mock function with the
  // given arguments; returns NULL if no matching ON_CALL is found.
  // L = *
  const DefaultActionSpec<F>* FindDefaultActionSpec(
      const ArgumentTuple& args) const {
    for (typename std::vector<DefaultActionSpec<F> >::const_reverse_iterator it
             = default_actions_.rbegin();
         it != default_actions_.rend(); ++it) {
      const DefaultActionSpec<F>& spec = *it;
      if (spec.Matches(args))
        return &spec;
    }

    return NULL;
  }

  // Performs the default action of this mock function on the given arguments
  // and returns the result. Asserts with a helpful call descrption if there is
  // no valid return value. This method doesn't depend on the mutable state of
  // this object, and thus can be called concurrently without locking.
  // L = *
  Result PerformDefaultAction(const ArgumentTuple& args,
                              const string& call_description) const {
    const DefaultActionSpec<F>* const spec = FindDefaultActionSpec(args);
    if (spec != NULL) {
      return spec->GetAction().Perform(args);
    }
    Assert(DefaultValue<Result>::Exists(), "", -1,
           call_description + "\n    The mock function has no default action "
           "set, and its return type has no default value set.");
    return DefaultValue<Result>::Get();
  }

  // Registers this function mocker and the mock object owning it;
  // returns a reference to the function mocker object.  This is only
  // called by the ON_CALL() and EXPECT_CALL() macros.
  // L < g_gmock_mutex
  FunctionMocker<F>& RegisterOwner(const void* mock_obj) {
    {
      MutexLock l(&g_gmock_mutex);
      mock_obj_ = mock_obj;
    }
    Mock::Register(mock_obj, this);
    return *::testing::internal::down_cast<FunctionMocker<F>*>(this);
  }

  // The following two functions are from UntypedFunctionMockerBase.

  // Verifies that all expectations on this mock function have been
  // satisfied.  Reports one or more Google Test non-fatal failures
  // and returns false if not.
  // L >= g_gmock_mutex
  virtual bool VerifyAndClearExpectationsLocked();

  // Clears the ON_CALL()s set on this mock function.
  // L >= g_gmock_mutex
  virtual void ClearDefaultActionsLocked() {
    g_gmock_mutex.AssertHeld();
    default_actions_.clear();
  }

  // Sets the name of the function being mocked.  Will be called upon
  // each invocation of this mock function.
  // L < g_gmock_mutex
  void SetOwnerAndName(const void* mock_obj, const char* name) {
    // We protect name_ under g_gmock_mutex in case this mock function
    // is called from two threads concurrently.
    MutexLock l(&g_gmock_mutex);
    mock_obj_ = mock_obj;
    name_ = name;
  }

  // Returns the address of the mock object this method belongs to.
  // Must be called after SetOwnerAndName() has been called.
  // L < g_gmock_mutex
  const void* MockObject() const {
    const void* mock_obj;
    {
      // We protect mock_obj_ under g_gmock_mutex in case this mock
      // function is called from two threads concurrently.
      MutexLock l(&g_gmock_mutex);
      mock_obj = mock_obj_;
    }
    return mock_obj;
  }

  // Returns the name of the function being mocked.  Must be called
  // after SetOwnerAndName() has been called.
  // L < g_gmock_mutex
  const char* Name() const {
    const char* name;
    {
      // We protect name_ under g_gmock_mutex in case this mock
      // function is called from two threads concurrently.
      MutexLock l(&g_gmock_mutex);
      name = name_;
    }
    return name;
  }

 protected:
  template <typename Function>
  friend class MockSpec;

  // Returns the result of invoking this mock function with the given
  // arguments.  This function can be safely called from multiple
  // threads concurrently.
  // L < g_gmock_mutex
  Result InvokeWith(const ArgumentTuple& args);

  // Adds and returns a default action spec for this mock function.
  // L < g_gmock_mutex
  DefaultActionSpec<F>& AddNewDefaultActionSpec(
      const char* file, int line,
      const ArgumentMatcherTuple& m) {
    Mock::RegisterUseByOnCallOrExpectCall(MockObject(), file, line);
    default_actions_.push_back(DefaultActionSpec<F>(file, line, m));
    return default_actions_.back();
  }

  // Adds and returns an expectation spec for this mock function.
  // L < g_gmock_mutex
  TypedExpectation<F>& AddNewExpectation(
      const char* file,
      int line,
      const string& source_text,
      const ArgumentMatcherTuple& m) {
    Mock::RegisterUseByOnCallOrExpectCall(MockObject(), file, line);
    const linked_ptr<TypedExpectation<F> > expectation(
        new TypedExpectation<F>(this, file, line, source_text, m));
    expectations_.push_back(expectation);

    // Adds this expectation into the implicit sequence if there is one.
    Sequence* const implicit_sequence = g_gmock_implicit_sequence.get();
    if (implicit_sequence != NULL) {
      implicit_sequence->AddExpectation(Expectation(expectation));
    }

    return *expectation;
  }

  // The current spec (either default action spec or expectation spec)
  // being described on this function mocker.
  MockSpec<F>& current_spec() { return current_spec_; }

 private:
  template <typename Func> friend class TypedExpectation;

  typedef std::vector<internal::linked_ptr<TypedExpectation<F> > >
  TypedExpectations;

  // Returns an Expectation object that references and co-owns exp,
  // which must be an expectation on this mock function.
  Expectation GetHandleOf(TypedExpectation<F>* exp) {
    for (typename TypedExpectations::const_iterator it = expectations_.begin();
         it != expectations_.end(); ++it) {
      if (it->get() == exp) {
        return Expectation(*it);
      }
    }

    Assert(false, __FILE__, __LINE__, "Cannot find expectation.");
    return Expectation();
    // The above statement is just to make the code compile, and will
    // never be executed.
  }

  // Some utilities needed for implementing InvokeWith().

  // Describes what default action will be performed for the given
  // arguments.
  // L = *
  void DescribeDefaultActionTo(const ArgumentTuple& args,
                               ::std::ostream* os) const {
    const DefaultActionSpec<F>* const spec = FindDefaultActionSpec(args);

    if (spec == NULL) {
      *os << (internal::type_equals<Result, void>::value ?
              "returning directly.\n" :
              "returning default value.\n");
    } else {
      *os << "taking default action specified at:\n"
          << spec->file() << ":" << spec->line() << ":\n";
    }
  }

  // Writes a message that the call is uninteresting (i.e. neither
  // explicitly expected nor explicitly unexpected) to the given
  // ostream.
  // L < g_gmock_mutex
  void DescribeUninterestingCall(const ArgumentTuple& args,
                                 ::std::ostream* os) const {
    *os << "Uninteresting mock function call - ";
    DescribeDefaultActionTo(args, os);
    *os << "    Function call: " << Name();
    UniversalPrinter<ArgumentTuple>::Print(args, os);
  }

  // Critical section: We must find the matching expectation and the
  // corresponding action that needs to be taken in an ATOMIC
  // transaction.  Otherwise another thread may call this mock
  // method in the middle and mess up the state.
  //
  // However, performing the action has to be left out of the critical
  // section.  The reason is that we have no control on what the
  // action does (it can invoke an arbitrary user function or even a
  // mock function) and excessive locking could cause a dead lock.
  // L < g_gmock_mutex
  bool FindMatchingExpectationAndAction(
      const ArgumentTuple& args, TypedExpectation<F>** exp, Action<F>* action,
      bool* is_excessive, ::std::ostream* what, ::std::ostream* why) {
    MutexLock l(&g_gmock_mutex);
    *exp = this->FindMatchingExpectationLocked(args);
    if (*exp == NULL) {  // A match wasn't found.
      *action = DoDefault();
      this->FormatUnexpectedCallMessageLocked(args, what, why);
      return false;
    }

    // This line must be done before calling GetActionForArguments(),
    // which will increment the call count for *exp and thus affect
    // its saturation status.
    *is_excessive = (*exp)->IsSaturated();
    *action = (*exp)->GetActionForArguments(this, args, what, why);
    return true;
  }

  // Returns the expectation that matches the arguments, or NULL if no
  // expectation matches them.
  // L >= g_gmock_mutex
  TypedExpectation<F>* FindMatchingExpectationLocked(
      const ArgumentTuple& args) const {
    g_gmock_mutex.AssertHeld();
    for (typename TypedExpectations::const_reverse_iterator it =
             expectations_.rbegin();
         it != expectations_.rend(); ++it) {
      TypedExpectation<F>* const exp = it->get();
      if (exp->ShouldHandleArguments(args)) {
        return exp;
      }
    }
    return NULL;
  }

  // Returns a message that the arguments don't match any expectation.
  // L >= g_gmock_mutex
  void FormatUnexpectedCallMessageLocked(const ArgumentTuple& args,
                                         ::std::ostream* os,
                                         ::std::ostream* why) const {
    g_gmock_mutex.AssertHeld();
    *os << "\nUnexpected mock function call - ";
    DescribeDefaultActionTo(args, os);
    PrintTriedExpectationsLocked(args, why);
  }

  // Prints a list of expectations that have been tried against the
  // current mock function call.
  // L >= g_gmock_mutex
  void PrintTriedExpectationsLocked(const ArgumentTuple& args,
                                    ::std::ostream* why) const {
    g_gmock_mutex.AssertHeld();
    const int count = static_cast<int>(expectations_.size());
    *why << "Google Mock tried the following " << count << " "
         << (count == 1 ? "expectation, but it didn't match" :
             "expectations, but none matched")
         << ":\n";
    for (int i = 0; i < count; i++) {
      *why << "\n";
      expectations_[i]->DescribeLocationTo(why);
      if (count > 1) {
        *why << "tried expectation #" << i << ": ";
      }
      *why << expectations_[i]->source_text() << "...\n";
      expectations_[i]->ExplainMatchResultTo(args, why);
      expectations_[i]->DescribeCallCountTo(why);
    }
  }

  // Address of the mock object this mock method belongs to.  Only
  // valid after this mock method has been called or
  // ON_CALL/EXPECT_CALL has been invoked on it.
  const void* mock_obj_;  // Protected by g_gmock_mutex.

  // Name of the function being mocked.  Only valid after this mock
  // method has been called.
  const char* name_;  // Protected by g_gmock_mutex.

  // The current spec (either default action spec or expectation spec)
  // being described on this function mocker.
  MockSpec<F> current_spec_;

  // All default action specs for this function mocker.
  std::vector<DefaultActionSpec<F> > default_actions_;
  // All expectations for this function mocker.
  TypedExpectations expectations_;

  // There is no generally useful and implementable semantics of
  // copying a mock object, so copying a mock is usually a user error.
  // Thus we disallow copying function mockers.  If the user really
  // wants to copy a mock object, he should implement his own copy
  // operation, for example:
  //
  //   class MockFoo : public Foo {
  //    public:
  //     // Defines a copy constructor explicitly.
  //     MockFoo(const MockFoo& src) {}
  //     ...
  //   };
  GTEST_DISALLOW_COPY_AND_ASSIGN_(FunctionMockerBase);
};  // class FunctionMockerBase

#ifdef _MSC_VER
#pragma warning(pop)  // Restores the warning state.
#endif  // _MSV_VER

// Implements methods of FunctionMockerBase.

// Verifies that all expectations on this mock function have been
// satisfied.  Reports one or more Google Test non-fatal failures and
// returns false if not.
// L >= g_gmock_mutex
template <typename F>
bool FunctionMockerBase<F>::VerifyAndClearExpectationsLocked() {
  g_gmock_mutex.AssertHeld();
  bool expectations_met = true;
  for (typename TypedExpectations::const_iterator it = expectations_.begin();
       it != expectations_.end(); ++it) {
    TypedExpectation<F>* const exp = it->get();

    if (exp->IsOverSaturated()) {
      // There was an upper-bound violation.  Since the error was
      // already reported when it occurred, there is no need to do
      // anything here.
      expectations_met = false;
    } else if (!exp->IsSatisfied()) {
      expectations_met = false;
      ::std::stringstream ss;
      ss  << "Actual function call count doesn't match "
          << exp->source_text() << "...\n";
      // No need to show the source file location of the expectation
      // in the description, as the Expect() call that follows already
      // takes care of it.
      exp->MaybeDescribeExtraMatcherTo(&ss);
      exp->DescribeCallCountTo(&ss);
      Expect(false, exp->file(), exp->line(), ss.str());
    }
  }
  expectations_.clear();
  return expectations_met;
}

// Reports an uninteresting call (whose description is in msg) in the
// manner specified by 'reaction'.
void ReportUninterestingCall(CallReaction reaction, const string& msg);

// Calculates the result of invoking this mock function with the given
// arguments, prints it, and returns it.
// L < g_gmock_mutex
template <typename F>
typename Function<F>::Result FunctionMockerBase<F>::InvokeWith(
    const typename Function<F>::ArgumentTuple& args) {
  typedef ActionResultHolder<Result> ResultHolder;

  if (expectations_.size() == 0) {
    // No expectation is set on this mock method - we have an
    // uninteresting call.

    // We must get Google Mock's reaction on uninteresting calls
    // made on this mock object BEFORE performing the action,
    // because the action may DELETE the mock object and make the
    // following expression meaningless.
    const CallReaction reaction =
        Mock::GetReactionOnUninterestingCalls(MockObject());

    // True iff we need to print this call's arguments and return
    // value.  This definition must be kept in sync with
    // the behavior of ReportUninterestingCall().
    const bool need_to_report_uninteresting_call =
        // If the user allows this uninteresting call, we print it
        // only when he wants informational messages.
        reaction == ALLOW ? LogIsVisible(INFO) :
        // If the user wants this to be a warning, we print it only
        // when he wants to see warnings.
        reaction == WARN ? LogIsVisible(WARNING) :
        // Otherwise, the user wants this to be an error, and we
        // should always print detailed information in the error.
        true;

    if (!need_to_report_uninteresting_call) {
      // Perform the action without printing the call information.
      return PerformDefaultAction(args, "");
    }

    // Warns about the uninteresting call.
    ::std::stringstream ss;
    DescribeUninterestingCall(args, &ss);

    // Calculates the function result.
    const ResultHolder result =
        ResultHolder::PerformDefaultAction(this, args, ss.str());

    // Prints the function result.
    result.PrintAsActionResult(&ss);

    ReportUninterestingCall(reaction, ss.str());
    return result.value();
  }

  bool is_excessive = false;
  ::std::stringstream ss;
  ::std::stringstream why;
  ::std::stringstream loc;
  Action<F> action;
  TypedExpectation<F>* exp;

  // The FindMatchingExpectationAndAction() function acquires and
  // releases g_gmock_mutex.
  const bool found = FindMatchingExpectationAndAction(
      args, &exp, &action, &is_excessive, &ss, &why);

  // True iff we need to print the call's arguments and return value.
  // This definition must be kept in sync with the uses of Expect()
  // and Log() in this function.
  const bool need_to_report_call = !found || is_excessive || LogIsVisible(INFO);
  if (!need_to_report_call) {
    // Perform the action without printing the call information.
    return action.IsDoDefault() ? PerformDefaultAction(args, "") :
        action.Perform(args);
  }

  ss << "    Function call: " << Name();
  UniversalPrinter<ArgumentTuple>::Print(args, &ss);

  // In case the action deletes a piece of the expectation, we
  // generate the message beforehand.
  if (found && !is_excessive) {
    exp->DescribeLocationTo(&loc);
  }

  const ResultHolder result = action.IsDoDefault() ?
      ResultHolder::PerformDefaultAction(this, args, ss.str()) :
      ResultHolder::PerformAction(action, args);
  result.PrintAsActionResult(&ss);
  ss << "\n" << why.str();

  if (!found) {
    // No expectation matches this call - reports a failure.
    Expect(false, NULL, -1, ss.str());
  } else if (is_excessive) {
    // We had an upper-bound violation and the failure message is in ss.
    Expect(false, exp->file(), exp->line(), ss.str());
  } else {
    // We had an expected call and the matching expectation is
    // described in ss.
    Log(INFO, loc.str() + ss.str(), 2);
  }
  return result.value();
}

}  // namespace internal

// The style guide prohibits "using" statements in a namespace scope
// inside a header file.  However, the MockSpec class template is
// meant to be defined in the ::testing namespace.  The following line
// is just a trick for working around a bug in MSVC 8.0, which cannot
// handle it if we define MockSpec in ::testing.
using internal::MockSpec;

// Const(x) is a convenient function for obtaining a const reference
// to x.  This is useful for setting expectations on an overloaded
// const mock method, e.g.
//
//   class MockFoo : public FooInterface {
//    public:
//     MOCK_METHOD0(Bar, int());
//     MOCK_CONST_METHOD0(Bar, int&());
//   };
//
//   MockFoo foo;
//   // Expects a call to non-const MockFoo::Bar().
//   EXPECT_CALL(foo, Bar());
//   // Expects a call to const MockFoo::Bar().
//   EXPECT_CALL(Const(foo), Bar());
template <typename T>
inline const T& Const(const T& x) { return x; }

// Constructs an Expectation object that references and co-owns exp.
inline Expectation::Expectation(internal::ExpectationBase& exp)  // NOLINT
    : expectation_base_(exp.GetHandle().expectation_base()) {}

}  // namespace testing

// A separate macro is required to avoid compile errors when the name
// of the method used in call is a result of macro expansion.
// See CompilesWithMethodNameExpandedFromMacro tests in
// internal/gmock-spec-builders_test.cc for more details.
#define GMOCK_ON_CALL_IMPL_(obj, call) \
    ((obj).gmock_##call).InternalDefaultActionSetAt(__FILE__, __LINE__, \
                                                    #obj, #call)
#define ON_CALL(obj, call) GMOCK_ON_CALL_IMPL_(obj, call)

#define GMOCK_EXPECT_CALL_IMPL_(obj, call) \
    ((obj).gmock_##call).InternalExpectedAt(__FILE__, __LINE__, #obj, #call)
#define EXPECT_CALL(obj, call) GMOCK_EXPECT_CALL_IMPL_(obj, call)

#endif  // GMOCK_INCLUDE_GMOCK_GMOCK_SPEC_BUILDERS_H_

namespace testing {
namespace internal {

template <typename F>
class FunctionMockerBase;

// Note: class FunctionMocker really belongs to the ::testing
// namespace.  However if we define it in ::testing, MSVC will
// complain when classes in ::testing::internal declare it as a
// friend class template.  To workaround this compiler bug, we define
// FunctionMocker in ::testing::internal and import it into ::testing.
template <typename F>
class FunctionMocker;

template <typename R>
class FunctionMocker<R()> : public
    internal::FunctionMockerBase<R()> {
 public:
  typedef R F();
  typedef typename internal::Function<F>::ArgumentTuple ArgumentTuple;

  MockSpec<F>& With() {
    return this->current_spec();
  }

  R Invoke() {
    // Even though gcc and MSVC don't enforce it, 'this->' is required
    // by the C++ standard [14.6.4] here, as the base class type is
    // dependent on the template argument (and thus shouldn't be
    // looked into when resolving InvokeWith).
    return this->InvokeWith(ArgumentTuple());
  }
};

template <typename R, typename A1>
class FunctionMocker<R(A1)> : public
    internal::FunctionMockerBase<R(A1)> {
 public:
  typedef R F(A1);
  typedef typename internal::Function<F>::ArgumentTuple ArgumentTuple;

  MockSpec<F>& With(const Matcher<A1>& m1) {
    this->current_spec().SetMatchers(::std::tr1::make_tuple(m1));
    return this->current_spec();
  }

  R Invoke(A1 a1) {
    // Even though gcc and MSVC don't enforce it, 'this->' is required
    // by the C++ standard [14.6.4] here, as the base class type is
    // dependent on the template argument (and thus shouldn't be
    // looked into when resolving InvokeWith).
    return this->InvokeWith(ArgumentTuple(a1));
  }
};

template <typename R, typename A1, typename A2>
class FunctionMocker<R(A1, A2)> : public
    internal::FunctionMockerBase<R(A1, A2)> {
 public:
  typedef R F(A1, A2);
  typedef typename internal::Function<F>::ArgumentTuple ArgumentTuple;

  MockSpec<F>& With(const Matcher<A1>& m1, const Matcher<A2>& m2) {
    this->current_spec().SetMatchers(::std::tr1::make_tuple(m1, m2));
    return this->current_spec();
  }

  R Invoke(A1 a1, A2 a2) {
    // Even though gcc and MSVC don't enforce it, 'this->' is required
    // by the C++ standard [14.6.4] here, as the base class type is
    // dependent on the template argument (and thus shouldn't be
    // looked into when resolving InvokeWith).
    return this->InvokeWith(ArgumentTuple(a1, a2));
  }
};

template <typename R, typename A1, typename A2, typename A3>
class FunctionMocker<R(A1, A2, A3)> : public
    internal::FunctionMockerBase<R(A1, A2, A3)> {
 public:
  typedef R F(A1, A2, A3);
  typedef typename internal::Function<F>::ArgumentTuple ArgumentTuple;

  MockSpec<F>& With(const Matcher<A1>& m1, const Matcher<A2>& m2,
      const Matcher<A3>& m3) {
    this->current_spec().SetMatchers(::std::tr1::make_tuple(m1, m2, m3));
    return this->current_spec();
  }

  R Invoke(A1 a1, A2 a2, A3 a3) {
    // Even though gcc and MSVC don't enforce it, 'this->' is required
    // by the C++ standard [14.6.4] here, as the base class type is
    // dependent on the template argument (and thus shouldn't be
    // looked into when resolving InvokeWith).
    return this->InvokeWith(ArgumentTuple(a1, a2, a3));
  }
};

template <typename R, typename A1, typename A2, typename A3, typename A4>
class FunctionMocker<R(A1, A2, A3, A4)> : public
    internal::FunctionMockerBase<R(A1, A2, A3, A4)> {
 public:
  typedef R F(A1, A2, A3, A4);
  typedef typename internal::Function<F>::ArgumentTuple ArgumentTuple;

  MockSpec<F>& With(const Matcher<A1>& m1, const Matcher<A2>& m2,
      const Matcher<A3>& m3, const Matcher<A4>& m4) {
    this->current_spec().SetMatchers(::std::tr1::make_tuple(m1, m2, m3, m4));
    return this->current_spec();
  }

  R Invoke(A1 a1, A2 a2, A3 a3, A4 a4) {
    // Even though gcc and MSVC don't enforce it, 'this->' is required
    // by the C++ standard [14.6.4] here, as the base class type is
    // dependent on the template argument (and thus shouldn't be
    // looked into when resolving InvokeWith).
    return this->InvokeWith(ArgumentTuple(a1, a2, a3, a4));
  }
};

template <typename R, typename A1, typename A2, typename A3, typename A4,
    typename A5>
class FunctionMocker<R(A1, A2, A3, A4, A5)> : public
    internal::FunctionMockerBase<R(A1, A2, A3, A4, A5)> {
 public:
  typedef R F(A1, A2, A3, A4, A5);
  typedef typename internal::Function<F>::ArgumentTuple ArgumentTuple;

  MockSpec<F>& With(const Matcher<A1>& m1, const Matcher<A2>& m2,
      const Matcher<A3>& m3, const Matcher<A4>& m4, const Matcher<A5>& m5) {
    this->current_spec().SetMatchers(::std::tr1::make_tuple(m1, m2, m3, m4,
        m5));
    return this->current_spec();
  }

  R Invoke(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
    // Even though gcc and MSVC don't enforce it, 'this->' is required
    // by the C++ standard [14.6.4] here, as the base class type is
    // dependent on the template argument (and thus shouldn't be
    // looked into when resolving InvokeWith).
    return this->InvokeWith(ArgumentTuple(a1, a2, a3, a4, a5));
  }
};

template <typename R, typename A1, typename A2, typename A3, typename A4,
    typename A5, typename A6>
class FunctionMocker<R(A1, A2, A3, A4, A5, A6)> : public
    internal::FunctionMockerBase<R(A1, A2, A3, A4, A5, A6)> {
 public:
  typedef R F(A1, A2, A3, A4, A5, A6);
  typedef typename internal::Function<F>::ArgumentTuple ArgumentTuple;

  MockSpec<F>& With(const Matcher<A1>& m1, const Matcher<A2>& m2,
      const Matcher<A3>& m3, const Matcher<A4>& m4, const Matcher<A5>& m5,
      const Matcher<A6>& m6) {
    this->current_spec().SetMatchers(::std::tr1::make_tuple(m1, m2, m3, m4, m5,
        m6));
    return this->current_spec();
  }

  R Invoke(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) {
    // Even though gcc and MSVC don't enforce it, 'this->' is required
    // by the C++ standard [14.6.4] here, as the base class type is
    // dependent on the template argument (and thus shouldn't be
    // looked into when resolving InvokeWith).
    return this->InvokeWith(ArgumentTuple(a1, a2, a3, a4, a5, a6));
  }
};

template <typename R, typename A1, typename A2, typename A3, typename A4,
    typename A5, typename A6, typename A7>
class FunctionMocker<R(A1, A2, A3, A4, A5, A6, A7)> : public
    internal::FunctionMockerBase<R(A1, A2, A3, A4, A5, A6, A7)> {
 public:
  typedef R F(A1, A2, A3, A4, A5, A6, A7);
  typedef typename internal::Function<F>::ArgumentTuple ArgumentTuple;

  MockSpec<F>& With(const Matcher<A1>& m1, const Matcher<A2>& m2,
      const Matcher<A3>& m3, const Matcher<A4>& m4, const Matcher<A5>& m5,
      const Matcher<A6>& m6, const Matcher<A7>& m7) {
    this->current_spec().SetMatchers(::std::tr1::make_tuple(m1, m2, m3, m4, m5,
        m6, m7));
    return this->current_spec();
  }

  R Invoke(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) {
    // Even though gcc and MSVC don't enforce it, 'this->' is required
    // by the C++ standard [14.6.4] here, as the base class type is
    // dependent on the template argument (and thus shouldn't be
    // looked into when resolving InvokeWith).
    return this->InvokeWith(ArgumentTuple(a1, a2, a3, a4, a5, a6, a7));
  }
};

template <typename R, typename A1, typename A2, typename A3, typename A4,
    typename A5, typename A6, typename A7, typename A8>
class FunctionMocker<R(A1, A2, A3, A4, A5, A6, A7, A8)> : public
    internal::FunctionMockerBase<R(A1, A2, A3, A4, A5, A6, A7, A8)> {
 public:
  typedef R F(A1, A2, A3, A4, A5, A6, A7, A8);
  typedef typename internal::Function<F>::ArgumentTuple ArgumentTuple;

  MockSpec<F>& With(const Matcher<A1>& m1, const Matcher<A2>& m2,
      const Matcher<A3>& m3, const Matcher<A4>& m4, const Matcher<A5>& m5,
      const Matcher<A6>& m6, const Matcher<A7>& m7, const Matcher<A8>& m8) {
    this->current_spec().SetMatchers(::std::tr1::make_tuple(m1, m2, m3, m4, m5,
        m6, m7, m8));
    return this->current_spec();
  }

  R Invoke(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8) {
    // Even though gcc and MSVC don't enforce it, 'this->' is required
    // by the C++ standard [14.6.4] here, as the base class type is
    // dependent on the template argument (and thus shouldn't be
    // looked into when resolving InvokeWith).
    return this->InvokeWith(ArgumentTuple(a1, a2, a3, a4, a5, a6, a7, a8));
  }
};

template <typename R, typename A1, typename A2, typename A3, typename A4,
    typename A5, typename A6, typename A7, typename A8, typename A9>
class FunctionMocker<R(A1, A2, A3, A4, A5, A6, A7, A8, A9)> : public
    internal::FunctionMockerBase<R(A1, A2, A3, A4, A5, A6, A7, A8, A9)> {
 public:
  typedef R F(A1, A2, A3, A4, A5, A6, A7, A8, A9);
  typedef typename internal::Function<F>::ArgumentTuple ArgumentTuple;

  MockSpec<F>& With(const Matcher<A1>& m1, const Matcher<A2>& m2,
      const Matcher<A3>& m3, const Matcher<A4>& m4, const Matcher<A5>& m5,
      const Matcher<A6>& m6, const Matcher<A7>& m7, const Matcher<A8>& m8,
      const Matcher<A9>& m9) {
    this->current_spec().SetMatchers(::std::tr1::make_tuple(m1, m2, m3, m4, m5,
        m6, m7, m8, m9));
    return this->current_spec();
  }

  R Invoke(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9) {
    // Even though gcc and MSVC don't enforce it, 'this->' is required
    // by the C++ standard [14.6.4] here, as the base class type is
    // dependent on the template argument (and thus shouldn't be
    // looked into when resolving InvokeWith).
    return this->InvokeWith(ArgumentTuple(a1, a2, a3, a4, a5, a6, a7, a8, a9));
  }
};

template <typename R, typename A1, typename A2, typename A3, typename A4,
    typename A5, typename A6, typename A7, typename A8, typename A9,
    typename A10>
class FunctionMocker<R(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)> : public
    internal::FunctionMockerBase<R(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)> {
 public:
  typedef R F(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);
  typedef typename internal::Function<F>::ArgumentTuple ArgumentTuple;

  MockSpec<F>& With(const Matcher<A1>& m1, const Matcher<A2>& m2,
      const Matcher<A3>& m3, const Matcher<A4>& m4, const Matcher<A5>& m5,
      const Matcher<A6>& m6, const Matcher<A7>& m7, const Matcher<A8>& m8,
      const Matcher<A9>& m9, const Matcher<A10>& m10) {
    this->current_spec().SetMatchers(::std::tr1::make_tuple(m1, m2, m3, m4, m5,
        m6, m7, m8, m9, m10));
    return this->current_spec();
  }

  R Invoke(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9,
      A10 a10) {
    // Even though gcc and MSVC don't enforce it, 'this->' is required
    // by the C++ standard [14.6.4] here, as the base class type is
    // dependent on the template argument (and thus shouldn't be
    // looked into when resolving InvokeWith).
    return this->InvokeWith(ArgumentTuple(a1, a2, a3, a4, a5, a6, a7, a8, a9,
        a10));
  }
};

}  // namespace internal

// The style guide prohibits "using" statements in a namespace scope
// inside a header file.  However, the FunctionMocker class template
// is meant to be defined in the ::testing namespace.  The following
// line is just a trick for working around a bug in MSVC 8.0, which
// cannot handle it if we define FunctionMocker in ::testing.
using internal::FunctionMocker;

// The result type of function type F.
// INTERNAL IMPLEMENTATION - DON'T USE IN USER CODE!!!
#define GMOCK_RESULT_(tn, F) tn ::testing::internal::Function<F>::Result

// The type of argument N of function type F.
// INTERNAL IMPLEMENTATION - DON'T USE IN USER CODE!!!
#define GMOCK_ARG_(tn, F, N) tn ::testing::internal::Function<F>::Argument##N

// The matcher type for argument N of function type F.
// INTERNAL IMPLEMENTATION - DON'T USE IN USER CODE!!!
#define GMOCK_MATCHER_(tn, F, N) const ::testing::Matcher<GMOCK_ARG_(tn, F, N)>&

// The variable for mocking the given method.
// INTERNAL IMPLEMENTATION - DON'T USE IN USER CODE!!!
#define GMOCK_MOCKER_(arity, constness, Method) \
    GMOCK_CONCAT_TOKEN_(gmock##constness##arity##_##Method##_, __LINE__)

// INTERNAL IMPLEMENTATION - DON'T USE IN USER CODE!!!
#define GMOCK_METHOD0_(tn, constness, ct, Method, F) \
  GMOCK_RESULT_(tn, F) ct Method() constness { \
    GMOCK_COMPILE_ASSERT_(::std::tr1::tuple_size< \
        tn ::testing::internal::Function<F>::ArgumentTuple>::value == 0, \
        this_method_does_not_take_0_arguments); \
    GMOCK_MOCKER_(0, constness, Method).SetOwnerAndName(this, #Method); \
    return GMOCK_MOCKER_(0, constness, Method).Invoke(); \
  } \
  ::testing::MockSpec<F>& \
      gmock_##Method() constness { \
    return GMOCK_MOCKER_(0, constness, Method).RegisterOwner(this).With(); \
  } \
  mutable ::testing::FunctionMocker<F> GMOCK_MOCKER_(0, constness, Method)

// INTERNAL IMPLEMENTATION - DON'T USE IN USER CODE!!!
#define GMOCK_METHOD1_(tn, constness, ct, Method, F) \
  GMOCK_RESULT_(tn, F) ct Method(GMOCK_ARG_(tn, F, 1) gmock_a1) constness { \
    GMOCK_COMPILE_ASSERT_(::std::tr1::tuple_size< \
        tn ::testing::internal::Function<F>::ArgumentTuple>::value == 1, \
        this_method_does_not_take_1_argument); \
    GMOCK_MOCKER_(1, constness, Method).SetOwnerAndName(this, #Method); \
    return GMOCK_MOCKER_(1, constness, Method).Invoke(gmock_a1); \
  } \
  ::testing::MockSpec<F>& \
      gmock_##Method(GMOCK_MATCHER_(tn, F, 1) gmock_a1) constness { \
    return GMOCK_MOCKER_(1, constness, \
        Method).RegisterOwner(this).With(gmock_a1); \
  } \
  mutable ::testing::FunctionMocker<F> GMOCK_MOCKER_(1, constness, Method)

// INTERNAL IMPLEMENTATION - DON'T USE IN USER CODE!!!
#define GMOCK_METHOD2_(tn, constness, ct, Method, F) \
  GMOCK_RESULT_(tn, F) ct Method(GMOCK_ARG_(tn, F, 1) gmock_a1, \
                                 GMOCK_ARG_(tn, F, 2) gmock_a2) constness { \
    GMOCK_COMPILE_ASSERT_(::std::tr1::tuple_size< \
        tn ::testing::internal::Function<F>::ArgumentTuple>::value == 2, \
        this_method_does_not_take_2_arguments); \
    GMOCK_MOCKER_(2, constness, Method).SetOwnerAndName(this, #Method); \
    return GMOCK_MOCKER_(2, constness, Method).Invoke(gmock_a1, gmock_a2); \
  } \
  ::testing::MockSpec<F>& \
      gmock_##Method(GMOCK_MATCHER_(tn, F, 1) gmock_a1, \
                     GMOCK_MATCHER_(tn, F, 2) gmock_a2) constness { \
    return GMOCK_MOCKER_(2, constness, \
        Method).RegisterOwner(this).With(gmock_a1, gmock_a2); \
  } \
  mutable ::testing::FunctionMocker<F> GMOCK_MOCKER_(2, constness, Method)

// INTERNAL IMPLEMENTATION - DON'T USE IN USER CODE!!!
#define GMOCK_METHOD3_(tn, constness, ct, Method, F) \
  GMOCK_RESULT_(tn, F) ct Method(GMOCK_ARG_(tn, F, 1) gmock_a1, \
                                 GMOCK_ARG_(tn, F, 2) gmock_a2, \
                                 GMOCK_ARG_(tn, F, 3) gmock_a3) constness { \
    GMOCK_COMPILE_ASSERT_(::std::tr1::tuple_size< \
        tn ::testing::internal::Function<F>::ArgumentTuple>::value == 3, \
        this_method_does_not_take_3_arguments); \
    GMOCK_MOCKER_(3, constness, Method).SetOwnerAndName(this, #Method); \
    return GMOCK_MOCKER_(3, constness, Method).Invoke(gmock_a1, gmock_a2, \
        gmock_a3); \
  } \
  ::testing::MockSpec<F>& \
      gmock_##Method(GMOCK_MATCHER_(tn, F, 1) gmock_a1, \
                     GMOCK_MATCHER_(tn, F, 2) gmock_a2, \
                     GMOCK_MATCHER_(tn, F, 3) gmock_a3) constness { \
    return GMOCK_MOCKER_(3, constness, \
        Method).RegisterOwner(this).With(gmock_a1, gmock_a2, gmock_a3); \
  } \
  mutable ::testing::FunctionMocker<F> GMOCK_MOCKER_(3, constness, Method)

// INTERNAL IMPLEMENTATION - DON'T USE IN USER CODE!!!
#define GMOCK_METHOD4_(tn, constness, ct, Method, F) \
  GMOCK_RESULT_(tn, F) ct Method(GMOCK_ARG_(tn, F, 1) gmock_a1, \
                                 GMOCK_ARG_(tn, F, 2) gmock_a2, \
                                 GMOCK_ARG_(tn, F, 3) gmock_a3, \
                                 GMOCK_ARG_(tn, F, 4) gmock_a4) constness { \
    GMOCK_COMPILE_ASSERT_(::std::tr1::tuple_size< \
        tn ::testing::internal::Function<F>::ArgumentTuple>::value == 4, \
        this_method_does_not_take_4_arguments); \
    GMOCK_MOCKER_(4, constness, Method).SetOwnerAndName(this, #Method); \
    return GMOCK_MOCKER_(4, constness, Method).Invoke(gmock_a1, gmock_a2, \
        gmock_a3, gmock_a4); \
  } \
  ::testing::MockSpec<F>& \
      gmock_##Method(GMOCK_MATCHER_(tn, F, 1) gmock_a1, \
                     GMOCK_MATCHER_(tn, F, 2) gmock_a2, \
                     GMOCK_MATCHER_(tn, F, 3) gmock_a3, \
                     GMOCK_MATCHER_(tn, F, 4) gmock_a4) constness { \
    return GMOCK_MOCKER_(4, constness, \
        Method).RegisterOwner(this).With(gmock_a1, gmock_a2, gmock_a3, \
        gmock_a4); \
  } \
  mutable ::testing::FunctionMocker<F> GMOCK_MOCKER_(4, constness, Method)

// INTERNAL IMPLEMENTATION - DON'T USE IN USER CODE!!!
#define GMOCK_METHOD5_(tn, constness, ct, Method, F) \
  GMOCK_RESULT_(tn, F) ct Method(GMOCK_ARG_(tn, F, 1) gmock_a1, \
                                 GMOCK_ARG_(tn, F, 2) gmock_a2, \
                                 GMOCK_ARG_(tn, F, 3) gmock_a3, \
                                 GMOCK_ARG_(tn, F, 4) gmock_a4, \
                                 GMOCK_ARG_(tn, F, 5) gmock_a5) constness { \
    GMOCK_COMPILE_ASSERT_(::std::tr1::tuple_size< \
        tn ::testing::internal::Function<F>::ArgumentTuple>::value == 5, \
        this_method_does_not_take_5_arguments); \
    GMOCK_MOCKER_(5, constness, Method).SetOwnerAndName(this, #Method); \
    return GMOCK_MOCKER_(5, constness, Method).Invoke(gmock_a1, gmock_a2, \
        gmock_a3, gmock_a4, gmock_a5); \
  } \
  ::testing::MockSpec<F>& \
      gmock_##Method(GMOCK_MATCHER_(tn, F, 1) gmock_a1, \
                     GMOCK_MATCHER_(tn, F, 2) gmock_a2, \
                     GMOCK_MATCHER_(tn, F, 3) gmock_a3, \
                     GMOCK_MATCHER_(tn, F, 4) gmock_a4, \
                     GMOCK_MATCHER_(tn, F, 5) gmock_a5) constness { \
    return GMOCK_MOCKER_(5, constness, \
        Method).RegisterOwner(this).With(gmock_a1, gmock_a2, gmock_a3, \
        gmock_a4, gmock_a5); \
  } \
  mutable ::testing::FunctionMocker<F> GMOCK_MOCKER_(5, constness, Method)

// INTERNAL IMPLEMENTATION - DON'T USE IN USER CODE!!!
#define GMOCK_METHOD6_(tn, constness, ct, Method, F) \
  GMOCK_RESULT_(tn, F) ct Method(GMOCK_ARG_(tn, F, 1) gmock_a1, \
                                 GMOCK_ARG_(tn, F, 2) gmock_a2, \
                                 GMOCK_ARG_(tn, F, 3) gmock_a3, \
                                 GMOCK_ARG_(tn, F, 4) gmock_a4, \
                                 GMOCK_ARG_(tn, F, 5) gmock_a5, \
                                 GMOCK_ARG_(tn, F, 6) gmock_a6) constness { \
    GMOCK_COMPILE_ASSERT_(::std::tr1::tuple_size< \
        tn ::testing::internal::Function<F>::ArgumentTuple>::value == 6, \
        this_method_does_not_take_6_arguments); \
    GMOCK_MOCKER_(6, constness, Method).SetOwnerAndName(this, #Method); \
    return GMOCK_MOCKER_(6, constness, Method).Invoke(gmock_a1, gmock_a2, \
        gmock_a3, gmock_a4, gmock_a5, gmock_a6); \
  } \
  ::testing::MockSpec<F>& \
      gmock_##Method(GMOCK_MATCHER_(tn, F, 1) gmock_a1, \
                     GMOCK_MATCHER_(tn, F, 2) gmock_a2, \
                     GMOCK_MATCHER_(tn, F, 3) gmock_a3, \
                     GMOCK_MATCHER_(tn, F, 4) gmock_a4, \
                     GMOCK_MATCHER_(tn, F, 5) gmock_a5, \
                     GMOCK_MATCHER_(tn, F, 6) gmock_a6) constness { \
    return GMOCK_MOCKER_(6, constness, \
        Method).RegisterOwner(this).With(gmock_a1, gmock_a2, gmock_a3, \
        gmock_a4, gmock_a5, gmock_a6); \
  } \
  mutable ::testing::FunctionMocker<F> GMOCK_MOCKER_(6, constness, Method)

// INTERNAL IMPLEMENTATION - DON'T USE IN USER CODE!!!
#define GMOCK_METHOD7_(tn, constness, ct, Method, F) \
  GMOCK_RESULT_(tn, F) ct Method(GMOCK_ARG_(tn, F, 1) gmock_a1, \
                                 GMOCK_ARG_(tn, F, 2) gmock_a2, \
                                 GMOCK_ARG_(tn, F, 3) gmock_a3, \
                                 GMOCK_ARG_(tn, F, 4) gmock_a4, \
                                 GMOCK_ARG_(tn, F, 5) gmock_a5, \
                                 GMOCK_ARG_(tn, F, 6) gmock_a6, \
                                 GMOCK_ARG_(tn, F, 7) gmock_a7) constness { \
    GMOCK_COMPILE_ASSERT_(::std::tr1::tuple_size< \
        tn ::testing::internal::Function<F>::ArgumentTuple>::value == 7, \
        this_method_does_not_take_7_arguments); \
    GMOCK_MOCKER_(7, constness, Method).SetOwnerAndName(this, #Method); \
    return GMOCK_MOCKER_(7, constness, Method).Invoke(gmock_a1, gmock_a2, \
        gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7); \
  } \
  ::testing::MockSpec<F>& \
      gmock_##Method(GMOCK_MATCHER_(tn, F, 1) gmock_a1, \
                     GMOCK_MATCHER_(tn, F, 2) gmock_a2, \
                     GMOCK_MATCHER_(tn, F, 3) gmock_a3, \
                     GMOCK_MATCHER_(tn, F, 4) gmock_a4, \
                     GMOCK_MATCHER_(tn, F, 5) gmock_a5, \
                     GMOCK_MATCHER_(tn, F, 6) gmock_a6, \
                     GMOCK_MATCHER_(tn, F, 7) gmock_a7) constness { \
    return GMOCK_MOCKER_(7, constness, \
        Method).RegisterOwner(this).With(gmock_a1, gmock_a2, gmock_a3, \
        gmock_a4, gmock_a5, gmock_a6, gmock_a7); \
  } \
  mutable ::testing::FunctionMocker<F> GMOCK_MOCKER_(7, constness, Method)

// INTERNAL IMPLEMENTATION - DON'T USE IN USER CODE!!!
#define GMOCK_METHOD8_(tn, constness, ct, Method, F) \
  GMOCK_RESULT_(tn, F) ct Method(GMOCK_ARG_(tn, F, 1) gmock_a1, \
                                 GMOCK_ARG_(tn, F, 2) gmock_a2, \
                                 GMOCK_ARG_(tn, F, 3) gmock_a3, \
                                 GMOCK_ARG_(tn, F, 4) gmock_a4, \
                                 GMOCK_ARG_(tn, F, 5) gmock_a5, \
                                 GMOCK_ARG_(tn, F, 6) gmock_a6, \
                                 GMOCK_ARG_(tn, F, 7) gmock_a7, \
                                 GMOCK_ARG_(tn, F, 8) gmock_a8) constness { \
    GMOCK_COMPILE_ASSERT_(::std::tr1::tuple_size< \
        tn ::testing::internal::Function<F>::ArgumentTuple>::value == 8, \
        this_method_does_not_take_8_arguments); \
    GMOCK_MOCKER_(8, constness, Method).SetOwnerAndName(this, #Method); \
    return GMOCK_MOCKER_(8, constness, Method).Invoke(gmock_a1, gmock_a2, \
        gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8); \
  } \
  ::testing::MockSpec<F>& \
      gmock_##Method(GMOCK_MATCHER_(tn, F, 1) gmock_a1, \
                     GMOCK_MATCHER_(tn, F, 2) gmock_a2, \
                     GMOCK_MATCHER_(tn, F, 3) gmock_a3, \
                     GMOCK_MATCHER_(tn, F, 4) gmock_a4, \
                     GMOCK_MATCHER_(tn, F, 5) gmock_a5, \
                     GMOCK_MATCHER_(tn, F, 6) gmock_a6, \
                     GMOCK_MATCHER_(tn, F, 7) gmock_a7, \
                     GMOCK_MATCHER_(tn, F, 8) gmock_a8) constness { \
    return GMOCK_MOCKER_(8, constness, \
        Method).RegisterOwner(this).With(gmock_a1, gmock_a2, gmock_a3, \
        gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8); \
  } \
  mutable ::testing::FunctionMocker<F> GMOCK_MOCKER_(8, constness, Method)

// INTERNAL IMPLEMENTATION - DON'T USE IN USER CODE!!!
#define GMOCK_METHOD9_(tn, constness, ct, Method, F) \
  GMOCK_RESULT_(tn, F) ct Method(GMOCK_ARG_(tn, F, 1) gmock_a1, \
                                 GMOCK_ARG_(tn, F, 2) gmock_a2, \
                                 GMOCK_ARG_(tn, F, 3) gmock_a3, \
                                 GMOCK_ARG_(tn, F, 4) gmock_a4, \
                                 GMOCK_ARG_(tn, F, 5) gmock_a5, \
                                 GMOCK_ARG_(tn, F, 6) gmock_a6, \
                                 GMOCK_ARG_(tn, F, 7) gmock_a7, \
                                 GMOCK_ARG_(tn, F, 8) gmock_a8, \
                                 GMOCK_ARG_(tn, F, 9) gmock_a9) constness { \
    GMOCK_COMPILE_ASSERT_(::std::tr1::tuple_size< \
        tn ::testing::internal::Function<F>::ArgumentTuple>::value == 9, \
        this_method_does_not_take_9_arguments); \
    GMOCK_MOCKER_(9, constness, Method).SetOwnerAndName(this, #Method); \
    return GMOCK_MOCKER_(9, constness, Method).Invoke(gmock_a1, gmock_a2, \
        gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, \
        gmock_a9); \
  } \
  ::testing::MockSpec<F>& \
      gmock_##Method(GMOCK_MATCHER_(tn, F, 1) gmock_a1, \
                     GMOCK_MATCHER_(tn, F, 2) gmock_a2, \
                     GMOCK_MATCHER_(tn, F, 3) gmock_a3, \
                     GMOCK_MATCHER_(tn, F, 4) gmock_a4, \
                     GMOCK_MATCHER_(tn, F, 5) gmock_a5, \
                     GMOCK_MATCHER_(tn, F, 6) gmock_a6, \
                     GMOCK_MATCHER_(tn, F, 7) gmock_a7, \
                     GMOCK_MATCHER_(tn, F, 8) gmock_a8, \
                     GMOCK_MATCHER_(tn, F, 9) gmock_a9) constness { \
    return GMOCK_MOCKER_(9, constness, \
        Method).RegisterOwner(this).With(gmock_a1, gmock_a2, gmock_a3, \
        gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, gmock_a9); \
  } \
  mutable ::testing::FunctionMocker<F> GMOCK_MOCKER_(9, constness, Method)

// INTERNAL IMPLEMENTATION - DON'T USE IN USER CODE!!!
#define GMOCK_METHOD10_(tn, constness, ct, Method, F) \
  GMOCK_RESULT_(tn, F) ct Method(GMOCK_ARG_(tn, F, 1) gmock_a1, \
                                 GMOCK_ARG_(tn, F, 2) gmock_a2, \
                                 GMOCK_ARG_(tn, F, 3) gmock_a3, \
                                 GMOCK_ARG_(tn, F, 4) gmock_a4, \
                                 GMOCK_ARG_(tn, F, 5) gmock_a5, \
                                 GMOCK_ARG_(tn, F, 6) gmock_a6, \
                                 GMOCK_ARG_(tn, F, 7) gmock_a7, \
                                 GMOCK_ARG_(tn, F, 8) gmock_a8, \
                                 GMOCK_ARG_(tn, F, 9) gmock_a9, \
                                 GMOCK_ARG_(tn, F, 10) gmock_a10) constness { \
    GMOCK_COMPILE_ASSERT_(::std::tr1::tuple_size< \
        tn ::testing::internal::Function<F>::ArgumentTuple>::value == 10, \
        this_method_does_not_take_10_arguments); \
    GMOCK_MOCKER_(10, constness, Method).SetOwnerAndName(this, #Method); \
    return GMOCK_MOCKER_(10, constness, Method).Invoke(gmock_a1, gmock_a2, \
        gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, gmock_a9, \
        gmock_a10); \
  } \
  ::testing::MockSpec<F>& \
      gmock_##Method(GMOCK_MATCHER_(tn, F, 1) gmock_a1, \
                     GMOCK_MATCHER_(tn, F, 2) gmock_a2, \
                     GMOCK_MATCHER_(tn, F, 3) gmock_a3, \
                     GMOCK_MATCHER_(tn, F, 4) gmock_a4, \
                     GMOCK_MATCHER_(tn, F, 5) gmock_a5, \
                     GMOCK_MATCHER_(tn, F, 6) gmock_a6, \
                     GMOCK_MATCHER_(tn, F, 7) gmock_a7, \
                     GMOCK_MATCHER_(tn, F, 8) gmock_a8, \
                     GMOCK_MATCHER_(tn, F, 9) gmock_a9, \
                     GMOCK_MATCHER_(tn, F, 10) gmock_a10) constness { \
    return GMOCK_MOCKER_(10, constness, \
        Method).RegisterOwner(this).With(gmock_a1, gmock_a2, gmock_a3, \
        gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, gmock_a9, \
        gmock_a10); \
  } \
  mutable ::testing::FunctionMocker<F> GMOCK_MOCKER_(10, constness, Method)

#define MOCK_METHOD0(m, F) GMOCK_METHOD0_(, , , m, F)
#define MOCK_METHOD1(m, F) GMOCK_METHOD1_(, , , m, F)
#define MOCK_METHOD2(m, F) GMOCK_METHOD2_(, , , m, F)
#define MOCK_METHOD3(m, F) GMOCK_METHOD3_(, , , m, F)
#define MOCK_METHOD4(m, F) GMOCK_METHOD4_(, , , m, F)
#define MOCK_METHOD5(m, F) GMOCK_METHOD5_(, , , m, F)
#define MOCK_METHOD6(m, F) GMOCK_METHOD6_(, , , m, F)
#define MOCK_METHOD7(m, F) GMOCK_METHOD7_(, , , m, F)
#define MOCK_METHOD8(m, F) GMOCK_METHOD8_(, , , m, F)
#define MOCK_METHOD9(m, F) GMOCK_METHOD9_(, , , m, F)
#define MOCK_METHOD10(m, F) GMOCK_METHOD10_(, , , m, F)

#define MOCK_CONST_METHOD0(m, F) GMOCK_METHOD0_(, const, , m, F)
#define MOCK_CONST_METHOD1(m, F) GMOCK_METHOD1_(, const, , m, F)
#define MOCK_CONST_METHOD2(m, F) GMOCK_METHOD2_(, const, , m, F)
#define MOCK_CONST_METHOD3(m, F) GMOCK_METHOD3_(, const, , m, F)
#define MOCK_CONST_METHOD4(m, F) GMOCK_METHOD4_(, const, , m, F)
#define MOCK_CONST_METHOD5(m, F) GMOCK_METHOD5_(, const, , m, F)
#define MOCK_CONST_METHOD6(m, F) GMOCK_METHOD6_(, const, , m, F)
#define MOCK_CONST_METHOD7(m, F) GMOCK_METHOD7_(, const, , m, F)
#define MOCK_CONST_METHOD8(m, F) GMOCK_METHOD8_(, const, , m, F)
#define MOCK_CONST_METHOD9(m, F) GMOCK_METHOD9_(, const, , m, F)
#define MOCK_CONST_METHOD10(m, F) GMOCK_METHOD10_(, const, , m, F)

#define MOCK_METHOD0_T(m, F) GMOCK_METHOD0_(typename, , , m, F)
#define MOCK_METHOD1_T(m, F) GMOCK_METHOD1_(typename, , , m, F)
#define MOCK_METHOD2_T(m, F) GMOCK_METHOD2_(typename, , , m, F)
#define MOCK_METHOD3_T(m, F) GMOCK_METHOD3_(typename, , , m, F)
#define MOCK_METHOD4_T(m, F) GMOCK_METHOD4_(typename, , , m, F)
#define MOCK_METHOD5_T(m, F) GMOCK_METHOD5_(typename, , , m, F)
#define MOCK_METHOD6_T(m, F) GMOCK_METHOD6_(typename, , , m, F)
#define MOCK_METHOD7_T(m, F) GMOCK_METHOD7_(typename, , , m, F)
#define MOCK_METHOD8_T(m, F) GMOCK_METHOD8_(typename, , , m, F)
#define MOCK_METHOD9_T(m, F) GMOCK_METHOD9_(typename, , , m, F)
#define MOCK_METHOD10_T(m, F) GMOCK_METHOD10_(typename, , , m, F)

#define MOCK_CONST_METHOD0_T(m, F) GMOCK_METHOD0_(typename, const, , m, F)
#define MOCK_CONST_METHOD1_T(m, F) GMOCK_METHOD1_(typename, const, , m, F)
#define MOCK_CONST_METHOD2_T(m, F) GMOCK_METHOD2_(typename, const, , m, F)
#define MOCK_CONST_METHOD3_T(m, F) GMOCK_METHOD3_(typename, const, , m, F)
#define MOCK_CONST_METHOD4_T(m, F) GMOCK_METHOD4_(typename, const, , m, F)
#define MOCK_CONST_METHOD5_T(m, F) GMOCK_METHOD5_(typename, const, , m, F)
#define MOCK_CONST_METHOD6_T(m, F) GMOCK_METHOD6_(typename, const, , m, F)
#define MOCK_CONST_METHOD7_T(m, F) GMOCK_METHOD7_(typename, const, , m, F)
#define MOCK_CONST_METHOD8_T(m, F) GMOCK_METHOD8_(typename, const, , m, F)
#define MOCK_CONST_METHOD9_T(m, F) GMOCK_METHOD9_(typename, const, , m, F)
#define MOCK_CONST_METHOD10_T(m, F) GMOCK_METHOD10_(typename, const, , m, F)

#define MOCK_METHOD0_WITH_CALLTYPE(ct, m, F) GMOCK_METHOD0_(, , ct, m, F)
#define MOCK_METHOD1_WITH_CALLTYPE(ct, m, F) GMOCK_METHOD1_(, , ct, m, F)
#define MOCK_METHOD2_WITH_CALLTYPE(ct, m, F) GMOCK_METHOD2_(, , ct, m, F)
#define MOCK_METHOD3_WITH_CALLTYPE(ct, m, F) GMOCK_METHOD3_(, , ct, m, F)
#define MOCK_METHOD4_WITH_CALLTYPE(ct, m, F) GMOCK_METHOD4_(, , ct, m, F)
#define MOCK_METHOD5_WITH_CALLTYPE(ct, m, F) GMOCK_METHOD5_(, , ct, m, F)
#define MOCK_METHOD6_WITH_CALLTYPE(ct, m, F) GMOCK_METHOD6_(, , ct, m, F)
#define MOCK_METHOD7_WITH_CALLTYPE(ct, m, F) GMOCK_METHOD7_(, , ct, m, F)
#define MOCK_METHOD8_WITH_CALLTYPE(ct, m, F) GMOCK_METHOD8_(, , ct, m, F)
#define MOCK_METHOD9_WITH_CALLTYPE(ct, m, F) GMOCK_METHOD9_(, , ct, m, F)
#define MOCK_METHOD10_WITH_CALLTYPE(ct, m, F) GMOCK_METHOD10_(, , ct, m, F)

#define MOCK_CONST_METHOD0_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD0_(, const, ct, m, F)
#define MOCK_CONST_METHOD1_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD1_(, const, ct, m, F)
#define MOCK_CONST_METHOD2_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD2_(, const, ct, m, F)
#define MOCK_CONST_METHOD3_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD3_(, const, ct, m, F)
#define MOCK_CONST_METHOD4_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD4_(, const, ct, m, F)
#define MOCK_CONST_METHOD5_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD5_(, const, ct, m, F)
#define MOCK_CONST_METHOD6_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD6_(, const, ct, m, F)
#define MOCK_CONST_METHOD7_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD7_(, const, ct, m, F)
#define MOCK_CONST_METHOD8_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD8_(, const, ct, m, F)
#define MOCK_CONST_METHOD9_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD9_(, const, ct, m, F)
#define MOCK_CONST_METHOD10_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD10_(, const, ct, m, F)

#define MOCK_METHOD0_T_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD0_(typename, , ct, m, F)
#define MOCK_METHOD1_T_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD1_(typename, , ct, m, F)
#define MOCK_METHOD2_T_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD2_(typename, , ct, m, F)
#define MOCK_METHOD3_T_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD3_(typename, , ct, m, F)
#define MOCK_METHOD4_T_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD4_(typename, , ct, m, F)
#define MOCK_METHOD5_T_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD5_(typename, , ct, m, F)
#define MOCK_METHOD6_T_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD6_(typename, , ct, m, F)
#define MOCK_METHOD7_T_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD7_(typename, , ct, m, F)
#define MOCK_METHOD8_T_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD8_(typename, , ct, m, F)
#define MOCK_METHOD9_T_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD9_(typename, , ct, m, F)
#define MOCK_METHOD10_T_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD10_(typename, , ct, m, F)

#define MOCK_CONST_METHOD0_T_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD0_(typename, const, ct, m, F)
#define MOCK_CONST_METHOD1_T_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD1_(typename, const, ct, m, F)
#define MOCK_CONST_METHOD2_T_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD2_(typename, const, ct, m, F)
#define MOCK_CONST_METHOD3_T_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD3_(typename, const, ct, m, F)
#define MOCK_CONST_METHOD4_T_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD4_(typename, const, ct, m, F)
#define MOCK_CONST_METHOD5_T_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD5_(typename, const, ct, m, F)
#define MOCK_CONST_METHOD6_T_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD6_(typename, const, ct, m, F)
#define MOCK_CONST_METHOD7_T_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD7_(typename, const, ct, m, F)
#define MOCK_CONST_METHOD8_T_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD8_(typename, const, ct, m, F)
#define MOCK_CONST_METHOD9_T_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD9_(typename, const, ct, m, F)
#define MOCK_CONST_METHOD10_T_WITH_CALLTYPE(ct, m, F) \
    GMOCK_METHOD10_(typename, const, ct, m, F)

// A MockFunction<F> class has one mock method whose type is F.  It is
// useful when you just want your test code to emit some messages and
// have Google Mock verify the right messages are sent (and perhaps at
// the right times).  For example, if you are exercising code:
//
//   Foo(1);
//   Foo(2);
//   Foo(3);
//
// and want to verify that Foo(1) and Foo(3) both invoke
// mock.Bar("a"), but Foo(2) doesn't invoke anything, you can write:
//
// TEST(FooTest, InvokesBarCorrectly) {
//   MyMock mock;
//   MockFunction<void(string check_point_name)> check;
//   {
//     InSequence s;
//
//     EXPECT_CALL(mock, Bar("a"));
//     EXPECT_CALL(check, Call("1"));
//     EXPECT_CALL(check, Call("2"));
//     EXPECT_CALL(mock, Bar("a"));
//   }
//   Foo(1);
//   check.Call("1");
//   Foo(2);
//   check.Call("2");
//   Foo(3);
// }
//
// The expectation spec says that the first Bar("a") must happen
// before check point "1", the second Bar("a") must happen after check
// point "2", and nothing should happen between the two check
// points. The explicit check points make it easy to tell which
// Bar("a") is called by which call to Foo().
template <typename F>
class MockFunction;

template <typename R>
class MockFunction<R()> {
 public:
  MockFunction() {}

  MOCK_METHOD0_T(Call, R());

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(MockFunction);
};

template <typename R, typename A0>
class MockFunction<R(A0)> {
 public:
  MockFunction() {}

  MOCK_METHOD1_T(Call, R(A0));

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(MockFunction);
};

template <typename R, typename A0, typename A1>
class MockFunction<R(A0, A1)> {
 public:
  MockFunction() {}

  MOCK_METHOD2_T(Call, R(A0, A1));

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(MockFunction);
};

template <typename R, typename A0, typename A1, typename A2>
class MockFunction<R(A0, A1, A2)> {
 public:
  MockFunction() {}

  MOCK_METHOD3_T(Call, R(A0, A1, A2));

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(MockFunction);
};

template <typename R, typename A0, typename A1, typename A2, typename A3>
class MockFunction<R(A0, A1, A2, A3)> {
 public:
  MockFunction() {}

  MOCK_METHOD4_T(Call, R(A0, A1, A2, A3));

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(MockFunction);
};

template <typename R, typename A0, typename A1, typename A2, typename A3,
    typename A4>
class MockFunction<R(A0, A1, A2, A3, A4)> {
 public:
  MockFunction() {}

  MOCK_METHOD5_T(Call, R(A0, A1, A2, A3, A4));

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(MockFunction);
};

template <typename R, typename A0, typename A1, typename A2, typename A3,
    typename A4, typename A5>
class MockFunction<R(A0, A1, A2, A3, A4, A5)> {
 public:
  MockFunction() {}

  MOCK_METHOD6_T(Call, R(A0, A1, A2, A3, A4, A5));

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(MockFunction);
};

template <typename R, typename A0, typename A1, typename A2, typename A3,
    typename A4, typename A5, typename A6>
class MockFunction<R(A0, A1, A2, A3, A4, A5, A6)> {
 public:
  MockFunction() {}

  MOCK_METHOD7_T(Call, R(A0, A1, A2, A3, A4, A5, A6));

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(MockFunction);
};

template <typename R, typename A0, typename A1, typename A2, typename A3,
    typename A4, typename A5, typename A6, typename A7>
class MockFunction<R(A0, A1, A2, A3, A4, A5, A6, A7)> {
 public:
  MockFunction() {}

  MOCK_METHOD8_T(Call, R(A0, A1, A2, A3, A4, A5, A6, A7));

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(MockFunction);
};

template <typename R, typename A0, typename A1, typename A2, typename A3,
    typename A4, typename A5, typename A6, typename A7, typename A8>
class MockFunction<R(A0, A1, A2, A3, A4, A5, A6, A7, A8)> {
 public:
  MockFunction() {}

  MOCK_METHOD9_T(Call, R(A0, A1, A2, A3, A4, A5, A6, A7, A8));

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(MockFunction);
};

template <typename R, typename A0, typename A1, typename A2, typename A3,
    typename A4, typename A5, typename A6, typename A7, typename A8,
    typename A9>
class MockFunction<R(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)> {
 public:
  MockFunction() {}

  MOCK_METHOD10_T(Call, R(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9));

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(MockFunction);
};

}  // namespace testing

#endif  // GMOCK_INCLUDE_GMOCK_GMOCK_GENERATED_FUNCTION_MOCKERS_H_
// This file was GENERATED by command:
//     pump.py gmock-generated-matchers.h.pump
// DO NOT EDIT BY HAND!!!

// Copyright 2008, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Google Mock - a framework for writing C++ mock classes.
//
// This file implements some commonly used variadic matchers.

#ifndef GMOCK_INCLUDE_GMOCK_GMOCK_GENERATED_MATCHERS_H_
#define GMOCK_INCLUDE_GMOCK_GMOCK_GENERATED_MATCHERS_H_

#include <sstream>
#include <string>
#include <vector>

namespace testing {
namespace internal {

// The type of the i-th (0-based) field of Tuple.
#define GMOCK_FIELD_TYPE_(Tuple, i) \
    typename ::std::tr1::tuple_element<i, Tuple>::type

// TupleFields<Tuple, k0, ..., kn> is for selecting fields from a
// tuple of type Tuple.  It has two members:
//
//   type: a tuple type whose i-th field is the ki-th field of Tuple.
//   GetSelectedFields(t): returns fields k0, ..., and kn of t as a tuple.
//
// For example, in class TupleFields<tuple<bool, char, int>, 2, 0>, we have:
//
//   type is tuple<int, bool>, and
//   GetSelectedFields(make_tuple(true, 'a', 42)) is (42, true).

template <class Tuple, int k0 = -1, int k1 = -1, int k2 = -1, int k3 = -1,
    int k4 = -1, int k5 = -1, int k6 = -1, int k7 = -1, int k8 = -1,
    int k9 = -1>
class TupleFields;

// This generic version is used when there are 10 selectors.
template <class Tuple, int k0, int k1, int k2, int k3, int k4, int k5, int k6,
    int k7, int k8, int k9>
class TupleFields {
 public:
  typedef ::std::tr1::tuple<GMOCK_FIELD_TYPE_(Tuple, k0),
      GMOCK_FIELD_TYPE_(Tuple, k1), GMOCK_FIELD_TYPE_(Tuple, k2),
      GMOCK_FIELD_TYPE_(Tuple, k3), GMOCK_FIELD_TYPE_(Tuple, k4),
      GMOCK_FIELD_TYPE_(Tuple, k5), GMOCK_FIELD_TYPE_(Tuple, k6),
      GMOCK_FIELD_TYPE_(Tuple, k7), GMOCK_FIELD_TYPE_(Tuple, k8),
      GMOCK_FIELD_TYPE_(Tuple, k9)> type;
  static type GetSelectedFields(const Tuple& t) {
    using ::std::tr1::get;
    return type(get<k0>(t), get<k1>(t), get<k2>(t), get<k3>(t), get<k4>(t),
        get<k5>(t), get<k6>(t), get<k7>(t), get<k8>(t), get<k9>(t));
  }
};

// The following specialization is used for 0 ~ 9 selectors.

template <class Tuple>
class TupleFields<Tuple, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1> {
 public:
  typedef ::std::tr1::tuple<> type;
  static type GetSelectedFields(const Tuple& /* t */) {
    using ::std::tr1::get;
    return type();
  }
};

template <class Tuple, int k0>
class TupleFields<Tuple, k0, -1, -1, -1, -1, -1, -1, -1, -1, -1> {
 public:
  typedef ::std::tr1::tuple<GMOCK_FIELD_TYPE_(Tuple, k0)> type;
  static type GetSelectedFields(const Tuple& t) {
    using ::std::tr1::get;
    return type(get<k0>(t));
  }
};

template <class Tuple, int k0, int k1>
class TupleFields<Tuple, k0, k1, -1, -1, -1, -1, -1, -1, -1, -1> {
 public:
  typedef ::std::tr1::tuple<GMOCK_FIELD_TYPE_(Tuple, k0),
      GMOCK_FIELD_TYPE_(Tuple, k1)> type;
  static type GetSelectedFields(const Tuple& t) {
    using ::std::tr1::get;
    return type(get<k0>(t), get<k1>(t));
  }
};

template <class Tuple, int k0, int k1, int k2>
class TupleFields<Tuple, k0, k1, k2, -1, -1, -1, -1, -1, -1, -1> {
 public:
  typedef ::std::tr1::tuple<GMOCK_FIELD_TYPE_(Tuple, k0),
      GMOCK_FIELD_TYPE_(Tuple, k1), GMOCK_FIELD_TYPE_(Tuple, k2)> type;
  static type GetSelectedFields(const Tuple& t) {
    using ::std::tr1::get;
    return type(get<k0>(t), get<k1>(t), get<k2>(t));
  }
};

template <class Tuple, int k0, int k1, int k2, int k3>
class TupleFields<Tuple, k0, k1, k2, k3, -1, -1, -1, -1, -1, -1> {
 public:
  typedef ::std::tr1::tuple<GMOCK_FIELD_TYPE_(Tuple, k0),
      GMOCK_FIELD_TYPE_(Tuple, k1), GMOCK_FIELD_TYPE_(Tuple, k2),
      GMOCK_FIELD_TYPE_(Tuple, k3)> type;
  static type GetSelectedFields(const Tuple& t) {
    using ::std::tr1::get;
    return type(get<k0>(t), get<k1>(t), get<k2>(t), get<k3>(t));
  }
};

template <class Tuple, int k0, int k1, int k2, int k3, int k4>
class TupleFields<Tuple, k0, k1, k2, k3, k4, -1, -1, -1, -1, -1> {
 public:
  typedef ::std::tr1::tuple<GMOCK_FIELD_TYPE_(Tuple, k0),
      GMOCK_FIELD_TYPE_(Tuple, k1), GMOCK_FIELD_TYPE_(Tuple, k2),
      GMOCK_FIELD_TYPE_(Tuple, k3), GMOCK_FIELD_TYPE_(Tuple, k4)> type;
  static type GetSelectedFields(const Tuple& t) {
    using ::std::tr1::get;
    return type(get<k0>(t), get<k1>(t), get<k2>(t), get<k3>(t), get<k4>(t));
  }
};

template <class Tuple, int k0, int k1, int k2, int k3, int k4, int k5>
class TupleFields<Tuple, k0, k1, k2, k3, k4, k5, -1, -1, -1, -1> {
 public:
  typedef ::std::tr1::tuple<GMOCK_FIELD_TYPE_(Tuple, k0),
      GMOCK_FIELD_TYPE_(Tuple, k1), GMOCK_FIELD_TYPE_(Tuple, k2),
      GMOCK_FIELD_TYPE_(Tuple, k3), GMOCK_FIELD_TYPE_(Tuple, k4),
      GMOCK_FIELD_TYPE_(Tuple, k5)> type;
  static type GetSelectedFields(const Tuple& t) {
    using ::std::tr1::get;
    return type(get<k0>(t), get<k1>(t), get<k2>(t), get<k3>(t), get<k4>(t),
        get<k5>(t));
  }
};

template <class Tuple, int k0, int k1, int k2, int k3, int k4, int k5, int k6>
class TupleFields<Tuple, k0, k1, k2, k3, k4, k5, k6, -1, -1, -1> {
 public:
  typedef ::std::tr1::tuple<GMOCK_FIELD_TYPE_(Tuple, k0),
      GMOCK_FIELD_TYPE_(Tuple, k1), GMOCK_FIELD_TYPE_(Tuple, k2),
      GMOCK_FIELD_TYPE_(Tuple, k3), GMOCK_FIELD_TYPE_(Tuple, k4),
      GMOCK_FIELD_TYPE_(Tuple, k5), GMOCK_FIELD_TYPE_(Tuple, k6)> type;
  static type GetSelectedFields(const Tuple& t) {
    using ::std::tr1::get;
    return type(get<k0>(t), get<k1>(t), get<k2>(t), get<k3>(t), get<k4>(t),
        get<k5>(t), get<k6>(t));
  }
};

template <class Tuple, int k0, int k1, int k2, int k3, int k4, int k5, int k6,
    int k7>
class TupleFields<Tuple, k0, k1, k2, k3, k4, k5, k6, k7, -1, -1> {
 public:
  typedef ::std::tr1::tuple<GMOCK_FIELD_TYPE_(Tuple, k0),
      GMOCK_FIELD_TYPE_(Tuple, k1), GMOCK_FIELD_TYPE_(Tuple, k2),
      GMOCK_FIELD_TYPE_(Tuple, k3), GMOCK_FIELD_TYPE_(Tuple, k4),
      GMOCK_FIELD_TYPE_(Tuple, k5), GMOCK_FIELD_TYPE_(Tuple, k6),
      GMOCK_FIELD_TYPE_(Tuple, k7)> type;
  static type GetSelectedFields(const Tuple& t) {
    using ::std::tr1::get;
    return type(get<k0>(t), get<k1>(t), get<k2>(t), get<k3>(t), get<k4>(t),
        get<k5>(t), get<k6>(t), get<k7>(t));
  }
};

template <class Tuple, int k0, int k1, int k2, int k3, int k4, int k5, int k6,
    int k7, int k8>
class TupleFields<Tuple, k0, k1, k2, k3, k4, k5, k6, k7, k8, -1> {
 public:
  typedef ::std::tr1::tuple<GMOCK_FIELD_TYPE_(Tuple, k0),
      GMOCK_FIELD_TYPE_(Tuple, k1), GMOCK_FIELD_TYPE_(Tuple, k2),
      GMOCK_FIELD_TYPE_(Tuple, k3), GMOCK_FIELD_TYPE_(Tuple, k4),
      GMOCK_FIELD_TYPE_(Tuple, k5), GMOCK_FIELD_TYPE_(Tuple, k6),
      GMOCK_FIELD_TYPE_(Tuple, k7), GMOCK_FIELD_TYPE_(Tuple, k8)> type;
  static type GetSelectedFields(const Tuple& t) {
    using ::std::tr1::get;
    return type(get<k0>(t), get<k1>(t), get<k2>(t), get<k3>(t), get<k4>(t),
        get<k5>(t), get<k6>(t), get<k7>(t), get<k8>(t));
  }
};

#undef GMOCK_FIELD_TYPE_

// Implements the Args() matcher.
template <class ArgsTuple, int k0 = -1, int k1 = -1, int k2 = -1, int k3 = -1,
    int k4 = -1, int k5 = -1, int k6 = -1, int k7 = -1, int k8 = -1,
    int k9 = -1>
class ArgsMatcherImpl : public MatcherInterface<ArgsTuple> {
 public:
  // ArgsTuple may have top-level const or reference modifiers.
  typedef GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(ArgsTuple)) RawArgsTuple;
  typedef typename internal::TupleFields<RawArgsTuple, k0, k1, k2, k3, k4, k5,
      k6, k7, k8, k9>::type SelectedArgs;
  typedef Matcher<const SelectedArgs&> MonomorphicInnerMatcher;

  template <typename InnerMatcher>
  explicit ArgsMatcherImpl(const InnerMatcher& inner_matcher)
      : inner_matcher_(SafeMatcherCast<const SelectedArgs&>(inner_matcher)) {}

  virtual bool MatchAndExplain(ArgsTuple args,
                               MatchResultListener* listener) const {
    const SelectedArgs& selected_args = GetSelectedArgs(args);
    if (!listener->IsInterested())
      return inner_matcher_.Matches(selected_args);

    PrintIndices(listener->stream());
    *listener << "are " << PrintToString(selected_args);

    StringMatchResultListener inner_listener;
    const bool match = inner_matcher_.MatchAndExplain(selected_args,
                                                      &inner_listener);
    PrintIfNotEmpty(inner_listener.str(), listener->stream());
    return match;
  }

  virtual void DescribeTo(::std::ostream* os) const {
    *os << "are a tuple ";
    PrintIndices(os);
    inner_matcher_.DescribeTo(os);
  }

  virtual void DescribeNegationTo(::std::ostream* os) const {
    *os << "are a tuple ";
    PrintIndices(os);
    inner_matcher_.DescribeNegationTo(os);
  }

 private:
  static SelectedArgs GetSelectedArgs(ArgsTuple args) {
    return TupleFields<RawArgsTuple, k0, k1, k2, k3, k4, k5, k6, k7, k8,
        k9>::GetSelectedFields(args);
  }

  // Prints the indices of the selected fields.
  static void PrintIndices(::std::ostream* os) {
    *os << "whose fields (";
    const int indices[10] = { k0, k1, k2, k3, k4, k5, k6, k7, k8, k9 };
    for (int i = 0; i < 10; i++) {
      if (indices[i] < 0)
        break;

      if (i >= 1)
        *os << ", ";

      *os << "#" << indices[i];
    }
    *os << ") ";
  }

  const MonomorphicInnerMatcher inner_matcher_;

  GTEST_DISALLOW_ASSIGN_(ArgsMatcherImpl);
};

template <class InnerMatcher, int k0 = -1, int k1 = -1, int k2 = -1,
    int k3 = -1, int k4 = -1, int k5 = -1, int k6 = -1, int k7 = -1,
    int k8 = -1, int k9 = -1>
class ArgsMatcher {
 public:
  explicit ArgsMatcher(const InnerMatcher& inner_matcher)
      : inner_matcher_(inner_matcher) {}

  template <typename ArgsTuple>
  operator Matcher<ArgsTuple>() const {
    return MakeMatcher(new ArgsMatcherImpl<ArgsTuple, k0, k1, k2, k3, k4, k5,
        k6, k7, k8, k9>(inner_matcher_));
  }

 private:
  const InnerMatcher inner_matcher_;

  GTEST_DISALLOW_ASSIGN_(ArgsMatcher);
};

// Implements ElementsAre() of 1-10 arguments.

template <typename T1>
class ElementsAreMatcher1 {
 public:
  explicit ElementsAreMatcher1(const T1& e1) : e1_(e1) {}

  template <typename Container>
  operator Matcher<Container>() const {
    typedef GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(Container))
        RawContainer;
    typedef typename internal::StlContainerView<RawContainer>::type::value_type
        Element;

    // Nokia's Symbian Compiler has a nasty bug where the object put
    // in a one-element local array is not destructed when the array
    // goes out of scope.  This leads to obvious badness as we've
    // added the linked_ptr in it to our other linked_ptrs list.
    // Hence we implement ElementsAreMatcher1 specially to avoid using
    // a local array.
    const Matcher<const Element&> matcher =
        MatcherCast<const Element&>(e1_);
    return MakeMatcher(new ElementsAreMatcherImpl<Container>(&matcher, 1));
  }

 private:
  const T1& e1_;

  GTEST_DISALLOW_ASSIGN_(ElementsAreMatcher1);
};

template <typename T1, typename T2>
class ElementsAreMatcher2 {
 public:
  ElementsAreMatcher2(const T1& e1, const T2& e2) : e1_(e1), e2_(e2) {}

  template <typename Container>
  operator Matcher<Container>() const {
    typedef GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(Container))
        RawContainer;
    typedef typename internal::StlContainerView<RawContainer>::type::value_type
        Element;

    const Matcher<const Element&> matchers[] = {
      MatcherCast<const Element&>(e1_),
      MatcherCast<const Element&>(e2_),
    };

    return MakeMatcher(new ElementsAreMatcherImpl<Container>(matchers, 2));
  }

 private:
  const T1& e1_;
  const T2& e2_;

  GTEST_DISALLOW_ASSIGN_(ElementsAreMatcher2);
};

template <typename T1, typename T2, typename T3>
class ElementsAreMatcher3 {
 public:
  ElementsAreMatcher3(const T1& e1, const T2& e2, const T3& e3) : e1_(e1),
      e2_(e2), e3_(e3) {}

  template <typename Container>
  operator Matcher<Container>() const {
    typedef GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(Container))
        RawContainer;
    typedef typename internal::StlContainerView<RawContainer>::type::value_type
        Element;

    const Matcher<const Element&> matchers[] = {
      MatcherCast<const Element&>(e1_),
      MatcherCast<const Element&>(e2_),
      MatcherCast<const Element&>(e3_),
    };

    return MakeMatcher(new ElementsAreMatcherImpl<Container>(matchers, 3));
  }

 private:
  const T1& e1_;
  const T2& e2_;
  const T3& e3_;

  GTEST_DISALLOW_ASSIGN_(ElementsAreMatcher3);
};

template <typename T1, typename T2, typename T3, typename T4>
class ElementsAreMatcher4 {
 public:
  ElementsAreMatcher4(const T1& e1, const T2& e2, const T3& e3,
      const T4& e4) : e1_(e1), e2_(e2), e3_(e3), e4_(e4) {}

  template <typename Container>
  operator Matcher<Container>() const {
    typedef GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(Container))
        RawContainer;
    typedef typename internal::StlContainerView<RawContainer>::type::value_type
        Element;

    const Matcher<const Element&> matchers[] = {
      MatcherCast<const Element&>(e1_),
      MatcherCast<const Element&>(e2_),
      MatcherCast<const Element&>(e3_),
      MatcherCast<const Element&>(e4_),
    };

    return MakeMatcher(new ElementsAreMatcherImpl<Container>(matchers, 4));
  }

 private:
  const T1& e1_;
  const T2& e2_;
  const T3& e3_;
  const T4& e4_;

  GTEST_DISALLOW_ASSIGN_(ElementsAreMatcher4);
};

template <typename T1, typename T2, typename T3, typename T4, typename T5>
class ElementsAreMatcher5 {
 public:
  ElementsAreMatcher5(const T1& e1, const T2& e2, const T3& e3, const T4& e4,
      const T5& e5) : e1_(e1), e2_(e2), e3_(e3), e4_(e4), e5_(e5) {}

  template <typename Container>
  operator Matcher<Container>() const {
    typedef GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(Container))
        RawContainer;
    typedef typename internal::StlContainerView<RawContainer>::type::value_type
        Element;

    const Matcher<const Element&> matchers[] = {
      MatcherCast<const Element&>(e1_),
      MatcherCast<const Element&>(e2_),
      MatcherCast<const Element&>(e3_),
      MatcherCast<const Element&>(e4_),
      MatcherCast<const Element&>(e5_),
    };

    return MakeMatcher(new ElementsAreMatcherImpl<Container>(matchers, 5));
  }

 private:
  const T1& e1_;
  const T2& e2_;
  const T3& e3_;
  const T4& e4_;
  const T5& e5_;

  GTEST_DISALLOW_ASSIGN_(ElementsAreMatcher5);
};

template <typename T1, typename T2, typename T3, typename T4, typename T5,
    typename T6>
class ElementsAreMatcher6 {
 public:
  ElementsAreMatcher6(const T1& e1, const T2& e2, const T3& e3, const T4& e4,
      const T5& e5, const T6& e6) : e1_(e1), e2_(e2), e3_(e3), e4_(e4),
      e5_(e5), e6_(e6) {}

  template <typename Container>
  operator Matcher<Container>() const {
    typedef GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(Container))
        RawContainer;
    typedef typename internal::StlContainerView<RawContainer>::type::value_type
        Element;

    const Matcher<const Element&> matchers[] = {
      MatcherCast<const Element&>(e1_),
      MatcherCast<const Element&>(e2_),
      MatcherCast<const Element&>(e3_),
      MatcherCast<const Element&>(e4_),
      MatcherCast<const Element&>(e5_),
      MatcherCast<const Element&>(e6_),
    };

    return MakeMatcher(new ElementsAreMatcherImpl<Container>(matchers, 6));
  }

 private:
  const T1& e1_;
  const T2& e2_;
  const T3& e3_;
  const T4& e4_;
  const T5& e5_;
  const T6& e6_;

  GTEST_DISALLOW_ASSIGN_(ElementsAreMatcher6);
};

template <typename T1, typename T2, typename T3, typename T4, typename T5,
    typename T6, typename T7>
class ElementsAreMatcher7 {
 public:
  ElementsAreMatcher7(const T1& e1, const T2& e2, const T3& e3, const T4& e4,
      const T5& e5, const T6& e6, const T7& e7) : e1_(e1), e2_(e2), e3_(e3),
      e4_(e4), e5_(e5), e6_(e6), e7_(e7) {}

  template <typename Container>
  operator Matcher<Container>() const {
    typedef GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(Container))
        RawContainer;
    typedef typename internal::StlContainerView<RawContainer>::type::value_type
        Element;

    const Matcher<const Element&> matchers[] = {
      MatcherCast<const Element&>(e1_),
      MatcherCast<const Element&>(e2_),
      MatcherCast<const Element&>(e3_),
      MatcherCast<const Element&>(e4_),
      MatcherCast<const Element&>(e5_),
      MatcherCast<const Element&>(e6_),
      MatcherCast<const Element&>(e7_),
    };

    return MakeMatcher(new ElementsAreMatcherImpl<Container>(matchers, 7));
  }

 private:
  const T1& e1_;
  const T2& e2_;
  const T3& e3_;
  const T4& e4_;
  const T5& e5_;
  const T6& e6_;
  const T7& e7_;

  GTEST_DISALLOW_ASSIGN_(ElementsAreMatcher7);
};

template <typename T1, typename T2, typename T3, typename T4, typename T5,
    typename T6, typename T7, typename T8>
class ElementsAreMatcher8 {
 public:
  ElementsAreMatcher8(const T1& e1, const T2& e2, const T3& e3, const T4& e4,
      const T5& e5, const T6& e6, const T7& e7, const T8& e8) : e1_(e1),
      e2_(e2), e3_(e3), e4_(e4), e5_(e5), e6_(e6), e7_(e7), e8_(e8) {}

  template <typename Container>
  operator Matcher<Container>() const {
    typedef GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(Container))
        RawContainer;
    typedef typename internal::StlContainerView<RawContainer>::type::value_type
        Element;

    const Matcher<const Element&> matchers[] = {
      MatcherCast<const Element&>(e1_),
      MatcherCast<const Element&>(e2_),
      MatcherCast<const Element&>(e3_),
      MatcherCast<const Element&>(e4_),
      MatcherCast<const Element&>(e5_),
      MatcherCast<const Element&>(e6_),
      MatcherCast<const Element&>(e7_),
      MatcherCast<const Element&>(e8_),
    };

    return MakeMatcher(new ElementsAreMatcherImpl<Container>(matchers, 8));
  }

 private:
  const T1& e1_;
  const T2& e2_;
  const T3& e3_;
  const T4& e4_;
  const T5& e5_;
  const T6& e6_;
  const T7& e7_;
  const T8& e8_;

  GTEST_DISALLOW_ASSIGN_(ElementsAreMatcher8);
};

template <typename T1, typename T2, typename T3, typename T4, typename T5,
    typename T6, typename T7, typename T8, typename T9>
class ElementsAreMatcher9 {
 public:
  ElementsAreMatcher9(const T1& e1, const T2& e2, const T3& e3, const T4& e4,
      const T5& e5, const T6& e6, const T7& e7, const T8& e8,
      const T9& e9) : e1_(e1), e2_(e2), e3_(e3), e4_(e4), e5_(e5), e6_(e6),
      e7_(e7), e8_(e8), e9_(e9) {}

  template <typename Container>
  operator Matcher<Container>() const {
    typedef GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(Container))
        RawContainer;
    typedef typename internal::StlContainerView<RawContainer>::type::value_type
        Element;

    const Matcher<const Element&> matchers[] = {
      MatcherCast<const Element&>(e1_),
      MatcherCast<const Element&>(e2_),
      MatcherCast<const Element&>(e3_),
      MatcherCast<const Element&>(e4_),
      MatcherCast<const Element&>(e5_),
      MatcherCast<const Element&>(e6_),
      MatcherCast<const Element&>(e7_),
      MatcherCast<const Element&>(e8_),
      MatcherCast<const Element&>(e9_),
    };

    return MakeMatcher(new ElementsAreMatcherImpl<Container>(matchers, 9));
  }

 private:
  const T1& e1_;
  const T2& e2_;
  const T3& e3_;
  const T4& e4_;
  const T5& e5_;
  const T6& e6_;
  const T7& e7_;
  const T8& e8_;
  const T9& e9_;

  GTEST_DISALLOW_ASSIGN_(ElementsAreMatcher9);
};

template <typename T1, typename T2, typename T3, typename T4, typename T5,
    typename T6, typename T7, typename T8, typename T9, typename T10>
class ElementsAreMatcher10 {
 public:
  ElementsAreMatcher10(const T1& e1, const T2& e2, const T3& e3, const T4& e4,
      const T5& e5, const T6& e6, const T7& e7, const T8& e8, const T9& e9,
      const T10& e10) : e1_(e1), e2_(e2), e3_(e3), e4_(e4), e5_(e5), e6_(e6),
      e7_(e7), e8_(e8), e9_(e9), e10_(e10) {}

  template <typename Container>
  operator Matcher<Container>() const {
    typedef GMOCK_REMOVE_CONST_(GMOCK_REMOVE_REFERENCE_(Container))
        RawContainer;
    typedef typename internal::StlContainerView<RawContainer>::type::value_type
        Element;

    const Matcher<const Element&> matchers[] = {
      MatcherCast<const Element&>(e1_),
      MatcherCast<const Element&>(e2_),
      MatcherCast<const Element&>(e3_),
      MatcherCast<const Element&>(e4_),
      MatcherCast<const Element&>(e5_),
      MatcherCast<const Element&>(e6_),
      MatcherCast<const Element&>(e7_),
      MatcherCast<const Element&>(e8_),
      MatcherCast<const Element&>(e9_),
      MatcherCast<const Element&>(e10_),
    };

    return MakeMatcher(new ElementsAreMatcherImpl<Container>(matchers, 10));
  }

 private:
  const T1& e1_;
  const T2& e2_;
  const T3& e3_;
  const T4& e4_;
  const T5& e5_;
  const T6& e6_;
  const T7& e7_;
  const T8& e8_;
  const T9& e9_;
  const T10& e10_;

  GTEST_DISALLOW_ASSIGN_(ElementsAreMatcher10);
};

}  // namespace internal

// Args<N1, N2, ..., Nk>(a_matcher) matches a tuple if the selected
// fields of it matches a_matcher.  C++ doesn't support default
// arguments for function templates, so we have to overload it.
template <typename InnerMatcher>
inline internal::ArgsMatcher<InnerMatcher>
Args(const InnerMatcher& matcher) {
  return internal::ArgsMatcher<InnerMatcher>(matcher);
}

template <int k1, typename InnerMatcher>
inline internal::ArgsMatcher<InnerMatcher, k1>
Args(const InnerMatcher& matcher) {
  return internal::ArgsMatcher<InnerMatcher, k1>(matcher);
}

template <int k1, int k2, typename InnerMatcher>
inline internal::ArgsMatcher<InnerMatcher, k1, k2>
Args(const InnerMatcher& matcher) {
  return internal::ArgsMatcher<InnerMatcher, k1, k2>(matcher);
}

template <int k1, int k2, int k3, typename InnerMatcher>
inline internal::ArgsMatcher<InnerMatcher, k1, k2, k3>
Args(const InnerMatcher& matcher) {
  return internal::ArgsMatcher<InnerMatcher, k1, k2, k3>(matcher);
}

template <int k1, int k2, int k3, int k4, typename InnerMatcher>
inline internal::ArgsMatcher<InnerMatcher, k1, k2, k3, k4>
Args(const InnerMatcher& matcher) {
  return internal::ArgsMatcher<InnerMatcher, k1, k2, k3, k4>(matcher);
}

template <int k1, int k2, int k3, int k4, int k5, typename InnerMatcher>
inline internal::ArgsMatcher<InnerMatcher, k1, k2, k3, k4, k5>
Args(const InnerMatcher& matcher) {
  return internal::ArgsMatcher<InnerMatcher, k1, k2, k3, k4, k5>(matcher);
}

template <int k1, int k2, int k3, int k4, int k5, int k6, typename InnerMatcher>
inline internal::ArgsMatcher<InnerMatcher, k1, k2, k3, k4, k5, k6>
Args(const InnerMatcher& matcher) {
  return internal::ArgsMatcher<InnerMatcher, k1, k2, k3, k4, k5, k6>(matcher);
}

template <int k1, int k2, int k3, int k4, int k5, int k6, int k7,
    typename InnerMatcher>
inline internal::ArgsMatcher<InnerMatcher, k1, k2, k3, k4, k5, k6, k7>
Args(const InnerMatcher& matcher) {
  return internal::ArgsMatcher<InnerMatcher, k1, k2, k3, k4, k5, k6,
      k7>(matcher);
}

template <int k1, int k2, int k3, int k4, int k5, int k6, int k7, int k8,
    typename InnerMatcher>
inline internal::ArgsMatcher<InnerMatcher, k1, k2, k3, k4, k5, k6, k7, k8>
Args(const InnerMatcher& matcher) {
  return internal::ArgsMatcher<InnerMatcher, k1, k2, k3, k4, k5, k6, k7,
      k8>(matcher);
}

template <int k1, int k2, int k3, int k4, int k5, int k6, int k7, int k8,
    int k9, typename InnerMatcher>
inline internal::ArgsMatcher<InnerMatcher, k1, k2, k3, k4, k5, k6, k7, k8, k9>
Args(const InnerMatcher& matcher) {
  return internal::ArgsMatcher<InnerMatcher, k1, k2, k3, k4, k5, k6, k7, k8,
      k9>(matcher);
}

template <int k1, int k2, int k3, int k4, int k5, int k6, int k7, int k8,
    int k9, int k10, typename InnerMatcher>
inline internal::ArgsMatcher<InnerMatcher, k1, k2, k3, k4, k5, k6, k7, k8, k9,
    k10>
Args(const InnerMatcher& matcher) {
  return internal::ArgsMatcher<InnerMatcher, k1, k2, k3, k4, k5, k6, k7, k8,
      k9, k10>(matcher);
}

// ElementsAre(e0, e1, ..., e_n) matches an STL-style container with
// (n + 1) elements, where the i-th element in the container must
// match the i-th argument in the list.  Each argument of
// ElementsAre() can be either a value or a matcher.  We support up to
// 10 arguments.
//
// NOTE: Since ElementsAre() cares about the order of the elements, it
// must not be used with containers whose elements's order is
// undefined (e.g. hash_map).

inline internal::ElementsAreMatcher0 ElementsAre() {
  return internal::ElementsAreMatcher0();
}

template <typename T1>
inline internal::ElementsAreMatcher1<T1> ElementsAre(const T1& e1) {
  return internal::ElementsAreMatcher1<T1>(e1);
}

template <typename T1, typename T2>
inline internal::ElementsAreMatcher2<T1, T2> ElementsAre(const T1& e1,
    const T2& e2) {
  return internal::ElementsAreMatcher2<T1, T2>(e1, e2);
}

template <typename T1, typename T2, typename T3>
inline internal::ElementsAreMatcher3<T1, T2, T3> ElementsAre(const T1& e1,
    const T2& e2, const T3& e3) {
  return internal::ElementsAreMatcher3<T1, T2, T3>(e1, e2, e3);
}

template <typename T1, typename T2, typename T3, typename T4>
inline internal::ElementsAreMatcher4<T1, T2, T3, T4> ElementsAre(const T1& e1,
    const T2& e2, const T3& e3, const T4& e4) {
  return internal::ElementsAreMatcher4<T1, T2, T3, T4>(e1, e2, e3, e4);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
inline internal::ElementsAreMatcher5<T1, T2, T3, T4,
    T5> ElementsAre(const T1& e1, const T2& e2, const T3& e3, const T4& e4,
    const T5& e5) {
  return internal::ElementsAreMatcher5<T1, T2, T3, T4, T5>(e1, e2, e3, e4, e5);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
    typename T6>
inline internal::ElementsAreMatcher6<T1, T2, T3, T4, T5,
    T6> ElementsAre(const T1& e1, const T2& e2, const T3& e3, const T4& e4,
    const T5& e5, const T6& e6) {
  return internal::ElementsAreMatcher6<T1, T2, T3, T4, T5, T6>(e1, e2, e3, e4,
      e5, e6);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
    typename T6, typename T7>
inline internal::ElementsAreMatcher7<T1, T2, T3, T4, T5, T6,
    T7> ElementsAre(const T1& e1, const T2& e2, const T3& e3, const T4& e4,
    const T5& e5, const T6& e6, const T7& e7) {
  return internal::ElementsAreMatcher7<T1, T2, T3, T4, T5, T6, T7>(e1, e2, e3,
      e4, e5, e6, e7);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
    typename T6, typename T7, typename T8>
inline internal::ElementsAreMatcher8<T1, T2, T3, T4, T5, T6, T7,
    T8> ElementsAre(const T1& e1, const T2& e2, const T3& e3, const T4& e4,
    const T5& e5, const T6& e6, const T7& e7, const T8& e8) {
  return internal::ElementsAreMatcher8<T1, T2, T3, T4, T5, T6, T7, T8>(e1, e2,
      e3, e4, e5, e6, e7, e8);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
    typename T6, typename T7, typename T8, typename T9>
inline internal::ElementsAreMatcher9<T1, T2, T3, T4, T5, T6, T7, T8,
    T9> ElementsAre(const T1& e1, const T2& e2, const T3& e3, const T4& e4,
    const T5& e5, const T6& e6, const T7& e7, const T8& e8, const T9& e9) {
  return internal::ElementsAreMatcher9<T1, T2, T3, T4, T5, T6, T7, T8, T9>(e1,
      e2, e3, e4, e5, e6, e7, e8, e9);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5,
    typename T6, typename T7, typename T8, typename T9, typename T10>
inline internal::ElementsAreMatcher10<T1, T2, T3, T4, T5, T6, T7, T8, T9,
    T10> ElementsAre(const T1& e1, const T2& e2, const T3& e3, const T4& e4,
    const T5& e5, const T6& e6, const T7& e7, const T8& e8, const T9& e9,
    const T10& e10) {
  return internal::ElementsAreMatcher10<T1, T2, T3, T4, T5, T6, T7, T8, T9,
      T10>(e1, e2, e3, e4, e5, e6, e7, e8, e9, e10);
}

// ElementsAreArray(array) and ElementAreArray(array, count) are like
// ElementsAre(), except that they take an array of values or
// matchers.  The former form infers the size of 'array', which must
// be a static C-style array.  In the latter form, 'array' can either
// be a static array or a pointer to a dynamically created array.

template <typename T>
inline internal::ElementsAreArrayMatcher<T> ElementsAreArray(
    const T* first, size_t count) {
  return internal::ElementsAreArrayMatcher<T>(first, count);
}

template <typename T, size_t N>
inline internal::ElementsAreArrayMatcher<T>
ElementsAreArray(const T (&array)[N]) {
  return internal::ElementsAreArrayMatcher<T>(array, N);
}

}  // namespace testing

// The MATCHER* family of macros can be used in a namespace scope to
// define custom matchers easily.
//
// Basic Usage
// ===========
//
// The syntax
//
//   MATCHER(name, description_string) { statements; }
//
// defines a matcher with the given name that executes the statements,
// which must return a bool to indicate if the match succeeds.  Inside
// the statements, you can refer to the value being matched by 'arg',
// and refer to its type by 'arg_type'.
//
// The description string documents what the matcher does, and is used
// to generate the failure message when the match fails.  Since a
// MATCHER() is usually defined in a header file shared by multiple
// C++ source files, we require the description to be a C-string
// literal to avoid possible side effects.  It can be empty, in which
// case we'll use the sequence of words in the matcher name as the
// description.
//
// For example:
//
//   MATCHER(IsEven, "") { return (arg % 2) == 0; }
//
// allows you to write
//
//   // Expects mock_foo.Bar(n) to be called where n is even.
//   EXPECT_CALL(mock_foo, Bar(IsEven()));
//
// or,
//
//   // Verifies that the value of some_expression is even.
//   EXPECT_THAT(some_expression, IsEven());
//
// If the above assertion fails, it will print something like:
//
//   Value of: some_expression
//   Expected: is even
//     Actual: 7
//
// where the description "is even" is automatically calculated from the
// matcher name IsEven.
//
// Argument Type
// =============
//
// Note that the type of the value being matched (arg_type) is
// determined by the context in which you use the matcher and is
// supplied to you by the compiler, so you don't need to worry about
// declaring it (nor can you).  This allows the matcher to be
// polymorphic.  For example, IsEven() can be used to match any type
// where the value of "(arg % 2) == 0" can be implicitly converted to
// a bool.  In the "Bar(IsEven())" example above, if method Bar()
// takes an int, 'arg_type' will be int; if it takes an unsigned long,
// 'arg_type' will be unsigned long; and so on.
//
// Parameterizing Matchers
// =======================
//
// Sometimes you'll want to parameterize the matcher.  For that you
// can use another macro:
//
//   MATCHER_P(name, param_name, description_string) { statements; }
//
// For example:
//
//   MATCHER_P(HasAbsoluteValue, value, "") { return abs(arg) == value; }
//
// will allow you to write:
//
//   EXPECT_THAT(Blah("a"), HasAbsoluteValue(n));
//
// which may lead to this message (assuming n is 10):
//
//   Value of: Blah("a")
//   Expected: has absolute value 10
//     Actual: -9
//
// Note that both the matcher description and its parameter are
// printed, making the message human-friendly.
//
// In the matcher definition body, you can write 'foo_type' to
// reference the type of a parameter named 'foo'.  For example, in the
// body of MATCHER_P(HasAbsoluteValue, value) above, you can write
// 'value_type' to refer to the type of 'value'.
//
// We also provide MATCHER_P2, MATCHER_P3, ..., up to MATCHER_P10 to
// support multi-parameter matchers.
//
// Describing Parameterized Matchers
// =================================
//
// When defining a parameterized matcher, you can use Python-style
// interpolations in the description string to refer to the parameter
// values.  We support the following syntax currently:
//
//   %%       a single '%' character
//   %(*)s    all parameters of the matcher printed as a tuple
//   %(foo)s  value of the matcher parameter named 'foo'
//
// For example,
//
//   MATCHER_P2(InClosedRange, low, hi, "is in range [%(low)s, %(hi)s]") {
//     return low <= arg && arg <= hi;
//   }
//   ...
//   EXPECT_THAT(3, InClosedRange(4, 6));
//
// would generate a failure that contains the message:
//
//   Expected: is in range [4, 6]
//
// If you specify "" as the description, the failure message will
// contain the sequence of words in the matcher name followed by the
// parameter values printed as a tuple.  For example,
//
//   MATCHER_P2(InClosedRange, low, hi, "") { ... }
//   ...
//   EXPECT_THAT(3, InClosedRange(4, 6));
//
// would generate a failure that contains the text:
//
//   Expected: in closed range (4, 6)
//
// Types of Matcher Parameters
// ===========================
//
// For the purpose of typing, you can view
//
//   MATCHER_Pk(Foo, p1, ..., pk, description_string) { ... }
//
// as shorthand for
//
//   template <typename p1_type, ..., typename pk_type>
//   FooMatcherPk<p1_type, ..., pk_type>
//   Foo(p1_type p1, ..., pk_type pk) { ... }
//
// When you write Foo(v1, ..., vk), the compiler infers the types of
// the parameters v1, ..., and vk for you.  If you are not happy with
// the result of the type inference, you can specify the types by
// explicitly instantiating the template, as in Foo<long, bool>(5,
// false).  As said earlier, you don't get to (or need to) specify
// 'arg_type' as that's determined by the context in which the matcher
// is used.  You can assign the result of expression Foo(p1, ..., pk)
// to a variable of type FooMatcherPk<p1_type, ..., pk_type>.  This
// can be useful when composing matchers.
//
// While you can instantiate a matcher template with reference types,
// passing the parameters by pointer usually makes your code more
// readable.  If, however, you still want to pass a parameter by
// reference, be aware that in the failure message generated by the
// matcher you will see the value of the referenced object but not its
// address.
//
// Explaining Match Results
// ========================
//
// Sometimes the matcher description alone isn't enough to explain why
// the match has failed or succeeded.  For example, when expecting a
// long string, it can be very helpful to also print the diff between
// the expected string and the actual one.  To achieve that, you can
// optionally stream additional information to a special variable
// named result_listener, whose type is a pointer to class
// MatchResultListener:
//
//   MATCHER_P(EqualsLongString, str, "") {
//     if (arg == str) return true;
//
//     *result_listener << "the difference: "
///                     << DiffStrings(str, arg);
//     return false;
//   }
//
// Overloading Matchers
// ====================
//
// You can overload matchers with different numbers of parameters:
//
//   MATCHER_P(Blah, a, description_string1) { ... }
//   MATCHER_P2(Blah, a, b, description_string2) { ... }
//
// Caveats
// =======
//
// When defining a new matcher, you should also consider implementing
// MatcherInterface or using MakePolymorphicMatcher().  These
// approaches require more work than the MATCHER* macros, but also
// give you more control on the types of the value being matched and
// the matcher parameters, which may leads to better compiler error
// messages when the matcher is used wrong.  They also allow
// overloading matchers based on parameter types (as opposed to just
// based on the number of parameters).
//
// MATCHER*() can only be used in a namespace scope.  The reason is
// that C++ doesn't yet allow function-local types to be used to
// instantiate templates.  The up-coming C++0x standard will fix this.
// Once that's done, we'll consider supporting using MATCHER*() inside
// a function.
//
// More Information
// ================
//
// To learn more about using these macros, please search for 'MATCHER'
// on http://code.google.com/p/googlemock/wiki/CookBook.

#define MATCHER(name, description)\
  class name##Matcher {\
   public:\
    template <typename arg_type>\
    class gmock_Impl : public ::testing::MatcherInterface<arg_type> {\
     public:\
      gmock_Impl(const ::testing::internal::Interpolations& gmock_interp)\
           : gmock_interp_(gmock_interp) {}\
      virtual bool MatchAndExplain(\
          arg_type arg, ::testing::MatchResultListener* result_listener) const;\
      virtual void DescribeTo(::std::ostream* gmock_os) const {\
        const ::testing::internal::Strings& gmock_printed_params = \
            ::testing::internal::UniversalTersePrintTupleFieldsToStrings(\
                ::std::tr1::tuple<>());\
        *gmock_os << ::testing::internal::FormatMatcherDescription(\
                     #name, description, gmock_interp_, gmock_printed_params);\
      }\
      const ::testing::internal::Interpolations gmock_interp_;\
     private:\
      GTEST_DISALLOW_ASSIGN_(gmock_Impl);\
    };\
    template <typename arg_type>\
    operator ::testing::Matcher<arg_type>() const {\
      return ::testing::Matcher<arg_type>(\
          new gmock_Impl<arg_type>(gmock_interp_));\
    }\
    name##Matcher() {\
      const char* gmock_param_names[] = { NULL };\
      gmock_interp_ = ::testing::internal::ValidateMatcherDescription(\
          gmock_param_names, ("" description ""));\
    }\
   private:\
    ::testing::internal::Interpolations gmock_interp_;\
    GTEST_DISALLOW_ASSIGN_(name##Matcher);\
  };\
  inline name##Matcher name() {\
    return name##Matcher();\
  }\
  template <typename arg_type>\
  bool name##Matcher::gmock_Impl<arg_type>::MatchAndExplain(\
      arg_type arg,\
      ::testing::MatchResultListener* result_listener GTEST_ATTRIBUTE_UNUSED_)\
          const

#define MATCHER_P(name, p0, description)\
  template <typename p0##_type>\
  class name##MatcherP {\
   public:\
    template <typename arg_type>\
    class gmock_Impl : public ::testing::MatcherInterface<arg_type> {\
     public:\
      explicit gmock_Impl(p0##_type gmock_p0, \
          const ::testing::internal::Interpolations& gmock_interp)\
           : p0(gmock_p0), gmock_interp_(gmock_interp) {}\
      virtual bool MatchAndExplain(\
          arg_type arg, ::testing::MatchResultListener* result_listener) const;\
      virtual void DescribeTo(::std::ostream* gmock_os) const {\
        const ::testing::internal::Strings& gmock_printed_params = \
            ::testing::internal::UniversalTersePrintTupleFieldsToStrings(\
                ::std::tr1::tuple<p0##_type>(p0));\
        *gmock_os << ::testing::internal::FormatMatcherDescription(\
                     #name, description, gmock_interp_, gmock_printed_params);\
      }\
      p0##_type p0;\
      const ::testing::internal::Interpolations gmock_interp_;\
     private:\
      GTEST_DISALLOW_ASSIGN_(gmock_Impl);\
    };\
    template <typename arg_type>\
    operator ::testing::Matcher<arg_type>() const {\
      return ::testing::Matcher<arg_type>(\
          new gmock_Impl<arg_type>(p0, gmock_interp_));\
    }\
    name##MatcherP(p0##_type gmock_p0) : p0(gmock_p0) {\
      const char* gmock_param_names[] = { #p0, NULL };\
      gmock_interp_ = ::testing::internal::ValidateMatcherDescription(\
          gmock_param_names, ("" description ""));\
    }\
    p0##_type p0;\
   private:\
    ::testing::internal::Interpolations gmock_interp_;\
    GTEST_DISALLOW_ASSIGN_(name##MatcherP);\
  };\
  template <typename p0##_type>\
  inline name##MatcherP<p0##_type> name(p0##_type p0) {\
    return name##MatcherP<p0##_type>(p0);\
  }\
  template <typename p0##_type>\
  template <typename arg_type>\
  bool name##MatcherP<p0##_type>::gmock_Impl<arg_type>::MatchAndExplain(\
      arg_type arg,\
      ::testing::MatchResultListener* result_listener GTEST_ATTRIBUTE_UNUSED_)\
          const

#define MATCHER_P2(name, p0, p1, description)\
  template <typename p0##_type, typename p1##_type>\
  class name##MatcherP2 {\
   public:\
    template <typename arg_type>\
    class gmock_Impl : public ::testing::MatcherInterface<arg_type> {\
     public:\
      gmock_Impl(p0##_type gmock_p0, p1##_type gmock_p1, \
          const ::testing::internal::Interpolations& gmock_interp)\
           : p0(gmock_p0), p1(gmock_p1), gmock_interp_(gmock_interp) {}\
      virtual bool MatchAndExplain(\
          arg_type arg, ::testing::MatchResultListener* result_listener) const;\
      virtual void DescribeTo(::std::ostream* gmock_os) const {\
        const ::testing::internal::Strings& gmock_printed_params = \
            ::testing::internal::UniversalTersePrintTupleFieldsToStrings(\
                ::std::tr1::tuple<p0##_type, p1##_type>(p0, p1));\
        *gmock_os << ::testing::internal::FormatMatcherDescription(\
                     #name, description, gmock_interp_, gmock_printed_params);\
      }\
      p0##_type p0;\
      p1##_type p1;\
      const ::testing::internal::Interpolations gmock_interp_;\
     private:\
      GTEST_DISALLOW_ASSIGN_(gmock_Impl);\
    };\
    template <typename arg_type>\
    operator ::testing::Matcher<arg_type>() const {\
      return ::testing::Matcher<arg_type>(\
          new gmock_Impl<arg_type>(p0, p1, gmock_interp_));\
    }\
    name##MatcherP2(p0##_type gmock_p0, p1##_type gmock_p1) : p0(gmock_p0), \
        p1(gmock_p1) {\
      const char* gmock_param_names[] = { #p0, #p1, NULL };\
      gmock_interp_ = ::testing::internal::ValidateMatcherDescription(\
          gmock_param_names, ("" description ""));\
    }\
    p0##_type p0;\
    p1##_type p1;\
   private:\
    ::testing::internal::Interpolations gmock_interp_;\
    GTEST_DISALLOW_ASSIGN_(name##MatcherP2);\
  };\
  template <typename p0##_type, typename p1##_type>\
  inline name##MatcherP2<p0##_type, p1##_type> name(p0##_type p0, \
      p1##_type p1) {\
    return name##MatcherP2<p0##_type, p1##_type>(p0, p1);\
  }\
  template <typename p0##_type, typename p1##_type>\
  template <typename arg_type>\
  bool name##MatcherP2<p0##_type, \
      p1##_type>::gmock_Impl<arg_type>::MatchAndExplain(\
      arg_type arg,\
      ::testing::MatchResultListener* result_listener GTEST_ATTRIBUTE_UNUSED_)\
          const

#define MATCHER_P3(name, p0, p1, p2, description)\
  template <typename p0##_type, typename p1##_type, typename p2##_type>\
  class name##MatcherP3 {\
   public:\
    template <typename arg_type>\
    class gmock_Impl : public ::testing::MatcherInterface<arg_type> {\
     public:\
      gmock_Impl(p0##_type gmock_p0, p1##_type gmock_p1, p2##_type gmock_p2, \
          const ::testing::internal::Interpolations& gmock_interp)\
           : p0(gmock_p0), p1(gmock_p1), p2(gmock_p2), \
               gmock_interp_(gmock_interp) {}\
      virtual bool MatchAndExplain(\
          arg_type arg, ::testing::MatchResultListener* result_listener) const;\
      virtual void DescribeTo(::std::ostream* gmock_os) const {\
        const ::testing::internal::Strings& gmock_printed_params = \
            ::testing::internal::UniversalTersePrintTupleFieldsToStrings(\
                ::std::tr1::tuple<p0##_type, p1##_type, p2##_type>(p0, p1, \
                    p2));\
        *gmock_os << ::testing::internal::FormatMatcherDescription(\
                     #name, description, gmock_interp_, gmock_printed_params);\
      }\
      p0##_type p0;\
      p1##_type p1;\
      p2##_type p2;\
      const ::testing::internal::Interpolations gmock_interp_;\
     private:\
      GTEST_DISALLOW_ASSIGN_(gmock_Impl);\
    };\
    template <typename arg_type>\
    operator ::testing::Matcher<arg_type>() const {\
      return ::testing::Matcher<arg_type>(\
          new gmock_Impl<arg_type>(p0, p1, p2, gmock_interp_));\
    }\
    name##MatcherP3(p0##_type gmock_p0, p1##_type gmock_p1, \
        p2##_type gmock_p2) : p0(gmock_p0), p1(gmock_p1), p2(gmock_p2) {\
      const char* gmock_param_names[] = { #p0, #p1, #p2, NULL };\
      gmock_interp_ = ::testing::internal::ValidateMatcherDescription(\
          gmock_param_names, ("" description ""));\
    }\
    p0##_type p0;\
    p1##_type p1;\
    p2##_type p2;\
   private:\
    ::testing::internal::Interpolations gmock_interp_;\
    GTEST_DISALLOW_ASSIGN_(name##MatcherP3);\
  };\
  template <typename p0##_type, typename p1##_type, typename p2##_type>\
  inline name##MatcherP3<p0##_type, p1##_type, p2##_type> name(p0##_type p0, \
      p1##_type p1, p2##_type p2) {\
    return name##MatcherP3<p0##_type, p1##_type, p2##_type>(p0, p1, p2);\
  }\
  template <typename p0##_type, typename p1##_type, typename p2##_type>\
  template <typename arg_type>\
  bool name##MatcherP3<p0##_type, p1##_type, \
      p2##_type>::gmock_Impl<arg_type>::MatchAndExplain(\
      arg_type arg,\
      ::testing::MatchResultListener* result_listener GTEST_ATTRIBUTE_UNUSED_)\
          const

#define MATCHER_P4(name, p0, p1, p2, p3, description)\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type>\
  class name##MatcherP4 {\
   public:\
    template <typename arg_type>\
    class gmock_Impl : public ::testing::MatcherInterface<arg_type> {\
     public:\
      gmock_Impl(p0##_type gmock_p0, p1##_type gmock_p1, p2##_type gmock_p2, \
          p3##_type gmock_p3, \
          const ::testing::internal::Interpolations& gmock_interp)\
           : p0(gmock_p0), p1(gmock_p1), p2(gmock_p2), p3(gmock_p3), \
               gmock_interp_(gmock_interp) {}\
      virtual bool MatchAndExplain(\
          arg_type arg, ::testing::MatchResultListener* result_listener) const;\
      virtual void DescribeTo(::std::ostream* gmock_os) const {\
        const ::testing::internal::Strings& gmock_printed_params = \
            ::testing::internal::UniversalTersePrintTupleFieldsToStrings(\
                ::std::tr1::tuple<p0##_type, p1##_type, p2##_type, \
                    p3##_type>(p0, p1, p2, p3));\
        *gmock_os << ::testing::internal::FormatMatcherDescription(\
                     #name, description, gmock_interp_, gmock_printed_params);\
      }\
      p0##_type p0;\
      p1##_type p1;\
      p2##_type p2;\
      p3##_type p3;\
      const ::testing::internal::Interpolations gmock_interp_;\
     private:\
      GTEST_DISALLOW_ASSIGN_(gmock_Impl);\
    };\
    template <typename arg_type>\
    operator ::testing::Matcher<arg_type>() const {\
      return ::testing::Matcher<arg_type>(\
          new gmock_Impl<arg_type>(p0, p1, p2, p3, gmock_interp_));\
    }\
    name##MatcherP4(p0##_type gmock_p0, p1##_type gmock_p1, \
        p2##_type gmock_p2, p3##_type gmock_p3) : p0(gmock_p0), p1(gmock_p1), \
        p2(gmock_p2), p3(gmock_p3) {\
      const char* gmock_param_names[] = { #p0, #p1, #p2, #p3, NULL };\
      gmock_interp_ = ::testing::internal::ValidateMatcherDescription(\
          gmock_param_names, ("" description ""));\
    }\
    p0##_type p0;\
    p1##_type p1;\
    p2##_type p2;\
    p3##_type p3;\
   private:\
    ::testing::internal::Interpolations gmock_interp_;\
    GTEST_DISALLOW_ASSIGN_(name##MatcherP4);\
  };\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type>\
  inline name##MatcherP4<p0##_type, p1##_type, p2##_type, \
      p3##_type> name(p0##_type p0, p1##_type p1, p2##_type p2, \
      p3##_type p3) {\
    return name##MatcherP4<p0##_type, p1##_type, p2##_type, p3##_type>(p0, \
        p1, p2, p3);\
  }\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type>\
  template <typename arg_type>\
  bool name##MatcherP4<p0##_type, p1##_type, p2##_type, \
      p3##_type>::gmock_Impl<arg_type>::MatchAndExplain(\
      arg_type arg,\
      ::testing::MatchResultListener* result_listener GTEST_ATTRIBUTE_UNUSED_)\
          const

#define MATCHER_P5(name, p0, p1, p2, p3, p4, description)\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type>\
  class name##MatcherP5 {\
   public:\
    template <typename arg_type>\
    class gmock_Impl : public ::testing::MatcherInterface<arg_type> {\
     public:\
      gmock_Impl(p0##_type gmock_p0, p1##_type gmock_p1, p2##_type gmock_p2, \
          p3##_type gmock_p3, p4##_type gmock_p4, \
          const ::testing::internal::Interpolations& gmock_interp)\
           : p0(gmock_p0), p1(gmock_p1), p2(gmock_p2), p3(gmock_p3), \
               p4(gmock_p4), gmock_interp_(gmock_interp) {}\
      virtual bool MatchAndExplain(\
          arg_type arg, ::testing::MatchResultListener* result_listener) const;\
      virtual void DescribeTo(::std::ostream* gmock_os) const {\
        const ::testing::internal::Strings& gmock_printed_params = \
            ::testing::internal::UniversalTersePrintTupleFieldsToStrings(\
                ::std::tr1::tuple<p0##_type, p1##_type, p2##_type, p3##_type, \
                    p4##_type>(p0, p1, p2, p3, p4));\
        *gmock_os << ::testing::internal::FormatMatcherDescription(\
                     #name, description, gmock_interp_, gmock_printed_params);\
      }\
      p0##_type p0;\
      p1##_type p1;\
      p2##_type p2;\
      p3##_type p3;\
      p4##_type p4;\
      const ::testing::internal::Interpolations gmock_interp_;\
     private:\
      GTEST_DISALLOW_ASSIGN_(gmock_Impl);\
    };\
    template <typename arg_type>\
    operator ::testing::Matcher<arg_type>() const {\
      return ::testing::Matcher<arg_type>(\
          new gmock_Impl<arg_type>(p0, p1, p2, p3, p4, gmock_interp_));\
    }\
    name##MatcherP5(p0##_type gmock_p0, p1##_type gmock_p1, \
        p2##_type gmock_p2, p3##_type gmock_p3, \
        p4##_type gmock_p4) : p0(gmock_p0), p1(gmock_p1), p2(gmock_p2), \
        p3(gmock_p3), p4(gmock_p4) {\
      const char* gmock_param_names[] = { #p0, #p1, #p2, #p3, #p4, NULL };\
      gmock_interp_ = ::testing::internal::ValidateMatcherDescription(\
          gmock_param_names, ("" description ""));\
    }\
    p0##_type p0;\
    p1##_type p1;\
    p2##_type p2;\
    p3##_type p3;\
    p4##_type p4;\
   private:\
    ::testing::internal::Interpolations gmock_interp_;\
    GTEST_DISALLOW_ASSIGN_(name##MatcherP5);\
  };\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type>\
  inline name##MatcherP5<p0##_type, p1##_type, p2##_type, p3##_type, \
      p4##_type> name(p0##_type p0, p1##_type p1, p2##_type p2, p3##_type p3, \
      p4##_type p4) {\
    return name##MatcherP5<p0##_type, p1##_type, p2##_type, p3##_type, \
        p4##_type>(p0, p1, p2, p3, p4);\
  }\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type>\
  template <typename arg_type>\
  bool name##MatcherP5<p0##_type, p1##_type, p2##_type, p3##_type, \
      p4##_type>::gmock_Impl<arg_type>::MatchAndExplain(\
      arg_type arg,\
      ::testing::MatchResultListener* result_listener GTEST_ATTRIBUTE_UNUSED_)\
          const

#define MATCHER_P6(name, p0, p1, p2, p3, p4, p5, description)\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type, typename p5##_type>\
  class name##MatcherP6 {\
   public:\
    template <typename arg_type>\
    class gmock_Impl : public ::testing::MatcherInterface<arg_type> {\
     public:\
      gmock_Impl(p0##_type gmock_p0, p1##_type gmock_p1, p2##_type gmock_p2, \
          p3##_type gmock_p3, p4##_type gmock_p4, p5##_type gmock_p5, \
          const ::testing::internal::Interpolations& gmock_interp)\
           : p0(gmock_p0), p1(gmock_p1), p2(gmock_p2), p3(gmock_p3), \
               p4(gmock_p4), p5(gmock_p5), gmock_interp_(gmock_interp) {}\
      virtual bool MatchAndExplain(\
          arg_type arg, ::testing::MatchResultListener* result_listener) const;\
      virtual void DescribeTo(::std::ostream* gmock_os) const {\
        const ::testing::internal::Strings& gmock_printed_params = \
            ::testing::internal::UniversalTersePrintTupleFieldsToStrings(\
                ::std::tr1::tuple<p0##_type, p1##_type, p2##_type, p3##_type, \
                    p4##_type, p5##_type>(p0, p1, p2, p3, p4, p5));\
        *gmock_os << ::testing::internal::FormatMatcherDescription(\
                     #name, description, gmock_interp_, gmock_printed_params);\
      }\
      p0##_type p0;\
      p1##_type p1;\
      p2##_type p2;\
      p3##_type p3;\
      p4##_type p4;\
      p5##_type p5;\
      const ::testing::internal::Interpolations gmock_interp_;\
     private:\
      GTEST_DISALLOW_ASSIGN_(gmock_Impl);\
    };\
    template <typename arg_type>\
    operator ::testing::Matcher<arg_type>() const {\
      return ::testing::Matcher<arg_type>(\
          new gmock_Impl<arg_type>(p0, p1, p2, p3, p4, p5, gmock_interp_));\
    }\
    name##MatcherP6(p0##_type gmock_p0, p1##_type gmock_p1, \
        p2##_type gmock_p2, p3##_type gmock_p3, p4##_type gmock_p4, \
        p5##_type gmock_p5) : p0(gmock_p0), p1(gmock_p1), p2(gmock_p2), \
        p3(gmock_p3), p4(gmock_p4), p5(gmock_p5) {\
      const char* gmock_param_names[] = { #p0, #p1, #p2, #p3, #p4, #p5, NULL };\
      gmock_interp_ = ::testing::internal::ValidateMatcherDescription(\
          gmock_param_names, ("" description ""));\
    }\
    p0##_type p0;\
    p1##_type p1;\
    p2##_type p2;\
    p3##_type p3;\
    p4##_type p4;\
    p5##_type p5;\
   private:\
    ::testing::internal::Interpolations gmock_interp_;\
    GTEST_DISALLOW_ASSIGN_(name##MatcherP6);\
  };\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type, typename p5##_type>\
  inline name##MatcherP6<p0##_type, p1##_type, p2##_type, p3##_type, \
      p4##_type, p5##_type> name(p0##_type p0, p1##_type p1, p2##_type p2, \
      p3##_type p3, p4##_type p4, p5##_type p5) {\
    return name##MatcherP6<p0##_type, p1##_type, p2##_type, p3##_type, \
        p4##_type, p5##_type>(p0, p1, p2, p3, p4, p5);\
  }\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type, typename p5##_type>\
  template <typename arg_type>\
  bool name##MatcherP6<p0##_type, p1##_type, p2##_type, p3##_type, p4##_type, \
      p5##_type>::gmock_Impl<arg_type>::MatchAndExplain(\
      arg_type arg,\
      ::testing::MatchResultListener* result_listener GTEST_ATTRIBUTE_UNUSED_)\
          const

#define MATCHER_P7(name, p0, p1, p2, p3, p4, p5, p6, description)\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type, typename p5##_type, \
      typename p6##_type>\
  class name##MatcherP7 {\
   public:\
    template <typename arg_type>\
    class gmock_Impl : public ::testing::MatcherInterface<arg_type> {\
     public:\
      gmock_Impl(p0##_type gmock_p0, p1##_type gmock_p1, p2##_type gmock_p2, \
          p3##_type gmock_p3, p4##_type gmock_p4, p5##_type gmock_p5, \
          p6##_type gmock_p6, \
          const ::testing::internal::Interpolations& gmock_interp)\
           : p0(gmock_p0), p1(gmock_p1), p2(gmock_p2), p3(gmock_p3), \
               p4(gmock_p4), p5(gmock_p5), p6(gmock_p6), \
               gmock_interp_(gmock_interp) {}\
      virtual bool MatchAndExplain(\
          arg_type arg, ::testing::MatchResultListener* result_listener) const;\
      virtual void DescribeTo(::std::ostream* gmock_os) const {\
        const ::testing::internal::Strings& gmock_printed_params = \
            ::testing::internal::UniversalTersePrintTupleFieldsToStrings(\
                ::std::tr1::tuple<p0##_type, p1##_type, p2##_type, p3##_type, \
                    p4##_type, p5##_type, p6##_type>(p0, p1, p2, p3, p4, p5, \
                    p6));\
        *gmock_os << ::testing::internal::FormatMatcherDescription(\
                     #name, description, gmock_interp_, gmock_printed_params);\
      }\
      p0##_type p0;\
      p1##_type p1;\
      p2##_type p2;\
      p3##_type p3;\
      p4##_type p4;\
      p5##_type p5;\
      p6##_type p6;\
      const ::testing::internal::Interpolations gmock_interp_;\
     private:\
      GTEST_DISALLOW_ASSIGN_(gmock_Impl);\
    };\
    template <typename arg_type>\
    operator ::testing::Matcher<arg_type>() const {\
      return ::testing::Matcher<arg_type>(\
          new gmock_Impl<arg_type>(p0, p1, p2, p3, p4, p5, p6, gmock_interp_));\
    }\
    name##MatcherP7(p0##_type gmock_p0, p1##_type gmock_p1, \
        p2##_type gmock_p2, p3##_type gmock_p3, p4##_type gmock_p4, \
        p5##_type gmock_p5, p6##_type gmock_p6) : p0(gmock_p0), p1(gmock_p1), \
        p2(gmock_p2), p3(gmock_p3), p4(gmock_p4), p5(gmock_p5), \
        p6(gmock_p6) {\
      const char* gmock_param_names[] = { #p0, #p1, #p2, #p3, #p4, #p5, #p6, \
          NULL };\
      gmock_interp_ = ::testing::internal::ValidateMatcherDescription(\
          gmock_param_names, ("" description ""));\
    }\
    p0##_type p0;\
    p1##_type p1;\
    p2##_type p2;\
    p3##_type p3;\
    p4##_type p4;\
    p5##_type p5;\
    p6##_type p6;\
   private:\
    ::testing::internal::Interpolations gmock_interp_;\
    GTEST_DISALLOW_ASSIGN_(name##MatcherP7);\
  };\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type, typename p5##_type, \
      typename p6##_type>\
  inline name##MatcherP7<p0##_type, p1##_type, p2##_type, p3##_type, \
      p4##_type, p5##_type, p6##_type> name(p0##_type p0, p1##_type p1, \
      p2##_type p2, p3##_type p3, p4##_type p4, p5##_type p5, \
      p6##_type p6) {\
    return name##MatcherP7<p0##_type, p1##_type, p2##_type, p3##_type, \
        p4##_type, p5##_type, p6##_type>(p0, p1, p2, p3, p4, p5, p6);\
  }\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type, typename p5##_type, \
      typename p6##_type>\
  template <typename arg_type>\
  bool name##MatcherP7<p0##_type, p1##_type, p2##_type, p3##_type, p4##_type, \
      p5##_type, p6##_type>::gmock_Impl<arg_type>::MatchAndExplain(\
      arg_type arg,\
      ::testing::MatchResultListener* result_listener GTEST_ATTRIBUTE_UNUSED_)\
          const

#define MATCHER_P8(name, p0, p1, p2, p3, p4, p5, p6, p7, description)\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type, typename p5##_type, \
      typename p6##_type, typename p7##_type>\
  class name##MatcherP8 {\
   public:\
    template <typename arg_type>\
    class gmock_Impl : public ::testing::MatcherInterface<arg_type> {\
     public:\
      gmock_Impl(p0##_type gmock_p0, p1##_type gmock_p1, p2##_type gmock_p2, \
          p3##_type gmock_p3, p4##_type gmock_p4, p5##_type gmock_p5, \
          p6##_type gmock_p6, p7##_type gmock_p7, \
          const ::testing::internal::Interpolations& gmock_interp)\
           : p0(gmock_p0), p1(gmock_p1), p2(gmock_p2), p3(gmock_p3), \
               p4(gmock_p4), p5(gmock_p5), p6(gmock_p6), p7(gmock_p7), \
               gmock_interp_(gmock_interp) {}\
      virtual bool MatchAndExplain(\
          arg_type arg, ::testing::MatchResultListener* result_listener) const;\
      virtual void DescribeTo(::std::ostream* gmock_os) const {\
        const ::testing::internal::Strings& gmock_printed_params = \
            ::testing::internal::UniversalTersePrintTupleFieldsToStrings(\
                ::std::tr1::tuple<p0##_type, p1##_type, p2##_type, p3##_type, \
                    p4##_type, p5##_type, p6##_type, p7##_type>(p0, p1, p2, \
                    p3, p4, p5, p6, p7));\
        *gmock_os << ::testing::internal::FormatMatcherDescription(\
                     #name, description, gmock_interp_, gmock_printed_params);\
      }\
      p0##_type p0;\
      p1##_type p1;\
      p2##_type p2;\
      p3##_type p3;\
      p4##_type p4;\
      p5##_type p5;\
      p6##_type p6;\
      p7##_type p7;\
      const ::testing::internal::Interpolations gmock_interp_;\
     private:\
      GTEST_DISALLOW_ASSIGN_(gmock_Impl);\
    };\
    template <typename arg_type>\
    operator ::testing::Matcher<arg_type>() const {\
      return ::testing::Matcher<arg_type>(\
          new gmock_Impl<arg_type>(p0, p1, p2, p3, p4, p5, p6, p7, \
              gmock_interp_));\
    }\
    name##MatcherP8(p0##_type gmock_p0, p1##_type gmock_p1, \
        p2##_type gmock_p2, p3##_type gmock_p3, p4##_type gmock_p4, \
        p5##_type gmock_p5, p6##_type gmock_p6, \
        p7##_type gmock_p7) : p0(gmock_p0), p1(gmock_p1), p2(gmock_p2), \
        p3(gmock_p3), p4(gmock_p4), p5(gmock_p5), p6(gmock_p6), \
        p7(gmock_p7) {\
      const char* gmock_param_names[] = { #p0, #p1, #p2, #p3, #p4, #p5, #p6, \
          #p7, NULL };\
      gmock_interp_ = ::testing::internal::ValidateMatcherDescription(\
          gmock_param_names, ("" description ""));\
    }\
    p0##_type p0;\
    p1##_type p1;\
    p2##_type p2;\
    p3##_type p3;\
    p4##_type p4;\
    p5##_type p5;\
    p6##_type p6;\
    p7##_type p7;\
   private:\
    ::testing::internal::Interpolations gmock_interp_;\
    GTEST_DISALLOW_ASSIGN_(name##MatcherP8);\
  };\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type, typename p5##_type, \
      typename p6##_type, typename p7##_type>\
  inline name##MatcherP8<p0##_type, p1##_type, p2##_type, p3##_type, \
      p4##_type, p5##_type, p6##_type, p7##_type> name(p0##_type p0, \
      p1##_type p1, p2##_type p2, p3##_type p3, p4##_type p4, p5##_type p5, \
      p6##_type p6, p7##_type p7) {\
    return name##MatcherP8<p0##_type, p1##_type, p2##_type, p3##_type, \
        p4##_type, p5##_type, p6##_type, p7##_type>(p0, p1, p2, p3, p4, p5, \
        p6, p7);\
  }\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type, typename p5##_type, \
      typename p6##_type, typename p7##_type>\
  template <typename arg_type>\
  bool name##MatcherP8<p0##_type, p1##_type, p2##_type, p3##_type, p4##_type, \
      p5##_type, p6##_type, \
      p7##_type>::gmock_Impl<arg_type>::MatchAndExplain(\
      arg_type arg,\
      ::testing::MatchResultListener* result_listener GTEST_ATTRIBUTE_UNUSED_)\
          const

#define MATCHER_P9(name, p0, p1, p2, p3, p4, p5, p6, p7, p8, description)\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type, typename p5##_type, \
      typename p6##_type, typename p7##_type, typename p8##_type>\
  class name##MatcherP9 {\
   public:\
    template <typename arg_type>\
    class gmock_Impl : public ::testing::MatcherInterface<arg_type> {\
     public:\
      gmock_Impl(p0##_type gmock_p0, p1##_type gmock_p1, p2##_type gmock_p2, \
          p3##_type gmock_p3, p4##_type gmock_p4, p5##_type gmock_p5, \
          p6##_type gmock_p6, p7##_type gmock_p7, p8##_type gmock_p8, \
          const ::testing::internal::Interpolations& gmock_interp)\
           : p0(gmock_p0), p1(gmock_p1), p2(gmock_p2), p3(gmock_p3), \
               p4(gmock_p4), p5(gmock_p5), p6(gmock_p6), p7(gmock_p7), \
               p8(gmock_p8), gmock_interp_(gmock_interp) {}\
      virtual bool MatchAndExplain(\
          arg_type arg, ::testing::MatchResultListener* result_listener) const;\
      virtual void DescribeTo(::std::ostream* gmock_os) const {\
        const ::testing::internal::Strings& gmock_printed_params = \
            ::testing::internal::UniversalTersePrintTupleFieldsToStrings(\
                ::std::tr1::tuple<p0##_type, p1##_type, p2##_type, p3##_type, \
                    p4##_type, p5##_type, p6##_type, p7##_type, \
                    p8##_type>(p0, p1, p2, p3, p4, p5, p6, p7, p8));\
        *gmock_os << ::testing::internal::FormatMatcherDescription(\
                     #name, description, gmock_interp_, gmock_printed_params);\
      }\
      p0##_type p0;\
      p1##_type p1;\
      p2##_type p2;\
      p3##_type p3;\
      p4##_type p4;\
      p5##_type p5;\
      p6##_type p6;\
      p7##_type p7;\
      p8##_type p8;\
      const ::testing::internal::Interpolations gmock_interp_;\
     private:\
      GTEST_DISALLOW_ASSIGN_(gmock_Impl);\
    };\
    template <typename arg_type>\
    operator ::testing::Matcher<arg_type>() const {\
      return ::testing::Matcher<arg_type>(\
          new gmock_Impl<arg_type>(p0, p1, p2, p3, p4, p5, p6, p7, p8, \
              gmock_interp_));\
    }\
    name##MatcherP9(p0##_type gmock_p0, p1##_type gmock_p1, \
        p2##_type gmock_p2, p3##_type gmock_p3, p4##_type gmock_p4, \
        p5##_type gmock_p5, p6##_type gmock_p6, p7##_type gmock_p7, \
        p8##_type gmock_p8) : p0(gmock_p0), p1(gmock_p1), p2(gmock_p2), \
        p3(gmock_p3), p4(gmock_p4), p5(gmock_p5), p6(gmock_p6), p7(gmock_p7), \
        p8(gmock_p8) {\
      const char* gmock_param_names[] = { #p0, #p1, #p2, #p3, #p4, #p5, #p6, \
          #p7, #p8, NULL };\
      gmock_interp_ = ::testing::internal::ValidateMatcherDescription(\
          gmock_param_names, ("" description ""));\
    }\
    p0##_type p0;\
    p1##_type p1;\
    p2##_type p2;\
    p3##_type p3;\
    p4##_type p4;\
    p5##_type p5;\
    p6##_type p6;\
    p7##_type p7;\
    p8##_type p8;\
   private:\
    ::testing::internal::Interpolations gmock_interp_;\
    GTEST_DISALLOW_ASSIGN_(name##MatcherP9);\
  };\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type, typename p5##_type, \
      typename p6##_type, typename p7##_type, typename p8##_type>\
  inline name##MatcherP9<p0##_type, p1##_type, p2##_type, p3##_type, \
      p4##_type, p5##_type, p6##_type, p7##_type, \
      p8##_type> name(p0##_type p0, p1##_type p1, p2##_type p2, p3##_type p3, \
      p4##_type p4, p5##_type p5, p6##_type p6, p7##_type p7, \
      p8##_type p8) {\
    return name##MatcherP9<p0##_type, p1##_type, p2##_type, p3##_type, \
        p4##_type, p5##_type, p6##_type, p7##_type, p8##_type>(p0, p1, p2, \
        p3, p4, p5, p6, p7, p8);\
  }\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type, typename p5##_type, \
      typename p6##_type, typename p7##_type, typename p8##_type>\
  template <typename arg_type>\
  bool name##MatcherP9<p0##_type, p1##_type, p2##_type, p3##_type, p4##_type, \
      p5##_type, p6##_type, p7##_type, \
      p8##_type>::gmock_Impl<arg_type>::MatchAndExplain(\
      arg_type arg,\
      ::testing::MatchResultListener* result_listener GTEST_ATTRIBUTE_UNUSED_)\
          const

#define MATCHER_P10(name, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, description)\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type, typename p5##_type, \
      typename p6##_type, typename p7##_type, typename p8##_type, \
      typename p9##_type>\
  class name##MatcherP10 {\
   public:\
    template <typename arg_type>\
    class gmock_Impl : public ::testing::MatcherInterface<arg_type> {\
     public:\
      gmock_Impl(p0##_type gmock_p0, p1##_type gmock_p1, p2##_type gmock_p2, \
          p3##_type gmock_p3, p4##_type gmock_p4, p5##_type gmock_p5, \
          p6##_type gmock_p6, p7##_type gmock_p7, p8##_type gmock_p8, \
          p9##_type gmock_p9, \
          const ::testing::internal::Interpolations& gmock_interp)\
           : p0(gmock_p0), p1(gmock_p1), p2(gmock_p2), p3(gmock_p3), \
               p4(gmock_p4), p5(gmock_p5), p6(gmock_p6), p7(gmock_p7), \
               p8(gmock_p8), p9(gmock_p9), gmock_interp_(gmock_interp) {}\
      virtual bool MatchAndExplain(\
          arg_type arg, ::testing::MatchResultListener* result_listener) const;\
      virtual void DescribeTo(::std::ostream* gmock_os) const {\
        const ::testing::internal::Strings& gmock_printed_params = \
            ::testing::internal::UniversalTersePrintTupleFieldsToStrings(\
                ::std::tr1::tuple<p0##_type, p1##_type, p2##_type, p3##_type, \
                    p4##_type, p5##_type, p6##_type, p7##_type, p8##_type, \
                    p9##_type>(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9));\
        *gmock_os << ::testing::internal::FormatMatcherDescription(\
                     #name, description, gmock_interp_, gmock_printed_params);\
      }\
      p0##_type p0;\
      p1##_type p1;\
      p2##_type p2;\
      p3##_type p3;\
      p4##_type p4;\
      p5##_type p5;\
      p6##_type p6;\
      p7##_type p7;\
      p8##_type p8;\
      p9##_type p9;\
      const ::testing::internal::Interpolations gmock_interp_;\
     private:\
      GTEST_DISALLOW_ASSIGN_(gmock_Impl);\
    };\
    template <typename arg_type>\
    operator ::testing::Matcher<arg_type>() const {\
      return ::testing::Matcher<arg_type>(\
          new gmock_Impl<arg_type>(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, \
              gmock_interp_));\
    }\
    name##MatcherP10(p0##_type gmock_p0, p1##_type gmock_p1, \
        p2##_type gmock_p2, p3##_type gmock_p3, p4##_type gmock_p4, \
        p5##_type gmock_p5, p6##_type gmock_p6, p7##_type gmock_p7, \
        p8##_type gmock_p8, p9##_type gmock_p9) : p0(gmock_p0), p1(gmock_p1), \
        p2(gmock_p2), p3(gmock_p3), p4(gmock_p4), p5(gmock_p5), p6(gmock_p6), \
        p7(gmock_p7), p8(gmock_p8), p9(gmock_p9) {\
      const char* gmock_param_names[] = { #p0, #p1, #p2, #p3, #p4, #p5, #p6, \
          #p7, #p8, #p9, NULL };\
      gmock_interp_ = ::testing::internal::ValidateMatcherDescription(\
          gmock_param_names, ("" description ""));\
    }\
    p0##_type p0;\
    p1##_type p1;\
    p2##_type p2;\
    p3##_type p3;\
    p4##_type p4;\
    p5##_type p5;\
    p6##_type p6;\
    p7##_type p7;\
    p8##_type p8;\
    p9##_type p9;\
   private:\
    ::testing::internal::Interpolations gmock_interp_;\
    GTEST_DISALLOW_ASSIGN_(name##MatcherP10);\
  };\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type, typename p5##_type, \
      typename p6##_type, typename p7##_type, typename p8##_type, \
      typename p9##_type>\
  inline name##MatcherP10<p0##_type, p1##_type, p2##_type, p3##_type, \
      p4##_type, p5##_type, p6##_type, p7##_type, p8##_type, \
      p9##_type> name(p0##_type p0, p1##_type p1, p2##_type p2, p3##_type p3, \
      p4##_type p4, p5##_type p5, p6##_type p6, p7##_type p7, p8##_type p8, \
      p9##_type p9) {\
    return name##MatcherP10<p0##_type, p1##_type, p2##_type, p3##_type, \
        p4##_type, p5##_type, p6##_type, p7##_type, p8##_type, p9##_type>(p0, \
        p1, p2, p3, p4, p5, p6, p7, p8, p9);\
  }\
  template <typename p0##_type, typename p1##_type, typename p2##_type, \
      typename p3##_type, typename p4##_type, typename p5##_type, \
      typename p6##_type, typename p7##_type, typename p8##_type, \
      typename p9##_type>\
  template <typename arg_type>\
  bool name##MatcherP10<p0##_type, p1##_type, p2##_type, p3##_type, \
      p4##_type, p5##_type, p6##_type, p7##_type, p8##_type, \
      p9##_type>::gmock_Impl<arg_type>::MatchAndExplain(\
      arg_type arg,\
      ::testing::MatchResultListener* result_listener GTEST_ATTRIBUTE_UNUSED_)\
          const

#endif  // GMOCK_INCLUDE_GMOCK_GMOCK_GENERATED_MATCHERS_H_
// Copyright 2007, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Author: wan@google.com (Zhanyong Wan)

// Google Mock - a framework for writing C++ mock classes.
//
// This file implements some actions that depend on gmock-generated-actions.h.

#ifndef GMOCK_INCLUDE_GMOCK_GMOCK_MORE_ACTIONS_H_
#define GMOCK_INCLUDE_GMOCK_GMOCK_MORE_ACTIONS_H_


namespace testing {
namespace internal {

// Implements the Invoke(f) action.  The template argument
// FunctionImpl is the implementation type of f, which can be either a
// function pointer or a functor.  Invoke(f) can be used as an
// Action<F> as long as f's type is compatible with F (i.e. f can be
// assigned to a tr1::function<F>).
template <typename FunctionImpl>
class InvokeAction {
 public:
  // The c'tor makes a copy of function_impl (either a function
  // pointer or a functor).
  explicit InvokeAction(FunctionImpl function_impl)
      : function_impl_(function_impl) {}

  template <typename Result, typename ArgumentTuple>
  Result Perform(const ArgumentTuple& args) {
    return InvokeHelper<Result, ArgumentTuple>::Invoke(function_impl_, args);
  }

 private:
  FunctionImpl function_impl_;

  GTEST_DISALLOW_ASSIGN_(InvokeAction);
};

// Implements the Invoke(object_ptr, &Class::Method) action.
template <class Class, typename MethodPtr>
class InvokeMethodAction {
 public:
  InvokeMethodAction(Class* obj_ptr, MethodPtr method_ptr)
      : obj_ptr_(obj_ptr), method_ptr_(method_ptr) {}

  template <typename Result, typename ArgumentTuple>
  Result Perform(const ArgumentTuple& args) const {
    return InvokeHelper<Result, ArgumentTuple>::InvokeMethod(
        obj_ptr_, method_ptr_, args);
  }

 private:
  Class* const obj_ptr_;
  const MethodPtr method_ptr_;

  GTEST_DISALLOW_ASSIGN_(InvokeMethodAction);
};

}  // namespace internal

// Various overloads for Invoke().

// Creates an action that invokes 'function_impl' with the mock
// function's arguments.
template <typename FunctionImpl>
PolymorphicAction<internal::InvokeAction<FunctionImpl> > Invoke(
    FunctionImpl function_impl) {
  return MakePolymorphicAction(
      internal::InvokeAction<FunctionImpl>(function_impl));
}

// Creates an action that invokes the given method on the given object
// with the mock function's arguments.
template <class Class, typename MethodPtr>
PolymorphicAction<internal::InvokeMethodAction<Class, MethodPtr> > Invoke(
    Class* obj_ptr, MethodPtr method_ptr) {
  return MakePolymorphicAction(
      internal::InvokeMethodAction<Class, MethodPtr>(obj_ptr, method_ptr));
}

// WithoutArgs(inner_action) can be used in a mock function with a
// non-empty argument list to perform inner_action, which takes no
// argument.  In other words, it adapts an action accepting no
// argument to one that accepts (and ignores) arguments.
template <typename InnerAction>
inline internal::WithArgsAction<InnerAction>
WithoutArgs(const InnerAction& action) {
  return internal::WithArgsAction<InnerAction>(action);
}

// WithArg<k>(an_action) creates an action that passes the k-th
// (0-based) argument of the mock function to an_action and performs
// it.  It adapts an action accepting one argument to one that accepts
// multiple arguments.  For convenience, we also provide
// WithArgs<k>(an_action) (defined below) as a synonym.
template <int k, typename InnerAction>
inline internal::WithArgsAction<InnerAction, k>
WithArg(const InnerAction& action) {
  return internal::WithArgsAction<InnerAction, k>(action);
}

// The ACTION*() macros trigger warning C4100 (unreferenced formal
// parameter) in MSVC with -W4.  Unfortunately they cannot be fixed in
// the macro definition, as the warnings are generated when the macro
// is expanded and macro expansion cannot contain #pragma.  Therefore
// we suppress them here.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4100)
#endif

// Action ReturnArg<k>() returns the k-th argument of the mock function.
ACTION_TEMPLATE(ReturnArg,
                HAS_1_TEMPLATE_PARAMS(int, k),
                AND_0_VALUE_PARAMS()) {
  return std::tr1::get<k>(args);
}

// Action SaveArg<k>(pointer) saves the k-th (0-based) argument of the
// mock function to *pointer.
ACTION_TEMPLATE(SaveArg,
                HAS_1_TEMPLATE_PARAMS(int, k),
                AND_1_VALUE_PARAMS(pointer)) {
  *pointer = ::std::tr1::get<k>(args);
}

// Action SetArgReferee<k>(value) assigns 'value' to the variable
// referenced by the k-th (0-based) argument of the mock function.
ACTION_TEMPLATE(SetArgReferee,
                HAS_1_TEMPLATE_PARAMS(int, k),
                AND_1_VALUE_PARAMS(value)) {
  typedef typename ::std::tr1::tuple_element<k, args_type>::type argk_type;
  // Ensures that argument #k is a reference.  If you get a compiler
  // error on the next line, you are using SetArgReferee<k>(value) in
  // a mock function whose k-th (0-based) argument is not a reference.
  GMOCK_COMPILE_ASSERT_(internal::is_reference<argk_type>::value,
                        SetArgReferee_must_be_used_with_a_reference_argument);
  ::std::tr1::get<k>(args) = value;
}

// Action SetArrayArgument<k>(first, last) copies the elements in
// source range [first, last) to the array pointed to by the k-th
// (0-based) argument, which can be either a pointer or an
// iterator. The action does not take ownership of the elements in the
// source range.
ACTION_TEMPLATE(SetArrayArgument,
                HAS_1_TEMPLATE_PARAMS(int, k),
                AND_2_VALUE_PARAMS(first, last)) {
  // Microsoft compiler deprecates ::std::copy, so we want to suppress warning
  // 4996 (Function call with parameters that may be unsafe) there.
#ifdef _MSC_VER
#pragma warning(push)          // Saves the current warning state.
#pragma warning(disable:4996)  // Temporarily disables warning 4996.
#endif
  ::std::copy(first, last, ::std::tr1::get<k>(args));
#ifdef _MSC_VER
#pragma warning(pop)           // Restores the warning state.
#endif
}

// Action DeleteArg<k>() deletes the k-th (0-based) argument of the mock
// function.
ACTION_TEMPLATE(DeleteArg,
                HAS_1_TEMPLATE_PARAMS(int, k),
                AND_0_VALUE_PARAMS()) {
  delete ::std::tr1::get<k>(args);
}

// Action Throw(exception) can be used in a mock function of any type
// to throw the given exception.  Any copyable value can be thrown.
#if GTEST_HAS_EXCEPTIONS
ACTION_P(Throw, exception) { throw exception; }
#endif  // GTEST_HAS_EXCEPTIONS

#ifdef _MSC_VER
#pragma warning(pop)
#endif

}  // namespace testing

#endif  // GMOCK_INCLUDE_GMOCK_GMOCK_MORE_ACTIONS_H_
// This file was GENERATED by a script.  DO NOT EDIT BY HAND!!!

// Copyright 2008, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Author: wan@google.com (Zhanyong Wan)

// Implements class templates NiceMock and StrictMock.
//
// Given a mock class MockFoo that is created using Google Mock,
// NiceMock<MockFoo> is a subclass of MockFoo that allows
// uninteresting calls (i.e. calls to mock methods that have no
// EXPECT_CALL specs), and StrictMock<MockFoo> is a subclass of
// MockFoo that treats all uninteresting calls as errors.
//
// NiceMock and StrictMock "inherits" the constructors of their
// respective base class, with up-to 10 arguments.  Therefore you can
// write NiceMock<MockFoo>(5, "a") to construct a nice mock where
// MockFoo has a constructor that accepts (int, const char*), for
// example.
//
// A known limitation is that NiceMock<MockFoo> and
// StrictMock<MockFoo> only works for mock methods defined using the
// MOCK_METHOD* family of macros DIRECTLY in the MockFoo class.  If a
// mock method is defined in a base class of MockFoo, the "nice" or
// "strict" modifier may not affect it, depending on the compiler.  In
// particular, nesting NiceMock and StrictMock is NOT supported.
//
// Another known limitation is that the constructors of the base mock
// cannot have arguments passed by non-const reference, which are
// banned by the Google C++ style guide anyway.

#ifndef GMOCK_INCLUDE_GMOCK_GMOCK_GENERATED_NICE_STRICT_H_
#define GMOCK_INCLUDE_GMOCK_GMOCK_GENERATED_NICE_STRICT_H_


namespace testing {

template <class MockClass>
class NiceMock : public MockClass {
 public:
  // We don't factor out the constructor body to a common method, as
  // we have to avoid a possible clash with members of MockClass.
  NiceMock() {
    ::testing::Mock::AllowUninterestingCalls(
        internal::implicit_cast<MockClass*>(this));
  }

  // C++ doesn't (yet) allow inheritance of constructors, so we have
  // to define it for each arity.
  template <typename A1>
  explicit NiceMock(const A1& a1) : MockClass(a1) {
    ::testing::Mock::AllowUninterestingCalls(
        internal::implicit_cast<MockClass*>(this));
  }
  template <typename A1, typename A2>
  NiceMock(const A1& a1, const A2& a2) : MockClass(a1, a2) {
    ::testing::Mock::AllowUninterestingCalls(
        internal::implicit_cast<MockClass*>(this));
  }

  template <typename A1, typename A2, typename A3>
  NiceMock(const A1& a1, const A2& a2, const A3& a3) : MockClass(a1, a2, a3) {
    ::testing::Mock::AllowUninterestingCalls(
        internal::implicit_cast<MockClass*>(this));
  }

  template <typename A1, typename A2, typename A3, typename A4>
  NiceMock(const A1& a1, const A2& a2, const A3& a3,
      const A4& a4) : MockClass(a1, a2, a3, a4) {
    ::testing::Mock::AllowUninterestingCalls(
        internal::implicit_cast<MockClass*>(this));
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5>
  NiceMock(const A1& a1, const A2& a2, const A3& a3, const A4& a4,
      const A5& a5) : MockClass(a1, a2, a3, a4, a5) {
    ::testing::Mock::AllowUninterestingCalls(
        internal::implicit_cast<MockClass*>(this));
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5,
      typename A6>
  NiceMock(const A1& a1, const A2& a2, const A3& a3, const A4& a4,
      const A5& a5, const A6& a6) : MockClass(a1, a2, a3, a4, a5, a6) {
    ::testing::Mock::AllowUninterestingCalls(
        internal::implicit_cast<MockClass*>(this));
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5,
      typename A6, typename A7>
  NiceMock(const A1& a1, const A2& a2, const A3& a3, const A4& a4,
      const A5& a5, const A6& a6, const A7& a7) : MockClass(a1, a2, a3, a4, a5,
      a6, a7) {
    ::testing::Mock::AllowUninterestingCalls(
        internal::implicit_cast<MockClass*>(this));
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5,
      typename A6, typename A7, typename A8>
  NiceMock(const A1& a1, const A2& a2, const A3& a3, const A4& a4,
      const A5& a5, const A6& a6, const A7& a7, const A8& a8) : MockClass(a1,
      a2, a3, a4, a5, a6, a7, a8) {
    ::testing::Mock::AllowUninterestingCalls(
        internal::implicit_cast<MockClass*>(this));
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5,
      typename A6, typename A7, typename A8, typename A9>
  NiceMock(const A1& a1, const A2& a2, const A3& a3, const A4& a4,
      const A5& a5, const A6& a6, const A7& a7, const A8& a8,
      const A9& a9) : MockClass(a1, a2, a3, a4, a5, a6, a7, a8, a9) {
    ::testing::Mock::AllowUninterestingCalls(
        internal::implicit_cast<MockClass*>(this));
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5,
      typename A6, typename A7, typename A8, typename A9, typename A10>
  NiceMock(const A1& a1, const A2& a2, const A3& a3, const A4& a4,
      const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9,
      const A10& a10) : MockClass(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) {
    ::testing::Mock::AllowUninterestingCalls(
        internal::implicit_cast<MockClass*>(this));
  }

  virtual ~NiceMock() {
    ::testing::Mock::UnregisterCallReaction(
        internal::implicit_cast<MockClass*>(this));
  }

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(NiceMock);
};

template <class MockClass>
class StrictMock : public MockClass {
 public:
  // We don't factor out the constructor body to a common method, as
  // we have to avoid a possible clash with members of MockClass.
  StrictMock() {
    ::testing::Mock::FailUninterestingCalls(
        internal::implicit_cast<MockClass*>(this));
  }

  template <typename A1>
  explicit StrictMock(const A1& a1) : MockClass(a1) {
    ::testing::Mock::FailUninterestingCalls(
        internal::implicit_cast<MockClass*>(this));
  }
  template <typename A1, typename A2>
  StrictMock(const A1& a1, const A2& a2) : MockClass(a1, a2) {
    ::testing::Mock::FailUninterestingCalls(
        internal::implicit_cast<MockClass*>(this));
  }

  template <typename A1, typename A2, typename A3>
  StrictMock(const A1& a1, const A2& a2, const A3& a3) : MockClass(a1, a2, a3) {
    ::testing::Mock::FailUninterestingCalls(
        internal::implicit_cast<MockClass*>(this));
  }

  template <typename A1, typename A2, typename A3, typename A4>
  StrictMock(const A1& a1, const A2& a2, const A3& a3,
      const A4& a4) : MockClass(a1, a2, a3, a4) {
    ::testing::Mock::FailUninterestingCalls(
        internal::implicit_cast<MockClass*>(this));
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5>
  StrictMock(const A1& a1, const A2& a2, const A3& a3, const A4& a4,
      const A5& a5) : MockClass(a1, a2, a3, a4, a5) {
    ::testing::Mock::FailUninterestingCalls(
        internal::implicit_cast<MockClass*>(this));
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5,
      typename A6>
  StrictMock(const A1& a1, const A2& a2, const A3& a3, const A4& a4,
      const A5& a5, const A6& a6) : MockClass(a1, a2, a3, a4, a5, a6) {
    ::testing::Mock::FailUninterestingCalls(
        internal::implicit_cast<MockClass*>(this));
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5,
      typename A6, typename A7>
  StrictMock(const A1& a1, const A2& a2, const A3& a3, const A4& a4,
      const A5& a5, const A6& a6, const A7& a7) : MockClass(a1, a2, a3, a4, a5,
      a6, a7) {
    ::testing::Mock::FailUninterestingCalls(
        internal::implicit_cast<MockClass*>(this));
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5,
      typename A6, typename A7, typename A8>
  StrictMock(const A1& a1, const A2& a2, const A3& a3, const A4& a4,
      const A5& a5, const A6& a6, const A7& a7, const A8& a8) : MockClass(a1,
      a2, a3, a4, a5, a6, a7, a8) {
    ::testing::Mock::FailUninterestingCalls(
        internal::implicit_cast<MockClass*>(this));
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5,
      typename A6, typename A7, typename A8, typename A9>
  StrictMock(const A1& a1, const A2& a2, const A3& a3, const A4& a4,
      const A5& a5, const A6& a6, const A7& a7, const A8& a8,
      const A9& a9) : MockClass(a1, a2, a3, a4, a5, a6, a7, a8, a9) {
    ::testing::Mock::FailUninterestingCalls(
        internal::implicit_cast<MockClass*>(this));
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5,
      typename A6, typename A7, typename A8, typename A9, typename A10>
  StrictMock(const A1& a1, const A2& a2, const A3& a3, const A4& a4,
      const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9,
      const A10& a10) : MockClass(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) {
    ::testing::Mock::FailUninterestingCalls(
        internal::implicit_cast<MockClass*>(this));
  }

  virtual ~StrictMock() {
    ::testing::Mock::UnregisterCallReaction(
        internal::implicit_cast<MockClass*>(this));
  }

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(StrictMock);
};

// The following specializations catch some (relatively more common)
// user errors of nesting nice and strict mocks.  They do NOT catch
// all possible errors.

// These specializations are declared but not defined, as NiceMock and
// StrictMock cannot be nested.
template <typename MockClass>
class NiceMock<NiceMock<MockClass> >;
template <typename MockClass>
class NiceMock<StrictMock<MockClass> >;
template <typename MockClass>
class StrictMock<NiceMock<MockClass> >;
template <typename MockClass>
class StrictMock<StrictMock<MockClass> >;

}  // namespace testing

#endif  // GMOCK_INCLUDE_GMOCK_GMOCK_GENERATED_NICE_STRICT_H_

namespace testing {

// Declares Google Mock flags that we want a user to use programmatically.
GMOCK_DECLARE_bool_(catch_leaked_mocks);
GMOCK_DECLARE_string_(verbose);

// Initializes Google Mock.  This must be called before running the
// tests.  In particular, it parses the command line for the flags
// that Google Mock recognizes.  Whenever a Google Mock flag is seen,
// it is removed from argv, and *argc is decremented.
//
// No value is returned.  Instead, the Google Mock flag variables are
// updated.
//
// Since Google Test is needed for Google Mock to work, this function
// also initializes Google Test and parses its flags, if that hasn't
// been done.
void InitGoogleMock(int* argc, char** argv);

// This overloaded version can be used in Windows programs compiled in
// UNICODE mode.
void InitGoogleMock(int* argc, wchar_t** argv);

}  // namespace testing

#endif  // GMOCK_INCLUDE_GMOCK_GMOCK_H_
