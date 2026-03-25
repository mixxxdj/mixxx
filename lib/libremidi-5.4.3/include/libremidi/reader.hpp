/*
Copyright (c) 2015, Dimitri Diakopoulos All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <libremidi/message.hpp>

NAMESPACE_LIBREMIDI
{
/**
 * @brief reads Standard MIDI files (SMF).
 *
 * Usage:

 * ```
 * libremidi::reader r;
 * auto res = r.parse(midi_bytes, num_bytes);
 * ```
 */
class LIBREMIDI_EXPORT reader
{
public:
  enum parse_result
  {
    invalid,    //! Nothing could be parsed
    incomplete, //! Some of the data could be parsed, but not all: there may be missing events /
                //! tracks
    complete,   //! All the data could be parsed but not necessarily validated
    validated   //! The data could be parsed and conforms to SMF rules
  };
  explicit reader(bool useAbsolute = false);
  ~reader();

  parse_result parse(const uint8_t* data, std::size_t size) noexcept;
  parse_result parse(const std::vector<uint8_t>& buffer) noexcept;
  parse_result parse(std::span<uint8_t> buffer) noexcept;

  [[nodiscard]] double get_end_time() const noexcept;

  float ticksPerBeat{}; // precision (number of ticks distinguishable per second)
  float startingTempo{};
  int format{};

  std::vector<midi_track> tracks;

private:
  bool m_useAbsoluteTicks{};
};
}

#if defined(LIBREMIDI_HEADER_ONLY)
  #include <libremidi/reader.cpp>
#endif
