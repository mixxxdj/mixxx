if(NOT LIBREMIDI_MODULE_BUILD)
macro(setup_example _example)
  target_link_libraries("${_example}" PRIVATE libremidi)
endmacro()

macro(add_example _example)
  string(REPLACE "/" "_" _exampleName "${_example}")
  add_executable("${_exampleName}" "examples/${_example}.cpp")
  setup_example("${_exampleName}")
endmacro()

macro(add_backend_example _example)
    add_executable("${_example}" "examples/backends/${_example}.cpp")
    setup_example("${_example}")
endmacro()

add_example(midiobserve)
add_example(echo)
add_example(cmidiin)
add_example(cmidiin2)
add_example(lookup)
add_example(midi1_to_midi2)
add_example(midiclock_in)
add_example(midiclock_out)
add_example(midiout)
# add_example(client)
add_example(midiprobe)
add_example(qmidiin)
add_example(sysextest)
add_example(minimal)
add_example(midi2_echo)
add_example(rawmidiin)

if(LIBREMIDI_HAS_STD_FLAT_SET AND LIBREMIDI_HAS_STD_PRINTLN)
  add_example(midi_to_pattern)
endif()

add_example(protocols/remote_control)

if(LIBREMIDI_NI_MIDI2)
  add_example(midi2_interop)
endif()

if(LIBREMIDI_HAS_STD_JTHREAD)
  add_example(multithread_midiout)
endif()

if(LIBREMIDI_HAS_ALSA)
  add_example(poll_share)
  target_link_libraries(poll_share PRIVATE ${ALSA_LIBRARIES})

  add_example(alsa_share)
  target_link_libraries(alsa_share PRIVATE ${ALSA_LIBRARIES})

  add_backend_example(midi1_in_alsa_seq)
  add_backend_example(midi1_out_alsa_seq)

  if(LIBREMIDI_HAS_ALSA_RAWMIDI)
    add_backend_example(midi1_in_alsa_rawmidi)
    add_backend_example(midi1_out_alsa_rawmidi)
  endif()

  if(LIBREMIDI_HAS_ALSA_UMP)
    add_backend_example(midi2_in_alsa_rawmidi)
    add_backend_example(midi2_in_alsa_seq)
    add_backend_example(midi2_out_alsa_rawmidi)
    add_backend_example(midi2_out_alsa_seq)
  endif()
endif()

if(LIBREMIDI_HAS_JACK)
    add_example(jack_share)
    add_backend_example(midi2_in_jack)
    add_backend_example(midi2_out_jack)
endif()

if(LIBREMIDI_HAS_PIPEWIRE)
    add_example(pipewire_share)
    add_backend_example(midi1_in_pipewire)
    add_backend_example(midi1_out_pipewire)

    if(LIBREMIDI_HAS_PIPEWIRE_UMP)
      add_backend_example(midi2_in_pipewire)
      add_backend_example(midi2_out_pipewire)
    endif()
endif()

if(LIBREMIDI_HAS_COREMIDI)
    add_example(coremidi_share)
endif()

if(LIBREMIDI_HAS_EMSCRIPTEN)
    add_example(emscripten_midiin)
endif()

if(LIBREMIDI_HAS_WINMIDI)
  add_backend_example(midi2_in_winmidi)
  add_backend_example(midi2_out_winmidi)
endif()

if(LIBREMIDI_HAS_KDMAPI)
  add_backend_example(midi1_out_kdmapi)
endif()

if(LIBREMIDI_HAS_NETWORK)
  add_example(network)
endif()

if(Boost_cobalt_FOUND)
  add_example(coroutines)
  target_link_libraries(coroutines PRIVATE Boost::cobalt)
endif()

add_executable(libremidi_c_api examples/c_api.c)
target_link_libraries(libremidi_c_api PRIVATE libremidi)
if(LIBREMIDI_HEADER_ONLY)
  target_sources(libremidi_c_api PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/libremidi/libremidi-c.cpp")
endif()
endif()

if(LIBREMIDI_MODULE_BUILD)
  add_executable(libremidi_modules
    examples/modules.cpp
  )
  target_link_libraries(libremidi_modules PRIVATE libremidi)
endif()
