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

#include <tuple>

#include "strided_slice.hpp"
namespace MDSPAN_IMPL_STANDARD_NAMESPACE {
namespace MDSPAN_IMPL_PROPOSED_NAMESPACE {
namespace detail {

// Mapping from submapping ranks to srcmapping ranks
// InvMapRank is an index_sequence, which we build recursively
// to contain the mapped indices.
// end of recursion specialization containing the final index_sequence
template <size_t Counter, size_t... MapIdxs>
MDSPAN_INLINE_FUNCTION
constexpr auto inv_map_rank(std::integral_constant<size_t, Counter>, std::index_sequence<MapIdxs...>) {
  return std::index_sequence<MapIdxs...>();
}

// specialization reducing rank by one (i.e., integral slice specifier)
template<size_t Counter, class Slice, class... SliceSpecifiers, size_t... MapIdxs>
MDSPAN_INLINE_FUNCTION
constexpr auto inv_map_rank(std::integral_constant<size_t, Counter>, std::index_sequence<MapIdxs...>, Slice,
                  SliceSpecifiers... slices) {
  using next_idx_seq_t = std::conditional_t<std::is_convertible_v<Slice, size_t>,
                                       std::index_sequence<MapIdxs...>,
                                       std::index_sequence<MapIdxs..., Counter>>;

  return inv_map_rank(std::integral_constant<size_t,Counter + 1>(), next_idx_seq_t(),
                                     slices...);
}

// Helper for identifying strided_slice
template <class T> struct is_strided_slice : std::false_type {};

template <class OffsetType, class ExtentType, class StrideType>
struct is_strided_slice<
    strided_slice<OffsetType, ExtentType, StrideType>> : std::true_type {};

// first_of(slice): getting begin of slice specifier range
MDSPAN_TEMPLATE_REQUIRES(
  class Integral,
  /* requires */(std::is_convertible_v<Integral, size_t>)
)
MDSPAN_INLINE_FUNCTION
constexpr Integral first_of(const Integral &i) {
  return i;
}

MDSPAN_INLINE_FUNCTION
constexpr std::integral_constant<size_t, 0>
first_of(const ::MDSPAN_IMPL_STANDARD_NAMESPACE::full_extent_t &) {
  return std::integral_constant<size_t, 0>();
}

MDSPAN_TEMPLATE_REQUIRES(
  class Slice,
  /* requires */(std::is_convertible_v<Slice, std::tuple<size_t, size_t>>)
)
MDSPAN_INLINE_FUNCTION
constexpr auto first_of(const Slice &i) {
  return std::get<0>(i);
}

template <class OffsetType, class ExtentType, class StrideType>
MDSPAN_INLINE_FUNCTION
constexpr OffsetType
first_of(const strided_slice<OffsetType, ExtentType, StrideType> &r) {
  return r.offset;
}

// last_of(slice): getting end of slice specifier range
// We need however not just the slice but also the extents
// of the original view and which rank from the extents.
// This is needed in the case of slice being full_extent_t.
MDSPAN_TEMPLATE_REQUIRES(
  size_t k, class Extents, class Integral,
  /* requires */(std::is_convertible_v<Integral, size_t>)
)
MDSPAN_INLINE_FUNCTION
constexpr Integral
    last_of(std::integral_constant<size_t, k>, const Extents &, const Integral &i) {
  return i;
}

MDSPAN_TEMPLATE_REQUIRES(
  size_t k, class Extents, class Slice,
  /* requires */(std::is_convertible_v<Slice, std::tuple<size_t, size_t>>)
)
MDSPAN_INLINE_FUNCTION
constexpr auto last_of(std::integral_constant<size_t, k>, const Extents &,
                       const Slice &i) {
  return std::get<1>(i);
}

// Suppress spurious warning with NVCC about no return statement.
// This is a known issue in NVCC and NVC++
// Depending on the CUDA and GCC version we need both the builtin
// and the diagnostic push. I tried really hard to find something shorter
// but no luck ...
#if defined __NVCC__
    #ifdef __NVCC_DIAG_PRAGMA_SUPPORT__
        #pragma nv_diagnostic push
        #pragma nv_diag_suppress = implicit_return_from_non_void_function
    #else
      #ifdef __CUDA_ARCH__
        #pragma diagnostic push
        #pragma diag_suppress implicit_return_from_non_void_function
      #endif
    #endif
#elif defined __NVCOMPILER
    #pragma    diagnostic push
    #pragma    diag_suppress = implicit_return_from_non_void_function
#endif
template <size_t k, class Extents>
MDSPAN_INLINE_FUNCTION
constexpr auto last_of(std::integral_constant<size_t, k>, const Extents &ext,
                       ::MDSPAN_IMPL_STANDARD_NAMESPACE::full_extent_t) {
  if constexpr (Extents::static_extent(k) == dynamic_extent) {
    return ext.extent(k);
  } else {
    return std::integral_constant<size_t, Extents::static_extent(k)>();
  }
#if defined(__NVCC__) && !defined(__CUDA_ARCH__) && defined(__GNUC__)
  // Even with CUDA_ARCH protection this thing warns about calling host function
  __builtin_unreachable();
#endif
}
#if defined __NVCC__
    #ifdef __NVCC_DIAG_PRAGMA_SUPPORT__
        #pragma nv_diagnostic pop
    #else
      #ifdef __CUDA_ARCH__
        #pragma diagnostic pop
      #endif
    #endif
#elif defined __NVCOMPILER
    #pragma    diagnostic pop
#endif

template <size_t k, class Extents, class OffsetType, class ExtentType,
          class StrideType>
MDSPAN_INLINE_FUNCTION
constexpr OffsetType
last_of(std::integral_constant<size_t, k>, const Extents &,
        const strided_slice<OffsetType, ExtentType, StrideType> &r) {
  return r.extent;
}

// get stride of slices
template <class T>
MDSPAN_INLINE_FUNCTION
constexpr auto stride_of(const T &) {
  return std::integral_constant<size_t, 1>();
}

template <class OffsetType, class ExtentType, class StrideType>
MDSPAN_INLINE_FUNCTION
constexpr auto
stride_of(const strided_slice<OffsetType, ExtentType, StrideType> &r) {
  return r.stride;
}

// divide which can deal with integral constant preservation
template <class IndexT, class T0, class T1>
MDSPAN_INLINE_FUNCTION
constexpr auto divide(const T0 &v0, const T1 &v1) {
  return IndexT(v0) / IndexT(v1);
}

template <class IndexT, class T0, T0 v0, class T1, T1 v1>
MDSPAN_INLINE_FUNCTION
constexpr auto divide(const std::integral_constant<T0, v0> &,
                      const std::integral_constant<T1, v1> &) {
  // cutting short division by zero
  // this is used for strided_slice with zero extent/stride
  return std::integral_constant<IndexT, v0 == 0 ? 0 : v0 / v1>();
}

// multiply which can deal with integral constant preservation
template <class IndexT, class T0, class T1>
MDSPAN_INLINE_FUNCTION
constexpr auto multiply(const T0 &v0, const T1 &v1) {
  return IndexT(v0) * IndexT(v1);
}

template <class IndexT, class T0, T0 v0, class T1, T1 v1>
MDSPAN_INLINE_FUNCTION
constexpr auto multiply(const std::integral_constant<T0, v0> &,
                        const std::integral_constant<T1, v1> &) {
  return std::integral_constant<IndexT, v0 * v1>();
}

// compute new static extent from range, preserving static knowledge
template <class Arg0, class Arg1> struct StaticExtentFromRange {
  constexpr static size_t value = dynamic_extent;
};

template <class Integral0, Integral0 val0, class Integral1, Integral1 val1>
struct StaticExtentFromRange<std::integral_constant<Integral0, val0>,
                             std::integral_constant<Integral1, val1>> {
  constexpr static size_t value = val1 - val0;
};

// compute new static extent from strided_slice, preserving static
// knowledge
template <class Arg0, class Arg1> struct StaticExtentFromStridedRange {
  constexpr static size_t value = dynamic_extent;
};

template <class Integral0, Integral0 val0, class Integral1, Integral1 val1>
struct StaticExtentFromStridedRange<std::integral_constant<Integral0, val0>,
                                    std::integral_constant<Integral1, val1>> {
  constexpr static size_t value = val0 > 0 ? 1 + (val0 - 1) / val1 : 0;
};

// creates new extents through recursive calls to next_extent member function
// next_extent has different overloads for different types of stride specifiers
template <size_t K, class Extents, size_t... NewExtents>
struct extents_constructor {
  MDSPAN_TEMPLATE_REQUIRES(
    class Slice, class... SlicesAndExtents,
    /* requires */(!std::is_convertible_v<Slice, size_t> &&
                   !is_strided_slice<Slice>::value)
  )
  MDSPAN_INLINE_FUNCTION
  constexpr static auto next_extent(const Extents &ext, const Slice &sl,
                                    SlicesAndExtents... slices_and_extents) {
    constexpr size_t new_static_extent = StaticExtentFromRange<
        decltype(first_of(std::declval<Slice>())),
        decltype(last_of(std::integral_constant<size_t, Extents::rank() - K>(),
                         std::declval<Extents>(),
                         std::declval<Slice>()))>::value;

    using next_t =
        extents_constructor<K - 1, Extents, NewExtents..., new_static_extent>;
    using index_t = typename Extents::index_type;
    return next_t::next_extent(
        ext, slices_and_extents...,
        index_t(last_of(std::integral_constant<size_t, Extents::rank() - K>(), ext,
                        sl)) -
            index_t(first_of(sl)));
  }

  MDSPAN_TEMPLATE_REQUIRES(
    class Slice, class... SlicesAndExtents,
    /* requires */ (std::is_convertible_v<Slice, size_t>)
  )
  MDSPAN_INLINE_FUNCTION
  constexpr static auto next_extent(const Extents &ext, const Slice &,
                                    SlicesAndExtents... slices_and_extents) {
    using next_t = extents_constructor<K - 1, Extents, NewExtents...>;
    return next_t::next_extent(ext, slices_and_extents...);
  }

  template <class OffsetType, class ExtentType, class StrideType,
            class... SlicesAndExtents>
  MDSPAN_INLINE_FUNCTION
  constexpr static auto
  next_extent(const Extents &ext,
              const strided_slice<OffsetType, ExtentType, StrideType> &r,
              SlicesAndExtents... slices_and_extents) {
    using index_t = typename Extents::index_type;
    using new_static_extent_t =
        StaticExtentFromStridedRange<ExtentType, StrideType>;
    if constexpr (new_static_extent_t::value == dynamic_extent) {
      using next_t =
          extents_constructor<K - 1, Extents, NewExtents..., dynamic_extent>;
      return next_t::next_extent(
          ext, slices_and_extents...,
          r.extent > 0 ? 1 + divide<index_t>(r.extent - 1, r.stride) : 0);
    } else {
      constexpr size_t new_static_extent = new_static_extent_t::value;
      using next_t =
          extents_constructor<K - 1, Extents, NewExtents..., new_static_extent>;
      return next_t::next_extent(
          ext, slices_and_extents..., index_t(divide<index_t>(ExtentType(), StrideType())));
    }
  }
};

template <class Extents, size_t... NewStaticExtents>
struct extents_constructor<0, Extents, NewStaticExtents...> {

  template <class... NewExtents>
  MDSPAN_INLINE_FUNCTION
  constexpr static auto next_extent(const Extents &, NewExtents... new_exts) {
    return extents<typename Extents::index_type, NewStaticExtents...>(
        new_exts...);
  }
};

} // namespace detail

// submdspan_extents creates new extents given src extents and submdspan slice
// specifiers
template <class IndexType, size_t... Extents, class... SliceSpecifiers>
MDSPAN_INLINE_FUNCTION
constexpr auto submdspan_extents(const extents<IndexType, Extents...> &src_exts,
                                 SliceSpecifiers... slices) {

  using ext_t = extents<IndexType, Extents...>;
  return detail::extents_constructor<ext_t::rank(), ext_t>::next_extent(
      src_exts, slices...);
}
} // namespace MDSPAN_IMPL_PROPOSED_NAMESPACE
} // namespace MDSPAN_IMPL_STANDARD_NAMESPACE
