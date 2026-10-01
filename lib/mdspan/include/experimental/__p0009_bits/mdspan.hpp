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

#include "default_accessor.hpp"
#include "layout_right.hpp"
#include "extents.hpp"
#include "trait_backports.hpp"
#include "compressed_pair.hpp"

namespace MDSPAN_IMPL_STANDARD_NAMESPACE {
template <
  class ElementType,
  class Extents,
  class LayoutPolicy = layout_right,
  class AccessorPolicy = default_accessor<ElementType>
>
class mdspan
{
private:
  static_assert(detail::__is_extents_v<Extents>,
                MDSPAN_IMPL_STANDARD_NAMESPACE_STRING "::mdspan's Extents template parameter must be a specialization of " MDSPAN_IMPL_STANDARD_NAMESPACE_STRING "::extents.");

  // Workaround for non-deducibility of the index sequence template parameter if it's given at the top level
  template <class>
  struct __deduction_workaround;

  template <size_t... Idxs>
  struct __deduction_workaround<std::index_sequence<Idxs...>>
  {
    MDSPAN_FORCE_INLINE_FUNCTION static constexpr
    size_t __size(mdspan const& __self) noexcept {
      return _MDSPAN_FOLD_TIMES_RIGHT((__self.__mapping_ref().extents().extent(Idxs)), /* * ... * */ size_t(1));
    }
    MDSPAN_FORCE_INLINE_FUNCTION static constexpr
    bool __empty(mdspan const& __self) noexcept {
      return (__self.rank()>0) && _MDSPAN_FOLD_OR((__self.__mapping_ref().extents().extent(Idxs)==index_type(0)));
    }
    template <class ReferenceType, class SizeType, size_t N>
    MDSPAN_FORCE_INLINE_FUNCTION static constexpr
    ReferenceType __callop(mdspan const& __self, const std::array<SizeType, N>& indices) noexcept {
      return __self.__accessor_ref().access(__self.__ptr_ref(), __self.__mapping_ref()(indices[Idxs]...));
    }
  };

public:

  //--------------------------------------------------------------------------------
  // Domain and codomain types

  using extents_type = Extents;
  using layout_type = LayoutPolicy;
  using accessor_type = AccessorPolicy;
  using mapping_type = typename layout_type::template mapping<extents_type>;
  using element_type = ElementType;
  using value_type = std::remove_cv_t<element_type>;
  using index_type = typename extents_type::index_type;
  using size_type = typename extents_type::size_type;
  using rank_type = typename extents_type::rank_type;
  using data_handle_type = typename accessor_type::data_handle_type;
  using reference = typename accessor_type::reference;

  MDSPAN_INLINE_FUNCTION static constexpr size_t rank() noexcept { return extents_type::rank(); }
  MDSPAN_INLINE_FUNCTION static constexpr size_t rank_dynamic() noexcept { return extents_type::rank_dynamic(); }
  MDSPAN_INLINE_FUNCTION static constexpr size_t static_extent(size_t r) noexcept { return extents_type::static_extent(r); }
  MDSPAN_INLINE_FUNCTION constexpr index_type extent(size_t r) const noexcept { return __mapping_ref().extents().extent(r); };

private:

  // Can't use defaulted parameter in the __deduction_workaround template because of a bug in MSVC warning C4348.
  using __impl = __deduction_workaround<std::make_index_sequence<extents_type::rank()>>;

  using __map_acc_pair_t = detail::__compressed_pair<mapping_type, accessor_type>;

public:

  //--------------------------------------------------------------------------------
  // [mdspan.basic.cons], mdspan constructors, assignment, and destructor

#if !MDSPAN_HAS_CXX_20
  MDSPAN_INLINE_FUNCTION_DEFAULTED constexpr mdspan() = default;
#else
  MDSPAN_INLINE_FUNCTION_DEFAULTED constexpr mdspan()
    requires(
       // nvhpc has a bug where using just rank_dynamic() here doesn't work ...
       (extents_type::rank_dynamic() > 0) &&
       _MDSPAN_TRAIT(std::is_default_constructible, data_handle_type) &&
       _MDSPAN_TRAIT(std::is_default_constructible, mapping_type) &&
       _MDSPAN_TRAIT(std::is_default_constructible, accessor_type)
     ) = default;
#endif
  MDSPAN_INLINE_FUNCTION_DEFAULTED constexpr mdspan(const mdspan&) = default;
  MDSPAN_INLINE_FUNCTION_DEFAULTED constexpr mdspan(mdspan&&) = default;

  MDSPAN_TEMPLATE_REQUIRES(
    class... SizeTypes,
    /* requires */ (
      _MDSPAN_FOLD_AND(_MDSPAN_TRAIT(std::is_convertible, SizeTypes, index_type) /* && ... */) &&
      _MDSPAN_FOLD_AND(_MDSPAN_TRAIT(std::is_nothrow_constructible, index_type, SizeTypes) /* && ... */) &&
      ((sizeof...(SizeTypes) == rank()) || (sizeof...(SizeTypes) == rank_dynamic())) &&
      _MDSPAN_TRAIT(std::is_constructible, mapping_type, extents_type) &&
      _MDSPAN_TRAIT(std::is_default_constructible, accessor_type)
    )
  )
  MDSPAN_INLINE_FUNCTION
  explicit constexpr mdspan(data_handle_type p, SizeTypes... dynamic_extents)
    // TODO @proposal-bug shouldn't I be allowed to do `move(p)` here?
    : __members(std::move(p), __map_acc_pair_t(mapping_type(extents_type(static_cast<index_type>(std::move(dynamic_extents))...)), accessor_type()))
  { }

  MDSPAN_TEMPLATE_REQUIRES(
    class SizeType, size_t N,
    /* requires */ (
      _MDSPAN_TRAIT(std::is_convertible, SizeType, index_type) &&
      _MDSPAN_TRAIT(std::is_nothrow_constructible, index_type, SizeType) &&
      ((N == rank()) || (N == rank_dynamic())) &&
      _MDSPAN_TRAIT(std::is_constructible, mapping_type, extents_type) &&
      _MDSPAN_TRAIT(std::is_default_constructible, accessor_type)
    )
  )
  MDSPAN_CONDITIONAL_EXPLICIT(N != rank_dynamic())
  MDSPAN_INLINE_FUNCTION
  constexpr mdspan(data_handle_type p, const std::array<SizeType, N>& dynamic_extents)
    : __members(std::move(p), __map_acc_pair_t(mapping_type(extents_type(dynamic_extents)), accessor_type()))
  { }

#ifdef __cpp_lib_span
  MDSPAN_TEMPLATE_REQUIRES(
    class SizeType, size_t N,
    /* requires */ (
      _MDSPAN_TRAIT(std::is_convertible, SizeType, index_type) &&
      _MDSPAN_TRAIT(std::is_nothrow_constructible, index_type, SizeType) &&
      ((N == rank()) || (N == rank_dynamic())) &&
      _MDSPAN_TRAIT(std::is_constructible, mapping_type, extents_type) &&
      _MDSPAN_TRAIT(std::is_default_constructible, accessor_type)
    )
  )
  MDSPAN_CONDITIONAL_EXPLICIT(N != rank_dynamic())
  MDSPAN_INLINE_FUNCTION
  constexpr mdspan(data_handle_type p, std::span<SizeType, N> dynamic_extents)
    : __members(std::move(p), __map_acc_pair_t(mapping_type(extents_type(as_const(dynamic_extents))), accessor_type()))
  { }
#endif

  MDSPAN_FUNCTION_REQUIRES(
    (MDSPAN_INLINE_FUNCTION constexpr),
    mdspan, (data_handle_type p, const extents_type& exts), ,
    /* requires */ (_MDSPAN_TRAIT(std::is_default_constructible, accessor_type) &&
                    _MDSPAN_TRAIT(std::is_constructible, mapping_type, extents_type))
  ) : __members(std::move(p), __map_acc_pair_t(mapping_type(exts), accessor_type()))
  { }

  MDSPAN_FUNCTION_REQUIRES(
    (MDSPAN_INLINE_FUNCTION constexpr),
    mdspan, (data_handle_type p, const mapping_type& m), ,
    /* requires */ (_MDSPAN_TRAIT(std::is_default_constructible, accessor_type))
  ) : __members(std::move(p), __map_acc_pair_t(m, accessor_type()))
  { }

  MDSPAN_INLINE_FUNCTION
  constexpr mdspan(data_handle_type p, const mapping_type& m, const accessor_type& a)
    : __members(std::move(p), __map_acc_pair_t(m, a))
  { }

  MDSPAN_TEMPLATE_REQUIRES(
    class OtherElementType, class OtherExtents, class OtherLayoutPolicy, class OtherAccessor,
    /* requires */ (
      _MDSPAN_TRAIT(std::is_constructible, mapping_type, typename OtherLayoutPolicy::template mapping<OtherExtents>) &&
      _MDSPAN_TRAIT(std::is_constructible, accessor_type, OtherAccessor)
    )
  )
  MDSPAN_INLINE_FUNCTION
  constexpr mdspan(const mdspan<OtherElementType, OtherExtents, OtherLayoutPolicy, OtherAccessor>& other)
    : __members(other.__ptr_ref(), __map_acc_pair_t(other.__mapping_ref(), other.__accessor_ref()))
  {
      static_assert(_MDSPAN_TRAIT(std::is_constructible, data_handle_type, typename OtherAccessor::data_handle_type),"Incompatible data_handle_type for mdspan construction");
      static_assert(_MDSPAN_TRAIT(std::is_constructible, extents_type, OtherExtents),"Incompatible extents for mdspan construction");
      /*
       * TODO: Check precondition
       * For each rank index r of extents_type, static_extent(r) == dynamic_extent || static_extent(r) == other.extent(r) is true.
       */
  }

  /* Might need this on NVIDIA?
  MDSPAN_INLINE_FUNCTION_DEFAULTED
  ~mdspan() = default;
  */

  MDSPAN_INLINE_FUNCTION_DEFAULTED _MDSPAN_CONSTEXPR_14_DEFAULTED mdspan& operator=(const mdspan&) = default;
  MDSPAN_INLINE_FUNCTION_DEFAULTED _MDSPAN_CONSTEXPR_14_DEFAULTED mdspan& operator=(mdspan&&) = default;


  //--------------------------------------------------------------------------------
  // [mdspan.basic.mapping], mdspan mapping domain multidimensional index to access codomain element

  #if MDSPAN_USE_BRACKET_OPERATOR
  MDSPAN_TEMPLATE_REQUIRES(
    class... SizeTypes,
    /* requires */ (
      _MDSPAN_FOLD_AND(_MDSPAN_TRAIT(std::is_convertible, SizeTypes, index_type) /* && ... */) &&
      _MDSPAN_FOLD_AND(_MDSPAN_TRAIT(std::is_nothrow_constructible, index_type, SizeTypes) /* && ... */) &&
      (rank() == sizeof...(SizeTypes))
    )
  )
  MDSPAN_FORCE_INLINE_FUNCTION
  constexpr reference operator[](SizeTypes... indices) const
  {
    return __accessor_ref().access(__ptr_ref(), __mapping_ref()(static_cast<index_type>(std::move(indices))...));
  }
  #endif

  MDSPAN_TEMPLATE_REQUIRES(
    class SizeType,
    /* requires */ (
      _MDSPAN_TRAIT(std::is_convertible, SizeType, index_type) &&
      _MDSPAN_TRAIT(std::is_nothrow_constructible, index_type, SizeType)
    )
  )
  MDSPAN_FORCE_INLINE_FUNCTION
  constexpr reference operator[](const std::array< SizeType, rank()>& indices) const
  {
    return __impl::template __callop<reference>(*this, indices);
  }

  #ifdef __cpp_lib_span
  MDSPAN_TEMPLATE_REQUIRES(
    class SizeType,
    /* requires */ (
      _MDSPAN_TRAIT(std::is_convertible, SizeType, index_type) &&
      _MDSPAN_TRAIT(std::is_nothrow_constructible, index_type, SizeType)
    )
  )
  MDSPAN_FORCE_INLINE_FUNCTION
  constexpr reference operator[](std::span<SizeType, rank()> indices) const
  {
    return __impl::template __callop<reference>(*this, indices);
  }
  #endif // __cpp_lib_span

  #if !MDSPAN_USE_BRACKET_OPERATOR
  MDSPAN_TEMPLATE_REQUIRES(
    class Index,
    /* requires */ (
      _MDSPAN_TRAIT(std::is_convertible, Index, index_type) &&
      _MDSPAN_TRAIT(std::is_nothrow_constructible, index_type, Index) &&
      extents_type::rank() == 1
    )
  )
  MDSPAN_FORCE_INLINE_FUNCTION
  constexpr reference operator[](Index idx) const
  {
    return __accessor_ref().access(__ptr_ref(), __mapping_ref()(static_cast<index_type>(std::move(idx))));
  }
  #endif

  #if MDSPAN_USE_PAREN_OPERATOR
  MDSPAN_TEMPLATE_REQUIRES(
    class... SizeTypes,
    /* requires */ (
      _MDSPAN_FOLD_AND(_MDSPAN_TRAIT(std::is_convertible, SizeTypes, index_type) /* && ... */) &&
      _MDSPAN_FOLD_AND(_MDSPAN_TRAIT(std::is_nothrow_constructible, index_type, SizeTypes) /* && ... */) &&
      extents_type::rank() == sizeof...(SizeTypes)
    )
  )
  MDSPAN_FORCE_INLINE_FUNCTION
  constexpr reference operator()(SizeTypes... indices) const
  {
    return __accessor_ref().access(__ptr_ref(), __mapping_ref()(static_cast<index_type>(std::move(indices))...));
  }

  MDSPAN_TEMPLATE_REQUIRES(
    class SizeType,
    /* requires */ (
      _MDSPAN_TRAIT(std::is_convertible, SizeType, index_type) &&
      _MDSPAN_TRAIT(std::is_nothrow_constructible, index_type, SizeType)
    )
  )
  MDSPAN_FORCE_INLINE_FUNCTION
  constexpr reference operator()(const std::array<SizeType, rank()>& indices) const
  {
    return __impl::template __callop<reference>(*this, indices);
  }

  #ifdef __cpp_lib_span
  MDSPAN_TEMPLATE_REQUIRES(
    class SizeType,
    /* requires */ (
      _MDSPAN_TRAIT(std::is_convertible, SizeType, index_type) &&
      _MDSPAN_TRAIT(std::is_nothrow_constructible, index_type, SizeType)
    )
  )
  MDSPAN_FORCE_INLINE_FUNCTION
  constexpr reference operator()(std::span<SizeType, rank()> indices) const
  {
    return __impl::template __callop<reference>(*this, indices);
  }
  #endif // __cpp_lib_span
  #endif // MDSPAN_USE_PAREN_OPERATOR

  MDSPAN_INLINE_FUNCTION constexpr size_t size() const noexcept {
    return __impl::__size(*this);
  };

  MDSPAN_INLINE_FUNCTION constexpr bool empty() const noexcept {
    return __impl::__empty(*this);
  };

  MDSPAN_INLINE_FUNCTION
  friend constexpr void swap(mdspan& x, mdspan& y) noexcept {
    // can't call the std::swap inside on HIP
    #if !defined(_MDSPAN_HAS_HIP) && !defined(_MDSPAN_HAS_CUDA)
    using std::swap;
    swap(x.__ptr_ref(), y.__ptr_ref());
    swap(x.__mapping_ref(), y.__mapping_ref());
    swap(x.__accessor_ref(), y.__accessor_ref());
    #else
    mdspan tmp = y;
    y = x;
    x = tmp;
    #endif
  }

  //--------------------------------------------------------------------------------
  // [mdspan.basic.domobs], mdspan observers of the domain multidimensional index space


  MDSPAN_INLINE_FUNCTION constexpr const extents_type& extents() const noexcept { return __mapping_ref().extents(); };
  MDSPAN_INLINE_FUNCTION constexpr const data_handle_type& data_handle() const noexcept { return __ptr_ref(); };
  MDSPAN_INLINE_FUNCTION constexpr const mapping_type& mapping() const noexcept { return __mapping_ref(); };
  MDSPAN_INLINE_FUNCTION constexpr const accessor_type& accessor() const noexcept { return __accessor_ref(); };

  //--------------------------------------------------------------------------------
  // [mdspan.basic.obs], mdspan observers of the mapping

  MDSPAN_INLINE_FUNCTION static constexpr bool is_always_unique() noexcept { return mapping_type::is_always_unique(); };
  MDSPAN_INLINE_FUNCTION static constexpr bool is_always_exhaustive() noexcept { return mapping_type::is_always_exhaustive(); };
  MDSPAN_INLINE_FUNCTION static constexpr bool is_always_strided() noexcept { return mapping_type::is_always_strided(); };

  MDSPAN_INLINE_FUNCTION constexpr bool is_unique() const noexcept { return __mapping_ref().is_unique(); };
  MDSPAN_INLINE_FUNCTION constexpr bool is_exhaustive() const noexcept { return __mapping_ref().is_exhaustive(); };
  MDSPAN_INLINE_FUNCTION constexpr bool is_strided() const noexcept { return __mapping_ref().is_strided(); };
  MDSPAN_INLINE_FUNCTION constexpr index_type stride(size_t r) const { return __mapping_ref().stride(r); };

private:

  detail::__compressed_pair<data_handle_type, __map_acc_pair_t> __members{};

  MDSPAN_FORCE_INLINE_FUNCTION _MDSPAN_CONSTEXPR_14 data_handle_type& __ptr_ref() noexcept { return __members.__first(); }
  MDSPAN_FORCE_INLINE_FUNCTION constexpr data_handle_type const& __ptr_ref() const noexcept { return __members.__first(); }
  MDSPAN_FORCE_INLINE_FUNCTION _MDSPAN_CONSTEXPR_14 mapping_type& __mapping_ref() noexcept { return __members.__second().__first(); }
  MDSPAN_FORCE_INLINE_FUNCTION constexpr mapping_type const& __mapping_ref() const noexcept { return __members.__second().__first(); }
  MDSPAN_FORCE_INLINE_FUNCTION _MDSPAN_CONSTEXPR_14 accessor_type& __accessor_ref() noexcept { return __members.__second().__second(); }
  MDSPAN_FORCE_INLINE_FUNCTION constexpr accessor_type const& __accessor_ref() const noexcept { return __members.__second().__second(); }

  template <class, class, class, class>
  friend class mdspan;

};

#if defined(_MDSPAN_USE_CLASS_TEMPLATE_ARGUMENT_DEDUCTION)
MDSPAN_TEMPLATE_REQUIRES(
  class ElementType, class... SizeTypes,
  /* requires */ _MDSPAN_FOLD_AND(_MDSPAN_TRAIT(std::is_integral, SizeTypes) /* && ... */) &&
  (sizeof...(SizeTypes) > 0)
)
MDSPAN_DEDUCTION_GUIDE explicit mdspan(ElementType*, SizeTypes...)
  -> mdspan<ElementType, ::MDSPAN_IMPL_STANDARD_NAMESPACE::dextents<size_t, sizeof...(SizeTypes)>>;

MDSPAN_TEMPLATE_REQUIRES(
  class Pointer,
  (_MDSPAN_TRAIT(std::is_pointer, std::remove_reference_t<Pointer>))
)
MDSPAN_DEDUCTION_GUIDE mdspan(Pointer&&) -> mdspan<std::remove_pointer_t<std::remove_reference_t<Pointer>>, extents<size_t>>;

MDSPAN_TEMPLATE_REQUIRES(
  class CArray,
  (_MDSPAN_TRAIT(std::is_array, CArray) && (std::rank_v<CArray> == 1))
)
MDSPAN_DEDUCTION_GUIDE mdspan(CArray&) -> mdspan<std::remove_all_extents_t<CArray>, extents<size_t, ::std::extent_v<CArray,0>>>;

template <class ElementType, class SizeType, size_t N>
MDSPAN_DEDUCTION_GUIDE mdspan(ElementType*, const ::std::array<SizeType, N>&)
  -> mdspan<ElementType, ::MDSPAN_IMPL_STANDARD_NAMESPACE::dextents<size_t, N>>;

#ifdef __cpp_lib_span
template <class ElementType, class SizeType, size_t N>
MDSPAN_DEDUCTION_GUIDE mdspan(ElementType*, ::std::span<SizeType, N>)
  -> mdspan<ElementType, ::MDSPAN_IMPL_STANDARD_NAMESPACE::dextents<size_t, N>>;
#endif

// This one is necessary because all the constructors take `data_handle_type`s, not
// `ElementType*`s, and `data_handle_type` is taken from `accessor_type::data_handle_type`, which
// seems to throw off automatic deduction guides.
template <class ElementType, class SizeType, size_t... ExtentsPack>
MDSPAN_DEDUCTION_GUIDE mdspan(ElementType*, const extents<SizeType, ExtentsPack...>&)
  -> mdspan<ElementType, ::MDSPAN_IMPL_STANDARD_NAMESPACE::extents<SizeType, ExtentsPack...>>;

template <class ElementType, class MappingType>
MDSPAN_DEDUCTION_GUIDE mdspan(ElementType*, const MappingType&)
  -> mdspan<ElementType, typename MappingType::extents_type, typename MappingType::layout_type>;

template <class MappingType, class AccessorType>
MDSPAN_DEDUCTION_GUIDE mdspan(const typename AccessorType::data_handle_type, const MappingType&, const AccessorType&)
  -> mdspan<typename AccessorType::element_type, typename MappingType::extents_type, typename MappingType::layout_type, AccessorType>;
#endif

} // end namespace MDSPAN_IMPL_STANDARD_NAMESPACE
