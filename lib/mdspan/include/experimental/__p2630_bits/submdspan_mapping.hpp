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

#include <array>
#include <type_traits>
#include <tuple>
#include <utility> // index_sequence

namespace MDSPAN_IMPL_STANDARD_NAMESPACE {
namespace MDSPAN_IMPL_PROPOSED_NAMESPACE {
//******************************************
// Return type of submdspan_mapping overloads
//******************************************
template <class Mapping> struct mapping_offset {
  Mapping mapping;
  size_t offset;
};
} // namespace MDSPAN_IMPL_PROPOSED_NAMESPACE

namespace detail {
using MDSPAN_IMPL_PROPOSED_NAMESPACE::detail::first_of;
using MDSPAN_IMPL_PROPOSED_NAMESPACE::detail::stride_of;
using MDSPAN_IMPL_PROPOSED_NAMESPACE::detail::inv_map_rank;

// constructs sub strides
template <class SrcMapping, class... slice_strides, size_t... InvMapIdxs>
MDSPAN_INLINE_FUNCTION
constexpr auto
construct_sub_strides(const SrcMapping &src_mapping,
                      std::index_sequence<InvMapIdxs...>,
                      const std::tuple<slice_strides...> &slices_stride_factor) {
  using index_type = typename SrcMapping::index_type;
  return std::array<typename SrcMapping::index_type, sizeof...(InvMapIdxs)>{
      (static_cast<index_type>(src_mapping.stride(InvMapIdxs)) *
       static_cast<index_type>(std::get<InvMapIdxs>(slices_stride_factor)))...};
}
} // namespace detail

//**********************************
// layout_left submdspan_mapping
//*********************************
namespace detail {

// Figure out whether to preserve layout_left
template <class IndexSequence, size_t SubRank, class... SliceSpecifiers>
struct preserve_layout_left_mapping;

template <class... SliceSpecifiers, size_t... Idx, size_t SubRank>
struct preserve_layout_left_mapping<std::index_sequence<Idx...>, SubRank,
                                    SliceSpecifiers...> {
  constexpr static bool value =
      // Preserve layout for rank 0
      (SubRank == 0) ||
      (
          // Slice specifiers up to subrank need to be full_extent_t - except
          // for the last one which could also be tuple but not a strided index
          // range slice specifiers after subrank are integrals
          ((Idx > SubRank - 1) || // these are only integral slice specifiers
           (std::is_same_v<SliceSpecifiers, full_extent_t>) ||
           ((Idx == SubRank - 1) &&
            std::is_convertible_v<SliceSpecifiers, std::tuple<size_t, size_t>>)) &&
          ...);
};
} // namespace detail

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
// Actual submdspan mapping call
template <class Extents, class... SliceSpecifiers>
MDSPAN_INLINE_FUNCTION
constexpr auto
submdspan_mapping(const layout_left::mapping<Extents> &src_mapping,
                  SliceSpecifiers... slices) {
  using MDSPAN_IMPL_PROPOSED_NAMESPACE::submdspan_extents;
  using MDSPAN_IMPL_PROPOSED_NAMESPACE::mapping_offset;

  // compute sub extents
  using src_ext_t = Extents;
  auto dst_ext = submdspan_extents(src_mapping.extents(), slices...);
  using dst_ext_t = decltype(dst_ext);

  // figure out sub layout type
  constexpr bool preserve_layout = detail::preserve_layout_left_mapping<
      decltype(std::make_index_sequence<src_ext_t::rank()>()), dst_ext_t::rank(),
      SliceSpecifiers...>::value;
  using dst_layout_t =
      std::conditional_t<preserve_layout, layout_left, layout_stride>;
  using dst_mapping_t = typename dst_layout_t::template mapping<dst_ext_t>;

  if constexpr (std::is_same_v<dst_layout_t, layout_left>) {
    // layout_left case
    return mapping_offset<dst_mapping_t>{
        dst_mapping_t(dst_ext),
        static_cast<size_t>(src_mapping(detail::first_of(slices)...))};
  } else {
    // layout_stride case
    auto inv_map = detail::inv_map_rank(
      std::integral_constant<size_t,0>(),
      std::index_sequence<>(),
      slices...);
    return mapping_offset<dst_mapping_t>{
        dst_mapping_t(dst_ext, detail::construct_sub_strides(
                                   src_mapping, inv_map,
    // HIP needs deduction guides to have markups so we need to be explicit
    // NVCC 11.0 has a bug with deduction guide here, tested that 11.2 does not have the issue
    #if defined(_MDSPAN_HAS_HIP) || (defined(__NVCC__) && (__CUDACC_VER_MAJOR__ * 100 + __CUDACC_VER_MINOR__ * 10) < 1120)
                                   std::tuple<decltype(detail::stride_of(slices))...>{detail::stride_of(slices)...})),
    #else
                                   std::tuple{detail::stride_of(slices)...})),
    #endif
        static_cast<size_t>(src_mapping(detail::first_of(slices)...))};
  }
#if defined(__NVCC__) && !defined(__CUDA_ARCH__) && defined(__GNUC__)
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

//**********************************
// layout_right submdspan_mapping
//*********************************
namespace detail {

// Figure out whether to preserve layout_right
template <class IndexSequence, size_t SubRank, class... SliceSpecifiers>
struct preserve_layout_right_mapping;

template <class... SliceSpecifiers, size_t... Idx, size_t SubRank>
struct preserve_layout_right_mapping<std::index_sequence<Idx...>, SubRank,
                                     SliceSpecifiers...> {
  constexpr static size_t SrcRank = sizeof...(SliceSpecifiers);
  constexpr static bool value =
      // Preserve layout for rank 0
      (SubRank == 0) ||
      (
          // The last subrank slice specifiers need to be full_extent_t - except
          // for the srcrank-subrank one which could also be tuple but not a
          // strided index range slice specifiers before srcrank-subrank are
          // integrals
          ((Idx <
            SrcRank - SubRank) || // these are only integral slice specifiers
           (std::is_same_v<SliceSpecifiers, full_extent_t>) ||
           ((Idx == SrcRank - SubRank) &&
            std::is_convertible_v<SliceSpecifiers, std::tuple<size_t, size_t>>)) &&
          ...);
};
} // namespace detail

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
template <class Extents, class... SliceSpecifiers>
MDSPAN_INLINE_FUNCTION
constexpr auto
submdspan_mapping(const layout_right::mapping<Extents> &src_mapping,
                  SliceSpecifiers... slices) {
  using MDSPAN_IMPL_PROPOSED_NAMESPACE::submdspan_extents;
  using MDSPAN_IMPL_PROPOSED_NAMESPACE::mapping_offset;

  // get sub extents
  using src_ext_t = Extents;
  auto dst_ext = submdspan_extents(src_mapping.extents(), slices...);
  using dst_ext_t = decltype(dst_ext);

  // determine new layout type
  constexpr bool preserve_layout = detail::preserve_layout_right_mapping<
      decltype(std::make_index_sequence<src_ext_t::rank()>()), dst_ext_t::rank(),
      SliceSpecifiers...>::value;
  using dst_layout_t =
      std::conditional_t<preserve_layout, layout_right, layout_stride>;
  using dst_mapping_t = typename dst_layout_t::template mapping<dst_ext_t>;

  if constexpr (std::is_same_v<dst_layout_t, layout_right>) {
    // layout_right case
    return mapping_offset<dst_mapping_t>{
        dst_mapping_t(dst_ext),
        static_cast<size_t>(src_mapping(detail::first_of(slices)...))};
  } else {
    // layout_stride case
    auto inv_map = detail::inv_map_rank(
      std::integral_constant<size_t,0>(),
      std::index_sequence<>(),
      slices...);
    return mapping_offset<dst_mapping_t>{
        dst_mapping_t(dst_ext, detail::construct_sub_strides(
                                   src_mapping, inv_map,
    // HIP needs deduction guides to have markups so we need to be explicit
    // NVCC 11.0 has a bug with deduction guide here, tested that 11.2 does not have the issue
    #if defined(_MDSPAN_HAS_HIP) || (defined(__NVCC__) && (__CUDACC_VER_MAJOR__ * 100 + __CUDACC_VER_MINOR__ * 10) < 1120)
                                   std::tuple<decltype(detail::stride_of(slices))...>{detail::stride_of(slices)...})),
    #else
                                   std::tuple{detail::stride_of(slices)...})),
    #endif
        static_cast<size_t>(src_mapping(detail::first_of(slices)...))};
  }
#if defined(__NVCC__) && !defined(__CUDA_ARCH__) && defined(__GNUC__)
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

//**********************************
// layout_stride submdspan_mapping
//*********************************
template <class Extents, class... SliceSpecifiers>
MDSPAN_INLINE_FUNCTION
constexpr auto
submdspan_mapping(const layout_stride::mapping<Extents> &src_mapping,
                  SliceSpecifiers... slices) {
  using MDSPAN_IMPL_PROPOSED_NAMESPACE::submdspan_extents;
  using MDSPAN_IMPL_PROPOSED_NAMESPACE::mapping_offset;
  auto dst_ext = submdspan_extents(src_mapping.extents(), slices...);
  using dst_ext_t = decltype(dst_ext);
  auto inv_map = detail::inv_map_rank(
      std::integral_constant<size_t,0>(),
      std::index_sequence<>(),
      slices...);
  using dst_mapping_t = typename layout_stride::template mapping<dst_ext_t>;
  return mapping_offset<dst_mapping_t>{
      dst_mapping_t(dst_ext, detail::construct_sub_strides(
                                 src_mapping, inv_map,
    // HIP needs deduction guides to have markups so we need to be explicit
    // NVCC 11.0 has a bug with deduction guide here, tested that 11.2 does not have the issue
    #if defined(_MDSPAN_HAS_HIP) || (defined(__NVCC__) && (__CUDACC_VER_MAJOR__ * 100 + __CUDACC_VER_MINOR__ * 10) < 1120)
                                 std::tuple<decltype(detail::stride_of(slices))...>(detail::stride_of(slices)...))),
#else
                                 std::tuple(detail::stride_of(slices)...))),
#endif
      static_cast<size_t>(src_mapping(detail::first_of(slices)...))};
}
} // namespace MDSPAN_IMPL_STANDARD_NAMESPACE
