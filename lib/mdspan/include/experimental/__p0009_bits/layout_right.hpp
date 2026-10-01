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
#include "extents.hpp"
#include <stdexcept>
#include "layout_stride.hpp"

namespace MDSPAN_IMPL_STANDARD_NAMESPACE {

//==============================================================================
template <class Extents>
class layout_right::mapping {
  public:
    using extents_type = Extents;
    using index_type = typename extents_type::index_type;
    using size_type = typename extents_type::size_type;
    using rank_type = typename extents_type::rank_type;
    using layout_type = layout_right;
  private:

    static_assert(detail::__is_extents_v<extents_type>,
                  MDSPAN_IMPL_STANDARD_NAMESPACE_STRING "::layout_right::mapping must be instantiated with a specialization of " MDSPAN_IMPL_STANDARD_NAMESPACE_STRING "::extents.");

    template <class>
    friend class mapping;

    // i0+(i1 + E(1)*(i2 + E(2)*i3))
    template <size_t r, size_t Rank>
    struct __rank_count {};

    template <size_t r, size_t Rank, class I, class... Indices>
    _MDSPAN_HOST_DEVICE
    constexpr index_type __compute_offset(
      index_type offset, __rank_count<r,Rank>, const I& i, Indices... idx) const {
      return __compute_offset(offset * __extents.extent(r) + i,__rank_count<r+1,Rank>(),  idx...);
    }

    template<class I, class ... Indices>
    _MDSPAN_HOST_DEVICE
    constexpr index_type __compute_offset(
      __rank_count<0,extents_type::rank()>, const I& i, Indices... idx) const {
      return __compute_offset(i,__rank_count<1,extents_type::rank()>(),idx...);
    }

    _MDSPAN_HOST_DEVICE
    constexpr index_type __compute_offset(size_t offset, __rank_count<extents_type::rank(), extents_type::rank()>) const {
      return static_cast<index_type>(offset);
    }

    _MDSPAN_HOST_DEVICE
    constexpr index_type __compute_offset(__rank_count<0,0>) const { return 0; }

  public:

    //--------------------------------------------------------------------------------

    MDSPAN_INLINE_FUNCTION_DEFAULTED constexpr mapping() noexcept = default;
    MDSPAN_INLINE_FUNCTION_DEFAULTED constexpr mapping(mapping const&) noexcept = default;

    _MDSPAN_HOST_DEVICE
    constexpr mapping(extents_type const& __exts) noexcept
      :__extents(__exts)
    { }

    MDSPAN_TEMPLATE_REQUIRES(
      class OtherExtents,
      /* requires */ (
        _MDSPAN_TRAIT(std::is_constructible, extents_type, OtherExtents)
      )
    )
    MDSPAN_CONDITIONAL_EXPLICIT((!std::is_convertible<OtherExtents, extents_type>::value)) // needs two () due to comma
    MDSPAN_INLINE_FUNCTION _MDSPAN_CONSTEXPR_14
    mapping(mapping<OtherExtents> const& other) noexcept // NOLINT(google-explicit-constructor)
      :__extents(other.extents())
    {
       /*
        * TODO: check precondition
        * other.required_span_size() is a representable value of type index_type
        */
    }

    MDSPAN_TEMPLATE_REQUIRES(
      class OtherExtents,
      /* requires */ (
        _MDSPAN_TRAIT(std::is_constructible, extents_type, OtherExtents) &&
        (extents_type::rank() <= 1)
      )
    )
    MDSPAN_CONDITIONAL_EXPLICIT((!std::is_convertible<OtherExtents, extents_type>::value)) // needs two () due to comma
    MDSPAN_INLINE_FUNCTION _MDSPAN_CONSTEXPR_14
    mapping(layout_left::mapping<OtherExtents> const& other) noexcept // NOLINT(google-explicit-constructor)
      :__extents(other.extents())
    {
       /*
        * TODO: check precondition
        * other.required_span_size() is a representable value of type index_type
        */
    }

    MDSPAN_TEMPLATE_REQUIRES(
      class OtherExtents,
      /* requires */ (
        _MDSPAN_TRAIT(std::is_constructible, extents_type, OtherExtents)
      )
    )
    MDSPAN_CONDITIONAL_EXPLICIT((extents_type::rank() > 0))
    MDSPAN_INLINE_FUNCTION _MDSPAN_CONSTEXPR_14
    mapping(layout_stride::mapping<OtherExtents> const& other) noexcept // NOLINT(google-explicit-constructor)
      :__extents(other.extents())
    {
       /*
        * TODO: check precondition
        * other.required_span_size() is a representable value of type index_type
        */
       #if !defined(_MDSPAN_HAS_CUDA) && !defined(_MDSPAN_HAS_HIP) && !defined(NDEBUG)
       index_type stride = 1;
       for(rank_type r=__extents.rank(); r>0; r--) {
         if(stride != static_cast<index_type>(other.stride(r-1))) {
           // Note this throw will lead to a terminate if triggered since this function is marked noexcept
           throw std::runtime_error("Assigning layout_stride to layout_right with invalid strides.");
         }
         stride *= __extents.extent(r-1);
       }
       #endif
    }

    MDSPAN_INLINE_FUNCTION_DEFAULTED _MDSPAN_CONSTEXPR_14_DEFAULTED mapping& operator=(mapping const&) noexcept = default;

    MDSPAN_INLINE_FUNCTION
    constexpr const extents_type& extents() const noexcept {
      return __extents;
    }

    MDSPAN_INLINE_FUNCTION
    constexpr index_type required_span_size() const noexcept {
      index_type value = 1;
      for(rank_type r=0; r != extents_type::rank(); ++r) value*=__extents.extent(r);
      return value;
    }

    //--------------------------------------------------------------------------------

    MDSPAN_TEMPLATE_REQUIRES(
      class... Indices,
      /* requires */ (
        (sizeof...(Indices) == extents_type::rank()) &&
        _MDSPAN_FOLD_AND(
           (_MDSPAN_TRAIT(std::is_convertible, Indices, index_type) &&
            _MDSPAN_TRAIT(std::is_nothrow_constructible, index_type, Indices))
        )
      )
    )
    _MDSPAN_HOST_DEVICE
    constexpr index_type operator()(Indices... idxs) const noexcept {
      return __compute_offset(__rank_count<0, extents_type::rank()>(), static_cast<index_type>(idxs)...);
    }

    MDSPAN_INLINE_FUNCTION static constexpr bool is_always_unique() noexcept { return true; }
    MDSPAN_INLINE_FUNCTION static constexpr bool is_always_exhaustive() noexcept { return true; }
    MDSPAN_INLINE_FUNCTION static constexpr bool is_always_strided() noexcept { return true; }
    MDSPAN_INLINE_FUNCTION constexpr bool is_unique() const noexcept { return true; }
    MDSPAN_INLINE_FUNCTION constexpr bool is_exhaustive() const noexcept { return true; }
    MDSPAN_INLINE_FUNCTION constexpr bool is_strided() const noexcept { return true; }

    MDSPAN_INLINE_FUNCTION
    constexpr index_type stride(rank_type i) const noexcept
#if MDSPAN_HAS_CXX_20
      requires ( Extents::rank() > 0 )
#endif
    {
      index_type value = 1;
      for(rank_type r=extents_type::rank()-1; r>i; r--) value*=__extents.extent(r);
      return value;
    }

    template<class OtherExtents>
    MDSPAN_INLINE_FUNCTION
    friend constexpr bool operator==(mapping const& lhs, mapping<OtherExtents> const& rhs) noexcept {
      return lhs.extents() == rhs.extents();
    }

    // In C++ 20 the not equal exists if equal is found
#if !(MDSPAN_HAS_CXX_20)
    template<class OtherExtents>
    MDSPAN_INLINE_FUNCTION
    friend constexpr bool operator!=(mapping const& lhs, mapping<OtherExtents> const& rhs) noexcept {
      return lhs.extents() != rhs.extents();
    }
#endif

    // Not really public, but currently needed to implement fully constexpr useable submdspan:
    template<size_t N, class SizeType, size_t ... E, size_t ... Idx>
    constexpr index_type __get_stride(MDSPAN_IMPL_STANDARD_NAMESPACE::extents<SizeType, E...>,std::integer_sequence<size_t, Idx...>) const {
      return _MDSPAN_FOLD_TIMES_RIGHT((Idx>N? __extents.template __extent<Idx>():1),1);
    }
    template<size_t N>
    constexpr index_type __stride() const noexcept {
      return __get_stride<N>(__extents, std::make_index_sequence<extents_type::rank()>());
    }

private:
   _MDSPAN_NO_UNIQUE_ADDRESS extents_type __extents{};

};

} // end namespace MDSPAN_IMPL_STANDARD_NAMESPACE

