#pragma once
#include <libremidi/config.hpp>
#include <libremidi/error.hpp>

#include <cstdint>
#include <functional>
#include <span>

NAMESPACE_LIBREMIDI
{
/**
 * Configuration for raw I/O MIDI 1.0 input.
 *
 * The user provides a function that will be called with a callback
 * to invoke whenever raw MIDI bytes are received from the transport
 * (serial port, SPI, USB, etc.)
 *
 * Example with Arduino:
 *   rawio_input_configuration::receive_callback stored_cb;
 *   .set_receive_callback = [&](auto cb) { stored_cb = cb; },
 *   .stop_receive = [&] { stored_cb = nullptr; }
 *   // In serialEvent(): stored_cb({buf, n}, 0);
 */
struct rawio_input_configuration
{
  /// Signature of the callback the library gives to the user.
  /// The user should call it with incoming MIDI bytes and a timestamp
  /// (in nanoseconds, or 0 if unknown).
  using receive_callback = std::function<void(std::span<const uint8_t>, int64_t)>;

  /// Called when the port opens. The library passes a callback that
  /// the user must store and invoke whenever bytes arrive.
  std::function<void(receive_callback)> set_receive_callback
      = [](const receive_callback&) {};

  /// Called when the port closes. The user should stop calling the
  /// receive callback after this.
  std::function<void()> stop_receive = [] {};
};

/**
 * Configuration for raw I/O MIDI 1.0 output.
 *
 * The user provides a function to write raw MIDI bytes to their transport.
 *
 * Example:
 *   .write_bytes = [](std::span<const uint8_t> bytes) {
 *     Serial.write(bytes.data(), bytes.size());
 *     return stdx::error{};
 *   }
 */
struct rawio_output_configuration
{
  /// Called by the library to send raw MIDI bytes over the user's transport.
  std::function<stdx::error(std::span<const uint8_t>)> write_bytes
      = [](std::span<const uint8_t>) { return stdx::error{}; };
};

/// Observer configuration - raw I/O has no port enumeration.
struct rawio_observer_configuration
{
};

/**
 * Configuration for raw I/O MIDI 2.0 (UMP) input.
 *
 * Same pattern as rawio_input_configuration but with uint32_t UMP words.
 */
struct rawio_ump_input_configuration
{
  /// Signature of the callback the library gives to the user.
  /// The user should call it with incoming UMP words and a timestamp
  /// (in nanoseconds, or 0 if unknown).
  using receive_callback = std::function<void(std::span<const uint32_t>, int64_t)>;

  /// Called when the port opens. The library passes a callback that
  /// the user must store and invoke whenever UMP words arrive.
  std::function<void(receive_callback)> set_receive_callback
      = [](const receive_callback&) {};

  /// Called when the port closes.
  std::function<void()> stop_receive = [] {};
};

/**
 * Configuration for raw I/O MIDI 2.0 (UMP) output.
 *
 * The user provides a function to write UMP words to their transport.
 */
struct rawio_ump_output_configuration
{
  /// Called by the library to send UMP words over the user's transport.
  std::function<stdx::error(std::span<const uint32_t>)> write_ump
      = [](std::span<const uint32_t>) { return stdx::error{}; };
};

/// Observer configuration - raw I/O UMP has no port enumeration.
struct rawio_ump_observer_configuration
{
};
}
