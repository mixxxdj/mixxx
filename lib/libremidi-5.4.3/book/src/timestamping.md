# Timestamping

libremidi provides many useful timestamping options for its input callbacks:

```
libremidi::input_configuration conf;

// No timestamping at all, all timestamps are zero
conf.timestamps = libremidi::NoTimestamp;

// In nanoseconds, timestamp is the time since the previous event (or zero)
conf.timestamps = libremidi::Relative;

// In nanoseconds, as per an arbitrary reference which may be provided by the host API,
// e.g. since the JACK cycle start, ALSA sequencer queue creation, through AudioHostTime on macOS.
// It offers the most precise ordering between events as it's the closest to the real timestamp of
// the event as provided by the host API.
// If the API does not provide any timing, it will be mapped to SystemMonotonic instead.
conf.timestamps = libremidi::Absolute;

// In nanoseconds, as per std::steady_clock::now() or equivalent (raw if possible).
// May be less precise than Absolute as timestamping is done within the library,
// but is more useful for system-wide synchronization.
// Note: depending on the backend, Absolute and SystemMonotonic may be the same.
conf.timestamps = libremidi::SystemMonotonic;

// For APIs which are based on audio process cycles such as JACK, timestamps will be in frames since
// the beginning of the current cycle's audio buffer
conf.timestamps = libremidi::AudioFrame;

// Will call the custom timestamping function provided by the user in the input configuration.
// We try to make sure that the input timestamp is as precise as possible ; it is given in the Absolute mode.
conf.get_timestamp = [] (libremidi::timestamp absolute) -> libremidi::timestamp { 
  // Map absolute timestamp to the desired time domain
  return absolute / 1e6;
};
conf.timestamps = libremidi::Custom;
```

For the absolute timestamps, the origin of the timestamp can be obtained with `midi_in::absolute_timestamp()`.
For instance, it will return the time at which the timestamping queue was created in the ALSA back-end.
