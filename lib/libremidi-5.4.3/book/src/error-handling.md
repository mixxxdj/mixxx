# Error handling

The default error handling in the real-time API is done with an error type inspired from the proposed [std::error](https://github.com/charles-salvia/std_error). It has the advantage of working correctly in a header-only library unlike standard `std::error_code` and easily allows turning the error into an exception if needed, as well as getting back the original host API error code if any, for instance an ALSA or WinMM error code.

Example: 
```cpp
if (auto err = midiin.open_port(pi[0]); err != stdx::error{}) {
  std::print("{}", err.message());
  if(err == std::errc::bad_argument)
    err.throw_exception();
  // etc.
}
```
## Error callbacks

It is also possible to set a callback function which will be invoked upon error, for the `midi_in`, `observer` `midi_out` classes.

(Some classes may still throw, such as when creating invalid MIDI messages with the `libremidi::message` helpers, or the `observer` classes).

```cpp
// Create the configuration
libremidi::input_configuration conf{
    .on_message = /* usual message callback */
  , .on_error = [] (libremidi::midi_error code, std::string_view info) {
      // ... log error however you want
    }
  , .on_warning = [] (libremidi::midi_error code, std::string_view info) {
      // ... log warning however you want
    }
};

// Create the midi object
libremidi::midi_in midi{conf};
```

Ditto for `midi_out` and `observer`.

## Error handling in the MIDI file API

In this case, due to the non-realtime nature of the code, and to make the implementation legible, error handling is done with exceptions.