#pragma once
#include <libremidi/api.hpp>
#include <libremidi/config.hpp>
#include <libremidi/error.hpp>
#include <libremidi/input_configuration.hpp>
#include <libremidi/observer_configuration.hpp>
#include <libremidi/output_configuration.hpp>

#include <string_view>

NAMESPACE_LIBREMIDI
{
struct error_handler
{
  //! Error reporting function for libremidi classes.
  void error_impl(
      const midi_error_callback& callback, std::string_view errorString,
      const source_location& location) const
  {
    if (callback)
    {
      if (first_error)
        return;

      first_error = true;
      callback(errorString, location);
      first_error = false;
    }
    else
    {
      LIBREMIDI_LOG(errorString, " (", location.function_name(), ":", location.line(), ")");
    }
  }

  //! Warning reporting function for libremidi classes.
  void warning_impl(
      const midi_warning_callback& callback, std::string_view errorString,
      const source_location& location) const
  {
    if (callback)
    {
      if (first_warning)
        return;

      first_warning = true;
      callback(errorString, location);
      first_warning = false;
    }
    else
    {
      LIBREMIDI_LOG(errorString, " (", location.function_name(), ":", location.line(), ")");
    }
  }

  // To prevent infinite error loops
  mutable bool first_error{};
  mutable bool first_warning{};
};

// Needed as apple still doesn't support source_location in xcode 15.3
#define libremidi_handle_error(config, str) \
  error_impl(config.on_error, str, libremidi::source_location::current())
#define libremidi_handle_warning(config, str) \
  warning_impl(config.on_warning, str, libremidi::source_location::current())
}
