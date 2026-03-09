# Sending MIDI 2 messages to a device

```cpp
// Create the midi object
libremidi::midi_out midi;

// Open a given midi port. Same as for input:
if(auto port = libremidi::midi2::out_default_port())
  midi.open_port(*port);

// Option A: send fixed amount of bytes for most basic cases
midi.send_ump(A, B, C, D); // Overloads exist for 1, 2, 3, 4 uint32_t

// Option B: send a raw byte array.
// This can contain a sequence of UMP messages.
uint32_t bytes[2] = { ... };
midi.send_ump(bytes, sizeof(bytes));

// Option C: std::span<uint32_t>
// This allows to pass std::vector, std::array and the likes
// This can contain a sequence of UMP messages.
midi.send_ump(std::span<uint32_t>{ ... your span-compatible data-structure ... });

// Option D: helpers with the libremidi::ump class
// The helpers haven't been implemented yet :(
midi.send_ump(libremidi::ump{ /* a message */ });
```
