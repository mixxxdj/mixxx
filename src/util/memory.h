// Taken from https://github.com/bstreiff/cppbits
// Thank you Brandon Streiff!

// Implementation of C++14's make_unique for C++11 compilers.
//
// This has been tested with:
// - MSVC 11.0 (Visual Studio 2012)
// - gcc 4.6.3
// - Xcode 4.4 (with clang "4.0")
//
// It is based off an implementation proposed by Stephan T. Lavavej for
// inclusion in the C++14 standard:
//    http://isocpp.org/files/papers/N3656.txt
// Where appropriate, it borrows the use of MSVC's _VARIADIC_EXPAND_0X macro
// machinery to compensate for lack of variadic templates.
//
// This file injects make_unique into the std namespace, which I acknowledge is
// technically forbidden ([C++11: 17.6.4.2.2.1/1]), but is necessary in order
// to have syntax compatibility with C++14.
//
// I perform compiler version checking for MSVC, gcc, and clang to ensure that
// we don't add make_unique if it is already there (instead, we include
// <memory> to get the compiler-provided one). You can override the compiler
// version checking by defining the symbol COMPILER_SUPPORTS_MAKE_UNIQUE.
//
//
// ===============================================================================
// This file is released into the public domain. See LICENCE for more information.
// ===============================================================================

#ifndef MIXXX_UTIL_MEMORY_H
#define MIXXX_UTIL_MEMORY_H

// If user hasn't specified COMPILER_SUPPORTS_MAKE_UNIQUE then try to figure out
// based on compiler version if std::make_unique is provided.
#if !defined(COMPILER_SUPPORTS_MAKE_UNIQUE)
   // Compiling with -std=c++11 sets __cplusplus=201103L and disables
   // std::make_unique() from C++14! We need to take this into account.
   #define CPLUSPLUS_SUPPORTS_MAKE_UNIQUE (__cplusplus > 201103L)
   #if defined(_MSC_VER)
      // std::make_unique was added in MSVC 12.0
      #if _MSC_VER >= 1800 // MSVC 12.0 (Visual Studio 2013)
         #define COMPILER_SUPPORTS_MAKE_UNIQUE
      #endif
   #elif defined(__clang__)
      // std::make_unique was added in clang 3.4, but not until Xcode 6.
      // Annoyingly, Apple makes the clang version defines match the version
      // of Xcode, not the version of clang.
      #define CLANG_VERSION (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
      #if defined(__APPLE__) && CLANG_VERSION >= 60000 && CPLUSPLUS_SUPPORTS_MAKE_UNIQUE
         #define COMPILER_SUPPORTS_MAKE_UNIQUE
      #elif !defined(__APPLE__) && CLANG_VERSION >= 30400 && CPLUSPLUS_SUPPORTS_MAKE_UNIQUE
         #define COMPILER_SUPPORTS_MAKE_UNIQUE
      #endif
   #elif defined(__GNUC__)
      // std::make_unique was added in gcc 4.9
      #define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
      #if GCC_VERSION >= 40900 && CPLUSPLUS_SUPPORTS_MAKE_UNIQUE
         #define COMPILER_SUPPORTS_MAKE_UNIQUE
      #endif
   #endif
#endif

#if defined(COMPILER_SUPPORTS_MAKE_UNIQUE)

// If the compiler supports std::make_unique, then pull in <memory> to get it.
#include <memory>

#else

// Otherwise, the compiler doesn't provide it, so implement it ourselves.

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

namespace std {

template<class _Ty> struct _Unique_if {
   typedef unique_ptr<_Ty> _Single_object;
};

template<class _Ty> struct _Unique_if<_Ty[]> {
   typedef unique_ptr<_Ty[]> _Unknown_bound;
};

template<class _Ty, size_t N> struct _Unique_if<_Ty[N]> {
   typedef void _Known_bound;
};

//
// template< class T, class... Args >
// unique_ptr<T> make_unique( Args&&... args);
//

#if defined(_MSC_VER) && (_MSC_VER < 1800)

// Macro machinery because MSVC 11.0 doesn't support variadic templates.
// The _VARIADIC_EXPAND_0X stuff is defined in <xstddef>
#define _MAKE_UNIQUE( \
   TEMPLATE_LIST, PADDING_LIST, LIST, COMMA, X1, X2, X3, X4) \
   template<class _Ty COMMA LIST(_CLASS_TYPE)> inline \
      typename _Unique_if<_Ty>::_Single_object make_unique(LIST(_TYPE_REFREF_ARG)) \
   { \
      return unique_ptr<_Ty>(new _Ty(LIST(_FORWARD_ARG))); \
   } \

_VARIADIC_EXPAND_0X(_MAKE_UNIQUE, , , , )
#undef _MAKE_UNIQUE

#else // not MSVC 11.0 or earlier

template<class _Ty, class... Args>
   typename _Unique_if<_Ty>::_Single_object
   make_unique(Args&&... args) {
      return unique_ptr<_Ty>(new _Ty(std::forward<Args>(args)...));
   }

#endif

// template< class T >
// unique_ptr<T> make_unique( std::size_t size );

template<class _Ty>
   typename _Unique_if<_Ty>::_Unknown_bound
   make_unique(size_t n) {
      typedef typename remove_extent<_Ty>::type U;
      return unique_ptr<_Ty>(new U[n]());
   }

// template< class T, class... Args >
// /* unspecified */ make_unique( Args&&... args ) = delete;

// MSVC 11.0 doesn't support deleted functions, so the best we can do
// is simply not define the function.
#if !(defined(_MSC_VER) && (_MSC_VER < 1800))

template<class T, class... Args>
   typename _Unique_if<T>::_Known_bound
   make_unique(Args&&...) = delete;

#endif

} // namespace std

#endif // !COMPILER_SUPPORTS_MAKE_UNIQUE

#endif /* MIXXX_UTIL_MEMORY_H */
