#if defined(__EMSCRIPTEN__)
  #include <libremidi/backends/emscripten/midi_access.hpp>

extern "C" {
LIBREMIDI_EXPORT
LIBREMIDI_INLINE
EMSCRIPTEN_KEEPALIVE
void libremidi_devices_poll()
{
  libremidi::webmidi_helpers::midi_access_emscripten::instance().devices_poll();
}

LIBREMIDI_EXPORT
LIBREMIDI_INLINE
EMSCRIPTEN_KEEPALIVE
void libremidi_devices_input(int port, double timestamp, int len, char* bytes)
{
  libremidi::webmidi_helpers::midi_access_emscripten::instance().devices_input(
      port, timestamp, len, bytes);
}
}
#endif
