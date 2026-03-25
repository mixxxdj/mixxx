# Context sharing

This allows to share a single context across multiple MIDI objects (such as a `jack_client_t` with JACK, a PipeWire main loop & filter, or a `MIDIClientRef` on macOS with CoreMIDI). 
If no context is passed, each object will create one as they used to.

Example:

```cpp
#include <libremidi/configurations.hpp>

...

libremidi::midi_in in{
    libremidi::input_configuration{.on_message = ...}
  , libremidi::alsa_seq::input_configuration{
      .client_name = "my client"
  } 
};
```

If one simply wants to share a context across libremidi objects (for instance, a single context shared across an `observer`, `midi_ins` and `midi_outs`), the following methods will create appropriate configurations from an observer's configuration: 

```cpp
// Create an observer with a fixed back-end
libremidi::observer obs{
    libremidi::observer_configuration{}
  , libremidi::observer_configuration_for(libremidi::API::JACK_MIDI)};

// The in and out will share the JACK client of the observer.
// Note that the observer has to outlive them.
libremidi::midi_in in{
    libremidi::input_configuration{.on_message = ...}
  , libremidi::midi_in_configuration_for(obs) 
};

libremidi::midi_out out{
    libremidi::output_configuration{...}
  , libremidi::midi_out_configuration_for(obs) 
};
```

In that case, note that the `obs` has ownership of for instance the JACK context object: it must outlive `in` and `out`.

The relevant examples are:
- `coremidi_share.cpp` for a complete example for CoreMIDI.
- `jack_share.cpp` for a complete example for JACK.
- `pipewire_share.cpp` for a complete example for PipeWire.
