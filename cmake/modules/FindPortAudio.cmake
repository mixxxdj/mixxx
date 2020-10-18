# - Try to find Portaudio
# Once done this will define
#
#  PortAudio_FOUND - system has Portaudio
#  PortAudio_INCLUDE_DIRS - the Portaudio include directory
#  PortAudio_LIBRARIES - Link these to use Portaudio

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_PortAudio portaudio-2.0)
endif()

find_path(PortAudio_INCLUDE_DIRS
  NAMES
    portaudio.h
  PATHS
      /usr/local/include
      /usr/include
  HINTS
    ${PC_PortAudio_INCLUDEDIR}
)

find_library(PortAudio_LIBRARIES
  NAMES
    portaudio
  PATHS
      /usr/local/lib
      /usr/lib
      /usr/lib64
  HINTS
    ${PC_PortAudio_LIBDIR}
)

mark_as_advanced(PortAudio_INCLUDE_DIRS PortAudio_LIBRARIES)

# Found PortAudio, but it may be version 18 which is not acceptable.
if(EXISTS ${PortAudio_INCLUDE_DIRS}/portaudio.h)
  include(CheckCXXSourceCompiles)
  set(CMAKE_REQUIRED_INCLUDES_SAVED ${CMAKE_REQUIRED_INCLUDES})
  set(CMAKE_REQUIRED_INCLUDES ${PortAudio_INCLUDE_DIRS})
  CHECK_CXX_SOURCE_COMPILES(
    "#include <portaudio.h>\nPaDeviceIndex pa_find_device_by_name(const char *name); int main () {return 0;}"
    PortAudio2_FOUND)
  set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES_SAVED})
  unset(CMAKE_REQUIRED_INCLUDES_SAVED)
  if(PortAudio2_FOUND)
    INCLUDE(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(PortAudio DEFAULT_MSG PortAudio_INCLUDE_DIRS PortAudio_LIBRARIES)
  else(PortAudio2_FOUND)
    message(STATUS
      "  portaudio.h not compatible (requires API 2.0)")
    set(PortAudio_FOUND FALSE)
  endif(PortAudio2_FOUND)
endif()
