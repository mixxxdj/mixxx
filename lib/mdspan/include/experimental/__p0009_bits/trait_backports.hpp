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
#ifndef MDSPAN_INCLUDE_EXPERIMENTAL_BITS_TRAIT_BACKPORTS_HPP_
#define MDSPAN_INCLUDE_EXPERIMENTAL_BITS_TRAIT_BACKPORTS_HPP_

#include "macros.hpp"
#include "config.hpp"

#include <type_traits>
#include <utility> // integer_sequence

//==============================================================================
// <editor-fold desc="Variable template trait backports (e.g., is_void_v)"> {{{1

#ifdef _MDSPAN_NEEDS_TRAIT_VARIABLE_TEMPLATE_BACKPORTS

#if _MDSPAN_USE_VARIABLE_TEMPLATES
namespace MDSPAN_IMPL_STANDARD_NAMESPACE {

#define _MDSPAN_BACKPORT_TRAIT(TRAIT) \
  template <class... Args> _MDSPAN_INLINE_VARIABLE constexpr auto TRAIT##_v = TRAIT<Args...>::value;

_MDSPAN_BACKPORT_TRAIT(is_assignable)
_MDSPAN_BACKPORT_TRAIT(is_constructible)
_MDSPAN_BACKPORT_TRAIT(is_convertible)
_MDSPAN_BACKPORT_TRAIT(is_default_constructible)
_MDSPAN_BACKPORT_TRAIT(is_trivially_destructible)
_MDSPAN_BACKPORT_TRAIT(is_same)
_MDSPAN_BACKPORT_TRAIT(is_empty)
_MDSPAN_BACKPORT_TRAIT(is_void)

#undef _MDSPAN_BACKPORT_TRAIT

} // end namespace MDSPAN_IMPL_STANDARD_NAMESPACE

#endif // _MDSPAN_USE_VARIABLE_TEMPLATES

#endif // _MDSPAN_NEEDS_TRAIT_VARIABLE_TEMPLATE_BACKPORTS

// </editor-fold> end Variable template trait backports (e.g., is_void_v) }}}1
//==============================================================================

//==============================================================================
// <editor-fold desc="integer sequence (ugh...)"> {{{1

#if !defined(_MDSPAN_USE_INTEGER_SEQUENCE) || !_MDSPAN_USE_INTEGER_SEQUENCE

namespace MDSPAN_IMPL_STANDARD_NAMESPACE {

template <class T, T... Vals>
struct integer_sequence {
  static constexpr size_t size() noexcept { return sizeof...(Vals); }
  using value_type = T;
};

template <size_t... Vals>
using index_sequence = std::integer_sequence<size_t, Vals...>;

namespace __detail {

template <class T, T N, T I, class Result>
struct __make_int_seq_impl;

template <class T, T N, T... Vals>
struct __make_int_seq_impl<T, N, N, integer_sequence<T, Vals...>>
{
  using type = integer_sequence<T, Vals...>;
};

template <class T, T N, T I, T... Vals>
struct __make_int_seq_impl<
  T, N, I, integer_sequence<T, Vals...>
> : __make_int_seq_impl<T, N, I+1, integer_sequence<T, Vals..., I>>
{ };

} // end namespace __detail

template <class T, T N>
using make_integer_sequence = typename __detail::__make_int_seq_impl<T, N, 0, integer_sequence<T>>::type;

template <size_t N>
using make_index_sequence = typename __detail::__make_int_seq_impl<size_t, N, 0, integer_sequence<size_t>>::type;

template <class... T>
using index_sequence_for = make_index_sequence<sizeof...(T)>;

} // end namespace MDSPAN_IMPL_STANDARD_NAMESPACE

#endif

// </editor-fold> end integer sequence (ugh...) }}}1
//==============================================================================

//==============================================================================
// <editor-fold desc="standard trait aliases"> {{{1

#if !defined(_MDSPAN_USE_STANDARD_TRAIT_ALIASES) || !_MDSPAN_USE_STANDARD_TRAIT_ALIASES

namespace MDSPAN_IMPL_STANDARD_NAMESPACE {

#define _MDSPAN_BACKPORT_TRAIT_ALIAS(TRAIT) \
  template <class... Args> using TRAIT##_t = typename TRAIT<Args...>::type;

_MDSPAN_BACKPORT_TRAIT_ALIAS(remove_cv)
_MDSPAN_BACKPORT_TRAIT_ALIAS(remove_reference)

template <bool _B, class _T=void>
using enable_if_t = typename enable_if<_B, _T>::type;

#undef _MDSPAN_BACKPORT_TRAIT_ALIAS

} // end namespace MDSPAN_IMPL_STANDARD_NAMESPACE

#endif

// </editor-fold> end standard trait aliases }}}1
//==============================================================================

#endif //MDSPAN_INCLUDE_EXPERIMENTAL_BITS_TRAIT_BACKPORTS_HPP_
