#.rst:
# FindFFmpeg
# ----------
#
# Try to find the required ffmpeg components (default: libavformat, libavutil, libavcodec)
#
# Next variables can be used to hint FFmpeg libs search:
#
# ::
#
#   PC_<component>_LIBRARY_DIRS
#   PC_FFmpeg_LIBRARY_DIRS
#   PC_<component>_INCLUDE_DIRS
#   PC_FFmpeg_INCLUDE_DIRS
#
# Once done this will define
#
# ::
#
#   FFmpeg_FOUND         - System has the all required components.
#   FFmpeg_INCLUDE_DIRS  - Include directory necessary for using the required components headers.
#   FFmpeg_LIBRARIES     - Link these to use the required ffmpeg components.
#   FFmpeg_DEFINITIONS   - Compiler switches required for using the required ffmpeg components.
#
# For each of the components it will additionally set.
#
# ::
#
#   libavcodec
#   libavdevice
#   libavformat
#   libavfilter
#   libavutil
#   libswscale
#   libswresample
#
# the following variables will be defined
#
# ::
#
#   <component>_FOUND        - System has <component>
#   <component>_INCLUDE_DIRS - Include directory necessary for using the <component> headers
#   <component>_LIBRARIES    - Link these to use <component>
#   <component>_DEFINITIONS  - Compiler switches required for using <component>
#   <component>_VERSION      - The components version
#
# the following import targets is created
#
# ::
#
#   FFmpeg::FFmpeg - for all components
#   FFmpeg::<component> - where <component> in lower case (FFmpeg::avcodec) for each components
#
# Copyright (c) 2006, Matthias Kretz, <kretz@kde.org>
# Copyright (c) 2008, Alexander Neundorf, <neundorf@kde.org>
# Copyright (c) 2011, Michael Jansen, <kde@michael-jansen.biz>
# Copyright (c) 2017, Alexander Drozdov, <adrozdoff@gmail.com>
# Copyright (c) 2019, Jan Holthuis, <holthuis.jan@googlemail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(FindPackageHandleStandardArgs)

# Qt-compliant component names (uppercase, no lib prefix):
#   AVCODEC, AVFORMAT, AVDEVICE, AVUTIL, AVFILTER, SWSCALE, SWRESAMPLE
if(NOT FFmpeg_FIND_COMPONENTS)
  set(FFmpeg_FIND_COMPONENTS AVCODEC AVFORMAT AVUTIL)
endif()

#
### Macro: find_component
#
# Checks for the given component by invoking pkgconfig and then looking up
# the libraries and include directories.
# component  - Qt-compliant uppercase name, e.g. AVCODEC
# pkgconfig  - pkg-config module name, e.g. libavcodec
# library    - library, e.g. avcodec
# header     - header path, e.g. libavcodec/avcodec.h
#
macro(find_component component pkgconfig library header)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  find_package(PkgConfig QUIET)
  if(PkgConfig_FOUND)
    pkg_check_modules(PC_FFmpeg_${component} QUIET ${pkgconfig})
  endif()

  find_path(
    FFmpeg_${component}_INCLUDE_DIRS
    ${header}
    HINTS
      ${PC_FFmpeg_${component}_INCLUDEDIR}
      ${PC_FFmpeg_${component}_INCLUDE_DIRS}
      ${PC_FFmpeg_INCLUDE_DIRS}
    PATH_SUFFIXES ffmpeg
  )

  find_library(
    FFmpeg_${component}_LIBRARIES
    NAMES ${PC_FFmpeg_${component}_LIBRARIES} ${library}
    HINTS
      ${PC_FFmpeg_${component}_LIBDIR}
      ${PC_FFmpeg_${component}_LIBRARY_DIRS}
      ${PC_FFmpeg_LIBRARY_DIRS}
  )

  #message(STATUS ${FFmpeg_${component}_LIBRARIES})
  #message(STATUS ${PC_FFmpeg_${component}_LIBRARIES})

  set(
    FFmpeg_${component}_DEFINITIONS
    ${PC_FFmpeg_${component}_CFLAGS_OTHER}
    CACHE STRING
    "The ${component} CFLAGS."
  )
  set(
    FFmpeg_${component}_VERSION
    ${PC_FFmpeg_${component}_VERSION}
    CACHE STRING
    "The ${component} version number."
  )

  if(FFmpeg_${component}_LIBRARIES AND FFmpeg_${component}_INCLUDE_DIRS)
    message(STATUS "  - ${component} ${FFmpeg_${component}_VERSION} found.")
    set(FFmpeg_${component}_FOUND TRUE)
  else()
    message(STATUS "  - ${component} not found.")
  endif()

  mark_as_advanced(
    FFmpeg_${component}_INCLUDE_DIRS
    FFmpeg_${component}_LIBRARIES
    FFmpeg_${component}_DEFINITIONS
    FFmpeg_${component}_VERSION
  )
endmacro()

message(STATUS "Searching for FFmpeg components")
# Check for all possible component.
find_component(AVCODEC    libavcodec    avcodec    libavcodec/avcodec.h)
find_component(AVFORMAT   libavformat   avformat   libavformat/avformat.h)
find_component(AVDEVICE   libavdevice   avdevice   libavdevice/avdevice.h)
find_component(AVUTIL     libavutil     avutil     libavutil/avutil.h)
find_component(AVFILTER   libavfilter   avfilter   libavfilter/avfilter.h)
find_component(SWSCALE    libswscale    swscale    libswscale/swscale.h)
find_component(SWRESAMPLE libswresample swresample libswresample/swresample.h)

set(FFmpeg_LIBRARIES "")
set(FFmpeg_DEFINITIONS "")
# Check if the required components were found and add their stuff to the FFmpeg_* vars.
foreach(component ${FFmpeg_FIND_COMPONENTS})
  if(FFmpeg_${component}_FOUND)
    #message(STATUS "Required component ${component} present.")
    set(FFmpeg_LIBRARIES ${FFmpeg_LIBRARIES} ${FFmpeg_${component}_LIBRARIES})
    set(
      FFmpeg_DEFINITIONS
      ${FFmpeg_DEFINITIONS}
      ${FFmpeg_${component}_DEFINITIONS}
    )
    list(APPEND FFmpeg_INCLUDE_DIRS ${FFmpeg_${component}_INCLUDE_DIRS})
  endif()
endforeach()

# Build the include path with duplicates removed.
if(FFmpeg_INCLUDE_DIRS)
  list(REMOVE_DUPLICATES FFmpeg_INCLUDE_DIRS)
endif()

# cache the vars.
set(
  FFmpeg_INCLUDE_DIRS
  ${FFmpeg_INCLUDE_DIRS}
  CACHE STRING
  "The FFmpeg include directories."
  FORCE
)
set(
  FFmpeg_LIBRARIES
  ${FFmpeg_LIBRARIES}
  CACHE STRING
  "The FFmpeg libraries."
  FORCE
)
set(
  FFmpeg_DEFINITIONS
  ${FFmpeg_DEFINITIONS}
  CACHE STRING
  "The FFmpeg cflags."
  FORCE
)

mark_as_advanced(FFmpeg_INCLUDE_DIRS FFmpeg_LIBRARIES FFmpeg_DEFINITIONS)

# Compile the list of required vars
set(FFmpeg_REQUIRED_VARS FFmpeg_LIBRARIES FFmpeg_INCLUDE_DIRS)
foreach(component ${FFmpeg_FIND_COMPONENTS})
  list(
    APPEND
    FFmpeg_REQUIRED_VARS
    FFmpeg_${component}_LIBRARIES
    FFmpeg_${component}_INCLUDE_DIRS
  )
endforeach()

# Give a nice error message if some of the required vars are missing.
find_package_handle_standard_args(FFmpeg DEFAULT_MSG ${FFmpeg_REQUIRED_VARS})
