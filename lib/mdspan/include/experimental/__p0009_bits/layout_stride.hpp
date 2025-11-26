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
#include "extents.hpp"
#include "trait_backports.hpp"
#include "compressed_pair.hpp"

#if !defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)
#  include "no_unique_address.hpp"
#endif

#include <algorithm>
#include <numeric>
#include <array>
#ifdef __cpp_lib_span
#include <span>
#endif
#if defined(_MDSPAN_USE_CONCEPTS) && MDSPAN_HAS_CXX_20 && defined(__cpp_lib_concepts)
#  include <concepts>
#endif

namespace MDSPAN_IMPL_STANDARD_NAMESPACE {

struct layout_left {
  template<class Extents>
    class mapping;
};
struct layout_right {
  template<class Extents>
    class mapping;
};

namespace detail {
  template<class Layout, class Mapping>
  constexpr bool __is_mapping_of =
    std::is_same<typename Layout::template mapping<typename Mapping::extents_type>, Mapping>::value;

#if defined(_MDSPAN_USE_CONCEPTS) && MDSPAN_HAS_CXX_20
#  if !defined(__cpp_lib_concepts)
  namespace internal {
  namespace detail {
  template <typename _Tp, typename _Up>
  concept __same_as = std::is_same_v<_Tp, _Up>;
  } // namespace detail
  template <class T, class U>
  concept __same_as = detail::__same_as<T, U> && detail::__same_as<U, T>;
  } // namespace internal
#  endif

  template<class M>
  concept __layout_mapping_alike = requires {
    requires __is_extents<typename M::extents_type>::value;
#if defined(__cpp_lib_concepts)
    { M::is_always_strided() } -> std::same_as<bool>;
    { M::is_always_exhaustive() } -> std::same_as<bool>;
    { M::is_always_unique() } -> std::same_as<bool>;
#else
    { M::is_always_strided() } -> internal::__same_as<bool>;
    { M::is_always_exhaustive() } -> internal::__same_as<bool>;
    { M::is_always_unique() } -> internal::__same_as<bool>;
#endif
    std::bool_constant<M::is_always_strided()>::value;
    std::bool_constant<M::is_always_exhaustive()>::value;
    std::bool_constant<M::is_always_unique()>::value;
  };
#endif
} // namespace detail

struct layout_stride {
  template <class Extents>
  class mapping
#if !defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)
    : private detail::__no_unique_address_emulation<
        detail::__compressed_pair<
          Extents,
          std::array<typename Extents::index_type, Extents::rank()>
        >
      >
#endif
  {
  public:
    using extents_type = Extents;
    using index_type = typename extents_type::index_type;
    using size_type = typename extents_type::size_type;
    using rank_type = typename extents_type::rank_type;
    using layout_type = layout_stride;

    // This could be a `requires`, but I think it's better and clearer as a `static_assert`.
    static_assert(detail::__is_extents_v<Extents>,
                  MDSPAN_IMPL_STANDARD_NAMESPACE_STRING "::layout_stride::mapping must be instantiated with a specialization of " MDSPAN_IMPL_STANDARD_NAMESPACE_STRING "::extents.");


  private:

    //----------------------------------------------------------------------------

    using __strides_storage_t = std::array<index_type, extents_type::rank()>;
    using __member_pair_t = detail::__compressed_pair<extents_type, __strides_storage_t>;

#if defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)
    _MDSPAN_NO_UNIQUE_ADDRESS __member_pair_t __members;
#else
    using __base_t = detail::__no_unique_address_emulation<__member_pair_t>;
#endif

    MDSPAN_FORCE_INLINE_FUNCTION constexpr __strides_storage_t const&
    __strides_storage() const noexcept {
#if defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)
      return __members.__second();
#else
      return this->__base_t::__ref().__second();
#endif
    }
    MDSPAN_FORCE_INLINE_FUNCTION _MDSPAN_CONSTEXPR_14 __strides_storage_t&
    __strides_storage() noexcept {
#if defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)
      return __members.__second();
#else
      return this->__base_t::__ref().__second();
#endif
    }

    template<class SizeType, size_t ... Ep, size_t ... Idx>
    _MDSPAN_HOST_DEVICE
    constexpr index_type __get_size(::MDSPAN_IMPL_STANDARD_NAMESPACE::extents<SizeType, Ep...>,std::integer_sequence<size_t, Idx...>) const {
      return _MDSPAN_FOLD_TIMES_RIGHT( static_cast<index_type>(extents().extent(Idx)), 1 );
    }

    //----------------------------------------------------------------------------

    template <class>
    friend class mapping;

    //----------------------------------------------------------------------------

    // Workaround for non-deducibility of the index sequence template parameter if it's given at the top level
    template <class>
    struct __deduction_workaround;

    template <size_t... Idxs>
    struct __deduction_workaround<std::index_sequence<Idxs...>>
    {
      template <class OtherExtents>
      MDSPAN_INLINE_FUNCTION
      static constexpr bool _eq_impl(mapping const& self, mapping<OtherExtents> const& other) noexcept {
        return    _MDSPAN_FOLD_AND((self.stride(Idxs) == other.stride(Idxs)) /* && ... */)
               && _MDSPAN_FOLD_AND((self.extents().extent(Idxs) == other.extents().extent(Idxs)) /* || ... */);
      }
      template <class OtherExtents>
      MDSPAN_INLINE_FUNCTION
      static constexpr bool _not_eq_impl(mapping const& self, mapping<OtherExtents> const& other) noexcept {
        return    _MDSPAN_FOLD_OR((self.stride(Idxs) != other.stride(Idxs)) /* || ... */)
               || _MDSPAN_FOLD_OR((self.extents().extent(Idxs) != other.extents().extent(Idxs)) /* || ... */);
      }

      template <class... Integral>
      MDSPAN_FORCE_INLINE_FUNCTION
      static constexpr size_t _call_op_impl(mapping const& self, Integral... idxs) noexcept {
        return _MDSPAN_FOLD_PLUS_RIGHT((idxs * self.stride(Idxs)), /* + ... + */ 0);
      }

      MDSPAN_INLINE_FUNCTION
      static constexpr size_t _req_span_size_impl(mapping const& self) noexcept {
        // assumes no negative strides; not sure if I'm allowed to assume that or not
        return __impl::_call_op_impl(self, (self.extents().template __extent<Idxs>() - 1)...) + 1;
      }

      template<class OtherMapping>
      MDSPAN_INLINE_FUNCTION
      static constexpr const __strides_storage_t fill_strides(const OtherMapping& map) {
        return __strides_storage_t{static_cast<index_type>(map.stride(Idxs))...};
      }

      MDSPAN_INLINE_FUNCTION
      static constexpr const __strides_storage_t& fill_strides(const __strides_storage_t& s) {
        return s;
      }

      template<class IntegralType>
      MDSPAN_INLINE_FUNCTION
      static constexpr const __strides_storage_t fill_strides(const std::array<IntegralType,extents_type::rank()>& s) {
        return __strides_storage_t{static_cast<index_type>(s[Idxs])...};
      }

#ifdef __cpp_lib_span
      template<class IntegralType>
      MDSPAN_INLINE_FUNCTION
      static constexpr const __strides_storage_t fill_strides(const std::span<IntegralType,extents_type::rank()>& s) {
        return __strides_storage_t{static_cast<index_type>(s[Idxs])...};
      }
#endif

      template<size_t K>
      MDSPAN_INLINE_FUNCTION
      static constexpr size_t __return_zero() { return 0; }

      template<class Mapping>
      MDSPAN_INLINE_FUNCTION
      static constexpr typename Mapping::index_type
        __OFFSET(const Mapping& m) { return m(__return_zero<Idxs>()...); }
    };

    // Can't use defaulted parameter in the __deduction_workaround template because of a bug in MSVC warning C4348.
    using __impl = __deduction_workaround<std::make_index_sequence<Extents::rank()>>;


    //----------------------------------------------------------------------------

#if defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)
    MDSPAN_INLINE_FUNCTION constexpr explicit
    mapping(__member_pair_t&& __m) : __members(::std::move(__m)) {}
#else
    MDSPAN_INLINE_FUNCTION constexpr explicit
    mapping(__base_t&& __b) : __base_t(::std::move(__b)) {}
#endif

  public:

    //--------------------------------------------------------------------------------

    MDSPAN_INLINE_FUNCTION_DEFAULTED constexpr mapping() noexcept = default;
    MDSPAN_INLINE_FUNCTION_DEFAULTED constexpr mapping(mapping const&) noexcept = default;

    MDSPAN_TEMPLATE_REQUIRES(
      class IntegralTypes,
      /* requires */ (
        // MSVC 19.32 does not like using index_type here, requires the typename Extents::index_type
        // error C2641: cannot deduce template arguments for 'MDSPAN_IMPL_STANDARD_NAMESPACE::layout_stride::mapping'
        _MDSPAN_TRAIT(std::is_convertible, const std::remove_const_t<IntegralTypes>&, typename Extents::index_type) &&
        _MDSPAN_TRAIT(std::is_nothrow_constructible, typename Extents::index_type, const std::remove_const_t<IntegralTypes>&)
      )
    )
    MDSPAN_INLINE_FUNCTION
    constexpr
    mapping(
      extents_type const& e,
      std::array<IntegralTypes, extents_type::rank()> const& s
    ) noexcept
#if defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)
      : __members{
#else
      : __base_t(__base_t{__member_pair_t(
#endif
          e, __strides_storage_t(__impl::fill_strides(s))
#if defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)
        }
#else
        )})
#endif
    {
      /*
       * TODO: check preconditions
       * - s[i] > 0 is true for all i in the range [0, rank_ ).
       * - REQUIRED-SPAN-SIZE(e, s) is a representable value of type index_type ([basic.fundamental]).
       * - If rank_ is greater than 0, then there exists a permutation P of the integers in the
       *   range [0, rank_), such that s[ pi ] >= s[ pi − 1 ] * e.extent( pi − 1 ) is true for
       *   all i in the range [1, rank_ ), where pi is the ith element of P.
       */
    }

#ifdef __cpp_lib_span
    MDSPAN_TEMPLATE_REQUIRES(
      class IntegralTypes,
      /* requires */ (
        // MSVC 19.32 does not like using index_type here, requires the typename Extents::index_type
        // error C2641: cannot deduce template arguments for 'MDSPAN_IMPL_STANDARD_NAMESPACE::layout_stride::mapping'
        _MDSPAN_TRAIT(std::is_convertible, const std::remove_const_t<IntegralTypes>&, typename Extents::index_type) &&
        _MDSPAN_TRAIT(std::is_nothrow_constructible, typename Extents::index_type, const std::remove_const_t<IntegralTypes>&)
      )
    )
    MDSPAN_INLINE_FUNCTION
    constexpr
    mapping(
      extents_type const& e,
      std::span<IntegralTypes, extents_type::rank()> const& s
    ) noexcept
#if defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)
      : __members{
#else
      : __base_t(__base_t{__member_pair_t(
#endif
          e, __strides_storage_t(__impl::fill_strides(s))
#if defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)
        }
#else
        )})
#endif
    {
      /*
       * TODO: check preconditions
       * - s[i] > 0 is true for all i in the range [0, rank_ ).
       * - REQUIRED-SPAN-SIZE(e, s) is a representable value of type index_type ([basic.fundamental]).
       * - If rank_ is greater than 0, then there exists a permutation P of the integers in the
       *   range [0, rank_), such that s[ pi ] >= s[ pi − 1 ] * e.extent( pi − 1 ) is true for
       *   all i in the range [1, rank_ ), where pi is the ith element of P.
       */
    }
#endif // __cpp_lib_span

#if !(defined(_MDSPAN_USE_CONCEPTS) && MDSPAN_HAS_CXX_20)
    MDSPAN_TEMPLATE_REQUIRES(
      class StridedLayoutMapping,
      /* requires */ (
        _MDSPAN_TRAIT(std::is_constructible, extents_type, typename StridedLayoutMapping::extents_type) &&
        detail::__is_mapping_of<typename StridedLayoutMapping::layout_type, StridedLayoutMapping> &&
        StridedLayoutMapping::is_always_unique() &&
        StridedLayoutMapping::is_always_strided()
      )
    )
#else
    template<class StridedLayoutMapping>
    requires(
         detail::__layout_mapping_alike<StridedLayoutMapping> &&
         _MDSPAN_TRAIT(std::is_constructible, extents_type, typename StridedLayoutMapping::extents_type) &&
         StridedLayoutMapping::is_always_unique() &&
         StridedLayoutMapping::is_always_strided()
    )
#endif
    MDSPAN_CONDITIONAL_EXPLICIT(
      (!std::is_convertible<typename StridedLayoutMapping::extents_type, extents_type>::value) &&
      (detail::__is_mapping_of<layout_left, StridedLayoutMapping> ||
       detail::__is_mapping_of<layout_right, StridedLayoutMapping> ||
       detail::__is_mapping_of<layout_stride, StridedLayoutMapping>)
    ) // needs two () due to comma
    MDSPAN_INLINE_FUNCTION _MDSPAN_CONSTEXPR_14
    mapping(StridedLayoutMapping const& other) noexcept // NOLINT(google-explicit-constructor)
#if defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)
      : __members{
#else
      : __base_t(__base_t{__member_pair_t(
#endif
          other.extents(), __strides_storage_t(__impl::fill_strides(other))
#if defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)
        }
#else
        )})
#endif
    {
      /*
       * TODO: check preconditions
       * - other.stride(i) > 0 is true for all i in the range [0, rank_ ).
       * - other.required_span_size() is a representable value of type index_type ([basic.fundamental]).
       * - OFFSET(other) == 0
       */
    }

    //--------------------------------------------------------------------------------

    MDSPAN_INLINE_FUNCTION_DEFAULTED _MDSPAN_CONSTEXPR_14_DEFAULTED
    mapping& operator=(mapping const&) noexcept = default;

    MDSPAN_INLINE_FUNCTION constexpr const extents_type& extents() const noexcept {
#if defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)
      return __members.__first();
#else
      return this->__base_t::__ref().__first();
#endif
    };

    MDSPAN_INLINE_FUNCTION
    constexpr std::array< index_type, extents_type::rank() > strides() const noexcept {
      return __strides_storage();
    }

    MDSPAN_INLINE_FUNCTION
    constexpr index_type required_span_size() const noexcept {
      index_type span_size = 1;
      for(unsigned r = 0; r < extents_type::rank(); r++) {
        // Return early if any of the extents are zero
        if(extents().extent(r)==0) return 0;
        span_size += ( static_cast<index_type>(extents().extent(r) - 1 ) * __strides_storage()[r]);
      }
      return span_size;
    }


    MDSPAN_TEMPLATE_REQUIRES(
      class... Indices,
      /* requires */ (
        sizeof...(Indices) == Extents::rank() &&
        _MDSPAN_FOLD_AND(_MDSPAN_TRAIT(std::is_convertible, Indices, index_type) /*&& ...*/ ) &&
        _MDSPAN_FOLD_AND(_MDSPAN_TRAIT(std::is_nothrow_constructible, index_type, Indices) /*&& ...*/)
      )
    )
    MDSPAN_FORCE_INLINE_FUNCTION
    constexpr index_type operator()(Indices... idxs) const noexcept {
      return static_cast<index_type>(__impl::_call_op_impl(*this, static_cast<index_type>(idxs)...));
    }

    MDSPAN_INLINE_FUNCTION static constexpr bool is_always_unique() noexcept { return true; }
    MDSPAN_INLINE_FUNCTION static constexpr bool is_always_exhaustive() noexcept {
      return false;
    }
    MDSPAN_INLINE_FUNCTION static constexpr bool is_always_strided() noexcept { return true; }

    MDSPAN_INLINE_FUNCTION static constexpr bool is_unique() noexcept { return true; }
    MDSPAN_INLINE_FUNCTION _MDSPAN_CONSTEXPR_14 bool is_exhaustive() const noexcept {
      return required_span_size() == __get_size(extents(), std::make_index_sequence<extents_type::rank()>());
    }
    MDSPAN_INLINE_FUNCTION static constexpr bool is_strided() noexcept { return true; }


    MDSPAN_INLINE_FUNCTION
    constexpr index_type stride(rank_type r) const noexcept
#if MDSPAN_HAS_CXX_20
      requires ( Extents::rank() > 0 )
#endif
    {
      return __strides_storage()[r];
    }

#if !(defined(_MDSPAN_USE_CONCEPTS) && MDSPAN_HAS_CXX_20)
    MDSPAN_TEMPLATE_REQUIRES(
      class StridedLayoutMapping,
      /* requires */ (
        detail::__is_mapping_of<typename StridedLayoutMapping::layout_type, StridedLayoutMapping> &&
        (extents_type::rank() == StridedLayoutMapping::extents_type::rank()) &&
        StridedLayoutMapping::is_always_strided()
      )
    )
#else
    template<class StridedLayoutMapping>
    requires(
         detail::__layout_mapping_alike<StridedLayoutMapping> &&
         (extents_type::rank() == StridedLayoutMapping::extents_type::rank()) &&
         StridedLayoutMapping::is_always_strided()
    )
#endif
    MDSPAN_INLINE_FUNCTION
    friend constexpr bool operator==(const mapping& x, const StridedLayoutMapping& y) noexcept {
      bool strides_match = true;
      for(rank_type r = 0; r < extents_type::rank(); r++)
        strides_match = strides_match && (x.stride(r) == y.stride(r));
      return (x.extents() == y.extents()) &&
             (__impl::__OFFSET(y)== static_cast<typename StridedLayoutMapping::index_type>(0)) &&
             strides_match;
    }

    // This one is not technically part of the proposal. Just here to make implementation a bit more optimal hopefully
    MDSPAN_TEMPLATE_REQUIRES(
      class OtherExtents,
      /* requires */ (
        (extents_type::rank() == OtherExtents::rank())
      )
    )
    MDSPAN_INLINE_FUNCTION
    friend constexpr bool operator==(mapping const& lhs, mapping<OtherExtents> const& rhs) noexcept {
      return __impl::_eq_impl(lhs, rhs);
    }

#if !MDSPAN_HAS_CXX_20
    MDSPAN_TEMPLATE_REQUIRES(
      class StridedLayoutMapping,
      /* requires */ (
        detail::__is_mapping_of<typename StridedLayoutMapping::layout_type, StridedLayoutMapping> &&
        (extents_type::rank() == StridedLayoutMapping::extents_type::rank()) &&
        StridedLayoutMapping::is_always_strided()
      )
    )
    MDSPAN_INLINE_FUNCTION
    friend constexpr bool operator!=(const mapping& x, const StridedLayoutMapping& y) noexcept {
      return not (x == y);
    }

    MDSPAN_TEMPLATE_REQUIRES(
      class OtherExtents,
      /* requires */ (
        (extents_type::rank() == OtherExtents::rank())
      )
    )
    MDSPAN_INLINE_FUNCTION
    friend constexpr bool operator!=(mapping const& lhs, mapping<OtherExtents> const& rhs) noexcept {
      return __impl::_not_eq_impl(lhs, rhs);
    }
#endif

  };
};

} // end namespace MDSPAN_IMPL_STANDARD_NAMESPACE
