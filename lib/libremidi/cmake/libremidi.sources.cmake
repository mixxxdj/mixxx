target_sources(libremidi PRIVATE
    include/libremidi/backends/alsa_raw/config.hpp
    include/libremidi/backends/alsa_raw/helpers.hpp
    include/libremidi/backends/alsa_raw/midi_in.hpp
    include/libremidi/backends/alsa_raw/midi_out.hpp
    include/libremidi/backends/alsa_raw/observer.hpp
    include/libremidi/backends/alsa_raw.hpp

    include/libremidi/backends/alsa_raw_ump/config.hpp
    include/libremidi/backends/alsa_raw_ump/helpers.hpp
    include/libremidi/backends/alsa_raw_ump/midi_in.hpp
    include/libremidi/backends/alsa_raw_ump/midi_out.hpp
    include/libremidi/backends/alsa_raw_ump/observer.hpp
    include/libremidi/backends/alsa_raw_ump.hpp

    include/libremidi/backends/alsa_seq/config.hpp
    include/libremidi/backends/alsa_seq/helpers.hpp
    include/libremidi/backends/alsa_seq/midi_in.hpp
    include/libremidi/backends/alsa_seq/midi_out.hpp
    include/libremidi/backends/alsa_seq/observer.hpp
    include/libremidi/backends/alsa_seq/shared_handler.hpp
    include/libremidi/backends/alsa_seq.hpp

    include/libremidi/backends/alsa_seq_ump/config.hpp
    include/libremidi/backends/alsa_seq_ump/helpers.hpp
    include/libremidi/backends/alsa_seq_ump/midi_out.hpp
    include/libremidi/backends/alsa_seq_ump.hpp

    include/libremidi/backends/coremidi/config.hpp
    include/libremidi/backends/coremidi/helpers.hpp
    include/libremidi/backends/coremidi/midi_in.hpp
    include/libremidi/backends/coremidi/midi_out.hpp
    include/libremidi/backends/coremidi/observer.hpp
    include/libremidi/backends/coremidi.hpp

    include/libremidi/backends/coremidi_ump/config.hpp
    include/libremidi/backends/coremidi_ump/helpers.hpp
    include/libremidi/backends/coremidi_ump/midi_in.hpp
    include/libremidi/backends/coremidi_ump/midi_out.hpp
    include/libremidi/backends/coremidi_ump/observer.hpp
    include/libremidi/backends/coremidi_ump.hpp

    include/libremidi/backends/dummy.hpp

    include/libremidi/backends/emscripten/config.hpp
    include/libremidi/backends/emscripten/helpers.hpp
    include/libremidi/backends/emscripten/midi_access.hpp
    include/libremidi/backends/emscripten/midi_in.hpp
    include/libremidi/backends/emscripten/midi_out.hpp
    include/libremidi/backends/emscripten/observer.hpp
    include/libremidi/backends/emscripten.hpp

    include/libremidi/backends/jack/config.hpp
    include/libremidi/backends/jack/error_domain.hpp
    include/libremidi/backends/jack/helpers.hpp
    include/libremidi/backends/jack/midi_in.hpp
    include/libremidi/backends/jack/midi_out.hpp
    include/libremidi/backends/jack/observer.hpp
    include/libremidi/backends/jack/shared_handler.hpp
    include/libremidi/backends/jack.hpp

    include/libremidi/backends/jack_ump/config.hpp
    include/libremidi/backends/jack_ump/midi_in.hpp
    include/libremidi/backends/jack_ump/midi_out.hpp
    include/libremidi/backends/jack_ump/observer.hpp
    include/libremidi/backends/jack_ump.hpp

    include/libremidi/backends/kdmapi/config.hpp
    include/libremidi/backends/kdmapi/helpers.hpp
    include/libremidi/backends/kdmapi/midi_in.hpp
    include/libremidi/backends/kdmapi/midi_out.hpp
    include/libremidi/backends/kdmapi/observer.hpp
    include/libremidi/backends/kdmapi.hpp

    include/libremidi/backends/keyboard/config.hpp
    include/libremidi/backends/keyboard/midi_in.hpp

    include/libremidi/backends/linux/alsa.hpp
    include/libremidi/backends/linux/dylib_loader.hpp
    include/libremidi/backends/linux/helpers.hpp
    include/libremidi/backends/linux/pipewire.hpp
    include/libremidi/backends/linux/udev.hpp

    include/libremidi/backends/net/config.hpp
    include/libremidi/backends/net/helpers.hpp
    include/libremidi/backends/net/midi_in.hpp
    include/libremidi/backends/net/midi_out.hpp
    include/libremidi/backends/net/observer.hpp

    include/libremidi/backends/pipewire/config.hpp
    include/libremidi/backends/pipewire/context.hpp
    include/libremidi/backends/pipewire/helpers.hpp
    include/libremidi/backends/pipewire/midi_in.hpp
    include/libremidi/backends/pipewire/midi_out.hpp
    include/libremidi/backends/pipewire/observer.hpp
    include/libremidi/backends/pipewire/shared_handler.hpp
    include/libremidi/backends/pipewire.hpp
    include/libremidi/backends/pipewire_ump.hpp

    include/libremidi/backends/winmidi/config.hpp
    include/libremidi/backends/winmidi/helpers.hpp
    include/libremidi/backends/winmidi/midi_in.hpp
    include/libremidi/backends/winmidi/midi_out.hpp
    include/libremidi/backends/winmidi/observer.hpp
    include/libremidi/backends/winmidi.hpp

    include/libremidi/backends/winmm/config.hpp
    include/libremidi/backends/winmm/helpers.hpp
    include/libremidi/backends/winmm/midi_in.hpp
    include/libremidi/backends/winmm/midi_out.hpp
    include/libremidi/backends/winmm/observer.hpp
    include/libremidi/backends/winmm.hpp

    include/libremidi/backends/winuwp/config.hpp
    include/libremidi/backends/winuwp/helpers.hpp
    include/libremidi/backends/winuwp/midi_in.hpp
    include/libremidi/backends/winuwp/midi_out.hpp
    include/libremidi/backends/winuwp/observer.hpp
    include/libremidi/backends/winuwp.hpp

    include/libremidi/detail/conversion.hpp
    include/libremidi/detail/memory.hpp
    include/libremidi/detail/midi_api.hpp
    include/libremidi/detail/midi_in.hpp
    include/libremidi/detail/midi_out.hpp
    include/libremidi/detail/midi_stream_decoder.hpp
    include/libremidi/detail/observer.hpp
    include/libremidi/detail/semaphore.hpp
    include/libremidi/detail/ump_stream.hpp

    include/libremidi/api.hpp
    # include/libremidi/client.cpp
    # include/libremidi/client.hpp
    include/libremidi/config.hpp
    include/libremidi/configurations.hpp
    include/libremidi/error.hpp
    include/libremidi/error_handler.hpp
    include/libremidi/input_configuration.hpp
    include/libremidi/libremidi.hpp
    include/libremidi/message.hpp
    include/libremidi/port_comparison.hpp
    include/libremidi/port_information.hpp
    include/libremidi/output_configuration.hpp
    include/libremidi/ump_events.hpp

    include/libremidi/reader.hpp
    include/libremidi/writer.hpp

    include/libremidi/api-c.h
    include/libremidi/libremidi-c.h
)

if(NOT LIBREMIDI_HEADER_ONLY AND NOT LIBREMIDI_MODULE_BUILD)
  target_sources(libremidi PRIVATE
    include/libremidi/libremidi.cpp
    include/libremidi/midi_in.cpp
    include/libremidi/midi_out.cpp
    include/libremidi/observer.cpp
    include/libremidi/reader.cpp
    include/libremidi/writer.cpp

    include/libremidi/backends/emscripten/midi_access.cpp
    include/libremidi/backends/emscripten/midi_in.cpp
    include/libremidi/backends/emscripten/midi_out.cpp
    include/libremidi/backends/emscripten/observer.cpp

    include/libremidi/libremidi-c.cpp
  )
endif()
