if(NOT EMSCRIPTEN)
  return()
endif()

message(STATUS "libremidi: using Emscripten MIDI")
set(LIBREMIDI_HAS_EMSCRIPTEN 1)

set(CMAKE_EXECUTABLE_SUFFIX .html)
target_compile_definitions(libremidi ${_public} LIBREMIDI_EMSCRIPTEN)
target_link_options(libremidi ${_public} "SHELL:-s 'EXPORTED_FUNCTIONS=[\"_main\", \"_free\", \"_libremidi_devices_poll\", \"_libremidi_devices_input\"]'")
