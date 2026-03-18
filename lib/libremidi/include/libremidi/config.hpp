#pragma once

#define LIBREMIDI_VERSION "5.4.2"

// clang-format off
#if !defined(LIBREMIDI_BASE_NAMESPACE)
  #define LIBREMIDI_BASE_NAMESPACE libremidi
#endif
#if defined(LIBREMIDI_MODULE_BUILD)
  #define LIBREMIDI_MODULE_EXPORT export
  #define LIBREMIDI_MODULE_BEGIN_EXPORT export {
  #define LIBREMIDI_MODULE_END_EXPORT }
  #define NAMESPACE_LIBREMIDI export namespace LIBREMIDI_BASE_NAMESPACE
  #define NAMESPACE_STDX export namespace stdx
  #define LIBREMIDI_STATIC inline
  #define LIBREMIDI_STATIC_IMPLEMENTATION inline
  #define LIBREMIDI_STATIC_INLINE_IMPLEMENTATION inline
  #define LIBREMIDI_INLINE_IMPLEMENTATION
  #define LIBREMIDI_ANONYMOUS_NAMESPACE detail
#else
  #define LIBREMIDI_MODULE_EXPORT
  #define LIBREMIDI_MODULE_BEGIN_EXPORT
  #define LIBREMIDI_MODULE_END_EXPORT
  #define NAMESPACE_LIBREMIDI namespace LIBREMIDI_BASE_NAMESPACE
  #define NAMESPACE_STDX namespace stdx
  #define LIBREMIDI_STATIC static inline

#if defined(LIBREMIDI_HEADER_ONLY)
  #define LIBREMIDI_STATIC_IMPLEMENTATION static
  #define LIBREMIDI_STATIC_INLINE_IMPLEMENTATION static inline
  #define LIBREMIDI_INLINE_IMPLEMENTATION inline
#else
  #define LIBREMIDI_STATIC_IMPLEMENTATION static
  #define LIBREMIDI_STATIC_INLINE_IMPLEMENTATION static
  #define LIBREMIDI_INLINE_IMPLEMENTATION
#endif

  #define LIBREMIDI_ANONYMOUS_NAMESPACE
#endif
// clang-format on

#if defined(_MSC_VER)
  #define NOMINMAX 1
  #define WIN32_LEAN_AND_MEAN
#endif

// Dynamic exports
#if defined(LIBREMIDI_EXPORTS) || defined(LIBREMIDI_MODULE_BUILD)
  #if defined(_MSC_VER)
    #define LIBREMIDI_EXPORT __declspec(dllexport)
  #elif defined(__GNUC__) || defined(__clang__)
    #define LIBREMIDI_EXPORT __attribute__((visibility("default")))
  #endif
#else
  #define LIBREMIDI_EXPORT
#endif

// Boost check to prevent ABI issues
#if defined(LIBREMIDI_USE_BOOST)
  #if !__has_include(<boost/container/small_vector.hpp>)
    #error \
        "Boost was used for building libremidi but is not found when using it. Add Boost to your include paths."
  #endif

  #if defined(LIBREMIDI_NO_BOOST)
    #error "Boost was used for building libremidi but LIBREMIDI_NO_BOOST is defined."
  #endif
#endif

// Use boost::small_vector if available
#if __has_include(<boost/container/small_vector.hpp>) && !defined(LIBREMIDI_NO_BOOST)
  #if LIBREMIDI_SLIM_MESSAGE > 0
    #include <boost/container/static_vector.hpp>
NAMESPACE_LIBREMIDI
{
using midi_bytes = boost::container::static_vector<unsigned char, LIBREMIDI_SLIM_MESSAGE>;
}
  #else
    #include <boost/container/small_vector.hpp>
NAMESPACE_LIBREMIDI
{
LIBREMIDI_STATIC constexpr int small_vector_minimum_size
    = sizeof(boost::container::small_vector<unsigned char, 1>);
using midi_bytes = boost::container::small_vector<unsigned char, small_vector_minimum_size>;
}
  #endif
#else
  #include <vector>
NAMESPACE_LIBREMIDI
{
using midi_bytes = std::vector<unsigned char>;
}
#endif

// Use boost::variant2 if available
#if __has_include(<boost/variant2.hpp>) && !defined(LIBREMIDI_NO_BOOST_VARIANT2)
  #include <boost/variant2.hpp>
  #define LIBREMIDI_VARIANT_IS_BOOST_VARIANT2
namespace libremidi_variant_alias = boost::variant2;
#endif

#if !defined(LIBREMIDI_VARIANT_IS_BOOST_VARIANT2)
  #include <variant>
namespace libremidi_variant_alias = std;
#endif

namespace libremidi
{
using monostate = libremidi_variant_alias::monostate;
template <typename T>
concept nothrow_move_constructible = std::is_nothrow_move_constructible_v<T>;

template <nothrow_move_constructible... Args>
using variant = libremidi_variant_alias::variant<Args...>;

template <typename... Args>
using slow_variant = libremidi_variant_alias::variant<Args...>;

template <std::size_t N, typename T>
using variant_element = libremidi_variant_alias::variant_alternative<N, T>;
template <std::size_t N, typename T>
using variant_element_t = libremidi_variant_alias::variant_alternative_t<N, T>;

using libremidi_variant_alias::operator==;
using libremidi_variant_alias::operator!=;
using libremidi_variant_alias::operator<;
using libremidi_variant_alias::operator>;
using libremidi_variant_alias::operator<=;
using libremidi_variant_alias::operator>=;

// using boost::variant2::in_place;
// using libremidi_variant_alias::in_place;
using libremidi_variant_alias::get;
using libremidi_variant_alias::get_if;
using libremidi_variant_alias::in_place_index;
using libremidi_variant_alias::in_place_type;
using libremidi_variant_alias::visit;
}

#if __has_include(<midi/universal_packet.h>) && defined(LIBREMIDI_USE_NI_MIDI2)
  #define LIBREMIDI_NI_MIDI2_COMPAT 1
#endif

#if defined(LIBREMIDI_HEADER_ONLY)
  #define LIBREMIDI_INLINE inline
#else
  #define LIBREMIDI_INLINE
#endif

#if __cpp_contracts >= 202502L
  #define LIBREMIDI_PRECONDITION(...) pre(__VA_ARGS__)
#else
  #define LIBREMIDI_PRECONDITION(...)
#endif
