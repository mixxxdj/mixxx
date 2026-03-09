# MIDI 2 integrations

libremidi integrates with the existing MIDI 2 ecosystem.

## cmidi2

We ship Atsushi Eno's [cmidi2](https://github.com/atsushieno/cmidi2) header-only MIDI 2 implementation as part of the library.
This allows to quickly match incoming messages with MIDI 2 types. A lot of useful utility functions are provided.
  
See a basic `midi2_echo.cpp` example: it uses the following printing function:

```cpp
std::ostream& operator<<(std::ostream& s, const libremidi::ump& message)
{
  // Automatic conversion from libremidi::ump& to cmidi2_ump*
  // Note that cmidi2_ump is just a typedef for uint32_t.
  const cmidi2_ump* b = message;
  
  // Read MIDI 2 information
  int bytes = cmidi2_ump_get_num_bytes(message.data[0]);
  int group = cmidi2_ump_get_group(b);
  int status = cmidi2_ump_get_status_code(b);
  int channel = cmidi2_ump_get_channel(b);
  s << "[ " << bytes << " | " << group;

  switch ((libremidi::message_type)status)
  {
    case libremidi::message_type::NOTE_ON:
      s << " | note on: " << channel << (int)cmidi2_ump_get_midi2_note_note(b) << " | "
        << cmidi2_ump_get_midi2_note_velocity(b);
      break;
    case libremidi::message_type::NOTE_OFF:
      s << " | note off: " << channel << (int)cmidi2_ump_get_midi2_note_note(b) << " | "
        << cmidi2_ump_get_midi2_note_velocity(b);
      break;
    case libremidi::message_type::CONTROL_CHANGE:
      s << " | cc: " << channel << (int)cmidi2_ump_get_midi2_cc_index(b) << " | "
        << cmidi2_ump_get_midi2_cc_data(b);
      break;

    default:
      break;
  }
  s << " ]";
  return s;
}
```

## ni-midi2

It is possible to enable compatibility with [ni-midi2](https://github.com/midi2-dev/ni-midi2, the MIDI Association-backed MIDI 2 implementation.
This is done with the `LIBREMIDI_NI_MIDI2` CMake switch: `-DLIBREMIDI_NI_MIDI2=ON`.

An example of initialisation of a MIDI 2-compliant communication can be found in the `midi2_interop.cpp` example.

Note that due to the complexity of MIDI 2, we believe end-users will want to build their 
own, custom-tailored stack and API on top of these base layers.

Current interoperability features are: 

- `libremidi::ump` can be converted to `midi::universal_packet` and conversely.
- `libremidi::midi_out::send_ump` can be called with `midi::universal_packet`, `midi::sysex7_packet` and `midi::sysex8_packet`.

This ensures that most MIDI-CI messages can be sent directly.
Example:

```
libremidi::midi_out midiout;

...

midiout.send_ump(midi::ci::make_discovery_inquiry(my_muid, id, 0x02, 512));
```
