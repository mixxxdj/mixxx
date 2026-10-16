#[=======================================================================[.rst:
FindFdkAac
----------

Finds the Fraunhofer FDK AAC library (libfdk-aac).

FDK-AAC is used in two distinct ways in Mixxx:
``Encoding in EncoderFdkAac``
  which loads libfdk-aac dynamically the DLL/dylib
  which is bundled in the Windows and macOS packages
``FFmpeg AAC decoding in SoundSourceFFmpeg``
  prefer this decoder over FFmpeg's built-in aac decoder
  and it's used for AAC encoded STEM files.

Imported Targets
^^^^^^^^^^^^^^^^

``FdkAac::FdkAac``
  The FDK-AAC library.

Result Variables
^^^^^^^^^^^^^^^^

``FdkAac_FOUND``
  True if the system has the FDK-AAC library.
``FdkAac_INCLUDE_DIRS``
  Include directories needed to use FDK-AAC (may be empty when headers
  are absent — only required when compiling against FDK-AAC directly).
``FdkAac_LIBRARIES``
  Libraries needed to link to FDK-AAC.
``FdkAac_DEFINITIONS``
  Compile definitions needed to use FDK-AAC.
``FdkAac_DLL``
  (Windows only) Full path to ``fdk-aac.dll``, suitable for install().

Cache Variables
^^^^^^^^^^^^^^^

``FdkAac_INCLUDE_DIR``
  The directory containing ``fdk-aac/aacenc_lib.h`` (optional).
``FdkAac_LIBRARY``
  The FDK-AAC link library (``.dylib`` / ``.so`` / ``.a`` / ``.lib``).
``FdkAac_DLL``
  (Windows only) Path to ``fdk-aac.dll``.

#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(PC_FdkAac QUIET fdk-aac)
endif()

find_path(
  FdkAac_INCLUDE_DIR
  NAMES fdk-aac/aacenc_lib.h
  HINTS ${PC_FdkAac_INCLUDE_DIRS}
  DOC "FDK-AAC include directory"
)
mark_as_advanced(FdkAac_INCLUDE_DIR)

find_library(
  FdkAac_LIBRARY
  NAMES fdk-aac libfdk-aac
  HINTS ${PC_FdkAac_LIBRARY_DIRS}
  DOC "FDK-AAC library"
)
mark_as_advanced(FdkAac_LIBRARY)

# On Windows find_library() returns the .lib import library.
# Locate the companion .dll separately so callers can install it.
if(WIN32)
  find_file(
    FdkAac_DLL
    NAMES fdk-aac.dll
    HINTS ${PC_FdkAac_LIBRARY_DIRS}
    PATH_SUFFIXES ${CMAKE_INSTALL_BINDIR} bin
    DOC "FDK-AAC DLL (Windows runtime)"
  )
  mark_as_advanced(FdkAac_DLL)
endif()

if(DEFINED PC_FdkAac_VERSION AND NOT PC_FdkAac_VERSION STREQUAL "")
  set(FdkAac_VERSION "${PC_FdkAac_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
# Only the library is required. Headers are optional as no Mixxx code includes
# FDK-AAC headers directly, so the include dir may legitimately be absent.
find_package_handle_standard_args(
  FdkAac
  REQUIRED_VARS FdkAac_LIBRARY
  VERSION_VAR FdkAac_VERSION
)

if(FdkAac_FOUND)
  set(FdkAac_LIBRARIES "${FdkAac_LIBRARY}")
  set(FdkAac_INCLUDE_DIRS "${FdkAac_INCLUDE_DIR}")
  set(FdkAac_DEFINITIONS ${PC_FdkAac_CFLAGS_OTHER})

  if(NOT TARGET FdkAac::FdkAac)
    if(WIN32 AND FdkAac_DLL)
      # Create a SHARED IMPORTED target so $<TARGET_RUNTIME_DLLS> can discover
      # fdk-aac.dll automatically.  The contract is:
      #   IMPORTED_LOCATION  = the .dll  (runtime artifact)
      #   IMPORTED_IMPLIB    = the .lib  (link artifact)
      add_library(FdkAac::FdkAac SHARED IMPORTED)
      set_target_properties(
        FdkAac::FdkAac
        PROPERTIES
          IMPORTED_LOCATION "${FdkAac_DLL}"
          IMPORTED_IMPLIB "${FdkAac_LIBRARY}"
          INTERFACE_COMPILE_OPTIONS "${FdkAac_DEFINITIONS}"
      )
    else()
      # On non-Windows UNKNOWN IMPORTED means the normal shared/static lib.
      # On Windows with no DLL found it links fine but $<TARGET_RUNTIME_DLLS>
      # will not pick it up (DLL must be deployed manually).
      add_library(FdkAac::FdkAac UNKNOWN IMPORTED)
      set_target_properties(
        FdkAac::FdkAac
        PROPERTIES
          IMPORTED_LOCATION "${FdkAac_LIBRARY}"
          INTERFACE_COMPILE_OPTIONS "${FdkAac_DEFINITIONS}"
      )
    endif()

    if(FdkAac_INCLUDE_DIR)
      set_target_properties(
        FdkAac::FdkAac
        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${FdkAac_INCLUDE_DIR}"
      )
    endif()
  endif()
endif()
