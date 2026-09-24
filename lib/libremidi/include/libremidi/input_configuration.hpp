#pragma once
#include <libremidi/config.hpp>
#include <libremidi/error.hpp>
#include <libremidi/message.hpp>
#include <libremidi/ump.hpp>

#include <functional>

NAMESPACE_LIBREMIDI
{
//! Specify how timestamps are handled in the system
enum timestamp_mode
{
  //! No timestamping at all, all timestamps are zero
  NoTimestamp,

  //! In nanoseconds, timestamp is the time since the previous event (or zero)
  Relative,

  //! In nanoseconds, as per an arbitrary reference which may be provided by the host API,
  //! e.g. since the JACK cycle start, ALSA sequencer queue creation, through AudioHostTime on macOS.
  //! It offers the most precise ordering between events as it's the closest to the real timestamp of
  //! the event as provided by the host API.
  //! If the API does not provide any timing, it will be mapped to SystemMonotonic instead.
  Absolute,

  //! In nanoseconds, as per std::steady_clock::now() or equivalent (raw if possible).
  //! May be less precise than Absolute as timestamping is done within the library,
  //! but is more useful for system-wide synchronization.
  //! Note: depending on the backend, Absolute and SystemMonotonic may be the same.
  SystemMonotonic,

  //! For APIs which are based on audio process cycles such as JACK, timestamps will be in frames since
  //! the beginning of the current cycle's audio buffer
  AudioFrame,

  //! Will call the custom timestamping function provided by the user in the input configuration.
  Custom
};

using timestamp = int64_t;
using message_callback = std::function<void(message&& message)>;
using raw_callback = std::function<void(std::span<const uint8_t>, timestamp)>;
using timestamp_callback = std::function<timestamp(timestamp)>;
struct input_configuration
{
  //! Set a callback function to be invoked for incoming MIDI messages.
  //! Either this or on_raw_message must be set
  message_callback on_message{};

  //! Invoked for incoming MIDI bytes. No transformation, no filtering, no packetization,
  //! just the MIDI data straight from the source.
  raw_callback on_raw_data{};

  //! Set a custom callback function to be invoked for timestamping MIDI messages.
  //! Input: the API provided timestamp in nanoseconds, if available, for reference.
  //! (e.g. the same as "Absolute").
  //! Mandatory if timestamps == timestamp_mode::Custom, unused otherwise.
  timestamp_callback get_timestamp{};

  //! Set an error callback function to be invoked when an error has occured.
  /*!
    The callback function will be called whenever an error has occured. It is
    best to set the error callback function before opening a port.
  */
  midi_error_callback on_error{};
  midi_warning_callback on_warning{};

  //! Specify whether certain MIDI message types should be queued or ignored
  //! during input.
  /*!
    By default, MIDI timing and active sensing messages are ignored
    during message input because of their relative high data rates.
    MIDI sysex messages are ignored by default as well.  Variable
    values of "true" imply that the respective message type will be
    ignored.
  */
  uint32_t ignore_sysex : 1 = true;
  uint32_t ignore_timing : 1 = true;
  uint32_t ignore_sensing : 1 = true;

  //! Timestamp mode. See @libremidi::timestamp_mode
  uint32_t timestamps : 3 = timestamp_mode::Absolute;
};

using ump_callback = std::function<void(ump&&)>;
using raw_ump_callback = std::function<void(std::span<const uint32_t>, timestamp)>;
struct ump_input_configuration
{
  //! Set a callback function to be invoked for incoming UMP messages.
  //! Either this or on_raw_message must be set
  ump_callback on_message{};

  //! Invoked for incoming UMP bytes. No transformation, no filtering, no packetization,
  //! just the UMP data straight from the source.
  raw_ump_callback on_raw_data{};

  //! Set a custom callback function to be invoked for timestamping MIDI messages.
  //! Input: the API provided timestamp in nanoseconds, if available, for reference.
  //! (e.g. the same as "Absolute").
  //! Mandatory if timestamps == timestamp_mode::Custom, unused otherwise.
  timestamp_callback get_timestamp{};

  //! Set an error callback function to be invoked when an error has occured.
  /*!
    The callback function will be called whenever an error has occured. It is
    best to set the error callback function before opening a port.
  */
  midi_error_callback on_error{};
  midi_warning_callback on_warning{};

  //! Specify whether certain MIDI message types should be queued or ignored
  //! during input.
  /*!
    By default, MIDI timing and active sensing messages are ignored
    during message input because of their relative high data rates.
    MIDI sysex messages are ignored by default as well.  Variable
    values of "true" imply that the respective message type will be
    ignored.
  */
  uint32_t ignore_sysex : 1 = true;
  uint32_t ignore_timing : 1 = true;
  uint32_t ignore_sensing : 1 = true;

  uint32_t timestamps : 3 = timestamp_mode::Absolute;

  //! Upscale MIDI 1 channel events to MIDI 2 channel events.
  //! Note that this only has an effect on Windows with MIDI Services
  //! as other platforms already do this by default.
  uint32_t midi1_channel_events_to_midi2 : 1 = true;
};
}
