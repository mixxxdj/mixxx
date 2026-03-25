# Computer keyboard input

This backend allwos to use the computer keys to play MIDI.
The backend does not directly read the key events as this would require making the library much more complex.
Instead, it provides a callback that you can plug into your favourite GUI toolkit to process scan codes.

The mapping is customizable. By default:

```
 ,---,---,---,---,---,---,---,---,---,---,---,---,---,-------,
 | V0| V1| V2| V3| V4| V5| V6| V7| V8| V9|V10|V11|V12| <-    |
 |---'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-----|
 | ->| |   | C#| D#|   | F#| G#| A#|   | C#| D#|   | F#|     |
 |-----',--',--',--',--',--',--',--',--',--',--',--',--'|    |
 | Caps | C | D | E | F | G | A | B | C | D | E | F | G |    |
 |----,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'---'----|
 | -^ |   | O-| O+| V-| V+|   |   |   |   |   |   |   ----^  |
 |----'-,-',--'--,'---'---'---'---'---'---'-,-'---',--,------|
 | ctrl |  | alt |                          |altgr |  | ctrl |
 '------'  '-----'--------------------------'------'  '------'
```

Where V0 to V12 set the velocity between 0 and 127 in steps of ~10, O- / O+ increase or decrease the octave and V- / V+ increase or decrease the velocity by 10.


Example:
```
#include <libremidi/backends/keyboard/config.hpp>

libremidi::kbd_input_configuration api_conf;
api_conf.set_input_scancode_callbacks(
[] )
