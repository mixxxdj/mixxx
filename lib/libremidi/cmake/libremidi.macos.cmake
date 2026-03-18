if(NOT APPLE)
  return()
endif()
if(LIBREMIDI_NO_COREMIDI)
  return()
endif()

message(STATUS "libremidi: using CoreMIDI")
find_library(COREMIDI_LIBRARY CoreMIDI)
find_library(COREAUDIO_LIBRARY CoreAudio)
find_library(COREFOUNDATION_LIBRARY CoreFoundation)

set(LIBREMIDI_HAS_COREMIDI 1)

target_compile_definitions(libremidi ${_public} LIBREMIDI_COREMIDI)

target_link_libraries(libremidi
  ${_public}
    ${COREFOUNDATION_LIBRARY}
    ${COREAUDIO_LIBRARY}
    ${COREMIDI_LIBRARY}
 )
