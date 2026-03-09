# Custom polling

Traditionnally, RtMidi (and most other MIDI libraries) opened a dedicated MIDI thread from which messages are read, and then transferred to the rest of the app.

If your app is based on an event loop that can poll file descriptors, such as `poll()`, the relevant back-ends will allow to instead control polling manually by providing you with the file descriptors, 
which can then be injected into your app's main loop. 

Thus, this enables complete control over threading (and can also remove the need for synchronisation as this allows to make a callback-based yet single-threaded app, for simple applications which do not wish to reimplement MIDI filtering & parsing for back-ends such as ALSA Sequencer or RawMidi).

Since this feature is pretty complicated to implement correctly unless you already have an easily accessible `poll` loop in your app, we recommend checking the examples: 

See:
- `poll_share.cpp` for a complete example for ALSA RawMidi (recommended).
- `alsa_share.cpp` for a complete example for ALSA Seq.