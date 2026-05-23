if(LIBREMIDI_NO_WINMIDI)
  return()
endif()

if(LIBREMIDI_DOWNLOAD_CPPWINRT)
  if(NOT EXISTS "${CMAKE_BINARY_DIR}/winmidi-headers.zip")
    file(DOWNLOAD
      https://github.com/microsoft/MIDI/releases/download/rc-1/Microsoft.Windows.Devices.Midi2.1.0.14-rc.1.209.nupkg
      "${CMAKE_BINARY_DIR}/winmidi-headers.zip"
    )
  endif()
  set(LIBREMIDI_WINMIDI_HEADERS_ZIP "${CMAKE_BINARY_DIR}/winmidi-headers.zip")
endif()

if(NOT LIBREMIDI_WINMIDI_HEADERS_ZIP)
  return()
endif()

file(ARCHIVE_EXTRACT
  INPUT "${LIBREMIDI_WINMIDI_HEADERS_ZIP}"
  DESTINATION "${CMAKE_BINARY_DIR}/winmidi-headers/"
)

file(MAKE_DIRECTORY
  "${CMAKE_BINARY_DIR}/cppwinrt/"
)

file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/cppwinrt-winmidi/")

if(CPPWINRT_TOOL)
  # Enumerate winmd IDL files and store them in a response file
  file(TO_CMAKE_PATH "${CMAKE_WINDOWS_KITS_10_DIR}/References/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}" winsdk)
  file(GLOB winmds "${winsdk}/*/*/*.winmd")

  set(args "")
  string(APPEND args "-input \"${CMAKE_BINARY_DIR}/winmidi-headers/ref/native/Microsoft.Windows.Devices.Midi2.winmd\"\n")

  foreach(winmd IN LISTS winmds)
    string(APPEND args "-ref \"${winmd}\"\n")
  endforeach()

  # Recreate the sources
  file(WRITE "${CMAKE_BINARY_DIR}/cppwinrt-src/cppwinrt-winmidi.rsp" "${args}")

  execute_process(
    COMMAND "${CPPWINRT_TOOL}"
      "@${CMAKE_BINARY_DIR}/cppwinrt-src/cppwinrt-winmidi.rsp"
      -output "${CMAKE_BINARY_DIR}/cppwinrt-winmidi"
  )

  file(
    COPY
      "${CMAKE_BINARY_DIR}/winmidi-headers/build/native/include/winmidi/init"
      "${CMAKE_BINARY_DIR}/winmidi-headers/build/native/include/winmidi/WindowsMidiServicesAppSdkComExtensions.h"
    DESTINATION
      "${CMAKE_BINARY_DIR}/cppwinrt-winmidi/"
  )
else()
  # In case we don't have cppwinrt we can still try to just use the SDK headers directly
  file(
    COPY
      "${CMAKE_BINARY_DIR}/winmidi-headers/build/native/include/winmidi"
    DESTINATION
      "${CMAKE_BINARY_DIR}/cppwinrt-winmidi/"
  )
endif()

message(STATUS "libremidi: using Windows MIDI Services")
set(LIBREMIDI_HAS_WINMIDI 1)

target_include_directories(libremidi SYSTEM ${_public}
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/cppwinrt>
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/cppwinrt-winmidi>
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/cppwinrt-winmidi/winmidi>
)
target_compile_definitions(libremidi ${_public} LIBREMIDI_WINMIDI)
target_link_libraries(libremidi ${_public} RuntimeObject windowsapp)
