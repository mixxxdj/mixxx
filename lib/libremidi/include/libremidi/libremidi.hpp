#pragma once
/* This software is based on the RtMidi and ModernMidi libraries.

  RtMidi WWW site: http://music.mcgill.ca/~gary/libremidi/

  RtMidi: realtime MIDI i/o C++ classes
  Copyright (c) 2003-2017 Gary P. Scavone

  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation files
  (the "Software"), to deal in the Software without restriction,
  including without limitation the rights to use, copy, modify, merge,
  publish, distribute, sublicense, and/or sell copies of the Software,
  and to permit persons to whom the Software is furnished to do so,
  subject to the following conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  Any person wishing to distribute modifications to the Software is
  asked to send the modifications to the original developer so that
  they can be incorporated into the canonical version.  This is,
  however, not a binding provision of this license.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
  ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
  CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 ------------

  ModernMidi Copyright (c) 2015, Dimitri Diakopoulos All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#include <libremidi/api.hpp>
#include <libremidi/configurations.hpp>
#include <libremidi/defaults.hpp>
#include <libremidi/input_configuration.hpp>
#include <libremidi/message.hpp>
#include <libremidi/observer_configuration.hpp>
#include <libremidi/output_configuration.hpp>

#if LIBREMIDI_NI_MIDI2_COMPAT
  #include <midi/sysex.h>
  #include <midi/universal_packet.h>
#endif

NAMESPACE_LIBREMIDI
{
//! Main class for observing hotplug of MIDI 1.0 and 2.0 devices.
//! The callbacks will be called whenever a device is added or removed
//! for a given API.
class LIBREMIDI_EXPORT observer
{
public:
  //! Open an observer instance with the given configuration.
  //!
  //! * api_conf can be an instance of observer_configuration,
  //!   such as jack_observer_configuration, winmm_observer_configuration, etc...
  //! * if no callbacks are passed, no secondary thread will be created unless absolutely necessary
  explicit observer(const observer_configuration& conf = {}) noexcept;
  explicit observer(const observer_configuration& conf, observer_api_configuration api_conf);
  observer(const observer&) = delete;
  observer(observer&& other) noexcept;
  observer& operator=(const observer&) = delete;
  observer& operator=(observer&& other) noexcept;
  ~observer();

  [[nodiscard]] libremidi::API get_current_api() const noexcept;

  //! Return identifiers for the available MIDI ports
  [[nodiscard]] std::vector<libremidi::input_port> get_input_ports() const noexcept;
  [[nodiscard]] std::vector<libremidi::output_port> get_output_ports() const noexcept;

private:
  std::unique_ptr<class observer_api> m_impl;
};

//! Main class for receiving MIDI 1.0 and 2.0 messages.
class LIBREMIDI_EXPORT midi_in
{
public:
  //! Construct a midi_in object with the default MIDI 1 back-end for the platform
  explicit midi_in(const input_configuration& conf) noexcept;

  //! Construct a midi_in object with a configuration object for a specific MIDI 1 back-end
  //! see configuration.hpp for the available configuration types.
  //! An exception will be thrown if the requested back-end cannot be opened.
  explicit midi_in(const input_configuration& conf, const input_api_configuration& api_conf);

  //! Construct a midi_in object with the default MIDI 2 back-end for the platform
  explicit midi_in(const ump_input_configuration& conf) noexcept;

  //! Construct a midi_in object with a configuration object for a specific MIDI 2 back-end
  //! see configuration.hpp for the available configuration types.
  //! An exception will be thrown if the requested back-end cannot be opened.
  explicit midi_in(const ump_input_configuration& conf, const input_api_configuration& api_conf);

  midi_in(const midi_in&) = delete;
  midi_in(midi_in&& other) noexcept;
  midi_in& operator=(const midi_in&) = delete;
  midi_in& operator=(midi_in&& other) noexcept;
  ~midi_in();

  //! Returns the MIDI API specifier for the current instance of midi_in.
  [[nodiscard]] libremidi::API get_current_api() const noexcept;

  //! Open a MIDI input connection
  stdx::error
  open_port(const input_port& pt, std::string_view local_port_name = "libremidi input");

  //! Create a virtual input port, with optional name, to allow software
  //! connections.
  //!
  //! \param portName An optional name for the application port that is
  //!                 used to connect to portId can be specified.
  stdx::error open_virtual_port(std::string_view portName = "libremidi virtual port");

  stdx::error set_port_name(std::string_view portName);

  //! Close an open MIDI connection (if one exists).
  stdx::error close_port();

  //! Returns true if a port has been opened successfully with open_port or open_virtual_port
  [[nodiscard]] bool is_port_open() const noexcept;

  //! Returns true if a port is connected to another port.
  //! Never true for virtual ports.
  [[nodiscard]] bool is_port_connected() const noexcept;

  //! Returns the current timestamp for absolute ticks.
  timestamp absolute_timestamp() const noexcept;

private:
  std::unique_ptr<class midi_in_api> m_impl;
};

//! Main class for sending MIDI 1.0 and 2.0 messages.
class LIBREMIDI_EXPORT midi_out
{
public:
  //! Construct a midi_out object with the default back-end for the platform
  explicit midi_out(const output_configuration& conf = {}) noexcept;

  //! Construct a midi_out object with a configuration object for a specific back-end
  //! see configuration.hpp for the available configuration types.
  //! An exception will be thrown if the requested back-end cannot be opened.
  explicit midi_out(const output_configuration& conf, const output_api_configuration& api_conf);

  midi_out(const midi_out&) = delete;
  midi_out(midi_out&& other) noexcept;
  midi_out& operator=(const midi_out&) = delete;
  midi_out& operator=(midi_out&& other) noexcept;
  ~midi_out();

  //! Returns the MIDI API specifier for the current instance of midi_out.
  [[nodiscard]] libremidi::API get_current_api() const noexcept;

  //! Open a MIDI output connection.
  stdx::error
  open_port(const output_port& pt, std::string_view local_port_name = "libremidi output") const;

  //! Close an open MIDI connection (if one exists).
  stdx::error close_port() const;

  //! Returns true if a port has been opened successfully with open_port or open_virtual_port
  [[nodiscard]] bool is_port_open() const noexcept;

  //! Returns true if a port is connected to another port.
  //! Never true for virtual ports.
  [[nodiscard]] bool is_port_connected() const noexcept;

  //! Create a virtual output port, with optional name, to allow software
  //! connections.
  //!
  //! \param portName An optional name for the application port that is
  //!                 used to connect to portId can be specified.
  stdx::error open_virtual_port(std::string_view portName = "libremidi virtual port") const;

  stdx::error set_port_name(std::string_view portName) const;

  //! Immediately send a single message out an open MIDI output port.
  /*!
      An exception is thrown if an error occurs during output or an
      output connection was not previously established.
  */
  stdx::error send_message(const libremidi::message& message) const;

  //! Immediately send a single message to an open MIDI output port.
  stdx::error send_message(const unsigned char* message, size_t size) const;
  stdx::error send_message(std::span<const unsigned char>) const;
  stdx::error send_message(unsigned char b0) const;
  stdx::error send_message(unsigned char b0, unsigned char b1) const;
  stdx::error send_message(unsigned char b0, unsigned char b1, unsigned char b2) const;

  // Avoid silly mistakes:
  stdx::error send_message(auto* message) const noexcept = delete;
  stdx::error send_message(const auto* message) const noexcept = delete;

  //! Current time in the timestamp referential
  int64_t current_time();

  //! Try to schedule a message later in time if the underlying API supports it
  //! (currently not implemented anywhere)
  stdx::error schedule_message(int64_t timestamp, const unsigned char* message, size_t size) const;

  //! Immediately send a single UMP packet to an open MIDI output port.
  stdx::error send_ump(const uint32_t* message, size_t size) const;
  stdx::error send_ump(const libremidi::ump&) const;
  stdx::error send_ump(std::span<const uint32_t>) const;
  stdx::error send_ump(uint32_t b0) const;
  stdx::error send_ump(uint32_t b0, uint32_t b1) const;
  stdx::error send_ump(uint32_t b0, uint32_t b1, uint32_t b2) const;
  stdx::error send_ump(uint32_t b0, uint32_t b1, uint32_t b2, uint32_t b3) const;
  // Better compat with cmidi2
  stdx::error send_ump(int32_t b0) const;
  stdx::error send_ump(int64_t b01) const;
  stdx::error send_ump(uint64_t b01) const;

// Interop with ni-midi2
#if LIBREMIDI_NI_MIDI2_COMPAT
  stdx::error send_ump(const midi::universal_packet& pkt) const { send_ump(pkt.data, pkt.size()); }
  stdx::error send_ump(const midi::sysex7& msg, int group = 0)
  {
    midi::send_sysex7(msg, group, [&](const midi::sysex7_packet& x) { send_ump(x.data); });
  }
  stdx::error send_ump(const midi::sysex8& msg, int stream, int group = 0)
  {
    midi::send_sysex8(msg, stream, group, [&](const midi::sysex8_packet& x) { send_ump(x.data); });
  }
#endif

  //! Try to schedule an UMP packet later in time if the underlying API supports it
  //! (currently not implemented anywhere)
  stdx::error schedule_ump(int64_t timestamp, const uint32_t* message, size_t size) const;

private:
  std::unique_ptr<class midi_out_api> m_impl;
};
}

#if defined(LIBREMIDI_HEADER_ONLY)
  #include <libremidi/libremidi.cpp>
  #include <libremidi/midi_in.cpp>
  #include <libremidi/midi_out.cpp>
  #include <libremidi/observer.cpp>

  #if defined(__EMSCRIPTEN__)
    #include <libremidi/backends/emscripten/midi_access.cpp>
    #include <libremidi/backends/emscripten/midi_in.cpp>
    #include <libremidi/backends/emscripten/midi_out.cpp>
    #include <libremidi/backends/emscripten/observer.cpp>
  #endif
#endif
