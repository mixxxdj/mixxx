cmake_minimum_required(VERSION 3.22..4.2.0 FATAL_ERROR)
project(libremidi
  VERSION 5.4.2
  DESCRIPTION "A cross-platform MIDI library"
  LANGUAGES C CXX
  HOMEPAGE_URL "https://github.com/jcelerier/libremidi"
)

# Still support BOOST_ROOT
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 OLD)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CMakeDependentOption)
include(CheckSymbolExists)
include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include(CheckIncludeFileCXX)
include(FetchContent)
include(GNUInstallDirs)

### Options ###
option(LIBREMIDI_MODULE_BUILD "C++ module mode" OFF)
option(LIBREMIDI_HEADER_ONLY "Header-only mode" OFF)
option(LIBREMIDI_LIBRARY_MODE "Library mode: LIBRARY,HEADER_ONLY,MODULE" "")
if("${LIBREMIDI_LIBRARY_MODE}" MATCHES "HEADER_ONLY")
  set(LIBREMIDI_HEADER_ONLY ON)
  set(LIBREMIDI_MODULE_BUILD OFF)
elseif("${LIBREMIDI_LIBRARY_MODE}" MATCHES "MODULE")
  set(CMAKE_CXX_SCAN_FOR_MODULES ON)
  set(LIBREMIDI_HEADER_ONLY OFF)
  set(LIBREMIDI_MODULE_BUILD ON)
  set(LIBREMIDI_TESTS OFF)
elseif("${LIBREMIDI_LIBRARY_MODE}" MATCHES "LIBRARY")
  set(LIBREMIDI_HEADER_ONLY OFF)
  set(LIBREMIDI_MODULE_BUILD OFF)
endif()

cmake_dependent_option(LIBREMIDI_NO_COREMIDI "Disable CoreMidi back-end" OFF "APPLE" OFF)
cmake_dependent_option(LIBREMIDI_NO_WINMM "Disable WinMM back-end" OFF "WIN32" OFF)
cmake_dependent_option(LIBREMIDI_NO_WINUWP "Disable UWP back-end" OFF "WIN32" OFF)
cmake_dependent_option(LIBREMIDI_NO_WINMIDI "Disable WinMIDI back-end" OFF "WIN32" OFF)
# if(LINUX) in CMake 3.25
cmake_dependent_option(LIBREMIDI_NO_ALSA "Disable ALSA back-end" OFF "UNIX; NOT APPLE" OFF)
cmake_dependent_option(LIBREMIDI_NO_UDEV "Disable udev support for ALSA" OFF "UNIX; NOT APPLE" OFF)
cmake_dependent_option(LIBREMIDI_DOWNLOAD_CPPWINRT "Download cppwinrt and MIDI2 nupkgs" ON "WIN32" OFF)

# https://github.com/microsoft/MIDI/releases/download/dev-preview-9-namm-4/Microsoft.Windows.Devices.Midi2.1.0.2-preview-9.250121-1820.nupkg renamed to .zip
cmake_dependent_option(LIBREMIDI_WINMIDI_HEADERS_ZIP "Path to the WinMIDI headers archive" "" "WIN32" "")

option(LIBREMIDI_NO_JACK "Disable JACK back-end" OFF)
option(LIBREMIDI_NO_PIPEWIRE "Disable PipeWire back-end" OFF)
option(LIBREMIDI_NO_NETWORK "Disable Network back-end" OFF)
option(LIBREMIDI_NO_KEYBOARD "Disable Computer keyboard back-end" OFF)
cmake_dependent_option(LIBREMIDI_NO_ANDROID "Disable Android AMidi back-end" OFF "ANDROID" OFF)

option(LIBREMIDI_NO_EXPORTS "Disable dynamic symbol exporting" OFF)
option(LIBREMIDI_NO_BOOST "Do not use Boost if available" OFF)
option(LIBREMIDI_SLIM_MESSAGE "Use a fixed-size message format" 0)
option(LIBREMIDI_FIND_BOOST "Actively look for Boost" OFF)
option(LIBREMIDI_EXAMPLES "Enable examples" OFF)
option(LIBREMIDI_TESTS "Enable tests" OFF)
option(LIBREMIDI_NI_MIDI2 "Enable compatibility with ni-midi2" OFF)
option(LIBREMIDI_CI "To be enabled only in CI, some tests cannot run there. Also enables -Werror." OFF)

cmake_dependent_option(LIBREMIDI_NO_WARNINGS "Disables warnings from library compilation" OFF "NOT LIBREMIDI_HEADER_ONLY" ON)

### Basic setup ###
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT CMAKE_CXX_STANDARD OR ("${CMAKE_CXX_STANDARD}" VERSION_LESS 20))
  set(CMAKE_CXX_STANDARD 20)
endif()

### Dependencies ###
include(libremidi.deps)

### Main library ###
include(libremidi.library)

### Backends ###
include(libremidi.emscripten)
include(libremidi.win32)
include(libremidi.macos)
include(libremidi.alsa)
include(libremidi.jack)
include(libremidi.pipewire)
include(libremidi.keyboard)
include(libremidi.net)
include(libremidi.android)

### Install ###
include(libremidi.install)

### Examples ###
if(LIBREMIDI_EXAMPLES)
  message(STATUS "libremidi: compiling examples")
  include(libremidi.examples)
endif()

### Tests ###
if(LIBREMIDI_TESTS)
  message(STATUS "libremidi: compiling tests")
  include(libremidi.tests)
endif()
