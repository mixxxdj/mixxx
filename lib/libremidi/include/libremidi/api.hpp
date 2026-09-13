#pragma once
#include <libremidi/api-c.h>
#include <libremidi/config.hpp>

#include <string_view>
#include <vector>

NAMESPACE_LIBREMIDI
{
//! MIDI API specifier arguments.
//! To get information on which feature is supported by each back-end, check their backend file
//! in e.g. backends/winmm.hpp, etc.
using API = libremidi_api;

/**
 * \brief A function to determine the available compiled MIDI 1.0 APIs.

  The values returned in the std::vector can be compared against
  the enumerated list values.  Note that there can be more than one
  API compiled for certain operating systems.
*/
LIBREMIDI_EXPORT std::vector<libremidi::API> available_apis() noexcept;

/**
 * \brief A function to determine the available compiled MIDI 2.0 APIs.

  The values returned in the std::vector can be compared against
  the enumerated list values.  Note that there can be more than one
  API compiled for certain operating systems.
*/
LIBREMIDI_EXPORT std::vector<libremidi::API> available_ump_apis() noexcept;

//! A static function to determine the current version.
LIBREMIDI_EXPORT std::string_view get_version() noexcept;

//! Map from and to API names
LIBREMIDI_EXPORT std::string_view get_api_name(libremidi::API api);
//! Map from and to API names
LIBREMIDI_EXPORT std::string_view get_api_display_name(libremidi::API api);
//! Look-up an API through its name
LIBREMIDI_EXPORT libremidi::API get_compiled_api_by_name(std::string_view api);

inline constexpr bool is_midi1(libremidi::API api)
{
  return (static_cast<int>(api) >= 0x1 && static_cast<int>(api) < 0x1000)
         || api == libremidi::API::DUMMY;
}

inline constexpr bool is_midi2(libremidi::API api)
{
  return static_cast<int>(api) >= 0x1000;
}

namespace midi1
{
//! Returns the default MIDI 1.0 backend to use for the target OS.
inline constexpr libremidi::API default_api() noexcept
{
#if defined(__APPLE__)
  return API::COREMIDI;
#elif defined(_WIN32)
  return API::WINDOWS_MM;
#elif defined(LIBREMIDI_ALSA)
  return API::ALSA_SEQ;
#elif defined(__emscripten__)
  return API::EMSCRIPTEN_WEBMIDI;
#elif defined(LIBREMIDI_ANDROID)
  return API::ANDROID_AMIDI;
#else
  return API::DUMMY;
#endif
}
}

namespace midi2
{
//! Returns the default MIDI 2.0 backend to use for the target OS.
inline constexpr libremidi::API default_api() noexcept
{
#if defined(__APPLE__)
  return API::COREMIDI_UMP;
#elif defined(_WIN32)
  return API::WINDOWS_MIDI_SERVICES;
#elif defined(LIBREMIDI_ALSA)
  return API::ALSA_SEQ_UMP;
#elif defined(__emscripten__)
  return API::DUMMY;
#else
  return API::DUMMY;
#endif
}
}
}
