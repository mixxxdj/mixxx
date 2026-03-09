# Advanced configuration

The `midi_in`, `midi_out` and `midi_observer` objects are configured through a `input_configuration` (resp. `output_`, etc.) object passed in argument to the constructor.

Example:

```cpp
#include <libremidi/configurations.hpp>

...

libremidi::midi_in in{
    libremidi::input_configuration{
      .on_message = ...
    , .ignore_sysex = false
    , .ignore_sensing = true
    }
};
```

Note that by default, sysex are ignored and have to be enabled with `ignore_sysex = false` if desired.

## Custom back-end configuration

Additionnally, each back-end supports back-end specific configuration options, to enable users to tap into advanced features of a given API while retaining the general C++ abstraction.

For instance, this enables to set output buffer sizes, chunking parameters, etc. for back-ends which support the feature.


```cpp
#include <libremidi/backends.hpp>

...

libremidi::midi_in in{
    libremidi::input_configuration{
      .on_message = ...
    },
    libremidi::pipewire_input_configuration{
      .client_name = "My app"
    }
};
```
