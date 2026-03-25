# Enumerating ports

The required header is `#include <libremidi/libremidi.hpp>`.

Inputs:
```cpp
libremidi::observer obs;
for(const libremidi::input_port& port : obs.get_input_ports()) {
  std::cout << port.port_name << "\n";
}
```

Outputs:
```cpp
libremidi::observer obs;
for(const libremidi::output_port& port : obs.get_output_ports()) {
  std::cout << port.port_name << "\n";
}
```

See `midiprobe.cpp` for a simple example.
