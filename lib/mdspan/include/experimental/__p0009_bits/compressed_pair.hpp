//@HEADER
// ************************************************************************
//
//                        Kokkos v. 4.0
//       Copyright (2022) National Technology & Engineering
//               Solutions of Sandia, LLC (NTESS).
//
// Under the terms of Contract DE-NA0003525 with NTESS,
// the U.S. Government retains certain rights in this software.
//
// Part of Kokkos, under the Apache License v2.0 with LLVM Exceptions.
// See https://kokkos.org/LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//@HEADER
#pragma once

#include "macros.hpp"
#include "trait_backports.hpp"

#if !defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)
#  include "no_unique_address.hpp"
#endif

namespace MDSPAN_IMPL_STANDARD_NAMESPACE {
namespace detail {

// For no unique address emulation, this is the case taken when neither are empty.
// For real `[[no_unique_address]]`, this case is always taken.
template <class _T1, class _T2, class _Enable = void> struct __compressed_pair {
  _MDSPAN_NO_UNIQUE_ADDRESS _T1 __t1_val;
  _MDSPAN_NO_UNIQUE_ADDRESS _T2 __t2_val;
  MDSPAN_FORCE_INLINE_FUNCTION _MDSPAN_CONSTEXPR_14 _T1 &__first() noexcept { return __t1_val; }
  MDSPAN_FORCE_INLINE_FUNCTION constexpr _T1 const &__first() const noexcept {
    return __t1_val;
  }
  MDSPAN_FORCE_INLINE_FUNCTION _MDSPAN_CONSTEXPR_14 _T2 &__second() noexcept { return __t2_val; }
  MDSPAN_FORCE_INLINE_FUNCTION constexpr _T2 const &__second() const noexcept {
    return __t2_val;
  }

  MDSPAN_INLINE_FUNCTION_DEFAULTED
  constexpr __compressed_pair() noexcept = default;
  MDSPAN_INLINE_FUNCTION_DEFAULTED
  constexpr __compressed_pair(__compressed_pair const &) noexcept = default;
  MDSPAN_INLINE_FUNCTION_DEFAULTED
  constexpr __compressed_pair(__compressed_pair &&) noexcept = default;
  MDSPAN_INLINE_FUNCTION_DEFAULTED
  _MDSPAN_CONSTEXPR_14_DEFAULTED __compressed_pair &
  operator=(__compressed_pair const &) noexcept = default;
  MDSPAN_INLINE_FUNCTION_DEFAULTED
  _MDSPAN_CONSTEXPR_14_DEFAULTED __compressed_pair &
  operator=(__compressed_pair &&) noexcept = default;
  MDSPAN_INLINE_FUNCTION_DEFAULTED
  ~__compressed_pair() noexcept = default;
  template <class _T1Like, class _T2Like>
  MDSPAN_INLINE_FUNCTION constexpr __compressed_pair(_T1Like &&__t1, _T2Like &&__t2)
      : __t1_val((_T1Like &&) __t1), __t2_val((_T2Like &&) __t2) {}
};

#if !defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)

// First empty.
template <class _T1, class _T2>
struct __compressed_pair<
    _T1, _T2,
    std::enable_if_t<_MDSPAN_TRAIT(std::is_empty, _T1) && !_MDSPAN_TRAIT(std::is_empty, _T2)>>
    : private _T1 {
  _T2 __t2_val;
  MDSPAN_FORCE_INLINE_FUNCTION _MDSPAN_CONSTEXPR_14 _T1 &__first() noexcept {
    return *static_cast<_T1 *>(this);
  }
  MDSPAN_FORCE_INLINE_FUNCTION constexpr _T1 const &__first() const noexcept {
    return *static_cast<_T1 const *>(this);
  }
  MDSPAN_FORCE_INLINE_FUNCTION _MDSPAN_CONSTEXPR_14 _T2 &__second() noexcept { return __t2_val; }
  MDSPAN_FORCE_INLINE_FUNCTION constexpr _T2 const &__second() const noexcept {
    return __t2_val;
  }

  MDSPAN_INLINE_FUNCTION_DEFAULTED
  constexpr __compressed_pair() noexcept = default;
  MDSPAN_INLINE_FUNCTION_DEFAULTED
  constexpr __compressed_pair(__compressed_pair const &) noexcept = default;
  MDSPAN_INLINE_FUNCTION_DEFAULTED
  constexpr __compressed_pair(__compressed_pair &&) noexcept = default;
  MDSPAN_INLINE_FUNCTION_DEFAULTED
  _MDSPAN_CONSTEXPR_14_DEFAULTED __compressed_pair &
  operator=(__compressed_pair const &) noexcept = default;
  MDSPAN_INLINE_FUNCTION_DEFAULTED
  _MDSPAN_CONSTEXPR_14_DEFAULTED __compressed_pair &
  operator=(__compressed_pair &&) noexcept = default;
  MDSPAN_INLINE_FUNCTION_DEFAULTED
  ~__compressed_pair() noexcept = default;
  template <class _T1Like, class _T2Like>
  MDSPAN_INLINE_FUNCTION constexpr __compressed_pair(_T1Like &&__t1, _T2Like &&__t2)
      : _T1((_T1Like &&) __t1), __t2_val((_T2Like &&) __t2) {}
};

// Second empty.
template <class _T1, class _T2>
struct __compressed_pair<
    _T1, _T2,
    std::enable_if_t<!_MDSPAN_TRAIT(std::is_empty, _T1) && _MDSPAN_TRAIT(std::is_empty, _T2)>>
    : private _T2 {
  _T1 __t1_val;
  MDSPAN_FORCE_INLINE_FUNCTION _MDSPAN_CONSTEXPR_14 _T1 &__first() noexcept { return __t1_val; }
  MDSPAN_FORCE_INLINE_FUNCTION constexpr _T1 const &__first() const noexcept {
    return __t1_val;
  }
  MDSPAN_FORCE_INLINE_FUNCTION _MDSPAN_CONSTEXPR_14 _T2 &__second() noexcept {
    return *static_cast<_T2 *>(this);
  }
  MDSPAN_FORCE_INLINE_FUNCTION constexpr _T2 const &__second() const noexcept {
    return *static_cast<_T2 const *>(this);
  }

  MDSPAN_INLINE_FUNCTION_DEFAULTED
  constexpr __compressed_pair() noexcept = default;
  MDSPAN_INLINE_FUNCTION_DEFAULTED
  constexpr __compressed_pair(__compressed_pair const &) noexcept = default;
  MDSPAN_INLINE_FUNCTION_DEFAULTED
  constexpr __compressed_pair(__compressed_pair &&) noexcept = default;
  MDSPAN_INLINE_FUNCTION_DEFAULTED
  _MDSPAN_CONSTEXPR_14_DEFAULTED __compressed_pair &
  operator=(__compressed_pair const &) noexcept = default;
  MDSPAN_INLINE_FUNCTION_DEFAULTED
  _MDSPAN_CONSTEXPR_14_DEFAULTED __compressed_pair &
  operator=(__compressed_pair &&) noexcept = default;
  MDSPAN_INLINE_FUNCTION_DEFAULTED
  ~__compressed_pair() noexcept = default;

  template <class _T1Like, class _T2Like>
  MDSPAN_INLINE_FUNCTION constexpr __compressed_pair(_T1Like &&__t1, _T2Like &&__t2)
      : _T2((_T2Like &&) __t2), __t1_val((_T1Like &&) __t1) {}
};

// Both empty.
template <class _T1, class _T2>
struct __compressed_pair<
    _T1, _T2,
    std::enable_if_t<_MDSPAN_TRAIT(std::is_empty, _T1) && _MDSPAN_TRAIT(std::is_empty, _T2)>>
    // We need to use the __no_unique_address_emulation wrapper here to avoid
    // base class ambiguities.
#ifdef _MDSPAN_COMPILER_MSVC
// MSVC doesn't allow you to access public static member functions of a type
// when you *happen* to privately inherit from that type.
    : protected __no_unique_address_emulation<_T1, 0>,
      protected __no_unique_address_emulation<_T2, 1>
#else
    : private __no_unique_address_emulation<_T1, 0>,
      private __no_unique_address_emulation<_T2, 1>
#endif
{
  using __first_base_t = __no_unique_address_emulation<_T1, 0>;
  using __second_base_t = __no_unique_address_emulation<_T2, 1>;

  MDSPAN_FORCE_INLINE_FUNCTION _MDSPAN_CONSTEXPR_14 _T1 &__first() noexcept {
    return this->__first_base_t::__ref();
  }
  MDSPAN_FORCE_INLINE_FUNCTION constexpr _T1 const &__first() const noexcept {
    return this->__first_base_t::__ref();
  }
  MDSPAN_FORCE_INLINE_FUNCTION _MDSPAN_CONSTEXPR_14 _T2 &__second() noexcept {
    return this->__second_base_t::__ref();
  }
  MDSPAN_FORCE_INLINE_FUNCTION constexpr _T2 const &__second() const noexcept {
    return this->__second_base_t::__ref();
  }

  MDSPAN_INLINE_FUNCTION_DEFAULTED
  constexpr __compressed_pair() noexcept = default;
  MDSPAN_INLINE_FUNCTION_DEFAULTED
  constexpr __compressed_pair(__compressed_pair const &) noexcept = default;
  MDSPAN_INLINE_FUNCTION_DEFAULTED
  constexpr __compressed_pair(__compressed_pair &&) noexcept = default;
  MDSPAN_INLINE_FUNCTION_DEFAULTED
  _MDSPAN_CONSTEXPR_14_DEFAULTED __compressed_pair &
  operator=(__compressed_pair const &) noexcept = default;
  MDSPAN_INLINE_FUNCTION_DEFAULTED
  _MDSPAN_CONSTEXPR_14_DEFAULTED __compressed_pair &
  operator=(__compressed_pair &&) noexcept = default;
  MDSPAN_INLINE_FUNCTION_DEFAULTED
  ~__compressed_pair() noexcept = default;
  template <class _T1Like, class _T2Like>
  MDSPAN_INLINE_FUNCTION constexpr __compressed_pair(_T1Like &&__t1, _T2Like &&__t2) noexcept
    : __first_base_t(_T1((_T1Like &&) __t1)),
      __second_base_t(_T2((_T2Like &&) __t2))
  { }
};

#endif // !defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)

} // end namespace detail
} // end namespace MDSPAN_IMPL_STANDARD_NAMESPACE
