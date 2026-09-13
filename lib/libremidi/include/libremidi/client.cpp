#if !defined(LIBREMIDI_HEADER_ONLY)
  #include "client.hpp"
#endif
#include <libremidi/backends.hpp>
#include <libremidi/shared_context.hpp>

#ifdef LIBREMIDI_ALSA
  #include <libremidi/backends/alsa_seq/shared_handler.hpp>
#endif
#ifdef LIBREMIDI_JACK
  #include <libremidi/backends/jack/shared_handler.hpp>
#endif
NAMESPACE_LIBREMIDI
{
LIBREMIDI_INLINE
shared_configurations
create_shared_context(const libremidi::API api, [[maybe_unused]] std::string_view client_name)
{
  switch (api)
  {
#if __has_include(<boost/lockfree/spsc_queue.hpp>)
  #if defined(LIBREMIDI_ALSA)
    case libremidi::API::ALSA_SEQ:
      return alsa_seq::shared_handler::make(client_name);
  #endif

  #if defined(LIBREMIDI_JACK)
    case libremidi::API::JACK_MIDI:
      return jack::shared_handler::make(client_name);
  #endif
#endif
    case libremidi::API::COREMIDI:
    case libremidi::API::COREMIDI_UMP:
      // TODO

    case libremidi::API::WINDOWS_MIDI_SERVICES:
      // TODO

    default:
      return {
          .context = {},
          .observer = observer_configuration_for(api),
          .in = midi_in_configuration_for(api),
          .out = midi_out_configuration_for(api)};
  }
}
}
