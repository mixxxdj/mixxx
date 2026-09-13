#pragma once
#include <libremidi/config.hpp>
#include <libremidi/error.hpp>
#include <libremidi/input_configuration.hpp>

NAMESPACE_LIBREMIDI
{
struct output_configuration
{
  //! Set an error callback function to be invoked when an error has occured.
  /*!
    The callback function will be called whenever an error has occured. It is
    best to set the error callback function before opening a port.
  */
  midi_error_callback on_error{};
  midi_warning_callback on_warning{};

  //! Timestamp mode for the timestamps passed to schedule_message
  uint32_t timestamps : 3 = timestamp_mode::Absolute;
};
}
