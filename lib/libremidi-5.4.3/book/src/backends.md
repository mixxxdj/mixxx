# Feature matrix

This table shows which feature is supported by which backend so far, for advanced features.

It may be because the backend does not provide the ability at all (N/A), 
or because it has not been implemented yet.


## Linux & BSD

|               | ALSA Raw | ALSA Seq | PipeWire |
|---------------|----------|----------|----------|
| MIDI 1        | Yes      | Yes      | Yes      |
| MIDI 2        | Yes      | Yes      | N/A      |
| Virtual ports | N/A      | Yes      | Yes      |
| Observer      | Yes      | Yes      | Yes      |
| Scheduling    | No       | No       | No       |


### Special features
- The ALSA Raw back-end allows to perform chunked sending of MIDI messages, 
which can be useful to upload firmwares.

- `libasound` and `libpipewire` are always loaded through `dlopen`.
JACK can also be, optionally.

This allows libremidi to be built on a system with e.g. PipeWire support 
without preventing application loading if the end user does not use it.

## Windows 

|               | WinMM | UWP | WinMIDI |
|---------------|-------|-----|---------|
| MIDI 1        | Yes   | Yes | No      |
| MIDI 2        | N/A   | N/A | Yes     |
| Virtual ports | N/A   | No  | No      |
| Observer      | Yes   | Yes | Yes     |
| Scheduling    | No    | No  | No      |

## Mac & iOS

|               | CoreMIDI |
|---------------|----------|
| MIDI 1        | Yes      |
| MIDI 2        | Yes      |
| Virtual ports | Yes      |
| Observer      | Yes      |
| Scheduling    | No       |

## Web

|               | Emscripten WebMIDI |
|---------------|--------------------|
| MIDI 1        | Yes                |
| MIDI 2        | N/A                |
| Virtual ports | N/A                |
| Observer      | Yes                |
| Scheduling    | No                 |

## Shared backends

|               | JACK |
|---------------|------|
| MIDI 1        | Yes  |
| MIDI 2        | N/A  |
| Virtual ports | Yes  |
| Observer      | Yes  |
| Scheduling    | No   |
