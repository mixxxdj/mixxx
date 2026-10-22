#pragma once

#include <libremidi/backends/linux/dylib_loader.hpp>

#include <jack/jack.h>
#include <jack/midiport.h>
#include <jack/ringbuffer.h>

NAMESPACE_LIBREMIDI
{

struct libjack
{
  static dylib_loader load_library()
  {
#if defined(_WIN64)
    dylib_loader lib{"libjack64.dll"};
    if (!lib)
      lib = dylib_loader{"libjack.dll"};
    return lib;
#elif defined(_WIN32)
    return dylib_loader{"libjack.dll"};
#elif defined(__APPLE__)
    return dylib_loader{"libjack.0.dylib"};
#else
    return dylib_loader{"libjack.so.0"};
#endif
  }

  explicit libjack()
      : library{load_library()}
  {
    if (!library)
    {
      available = false;
      return;
    }

    // Version
    LIBREMIDI_SYMBOL_INIT(jack, get_version)

    // Client
    LIBREMIDI_SYMBOL_INIT(jack, client_open)
    LIBREMIDI_SYMBOL_INIT(jack, client_close)
    LIBREMIDI_SYMBOL_INIT(jack, activate)
    LIBREMIDI_SYMBOL_INIT(jack, deactivate)
    LIBREMIDI_SYMBOL_INIT(jack, get_client_name)
    LIBREMIDI_SYMBOL_INIT(jack, on_shutdown)

    // Port query & connection
    LIBREMIDI_SYMBOL_INIT(jack, get_ports)
    LIBREMIDI_SYMBOL_INIT(jack, connect)
    LIBREMIDI_SYMBOL_INIT(jack, free)

    // Callbacks
    LIBREMIDI_SYMBOL_INIT(jack, set_process_callback)
    LIBREMIDI_SYMBOL_INIT(jack, set_sample_rate_callback)
    LIBREMIDI_SYMBOL_INIT(jack, set_port_registration_callback)
    LIBREMIDI_SYMBOL_INIT(jack, set_port_rename_callback)
    LIBREMIDI_SYMBOL_INIT(jack, set_timebase_callback)
    LIBREMIDI_SYMBOL_INIT(jack, set_sync_callback)

    // Timing
    LIBREMIDI_SYMBOL_INIT(jack, get_sample_rate)
    LIBREMIDI_SYMBOL_INIT(jack, get_buffer_size)
    LIBREMIDI_SYMBOL_INIT(jack, frame_time)
    LIBREMIDI_SYMBOL_INIT(jack, frames_to_time)
    LIBREMIDI_SYMBOL_INIT(jack, get_cycle_times)

    // Transport
    LIBREMIDI_SYMBOL_INIT(jack, transport_query)
  }

  static const libjack& instance()
  {
    static const libjack self;
    return self;
  }

  dylib_loader library;
  bool available{true};

  // Version
  LIBREMIDI_SYMBOL_DEF(jack, get_version)

  // Client
  LIBREMIDI_SYMBOL_DEF(jack, client_open)
  LIBREMIDI_SYMBOL_DEF(jack, client_close)
  LIBREMIDI_SYMBOL_DEF(jack, activate)
  LIBREMIDI_SYMBOL_DEF(jack, deactivate)
  LIBREMIDI_SYMBOL_DEF(jack, get_client_name)
  LIBREMIDI_SYMBOL_DEF(jack, on_shutdown)

  // Port query & connection
  LIBREMIDI_SYMBOL_DEF(jack, get_ports)
  LIBREMIDI_SYMBOL_DEF(jack, connect)
  LIBREMIDI_SYMBOL_DEF(jack, free)

  // Callbacks
  LIBREMIDI_SYMBOL_DEF(jack, set_process_callback)
  LIBREMIDI_SYMBOL_DEF(jack, set_sample_rate_callback)
  LIBREMIDI_SYMBOL_DEF(jack, set_port_registration_callback)
  LIBREMIDI_SYMBOL_DEF(jack, set_port_rename_callback)
  LIBREMIDI_SYMBOL_DEF(jack, set_timebase_callback)
  LIBREMIDI_SYMBOL_DEF(jack, set_sync_callback)

  // Timing
  LIBREMIDI_SYMBOL_DEF(jack, get_sample_rate)
  LIBREMIDI_SYMBOL_DEF(jack, get_buffer_size)
  LIBREMIDI_SYMBOL_DEF(jack, frame_time)
  LIBREMIDI_SYMBOL_DEF(jack, frames_to_time)
  LIBREMIDI_SYMBOL_DEF(jack, get_cycle_times)

  // Transport
  LIBREMIDI_SYMBOL_DEF(jack, transport_query)

  struct port_t
  {
    explicit port_t(const dylib_loader& library)
    {
      if (!library)
      {
        available = false;
        return;
      }

      LIBREMIDI_SYMBOL_INIT2(jack_port, register, register_port)
      LIBREMIDI_SYMBOL_INIT(jack_port, unregister)
      LIBREMIDI_SYMBOL_INIT(jack_port, by_name)
      LIBREMIDI_SYMBOL_INIT(jack_port, by_id)
      LIBREMIDI_SYMBOL_INIT(jack_port, name)
      LIBREMIDI_SYMBOL_INIT(jack_port, short_name)
      LIBREMIDI_SYMBOL_INIT(jack_port, type)
      LIBREMIDI_SYMBOL_INIT(jack_port, flags)
      LIBREMIDI_SYMBOL_INIT(jack_port, rename)
      LIBREMIDI_SYMBOL_INIT(jack_port, get_buffer)
      LIBREMIDI_SYMBOL_INIT(jack_port, get_aliases)
      LIBREMIDI_SYMBOL_INIT(jack_port, name_size)
    }
    bool available{true};

    LIBREMIDI_SYMBOL_DEF2(jack_port, register, register_port)
    LIBREMIDI_SYMBOL_DEF(jack_port, unregister)
    LIBREMIDI_SYMBOL_DEF(jack_port, by_name)
    LIBREMIDI_SYMBOL_DEF(jack_port, by_id)
    LIBREMIDI_SYMBOL_DEF(jack_port, name)
    LIBREMIDI_SYMBOL_DEF(jack_port, short_name)
    LIBREMIDI_SYMBOL_DEF(jack_port, type)
    LIBREMIDI_SYMBOL_DEF(jack_port, flags)
    LIBREMIDI_SYMBOL_DEF(jack_port, rename)
    LIBREMIDI_SYMBOL_DEF(jack_port, get_buffer)
    LIBREMIDI_SYMBOL_DEF(jack_port, get_aliases)
    LIBREMIDI_SYMBOL_DEF(jack_port, name_size)
  } port{library};

  struct midi_t
  {
    explicit midi_t(const dylib_loader& library)
    {
      if (!library)
      {
        available = false;
        return;
      }

      LIBREMIDI_SYMBOL_INIT(jack_midi, get_event_count)
      LIBREMIDI_SYMBOL_INIT(jack_midi, event_get)
      LIBREMIDI_SYMBOL_INIT(jack_midi, event_write)
      LIBREMIDI_SYMBOL_INIT(jack_midi, event_reserve)
      LIBREMIDI_SYMBOL_INIT(jack_midi, clear_buffer)
    }
    bool available{true};

    LIBREMIDI_SYMBOL_DEF(jack_midi, get_event_count)
    LIBREMIDI_SYMBOL_DEF(jack_midi, event_get)
    LIBREMIDI_SYMBOL_DEF(jack_midi, event_write)
    LIBREMIDI_SYMBOL_DEF(jack_midi, event_reserve)
    LIBREMIDI_SYMBOL_DEF(jack_midi, clear_buffer)
  } midi{library};

  struct ringbuffer_t
  {
    explicit ringbuffer_t(const dylib_loader& library)
    {
      if (!library)
      {
        available = false;
        return;
      }

      LIBREMIDI_SYMBOL_INIT(jack_ringbuffer, create)
      LIBREMIDI_SYMBOL_INIT(jack_ringbuffer, free)
      LIBREMIDI_SYMBOL_INIT(jack_ringbuffer, write)
      LIBREMIDI_SYMBOL_INIT(jack_ringbuffer, read)
      LIBREMIDI_SYMBOL_INIT(jack_ringbuffer, peek)
      LIBREMIDI_SYMBOL_INIT(jack_ringbuffer, write_space)
      LIBREMIDI_SYMBOL_INIT(jack_ringbuffer, read_space)
      LIBREMIDI_SYMBOL_INIT(jack_ringbuffer, read_advance)
    }
    bool available{true};

    LIBREMIDI_SYMBOL_DEF(jack_ringbuffer, create)
    LIBREMIDI_SYMBOL_DEF(jack_ringbuffer, free)
    LIBREMIDI_SYMBOL_DEF(jack_ringbuffer, write)
    LIBREMIDI_SYMBOL_DEF(jack_ringbuffer, read)
    LIBREMIDI_SYMBOL_DEF(jack_ringbuffer, peek)
    LIBREMIDI_SYMBOL_DEF(jack_ringbuffer, write_space)
    LIBREMIDI_SYMBOL_DEF(jack_ringbuffer, read_space)
    LIBREMIDI_SYMBOL_DEF(jack_ringbuffer, read_advance)
  } ringbuffer{library};
};

}
