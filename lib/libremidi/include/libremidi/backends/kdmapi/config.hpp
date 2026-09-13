#pragma once
#include <libremidi/config.hpp>

namespace libremidi::kdmapi
{
// KDMAPI does not support MIDI input
struct input_configuration
{
};

struct output_configuration
{
  // Use the no-buffer variant of SendDirectData for lowest latency
  // This bypasses OmniMIDI's internal buffer
  bool use_no_buffer = false;
};

struct observer_configuration
{
};

}

