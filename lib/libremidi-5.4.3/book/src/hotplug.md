# Device connection / disconnection notification

```cpp
// The callbacks will be called when the relevant event happens.
// Note that they may be called from other threads than the main thread.

libremidi::observer_configuration conf{
    .input_added = [&] (const libremidi::input_port& id) {
      std::cout << "Input connected: " << id.port_name << std::endl;
    },
    .input_removed = [&] (const libremidi::input_port& id) {
      std::cout << "Input removed: " << id.port_name << std::endl;
    },
    .output_added = [&] (const libremidi::output_port& id) {
      std::cout << "Output connected: " << id.port_name << std::endl;
    },
    .output_removed = [&] (const libremidi::output_port& id) {
      std::cout << "Output removed: " << id.port_name << std::endl;
}};

libremidi::observer obs{std::move(conf)};
```

See `midiobserve.cpp` or `emscripten_midiin.cpp` for an example.
