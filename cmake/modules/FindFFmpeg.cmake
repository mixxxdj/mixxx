#.rst:
# FindFFmpeg
# ----------
#
# Try to find the required FFmpeg components (default: AVCODEC, AVFORMAT, AVUTIL)
#
# Next variables can be used to hint FFmpeg libs search:
#
# ::
#
#   PC_FFmpeg_LIBRARY_DIRS
#   PC_FFmpeg_INCLUDE_DIRS
#
# Once done this will define
#
# ::
#
#   FFmpeg_FOUND         - System has all required components.
#   FFmpeg_INCLUDE_DIRS  - Include directories for all required components.
#   FFmpeg_LIBRARIES     - Libraries to link for all required components.
#   FFmpeg_DEFINITIONS   - Compiler switches required for using FFmpeg.
#
# For each of the components it will additionally set.
#
# ::
#
#   FFmpeg_<COMPONENT>_FOUND        - System has <COMPONENT>
#   FFmpeg_<COMPONENT>_INCLUDE_DIRS - Include directories for <COMPONENT>
#   FFmpeg_<COMPONENT>_LIBRARIES    - Libraries to link for <COMPONENT>
#   FFmpeg_<COMPONENT>_DEFINITIONS  - Compiler switches for <COMPONENT>
#   FFmpeg_<COMPONENT>_VERSION      - Version of <COMPONENT>
#
# The following imported targets are created:
#
# ::
#
#   FFmpeg::FFmpeg     - interface target aggregating all found components
#   FFmpeg::avcodec    - libavcodec
#   FFmpeg::avformat   - libavformat
#   FFmpeg::avdevice   - libavdevice
#   FFmpeg::avutil     - libavutil
#   FFmpeg::avfilter   - libavfilter
#   FFmpeg::swscale    - libswscale
#   FFmpeg::swresample - libswresample
#
# Component names are Qt-compliant uppercase (AVCODEC, AVFORMAT, …) as
# required by Qt6's internal find_dependency() calls in
# Qt6FFmpegMediaPluginImplPrivateDependencies.cmake:
#
#   find_dependency(FFmpeg COMPONENTS AVCODEC AVFORMAT AVUTIL SWRESAMPLE SWSCALE)
#
# Imported target names are lowercase (FFmpeg::avcodec, …) as listed in
# Qt6FFmpegMediaPluginImplPrivateDependencies.cmake:
#
#   provided_targets "FFmpeg::avcodec;FFmpeg::avformat;FFmpeg::avutil;
#                     FFmpeg::swresample;FFmpeg::swscale"
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
include(IsStaticLibrary)

# Early-return guard: Qt6's find_dependency() chain calls find_package(FFmpeg)
# once per multimedia plugin.  Imported targets are global and persistent, so
# if FFmpeg::avcodec already exists the search is done — return silently.
if(TARGET FFmpeg::avcodec)
  set(FFmpeg_FOUND TRUE)
  return()
endif()

if(NOT FFmpeg_FIND_COMPONENTS)
  set(FFmpeg_FIND_COMPONENTS AVCODEC AVFORMAT AVUTIL)
endif()

# Component table: UPPERCASE  lowercase-stem  pkg-config-name  primary-header
set(
  _FFmpeg_components
  "AVCODEC    avcodec    libavcodec    libavcodec/avcodec.h"
  "AVFORMAT   avformat   libavformat   libavformat/avformat.h"
  "AVDEVICE   avdevice   libavdevice   libavdevice/avdevice.h"
  "AVUTIL     avutil     libavutil     libavutil/avutil.h"
  "AVFILTER   avfilter   libavfilter   libavfilter/avfilter.h"
  "SWSCALE    swscale    libswscale    libswscale/swscale.h"
  "SWRESAMPLE swresample libswresample libswresample/swresample.h"
)
foreach(_entry IN LISTS _FFmpeg_components)
  separate_arguments(_fields UNIX_COMMAND "${_entry}")
  list(GET _fields 0 _uc)
  list(GET _fields 1 _lc)
  list(GET _fields 2 _pc)
  list(GET _fields 3 _hdr)
  set(_FFmpeg_${_uc}_lower "${_lc}")
  set(_FFmpeg_${_uc}_pkgconfig "${_pc}")
  set(_FFmpeg_${_uc}_header "${_hdr}")
endforeach()
unset(_entry)
unset(_fields)
unset(_uc)
unset(_lc)
unset(_pc)
unset(_hdr)

find_package(PkgConfig QUIET)

macro(find_component component)
  set(_lower "${_FFmpeg_${component}_lower}")
  set(_pkgcfg "${_FFmpeg_${component}_pkgconfig}")
  set(_header "${_FFmpeg_${component}_header}")

  if(PkgConfig_FOUND)
    pkg_check_modules(PC_FFmpeg_${component} QUIET ${_pkgcfg})
  endif()

  find_path(
    FFmpeg_${component}_INCLUDE_DIRS
    ${_header}
    HINTS
      ${PC_FFmpeg_${component}_INCLUDEDIR}
      ${PC_FFmpeg_${component}_INCLUDE_DIRS}
      ${PC_FFmpeg_INCLUDE_DIRS}
    PATH_SUFFIXES ffmpeg
  )
  find_library(
    FFmpeg_${component}_LIBRARIES
    NAMES ${PC_FFmpeg_${component}_LIBRARIES} ${_lower}
    HINTS
      ${PC_FFmpeg_${component}_LIBDIR}
      ${PC_FFmpeg_${component}_LIBRARY_DIRS}
      ${PC_FFmpeg_LIBRARY_DIRS}
  )
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
  mark_as_advanced(
    FFmpeg_${component}_INCLUDE_DIRS
    FFmpeg_${component}_LIBRARIES
    FFmpeg_${component}_DEFINITIONS
    FFmpeg_${component}_VERSION
  )

  if(FFmpeg_${component}_LIBRARIES AND FFmpeg_${component}_INCLUDE_DIRS)
    message(STATUS "  - ${component} ${FFmpeg_${component}_VERSION} found.")
    set(FFmpeg_${component}_FOUND TRUE)
  else()
    message(STATUS "  - ${component} not found.")
  endif()
  unset(_lower)
  unset(_pkgcfg)
  unset(_header)
endmacro()

message(STATUS "Searching for FFmpeg components")
foreach(
  _comp
  AVCODEC
  AVFORMAT
  AVDEVICE
  AVUTIL
  AVFILTER
  SWSCALE
  SWRESAMPLE
)
  find_component(${_comp})
endforeach()
unset(_comp)

# Aggregate results for requested components
set(FFmpeg_LIBRARIES "")
set(FFmpeg_DEFINITIONS "")
set(FFmpeg_INCLUDE_DIRS "")
foreach(component ${FFmpeg_FIND_COMPONENTS})
  if(FFmpeg_${component}_FOUND)
    list(APPEND FFmpeg_LIBRARIES ${FFmpeg_${component}_LIBRARIES})
    list(APPEND FFmpeg_DEFINITIONS ${FFmpeg_${component}_DEFINITIONS})
    list(APPEND FFmpeg_INCLUDE_DIRS ${FFmpeg_${component}_INCLUDE_DIRS})
  endif()
endforeach()
list(REMOVE_DUPLICATES FFmpeg_INCLUDE_DIRS)

# Cache aggregates (consumed by CMakeLists.txt via "${FFmpeg_LIBRARIES}")
set(
  FFmpeg_LIBRARIES
  ${FFmpeg_LIBRARIES}
  CACHE STRING
  "The FFmpeg libraries."
  FORCE
)
set(
  FFmpeg_INCLUDE_DIRS
  ${FFmpeg_INCLUDE_DIRS}
  CACHE STRING
  "The FFmpeg include directories."
  FORCE
)
set(
  FFmpeg_DEFINITIONS
  ${FFmpeg_DEFINITIONS}
  CACHE STRING
  "The FFmpeg cflags."
  FORCE
)
mark_as_advanced(FFmpeg_LIBRARIES FFmpeg_INCLUDE_DIRS FFmpeg_DEFINITIONS)

# Build required-vars list from requested components
set(_FFmpeg_required FFmpeg_LIBRARIES FFmpeg_INCLUDE_DIRS)
foreach(component ${FFmpeg_FIND_COMPONENTS})
  list(
    APPEND
    _FFmpeg_required
    FFmpeg_${component}_LIBRARIES
    FFmpeg_${component}_INCLUDE_DIRS
  )
endforeach()
find_package_handle_standard_args(FFmpeg DEFAULT_MSG ${_FFmpeg_required})
unset(_FFmpeg_required)

if(FFmpeg_FOUND)
  # Create per-component IMPORTED targets
  foreach(
    component
    AVCODEC
    AVFORMAT
    AVDEVICE
    AVUTIL
    AVFILTER
    SWSCALE
    SWRESAMPLE
  )
    if(FFmpeg_${component}_FOUND)
      set(_target "FFmpeg::${_FFmpeg_${component}_lower}")
      if(NOT TARGET ${_target})
        add_library(${_target} UNKNOWN IMPORTED)
        set_target_properties(
          ${_target}
          PROPERTIES
            IMPORTED_LOCATION "${FFmpeg_${component}_LIBRARIES}"
            INTERFACE_INCLUDE_DIRECTORIES "${FFmpeg_${component}_INCLUDE_DIRS}"
            INTERFACE_COMPILE_OPTIONS "${FFmpeg_${component}_DEFINITIONS}"
        )
      endif()
      unset(_target)
    endif()
  endforeach()

  # When libavcodec is static it may have been built with --enable-libfdk-aac.
  # Add FdkAac::FdkAac as a transitive dep so the final linker resolves the
  # aacEnc*/aacDec* symbols (mirrors FindChromaprint.cmake's FFTW3 pattern).
  if(TARGET FFmpeg::avcodec)
    is_static_library(_avcodec_static FFmpeg::avcodec)
    if(_avcodec_static)
      find_package(FdkAac QUIET)
      if(FdkAac_FOUND)
        set_property(
          TARGET FFmpeg::avcodec
          APPEND
          PROPERTY INTERFACE_LINK_LIBRARIES FdkAac::FdkAac
        )
        # Also append to the cached FFmpeg_LIBRARIES variable so any remaining
        # consumers that use "${FFmpeg_LIBRARIES}" directly (e.g. legacy call
        # sites) also get the dependency resolved.
        list(APPEND FFmpeg_LIBRARIES ${FdkAac_LIBRARY})
        set(
          FFmpeg_LIBRARIES
          ${FFmpeg_LIBRARIES}
          CACHE STRING
          "The FFmpeg libraries."
          FORCE
        )
      endif()
    endif()
    unset(_avcodec_static)
  endif()

  # On Apple platforms the static FFmpeg libraries reference VideoToolbox,
  # CoreMedia and CoreVideo symbols (e.g. av_map_videotoolbox_format_to_pixfmt
  # in libavutil, hardware-accelerated codecs in libavcodec).  Add the
  # required frameworks as transitive interface deps on the affected targets.
  if(APPLE)
    foreach(_component_target IN ITEMS FFmpeg::avcodec FFmpeg::avutil)
      if(TARGET ${_component_target})
        is_static_library(_is_static ${_component_target})
        if(_is_static)
          set_property(
            TARGET ${_component_target}
            APPEND
            PROPERTY
              INTERFACE_LINK_LIBRARIES
                "-framework VideoToolbox"
                "-framework CoreMedia"
                "-framework CoreVideo"
          )
        endif()
        unset(_is_static)
      endif()
    endforeach()
    unset(_component_target)

    # Also append to FFmpeg_LIBRARIES for legacy consumers
    is_static_library(_avutil_static FFmpeg::avutil)
    if(_avutil_static)
      list(
        APPEND
        FFmpeg_LIBRARIES
        "-framework VideoToolbox"
        "-framework CoreMedia"
        "-framework CoreVideo"
      )
      set(
        FFmpeg_LIBRARIES
        ${FFmpeg_LIBRARIES}
        CACHE STRING
        "The FFmpeg libraries."
        FORCE
      )
    endif()
    unset(_avutil_static)
  endif()

  # Aggregate convenience target
  if(NOT TARGET FFmpeg::FFmpeg)
    add_library(FFmpeg::FFmpeg INTERFACE IMPORTED)
    foreach(component ${FFmpeg_FIND_COMPONENTS})
      if(FFmpeg_${component}_FOUND)
        target_link_libraries(
          FFmpeg::FFmpeg
          INTERFACE "FFmpeg::${_FFmpeg_${component}_lower}"
        )
      endif()
    endforeach()
  endif()
endif()
