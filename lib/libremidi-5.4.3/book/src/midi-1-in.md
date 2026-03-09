# Reading MIDI 1 messages from a device through callbacks

```cpp
// Set the configuration of our MIDI port
// Note that the callback will be invoked from a separate thread,
// it is up to you to protect your data structures afterwards.
// For instance if you are using a GUI toolkit, don't do GUI actions
// in that callback !
auto my_callback = [](const libremidi::message& message) {
  // how many bytes
  message.size();
  // access to the individual bytes
  message[i];
  // access to the timestamp
  message.timestamp;
};

// Create the midi object
libremidi::midi_in midi{ 
  libremidi::input_configuration{ .on_message = my_callback } 
};

// Open a given midi port. 
// The argument is a libremidi::input_port gotten from a libremidi::observer. 
midi.open_port(/* a port */);
// Alternatively, to get the default port for the system: 
if(auto port = libremidi::midi1::in_default_port())
  midi.open_port(*port);

// Note that only one port can be open at a given time on a midi_in or midi_out object.

```
