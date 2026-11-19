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

#include "../mdspan"
#include <cassert>
#include <vector>

namespace MDSPAN_IMPL_STANDARD_NAMESPACE {
namespace MDSPAN_IMPL_PROPOSED_NAMESPACE {

namespace {
  template<class Extents>
  struct size_of_extents;

  template<class IndexType, size_t ... Extents>
  struct size_of_extents<extents<IndexType, Extents...>> {
    constexpr static size_t value() {
      size_t size = 1;
      for(size_t r=0; r<extents<IndexType, Extents...>::rank(); r++)
        size *= extents<IndexType, Extents...>::static_extent(r);
      return size;
    }
  };
}

namespace {
  template<class C>
  struct container_is_array :  std::false_type {
    template<class M>
    static constexpr C construct(const M& m) { return C(m.required_span_size()); }
  };
  template<class T, size_t N>
  struct container_is_array<std::array<T,N>> : std::true_type {
    template<class M>
    static constexpr std::array<T,N> construct(const M&) { return std::array<T,N>(); }
  };
}

template <
  class ElementType,
  class Extents,
  class LayoutPolicy = layout_right,
  class Container = std::vector<ElementType>
>
class mdarray {
private:
  static_assert(::MDSPAN_IMPL_STANDARD_NAMESPACE::detail::__is_extents_v<Extents>,
                MDSPAN_IMPL_PROPOSED_NAMESPACE_STRING "::mdspan's Extents template parameter must be a specialization of " MDSPAN_IMPL_STANDARD_NAMESPACE_STRING "::extents.");

public:

  //--------------------------------------------------------------------------------
  // Domain and codomain types

  using extents_type = Extents;
  using layout_type = LayoutPolicy;
  using container_type = Container;
  using mapping_type = typename layout_type::template mapping<extents_type>;
  using element_type = ElementType;
  using mdspan_type = mdspan<element_type, extents_type, layout_type>;
  using const_mdspan_type = mdspan<const element_type, extents_type, layout_type>;
  using value_type = std::remove_cv_t<element_type>;
  using index_type = typename Extents::index_type;
  using size_type = typename Extents::size_type;
  using rank_type = typename Extents::rank_type;
  using pointer = typename container_type::pointer;
  using reference = typename container_type::reference;
  using const_pointer = typename container_type::const_pointer;
  using const_reference = typename container_type::const_reference;

public:

  //--------------------------------------------------------------------------------
  // [mdspan.basic.cons], mdspan constructors, assignment, and destructor

#if !(MDSPAN_HAS_CXX_20)
  MDSPAN_FUNCTION_REQUIRES(
    (MDSPAN_INLINE_FUNCTION_DEFAULTED constexpr),
    mdarray, (), ,
    /* requires */ (extents_type::rank_dynamic()!=0)) {}
#else
  MDSPAN_INLINE_FUNCTION_DEFAULTED constexpr mdarray() requires(extents_type::rank_dynamic()!=0) = default;
#endif
  MDSPAN_INLINE_FUNCTION_DEFAULTED constexpr mdarray(const mdarray&) = default;
  MDSPAN_INLINE_FUNCTION_DEFAULTED constexpr mdarray(mdarray&&) = default;

  // Constructors for container types constructible from a size
  MDSPAN_TEMPLATE_REQUIRES(
    class... SizeTypes,
    /* requires */ (
      _MDSPAN_FOLD_AND(_MDSPAN_TRAIT( std::is_convertible, SizeTypes, index_type) /* && ... */) &&
      _MDSPAN_TRAIT( std::is_constructible, extents_type, SizeTypes...) &&
      _MDSPAN_TRAIT( std::is_constructible, mapping_type, extents_type) &&
      (_MDSPAN_TRAIT( std::is_constructible, container_type, size_t) ||
       container_is_array<container_type>::value) &&
      (extents_type::rank()>0 || extents_type::rank_dynamic()==0)
    )
  )
  MDSPAN_INLINE_FUNCTION
  explicit constexpr mdarray(SizeTypes... dynamic_extents)
    : map_(extents_type(dynamic_extents...)), ctr_(container_is_array<container_type>::construct(map_))
  { }

  MDSPAN_FUNCTION_REQUIRES(
    (MDSPAN_INLINE_FUNCTION constexpr),
    mdarray, (const extents_type& exts), ,
    /* requires */ ((_MDSPAN_TRAIT( std::is_constructible, container_type, size_t) ||
                     container_is_array<container_type>::value) &&
                    _MDSPAN_TRAIT( std::is_constructible, mapping_type, extents_type))
  ) : map_(exts), ctr_(container_is_array<container_type>::construct(map_))
  { }

  MDSPAN_FUNCTION_REQUIRES(
    (MDSPAN_INLINE_FUNCTION constexpr),
    mdarray, (const mapping_type& m), ,
    /* requires */ (_MDSPAN_TRAIT( std::is_constructible, container_type, size_t) ||
                    container_is_array<container_type>::value)
  ) : map_(m), ctr_(container_is_array<container_type>::construct(map_))
  { }

  // Constructors from container
  MDSPAN_TEMPLATE_REQUIRES(
    class... SizeTypes,
    /* requires */ (
      _MDSPAN_FOLD_AND(_MDSPAN_TRAIT( std::is_convertible, SizeTypes, index_type) /* && ... */) &&
      _MDSPAN_TRAIT( std::is_constructible, extents_type, SizeTypes...) &&
      _MDSPAN_TRAIT( std::is_constructible, mapping_type, extents_type)
    )
  )
  MDSPAN_INLINE_FUNCTION
  explicit constexpr mdarray(const container_type& ctr, SizeTypes... dynamic_extents)
    : map_(extents_type(dynamic_extents...)), ctr_(ctr)
  { assert(ctr.size() >= static_cast<size_t>(map_.required_span_size())); }


  MDSPAN_FUNCTION_REQUIRES(
    (MDSPAN_INLINE_FUNCTION constexpr),
    mdarray, (const container_type& ctr, const extents_type& exts), ,
    /* requires */ (_MDSPAN_TRAIT( std::is_constructible, mapping_type, extents_type))
  ) : map_(exts), ctr_(ctr)
  { assert(ctr.size() >= static_cast<size_t>(map_.required_span_size())); }

  constexpr mdarray(const container_type& ctr, const mapping_type& m)
    : map_(m), ctr_(ctr)
  { assert(ctr.size() >= static_cast<size_t>(map_.required_span_size())); }


  // Constructors from container
  MDSPAN_TEMPLATE_REQUIRES(
    class... SizeTypes,
    /* requires */ (
      _MDSPAN_FOLD_AND(_MDSPAN_TRAIT( std::is_convertible, SizeTypes, index_type) /* && ... */) &&
      _MDSPAN_TRAIT( std::is_constructible, extents_type, SizeTypes...) &&
      _MDSPAN_TRAIT( std::is_constructible, mapping_type, extents_type)
    )
  )
  MDSPAN_INLINE_FUNCTION
  explicit constexpr mdarray(container_type&& ctr, SizeTypes... dynamic_extents)
    : map_(extents_type(dynamic_extents...)), ctr_(std::move(ctr))
  { assert(ctr_.size() >= static_cast<size_t>(map_.required_span_size())); }


  MDSPAN_FUNCTION_REQUIRES(
    (MDSPAN_INLINE_FUNCTION constexpr),
    mdarray, (container_type&& ctr, const extents_type& exts), ,
    /* requires */ (_MDSPAN_TRAIT( std::is_constructible, mapping_type, extents_type))
  ) : map_(exts), ctr_(std::move(ctr))
  { assert(ctr_.size() >= static_cast<size_t>(map_.required_span_size())); }

  constexpr mdarray(container_type&& ctr, const mapping_type& m)
    : map_(m), ctr_(std::move(ctr))
  { assert(ctr_.size() >= static_cast<size_t>(map_.required_span_size())); }



  MDSPAN_TEMPLATE_REQUIRES(
    class OtherElementType, class OtherExtents, class OtherLayoutPolicy, class OtherContainer,
    /* requires */ (
      _MDSPAN_TRAIT( std::is_constructible, mapping_type, typename OtherLayoutPolicy::template mapping<OtherExtents>) &&
      _MDSPAN_TRAIT( std::is_constructible, container_type, OtherContainer)
    )
  )
  MDSPAN_INLINE_FUNCTION
  constexpr mdarray(const mdarray<OtherElementType, OtherExtents, OtherLayoutPolicy, OtherContainer>& other)
    : map_(other.mapping()), ctr_(other.container())
  {
    static_assert( std::is_constructible<extents_type, OtherExtents>::value, "");
  }

  // Constructors for container types constructible from a size and allocator
  MDSPAN_TEMPLATE_REQUIRES(
    class Alloc,
    /* requires */ (_MDSPAN_TRAIT( std::is_constructible, container_type, size_t, Alloc) &&
                    _MDSPAN_TRAIT( std::is_constructible, mapping_type, extents_type))
  )
  MDSPAN_INLINE_FUNCTION
  constexpr mdarray(const extents_type& exts, const Alloc& a)
    : map_(exts), ctr_(map_.required_span_size(), a)
  { }

  MDSPAN_TEMPLATE_REQUIRES(
    class Alloc,
    /* requires */ (_MDSPAN_TRAIT( std::is_constructible, container_type, size_t, Alloc))
  )
  MDSPAN_INLINE_FUNCTION
  constexpr mdarray(const mapping_type& map, const Alloc& a)
    : map_(map), ctr_(map_.required_span_size(), a)
  { }

  // Constructors for container types constructible from a container and allocator
  MDSPAN_TEMPLATE_REQUIRES(
    class Alloc,
    /* requires */ (_MDSPAN_TRAIT( std::is_constructible, container_type, container_type, Alloc) &&
                    _MDSPAN_TRAIT( std::is_constructible, mapping_type, extents_type))
  )
  MDSPAN_INLINE_FUNCTION
  constexpr mdarray(const container_type& ctr, const extents_type& exts, const Alloc& a)
    : map_(exts), ctr_(ctr, a)
  { assert(ctr_.size() >= static_cast<size_t>(map_.required_span_size())); }

  MDSPAN_TEMPLATE_REQUIRES(
    class Alloc,
    /* requires */ (_MDSPAN_TRAIT( std::is_constructible, container_type, size_t, Alloc))
  )
  MDSPAN_INLINE_FUNCTION
  constexpr mdarray(const container_type& ctr, const mapping_type& map, const Alloc& a)
    : map_(map), ctr_(ctr, a)
  { assert(ctr_.size() >= static_cast<size_t>(map_.required_span_size())); }

  MDSPAN_TEMPLATE_REQUIRES(
    class Alloc,
    /* requires */ (_MDSPAN_TRAIT( std::is_constructible, container_type, container_type, Alloc) &&
                    _MDSPAN_TRAIT( std::is_constructible, mapping_type, extents_type))
  )
  MDSPAN_INLINE_FUNCTION
  constexpr mdarray(container_type&& ctr, const extents_type& exts, const Alloc& a)
    : map_(exts), ctr_(std::move(ctr), a)
  { assert(ctr_.size() >= static_cast<size_t>(map_.required_span_size())); }

  MDSPAN_TEMPLATE_REQUIRES(
    class Alloc,
    /* requires */ (_MDSPAN_TRAIT( std::is_constructible, container_type, size_t, Alloc))
  )
  MDSPAN_INLINE_FUNCTION
  constexpr mdarray(container_type&& ctr, const mapping_type& map, const Alloc& a)
    : map_(map), ctr_(std::move(ctr), a)
  { assert(ctr_.size() >= map_.required_span_size()); }

  MDSPAN_TEMPLATE_REQUIRES(
    class OtherElementType, class OtherExtents, class OtherLayoutPolicy, class OtherContainer, class Alloc,
    /* requires */ (
      _MDSPAN_TRAIT( std::is_constructible, mapping_type, typename OtherLayoutPolicy::template mapping<OtherExtents>) &&
      _MDSPAN_TRAIT( std::is_constructible, container_type, OtherContainer, Alloc)
    )
  )
  MDSPAN_INLINE_FUNCTION
  constexpr mdarray(const mdarray<OtherElementType, OtherExtents, OtherLayoutPolicy, OtherContainer>& other, const Alloc& a)
    : map_(other.mapping()), ctr_(other.container(), a)
  {
    static_assert( std::is_constructible<extents_type, OtherExtents>::value, "");
  }

  MDSPAN_INLINE_FUNCTION_DEFAULTED constexpr mdarray& operator= (const mdarray&) = default;
  MDSPAN_INLINE_FUNCTION_DEFAULTED constexpr mdarray& operator= (mdarray&&) = default;
  MDSPAN_INLINE_FUNCTION_DEFAULTED
  ~mdarray() = default;

  //--------------------------------------------------------------------------------
  // [mdspan.basic.mapping], mdspan mapping domain multidimensional index to access codomain element

  #if MDSPAN_USE_BRACKET_OPERATOR
  MDSPAN_TEMPLATE_REQUIRES(
    class... SizeTypes,
    /* requires */ (
      _MDSPAN_FOLD_AND(_MDSPAN_TRAIT( std::is_convertible, SizeTypes, index_type) /* && ... */) &&
      extents_type::rank() == sizeof...(SizeTypes)
    )
  )
  MDSPAN_FORCE_INLINE_FUNCTION
  constexpr const_reference operator[](SizeTypes... indices) const noexcept
  {
    return ctr_[map_(static_cast<index_type>(std::move(indices))...)];
  }

  MDSPAN_TEMPLATE_REQUIRES(
    class... SizeTypes,
    /* requires */ (
      _MDSPAN_FOLD_AND(_MDSPAN_TRAIT( std::is_convertible, SizeTypes, index_type) /* && ... */) &&
      extents_type::rank() == sizeof...(SizeTypes)
    )
  )
  MDSPAN_FORCE_INLINE_FUNCTION
  constexpr reference operator[](SizeTypes... indices) noexcept
  {
    return ctr_[map_(static_cast<index_type>(std::move(indices))...)];
  }
  #endif

#if 0
  MDSPAN_TEMPLATE_REQUIRES(
    class SizeType, size_t N,
    /* requires */ (
      _MDSPAN_TRAIT( std::is_convertible, SizeType, index_type) &&
      N == extents_type::rank()
    )
  )
  MDSPAN_FORCE_INLINE_FUNCTION
  constexpr const_reference operator[](const std::array<SizeType, N>& indices) const noexcept
  {
    return __impl::template __callop<reference>(*this, indices);
  }

  MDSPAN_TEMPLATE_REQUIRES(
    class SizeType, size_t N,
    /* requires */ (
      _MDSPAN_TRAIT( std::is_convertible, SizeType, index_type) &&
      N == extents_type::rank()
    )
  )
  MDSPAN_FORCE_INLINE_FUNCTION
  constexpr reference operator[](const std::array<SizeType, N>& indices) noexcept
  {
    return __impl::template __callop<reference>(*this, indices);
  }
#endif


  #if MDSPAN_USE_PAREN_OPERATOR
  MDSPAN_TEMPLATE_REQUIRES(
    class... SizeTypes,
    /* requires */ (
      _MDSPAN_FOLD_AND(_MDSPAN_TRAIT( std::is_convertible, SizeTypes, index_type) /* && ... */) &&
      extents_type::rank() == sizeof...(SizeTypes)
    )
  )
  MDSPAN_FORCE_INLINE_FUNCTION
  constexpr const_reference operator()(SizeTypes... indices) const noexcept
  {
    return ctr_[map_(static_cast<index_type>(std::move(indices))...)];
  }
  MDSPAN_TEMPLATE_REQUIRES(
    class... SizeTypes,
    /* requires */ (
      _MDSPAN_FOLD_AND(_MDSPAN_TRAIT( std::is_convertible, SizeTypes, index_type) /* && ... */) &&
      extents_type::rank() == sizeof...(SizeTypes)
    )
  )
  MDSPAN_FORCE_INLINE_FUNCTION
  constexpr reference operator()(SizeTypes... indices) noexcept
  {
    return ctr_[map_(static_cast<index_type>(std::move(indices))...)];
  }

#if 0
  MDSPAN_TEMPLATE_REQUIRES(
    class SizeType, size_t N,
    /* requires */ (
      _MDSPAN_TRAIT( std::is_convertible, SizeType, index_type) &&
      N == extents_type::rank()
    )
  )
  MDSPAN_FORCE_INLINE_FUNCTION
  constexpr const_reference operator()(const std::array<SizeType, N>& indices) const noexcept
  {
    return __impl::template __callop<reference>(*this, indices);
  }

  MDSPAN_TEMPLATE_REQUIRES(
    class SizeType, size_t N,
    /* requires */ (
      _MDSPAN_TRAIT( std::is_convertible, SizeType, index_type) &&
      N == extents_type::rank()
    )
  )
  MDSPAN_FORCE_INLINE_FUNCTION
  constexpr reference operator()(const std::array<SizeType, N>& indices) noexcept
  {
    return __impl::template __callop<reference>(*this, indices);
  }
#endif
  #endif

  MDSPAN_INLINE_FUNCTION constexpr pointer data() noexcept { return ctr_.data(); };
  MDSPAN_INLINE_FUNCTION constexpr const_pointer data() const noexcept { return ctr_.data(); };
  MDSPAN_INLINE_FUNCTION constexpr container_type& container() noexcept { return ctr_; };
  MDSPAN_INLINE_FUNCTION constexpr const container_type& container() const noexcept { return ctr_; };

  //--------------------------------------------------------------------------------
  // [mdspan.basic.domobs], mdspan observers of the domain multidimensional index space

  MDSPAN_INLINE_FUNCTION static constexpr rank_type rank() noexcept { return extents_type::rank(); }
  MDSPAN_INLINE_FUNCTION static constexpr rank_type rank_dynamic() noexcept { return extents_type::rank_dynamic(); }
  MDSPAN_INLINE_FUNCTION static constexpr size_t static_extent(size_t r) noexcept { return extents_type::static_extent(r); }

  MDSPAN_INLINE_FUNCTION constexpr const extents_type& extents() const noexcept { return map_.extents(); };
  MDSPAN_INLINE_FUNCTION constexpr index_type extent(size_t r) const noexcept { return map_.extents().extent(r); };
  MDSPAN_INLINE_FUNCTION constexpr index_type size() const noexcept {
//    return __impl::__size(*this);
    return ctr_.size();
  };


  //--------------------------------------------------------------------------------
  // [mdspan.basic.obs], mdspan observers of the mapping

  MDSPAN_INLINE_FUNCTION static constexpr bool is_always_unique() noexcept { return mapping_type::is_always_unique(); };
  MDSPAN_INLINE_FUNCTION static constexpr bool is_always_exhaustive() noexcept { return mapping_type::is_always_exhaustive(); };
  MDSPAN_INLINE_FUNCTION static constexpr bool is_always_strided() noexcept { return mapping_type::is_always_strided(); };

  MDSPAN_INLINE_FUNCTION constexpr const mapping_type& mapping() const noexcept { return map_; };
  MDSPAN_INLINE_FUNCTION constexpr bool is_unique() const noexcept { return map_.is_unique(); };
  MDSPAN_INLINE_FUNCTION constexpr bool is_exhaustive() const noexcept { return map_.is_exhaustive(); };
  MDSPAN_INLINE_FUNCTION constexpr bool is_strided() const noexcept { return map_.is_strided(); };
  MDSPAN_INLINE_FUNCTION constexpr index_type stride(size_t r) const { return map_.stride(r); };

  // Converstion to mdspan
  MDSPAN_TEMPLATE_REQUIRES(
    class OtherElementType, class OtherExtents,
    class OtherLayoutType, class OtherAccessorType,
    /* requires */ (
      _MDSPAN_TRAIT(std::is_assignable, mdspan_type,
                       mdspan<OtherElementType, OtherExtents, OtherLayoutType, OtherAccessorType>)
    )
  )
  constexpr operator mdspan<OtherElementType, OtherExtents, OtherLayoutType, OtherAccessorType> () {
    return mdspan_type(data(), map_);
  }

  MDSPAN_TEMPLATE_REQUIRES(
    class OtherElementType, class OtherExtents,
    class OtherLayoutType, class OtherAccessorType,
    /* requires */ (
      _MDSPAN_TRAIT(std::is_assignable, const_mdspan_type,
                      mdspan<OtherElementType, OtherExtents, OtherLayoutType, OtherAccessorType>)
    )
  )
  constexpr operator mdspan<OtherElementType, OtherExtents, OtherLayoutType, OtherAccessorType> () const {
    return const_mdspan_type(data(), map_);
  }

  MDSPAN_TEMPLATE_REQUIRES(
    class OtherAccessorType = default_accessor<element_type>,
    /* requires */ (
      _MDSPAN_TRAIT(std::is_assignable, mdspan_type,
                      mdspan<element_type, extents_type, layout_type, OtherAccessorType>)
    )
  )
  constexpr mdspan<element_type, extents_type, layout_type, OtherAccessorType>
    to_mdspan(const OtherAccessorType& a = default_accessor<element_type>()) {
      return mdspan<element_type, extents_type, layout_type, OtherAccessorType>(data(), map_, a);
  }

  MDSPAN_TEMPLATE_REQUIRES(
    class OtherAccessorType = default_accessor<const element_type>,
    /* requires */ (
      _MDSPAN_TRAIT(std::is_assignable, const_mdspan_type,
                      mdspan<const element_type, extents_type, layout_type, OtherAccessorType>)
    )
  )
  constexpr mdspan<const element_type, extents_type, layout_type, OtherAccessorType>
    to_mdspan(const OtherAccessorType& a = default_accessor<const element_type>()) const {
      return mdspan<const element_type, extents_type, layout_type, OtherAccessorType>(data(), map_, a);
  }

private:
  mapping_type map_;
  container_type ctr_;

  template <class, class, class, class>
  friend class mdarray;
};


} // end namespace MDSPAN_IMPL_PROPOSED_NAMESPACE
} // end namespace MDSPAN_IMPL_STANDARD_NAMESPACE
