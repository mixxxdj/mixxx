# Reading MIDI 2 messages from a device through callbacks

Note that the MIDI 1 and MIDI 2 send and receive functions are useable no matter 
the kind of backend used (e.g. one can send UMPs to MIDI 1 backends and MIDI 1 messages to MIDI 2 backends). This conversion is done in a best-effort way.

Note also that libremidi by default will upscale MIDI 1 to MIDI 2 channel events when coming from user-code: it is safe to assume than when using UMP input, only MIDI 2 channel events have to be processed, not MIDI 1 channel events encapsulated into UMPs.

For Windows MIDI Services, a toggle allows to change this behaviour.

```cpp
// Set the configuration of our MIDI port, same warnings apply than for MIDI 1.
// Note that an UMP message is always at most 4 * 32 bits = 16 bytes.
// Added to the 64-bit timestamp this is 24 bytes for a libremidi::ump 
// which is definitely small enough to be passed by value.
// Note that libremidi::ump is entirely constexpr.
auto my_callback = [](libremidi::ump message) {
  // how many 32-bit UMP base elements (e.g. at least 1 uint32_t)
  message.size();
  // access to the individual UMP 
  message[i];
  // access to the timestamp
  message.timestamp;
};

// Create the midi object
libremidi::midi_in midi{ 
  libremidi::ump_input_configuration{ .on_message = my_callback }
};


// Open a given midi port. 
// The argument is a libremidi::input_port gotten from a libremidi::observer. 
midi.open_port(/* a port */);
// Alternatively, to get the default port for the system: 
midi.open_port(libremidi::midi2::in_default_port());

```
