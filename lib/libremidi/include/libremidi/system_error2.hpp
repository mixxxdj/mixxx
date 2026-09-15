#pragma once
#ifndef STDX_COMPILER_HPP
  #define STDX_COMPILER_HPP

  // Check compiler macros.  Note that Clang defines __GNUC__ and other GNU macros as well,
  // but GNU does not define Clang macros, so we must check for Clang first.

  #if defined(__llvm__) || defined(__clang__)

  // -------- LLVM/Clang

    #define STDX_CLANG_COMPILER 1

    #if defined(__cpp_variable_templates) && (__cplusplus >= 201703L)
      #define STDX_VARIABLE_TEMPLATES 1
    #endif

  #elif defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)

  // -------- GNU G++

    #include <version>

    #define STDX_GCC_COMPILER 1

    #if (__GNUC__ >= 5) && (__cplusplus >= 201703L)
      #define STDX_VARIABLE_TEMPLATES 1
    #endif

    #if ((__GNUC__ > 7) || ((__GNUC__ == 7) && (__GNUC_MINOR__ >= 1)))
      #define STDX_TRIVIALLY_MOVE_CONSTRUCTIBLE 1
    #endif

  #endif

  #if defined(STDX_GCC_COMPILER)
    #if (__GNUC__ == 7) && ((__GNUC_MINOR__ >= 1) && (__GNUC_MINOR__ <= 3))
      #define STDX_GCC7_WORKAROUND_CONSTEXPR
    #else
      #define STDX_GCC7_WORKAROUND_CONSTEXPR constexpr
    #endif
  #else
    #define STDX_GCC7_WORKAROUND_CONSTEXPR constexpr
  #endif

  // Add a legacy constexpr macro for cases where GCC < 5 incorrectly applies the const
  // qualifier to constexpr member functions or does not support relaxed constexpr functions
  //
  #if defined(STDX_GCC_COMPILER) && (__GNUC__ < 5)
    #define STDX_LEGACY_CONSTEXPR
  #else
    #define STDX_LEGACY_CONSTEXPR constexpr
  #endif

  #if defined(_MSC_VER) && (_MSC_VER >= 1910)
    #define STDX_MSVC_EMPTY_BASE_CLASSES __declspec(empty_bases)
  #else
    #define STDX_MSVC_EMPTY_BASE_CLASSES
  #endif

  #if defined(__cpp_impl_trivially_relocatable)
    #define STDX_TRIVIALLY_RELOCATABLE [[trivially_relocatable]]
  #else
    #define STDX_TRIVIALLY_RELOCATABLE
  #endif

#if defined(__clang__) && defined(__has_warning)
  #pragma clang diagnostic push
  #if __has_warning("-Wdeprecated-declarations")
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
  #endif
#endif
#endif // STDX_COMPILER_HPP


#ifndef STDX_TYPE_TRAITS_HPP
  #define STDX_TYPE_TRAITS_HPP

  #include <type_traits>

NAMESPACE_STDX {

// Implementation of std::void_t for use with pre-C++17 compilers.
//
namespace detail {

template <class... T>
struct void_t_impl
{
  using type = void;
};

} // end namespace detail

template <class... T>
using void_t = typename detail::void_t_impl<T...>::type;

template <class T, class U = T>
struct dependent_type
{
  using type = U;
};

template <class T, class U = T>
using dependent_type_t = typename dependent_type<T, U>::type;

template <class... B>
struct disjunction : std::false_type
{ };

template <class B1>
struct disjunction<B1> : B1
{ };

template<class B1, class... Bn>
struct disjunction<B1, Bn...>
    :
      std::conditional_t<
          bool(B1::value),
          B1,
          disjunction<Bn...>
          >
{ };

struct sentinel_type
{
  static constexpr bool value = true;
  using type = void;
};

template <bool B>
struct bool_constant : std::integral_constant<bool, B>
{ };

// Implementation of std::remove_cvref for use with pre-C++20 compilers.
//
template <class T>
struct remove_cvref
{
  using type = std::remove_cv_t<std::remove_reference_t<T>>;
};

template <class T>
using remove_cvref_t = typename remove_cvref<T>::type;

  #if defined(STDX_GCC_COMPILER)
    // Support missing functionality on older compilers (<= gcc 4.7)
    //
    #if (__GNUC__ == 4) && (__GNUC_MINOR__ <= 7)
    // Map the old incorrect type-trait names to the newer correct ones
template <class T>
using is_trivially_copyable = std::is_trivial<T>;

template <class T>
using is_trivially_copy_constructible = std::has_trivial_copy_constructor<T>;

template <class T>
using is_trivially_destructible = std::has_trivial_destructor<T>;
    #elif (__GNUC__ < 5)
template <class T>
using is_trivially_copyable = std::is_trivial<T>;

template <class T>
using is_trivially_copy_constructible = std::has_trivial_copy_constructor<T>;

template <class T>
using is_trivially_destructible = std::is_trivially_destructible<T>;
    #else
using std::is_trivially_destructible;
using std::is_trivially_copyable;
using std::is_trivially_copy_constructible;
    #endif
  #else
using std::is_trivially_destructible;
using std::is_trivially_copyable;
using std::is_trivially_copy_constructible;
  #endif

  #if defined(STDX_TRIVIALLY_MOVE_CONSTRUCTIBLE)
template <class T>
using is_trivially_move_constructible = std::is_trivially_move_constructible<T>;
  #else
template <class T>
using is_trivially_move_constructible = is_trivially_copyable<T>;
  #endif

  #if defined(__cpp_lib_trivially_relocatable)
using std::is_trivially_relocatable;
  #elif defined(__has_builtin)
    #if __has_builtin(__builtin_is_cpp_trivially_relocatable)
template <class T>
struct is_trivially_relocatable : std::bool_constant<__builtin_is_cpp_trivially_relocatable(T)> { };
      #define STDX_MUST_SPECIALIZE_IS_TRIVIALLY_RELOCATABLE
    #elif __has_builtin(__is_trivially_relocatable)
template <class T>
struct is_trivially_relocatable : std::bool_constant<__is_trivially_relocatable(T)> { };
      #define STDX_MUST_SPECIALIZE_IS_TRIVIALLY_RELOCATABLE
    #else
template <class T>
struct is_trivially_relocatable : is_trivially_copyable<T> { };
      #define STDX_MUST_SPECIALIZE_IS_TRIVIALLY_RELOCATABLE
    #endif
  #else
template <class T>
struct is_trivially_relocatable : is_trivially_copyable<T> { };
    #define STDX_MUST_SPECIALIZE_IS_TRIVIALLY_RELOCATABLE
  #endif

  #if __cplusplus >= 201703L
    #define STDX_LEGACY_INLINE_CONSTEXPR inline constexpr
  #else
    #define STDX_LEGACY_INLINE_CONSTEXPR constexpr
  #endif

} // end namespace stdx

#endif



#ifndef STDX_BIT_CAST_HPP
  #define STDX_BIT_CAST_HPP

  #include <cstdint>
  #include <cstring>


NAMESPACE_STDX {

namespace detail {

template <class To, class From>
using use_static_cast = bool_constant<
    ((std::is_integral<To>::value || std::is_enum<To>::value)
     && (std::is_integral<From>::value || std::is_enum<From>::value))
    || (std::is_same<To, From>::value && std::is_copy_constructible<To>::value)
    >;

template <class T>
using is_integral_ptr_t = bool_constant<
    std::is_same<T, std::intptr_t>::value
    || std::is_same<T, std::uintptr_t>::value
    >;

template <class To, class From>
using use_reinterpret_cast = bool_constant<
    !std::is_same<To, From>::value
    && ((
            std::is_pointer<To>::value
            && std::is_pointer<From>::value
            && std::is_convertible<From, To>::value
            )
        || (std::is_pointer<To>::value && is_integral_ptr_t<From>::value)
        || (std::is_pointer<From>::value && is_integral_ptr_t<To>::value)
        )
    >;

  #if defined(STDX_GCC_COMPILER)
template <class To, class From>
using use_union_type_punning = bool_constant<
    !use_static_cast<To, From>::value
    && !use_reinterpret_cast<To, From>::value
    && !std::is_array<To>::value
    && !std::is_array<From>::value
    >;

template <class To, class From>
union bit_cast_union
{
  From from;
  To to;
};
  #else
template <class To, class From>
using use_union_type_punning = std::false_type;
  #endif

template <class To, class From>
using can_bit_cast = bool_constant<
    (sizeof(To) == sizeof(From))
    && is_trivially_copyable<From>::value
    && is_trivially_copyable<To>::value
    >;

} // end namespace detail

template <
    class To,
    class From,
    class = std::enable_if_t<
        detail::can_bit_cast<To, From>::value
        && detail::use_static_cast<To, From>::value
        >
    >
constexpr To bit_cast(const From& from) noexcept
{
  return static_cast<To>(from);
}

template <
    class To,
    class From,
    class = std::enable_if_t<
        detail::can_bit_cast<To, From>::value
        && detail::use_reinterpret_cast<To, From>::value
        >,
    int = 0
    >
constexpr To bit_cast(const From& from) noexcept
{
  return reinterpret_cast<To>(from);
}

  #if defined(STDX_GCC_COMPILER) // GCC allows union type punning
template <
    class To,
    class From,
    class = std::enable_if_t<
        detail::can_bit_cast<To, From>::value
        && detail::use_union_type_punning<To, From>::value
        >,
    class = void
    >
constexpr To bit_cast(const From& from) noexcept
{
  return detail::bit_cast_union<To, From>{from}.to;
}
  #elif defined(STDX_CLANG_COMPILER)
    #if __has_builtin(__builtin_bit_cast)
template <
    class To,
    class From,
    class = std::enable_if_t<
        detail::can_bit_cast<To, From>::value
        && !detail::use_static_cast<To, From>::value
        && !detail::use_reinterpret_cast<To, From>::value
        >,
    class = void
    >
constexpr To bit_cast(const From& from) noexcept
{
  return __builtin_bit_cast(To, from);
}
    #else
template <
    class To,
    class From,
    class = std::enable_if_t<
        detail::can_bit_cast<To, From>::value
        && !detail::use_static_cast<To, From>::value
        && !detail::use_reinterpret_cast<To, From>::value
        >,
    class = void
    >
To bit_cast(const From& from) noexcept
{
  To to;
  std::memcpy(&to, &from, sizeof(To));
  return to;
}
    #endif
  #endif // STDX_CLANG_COMPILER

template <class To, class From>
struct is_bit_castable
    :
      bool_constant<
          (sizeof(To) == sizeof(From))
          && is_trivially_copyable<From>::value
          && is_trivially_copyable<To>::value
          >
{ };

} // end namespace stdx

#endif



#ifndef STDX_INTRUSIVE_POINTER_HPP
  #define STDX_INTRUSIVE_POINTER_HPP


  #include <cstdint>
  #include <atomic>
  #include <memory>

NAMESPACE_STDX {

struct default_intrusive_reference_count;
class default_intrusive_reference_control;

template <
    class T,
    class RefCountAccessor = default_intrusive_reference_count,
    class Deleter = std::default_delete<T>,
    class Pointer = T*
    >
class intrusive_ptr;

using ref_count_t = std::size_t;

struct enable_reference_count
{
protected:

  constexpr enable_reference_count() noexcept : m_reference_count(1)
  { }

public:

  std::atomic<ref_count_t>& shared_reference_count() noexcept
  {
    return m_reference_count;
  }

private:

  std::atomic<ref_count_t> m_reference_count;
};

struct default_intrusive_reference_count
{
  template <class Pointer>
  std::atomic<ref_count_t>& operator()(Pointer p) const noexcept
  {
    return p->shared_reference_count();
  }
};

namespace detail {

template <class Pointer>
struct pointer_wrapper
{
  constexpr pointer_wrapper() noexcept : shared_object(nullptr)
  { }

  constexpr explicit pointer_wrapper(Pointer p) noexcept : shared_object(p)
  { }

  constexpr pointer_wrapper(const pointer_wrapper& p) noexcept = default;

  pointer_wrapper(pointer_wrapper&& p) noexcept
      : shared_object(p.shared_object)
  {
    p.shared_object = nullptr;
  }

  pointer_wrapper& operator = (const pointer_wrapper&) noexcept = default;

  pointer_wrapper& operator = (pointer_wrapper&& p) noexcept
  {
    shared_object = p.shared_object;
    p.shared_object = nullptr;
    return *this;
  }

  void assign(Pointer ptr) noexcept
  {
    shared_object = ptr;
  }

  Pointer shared_object;
};

template <
    class T,
    class RefCountAccessor,
    class Deleter,
    class Pointer,
    class PointerImplementation
    >
class intrusive_ptr_base
{
protected:

  using pointer = Pointer;
  using count_type = ref_count_t;

  template <class, class, class, class>
  friend class reference_count_base;

  constexpr intrusive_ptr_base() = default;

  constexpr intrusive_ptr_base(pointer p) noexcept
      : m_impl(p)
  { }

  template <class RefCountAccessorForwardingReference>
  constexpr explicit intrusive_ptr_base(
      RefCountAccessorForwardingReference&& f,
      std::enable_if_t<
          std::is_constructible<
              RefCountAccessor,
              RefCountAccessorForwardingReference&&
              >::value
          >* = nullptr
      )
      : m_impl(std::forward<RefCountAccessorForwardingReference>(f))
  { }

  template <class RefCountAccessorForwardingReference>
  explicit intrusive_ptr_base(
      pointer ptr,
      RefCountAccessorForwardingReference&& f,
      std::enable_if_t<
          std::is_constructible<
              RefCountAccessor,
              RefCountAccessorForwardingReference&&
              >::value
          >* = nullptr
      )
      : m_impl(ptr, std::forward<RefCountAccessorForwardingReference>(f))
  { }

  template <
      class RefCountAccessorForwardingReference,
      class DeleterForwardingReference
      >
  constexpr explicit intrusive_ptr_base(
      RefCountAccessorForwardingReference&& f,
      DeleterForwardingReference&& d,
      typename std::enable_if<
          std::is_constructible<
              RefCountAccessor,
              RefCountAccessorForwardingReference&&
              >::value
          && std::is_constructible<
              Deleter,
              DeleterForwardingReference&&
              >::value
          >::type* = nullptr
      )
      :
      m_impl(
          std::forward<RefCountAccessorForwardingReference>(f),
          std::forward<DeleterForwardingReference>(d)
          )
  { }

  template <
      class RefCountAccessorForwardingReference,
      class DeleterForwardingReference
      >
  explicit intrusive_ptr_base(
      pointer ptr,
      RefCountAccessorForwardingReference&& f,
      DeleterForwardingReference&& d,
      std::enable_if_t<
          std::is_constructible<
              RefCountAccessor,
              RefCountAccessorForwardingReference&&
              >::value
          && std::is_constructible<
              Deleter,
              DeleterForwardingReference&&
              >::value
          >* = nullptr
      )
      :
      m_impl(
          ptr,
          std::forward<RefCountAccessorForwardingReference>(f),
          std::forward<DeleterForwardingReference>(d)
          )
  { }

  void assign(pointer ptr) noexcept
  {
    m_impl.assign(ptr);
  }

  template <
      class RefCountAccessorForwardingReference,
      class DeleterForwardingReference
      >
  void assign(
      pointer ptr,
      RefCountAccessorForwardingReference&& f,
      DeleterForwardingReference&& d
      )
  {
    m_impl.assign(
        ptr,
        std::forward<RefCountAccessorForwardingReference>(f),
        std::forward<DeleterForwardingReference>(d)
        );
  }

  void swap(intrusive_ptr_base& other)
  {
    m_impl.swap(other.m_impl);
  }

  struct STDX_MSVC_EMPTY_BASE_CLASSES impl
      :
        PointerImplementation,
        RefCountAccessor,
        Deleter
  {
    constexpr impl() = default;

    explicit impl(pointer ptr) noexcept : PointerImplementation(ptr)
    { }

    template <
        class RefCountAccess,
        class = std::enable_if_t<
            std::is_constructible<RefCountAccessor, RefCountAccess&&>::value
            >
        >
    constexpr explicit impl(RefCountAccess&& f)
        : RefCountAccessor(std::forward<RefCountAccess>(f))
    { }

    template <
        class RefCountAccess,
        class = std::enable_if_t<
            std::is_constructible<RefCountAccessor, RefCountAccess&&>::value
            >
        >
    impl(pointer ptr, RefCountAccess&& f)
        :
        PointerImplementation(ptr),
        RefCountAccessor(std::forward<RefCountAccessor>(f))
    { }

    template <
        class RefCountAccess,
        class D,
        class = std::enable_if_t<
            std::is_constructible<RefCountAccessor, RefCountAccess&&>::value
            && std::is_constructible<Deleter, D&&>::value
            >
        >
    constexpr impl(RefCountAccess&& f, D&& d)
        :
        RefCountAccessor(std::forward<RefCountAccess>(f)),
        Deleter(std::forward<D>(d))
    { }

    template <
        class RefCountAccess,
        class D,
        class = std::enable_if_t<
            std::is_constructible<RefCountAccessor, RefCountAccess&&>::value
            && std::is_constructible<Deleter, D&&>::value
            >
        >
    impl(pointer ptr, RefCountAccess&& f, D&& d)
        :
        PointerImplementation(ptr),
        RefCountAccessor(std::forward<RefCountAccess>(f)),
        Deleter(std::forward<D>(d))
    { }

    impl(const impl&) = default;
    impl& operator = (const impl&) = default;
    impl(impl&&) = default;
    impl& operator = (impl&&) = default;

    Deleter& get_deleter() noexcept
    {
      return static_cast<Deleter&>(*this);
    }

    const Deleter& get_deleter() const noexcept
    {
      return static_cast<const Deleter&>(*this);
    }

    void assign(pointer ptr) noexcept
    {
      static_cast<PointerImplementation&>(*this).assign(ptr);
    }

    void assign(std::nullptr_t) noexcept
    {
      static_cast<PointerImplementation&>(*this).assign(nullptr);
    }

    void swap(impl& other)
    {
      std::swap(
          static_cast<PointerImplementation&>(*this),
          static_cast<PointerImplementation&>(other)
          );

      std::swap(static_cast<RefCountAccessor&>(*this), static_cast<RefCountAccessor&>(other));
      std::swap(
          static_cast<Deleter&>(this->get_deleter()),
          static_cast<Deleter&>(other.get_deleter())
          );
    }
  };

  impl m_impl;

  void increment_shared_reference_count(
      std::memory_order order = std::memory_order_relaxed
      ) const noexcept
  {
    if (ptr()) ref_count_func()(ptr()).fetch_add(1, order);
  }

  void decrement_shared_reference_count() noexcept
  {
    if (ptr())
    {
      if (ref_count_func()(ptr()).fetch_sub(1, std::memory_order_release) == 1)
      {
        std::atomic_thread_fence(std::memory_order_acquire);
        invoke_deleter(ptr());
      }
    }
  }

  // ----- accessors and modifiers

  pointer& ptr() noexcept
  {
    return static_cast<PointerImplementation&>(m_impl).shared_object;
  }

  constexpr pointer ptr() const noexcept
  {
    return static_cast<const PointerImplementation&>(m_impl).shared_object;
  }

  RefCountAccessor& ref_count_func() noexcept
  {
    return static_cast<RefCountAccessor&>(m_impl);
  }

  const RefCountAccessor& ref_count_func() const noexcept
  {
    return static_cast<const RefCountAccessor&>(m_impl);
  }

  Deleter& deleter() noexcept
  {
    return m_impl.get_deleter();
  }

  const Deleter& deleter() const noexcept
  {
    return m_impl.get_deleter();
  }

  intrusive_ptr<
      T,
      RefCountAccessor,
      Deleter,
      Pointer
      > make_intrusive_pointer(pointer p) const noexcept
  {
    return intrusive_ptr<T, RefCountAccessor, Deleter, Pointer>{
        p,
        ref_count_func(),
        deleter()
    };
  }

private:

  void invoke_deleter(pointer p)
  {
    m_impl.get_deleter()(p);
  }

  void invoke_deleter(pointer p) const
  {
    m_impl.get_deleter()(p);
  }

  template <class WeakReferenceCountDescriptor>
  void invoke_deleter(pointer p, WeakReferenceCountDescriptor* d)
  {
    maybe_delete_shared_object(p, m_impl.get_deleter(), d);
  }

  template <class WeakReferenceCountDescriptor>
  void invoke_deleter(pointer p, WeakReferenceCountDescriptor* d) const
  {
    maybe_delete_shared_object(p, m_impl.get_deleter(), d);
  }
};

} // end namespace detail

template <
    class T,
    class RefCountAccessor,
    class Deleter,
    class Pointer
    >
class STDX_TRIVIALLY_RELOCATABLE intrusive_ptr
    :
      public detail::intrusive_ptr_base<
          T,
          RefCountAccessor,
          Deleter,
          Pointer,
          detail::pointer_wrapper<Pointer>
          >
{
  using base_type = detail::intrusive_ptr_base<
      T,
      RefCountAccessor,
      Deleter,
      Pointer,
      detail::pointer_wrapper<Pointer>
      >;

public:

  using pointer = Pointer;
  using element_type = T;
  using ref_count_accessor = RefCountAccessor;
  using deleter_type = Deleter;
  using count_type = typename base_type::count_type;

  constexpr intrusive_ptr() noexcept : base_type()
  { }

  constexpr intrusive_ptr(std::nullptr_t) noexcept : base_type()
  { }

  template <class RefCountAccess>
  constexpr intrusive_ptr(std::nullptr_t, RefCountAccess&& f)
      : base_type(std::forward<RefCountAccess>(f))
  { }

  template <class RefCountAccess, class D>
  constexpr intrusive_ptr(std::nullptr_t, RefCountAccess&& f,	D&& d)
      : base_type(std::forward<RefCountAccess>(f), std::forward<D>(d))
  { }

  constexpr explicit intrusive_ptr(Pointer ptr) noexcept
      : base_type(ptr)
  {
    // reference count must initially be >= 1
  }

  template <class RefCountAccess>
  intrusive_ptr(Pointer ptr, RefCountAccess&& f) noexcept
      : base_type(ptr, std::forward<RefCountAccess>(f))
  {
    // reference count must initially be >= 1
  }

  template <class RefCountAccess, class D>
  intrusive_ptr(Pointer ptr, RefCountAccess&& f, D&& d) noexcept
      :
      base_type(
          ptr,
          std::forward<RefCountAccess>(f),
          std::forward<D>(d)
          )
  {
    // reference count must initially be >= 1
  }

  // Copy constructor
  //
  intrusive_ptr(const intrusive_ptr& rhs) noexcept
      : base_type(rhs)
  {
    this->increment_shared_reference_count();
  }

  // Converting copy-constructor
  //
  template <
      class Y,
      class Ptr,
      class = std::enable_if_t<std::is_convertible<Ptr, pointer>::value>
      >
  intrusive_ptr(const intrusive_ptr<Y, RefCountAccessor, Deleter, Ptr>& rhs) noexcept
      :
      base_type(
          rhs.get(),
          rhs.ref_count_func(),
          rhs.get_deleter()
          )
  {
    this->increment_shared_reference_count();
  }

  // Move constructor
  //
  intrusive_ptr(intrusive_ptr&& rhs) noexcept
      : base_type(std::move(rhs))
  { }

  // Copy assignment
  //
  intrusive_ptr& operator = (const intrusive_ptr& rhs) noexcept
  {
    rhs.increment_shared_reference_count();
    this->decrement_shared_reference_count();

    static_cast<base_type&>(*this) = static_cast<const base_type&>(rhs);
    return *this;
  }

  // Move assignment
  //
  intrusive_ptr& operator = (intrusive_ptr&& rhs) noexcept
  {
    if (this != std::addressof(rhs))
    {
      this->decrement_shared_reference_count();
      static_cast<base_type&>(*this) = std::move(static_cast<base_type&>(rhs));
    }

    return *this;
  }

  ~intrusive_ptr() noexcept
  {
    this->decrement_shared_reference_count();
  }

  void reset() noexcept
  {
    this->decrement_shared_reference_count();
    this->assign(nullptr);
  }

  void reset(std::nullptr_t) noexcept
  {
    reset();
  }

  void reset(Pointer ptr) noexcept
  {
    if (this->ptr() != ptr)
    {
      this->decrement_shared_reference_count();
      this->assign(ptr);
      this->increment_shared_reference_count();
    }
  }

  void swap(intrusive_ptr& other) noexcept
  {
    if (this->get() != other.get())
    {
      base_type::swap(other);
    }
  }

  pointer get() const noexcept
  {
    return this->ptr();
  }

  element_type& operator * () const noexcept
  {
    return *this->get();
  }

  pointer operator -> () const noexcept
  {
    return this->get();
  }

  count_type use_count() const noexcept
  {
    return this->ref_count_func()(get()).load(std::memory_order_acquire);
  }

  explicit operator bool() const noexcept
  {
    return static_cast<bool>(this->get());
  }

  Deleter get_deleter() noexcept
  {
    return this->deleter();
  }

  const Deleter& get_deleter() const noexcept
  {
    return this->deleter();
  }

  RefCountAccessor ref_count_access() noexcept
  {
    return this->ref_count_func();
  }

  const RefCountAccessor& ref_count_access() const noexcept
  {
    return this->ref_count_func();
  }

private:

  template <class Y, class G, class D, class P>
  friend class intrusive_ptr;
};

// -------------- Global equality operators
//
template <class T, class G1, class D1, class P1, class U, class G2, class D2, class P2>
bool operator == (
    const intrusive_ptr<T, G1, D1, P1>& lhs,
    const intrusive_ptr<U, G2, D2, P2>& rhs
    ) noexcept
{
  return lhs.get() == rhs.get();
}

template <class T, class G1, class D1, class P1, class U, class G2, class D2, class P2>
bool operator != (
    const intrusive_ptr<T, G1, D1, P1>& lhs,
    const intrusive_ptr<U, G2, D2, P2>& rhs
    ) noexcept
{
  return !(lhs == rhs);
}

template <class T, class G1, class D1, class P1, class U, class G2, class D2, class P2>
bool operator < (
    const intrusive_ptr<T, G1, D1, P1>& lhs,
    const intrusive_ptr<U, G2, D2, P2>& rhs
    ) noexcept
{
  using pointer1 = typename intrusive_ptr<T, G1, D1, P1>::pointer;
  using pointer2 = typename intrusive_ptr<U, G2, D2, P2>::pointer;
  using common_type = typename std::common_type<pointer1, pointer2>::type;

  return std::less<common_type>{}(lhs.get(), rhs.get());
}

template <class T, class G1, class D1, class P1, class U, class G2, class D2, class P2>
bool operator > (
    const intrusive_ptr<T, G1, D1, P1>& lhs,
    const intrusive_ptr<U, G2, D2, P2>& rhs
    ) noexcept
{
  return rhs < lhs;
}

template <class T, class G1, class D1, class P1, class U, class G2, class D2, class P2>
bool operator <= (
    const intrusive_ptr<T, G1, D1, P1>& lhs,
    const intrusive_ptr<U, G2, D2, P2>& rhs
    ) noexcept
{
  return !(rhs < lhs);
}

template <class T, class G1, class D1, class P1, class U, class G2, class D2, class P2>
bool operator >= (
    const intrusive_ptr<T, G1, D1, P1>& lhs,
    const intrusive_ptr<U, G2, D2, P2>& rhs
    ) noexcept
{
  return !(lhs < rhs);
}

template <class T, class G1, class D1, class P1>
bool operator == (const intrusive_ptr<T, G1, D1, P1>& lhs, std::nullptr_t) noexcept
{
  return !lhs;
}

template <class T, class G1, class D1, class P1>
bool operator == (std::nullptr_t, const intrusive_ptr<T, G1, D1, P1>& rhs) noexcept
{
  return !rhs;
}

template <class T, class G1, class D1, class P1>
bool operator != (const intrusive_ptr<T, G1, D1, P1>& lhs, std::nullptr_t) noexcept
{
  return bool(lhs);
}

template <class T, class G1, class D1, class P1>
bool operator != (std::nullptr_t, const intrusive_ptr<T, G1, D1, P1>& rhs) noexcept
{
  return bool(rhs);
}

template <class T, class G1, class D1, class P1>
bool operator < (const intrusive_ptr<T, G1, D1, P1>& lhs, std::nullptr_t) noexcept
{
  using pointer = typename intrusive_ptr<T, G1, D1, P1>::pointer;
  return std::less<pointer>{}(lhs.get(), nullptr);
}

template <class T, class G1, class D1, class P1>
bool operator < (std::nullptr_t, const intrusive_ptr<T, G1, D1, P1>& rhs) noexcept
{
  using pointer = typename intrusive_ptr<T, G1, D1, P1>::pointer;
  return std::less<pointer>{}(nullptr, rhs.get());
}

template <class T, class G1, class D1, class P1>
bool operator > (const intrusive_ptr<T, G1, D1, P1>& lhs, std::nullptr_t) noexcept
{
  return (nullptr < lhs);
}

template <class T, class G1, class D1, class P1>
bool operator > (std::nullptr_t lhs, const intrusive_ptr<T, G1, D1, P1>& rhs) noexcept
{
  return (rhs < nullptr);
}

template <class T, class G1, class D1, class P1>
bool operator <= (const intrusive_ptr<T, G1, D1, P1>& lhs, std::nullptr_t rhs) noexcept
{
  return !(nullptr < lhs);
}

template <class T, class G1, class D1, class P1>
bool operator <= (std::nullptr_t lhs, const intrusive_ptr<T, G1, D1, P1>& rhs) noexcept
{
  return !(rhs < nullptr);
}

template <class T, class G1, class D1, class P1>
bool operator >= (const intrusive_ptr<T, G1, D1, P1>& lhs, std::nullptr_t rhs) noexcept
{
  return !(lhs < nullptr);
}

template <class T, class G1, class D1, class P1>
bool operator >= (std::nullptr_t lhs, const intrusive_ptr<T, G1, D1, P1>& rhs) noexcept
{
  return !(nullptr < rhs);
}

template <class T, class G, class D, class P>
void swap(intrusive_ptr<T, G, D, P>& lhs, intrusive_ptr<T, G, D, P>& rhs) noexcept
{
  lhs.swap(rhs);
}

  #ifdef STDX_MUST_SPECIALIZE_IS_TRIVIALLY_RELOCATABLE
template <class Y, class G, class D, class P>
struct is_trivially_relocatable<intrusive_ptr<Y,G,D,P>> : std::true_type
{ };
  #endif

} // end namespace stdx

#endif // include guard



#ifndef STDX_STRING_REF_HPP
  #define STDX_STRING_REF_HPP

  #include <cstring>
  #include <cstddef>
  #include <atomic>

NAMESPACE_STDX {

class string_ref;

namespace detail {

constexpr const char* cstring_null_scan(const char* s) noexcept
{
  return *s ? cstring_null_scan(s + 1) : s;
}

} // end namespace detail

class string_ref
{
protected:

  struct state_type;

public:

  using value_type = const char;
  using size_type = std::size_t;
  using pointer = const char*;
  using const_pointer = const char*;
  using iterator = const char*;
  using const_iterator = const char*;

  struct resource_management
  {
    using copy_constructor = state_type(*)(const string_ref&);
    using move_constructor = state_type(*)(string_ref&&);
    using destructor = void(*)(string_ref&);

    constexpr resource_management() noexcept
        : copy{nullptr}, move{nullptr}, destroy{nullptr}
    { }

    constexpr resource_management(
        copy_constructor cctor,
        move_constructor mctor,
        destructor dtor
        ) noexcept
        : copy{cctor}, move{mctor}, destroy{dtor}
    { }

    copy_constructor copy;
    move_constructor move;
    destructor destroy;
  };

  constexpr string_ref() noexcept : m_begin(nullptr), m_end(nullptr), context{}
  { }

  constexpr string_ref(const char* beg) noexcept
      : m_begin(beg), m_end(detail::cstring_null_scan(beg)), context{}
  { }

  constexpr string_ref(const char* beg, const char* e) noexcept
      : m_begin(beg), m_end(e), context{}
  { }

  constexpr string_ref(const char* beg, resource_management rm) noexcept
      :
      m_begin(beg),
      m_end(detail::cstring_null_scan(beg)),
      m_resource_management(rm),
      context{}
  { }

  constexpr string_ref(const char* beg, const char* e, resource_management rm) noexcept
      : m_begin(beg), m_end(e), m_resource_management(rm), context{}
  { }

  constexpr string_ref(
      const char* beg,
      const char* e,
      resource_management rm,
      void* ctx
      ) noexcept
      : m_begin(beg), m_end(e), m_resource_management(rm), context{ctx}
  { }

  STDX_GCC7_WORKAROUND_CONSTEXPR string_ref(const string_ref& s)
      : string_ref{s.m_resource_management.copy ? s.m_resource_management.copy(s) : s.state()}
  { }

  STDX_GCC7_WORKAROUND_CONSTEXPR string_ref(string_ref&& s)
      :
      string_ref{
          s.m_resource_management.move ?
              s.m_resource_management.move(std::move(s)) : s.state()
      }
  { }

  string_ref& operator = (const string_ref& s)
  {
    string_ref tmp = s;
    *this = std::move(tmp);
    return *this;
  }

  string_ref& operator = (string_ref&& s)
  {
    if (this != &s)
    {
      if (m_resource_management.destroy) m_resource_management.destroy(*this);

      // This is legal because of the common initial sequence and the fact
      // that any type erased object must be trivially relocatable.
      *this = string_ref_state_union{std::move(s)}.state;
    }

    return *this;
  }

  ~string_ref() noexcept
  {
    if (m_resource_management.destroy) m_resource_management.destroy(*this);
  }

  bool empty() const noexcept { return m_begin == m_end; }

  size_type size() const noexcept { return m_end - m_begin; }

  const_pointer data() const noexcept { return m_begin; }

  iterator begin() noexcept { return m_begin; }

  iterator end() noexcept { return m_end; }

  const_iterator begin() const noexcept { return m_begin; }

  const_iterator end() const noexcept { return m_end; }

  const_iterator cbegin() const noexcept { return m_begin; }

  const_iterator cend() const noexcept { return m_end; }

protected:

  struct state_type
  {
    pointer m_begin;
    pointer m_end;
    resource_management m_resource_management;
    void* context;
  };

  state_type state() const noexcept
  {
    return state_type{m_begin, m_end, m_resource_management, context};
  }

  constexpr explicit string_ref(const state_type& s) noexcept
      :
      m_begin(s.m_begin),
      m_end(s.m_end),
      m_resource_management(s.m_resource_management),
      context(s.context)
  { }

  void clear() noexcept
  {
    m_begin = nullptr;
    m_end = nullptr;
  }

  template <class StringRef>
  union string_ref_state_union_type
  {
    explicit string_ref_state_union_type(StringRef&& s) : str(std::move(s))
    { }

    ~string_ref_state_union_type() noexcept {}

    StringRef str;
    state_type state;
  };

  using string_ref_state_union = string_ref_state_union_type<string_ref>;

  void operator = (const state_type& s) noexcept
  {
    m_begin = s.m_begin;
    m_end = s.m_end;
    m_resource_management = s.m_resource_management;
    context = s.context;
  }

  pointer m_begin;
  pointer m_end;
  resource_management m_resource_management;
  void* context;
};

inline bool operator == (const string_ref& lhs, const string_ref& rhs) noexcept
{
  return (lhs.size() == rhs.size()) && (std::memcmp(lhs.data(), rhs.data(), lhs.size()) == 0);
}

inline bool operator != (const string_ref& lhs, const string_ref& rhs) noexcept
{
  return !(lhs == rhs);
}

inline bool operator < (const string_ref& lhs, const string_ref& rhs) noexcept
{
  const std::size_t sz = (lhs.size() < rhs.size()) ? lhs.size() : rhs.size();
  int result = std::memcmp(lhs.data(), rhs.data(), sz);
  if (result == 0) return lhs.size() < rhs.size();
  return result < 0;
}

inline bool operator > (const string_ref& lhs, const string_ref& rhs) noexcept
{
  return rhs < lhs;
}

inline bool operator <= (const string_ref& lhs, const string_ref& rhs) noexcept
{
  return !(lhs > rhs);
}

inline bool operator >= (const string_ref& lhs, const string_ref& rhs) noexcept
{
  return !(lhs < rhs);
}

// Reference-counted allocated string
//
class shared_string_ref : public string_ref
{
  struct string_arena_base
  {
    mutable std::atomic<std::size_t> ref_count;
    std::size_t length;
  };

  struct string_arena : string_arena_base
  {
    constexpr explicit string_arena(std::size_t length) noexcept
        : string_arena_base{{1}, length}
    { }

    constexpr static std::size_t header_size() noexcept
    {
      return sizeof(string_arena);
    }

    char* data() noexcept
    {
      return reinterpret_cast<char*>(this) + header_size();
    }

    const char* data() const noexcept
    {
      return reinterpret_cast<const char*>(this) + header_size();
    }

    const char* begin() const noexcept { return data(); }

    const char* end() const noexcept { return data() + length; }
  };

  string_ref allocate_string_ref(const char* s, std::size_t length)
  {
    const std::size_t arena_size = string_arena::header_size() + length;
    char* buf = static_cast<char*>(::operator new(arena_size));
    string_arena* a = new (buf) string_arena{length};
    std::memcpy(a->data(), s, length);
    return shared_string_ref{a};
  }

  explicit shared_string_ref(string_arena* a) noexcept
      :
      string_ref{
          a->begin(),
          a->end(),
          string_ref::resource_management{&copy_construct, &move_construct, &destroy},
          a
      }
  { }

  const string_arena_base* get_arena() const noexcept
  {
    return static_cast<string_arena_base*>(this->context);
  }

  string_arena_base* get_arena() noexcept
  {
    return static_cast<string_arena_base*>(this->context);
  }

  static string_ref::state_type copy_construct(const string_ref& base) noexcept
  {
    const shared_string_ref& s = static_cast<const shared_string_ref&>(base);
    const string_arena_base* a = s.get_arena();
    if (a) a->ref_count.fetch_add(1, std::memory_order_relaxed);
    return s.state();
  }

  static string_ref::state_type move_construct(string_ref&& base) noexcept
  {
    shared_string_ref& s = static_cast<shared_string_ref&>(base);
    auto st = s.state();
    s.context = nullptr;
    s.clear();
    return st;
  }

  static void destroy(string_ref& base) noexcept
  {
    shared_string_ref& s = static_cast<shared_string_ref&>(base);
    string_arena* a = static_cast<string_arena*>(s.get_arena());
    if (a && (a->ref_count.fetch_sub(1, std::memory_order_release) == 1))
    {
      std::atomic_thread_fence(std::memory_order_acquire);
      ::operator delete(a);
    }
  }

  template <class Allocator>
  struct allocated_string_arena : string_arena_base
  {
    constexpr allocated_string_arena(const Allocator& alloc, std::size_t length) noexcept
        : string_arena_base{{1}, length}, allocator(alloc)
    { }

    constexpr static std::size_t header_size() noexcept
    {
      return sizeof(allocated_string_arena);
    }

    char* data() noexcept
    {
      return reinterpret_cast<char*>(this) + header_size();
    }

    const char* data() const noexcept
    {
      return reinterpret_cast<const char*>(this) + header_size();
    }

    const char* begin() const noexcept { return data(); }

    const char* end() const noexcept { return data() + length; }

    std::size_t allocated_size() const noexcept { return header_size() + length; }

    Allocator allocator;
  };

  template <class Allocator>
  explicit shared_string_ref(allocated_string_arena<Allocator>* a) noexcept
      :
      string_ref{
          a->begin(),
          a->end(),
          string_ref::resource_management{
              &copy_construct,
              &move_construct,
              &allocator_destroy<Allocator>
          },
          a
      }
  { }

  template <class Allocator>
  string_ref allocate_string_ref(
      const Allocator& allocator,
      const char* s,
      std::size_t length
      )
  {
    using allocator_type = typename std::allocator_traits<
        Allocator
        >::template rebind_alloc<char>;
    using arena_type = allocated_string_arena<allocator_type>;

    allocator_type alloc{allocator};
    const std::size_t arena_size = arena_type::header_size() + length;
    char* buf = alloc.allocate(arena_size);
    arena_type* a = new (buf) arena_type{alloc, length};
    std::memcpy(a->data(), s, length);
    return shared_string_ref{a};
  }

  template <class Allocator>
  static void allocator_destroy(string_ref& base) noexcept
  {
    using arena_type = allocated_string_arena<Allocator>;

    shared_string_ref& s = static_cast<shared_string_ref&>(base);
    arena_type* a = static_cast<arena_type*>(s.get_arena());
    if (a && (a->ref_count.fetch_sub(1, std::memory_order_release) == 1))
    {
      std::atomic_thread_fence(std::memory_order_acquire);

      Allocator alloc = std::move(a->allocator);
      const std::size_t allocated_size = a->allocated_size();
      a->~arena_type();
      alloc.deallocate(reinterpret_cast<char*>(a), allocated_size);
    }
  }

public:

  shared_string_ref(const char* beg)
      : string_ref{allocate_string_ref(beg, detail::cstring_null_scan(beg) - beg)}
  { }

  shared_string_ref(const char* beg, const char* end)
      : string_ref{allocate_string_ref(beg, end - beg)}
  { }

  template <class Allocator>
  shared_string_ref(const Allocator& alloc, const char* beg)
      : string_ref{allocate_string_ref(alloc, beg, detail::cstring_null_scan(beg) - beg)}
  { }

  template <class Allocator>
  shared_string_ref(const Allocator& alloc, const char* beg, const char* end)
      : string_ref{allocate_string_ref(alloc, beg, end - beg)}
  { }

  std::size_t use_count() const noexcept
  {
    const string_arena_base* a = get_arena();
    return a ? a->ref_count.load(std::memory_order_acquire) : 0;
  }
};

} // end namespace stdx

#endif



#ifndef STDX_LAUNDER_HPP
  #define STDX_LAUNDER_HPP

  #include <new>


NAMESPACE_STDX {

  #if __cplusplus >= 201703L
    #if defined(__cpp_lib_launder)
      #define STDX_HAVE_NATIVE_LAUNDER 1
using std::launder;
    #elif defined(STDX_CLANG_COMPILER)
      #if __has_builtin(__builtin_launder)
        #define STDX_HAVE_NATIVE_LAUNDER 1
template <class T>
constexpr T* launder(T* p) noexcept
{
  return __builtin_launder(p);
}
      #endif
    #endif
  #endif

  #if !defined(STDX_HAVE_NATIVE_LAUNDER)
template <class T>
constexpr T* launder(T* p) noexcept
{
  return p;
}
  #endif

} // end namespace stdx

#endif



#ifndef STDX_ERROR_HPP
  #define STDX_ERROR_HPP

  #include <exception>
  #include <stdexcept>
  #include <system_error>
  #include <memory>
  #include <cassert>


NAMESPACE_STDX {

class error;

namespace detail {

template <class... Args>
struct error_ctor_args;

} // end namespace detail

} // end namespace stdx

namespace stdx_adl {

namespace detail {

template <class Args, class Enable = void>
struct can_use_adl_to_call_make_error : std::false_type
{ };

inline void make_error() noexcept { }

template <class... Args>
struct can_use_adl_to_call_make_error<
    stdx::detail::error_ctor_args<Args...>,
    std::enable_if_t<
        std::is_same<
            decltype(make_error(std::declval<Args>()...)),
            stdx::error
            >::value
        >
    >
    : std::true_type
{ };

template <class... Args>
constexpr auto construct_error_from_adl(Args&&... args) noexcept(
    noexcept(make_error(std::forward<Args>(args)...))
    )
{
  return make_error(std::forward<Args>(args)...);
}
}

} // end namespace stdx_adl

NAMESPACE_STDX {

enum class dynamic_exception_errc
{
  runtime_error = 1,
  domain_error,
  invalid_argument,
  length_error,
  out_of_range,
  logic_error,
  range_error,
  overflow_error,
  underflow_error,
  bad_alloc,
  bad_array_new_length,
  bad_optional_access,
  bad_typeid,
  bad_any_cast,
  bad_cast,
  bad_weak_ptr,
  bad_function_call,
  bad_exception,
  bad_variant_access,
  unspecified_exception
};

std::error_code error_code_from_exception(std::exception_ptr eptr) noexcept;

// -------------------- error_traits
//
template <class E>
struct error_traits
{ };

namespace detail {

template <class E, class Enable = void>
struct is_convertible_from_exception_using_traits : std::false_type
{ };

template <class E>
struct is_convertible_from_exception_using_traits<
    E,
    std::enable_if_t<
        std::is_convertible<
            decltype(error_traits<E>::from_exception(std::declval<std::exception_ptr>())),
            E
            >::value
        >
    >
    : std::true_type
{ };

template <class E, class Enable = void>
struct is_convertible_to_exception_using_traits : std::false_type
{ };

template <class E>
struct is_convertible_to_exception_using_traits<
    E,
    std::enable_if_t<
        std::is_convertible<
            decltype(error_traits<E>::to_exception(std::declval<E>())),
            std::exception_ptr
            >::value
        >
    >
    : std::true_type
{ };

template <class E>
E from_exception_impl(std::exception_ptr e, std::is_convertible<std::exception_ptr, E>) noexcept
{
  return e;
}

template <class E>
E from_exception_impl(
    std::exception_ptr e,
    is_convertible_from_exception_using_traits<E>
    ) noexcept
{
  return error_traits<E>::from_exception(std::move(e));
}

void from_exception_impl(std::exception_ptr, sentinel_type) = delete;

template <class E, class G>
std::exception_ptr to_exception_impl(E&& e, std::is_convertible<G, std::exception_ptr>) noexcept
{
  return std::forward<E>(e);
}

template <class E, class G>
std::exception_ptr to_exception_impl(
    E&& e,
    is_convertible_to_exception_using_traits<G>
    ) noexcept
{
  return error_traits<E>::to_exception(std::forward<E>(e));
}

template <class E>
void to_exception_impl(const E&, sentinel_type) = delete;

} // end namespace detail

template <class E>
E from_exception(std::exception_ptr e) noexcept
{
  return detail::from_exception_impl<E>(
      std::move(e),
      disjunction<
          std::is_convertible<std::exception_ptr, E>,
          detail::is_convertible_from_exception_using_traits<E>,
          sentinel_type
          >{}
      );
}

template <class E>
std::exception_ptr to_exception(E&& e) noexcept
{
  return detail::to_exception_impl(
      std::forward<E>(e),
      disjunction<
          std::is_convertible<std::decay_t<E>, std::exception_ptr>,
          detail::is_convertible_to_exception_using_traits<std::decay_t<E>>,
          sentinel_type
          >{}
      );
}

struct error_domain_id
{
  constexpr error_domain_id(std::uint64_t l, std::uint64_t h) noexcept
      : lo(l), hi(h)
  { }

private:

  friend constexpr bool operator == (const error_domain_id&, const error_domain_id&) noexcept;

  std::uint64_t lo;
  std::uint64_t hi;
};

constexpr bool operator == (const error_domain_id& lhs, const error_domain_id& rhs) noexcept
{
  return (lhs.lo == rhs.lo) && (lhs.hi == rhs.hi);
}

constexpr bool operator != (const error_domain_id& lhs, const error_domain_id& rhs) noexcept
{
  return !(lhs == rhs);
}

template <class T = void>
struct error_value;

namespace detail {

template <class T>
struct is_error_value : std::false_type
{ };

template <class T>
struct is_error_value<error_value<T>> : std::true_type
{ };

} // end namespace detail

struct error_resource_management
{
  using copy_constructor = error_value<void>(*)(const error&);
  using move_constructor = error_value<void>(*)(error&&);
  using destructor = void(*)(error&);

  constexpr error_resource_management() noexcept
      : copy{nullptr}, move{nullptr}, destroy{nullptr}
  { }

  constexpr error_resource_management(
      copy_constructor cctor,
      move_constructor mctor,
      destructor dtor
      ) noexcept
      : copy(cctor), move(mctor), destroy(dtor)
  { }

  copy_constructor copy;
  move_constructor move;
  destructor destroy;
};

class error_domain
{
public:

  virtual string_ref name() const noexcept = 0;
  virtual bool equivalent(const error& lhs, const error& rhs) const noexcept = 0;
  virtual string_ref message(const error&) const noexcept = 0;

  virtual void throw_exception(const error& e) const;

  friend class error;
  friend constexpr bool operator == (const error_domain&, const error_domain&) noexcept;
  friend constexpr bool operator != (const error_domain&, const error_domain&) noexcept;

  constexpr error_domain_id id() const noexcept
  {
    return m_id;
  }

protected:

  constexpr explicit error_domain(error_domain_id id) noexcept
      :
      m_id{id},
      m_resource_management{}
  { }

  constexpr error_domain(error_domain_id id, error_resource_management erm) noexcept
      :
      m_id{id},
      m_resource_management{erm}
  { }

  error_domain(const error_domain &) = default;
  error_domain(error_domain &&) = default;
  error_domain &operator = (const error_domain &) = default;
  error_domain &operator = (error_domain&&) = default;
  ~error_domain() = default;

  template <
      class E = error,
      class = std::enable_if_t<
          std::is_convertible<const E&, error>::value
          >
      >
  constexpr dependent_type_t<E, error_value<>> copy(const E& e) const
  {
    return m_resource_management.copy ? m_resource_management.copy(e) : error_value<>{e.m_value};
  }

  template <
      class E = error,
      class = std::enable_if_t<
          std::is_rvalue_reference<E&&>::value
          && std::is_convertible<E&&, error>::value
          >
      >
  constexpr dependent_type_t<remove_cvref_t<E>, error_value<>> move(E&& e) const
  {
    return m_resource_management.move ?
               m_resource_management.move(static_cast<E&&>(e)) : error_value<>{e.m_value};
  }

  void destroy(error& e) const noexcept
  {
    if (m_resource_management.destroy) m_resource_management.destroy(e);
  }

private:

  error_domain_id m_id;
  error_resource_management m_resource_management;
};

constexpr bool operator == (const error_domain& lhs, const error_domain& rhs) noexcept
{
  return lhs.id() == rhs.id();
}

constexpr bool operator != (const error_domain& lhs, const error_domain& rhs) noexcept
{
  return lhs.id() != rhs.id();
}

namespace detail {

template <class ErasedType, class T>
struct error_type_is_erasable
    :
      bool_constant<
          is_trivially_relocatable<T>::value
          && (sizeof(T) <= sizeof(ErasedType))
          && (alignof(T) <= alignof(ErasedType))
          >
{ };

template <class ErasedType>
struct error_type_is_erasable<ErasedType, void> : std::false_type
{ };

template <class ErasedType>
struct error_type_is_erasable<ErasedType, const void> : std::false_type
{ };

template <class T>
using can_use_static_cast = bool_constant<
    std::is_integral<T>::value
    || std::is_enum<T>::value
    >;

struct erased_error
{
  using integral_type = std::intptr_t;
  using storage_type = std::aligned_storage_t<sizeof(integral_type), alignof(integral_type)>;

  constexpr erased_error() noexcept : code{}
  { }

  template <
      class T,
      class = std::enable_if_t<
          error_type_is_erasable<integral_type, T>::value
          && can_use_static_cast<T>::value
          >
      >
  constexpr erased_error(T value) noexcept
      : code{static_cast<integral_type>(value)}
  { }

  template <
      class T,
      class = std::enable_if_t<
          error_type_is_erasable<integral_type, T>::value
          && !can_use_static_cast<T>::value
          && is_bit_castable<T, integral_type>::value
          >,
      class = void
      >
  constexpr erased_error(T value) noexcept
      : code{bit_cast<integral_type>(value)}
  { }

  template <
      class T,
      class = std::enable_if_t<
          error_type_is_erasable<integral_type, T>::value
          && !can_use_static_cast<T>::value
          && !is_bit_castable<T, integral_type>::value
          >,
      int = 0
      >
  erased_error(T value) noexcept(std::is_nothrow_move_constructible<T>::value)
  {
    new (&storage) T(std::move(value));
  }

  union
  {
    integral_type code;
    storage_type storage;
  };
};

template <
    class T,
    class = std::enable_if_t<
        error_type_is_erasable<erased_error::integral_type, T>::value
        && can_use_static_cast<T>::value
        >
    >
constexpr T error_cast_impl(erased_error e) noexcept
{
  return static_cast<T>(e.code);
}

template <
    class T,
    class = std::enable_if_t<
        error_type_is_erasable<erased_error::integral_type, T>::value
        && !can_use_static_cast<T>::value
        && is_bit_castable<erased_error::integral_type, T>::value
        >,
    class = void
    >
constexpr T error_cast_impl(erased_error e) noexcept
{
  return bit_cast<T>(e.code);
}

template <
    class T,
    class = std::enable_if_t<
        error_type_is_erasable<erased_error::integral_type, T>::value
        && !can_use_static_cast<T>::value
        && !is_bit_castable<erased_error::integral_type, T>::value
        >
    >
constexpr T error_cast_impl(erased_error&& e) noexcept(
    std::is_nothrow_move_constructible<T>::value
    )
{
  return std::move(*stdx::launder(reinterpret_cast<T*>(&e.storage)));
}

template <
    class T,
    class = std::enable_if_t<
        error_type_is_erasable<erased_error::integral_type, T>::value
        && !can_use_static_cast<T>::value
        && !is_bit_castable<erased_error::integral_type, T>::value
        >
    >
constexpr T error_cast_impl(const erased_error& e) noexcept(
    std::is_nothrow_copy_constructible<T>::value
    )
{
  return *stdx::launder(reinterpret_cast<const T*>(&e.storage));
}

} // end namespace detail

template <class T>
struct error_value
{
  using value_type = T;

  constexpr error_value(const T& v) noexcept(
      std::is_nothrow_copy_constructible<T>::value
      )
      : m_value(v)
  { }

  constexpr error_value(T&& v) noexcept(
      std::is_nothrow_move_constructible<T>::value
      )
      : m_value(std::move(v))
  { }

  constexpr const T& value() const & noexcept
  {
    return m_value;
  }

  STDX_LEGACY_CONSTEXPR T& value() & noexcept
  {
    return m_value;
  }

  constexpr const T&& value() const && noexcept
  {
    return static_cast<const T&&>(m_value);
  }

  STDX_LEGACY_CONSTEXPR T&& value() && noexcept
  {
    return static_cast<T&&>(m_value);
  }

  T m_value;
};

template <>
struct error_value<void>
{
  template <
      class T,
      class = std::enable_if_t<
          !std::is_same<remove_cvref_t<T>, error_value>::value
          && !detail::is_error_value<remove_cvref_t<T>>::value
          && std::is_constructible<detail::erased_error, T&&>::value
          >
      >
  constexpr error_value(T&& v) noexcept(
      std::is_nothrow_constructible<detail::erased_error, T&&>::value
      )
      : m_value(std::forward<T>(v))
  { }

  template <
      class T,
      class = std::enable_if_t<
          std::is_constructible<detail::erased_error, const T&>::value
          >
      >
  constexpr error_value(const error_value<T>& v) noexcept(
      std::is_nothrow_constructible<detail::erased_error, const T&>::value
      )
      : error_value(v.value())
  { }

  template <
      class T,
      class = std::enable_if_t<
          std::is_constructible<detail::erased_error, T&&>::value
          >
      >
  constexpr error_value(error_value<T>&& v) noexcept(
      std::is_nothrow_constructible<detail::erased_error, T&&>::value
      )
      : error_value(std::move(v.value()))
  { }

  friend class error;

private:

  detail::erased_error m_value;
};

class error;

namespace detail {

struct error_copy_construct_t {};
struct error_move_construct_t {};

template <class... Args>
struct error_ctor_args {};

template <class Args, class DecayedArgs>
struct can_construct_error_from_adl
{ };

template <class... Args, class... DecayedArgs>
struct can_construct_error_from_adl<error_ctor_args<Args...>, error_ctor_args<DecayedArgs...>>
    : stdx_adl::detail::can_use_adl_to_call_make_error<error_ctor_args<Args...>>
{ };

template <>
struct can_construct_error_from_adl<error_ctor_args<>, error_ctor_args<>> : std::false_type
{ };

template <class Error>
struct can_construct_error_from_adl<error_ctor_args<Error>, error_ctor_args<error>>
    : std::false_type
{ };

template <class E, class D, class T, class ErrorDomain>
struct can_construct_error_from_adl<
    error_ctor_args<E, D>,
    error_ctor_args<error_value<T>, ErrorDomain>
    >
    : std::false_type
{ };

template <class CC, class V, class ED, class ErrorDomain>
struct can_construct_error_from_adl<
    error_ctor_args<CC, V, ED>,
    error_ctor_args<error_copy_construct_t, error_value<>, ErrorDomain>
    >
    : std::false_type
{ };

template <class MC, class V, class ED, class ErrorDomain>
struct can_construct_error_from_adl<
    error_ctor_args<MC, V, ED>,
    error_ctor_args<error_move_construct_t, error_value<>, ErrorDomain>
    >
    : std::false_type
{ };

template <class E, class Enable = void>
struct is_convertible_to_error_using_traits : std::false_type
{ };

template <>
struct is_convertible_to_error_using_traits<error_ctor_args<error>>
    : std::false_type
{ };

template <class E>
struct is_convertible_to_error_using_traits<
    error_ctor_args<E>,
    std::enable_if_t<
        std::is_same<
            decltype(error_traits<E>::to_error(std::declval<E>())),
            error
            >::value
        >
    >
    : std::true_type
{ };

template <class E, class... Args>
constexpr auto construct_error_impl(
    is_convertible_to_error_using_traits<error_ctor_args<E>>,
    Args&&... args
    ) noexcept(noexcept(error_traits<E>::to_error(std::declval<Args&&>()...)))
{
  return error_traits<E>::to_error(std::forward<Args>(args)...);
}

template <class A1, class A2, class... Args>
constexpr auto construct_error_impl(
    can_construct_error_from_adl<A1, A2>,
    Args&&... args
    ) noexcept(noexcept(stdx_adl::detail::construct_error_from_adl(std::declval<Args&&>()...)))
{
  return stdx_adl::detail::construct_error_from_adl(std::forward<Args>(args)...);
}

struct cannot_construct_error
{
  static constexpr bool value = false;
  using type = void;
};

template <class... Args>
void construct_error_impl(cannot_construct_error, Args&&...) = delete;

template <class Args, class DecayedArgs>
using construct_error_disjunction_impl_t = disjunction<
    is_convertible_to_error_using_traits<DecayedArgs>,
    can_construct_error_from_adl<Args, DecayedArgs>,
    cannot_construct_error
    >;

template <class... Args>
using construct_error_disjunction_t = construct_error_disjunction_impl_t<
    error_ctor_args<Args...>,
    error_ctor_args<remove_cvref_t<Args>...>
    >;

struct error_move_access;
struct error_ref_access;
struct error_cref_access;

} // end namespace detail

// Generic domain for std::errc codes
//
class generic_error_domain : public error_domain
{
public:

  constexpr generic_error_domain() noexcept
      : error_domain{{0x574ce0d940b64a2bULL, 0xa7c4438dd858c9cfULL}}
  { }

  virtual string_ref name() const noexcept override
  {
    return "generic domain";
  }

  virtual bool equivalent(const error& lhs, const error& rhs) const noexcept override;

  virtual string_ref message(const error&) const noexcept override;
};

STDX_LEGACY_INLINE_CONSTEXPR generic_error_domain generic_domain {};

class STDX_TRIVIALLY_RELOCATABLE error
{
  using erased_type = detail::erased_error;

  constexpr error(detail::error_copy_construct_t, error_value<> v, const error_domain* d) noexcept
      : m_domain(d), m_value(v.m_value)
  { }

  constexpr error(detail::error_move_construct_t, error_value<> v, const error_domain* d) noexcept
      : m_domain(d), m_value(std::move(v.m_value))
  { }

public:

  constexpr error() noexcept : m_domain(&generic_domain), m_value{}
  { }

  constexpr error(const error& e)
      : error(detail::error_copy_construct_t{}, e.m_domain->copy(e), e.m_domain)
  { }

  constexpr error(error&& e)
      : error(detail::error_move_construct_t{}, e.m_domain->move(std::move(e)), e.m_domain)
  { }

  template <
      class T,
      class = std::enable_if_t<
          detail::error_type_is_erasable<erased_type, T>::value
          >
      >
  constexpr error(const error_value<T>& v, const error_domain& d) noexcept
      : m_domain(&d), m_value(v.value())
  { }

  template <
      class T,
      class = std::enable_if_t<
          detail::error_type_is_erasable<erased_type, T>::value
          >
      >
  constexpr error(error_value<T>&& v, const error_domain& d) noexcept
      : m_domain(&d), m_value(static_cast<T&&>(v.value()))
  { }

  constexpr error(error_value<> v, const error_domain& d) noexcept
      : m_domain(&d), m_value(v.m_value)
  { }

  template <
      class A,
      class... Args,
      class = std::enable_if_t<
          detail::construct_error_disjunction_t<A&&, Args&&...>::value
          >
      >
  constexpr error(A&& a, Args&&... args) noexcept(
      noexcept(
          detail::construct_error_impl(
              std::declval<detail::construct_error_disjunction_t<A&&, Args&&...>>(),
              std::forward<A>(a),
              std::forward<Args>(args)...
              )
          )
      )
      :
      error(
          detail::construct_error_impl(
              detail::construct_error_disjunction_t<A&&, Args&&...>{},
              std::forward<A>(a),
              std::forward<Args>(args)...
              )
          )
  { }

  error& operator = (const error& e)
  {
    error_value<> v = e.domain().copy(e);
    domain().destroy(*this);
    m_domain = e.m_domain;
    m_value = v.m_value;
    return *this;
  }

  error& operator = (error&& e) noexcept
  {
    if (this != &e)
    {
      error_value<> v = e.domain().move(std::move(e));
      domain().destroy(*this);
      m_domain = e.m_domain;
      m_value = v.m_value;
    }

    return *this;
  }

  ~error() noexcept
  {
    m_domain->destroy(*this);
  }

  bool is_set() const noexcept { return (*this != stdx::error{}); }

  const error_domain& domain() const noexcept
  {
    return *m_domain;
  }

  string_ref message() const noexcept
  {
    return domain().message(*this);
  }

  [[noreturn]] void throw_exception() const
  {
    domain().throw_exception(*this);
    abort();
  }

  friend class error_domain;
  friend struct detail::error_move_access;
  friend struct detail::error_ref_access;
  friend struct detail::error_cref_access;
  friend inline bool operator==(const error& lhs, const error& rhs) noexcept;
  friend inline bool operator!=(const error& lhs, const error& rhs) noexcept;

private:

  const error_domain* m_domain;
  erased_type m_value;
};

inline bool operator == (const error& lhs, const error& rhs) noexcept
{
  if (lhs.domain().equivalent(lhs, rhs)) return true;
  if (rhs.domain().equivalent(rhs, lhs)) return true;
  return false;
}

inline bool operator != (const error& lhs, const error& rhs) noexcept
{
  return !(lhs == rhs);
}

namespace detail {

struct error_move_access
{
  constexpr explicit error_move_access(error&& e) noexcept : m_value(&e.m_value)
  { }

  STDX_LEGACY_CONSTEXPR detail::erased_error&& rvalue_ref() noexcept
  {
    return std::move(*m_value);
  }

  detail::erased_error* m_value;
};

struct error_ref_access
{
  constexpr explicit error_ref_access(error& e) noexcept : m_ptr(&e.m_value)
  { }

  detail::erased_error& ref() noexcept { return *m_ptr; }

  detail::erased_error* m_ptr;
};

struct error_cref_access
{
  constexpr explicit error_cref_access(const error& e) noexcept : m_ptr(&e.m_value)
  { }

  constexpr const detail::erased_error& ref() const noexcept { return *m_ptr; }

  const detail::erased_error* m_ptr;
};

} // end namespace detail

template <
    class T,
    class = void_t<
        decltype(detail::error_cast_impl<T>(std::declval<const detail::erased_error&>()))
        >
    >
constexpr T error_cast(const error& e) noexcept(
    noexcept(detail::error_cast_impl<T>(std::declval<const detail::erased_error&>()))
    )
{
  return detail::error_cast_impl<T>(detail::error_cref_access{e}.ref());
}

template <
    class T,
    class = void_t<decltype(detail::error_cast_impl<T>(std::declval<detail::erased_error>()))>
    >
constexpr T error_cast(error&& e) noexcept(
    noexcept(detail::error_cast_impl<T>(std::declval<detail::erased_error>()))
    )
{
  return detail::error_cast_impl<T>(detail::error_move_access{std::move(e)}.rvalue_ref());
}

namespace detail {

struct default_error_constructors
{
  template <class T>
  static error_value<> copy_constructor(const error& e) noexcept(
      std::is_nothrow_copy_constructible<T>::value
      && std::is_nothrow_move_constructible<T>::value
      )
  {
    T value = error_cast<T>(e);
    return error_value<>{std::move(value)};
  }

  template <class T>
  static error_value<> move_constructor(error&& e) noexcept(
      std::is_nothrow_move_constructible<T>::value
      )
  {
    return error_value<>{error_cast<T>(std::move(e))};
  }

  template <class T>
  static void destructor(error& e) noexcept
  {
    detail::erased_error& value = error_ref_access{e}.ref();
    stdx::launder(reinterpret_cast<T*>(&value.storage))->~T();
  }

  template <class T>
  constexpr static error_resource_management::copy_constructor copy() noexcept
  {
    return is_trivially_copy_constructible<T>::value ?
               nullptr : &copy_constructor<T>;
  }

  template <class T>
  constexpr static error_resource_management::move_constructor move() noexcept
  {
    return is_trivially_move_constructible<T>::value ?
               nullptr : &move_constructor<T>;
  }

  template <class T>
  constexpr static error_resource_management::destructor destroy() noexcept
  {
    return is_trivially_destructible<T>::value ?
               nullptr : &destructor<T>;
  }
};

} // end namespace detail

template <class T>
struct default_error_resource_management_t : error_resource_management
{
  constexpr default_error_resource_management_t() noexcept
      :
      error_resource_management{
          detail::default_error_constructors::copy<T>(),
          detail::default_error_constructors::move<T>(),
          detail::default_error_constructors::destroy<T>()
      }
  {}
};

  #if defined(STDX_VARIABLE_TEMPLATES)
template <class T>
inline constexpr default_error_resource_management_t<T> default_error_resource_management {};
  #endif

template <>
struct error_traits<std::errc>
{
  static std::exception_ptr to_exception(std::errc ec) noexcept
  {
    return std::make_exception_ptr(std::make_error_code(ec));
  }

  static error to_error(std::errc ec) noexcept
  {
    return error{error_value<std::errc>{ec}, generic_domain};
  }
};

namespace detail {

struct error_code_wrapper : enable_reference_count
{
  explicit error_code_wrapper(std::error_code ec) noexcept : code(ec)
  { }

  std::error_code code;
};

} // end namespace detail

// Error domain mapping to std::error_code
//
class error_code_error_domain : public error_domain
{
  using internal_value_type = intrusive_ptr<detail::error_code_wrapper>;

  friend struct error_traits<std::error_code>;

public:

  constexpr error_code_error_domain() noexcept
      :
      error_domain{
          {0x84e99cdcecae4443ULL, 0x9050179b713fd2afULL},
          default_error_resource_management_t<internal_value_type>{}
      }
  { }

  virtual string_ref name() const noexcept override
  {
    return "std::error_code error domain";
  }

  virtual bool equivalent(const error& lhs, const error& rhs) const noexcept override;

  virtual string_ref message(const error& e) const noexcept override;

  [[noreturn]] virtual void throw_exception(const error& e) const override;
};

STDX_LEGACY_INLINE_CONSTEXPR error_code_error_domain error_code_domain {};

template <>
struct error_traits<std::error_code>
{
  static std::error_code from_exception(std::exception_ptr e) noexcept
  {
    return error_code_from_exception(std::move(e));
  }

  static std::exception_ptr to_exception(std::error_code ec) noexcept
  {
    return std::make_exception_ptr(std::system_error{ec});
  }

  static error to_error(std::error_code ec) noexcept;
};

namespace detail {

template <class Ptr, bool = (sizeof(Ptr) <= sizeof(std::intptr_t))>
struct exception_ptr_wrapper_impl
{
  struct control_block : enable_reference_count
  {
    explicit control_block(Ptr p) noexcept : ptr_(std::move(p))
    { }

    Ptr ptr_;
  };

  explicit exception_ptr_wrapper_impl(Ptr p) : ptr{new control_block{std::move(p)}}
  { }

  Ptr get() noexcept { return ptr ? ptr->ptr_ : Ptr{}; }

  intrusive_ptr<control_block> ptr;
};

template <class Ptr>
struct exception_ptr_wrapper_impl<Ptr, true>
{
  explicit exception_ptr_wrapper_impl(Ptr p) : ptr{std::move(p)}
  { }

  Ptr get() noexcept { return ptr; }

  Ptr ptr;
};

using exception_ptr_wrapper = exception_ptr_wrapper_impl<std::exception_ptr>;

static_assert(sizeof(exception_ptr_wrapper) == sizeof(std::intptr_t), "Internal library error");

} // end namespace detail

  #ifdef STDX_MUST_SPECIALIZE_IS_TRIVIALLY_RELOCATABLE
template <>
struct is_trivially_relocatable<detail::exception_ptr_wrapper> : std::true_type
{ };
  #endif

// Error domain mapping to std::exception_ptr
//
class dynamic_exception_error_domain : public error_domain
{
public:

  constexpr dynamic_exception_error_domain() noexcept
      :
      error_domain{
          {0x3c223c0aa3cf45e5ULL, 0x80dac24345cfb9fcULL},
          default_error_resource_management_t<detail::exception_ptr_wrapper>{}
      }
  { }

  virtual string_ref name() const noexcept override
  {
    return "dynamic exception domain";
  }

  virtual bool equivalent(const error& lhs, const error& rhs) const noexcept override;

  virtual string_ref message(const error&) const noexcept override;

  [[noreturn]] virtual void throw_exception(const error& e) const override
  {
    assert(e.domain() == *this);
    std::rethrow_exception(error_cast<detail::exception_ptr_wrapper>(e).get());
  }
};

STDX_LEGACY_INLINE_CONSTEXPR dynamic_exception_error_domain dynamic_exception_domain {};

// Error domain mapping to dynamic_exception_errc
//
class dynamic_exception_code_error_domain : public error_domain
{
public:

  constexpr dynamic_exception_code_error_domain() noexcept
      : error_domain{{0xa242506c26484677ULL, 0x82365303df25e338ULL}}
  { }

  virtual string_ref name() const noexcept override
  {
    return "dynamic exception code domain";
  }

  virtual bool equivalent(const error& lhs, const error& rhs) const noexcept override;

  virtual string_ref message(const error&) const noexcept override;
};

STDX_LEGACY_INLINE_CONSTEXPR dynamic_exception_code_error_domain dynamic_exception_code_domain {};

inline error make_error(dynamic_exception_errc code) noexcept
{
  return error{error_value<dynamic_exception_errc>{code}, dynamic_exception_code_domain};
}

struct thrown_dynamic_exception : std::exception
{
  explicit thrown_dynamic_exception(stdx::error e) noexcept : m_error(e)
  { }

  stdx::error error() const noexcept
  {
    return m_error;
  }

private:

  stdx::error m_error;
};

template <>
struct error_traits<std::exception_ptr>
{
  static std::exception_ptr from_exception(std::exception_ptr e) noexcept
  {
    return e;
  }

  static std::exception_ptr to_exception(std::exception_ptr e) noexcept
  {
    return e;
  }

  static error to_error(std::exception_ptr e) noexcept
  {
    return error{
        error_value<detail::exception_ptr_wrapper>{detail::exception_ptr_wrapper{e}},
        dynamic_exception_domain
    };
  }
};

} // end namespace stdx

namespace std {

template<>
struct is_error_code_enum<stdx::dynamic_exception_errc> : std::true_type
{ };

} // end namespace std

#endif



#if __cplusplus >= 201703L
  #include <any>
  #include <variant>
  #include <optional>
#endif

#include <functional>

NAMESPACE_STDX {

// namespace {

inline const char* dynamic_exception_errc_str(unsigned ev) noexcept
{
  constexpr const char* msg[] =
      {
          "Success",
          "std::runtime_error",
          "std::domain_error",
          "std::invalid_argument",
          "std::length_error",
          "std::out_of_range",
          "std::logic_error",
          "std::range_error",
          "std::overflow_error",
          "std::underflow_error",
          "std::bad_alloc",
          "std::bad_array_new_length",
          "std::bad_optional_access",
          "std::bad_typeid",
          "std::bad_any_cast",
          "std::bad_cast",
          "std::bad_weak_ptr",
          "std::bad_function_call",
          "std::bad_exception",
          "std::bad_variant_access",
          "unspecified dynamic exception"
      };

  assert(ev < (sizeof(msg) / sizeof(const char*)));
  return msg[ev];
}

class dynamic_exception_error_category : public std::error_category
{
public:

  const char* name() const noexcept override
  {
    return "dynamic_exception";
  }

  std::string message(int code) const override
  {
    return dynamic_exception_errc_str(code);
  }

  bool equivalent(int code, const std::error_condition& cond) const noexcept override
  {
    switch (static_cast<dynamic_exception_errc>(code))
    {
      case dynamic_exception_errc::domain_error:
        return (cond == std::errc::argument_out_of_domain);
      case dynamic_exception_errc::invalid_argument:
        return (cond == std::errc::invalid_argument);
      case dynamic_exception_errc::length_error:
        return (cond == std::errc::value_too_large);
      case dynamic_exception_errc::out_of_range:
      case dynamic_exception_errc::range_error:
      case dynamic_exception_errc::underflow_error:
        return (cond == std::errc::result_out_of_range);
      case dynamic_exception_errc::overflow_error:
        return (cond == std::errc::value_too_large);
      case dynamic_exception_errc::bad_alloc:
      case dynamic_exception_errc::bad_array_new_length:
        return (cond == std::errc::not_enough_memory);
      default:;
    }
    return false;
  }
};

inline const std::error_category& dynamic_exception_category() noexcept
{
  static const dynamic_exception_error_category dynamic_exception_error_category_instance;
  return dynamic_exception_error_category_instance;
}

//} // end anonymous namespace

inline std::error_code make_error_code(dynamic_exception_errc code) noexcept
{
  return std::error_code{static_cast<int>(code), dynamic_exception_category()};
}

inline std::error_code error_code_from_exception(std::exception_ptr eptr) noexcept
{
  if (!eptr) return make_error_code(dynamic_exception_errc::bad_exception);

  try
  {
    std::rethrow_exception(eptr);
  }
  catch (const std::domain_error&)
  {
    return make_error_code(dynamic_exception_errc::domain_error);
  }
  catch (const std::invalid_argument&)
  {
    return make_error_code(dynamic_exception_errc::invalid_argument);
  }
  catch (const std::length_error&)
  {
    return make_error_code(dynamic_exception_errc::length_error);
  }
  catch (const std::out_of_range&)
  {
    return make_error_code(dynamic_exception_errc::out_of_range);
  }
  catch (const std::logic_error&)
  {
    return make_error_code(dynamic_exception_errc::logic_error);
  }
  catch (const std::range_error&)
  {
    return make_error_code(dynamic_exception_errc::range_error);
  }
  catch (const std::overflow_error&)
  {
    return make_error_code(dynamic_exception_errc::overflow_error);
  }
  catch (const std::underflow_error&)
  {
    return make_error_code(dynamic_exception_errc::underflow_error);
  }
  catch (const std::system_error& e)
  {
    return e.code();
  }
  catch (const std::runtime_error&)
  {
    return make_error_code(dynamic_exception_errc::runtime_error);
  }
  catch (const std::bad_array_new_length&)
  {
    return make_error_code(dynamic_exception_errc::bad_array_new_length);
  }
  catch (const std::bad_alloc&)
  {
    return make_error_code(dynamic_exception_errc::bad_alloc);
  }
  catch (const std::bad_typeid&)
  {
    return make_error_code(dynamic_exception_errc::bad_typeid);
  }
#if __cplusplus >= 201703L
  catch (const std::bad_optional_access&)
  {
    return make_error_code(dynamic_exception_errc::bad_optional_access);
  }
  catch (const std::bad_any_cast&)
  {
    return make_error_code(dynamic_exception_errc::bad_any_cast);
  }
  catch (const std::bad_variant_access&)
  {
    return make_error_code(dynamic_exception_errc::bad_variant_access);
  }
#endif
  catch (const std::bad_cast&)
  {
    return make_error_code(dynamic_exception_errc::bad_cast);
  }
  catch (const std::bad_weak_ptr&)
  {
    return make_error_code(dynamic_exception_errc::bad_weak_ptr);
  }
  catch (const std::bad_function_call&)
  {
    return make_error_code(dynamic_exception_errc::bad_function_call);
  }
  catch (const std::bad_exception&)
  {
    return make_error_code(dynamic_exception_errc::bad_exception);
  }
  catch (...)
  { }

  return make_error_code(dynamic_exception_errc::unspecified_exception);
}

inline error error_from_exception(std::exception_ptr eptr) noexcept
{
  if (!eptr) return make_error(dynamic_exception_errc::bad_exception);

  try
  {
    std::rethrow_exception(eptr);
  }
  catch (const std::domain_error&)
  {
    return make_error(dynamic_exception_errc::domain_error);
  }
  catch (const std::invalid_argument&)
  {
    return make_error(dynamic_exception_errc::invalid_argument);
  }
  catch (const std::length_error&)
  {
    return make_error(dynamic_exception_errc::length_error);
  }
  catch (const std::out_of_range&)
  {
    return make_error(dynamic_exception_errc::out_of_range);
  }
  catch (const std::logic_error&)
  {
    return make_error(dynamic_exception_errc::logic_error);
  }
  catch (const std::range_error&)
  {
    return make_error(dynamic_exception_errc::range_error);
  }
  catch (const std::overflow_error&)
  {
    return make_error(dynamic_exception_errc::overflow_error);
  }
  catch (const std::underflow_error&)
  {
    return make_error(dynamic_exception_errc::underflow_error);
  }
  catch (const std::system_error& e)
  {
    return error{e.code()};
  }
  catch (const std::runtime_error&)
  {
    return make_error(dynamic_exception_errc::runtime_error);
  }
  catch (const std::bad_array_new_length&)
  {
    return make_error(dynamic_exception_errc::bad_array_new_length);
  }
  catch (const std::bad_alloc&)
  {
    return make_error(dynamic_exception_errc::bad_alloc);
  }
  catch (const std::bad_typeid&)
  {
    return make_error(dynamic_exception_errc::bad_typeid);
  }
#if __cplusplus >= 201703L
  catch (const std::bad_optional_access&)
  {
    return make_error(dynamic_exception_errc::bad_optional_access);
  }
  catch (const std::bad_any_cast&)
  {
    return make_error(dynamic_exception_errc::bad_any_cast);
  }
  catch (const std::bad_variant_access&)
  {
    return make_error(dynamic_exception_errc::bad_variant_access);
  }
#endif
  catch (const std::bad_cast&)
  {
    return make_error(dynamic_exception_errc::bad_cast);
  }
  catch (const std::bad_weak_ptr&)
  {
    return make_error(dynamic_exception_errc::bad_weak_ptr);
  }
  catch (const std::bad_function_call&)
  {
    return make_error(dynamic_exception_errc::bad_function_call);
  }
  catch (const std::bad_exception&)
  {
    return make_error(dynamic_exception_errc::bad_exception);
  }
  catch (...)
  { }

  return make_error(dynamic_exception_errc::unspecified_exception);
}

// ---------- ErrorDomain (abstract base class)
//
inline void error_domain::throw_exception(const error& e) const
{
  throw thrown_dynamic_exception{e};
}

// ---------- GenericErrorDomain
//
inline bool generic_error_domain::equivalent(const error& lhs, const error& rhs) const noexcept
{
  assert(lhs.domain() == *this);
  if (lhs.domain() == rhs.domain())
  {
    return error_cast<std::errc>(lhs) == error_cast<std::errc>(rhs);
  }

  return false;
}

// namespace {

inline string_ref generic_error_code_message(std::errc code) noexcept
{
  switch (code)
  {
    case std::errc::address_family_not_supported:
      return "Address family not supported by protocol";
    case std::errc::address_in_use:
      return "Address already in use";
    case std::errc::address_not_available:
      return "Cannot assign requested address";
    case std::errc::already_connected:
      return "Transport endpoint is already connected";
    case std::errc::argument_list_too_long:
      return "Argument list too long";
    case std::errc::argument_out_of_domain:
      return "Numerical argument out of domain";
    case std::errc::bad_address:
      return "Bad address";
    case std::errc::bad_file_descriptor:
      return "Bad file descriptor";
    case std::errc::bad_message:
      return "Bad message";
    case std::errc::broken_pipe:
      return "Broken pipe";
    case std::errc::connection_aborted:
      return "Software caused connection abort";
    case std::errc::connection_already_in_progress:
      return "Operation already in progress";
    case std::errc::connection_refused:
      return "Connection refused";
    case std::errc::connection_reset:
      return "Connection reset by peer";
    case std::errc::cross_device_link:
      return "Invalid cross-device link";
    case std::errc::destination_address_required:
      return "Destination address required";
    case std::errc::device_or_resource_busy:
      return "Device or resource busy";
    case std::errc::directory_not_empty:
      return "Directory not empty";
    case std::errc::executable_format_error:
      return "Exec format error";
    case std::errc::file_exists:
      return "File exists";
    case std::errc::file_too_large:
      return "File too large";
    case std::errc::filename_too_long:
      return "File name too long";
    case std::errc::function_not_supported:
      return "Function not implemented";
    case std::errc::host_unreachable:
      return "No route to host";
    case std::errc::identifier_removed:
      return "Identifier removed";
    case std::errc::illegal_byte_sequence:
      return "Invalid or incomplete multibyte or wide character";
    case std::errc::inappropriate_io_control_operation:
      return "Inappropriate ioctl for device";
    case std::errc::interrupted:
      return "Interrupted system call";
    case std::errc::invalid_argument:
      return "Invalid argument";
    case std::errc::invalid_seek:
      return "Illegal seek";
    case std::errc::io_error:
      return "Input/output error";
    case std::errc::is_a_directory:
      return "Is a directory";
    case std::errc::message_size:
      return "Message too long";
    case std::errc::network_down:
      return "Network is down";
    case std::errc::network_reset:
      return "Network dropped connection on reset";
    case std::errc::network_unreachable:
      return "Network is unreachable";
    case std::errc::no_buffer_space:
      return "No buffer space available";
    case std::errc::no_child_process:
      return "No child processes";
    case std::errc::no_link:
      return "Link has been severed";
    case std::errc::no_lock_available:
      return "No locks available";
    case std::errc::no_message:
      return "No message of desired type";
    case std::errc::no_protocol_option:
      return "Protocol not available";
    case std::errc::no_space_on_device:
      return "No space left on device";
    case std::errc::no_stream_resources:
      return "Out of streams resources";
    case std::errc::no_such_device_or_address:
      return "No such device or address";
    case std::errc::no_such_device:
      return "No such device";
    case std::errc::no_such_file_or_directory:
      return "No such file or directory";
    case std::errc::no_such_process:
      return "No such process";
    case std::errc::not_a_directory:
      return "Not a directory";
    case std::errc::not_a_socket:
      return "Socket operation on non-socket";
    case std::errc::not_a_stream:
      return "Device not a stream";
    case std::errc::not_connected:
      return "Transport endpoint is not connected";
    case std::errc::not_enough_memory:
      return "Cannot allocate memory";
#if ENOTSUP != EOPNOTSUPP
    case std::errc::not_supported:
      return "Operation not supported";
#endif
    case std::errc::operation_canceled:
      return "Operation canceled";
    case std::errc::operation_in_progress:
      return "Operation now in progress";
    case std::errc::operation_not_permitted:
      return "Operation not permitted";
    case std::errc::operation_not_supported:
      return "Operation not supported";
#if EAGAIN != EWOULDBLOCK
    case std::errc::operation_would_block:
      return "Resource temporarily unavailable";
#endif
    case std::errc::owner_dead:
      return "Owner died";
    case std::errc::permission_denied:
      return "Permission denied";
    case std::errc::protocol_error:
      return "Protocol error";
    case std::errc::protocol_not_supported:
      return "Protocol not supported";
    case std::errc::read_only_file_system:
      return "Read-only file system";
    case std::errc::resource_deadlock_would_occur:
      return "Resource deadlock avoided";
    case std::errc::resource_unavailable_try_again:
      return "Resource temporarily unavailable";
    case std::errc::result_out_of_range:
      return "Numerical result out of range";
    case std::errc::state_not_recoverable:
      return "State not recoverable";
    case std::errc::stream_timeout:
      return "Timer expired";
    case std::errc::text_file_busy:
      return "Text file busy";
    case std::errc::timed_out:
      return "Connection timed out";
    case std::errc::too_many_files_open_in_system:
      return "Too many open files in system";
    case std::errc::too_many_files_open:
      return "Too many open files";
    case std::errc::too_many_links:
      return "Too many links";
    case std::errc::too_many_symbolic_link_levels:
      return "Too many levels of symbolic links";
    case std::errc::value_too_large:
      return "Value too large for defined data type";
    case std::errc::wrong_protocol_type:
      return "Protocol wrong type for socket";
    default:
      return "Unspecified error";
  }
}

// } // end anonymous namespace

inline string_ref generic_error_domain::message(const error& e) const noexcept
{
  assert(e.domain() == *this);
  return generic_error_code_message(error_cast<std::errc>(e));
}

// ---------- ErrorCodeErrorDomain
//
inline string_ref error_code_error_domain::message(const error& e) const noexcept
{
  assert(e.domain() == *this);

  auto ptr = error_cast<internal_value_type>(e);
  if (ptr)
  {
    std::string msg = ptr->code.message();
    return shared_string_ref{msg.c_str(), msg.c_str() + msg.size()};
  }

  return string_ref{"Bad error code"};
}

inline void error_code_error_domain::throw_exception(const error& e) const
{
  assert(e.domain() == *this);

  std::error_code code;
  auto ptr = error_cast<internal_value_type>(e);
  if (ptr) code = ptr->code;
  throw std::system_error{code};
}

inline bool error_code_error_domain::equivalent(const error& lhs, const error& rhs) const noexcept
{
  assert(lhs.domain() == *this);

  if (lhs.domain() == rhs.domain())
  {
    auto ptr1 = error_cast<internal_value_type>(lhs);
    auto ptr2 = error_cast<internal_value_type>(rhs);
    if (ptr1 && ptr2) return ptr1->code == ptr2->code.default_error_condition();
    return false;
  }

  if (rhs.domain() == generic_domain)
  {
    auto ptr1 = error_cast<internal_value_type>(lhs);
    if (ptr1) return ptr1->code == error_cast<std::errc>(rhs);
  }

  return false;
}

inline stdx::error error_traits<std::error_code>::to_error(std::error_code ec) noexcept
{
  using internal_value_type = error_code_error_domain::internal_value_type;

  if (ec.category() == std::generic_category())
  {
    return error{
        error_value<std::errc>{static_cast<std::errc>(ec.default_error_condition().value())},
        generic_domain
    };
  }

  return error{
      error_value<internal_value_type>{internal_value_type{new detail::error_code_wrapper{ec}}},
      error_code_domain
  };
}

// ---------- DynamicExceptionErrorDomain
//
inline string_ref dynamic_exception_error_domain::message(const error& e) const noexcept
{
  assert(e.domain() == *this);

  std::exception_ptr eptr = error_cast<detail::exception_ptr_wrapper>(e).get();

  try
  {
    std::rethrow_exception(eptr);
  }
  catch (const std::exception& ex)
  {
    return shared_string_ref{ex.what()};
  }
  catch (...) {}

  return string_ref{"Unknown dynamic exception"};
}

// namespace {

inline std::errc dynamic_exception_code_to_generic_code(dynamic_exception_errc code) noexcept
{
  switch (code)
  {
    case dynamic_exception_errc::domain_error:
      return std::errc::argument_out_of_domain;
    case dynamic_exception_errc::invalid_argument:
      return std::errc::invalid_argument;
    case dynamic_exception_errc::length_error:
      return std::errc::value_too_large;
    case dynamic_exception_errc::out_of_range:
    case dynamic_exception_errc::range_error:
    case dynamic_exception_errc::underflow_error:
      return std::errc::result_out_of_range;
    case dynamic_exception_errc::overflow_error:
      return std::errc::value_too_large;
    case dynamic_exception_errc::bad_alloc:
    case dynamic_exception_errc::bad_array_new_length:
      return std::errc::not_enough_memory;
    default:;
  }
  return std::errc{};
}

// } // end anonymous namespace

inline bool dynamic_exception_error_domain::equivalent(const error& lhs, const error& rhs) const noexcept
{
  assert(lhs.domain() == *this);

  std::exception_ptr eptr = error_cast<detail::exception_ptr_wrapper>(lhs).get();

  if (rhs.domain() == *this)
  {
    std::exception_ptr eptr2 = error_cast<detail::exception_ptr_wrapper>(rhs).get();
    if (eptr == eptr2) return true;

    error e1 = error_from_exception(eptr);
    error e2 = error_from_exception(eptr2);
    return e1.domain().equivalent(e1, e2);
  }
  else if (rhs.domain() == error_code_domain)
  {
    std::error_code ec = error_code_from_exception(eptr);
    return error_code_domain.equivalent(rhs, error{ec});
  }

  error e = error_from_exception(eptr);
  return e.domain().equivalent(e, rhs);
}

// ---------- DynamicExceptionCodeErrorDomain
//
inline bool dynamic_exception_code_error_domain::equivalent(
    const error& lhs,
    const error& rhs
    ) const noexcept
{
  assert(lhs.domain() == *this);

  const dynamic_exception_errc code = error_cast<dynamic_exception_errc>(lhs);

  if (rhs.domain() == *this)
  {
    return code == error_cast<dynamic_exception_errc>(rhs);
  }
  else if (rhs.domain() == error_code_domain)
  {
    return error_code_domain.equivalent(rhs, make_error_code(code));
  }
  else if (rhs.domain() == generic_domain)
  {
    std::errc generic_code = dynamic_exception_code_to_generic_code(code);
    return generic_domain.equivalent(rhs, generic_code);
  }

  return false;
}

inline string_ref dynamic_exception_code_error_domain::message(const error& e) const noexcept
{
  assert(e.domain() == *this);
  return string_ref{
      dynamic_exception_errc_str(static_cast<unsigned>(error_cast<dynamic_exception_errc>(e)))
  };
}

} // end namespace stdx

#if defined(__clang__) && defined(__has_warning)
#pragma clang diagnostic pop
#endif
