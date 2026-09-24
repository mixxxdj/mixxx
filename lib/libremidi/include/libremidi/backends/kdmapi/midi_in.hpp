#pragma once
#include <libremidi/backends/kdmapi/config.hpp>
#include <libremidi/backends/dummy.hpp>
#include <libremidi/detail/midi_in.hpp>

namespace libremidi::kdmapi
{

using midi_in = libremidi::midi_in_dummy;

}
