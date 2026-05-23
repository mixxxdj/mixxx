if(LIBREMIDI_NO_KDMAPI)
  return()
endif()

if(NOT WIN32)
  return()
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES WindowsStore)
  return()
endif()

message(STATUS "libremidi: using KDMAPI (OmniMIDI)")
set(LIBREMIDI_HAS_KDMAPI 1)
target_compile_definitions(libremidi
  ${_public}
    LIBREMIDI_KDMAPI
)
