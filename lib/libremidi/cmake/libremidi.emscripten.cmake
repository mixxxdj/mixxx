if(NOT EMSCRIPTEN)
  return()
endif()

message(STATUS "libremidi: using Emscripten MIDI")
set(LIBREMIDI_HAS_EMSCRIPTEN 1)

set(CMAKE_EXECUTABLE_SUFFIX .html)
target_compile_definitions(libremidi ${_public} LIBREMIDI_EMSCRIPTEN)
# The WebMIDI backend calls Module._malloc (and _free) from JS to copy incoming
# message bytes onto the heap, and uses the HEAPU8 / UTF8ToString / stringToUTF8
# / lengthBytesUTF8 runtime helpers. Export all of them, otherwise receiving a
# message throws e.g. "Module._malloc is not a function".
target_link_options(libremidi ${_public} "SHELL:-s 'EXPORTED_FUNCTIONS=[\"_main\", \"_malloc\", \"_free\", \"_libremidi_devices_poll\", \"_libremidi_devices_input\"]'")
target_link_options(libremidi ${_public} "SHELL:-s 'EXPORTED_RUNTIME_METHODS=[\"HEAPU8\", \"UTF8ToString\", \"stringToUTF8\", \"lengthBytesUTF8\"]'")
