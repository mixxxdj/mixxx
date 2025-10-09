cmake_minimum_required(VERSION 3.21)
# lint_cmake: -readability/wonkycase

message(STATUS "CMAKE_VERSION: ${CMAKE_VERSION}")

# CMAKE_CXX_COMPILER_ID: Distinguish between "AppleClang" and "Clang"
if(POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW)
endif()

# MACOSX_RPATH is set by default
if(POLICY CMP0042)
  cmake_policy(SET CMP0042 NEW)
endif()

# Support new IN_LIST if() operator
if(POLICY CMP0057)
  cmake_policy(SET CMP0057 NEW)
endif()

# Enforce interprocedural optimization
if(POLICY CMP0069)
  cmake_policy(SET CMP0069 NEW)
endif()

# Let AUTOMOC and AUTOUIC process GENERATED files
if(POLICY CMP0071)
  cmake_policy(SET CMP0071 NEW)
endif()

# Propagate interface link properties
if(POLICY CMP0099)
  # This avoids a warning when qt deals with different behaviours controlled by this policy
  # in its cmake functions. See
  # https://github.com/qt/qtbase/commit/e3e1007f9778d8fc629a06f7d3d216bb7f81351b
  cmake_policy(SET CMP0099 NEW)
endif()

# An imported target missing its location property fails during generation.
if(POLICY CMP0111)
  cmake_policy(SET CMP0111 NEW)
endif()

# Set the timestamp of extracted files to the time of the extraction instead of
# the archived timestamp to make sure that dependent files are rebuilt if the
# URL changes.
if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

if(((APPLE AND NOT IOS) OR WIN32) AND NOT IS_DIRECTORY "${MIXXX_VCPKG_ROOT}")
  if(NOT DEFINED BUILDENV_BASEPATH)
    if(DEFINED ENV{BUILDENV_BASEPATH})
      set(BUILDENV_BASEPATH "$ENV{BUILDENV_BASEPATH}")
    else()
      set(BUILDENV_BASEPATH "${CMAKE_SOURCE_DIR}/buildenv")
    endif()
  endif()

  if(NOT DEFINED BUILDENV_URL)
    if(DEFINED ENV{BUILDENV_URL})
      set(BUILDENV_URL "$ENV{BUILDENV_URL}")
    elseif(NOT DEFINED BUILDENV_URL)
      message(FATAL_ERROR "BUILDENV_URL not specified")
    endif()
  endif()

  # Extract the filename from the URL
  get_filename_component(FILENAME_WITH_EXTENSION "${BUILDENV_URL}" NAME)

  # Remove the extension from the filename (note that our buildenv names contain two dots)
  string(
    REGEX REPLACE
    "\\.[^.]*$"
    ""
    BUILDENV_NAME
    "${FILENAME_WITH_EXTENSION}"
  )

  message(STATUS "BUILDENV_NAME is: ${BUILDENV_NAME}")

  if(
    NOT EXISTS "${BUILDENV_BASEPATH}/${BUILDENV_NAME}"
    OR NOT IS_DIRECTORY "${BUILDENV_BASEPATH}/${BUILDENV_NAME}"
  )
    if(NOT DEFINED BUILDENV_SHA256)
      if(DEFINED ENV{BUILDENV_SHA256})
        set(BUILDENV_SHA256 "$ENV{BUILDENV_SHA256}")
      elseif(NOT DEFINED BUILDENV_SHA256)
        message(STATUS "BUILDENV_SHA256 not specified")
      endif()
    endif()

    if(NOT EXISTS "${BUILDENV_BASEPATH}/${BUILDENV_NAME}.zip")
      message(
        STATUS
        "Downloading file ${BUILDENV_URL} to ${BUILDENV_BASEPATH} ..."
      )
      file(
        DOWNLOAD ${BUILDENV_URL} "${BUILDENV_BASEPATH}/${BUILDENV_NAME}.zip"
        SHOW_PROGRESS
        TLS_VERIFY ON
      )
    else()
      # Reference to suppress intentionally unused variable warnings
      set(_dummy "${BUILDENV_URL}")
    endif()

    if(NOT ${BUILDENV_SHA256} STREQUAL "")
      message(
        STATUS
        "Verify SHA256 of downloaded file ${BUILDENV_BASEPATH}/${BUILDENV_NAME}.zip ..."
      )
      file(SHA256 "${BUILDENV_BASEPATH}/${BUILDENV_NAME}.zip" actual_sha256)

      if(NOT actual_sha256 STREQUAL ${BUILDENV_SHA256})
        message(
          FATAL_ERROR
          "SHA256 checksum mismatch:\nexpected: ${BUILDENV_SHA256}\n     got: ${actual_sha256}"
        )
      else()
        message(STATUS "SHA256 ${BUILDENV_SHA256} is correct!")
      endif()
    endif()

    message(
      STATUS
      "Unpacking file ${BUILDENV_BASEPATH}/${BUILDENV_NAME}.zip ..."
    )
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E tar xzf "${BUILDENV_NAME}.zip"
      WORKING_DIRECTORY ${BUILDENV_BASEPATH}
    )
  else()
    # Reference to suppress intentionally unused variable warnings
    set(_dummy "${BUILDENV_SHA256}")
  endif()
else()
  # Reference to suppress intentionally unused variable warnings
  set(_dummy "${BUILDENV_URL} ${BUILDENV_BASEPATH} ${BUILDENV_SHA256}")
endif()

# Use this function to throw an error because the build environment is not set
# up correctly.
function(fatal_error_missing_env)
  if(WIN32)
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
      message(
        FATAL_ERROR
        "Did you download the Mixxx build environment using `${CMAKE_SOURCE_DIR}/tools/windows_buildenv.bat`?"
      )
    else()
      message(
        FATAL_ERROR
        "Did you download the Mixxx build environment using `${CMAKE_SOURCE_DIR}/tools/windows_release_buildenv.bat` or `${CMAKE_SOURCE_DIR}/tools/windows_buildenv.bat`(includes Debug)?"
      )
    endif()
  elseif(APPLE AND NOT IOS)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
      message(
        FATAL_ERROR
        "Did you build the Mixxx build environment as described here: https://github.com/mixxxdj/mixxx/wiki/Compiling-dependencies-for-macOS-arm64?"
      )
    elseif(CMAKE_BUILD_TYPE MATCHES "Debug")
      message(
        FATAL_ERROR
        "Did you download the Mixxx build environment using `source ${CMAKE_SOURCE_DIR}/tools/macos_buildenv.sh setup`"
      )
    else()
      message(
        FATAL_ERROR
        "Did you download the Mixxx build environment using `source ${CMAKE_SOURCE_DIR}/tools/macos_release_buildenv.sh setup` or `source ${CMAKE_SOURCE_DIR}/tools/macos_buildenv.sh setup` (includes Debug)?"
      )
    endif()
  elseif(UNIX AND NOT APPLE)
    # Linux, BSD, Solaris, Minix
    if(EXISTS "/etc/debian_version") # exists also on Ubuntu and Mint
      message(
        FATAL_ERROR
        "Did you install the required Debian dev packages via `${CMAKE_SOURCE_DIR}/tools/debian_buildenv.sh setup`?"
      )
    elseif(EXISTS "/etc/redhat-release") # exists also on Fedora Mageia Madndriva Alma CentOS
      message(
        FATAL_ERROR
        "Did you install the required RPM dev packages via `${CMAKE_SOURCE_DIR}/tools/rpm_buildenv.sh setup`?"
      )
    else()
      message(
        FATAL_ERROR
        "Did you install the equivalent dev packages listed in `${CMAKE_SOURCE_DIR}/tools/debian_buildenv.sh setup`?"
      )
    endif()
  elseif(DEFINED VCPKG_TARGET_TRIPLET)
    message(
      FATAL_ERROR
      "You are targeting ${VCPKG_TARGET_TRIPLET}, which does not have a prebuilt environment. Please make sure that -DMIXXX_VCPKG_ROOT points to a vcpkg environment containing installed dependencies for ${VCPKG_TARGET_TRIPLET}!"
    )
  else()
    message(
      FATAL_ERROR
      "You are building for an unknown platform and are missing a build environment. Please set -DVCPKG_TARGET_TRIPLET and make sure that -DMIXXX_VCPKG_ROOT points to a vcpkg environment containing installed dependencies for your target platform!"
    )
  endif()
endfunction()

if(UNIX AND NOT APPLE)
  execute_process(
    COMMAND uname -r
    OUTPUT_VARIABLE UNAME_R
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if(UNAME_R MATCHES ".*[Mm]icrosoft.*" OR UNAME_R MATCHES ".*WSL.*")
    message(
      FATAL_ERROR
      "Building on Windows Subsystem for Linux (WSL) is not supported. If you want to build Mixxx from command line on Windows, you need to use the \"x64 Native Tools Command Prompt for VS 2022\"!"
    )
  endif()
endif()

# We use here ENV{MIXXX_VCPKG_ROOT} as a workaround to find the overlay folders
# in manifest mode https://github.com/microsoft/vcpkg/issues/12289.
# Note: VCPKG_ROOT, the default location for the vcpkg cli tool is later
# adjusted by CMAKE_TOOLCHAIN_FILE.
if(DEFINED ENV{MIXXX_VCPKG_ROOT} AND NOT DEFINED MIXXX_VCPKG_ROOT)
  set(MIXXX_VCPKG_ROOT "$ENV{MIXXX_VCPKG_ROOT}")
endif()

if(DEFINED MIXXX_VCPKG_ROOT)
  if(
    EXISTS "${MIXXX_VCPKG_ROOT}/overlay/ports"
    OR NOT EXISTS "${MIXXX_VCPKG_ROOT}/ports"
  )
    # MIXXX_VCPKG_ROOT points to our vcpkg environment
    # and we configure the CMAKE_TOOLCHAIN_FILE and overlays accordingly
    message(STATUS "Using MIXXX_VCPKG_ROOT: ${MIXXX_VCPKG_ROOT}")
  else()
    message(
      STATUS
      "MIXXX_VCPKG_ROOT not correct (missing ${MIXXX_VCPKG_ROOT}/overlay/ports)"
    )
    fatal_error_missing_env()
  endif()

  if(NOT DEFINED VCPKG_OVERLAY_PORTS)
    # required for manifest mode
    set(VCPKG_OVERLAY_PORTS "${MIXXX_VCPKG_ROOT}/overlay/ports")
    if(APPLE)
      list(APPEND VCPKG_OVERLAY_PORTS "${MIXXX_VCPKG_ROOT}/overlay/osx")
    elseif(WIN32)
      list(APPEND VCPKG_OVERLAY_PORTS "${MIXXX_VCPKG_ROOT}/overlay/windows")
    endif()
  endif()

  if(NOT DEFINED VCPKG_OVERLAY_TRIPLETS)
    # required for manifest mode
    set(VCPKG_OVERLAY_TRIPLETS "${MIXXX_VCPKG_ROOT}/overlay/triplets")
  endif()

  if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(
      CMAKE_TOOLCHAIN_FILE
      "${MIXXX_VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING
      ""
    )
  endif()
endif()

if(NOT DEFINED VCPKG_TARGET_TRIPLET)
  if(DEFINED ENV{VCPKG_TARGET_TRIPLET})
    set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_TARGET_TRIPLET}")
  elseif(DEFINED ENV{VCPKG_DEFAULT_TRIPLET})
    set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}")
  endif()
endif()
set(
  X_VCPKG_APPLOCAL_DEPS_INSTALL
  ON
  CACHE BOOL
  "Automatically copy dependencies into the install target directory for executables."
  FORCE
)

# Set a default build type if none was specified
# See https://blog.kitware.com/cmake-and-the-default-build-type/ for details.
set(default_build_type "RelWithDebInfo")
if(EXISTS "${CMAKE_SOURCE_DIR}/.git" AND NOT WIN32)
  # On Windows, Debug builds are linked to unoptimized libs
  # generating unusable slow Mixxx builds.
  set(default_build_type "Debug")
endif()

if(NOT CMAKE_CONFIGURATION_TYPES)
  if(NOT CMAKE_BUILD_TYPE)
    message(
      STATUS
      "Setting CMAKE_BUILD_TYPE to '${default_build_type}' as none was specified."
    )
    set(
      CMAKE_BUILD_TYPE
      "${default_build_type}"
      CACHE STRING
      "Choose the type of build."
      FORCE
    )
    # Set the possible values of build type for cmake-gui
    set_property(
      CACHE CMAKE_BUILD_TYPE
      PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo"
    )
  elseif(NOT CMAKE_BUILD_TYPE MATCHES "^(Debug|Release|RelWithDebInfo)$")
    message(
      FATAL_ERROR
      "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} is not supported, use one of Debug, Release or RelWithDebInfo."
    )
  endif()
endif()

include(CMakeDependentOption)

option(QT6 "Build with Qt6" ON)

# Because of multiple concurrent definition of symbols caused by the rendergraph
# compile definition we need to disable QML by default. This avoids the risk of
# undefined behaviour in a stable build.
# See: https://github.com/mixxxdj/mixxx/issues/14766
# Once this is fixed we can revert the commit introducing this.
option(QML "Build with QML" OFF)

option(QOPENGL "Use QOpenGLWindow based widget instead of QGLWidget" ON)

if(QOPENGL)
  add_compile_definitions(MIXXX_USE_QOPENGL)
endif()

if(QML)
  add_compile_definitions(MIXXX_USE_QML)
endif()

if(VCPKG_TARGET_TRIPLET MATCHES "^wasm(32|64)-emscripten")
  message(STATUS "Targeting Emscripten (${VCPKG_TARGET_TRIPLET})")
  if(DEFINED ENV{EMSDK})
    message(STATUS "Found EMSDK at $ENV{EMSDK}")
  else()
    message(
      FATAL_ERROR
      "Please make sure emsdk is installed and the environment variable EMSDK is set (see https://emscripten.org/docs/getting_started/downloads.html)"
    )
  endif()
  if(NOT DEFINED VCPKG_CHAINLOAD_TOOLCHAIN_FILE)
    set(
      VCPKG_CHAINLOAD_TOOLCHAIN_FILE
      "$ENV{EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake"
      CACHE STRING
      ""
    )
  endif()
  # Enabling this causes Qt's FindWrapRt C++ compile check to fail as it tries
  # to run `clang-scan-deps` (because we set the C++ standard to 20). Emscripten
  # does not ship this binary yet, though. Potentially relevant upstream bug:
  # https://github.com/emscripten-core/emscripten/issues/21042
  set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
elseif(APPLE)
  # Check if xcode-select is installed
  execute_process(
    COMMAND xcode-select -v
    RESULT_VARIABLE XCODE_SELECT_RESULT
    OUTPUT_QUIET
  )
  if(XCODE_SELECT_RESULT)
    # xcode-select command failed, meaning it is not installed or not configured properly
    message(
      FATAL_ERROR
      "'xcode-select -v' failed with '${XCODE_SELECT_RESULT}'. You may need to install Xcode and run 'sudo xcode-select --install'."
    )
  endif()

  if(VCPKG_TARGET_TRIPLET MATCHES "^[a-zA-Z0-9]+-osx")
    message(STATUS "Targeting macOS (${VCPKG_TARGET_TRIPLET})")
    set(CMAKE_SYSTEM_NAME Darwin CACHE STRING "Target macOS")
    if(VCPKG_TARGET_TRIPLET MATCHES "^arm64-")
      # Minimum macOS version for arm64 Support
      set(
        CMAKE_OSX_DEPLOYMENT_TARGET
        11.0
        CACHE STRING
        "Minimum macOS version the build will be able to run on"
      )
      set(CMAKE_OSX_ARCHITECTURES arm64 CACHE STRING "The target architecture")
      set(
        CMAKE_SYSTEM_PROCESSOR
        arm64
        CACHE STRING
        "The target system processor"
      )
    else()
      if(QT6)
        # Minimum macOS version supported by Qt 6.5
        set(
          CMAKE_OSX_DEPLOYMENT_TARGET
          11.0
          CACHE STRING
          "Minimum macOS version the build will be able to run on"
        )
      else()
        # Minimum macOS version supported by Qt 5.12
        set(
          CMAKE_OSX_DEPLOYMENT_TARGET
          10.12
          CACHE STRING
          "Minimum macOS version the build will be able to run on"
        )
        # Needed for deployment target < 10.14
        add_compile_options(-fno-aligned-allocation)
      endif()
    endif()
  elseif(VCPKG_TARGET_TRIPLET MATCHES "^[a-zA-Z0-9]+-ios")
    message(STATUS "Targeting iOS (${VCPKG_TARGET_TRIPLET})")
    set(CMAKE_SYSTEM_NAME iOS CACHE STRING "Target iOS")
    set(
      CMAKE_OSX_DEPLOYMENT_TARGET
      14.0
      CACHE STRING
      "Minimum iOS version to target"
    )
  else()
    message(
      WARNING
      "Targeting an Apple platform, but VCPKG_TARGET_TRIPLET is not set. This is not a supported scenario!"
    )
  endif()
endif()

project(mixxx VERSION 2.7.0 LANGUAGES C CXX)
# Work around missing version suffixes support https://gitlab.kitware.com/cmake/cmake/-/issues/16716
set(MIXXX_VERSION_PRERELEASE "alpha") # set to "alpha" "beta" or ""

set(CMAKE_PROJECT_HOMEPAGE_URL "https://www.mixxx.org")
set(
  CMAKE_PROJECT_DESCRIPTION
  "Mixxx is Free DJ software that gives you everything you need to perform live mixes."
)

# Used for force control of color output
set(BUILD_COLORS "auto" CACHE STRING "Try to use colors auto/always/no")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
include(CheckSymbolExists)
include(CheckIncludeFileCXX)
include(ExternalProject)
include(GNUInstallDirs)
include(DefaultOption)
include(IsStaticLibrary)

# Verify VCPKG settings
if(DEFINED _VCPKG_INSTALLED_DIR)
  if(NOT EXISTS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
    # Fail early if this part of CMAKE_PREFIX_PATH does not exist
    # else the library lookups below will fail with misleading error messages
    message(
      STATUS
      "VCPKG_TARGET_TRIPLET dir not found: ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET} "
      "Make sure the VCPKG build environment is installed and contains the build for the selected triplet."
    )
    fatal_error_missing_env()
  else()
    message(STATUS "Using VCPKG_TARGET_TRIPLET: ${VCPKG_TARGET_TRIPLET}")
  endif()
endif()

#######################################################################
# Compilers and toolchains

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # GNU is GNU GCC
  set(GNU_GCC true)
else()
  set(GNU_GCC false)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  if(CMAKE_CXX_SIMULATE_ID MATCHES "MSVC")
    set(LLVM_CLANG false)
    set(MSVC true)
  else()
    # using regular Clang or AppleClang
    set(LLVM_CLANG true)
  endif()
else()
  set(LLVM_CLANG false)
endif()

# CMake implicitly sets the variable MSVC to true for Microsoft
# Visual C++ or another compiler simulating Visual C++.
# https://cmake.org/cmake/help/latest/variable/MSVC.html

#######################################################################

set(CMAKE_CXX_STANDARD 20)
if(MSVC)
  # Ensure MSVC populates __cplusplus correctly.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")
  # Remove unreferenced code and data
  # Since c++11 they can safely be removed to speed up linking.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:inline")

  if(NOT DEFINED MSVC_TOOLSET_VERSION OR MSVC_TOOLSET_VERSION VERSION_LESS 143)
    message(
      FATAL_ERROR
      "MSVC_TOOLSET_VERSION is ${MSVC_TOOLSET_VERSION}.\n"
      "Mixxx requires the Microsoft Visual C++ Redistributable toolset of version 143 (VS2022) or greater, "
      "as the VCPKG buildenv is built with this version.\n"
      "Please use the Visual Studio 2022 toolset therefore!"
    )
  endif()
endif()

# Speed up builds on HDDs and prevent wearing of SDDs
#
# This is only applies to gcc/clang, therefore this option is forcibly set to
# ON on all other compilers.
cmake_dependent_option(
  BUILD_LOW_MEMORY
  "Store temporary build files on disk by disabling the build option -pipe"
  OFF
  "GNU_GCC OR LLVM_CLANG"
  ON
)
if(NOT BUILD_LOW_MEMORY)
  add_compile_options(-pipe)
endif()

# Coverage
#
# This is only available with GCC, therefore this option is forcibly set to OFF
# for all other compilers.
cmake_dependent_option(
  COVERAGE
  "Coverage (i.e. gcov) support"
  OFF
  "GNU_GCC"
  OFF
)
if(COVERAGE)
  add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
  add_link_options(--coverage -fprofile-arcs -ftest-coverage)
endif()

# Profiling
#
# This is only available on Linux, therefore this option is forcibly set to OFF
# on all other platforms.
cmake_dependent_option(
  PROFILING
  "Profiling (e.g. gprof) support"
  OFF
  "UNIX;NOT APPLE"
  OFF
)
if(PROFILING)
  add_compile_options(-pg)
  add_link_options(-pg)
endif()

#
# Optimizations
#

set(
  OPTIMIZE
  "portable"
  CACHE STRING
  "Optimization and Tuning (set to off, portable, native, legacy)"
)
set_property(
  CACHE OPTIMIZE
  PROPERTY STRINGS "off" "portable" "native" "legacy"
)
string(TOLOWER "${OPTIMIZE}" OPTIMIZE)
message(STATUS "Optimization level: ${OPTIMIZE}")
message(STATUS "CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")

# CMAKE_INTERPROCEDURAL_OPTIMIZATION can be defined to override the default behaviour.
# We keep CMAKE_INTERPROCEDURAL_OPTIMIZATION unset (IPO disabled) to save
# build time at the cost of a bigger memory footprint at run-time.
# See https://github.com/mixxxdj/mixxx/pull/3589 for some test results
# Note: IPO has caused issues on Fedora https://bugzilla.rpmfusion.org/show_bug.cgi?id=5829
# Uncomment the following code to enable IPO for Release builds
#if(NOT DEFINED CMAKE_INTERPROCEDURAL_OPTIMIZATION AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT OPTIMIZE STREQUAL "off")
#  include(CheckIPOSupported)
#  check_ipo_supported(RESULT HAVE_IPO)
#  if(HAVE_IPO)
#    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
#  endif()
#endif()

if(MSVC)
  # Microsoft Visual Studio Compiler
  add_compile_options(/UTF8)
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x64|x86_64|AMD64)$")
    # Target architecture is x64 -> x64 has alsways SSE and SSE2 instruction sets
    message(STATUS "x64 Enabling SSE2 CPU optimizations (>= Pentium 4)")
    # Define gcc/clang style defines for SSE and SSE2 for compatibility
    add_compile_definitions("__SSE__" "__SSE2__")
  endif()

  # Needed for sccache
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    string(REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REPLACE "/Zi" "/Z7" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
  elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    string(
      REPLACE
      "/Zi"
      "/Z7"
      CMAKE_CXX_FLAGS_RELEASE
      "${CMAKE_CXX_FLAGS_RELEASE}"
    )
    string(REPLACE "/Zi" "/Z7" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
  elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    string(
      REPLACE
      "/Zi"
      "/Z7"
      CMAKE_CXX_FLAGS_RELWITHDEBINFO
      "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}"
    )
    string(
      REPLACE
      "/Zi"
      "/Z7"
      CMAKE_C_FLAGS_RELWITHDEBINFO
      "${CMAKE_C_FLAGS_RELWITHDEBINFO}"
    )
  endif()

  if(NOT OPTIMIZE STREQUAL "off")
    # Use the fastest floating point math library
    # http://msdn.microsoft.com/en-us/library/e7s85ffb.aspx
    # http://msdn.microsoft.com/en-us/library/ms235601.aspx
    add_compile_options(/fp:fast)

    # Suggested for unused code removal
    # http://msdn.microsoft.com/en-us/library/ms235601.aspx
    # http://msdn.microsoft.com/en-us/library/xsa71f43.aspx
    # http://msdn.microsoft.com/en-us/library/bxwfs976.aspx
    add_compile_options(/Gy)

    # For repeated local development builds, /INCREMENTAL offers much faster build times with the same performance of the executable,
    # unless link time code generation (like CMAKE_INTERPROCEDURAL_OPTIMIZATION) is used, which is contrary to incremental linking.

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
      # Optimize Debug Builds as well, to have "normal" behavior of Mixxx during development
      string(REPLACE "/Od" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
      string(REPLACE "/Od" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS}")
      string(REPLACE "/Ob0" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
      string(REPLACE "/Ob0" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS}")

      add_compile_options(/O2) # this implies /Od2
      # Remove /RTC1 flag set by CMAKE by default (conflicts with /O2)
      string(
        REPLACE
        "/RTC1"
        ""
        CMAKE_CXX_FLAGS_DEBUG
        "${CMAKE_CXX_FLAGS_DEBUG}"
      )
      string(REPLACE "/RTC1" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
      # For some reasons cmake uses /Ob1 in RelWithDebInfo https://gitlab.kitware.com/cmake/cmake/-/issues/20812
      # /O2 is applied by CMake and this implies /Od2
      string(
        REPLACE
        "/Ob1"
        ""
        CMAKE_CXX_FLAGS_RELWITHDEBINFO
        "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}"
      )
      string(
        REPLACE
        "/Ob1"
        ""
        CMAKE_C_FLAGS_RELWITHDEBINFO
        "${CMAKE_C_FLAGS_RELWITHDEBINFO}"
      )

      # Reduce the size of the binary in RelWithDebInfo builds
      # Do not use /OPT:ICF because it has no effect.
      # https://github.com/mixxxdj/mixxx/pull/3660#pullrequestreview-600137258
      add_link_options(/OPT:REF)

      # /INCREMENTAL is incompatible with /OPT:REF, but it's the CMake default for RelWithDebInfo
      # The CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO can be defined by the user in the GUI or in CMakeSettings.json,
      # therefore we can't rely on the default.
      string(
        FIND
        CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO
        "/INCREMENTAL:NO"
        INCREMENTAL_NO_POSITION
      )
      if(INCREMENTAL_NO_POSITION EQUAL -1)
        message(
          STATUS
          "Overwriting /INCREMENTAL by /INCREMENTAL:NO to allow link time code optimization"
        )
        string(
          REPLACE
          "/INCREMENTAL"
          "/INCREMENTAL:NO"
          CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO
          "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}"
        )
      endif()
      # Note: CMAKE_INTERPROCEDURAL_OPTIMIZATION sets the /GL and /LTCG flags for us
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
      # Reduce the size of the binary in Release builds
      # Do not use /OPT:ICF because it has no effect.
      # https://github.com/mixxxdj/mixxx/pull/3660#pullrequestreview-600137258
      add_link_options(/OPT:REF)
    endif()

    if(OPTIMIZE STREQUAL "portable")
      if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(i[3456]86|x86)$")
        # Target architecture is x86 with SSE and SSE2
        message(STATUS "x86 Enabling SSE2 CPU optimizations (>= Pentium 4)")
        # Define gcc/clang style defines for SSE and SSE2 for compatibility
        add_compile_definitions("__SSE__" "__SSE2__")
        # Set compiler option for SSE/SSE2
        add_compile_options(/arch:SSE2)
      endif()
    elseif(OPTIMIZE STREQUAL "native")
      message("Enabling optimizations for native system, specified by user")
      if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(i[3456]86|x86)$")
        # Target architecture is x86 with SSE and SSE2
        message(STATUS "x86 Enabling SSE2 CPU optimizations (>= Pentium 4)")
        # Define gcc/clang style defines for SSE and SSE2 for compatibility
        add_compile_definitions("__SSE__" "__SSE2__")
      endif()
      # Define the target processor instruction and other compiler optimization flags here:
      # https://docs.microsoft.com/en-us/cpp/build/reference/arch-x64?view=msvc-160
      # add_compile_options(/arch:AVX512)
      message(
        FATAL_ERROR
        "User need to set the MSVC compiler flags for the native processor here!"
      )
      add_compile_options("/favor:${CMAKE_SYSTEM_PROCESSOR}")
    elseif(OPTIMIZE STREQUAL "legacy")
      if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x64|x86_64|AMD64)$")
        message("Enabling pure x64 instruction set (without AVX etc.)")
      elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(i[3456]86|x86)$")
        message("Enabling pure i386 instruction set (without SSE/SSE2 etc.)")
      endif()
    else()
      message(
        FATAL_ERROR
        "Invalid value passed to OPTIMIZE option: ${OPTIMIZE}"
      )
    endif()
  else()
    # OPTIMIZE=off
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
      # Remove optimize flags set by cmake defaults
      string(
        REPLACE
        "/O2"
        ""
        CMAKE_CXX_FLAGS_RELEASE
        "${CMAKE_CXX_FLAGS_RELEASE}"
      )
      string(REPLACE "/O2" "" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
      string(
        REPLACE
        "/Ob2"
        ""
        CMAKE_CXX_FLAGS_RELEASE
        "${CMAKE_CXX_FLAGS_RELEASE}"
      )
      string(REPLACE "/Ob2" "" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
      add_compile_options(/Od) # this implies /Ob0
      add_compile_options(/RTC1)
    elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
      # Remove optimize flags set by cmake defaults
      string(
        REPLACE
        "/O2"
        ""
        CMAKE_CXX_FLAGS_RELWITHDEBINFO
        "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}"
      )
      string(
        REPLACE
        "/O2"
        ""
        CMAKE_C_FLAGS_RELWITHDEBINFO
        "${CMAKE_C_FLAGS_RELWITHDEBINFO}"
      )
      # For some reasons cmake uses /Ob1 in RelWithDebInfo https://gitlab.kitware.com/cmake/cmake/-/issues/20812
      string(
        REPLACE
        "/Ob1"
        ""
        CMAKE_CXX_FLAGS_RELWITHDEBINFO
        "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}"
      )
      string(
        REPLACE
        "/Ob1"
        ""
        CMAKE_C_FLAGS_RELWITHDEBINFO
        "${CMAKE_C_FLAGS_RELWITHDEBINFO}"
      )
      add_compile_options(/Od) # this implies /Ob0
      add_compile_options(/RTC1)
    endif()
  endif()
elseif(GNU_GCC OR LLVM_CLANG)
  if(NOT OPTIMIZE STREQUAL "off")
    # Common flags to all optimizations.
    # -ffast-math will prevent a performance penalty by denormals
    # (floating point values almost Zero are treated as Zero)
    # unfortunately that work only on 64 bit CPUs or with sse2 enabled
    # The following optimisation flags makes the engine code ~3 times
    # faster, measured on a Atom CPU.
    add_compile_options(-ffast-math -funroll-loops)
    if(EMSCRIPTEN)
      # Optimize for size + speed when targeting Emscripten/WebAssembly
      # This is recommended as we use asyncify:
      # See https://doc.qt.io/qt-6/wasm.html#asyncify
      add_compile_options(-Os)
    else()
      add_compile_options(-O3)
    endif()
    # set -fomit-frame-pointer when we don't profile and are not using
    # Clang sanitizers.
    # Note: It is only included in -O on machines where it does not
    # interfere with debugging
    if(NOT PROFILING AND NOT SANITIZERS)
      add_compile_options(-fomit-frame-pointer)
    endif()

    if(OPTIMIZE STREQUAL "portable")
      # portable: sse2 CPU (>= Pentium 4)
      if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(i[3456]86|x86|x64|x86_64|AMD64)$")
        message(STATUS "Enabling SSE2 CPU optimizations (>= Pentium 4)")
        if(NOT EMSCRIPTEN)
          add_compile_options(-mtune=generic)
        endif()
        # -mtune=generic picks the most common, but compatible options.
        # on arm platforms equivalent to -march=arch
        if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
          # the sse flags are not set by default on 32 bit builds
          # but are not supported on arm builds
          add_compile_options(-msse2)
          if(EMSCRIPTEN)
            add_compile_options(-msimd128)
          else()
            add_compile_options(-mfpmath=sse)
          endif()
        endif()
        # TODO(rryan): macOS can use SSE3, and possibly SSE 4.1 once
        # we require macOS 10.12.
        # https://stackoverflow.com/questions/45917280/mac-osx-minumum-support-sse-version
      elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm|armv7.*)$") # but not armv8
        add_compile_options(-mfloat-abi=hard -mfpu=neon)
      endif()
      # this sets macros __SSE2_MATH__ __SSE_MATH__ __SSE2__ __SSE__
      # This should be our default build for distribution
      # It's a little sketchy, but turning on SSE2 will gain
      # 100% performance in our filter code and allows us to
      # turns on denormal zeroing.
      # We don't really support CPU's earlier than Pentium 4,
      # which is the class of CPUs this decision affects.
      # The downside of this is that we aren't truly
      # i386 compatible, so builds that claim 'i386' will crash.
      # -- rryan 2/2011
      # Note: SSE2 is a core part of x64 CPUs
    elseif(OPTIMIZE STREQUAL "native")
      message("Enabling native optimizations for ${CMAKE_SYSTEM_PROCESSOR}")
      add_compile_options(-march=native)
      # Note: requires gcc >= 4.2.0
      # macros like __SSE2_MATH__ __SSE_MATH__ __SSE2__ __SSE__
      # are set automatically
      if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm|armv7.*)$") # but not armv8
        add_compile_options(-mfloat-abi=hard -mfpu=neon)
      endif()
    elseif(OPTIMIZE STREQUAL "legacy")
      if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(i[3456]86|x86|x64|x86_64|AMD64)$")
        message("Enabling pure i386 code")
        add_compile_options(-mtune=generic)
        # -mtune=generic pick the most common, but compatible options.
        # on arm platforms equivalent to -march=arch
      endif()
    else()
      message(
        FATAL_ERROR
        "Invalid value passed to OPTIMIZE option: ${OPTIMIZE}"
      )
    endif()
  endif()
endif()

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

if(WIN32)
  # Add support for lib prefix on Windows
  set(CMAKE_FIND_LIBRARY_PREFIXES "" "lib")
endif()

if(MSVC)
  if(NOT DEFINED CMAKE_DISABLE_PRECOMPILE_HEADERS)
    # With MSVC, PCH is faster than caching
    set(CMAKE_DISABLE_PRECOMPILE_HEADERS OFF)
  endif()
  set(
    CMAKE_DISABLE_PRECOMPILE_HEADERS
    ${CMAKE_DISABLE_PRECOMPILE_HEADERS}
    CACHE BOOL
    "Disable precompiled headers"
  )

  # sccache support
  find_program(SCCACHE_EXECUTABLE "sccache")
  if(SCCACHE_EXECUTABLE)
    message(STATUS "Found sccache: ${SCCACHE_EXECUTABLE}")
  else()
    message(STATUS "Could NOT find sccache (missing executable)")
  endif()
  default_option(SCCACHE_SUPPORT "Enable sccache support" "SCCACHE_EXECUTABLE;CMAKE_DISABLE_PRECOMPILE_HEADERS")
  message(STATUS "Support for sccache: ${SCCACHE_SUPPORT}")
  if(SCCACHE_SUPPORT)
    if(NOT CMAKE_DISABLE_PRECOMPILE_HEADERS)
      message(
        WARNING
        "sccache: Does not work with precompiled headers. Set CMAKE_DISABLE_PRECOMPILE_HEADERS=ON"
      )
    endif()
    set(CMAKE_C_COMPILER_LAUNCHER "${SCCACHE_EXECUTABLE}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${SCCACHE_EXECUTABLE}")
  endif()
else()
  # ccache support
  find_program(CCACHE_EXECUTABLE "ccache")
  if(CCACHE_EXECUTABLE)
    message(STATUS "Found ccache: ${CCACHE_EXECUTABLE}")
  else()
    message(STATUS "Could NOT find ccache (missing executable)")
  endif()
  default_option(CCACHE_SUPPORT "Enable ccache support" "CCACHE_EXECUTABLE")

  if(NOT DEFINED CMAKE_DISABLE_PRECOMPILE_HEADERS)
    set(CMAKE_DISABLE_PRECOMPILE_HEADERS ${CCACHE_SUPPORT})
  endif()
  set(
    CMAKE_DISABLE_PRECOMPILE_HEADERS
    ${CMAKE_DISABLE_PRECOMPILE_HEADERS}
    CACHE BOOL
    "Disable precompiled headers"
  )

  if(CCACHE_SUPPORT)
    if(GNU_GCC OR LLVM_CLANG)
      # without this compiler messages in `make` backend would be uncolored
      set(
        CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -fdiagnostics-color=${BUILD_COLORS}"
      )
    endif()
    if(NOT CMAKE_DISABLE_PRECOMPILE_HEADERS)
      execute_process(
        COMMAND "${CCACHE_EXECUTABLE}" "--get-config=sloppiness"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        OUTPUT_VARIABLE CCACHE_CONFIGURED_SLOPPINESS
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
      )
      if(
        NOT CCACHE_CONFIGURED_SLOPPINESS MATCHES "pch_defines"
        OR NOT CCACHE_CONFIGURED_SLOPPINESS MATCHES "time_macros"
      )
        message(
          WARNING
          "ccache: For use with precompiled headers, the setting \"sloppiness\" needs to "
          "be set to \"pch_defines,time_macros\". This can be done via the environment variable "
          "\"CCACHE_SLOPPINESS=pch_defines,time_macros\" or permanent via "
          "\"ccache --set-config=sloppiness=pch_defines,time_macros\"."
        )
      endif()
    endif()
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
  endif()
  message(STATUS "Support for ccache: ${CCACHE_SUPPORT}")
endif()

if(NOT MSVC)
  # Try to configure mold as linker via -fuse-ld=mold, support from gcc 12.1, gcc 11.2.0-16 or clang
  execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} -fuse-ld=mold -Wl,--version
    ERROR_QUIET
    OUTPUT_VARIABLE MOLD_FUSE_VERSION_STRING
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if(MOLD_FUSE_VERSION_STRING)
    set(MOLD_FUSE_FOUND TRUE)
  endif()
  if(NOT MOLD_FUSE_FOUND)
    # check if the symlink ld is in the mold folder for older compiler
    find_program(MOLD_SYMLINK "${CMAKE_INSTALL_LIBEXECDIR}/mold/ld")
    get_filename_component(MOLD_SYMLINK_DIRECTORY ${MOLD_SYMLINK} DIRECTORY)
  endif()
  if(MOLD_SYMLINK)
    set(MOLD_SYMLINK_FOUND TRUE)
  endif()
  default_option(MOLD_SUPPORT "Use 'mold' for linking" "MOLD_FUSE_FOUND OR MOLD_SYMLINK_FOUND")
  if(MOLD_SUPPORT)
    if(MOLD_FUSE_FOUND)
      message(STATUS "Selecting mold as linker")
      add_link_options("-fuse-ld=mold")
    elseif(MOLD_SYMLINK_FOUND)
      message(
        STATUS
        "Selecting mold as linker via ld symlink in ${MOLD_SYMLINK_DIRECTORY}"
      )
      add_link_options("-B${MOLD_SYMLINK_DIRECTORY}")
    else()
      message(FATAL_ERROR "Could NOT find mold (missing executable)")
    endif()
  else()
    # Try to configure lld as linker
    execute_process(
      COMMAND ${CMAKE_CXX_COMPILER} -fuse-ld=lld -Wl,--version
      ERROR_QUIET
      OUTPUT_VARIABLE LLD_VERSION_STRING
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(LLD_VERSION_STRING)
      string(
        REGEX MATCH
        "LLD ([0-9]+\\.[0-9]+\\.[0-9]+)"
        LLD_VERSION_MATCH
        "${LLD_VERSION_STRING}"
      )
      if(LLD_VERSION_MATCH)
        set(LLD_VERSION ${CMAKE_MATCH_1})
        message(STATUS "Found ld.lld with version: ${LLD_VERSION}")
      else()
        message(
          WARNING
          "Failed to parse ld.lld version from: ${LLD_VERSION_STRING}"
        )
      endif()
    endif()
    # LLD 10.0.0 does not work because of https://bugs.llvm.org/show_bug.cgi?id=45769
    default_option(LLD_SUPPORT "Use 'ld.lld' for linking" "LLD_VERSION VERSION_GREATER 10.0.0")
    if(LLD_SUPPORT)
      if(LLD_VERSION_STRING)
        if(LLD_VERSION VERSION_GREATER 10.0.0)
          message(STATUS "Selecting lld as linker")
          add_link_options("-fuse-ld=lld")
        else()
          message(FATAL_ERROR "Could NOT find ld.lld > 10.0.0")
        endif()
      else()
        message(FATAL_ERROR "Could NOT find ld.lld (missing executable)")
      endif()
    endif()
  endif()
endif()

if(CMAKE_VERSION VERSION_LESS "3.7.0")
  set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(
  CLANG_TIDY
  ""
  CACHE STRING
  "CMAKE_CXX_CLANG_TIDY equivalent that only applies to mixxx sources, not bundled dependencies"
)

# Mixxx itself
add_library(
  mixxx-lib
  STATIC
  EXCLUDE_FROM_ALL
  src/analyzer/analyzerbeats.cpp
  src/analyzer/analyzerebur128.cpp
  src/analyzer/analyzergain.cpp
  src/analyzer/analyzerkey.cpp
  src/analyzer/analyzerscheduledtrack.cpp
  src/analyzer/analyzersilence.cpp
  src/analyzer/analyzerthread.cpp
  src/analyzer/analyzertrack.cpp
  src/analyzer/analyzerwaveform.cpp
  src/analyzer/plugins/analyzerqueenmarybeats.cpp
  src/analyzer/plugins/analyzerqueenmarykey.cpp
  src/analyzer/plugins/analyzersoundtouchbeats.cpp
  src/analyzer/plugins/buffering_utils.cpp
  src/analyzer/trackanalysisscheduler.cpp
  src/audio/frame.cpp
  src/audio/signalinfo.cpp
  src/audio/streaminfo.cpp
  src/audio/types.cpp
  src/control/control.cpp
  src/control/controlaudiotaperpot.cpp
  src/control/controlbehavior.cpp
  src/control/controlcompressingproxy.cpp
  src/control/controleffectknob.cpp
  src/control/controlencoder.cpp
  src/control/controlindicator.cpp
  src/control/controlindicatortimer.cpp
  src/control/controllinpotmeter.cpp
  src/control/controllogpotmeter.cpp
  src/control/controlobject.cpp
  src/control/controlobjectscript.cpp
  src/control/controlpotmeter.cpp
  src/control/controlproxy.cpp
  src/control/controlpushbutton.cpp
  src/control/controlttrotary.cpp
  src/controllers/controller.cpp
  src/controllers/controllerenumerator.cpp
  src/controllers/controllerinputmappingtablemodel.cpp
  src/controllers/controllerlearningeventfilter.cpp
  src/controllers/controllermanager.cpp
  src/controllers/controllermappinginfo.cpp
  src/controllers/legacycontrollersettings.cpp
  src/controllers/legacycontrollersettingslayout.cpp
  src/controllers/controllermappinginfoenumerator.cpp
  src/controllers/controllermappingtablemodel.cpp
  src/controllers/controlleroutputmappingtablemodel.cpp
  src/controllers/controlpickermenu.cpp
  src/controllers/legacycontrollermappingfilehandler.cpp
  src/controllers/legacycontrollermapping.cpp
  src/controllers/delegates/controldelegate.cpp
  src/controllers/delegates/midibytedelegate.cpp
  src/controllers/delegates/midichanneldelegate.cpp
  src/controllers/delegates/midiopcodedelegate.cpp
  src/controllers/delegates/midioptionsdelegate.cpp
  src/controllers/dlgcontrollerlearning.cpp
  src/controllers/dlgcontrollerlearning.ui
  src/controllers/dlgprefcontroller.cpp
  src/controllers/dlgprefcontrollerdlg.ui
  src/controllers/dlgprefcontrollers.cpp
  src/controllers/dlgprefcontrollersdlg.ui
  src/controllers/keyboard/keyboardeventfilter.cpp
  src/controllers/learningutils.cpp
  src/controllers/legacycontrollermappingfilehandler.cpp
  src/controllers/midi/legacymidicontrollermapping.cpp
  src/controllers/midi/legacymidicontrollermappingfilehandler.cpp
  src/controllers/midi/midicontroller.cpp
  src/controllers/midi/midienumerator.cpp
  src/controllers/midi/midimessage.cpp
  src/controllers/midi/midioutputhandler.cpp
  src/controllers/midi/midiutils.cpp
  src/controllers/scripting/colormapper.cpp
  src/controllers/scripting/colormapperjsproxy.cpp
  src/controllers/scripting/controllerscriptenginebase.cpp
  src/controllers/scripting/controllerscriptmoduleengine.cpp
  src/controllers/scripting/legacy/controllerscriptenginelegacy.cpp
  src/controllers/scripting/legacy/controllerscriptinterfacelegacy.cpp
  src/controllers/scripting/legacy/scriptconnection.cpp
  src/controllers/scripting/legacy/scriptconnectionjsproxy.cpp
  src/controllers/softtakeover.cpp
  src/coreservices.cpp
  src/database/mixxxdb.cpp
  src/database/schemamanager.cpp
  src/dialog/dlgabout.cpp
  src/dialog/dlgaboutdlg.ui
  src/dialog/dlgdevelopertools.cpp
  src/dialog/dlgdevelopertoolsdlg.ui
  src/dialog/dlgkeywheel.cpp
  src/dialog/dlgkeywheel.ui
  src/dialog/dlgreplacecuecolor.cpp
  src/dialog/dlgreplacecuecolordlg.ui
  src/effects/backends/builtin/autopaneffect.cpp
  src/effects/backends/builtin/balanceeffect.cpp
  src/effects/backends/builtin/bessel4lvmixeqeffect.cpp
  src/effects/backends/builtin/bessel8lvmixeqeffect.cpp
  src/effects/backends/builtin/biquadfullkilleqeffect.cpp
  src/effects/backends/builtin/bitcrushereffect.cpp
  src/effects/backends/builtin/builtinbackend.cpp
  src/effects/backends/builtin/distortioneffect.cpp
  src/effects/backends/builtin/echoeffect.cpp
  src/effects/backends/builtin/filtereffect.cpp
  src/effects/backends/builtin/flangereffect.cpp
  src/effects/backends/builtin/glitcheffect.cpp
  src/effects/backends/builtin/graphiceqeffect.cpp
  src/effects/backends/builtin/linkwitzriley8eqeffect.cpp
  src/effects/backends/builtin/loudnesscontoureffect.cpp
  src/effects/backends/builtin/metronomeeffect.cpp
  src/effects/backends/builtin/metronomeclick.cpp
  src/effects/backends/builtin/moogladder4filtereffect.cpp
  src/effects/backends/builtin/compressoreffect.cpp
  src/effects/backends/builtin/autogaincontroleffect.cpp
  src/effects/backends/builtin/parametriceqeffect.cpp
  src/effects/backends/builtin/phasereffect.cpp
  src/effects/backends/builtin/reverbeffect.cpp
  src/effects/backends/builtin/threebandbiquadeqeffect.cpp
  src/effects/backends/builtin/tremoloeffect.cpp
  src/effects/backends/builtin/whitenoiseeffect.cpp
  src/effects/backends/effectmanifest.cpp
  src/effects/backends/effectmanifestparameter.cpp
  src/effects/backends/effectsbackend.cpp
  src/effects/backends/effectsbackendmanager.cpp
  src/effects/chains/equalizereffectchain.cpp
  src/effects/chains/outputeffectchain.cpp
  src/effects/chains/pergroupeffectchain.cpp
  src/effects/chains/quickeffectchain.cpp
  src/effects/chains/standardeffectchain.cpp
  src/effects/effectbuttonparameterslot.cpp
  src/effects/effectchain.cpp
  src/effects/effectchainmixmode.cpp
  src/effects/effectknobparameterslot.cpp
  src/effects/effectparameter.cpp
  src/effects/effectparameterslotbase.cpp
  src/effects/effectslot.cpp
  src/effects/effectsmanager.cpp
  src/effects/effectsmessenger.cpp
  src/effects/presets/effectchainpreset.cpp
  src/effects/presets/effectchainpresetmanager.cpp
  src/effects/presets/effectparameterpreset.cpp
  src/effects/presets/effectpreset.cpp
  src/effects/presets/effectpresetmanager.cpp
  src/effects/visibleeffectslist.cpp
  src/encoder/encoder.cpp
  src/encoder/encoderfdkaac.cpp
  src/encoder/encoderfdkaacsettings.cpp
  src/encoder/encoderflacsettings.cpp
  src/encoder/encodermp3.cpp
  src/encoder/encodermp3settings.cpp
  src/encoder/encodersndfileflac.cpp
  src/encoder/encodervorbis.cpp
  src/encoder/encodervorbissettings.cpp
  src/encoder/encoderwave.cpp
  src/encoder/encoderwavesettings.cpp
  src/engine/bufferscalers/enginebufferscale.cpp
  src/engine/bufferscalers/enginebufferscalelinear.cpp
  src/engine/bufferscalers/enginebufferscalest.cpp
  src/engine/cachingreader/cachingreader.cpp
  src/engine/cachingreader/cachingreaderchunk.cpp
  src/engine/cachingreader/cachingreaderworker.cpp
  src/engine/channelmixer.cpp
  src/engine/channels/engineaux.cpp
  src/engine/channels/enginechannel.cpp
  src/engine/channels/enginedeck.cpp
  src/engine/channels/enginemicrophone.cpp
  src/engine/controls/bpmcontrol.cpp
  src/engine/controls/clockcontrol.cpp
  src/engine/controls/cuecontrol.cpp
  src/engine/controls/enginecontrol.cpp
  src/engine/controls/keycontrol.cpp
  src/engine/controls/loopingcontrol.cpp
  src/engine/controls/quantizecontrol.cpp
  src/engine/controls/ratecontrol.cpp
  src/engine/effects/engineeffect.cpp
  src/engine/effects/engineeffectchain.cpp
  src/engine/effects/engineeffectsdelay.cpp
  src/engine/effects/engineeffectsmanager.cpp
  src/engine/enginebuffer.cpp
  src/engine/enginedelay.cpp
  src/engine/enginemixer.cpp
  src/engine/engineobject.cpp
  src/engine/enginepregain.cpp
  src/engine/enginesidechaincompressor.cpp
  src/engine/enginetalkoverducking.cpp
  src/engine/enginevumeter.cpp
  src/engine/engineworker.cpp
  src/engine/engineworkerscheduler.cpp
  src/engine/enginexfader.cpp
  src/engine/filters/enginefilterbessel4.cpp
  src/engine/filters/enginefilterbessel8.cpp
  src/engine/filters/enginefilterbiquad1.cpp
  src/engine/filters/enginefilterbutterworth4.cpp
  src/engine/filters/enginefilterbutterworth8.cpp
  src/engine/filters/enginefilterlinkwitzriley2.cpp
  src/engine/filters/enginefilterlinkwitzriley4.cpp
  src/engine/filters/enginefilterlinkwitzriley8.cpp
  src/engine/filters/enginefiltermoogladder4.cpp
  src/engine/positionscratchcontroller.cpp
  src/engine/readaheadmanager.cpp
  src/engine/sidechain/enginenetworkstream.cpp
  src/engine/sidechain/enginerecord.cpp
  src/engine/sidechain/enginesidechain.cpp
  src/engine/sidechain/networkinputstreamworker.cpp
  src/engine/sidechain/networkoutputstreamworker.cpp
  src/engine/sync/enginesync.cpp
  src/engine/sync/internalclock.cpp
  src/engine/sync/synccontrol.cpp
  src/errordialoghandler.cpp
  src/library/analysis/analysisfeature.cpp
  src/library/analysis/analysislibrarytablemodel.cpp
  src/library/analysis/dlganalysis.cpp
  src/library/analysis/dlganalysis.ui
  src/library/autodj/autodjfeature.cpp
  src/library/autodj/autodjprocessor.cpp
  src/library/autodj/dlgautodj.cpp
  src/library/autodj/dlgautodj.ui
  src/library/banshee/bansheedbconnection.cpp
  src/library/banshee/bansheefeature.cpp
  src/library/banshee/bansheeplaylistmodel.cpp
  src/library/baseexternallibraryfeature.cpp
  src/library/baseexternalplaylistmodel.cpp
  src/library/baseexternaltrackmodel.cpp
  src/library/basesqltablemodel.cpp
  src/library/basetrackcache.cpp
  src/library/basetracktablemodel.cpp
  src/library/browse/browsefeature.cpp
  src/library/browse/browsetablemodel.cpp
  src/library/browse/browsethread.cpp
  src/library/browse/foldertreemodel.cpp
  src/library/columncache.cpp
  src/library/coverart.cpp
  src/library/coverartcache.cpp
  src/library/coverartutils.cpp
  src/library/dao/analysisdao.cpp
  src/library/dao/autodjcratesdao.cpp
  src/library/dao/cuedao.cpp
  src/library/dao/directorydao.cpp
  src/library/dao/libraryhashdao.cpp
  src/library/dao/playlistdao.cpp
  src/library/dao/settingsdao.cpp
  src/library/dao/trackdao.cpp
  src/library/dao/trackschema.cpp
  src/library/tabledelegates/defaultdelegate.cpp
  src/library/dlgcoverartfullsize.cpp
  src/library/dlgcoverartfullsize.ui
  src/library/dlgtagfetcher.cpp
  src/library/dlgtagfetcher.ui
  src/library/dlgtrackinfo.cpp
  src/library/dlgtrackinfo.ui
  src/library/dlgtrackinfomulti.cpp
  src/library/dlgtrackinfomulti.ui
  src/library/dlgtrackmetadataexport.cpp
  src/library/export/coverartcopyworker.cpp
  src/library/export/dlgtrackexport.ui
  src/library/export/trackexportdlg.cpp
  src/library/export/trackexportwizard.cpp
  src/library/export/trackexportworker.cpp
  src/library/externaltrackcollection.cpp
  src/library/itunes/itunesdao.cpp
  src/library/itunes/itunesfeature.cpp
  src/library/itunes/itunesimporter.cpp
  src/library/itunes/itunesplaylistmodel.cpp
  src/library/itunes/itunestrackmodel.cpp
  src/library/itunes/itunesxmlimporter.cpp
  src/library/library_prefs.cpp
  src/library/library.cpp
  src/library/librarycontrol.cpp
  src/library/libraryfeature.cpp
  src/library/librarytablemodel.cpp
  src/library/missing_hidden/dlghidden.cpp
  src/library/missing_hidden/dlghidden.ui
  src/library/missing_hidden/dlgmissing.cpp
  src/library/missing_hidden/dlgmissing.ui
  src/library/missing_hidden/hiddentablemodel.cpp
  src/library/missing_hidden/missingtablemodel.cpp
  src/library/mixxxlibraryfeature.cpp
  src/library/overviewcache.cpp
  src/library/parser.cpp
  src/library/parsercsv.cpp
  src/library/parserm3u.cpp
  src/library/parserpls.cpp
  src/library/playlisttablemodel.cpp
  src/library/proxytrackmodel.cpp
  src/library/recording/dlgrecording.cpp
  src/library/recording/dlgrecording.ui
  src/library/recording/recordingfeature.cpp
  src/library/rekordbox/rekordboxfeature.cpp
  src/library/rhythmbox/rhythmboxfeature.cpp
  src/library/scanner/importfilestask.cpp
  src/library/scanner/libraryscanner.cpp
  src/library/scanner/libraryscannerdlg.cpp
  src/library/scanner/recursivescandirectorytask.cpp
  src/library/scanner/scannertask.cpp
  src/library/searchquery.cpp
  src/library/searchqueryparser.cpp
  src/library/serato/seratofeature.cpp
  src/library/serato/seratoplaylistmodel.cpp
  src/library/sidebarmodel.cpp
  src/library/starrating.cpp
  src/library/tabledelegates/bpmdelegate.cpp
  src/library/tabledelegates/checkboxdelegate.cpp
  src/library/tabledelegates/colordelegate.cpp
  src/library/tabledelegates/coverartdelegate.cpp
  src/library/tabledelegates/keydelegate.cpp
  src/library/tabledelegates/locationdelegate.cpp
  src/library/tabledelegates/multilineeditdelegate.cpp
  src/library/tabledelegates/overviewdelegate.cpp
  src/library/tabledelegates/previewbuttondelegate.cpp
  src/library/tabledelegates/stardelegate.cpp
  src/library/tabledelegates/stareditor.cpp
  src/library/tabledelegates/tableitemdelegate.cpp
  src/library/trackcollection.cpp
  src/library/trackcollectioniterator.cpp
  src/library/trackcollectionmanager.cpp
  src/library/trackloader.cpp
  src/library/trackmodeliterator.cpp
  src/library/trackprocessing.cpp
  src/library/trackset/baseplaylistfeature.cpp
  src/library/trackset/basetracksetfeature.cpp
  src/library/trackset/crate/cratefeature.cpp
  src/library/trackset/crate/cratefeaturehelper.cpp
  src/library/trackset/crate/cratestorage.cpp
  src/library/trackset/crate/cratetablemodel.cpp
  src/library/trackset/playlistfeature.cpp
  src/library/trackset/setlogfeature.cpp
  src/library/trackset/tracksettablemodel.cpp
  src/library/traktor/traktorfeature.cpp
  src/library/treeitem.cpp
  src/library/treeitemmodel.cpp
  src/mixer/auxiliary.cpp
  src/mixer/baseplayer.cpp
  src/mixer/basetrackplayer.cpp
  src/mixer/deck.cpp
  src/mixer/microphone.cpp
  src/mixer/playerinfo.cpp
  src/mixer/playermanager.cpp
  src/mixer/previewdeck.cpp
  src/mixer/sampler.cpp
  src/mixer/samplerbank.cpp
  src/mixxxapplication.cpp
  src/mixxxmainwindow.cpp
  src/musicbrainz/chromaprinter.cpp
  src/musicbrainz/crc.cpp
  src/musicbrainz/gzip.cpp
  src/musicbrainz/musicbrainz.cpp
  src/musicbrainz/musicbrainzxml.cpp
  src/musicbrainz/tagfetcher.cpp
  src/musicbrainz/web/acoustidlookuptask.cpp
  src/musicbrainz/web/coverartarchiveimagetask.cpp
  src/musicbrainz/web/coverartarchivelinkstask.cpp
  src/musicbrainz/web/musicbrainzrecordingstask.cpp
  src/nativeeventhandlerwin.cpp
  src/network/jsonwebtask.cpp
  src/network/networktask.cpp
  src/network/webtask.cpp
  src/osc/oscreceiver.cpp
  src/preferences/colorpaletteeditor.cpp
  src/preferences/colorpaletteeditor.cpp
  src/preferences/colorpaletteeditormodel.cpp
  src/preferences/colorpaletteeditormodel.cpp
  src/preferences/colorpalettesettings.cpp
  src/preferences/colorpalettesettings.cpp
  src/preferences/configobject.cpp
  src/preferences/constants.cpp
  src/preferences/dialog/dlgprefautodj.cpp
  src/preferences/dialog/dlgprefautodjdlg.ui
  src/preferences/dialog/dlgprefbeats.cpp
  src/preferences/dialog/dlgprefbeatsdlg.ui
  src/preferences/dialog/dlgprefcolors.cpp
  src/preferences/dialog/dlgprefcolorsdlg.ui
  src/preferences/dialog/dlgprefdeck.cpp
  src/preferences/dialog/dlgprefdeckdlg.ui
  src/preferences/dialog/dlgprefeffects.cpp
  src/preferences/dialog/dlgprefeffectsdlg.ui
  src/preferences/dialog/dlgpreferencepage.cpp
  src/preferences/dialog/dlgpreferences.cpp
  src/preferences/dialog/dlgpreferencesdlg.ui
  src/preferences/dialog/dlgprefinterface.cpp
  src/preferences/dialog/dlgprefinterfacedlg.ui
  src/preferences/dialog/dlgprefkey.cpp
  src/preferences/dialog/dlgprefkeydlg.ui
  src/preferences/dialog/dlgpreflibrary.cpp
  src/preferences/dialog/dlgpreflibrarydlg.ui
  src/preferences/dialog/dlgprefmixer.cpp
  src/preferences/dialog/dlgprefmixerdlg.ui
  src/preferences/dialog/dlgprefosc.cpp
  src/preferences/dialog/dlgprefoscdlg.ui
  src/preferences/dialog/dlgprefrecord.cpp
  src/preferences/dialog/dlgprefrecorddlg.ui
  src/preferences/dialog/dlgprefreplaygain.cpp
  src/preferences/dialog/dlgprefreplaygaindlg.ui
  src/preferences/dialog/dlgprefsound.cpp
  src/preferences/dialog/dlgprefsounddlg.ui
  src/preferences/dialog/dlgprefsounditem.cpp
  src/preferences/dialog/dlgprefsounditem.ui
  src/preferences/dialog/dlgprefvinyldlg.ui
  src/preferences/dialog/dlgprefwaveform.cpp
  src/preferences/dialog/dlgprefwaveformdlg.ui
  src/preferences/effectchainpresetlistmodel.cpp
  src/preferences/effectmanifesttablemodel.cpp
  src/preferences/replaygainsettings.cpp
  src/preferences/settingsmanager.cpp
  src/preferences/upgrade.cpp
  src/recording/recordingmanager.cpp
  src/skin/legacy/colorschemeparser.cpp
  src/skin/legacy/imgcolor.cpp
  src/skin/legacy/imginvert.cpp
  src/skin/legacy/imgloader.cpp
  src/skin/legacy/launchimage.cpp
  src/skin/legacy/legacyskin.cpp
  src/skin/legacy/legacyskinparser.cpp
  src/skin/legacy/pixmapsource.cpp
  src/skin/legacy/skincontext.cpp
  src/skin/legacy/tooltips.cpp
  src/skin/skincontrols.cpp
  src/skin/skinloader.cpp
  src/soundio/sounddevice.cpp
  src/soundio/sounddevicenetwork.cpp
  src/soundio/sounddeviceportaudio.cpp
  src/soundio/soundmanager.cpp
  src/soundio/soundmanagerconfig.cpp
  src/soundio/soundmanagerutil.cpp
  src/sources/audiosource.cpp
  src/sources/audiosourcestereoproxy.cpp
  src/sources/metadatasource.cpp
  src/sources/metadatasourcetaglib.cpp
  src/sources/readaheadframebuffer.cpp
  src/sources/soundsource.cpp
  src/sources/soundsourceflac.cpp
  src/sources/soundsourceoggvorbis.cpp
  src/sources/soundsourceprovider.cpp
  src/sources/soundsourceproviderregistry.cpp
  src/sources/soundsourceproxy.cpp
  src/sources/soundsourcesndfile.cpp
  src/track/albuminfo.cpp
  src/track/beatfactory.cpp
  src/track/beats.cpp
  src/track/beatutils.cpp
  src/track/bpm.cpp
  src/track/cue.cpp
  src/track/cueinfo.cpp
  src/track/cueinfoimporter.cpp
  src/track/globaltrackcache.cpp
  src/track/keyfactory.cpp
  src/track/keys.cpp
  src/track/keyutils.cpp
  src/track/playcounter.cpp
  src/track/replaygain.cpp
  src/track/serato/beatgrid.cpp
  src/track/serato/beatsimporter.cpp
  src/track/serato/color.cpp
  src/track/serato/cueinfoimporter.cpp
  src/track/serato/markers.cpp
  src/track/serato/markers2.cpp
  src/track/serato/tags.cpp
  src/track/taglib/trackmetadata_ape.cpp
  src/track/taglib/trackmetadata_common.cpp
  src/track/taglib/trackmetadata_file.cpp
  src/track/taglib/trackmetadata_id3v2.cpp
  src/track/taglib/trackmetadata_mp4.cpp
  src/track/taglib/trackmetadata_riff.cpp
  src/track/taglib/trackmetadata_xiph.cpp
  src/track/track.cpp
  src/track/trackinfo.cpp
  src/track/trackmetadata.cpp
  src/track/tracknumbers.cpp
  src/track/trackrecord.cpp
  src/track/trackref.cpp
  src/util/autofilereloader.cpp
  src/util/battery/battery.cpp
  src/util/cache.cpp
  src/util/clipboard.cpp
  src/util/cmdlineargs.cpp
  src/util/color/color.cpp
  src/util/color/colorpalette.cpp
  src/util/color/predefinedcolorpalettes.cpp
  src/util/colorcomponents.cpp
  src/util/console.cpp
  src/util/db/dbconnection.cpp
  src/util/db/dbconnectionpool.cpp
  src/util/db/dbconnectionpooled.cpp
  src/util/db/dbconnectionpooler.cpp
  src/util/db/fwdsqlquery.cpp
  src/util/db/fwdsqlqueryselectresult.cpp
  src/util/db/sqlite.cpp
  src/util/db/sqlqueryfinisher.cpp
  src/util/db/sqlstringformatter.cpp
  src/util/db/sqltransaction.cpp
  src/util/desktophelper.cpp
  src/util/dnd.cpp
  src/util/duration.cpp
  src/util/experiment.cpp
  src/util/file.cpp
  src/util/fileaccess.cpp
  src/util/fileinfo.cpp
  src/util/filename.cpp
  src/util/font.cpp
  src/util/imagefiledata.cpp
  src/util/imagefiledata.cpp
  src/util/imageutils.cpp
  src/util/indexrange.cpp
  src/util/logger.cpp
  src/util/logging.cpp
  src/util/mac.cpp
  src/util/moc_included_test.cpp
  src/util/movinginterquartilemean.cpp
  src/util/rangelist.cpp
  src/util/readaheadsamplebuffer.cpp
  src/util/ringdelaybuffer.cpp
  src/util/rotary.cpp
  src/util/runtimeloggingcategory.cpp
  src/util/safelywritablefile.cpp
  src/util/sample.cpp
  src/util/sandbox.cpp
  src/util/screensaver.cpp
  src/util/screensavermanager.cpp
  src/util/semanticversion.cpp
  src/util/stat.cpp
  src/util/statmodel.cpp
  src/util/statsmanager.cpp
  src/util/tapfilter.cpp
  src/util/task.cpp
  src/util/taskmonitor.cpp
  src/util/time.cpp
  src/util/timer.cpp
  src/util/valuetransformer.cpp
  src/util/versionstore.cpp
  src/util/widgethelper.cpp
  src/util/workerthread.cpp
  src/util/workerthreadscheduler.cpp
  src/util/xml.cpp
  src/waveform/guitick.cpp
  src/waveform/overviewtype.cpp
  src/waveform/renderers/glwaveformrenderbackground.cpp
  src/waveform/renderers/glvsynctestrenderer.cpp
  src/waveform/renderers/waveformmark.cpp
  src/waveform/renderers/waveformmarkrange.cpp
  src/waveform/renderers/waveformmarkset.cpp
  src/waveform/renderers/waveformoverviewrenderer.cpp
  src/waveform/renderers/waveformrenderbackground.cpp
  src/waveform/renderers/waveformrenderbeat.cpp
  src/waveform/renderers/waveformrendererabstract.cpp
  src/waveform/renderers/waveformrendererendoftrack.cpp
  src/waveform/renderers/waveformrendererfilteredsignal.cpp
  src/waveform/renderers/waveformrendererhsv.cpp
  src/waveform/renderers/waveformrendererpreroll.cpp
  src/waveform/renderers/waveformrendererrgb.cpp
  src/waveform/renderers/waveformrenderersignalbase.cpp
  src/waveform/renderers/waveformrendermark.cpp
  src/waveform/renderers/waveformrendermarkbase.cpp
  src/waveform/renderers/waveformrendermarkrange.cpp
  src/waveform/renderers/waveformsignalcolors.cpp
  src/waveform/renderers/waveformwidgetrenderer.cpp
  src/waveform/sharedglcontext.cpp
  src/waveform/visualplayposition.cpp
  src/waveform/visualsmanager.cpp
  src/waveform/vsyncthread.cpp
  src/waveform/waveform.cpp
  src/waveform/waveformfactory.cpp
  src/waveform/waveformmarklabel.cpp
  src/waveform/waveformwidgetfactory.cpp
  src/waveform/widgets/emptywaveformwidget.cpp
  src/waveform/widgets/hsvwaveformwidget.cpp
  src/waveform/widgets/rgbwaveformwidget.cpp
  src/waveform/widgets/softwarewaveformwidget.cpp
  src/waveform/widgets/waveformwidgetabstract.cpp
  src/waveform/widgets/glwaveformwidgetabstract.cpp
  src/waveform/widgets/glvsynctestwidget.cpp
  src/widget/controlwidgetconnection.cpp
  src/widget/findonwebmenufactory.cpp
  src/widget/findonwebmenuservices/findonwebmenudiscogs.cpp
  src/widget/findonwebmenuservices/findonwebmenulastfm.cpp
  src/widget/findonwebmenuservices/findonwebmenusoundcloud.cpp
  src/widget/hexspinbox.cpp
  src/widget/hotcuedrag.cpp
  src/widget/paintable.cpp
  src/widget/wanalysislibrarytableview.cpp
  src/widget/wbasewidget.cpp
  src/widget/wbattery.cpp
  src/widget/wbeatspinbox.cpp
  src/widget/wbpmeditor.cpp
  src/widget/wcollapsiblegroupbox.cpp
  src/widget/wcolorpicker.cpp
  src/widget/wcolorpickeraction.cpp
  src/widget/wcombobox.cpp
  src/widget/wcoverart.cpp
  src/widget/wcoverartlabel.cpp
  src/widget/wcoverartmenu.cpp
  src/widget/wcuemenupopup.cpp
  src/widget/wcuebutton.cpp
  src/widget/wdisplay.cpp
  src/widget/weffectbuttonparametername.cpp
  src/widget/weffectchain.cpp
  src/widget/weffectchainpresetbutton.cpp
  src/widget/weffectchainpresetselector.cpp
  src/widget/weffectknobparametername.cpp
  src/widget/weffectname.cpp
  src/widget/weffectmetaknob.cpp
  src/widget/weffectparameterknob.cpp
  src/widget/weffectparameterknobcomposed.cpp
  src/widget/weffectparameternamebase.cpp
  src/widget/weffectpushbutton.cpp
  src/widget/weffectselector.cpp
  src/widget/wfindonwebmenu.cpp
  src/widget/whotcuebutton.cpp
  src/widget/wimagestore.cpp
  src/widget/wkey.cpp
  src/widget/wknob.cpp
  src/widget/wknobcomposed.cpp
  src/widget/wlabel.cpp
  src/widget/wlibrary.cpp
  src/widget/wlibrarysidebar.cpp
  src/widget/wlibrarytableview.cpp
  src/widget/wlibrarytextbrowser.cpp
  src/widget/wmainmenubar.cpp
  src/widget/wmenucheckbox.cpp
  src/widget/wnumber.cpp
  src/widget/wnumberdb.cpp
  src/widget/wnumberpos.cpp
  src/widget/wnumberrate.cpp
  src/widget/woverview.cpp
  src/widget/wpixmapstore.cpp
  src/widget/wplaybutton.cpp
  src/widget/wpushbutton.cpp
  src/widget/wraterange.cpp
  src/widget/wstarratingaction.cpp
  src/widget/wrecordingduration.cpp
  src/widget/wscrollable.cpp
  src/widget/wsearchlineedit.cpp
  src/widget/wsearchrelatedtracksmenu.cpp
  src/widget/wsettingscheckboxlabel.cpp
  src/widget/wsingletoncontainer.cpp
  src/widget/wsizeawarestack.cpp
  src/widget/wskincolor.cpp
  src/widget/wslidercomposed.cpp
  src/widget/wspinny.cpp
  src/widget/wspinnybase.cpp
  src/widget/wsplitter.cpp
  src/widget/wstarrating.cpp
  src/widget/wstatuslight.cpp
  src/widget/wtime.cpp
  src/widget/wtrackmenu.cpp
  src/widget/wtrackproperty.cpp
  src/widget/wtracktableview.cpp
  src/widget/wtracktableviewheader.cpp
  src/widget/wtrackwidgetgroup.cpp
  src/widget/wvumeter.cpp
  src/widget/wvumeterbase.cpp
  src/widget/wvumeterlegacy.cpp
  src/widget/wwaveformviewer.cpp
  src/widget/wwidget.cpp
  src/widget/wwidgetgroup.cpp
  src/widget/wwidgetstack.cpp
  src/controllers/scripting/javascriptplayerproxy.cpp
  src/controllers/scripting/javascriptplayerproxy.h
)
set(MIXXX_COMMON_PRECOMPILED_HEADER src/util/assert.h)
set(
  MIXXX_LIB_PRECOMPILED_HEADER
  src/audio/frame.h
  src/audio/signalinfo.h
  src/audio/streaminfo.h
  src/audio/types.h
  src/control/control.h
  src/control/controlaudiotaperpot.h
  src/control/controlbehavior.h
  src/control/controlbuttonmode.h
  src/control/controlcompressingproxy.h
  src/control/controleffectknob.h
  src/control/controlencoder.h
  src/control/controlindicator.h
  src/control/controlindicatortimer.h
  src/control/controllinpotmeter.h
  src/control/controllogpotmeter.h
  src/control/controlmodel.h
  src/control/controlobject.h
  src/control/controlobjectscript.h
  src/control/controlpotmeter.h
  src/control/controlproxy.h
  src/control/controlpushbutton.h
  src/control/controlsortfiltermodel.h
  src/control/controlttrotary.h
  src/control/controlvalue.h
  src/control/convert.h
  src/control/pollingcontrolproxy.h
  src/controllers/defs_controllers.h
  src/defs_urls.h
  src/effects/defs.h
  src/engine/channelhandle.h
  src/engine/engine.h
  src/errordialoghandler.h
  src/track/track.h
  src/track/track_decl.h
  src/track/trackid.h
  src/track/trackinfo.h
  src/track/trackiterator.h
  src/track/trackmetadata.h
  src/track/tracknumbers.h
  src/track/trackrecord.h
  src/track/trackref.h
  src/util/always_false_v.h
  src/util/alphabetafilter.h
  src/util/battery/battery.h
  src/util/cache.h
  src/util/circularbuffer.h
  src/util/class.h
  src/util/cmdlineargs.h
  src/util/color/color.h
  src/util/color/colorpalette.h
  src/util/color/predefinedcolorpalettes.h
  src/util/color/rgbcolor.h
  src/util/colorcomponents.h
  src/util/compatibility/qatomic.h
  src/util/compatibility/qbytearray.h
  src/util/compatibility/qhash.h
  src/util/compatibility/qmutex.h
  src/util/console.h
  src/util/counter.h
  src/util/datetime.h
  src/util/db/dbconnection.h
  src/util/db/dbconnectionpool.h
  src/util/db/dbconnectionpooled.h
  src/util/db/dbconnectionpooler.h
  src/util/db/dbentity.h
  src/util/db/dbfieldindex.h
  src/util/db/dbid.h
  src/util/db/dbnamedentity.h
  src/util/db/fwdsqlquery.h
  src/util/db/fwdsqlqueryselectresult.h
  src/util/db/sqlite.h
  src/util/db/sqllikewildcards.h
  src/util/db/sqlqueryfinisher.h
  src/util/db/sqlstorage.h
  src/util/db/sqlstringformatter.h
  src/util/db/sqlsubselectmode.h
  src/util/db/sqltransaction.h
  src/util/debug.h
  src/util/defs.h
  src/util/denormalsarezero.h
  src/util/desktophelper.h
  src/util/dnd.h
  src/util/duration.h
  src/util/event.h
  src/util/experiment.h
  src/util/fifo.h
  src/util/file.h
  src/util/fileaccess.h
  src/util/fileinfo.h
  src/util/filename.h
  src/util/font.h
  src/util/fpclassify.h
  src/util/gitinfostore.h
  src/util/imagefiledata.h
  src/util/imageutils.h
  src/util/indexrange.h
  src/util/itemiterator.h
  src/util/lcs.h
  src/util/logger.h
  src/util/logging.h
  src/util/mac.h
  src/util/macros.h
  src/util/math.h
  src/util/messagepipe.h
  src/util/movinginterquartilemean.h
  src/util/mutex.h
  src/util/optional.h
  src/util/painterscope.h
  src/util/parented_ptr.h
  src/util/path.h
  src/util/performancetimer.h
  src/util/platform.h
  src/util/qt.h
  src/util/quuid.h
  src/util/rampingvalue.h
  src/util/rangelist.h
  src/util/readaheadsamplebuffer.h
  src/util/regex.h
  src/util/rescaler.h
  src/util/ringdelaybuffer.h
  src/util/rotary.h
  src/util/runtimeloggingcategory.h
  src/util/safelywritablefile.h
  src/util/sample.h
  src/util/samplebuffer.h
  src/util/sandbox.h
  src/util/scopedoverridecursor.h
  src/util/screensaver.h
  src/util/screensavermanager.h
  src/util/semanticversion.h
  src/util/singleton.h
  src/util/span.h
  src/util/stat.h
  src/util/statmodel.h
  src/util/statsmanager.h
  src/util/string.h
  src/util/stringformat.h
  src/util/tapfilter.h
  src/util/task.h
  src/util/taskmonitor.h
  src/util/thread_affinity.h
  src/util/thread_annotations.h
  src/util/time.h
  src/util/timer.h
  src/util/trace.h
  src/util/translations.h
  src/util/types.h
  src/util/unique_ptr_vector.h
  src/util/valuetransformer.h
  src/util/versionstore.h
  src/util/widgethelper.h
  src/util/workerthread.h
  src/util/workerthreadscheduler.h
  src/util/xml.h
)
if(NOT QML)
  target_sources(
    mixxx-lib
    PRIVATE
      # The following sources need to be in the QML target in order to get QML_ELEMENT properly interpreted.
      # However, if we build Mixxx without QML support, these are still required, so it gets appended to the
      # main target
      src/control/controlmodel.cpp
      src/control/controlsortfiltermodel.cpp
  )
else()
  target_sources(
    mixxx-lib
    PRIVATE
      # The following source depends of QML being available but aren't part of the new QML UI
      src/controllers/rendering/controllerrenderingengine.cpp
      src/controllers/controllerenginethreadcontrol.cpp
      src/controllers/controllerscreenpreview.cpp
  )
endif()
if(QOPENGL)
  target_sources(
    mixxx-lib
    PRIVATE
      src/shaders/endoftrackshader.cpp
      src/shaders/slipmodeshader.cpp
      src/shaders/patternshader.cpp
      src/shaders/rgbashader.cpp
      src/shaders/rgbshader.cpp
      src/shaders/shader.cpp
      src/shaders/textureshader.cpp
      src/shaders/unicolorshader.cpp
      src/shaders/vinylqualityshader.cpp
      src/util/opengltexture2d.cpp
      src/waveform/renderers/allshader/digitsrenderer.cpp
      src/waveform/renderers/allshader/matrixforwidgetgeometry.cpp
      src/waveform/renderers/allshader/waveformrenderbackground.cpp
      src/waveform/renderers/allshader/waveformrenderbeat.cpp
      src/waveform/renderers/allshader/waveformrenderer.cpp
      src/waveform/renderers/allshader/waveformrendererendoftrack.cpp
      src/waveform/renderers/allshader/waveformrendererslipmode.cpp
      src/waveform/renderers/allshader/waveformrendererfiltered.cpp
      src/waveform/renderers/allshader/waveformrendererhsv.cpp
      src/waveform/renderers/allshader/waveformrendererpreroll.cpp
      src/waveform/renderers/allshader/waveformrendererrgb.cpp
      src/waveform/renderers/allshader/waveformrenderertextured.cpp
      src/waveform/renderers/allshader/waveformrenderersignalbase.cpp
      src/waveform/renderers/allshader/waveformrenderersimple.cpp
      src/waveform/renderers/allshader/waveformrendermark.cpp
      src/waveform/renderers/allshader/waveformrendermarkrange.cpp
      src/waveform/widgets/allshader/waveformwidget.cpp
      src/widget/openglwindow.cpp
      src/widget/tooltipqopengl.cpp
      src/widget/wglwidgetqopengl.cpp
      src/widget/winitialglwidget.cpp
      src/widget/wspinnyglsl.cpp
      src/widget/wvumeterglsl.cpp
  )
else()
  target_sources(
    mixxx-lib
    PRIVATE
      src/waveform/renderers/qtvsynctestrenderer.cpp
      src/waveform/renderers/qtwaveformrendererfilteredsignal.cpp
      src/waveform/renderers/qtwaveformrenderersimplesignal.cpp
      src/widget/wglwidgetqglwidget.cpp
  )
endif()

set_source_files_properties(
  src/util/moc_included_test.cpp
  PROPERTIES SKIP_PRECOMPILE_HEADERS ON
)

set_target_properties(
  mixxx-lib
  PROPERTIES AUTOMOC ON AUTOUIC ON CXX_CLANG_TIDY "${CLANG_TIDY}"
)
target_include_directories(
  mixxx-lib
  PUBLIC src "${CMAKE_CURRENT_BINARY_DIR}/src"
)
if(UNIX AND NOT APPLE)
  target_sources(mixxx-lib PRIVATE src/util/rlimit.cpp)
  set(MIXXX_SETTINGS_PATH ".mixxx/")
endif()

if(APPLE)
  enable_language(OBJC OBJCXX)

  # Enable Automatic Reference Counting (ARC) when compiling Objective-C(++).
  # This frees us from having to worry about memory management when interfacing
  # with Apple frameworks (e.g. as in itunesmacosimporter.mm) since the compiler
  # will automatically insert retain/release calls on Objective-C objects.
  target_compile_options(mixxx-lib PUBLIC -fobjc-arc)

  # Disable deprecation warnings for OpenGL on macOS as we won't switch to
  # Apple's Metal API in the foreseeable future.
  target_compile_definitions(mixxx-lib PUBLIC GL_SILENCE_DEPRECATION)

  target_sources(mixxx-lib PRIVATE src/util/appleosversion.mm)

  if(IOS)
    target_sources(
      mixxx-lib
      PRIVATE src/soundio/soundmanagerios.mm src/util/screensaverios.mm
    )

    option(IOS_ITUNES_LIBRARY "Native iOS music library integration" ON)
    if(IOS_ITUNES_LIBRARY)
      target_sources(
        mixxx-lib
        PRIVATE
          src/library/itunes/itunesiosassetexporter.mm
          src/library/itunes/itunesiosimporter.mm
          src/library/itunes/itunesiostrackresolver.cpp
      )
      target_link_libraries(mixxx-lib PRIVATE "-weak_framework MediaPlayer")
      target_compile_definitions(mixxx-lib PUBLIC __IOS_ITUNES_LIBRARY__)
    endif()
  else()
    target_sources(mixxx-lib PRIVATE src/util/darkappearance.mm)

    option(
      MACOS_ITUNES_LIBRARY
      "Native macOS iTunes/Music.app library integration"
      ON
    )
    if(MACOS_ITUNES_LIBRARY)
      target_sources(
        mixxx-lib
        PRIVATE src/library/itunes/itunesmacosimporter.mm
      )
      target_link_libraries(mixxx-lib PRIVATE "-weak_framework iTunesLibrary")
      target_compile_definitions(mixxx-lib PUBLIC __MACOS_ITUNES_LIBRARY__)
    endif()
  endif()

  option(AU_EFFECTS "Audio Unit (AU) effects integration" ON)
  if(AU_EFFECTS)
    target_sources(
      mixxx-lib
      PRIVATE
        src/effects/backends/audiounit/audiounitbackend.mm
        src/effects/backends/audiounit/audiounitmanager.mm
        src/effects/backends/audiounit/audiouniteffectprocessor.mm
        src/effects/backends/audiounit/audiounitmanifest.mm
    )
    target_link_libraries(
      mixxx-lib
      PRIVATE "-weak_framework AudioToolbox" "-weak_framework AVFAudio"
    )
    target_compile_definitions(mixxx-lib PUBLIC __AU_EFFECTS__)
  endif()
endif()

if(EMSCRIPTEN)
  # We need asyncify to support asynchronous calls (e.g. QDialog::exec)
  # when targeting Emscripten/WebAssembly.
  # See https://doc.qt.io/qt-6/wasm.html#asyncify
  target_link_options(mixxx-lib PUBLIC -sASYNCIFY)
endif()

# QML Debugging
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(mixxx-lib PUBLIC QT_QML_DEBUG)
  message(
    STATUS
    "Enabling QML Debugging! This poses a security risk as Mixxx will open a TCP port for debugging"
  )
endif()

option(WARNINGS_PEDANTIC "Let the compiler show even more warnings" OFF)
if(MSVC)
  if(WARNINGS_PEDANTIC)
    target_compile_options(mixxx-lib PUBLIC /W4)
  else()
    target_compile_options(
      mixxx-lib
      PUBLIC
        /W3 # Warning Level 3 (production quality)
        /wd4200 # C4200: nonstandard extension used: zero-sized array in struct/union
      # Note: Even with CMAKE_C_STANDARD = 99 MSVC does not complain about C99 flexible array members
    )
    target_compile_definitions(
      mixxx-lib
      PUBLIC
        # Eve: to prevent multiple instances of winsock(2) being loaded
        WIN32_LEAN_AND_MEAN
        _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
        _CRT_SECURE_NO_WARNINGS
    )
  endif()
else()
  # TODO: Add -Wtrampolines, not yet supported by clazy
  target_compile_options(
    mixxx-lib
    PUBLIC
      -Wall
      -Wextra
      $<$<COMPILE_LANGUAGE:CXX>:-Woverloaded-virtual>
      -Wfloat-conversion
      -Werror=return-type
      -Wformat=2
      -Wformat-security
      -Wvla
      -Wundef
  )
  if(WARNINGS_PEDANTIC)
    target_compile_options(mixxx-lib PUBLIC -pedantic)
  endif()
endif()

option(INFO_VECTORIZE "Let the compiler show vectorized loops" OFF)
if(INFO_VECTORIZE)
  if(MSVC)
    target_compile_options(mixxx-lib PUBLIC /Qvec-report:1)
  elseif(GNU_GCC)
    target_compile_options(mixxx-lib PUBLIC -fopt-info-vec-optimized)
  elseif(LLVM_CLANG)
    target_compile_options(mixxx-lib PUBLIC -Rpass=loop-vectorize)
  else()
    message(STATUS "INFO_VECTORIZE not implemented for this compiler.")
  endif()
endif()

option(RELATIVE_MACRO_PATHS "Relativize __FILE__ paths" ON)
if(RELATIVE_MACRO_PATHS)
  if(NOT MSVC)
    target_compile_options(
      mixxx-lib
      PUBLIC "-fmacro-prefix-map=${CMAKE_SOURCE_DIR}=."
    )
  endif()
endif()

option(WARNINGS_FATAL "Fail if compiler generates a warning" OFF)
if(WARNINGS_FATAL)
  if(MSVC)
    target_compile_options(mixxx-lib PUBLIC /WX)
  else()
    target_compile_options(mixxx-lib PUBLIC -Werror)
  endif()
endif()

target_compile_definitions(
  mixxx-lib
  PUBLIC
    "${CMAKE_SYSTEM_PROCESSOR}"
    $<$<CONFIG:Debug>:MIXXX_BUILD_DEBUG>
    $<$<CONFIG:Debug>:MIXXX_DEBUG_ASSERTIONS_ENABLED>
    $<$<NOT:$<CONFIG:Debug>>:MIXXX_BUILD_RELEASE>
)

# Mac-specific options
#
# These options are OFF by default, and since they are only available on macOS,
# they are forcibly set to OFF on all other platforms.
cmake_dependent_option(
  MACOS_BUNDLE
  "Install files to proper locations to make an .app bundle"
  OFF
  "APPLE"
  OFF
)
cmake_dependent_option(
  MACAPPSTORE
  "Build for Mac App Store"
  OFF
  "APPLE"
  OFF
)
if(MACAPPSTORE)
  target_compile_definitions(mixxx-lib PUBLIC __MACAPPSTORE__)
endif()

# Windows-specific options
if(WIN32)
  # https://docs.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt
  # _WIN32_WINNT_WIN7 = 0x0601
  target_compile_definitions(mixxx-lib PUBLIC WINVER=0x0601)
  target_compile_definitions(mixxx-lib PUBLIC _WIN32_WINNT=0x0601)
  # Eve: to prevent multiple instances of winsock(2) being loaded
  target_compile_definitions(mixxx-lib PUBLIC WIN32_LEAN_AND_MEAN)
  if(MSVC)
    target_compile_definitions(mixxx-lib PUBLIC _USE_MATH_DEFINES)
  endif()
endif()

#
# Installation directories
#
set(MIXXX_INSTALL_BINDIR ".")
set(MIXXX_INSTALL_DATADIR ".")
set(MIXXX_INSTALL_DOCDIR "./doc")
set(MIXXX_INSTALL_LICENSEDIR "./doc")
if(APPLE AND MACOS_BUNDLE)
  set(MIXXX_INSTALL_BINDIR "${CMAKE_INSTALL_BINDIR}")
  set(
    MACOS_BUNDLE_NAME
    Mixxx
    CACHE STRING
    "The macOS app bundle and executable name"
  )
  set(
    MACOS_BUNDLE_IDENTIFIER
    org.mixxx.mixxx
    CACHE STRING
    "The macOS app bundle identifier"
  )
  set(MIXXX_INSTALL_PREFIX "${MACOS_BUNDLE_NAME}.app")
  set(MIXXX_INSTALL_DATADIR "${MIXXX_INSTALL_PREFIX}/Contents/Resources")
  set(MIXXX_INSTALL_DOCDIR "${MIXXX_INSTALL_DATADIR}")
  set(MIXXX_INSTALL_LICENSEDIR "${MIXXX_INSTALL_DATADIR}/licenses")
elseif(APPLE AND IOS)
  set(MIXXX_INSTALL_BINDIR "${CMAKE_INSTALL_BINDIR}")
  set(
    IOS_BUNDLE_NAME
    Mixxx
    CACHE STRING
    "The iOS app bundle and executable name"
  )
  set(
    IOS_BUNDLE_IDENTIFIER
    org.mixxx.mixxx
    CACHE STRING
    "The iOS app bundle identifier"
  )
elseif(UNIX)
  set(MIXXX_INSTALL_BINDIR "${CMAKE_INSTALL_BINDIR}")
  set(MIXXX_INSTALL_DATADIR "${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}")
  set(MIXXX_INSTALL_DOCDIR "${CMAKE_INSTALL_DOCDIR}")
  set(MIXXX_INSTALL_LICENSEDIR "${CMAKE_INSTALL_DOCDIR}")
endif()

if(WIN32)
  target_compile_definitions(mixxx-lib PUBLIC __WINDOWS__)

  # Helps prevent duplicate symbols
  target_compile_definitions(mixxx-lib PUBLIC _ATL_MIN_CRT)

  # Need this on Windows until we have UTF16 support in Mixxx use stl min max
  # defines
  # http://connect.microsoft.com/VisualStudio/feedback/details/553420/std-cpp-
  # max-and-std-cpp-min-not-available-in-visual-c-2010
  target_compile_definitions(mixxx-lib PUBLIC NOMINMAX UNICODE)

  # shoutidjc/shout.h checks for WIN32 to see if we are on Windows.
  target_compile_definitions(mixxx-lib PUBLIC WIN32)

  target_link_libraries(mixxx-lib PRIVATE comctl32 shell32)

  if(MSVC)
    target_link_options(mixxx-lib PUBLIC /entry:mainCRTStartup)
    # Force MSVS to generate a manifest (MSVC2010)
    target_link_options(mixxx-lib PUBLIC /manifest)
  endif()
elseif(UNIX)
  if(APPLE)
    target_compile_definitions(mixxx-lib PUBLIC __APPLE__)
  else()
    target_compile_definitions(mixxx-lib PUBLIC __UNIX__)
    if(CMAKE_SYSTEM_NAME STREQUAL Linux)
      target_compile_definitions(mixxx-lib PUBLIC __LINUX__)
    elseif(CMAKE_SYSTEM_NAME MATCHES "^.*BSD$")
      target_compile_definitions(mixxx-lib PUBLIC __BSD__)
    endif()
  endif()
endif()

# The mixxx executable
if(QT6)
  find_package(Qt6 COMPONENTS Core) # For Qt Core cmake functions
  # This is the first package form the environment, if this fails give hints how to install the environment
  if(NOT Qt6_FOUND)
    fatal_error_missing_env()
  endif()
  # qt_add_executable() is the recommended initial call for qt_finalize_target()
  # below that takes care of the correct object order in the resulting binary
  # According to https://doc.qt.io/qt-6/qt-finalize-target.html it is importand for
  # builds with Qt < 3.21
  # Eve: to prevent multiple instances of winsock(2) being loaded
  ### add_definitions(-DWIN32_LEAN_AND_MEAN)
  qt_add_executable(mixxx WIN32 src/main.cpp MANUAL_FINALIZATION)
else()
  find_package(Qt5 COMPONENTS Core) # For Qt Core cmake functions
  # This is the first package form the environment, if this fails give hints how to install the environment
  if(NOT Qt5_FOUND)
    fatal_error_missing_env()
  endif()
  add_executable(mixxx WIN32 src/main.cpp)
endif()

set_target_properties(mixxx-lib PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY}")
target_link_libraries(mixxx PRIVATE mixxx-lib mixxx-gitinfostore)

#
# Installation and Packaging
#
if(APPLE)
  if(IOS)
    set(IOS_BUNDLE_VERSION "${CMAKE_PROJECT_VERSION}")
    set(IOS_BUNDLE_SHORTVERSION "${CMAKE_PROJECT_VERSION}")

    # https://stackoverflow.com/questions/54830302/how-to-add-the-copy-bundle-resources-phase-with-cmake-in-xcode
    # https://discourse.cmake.org/t/ios-resource-management/758/7
    file(GLOB IOS_RESOURCES res/**)
    list(APPEND IOS_RESOURCES packaging/ios/Assets.xcassets)
    target_sources(mixxx PUBLIC ${IOS_RESOURCES})
    set_source_files_properties(
      ${IOS_RESOURCES}
      PROPERTIES MACOSX_PACKAGE_LOCATION Resources
    )
    source_group(Resources FILES ${IOS_RESOURCES})

    set(
      QT_IOS_LAUNCH_SCREEN
      "${CMAKE_CURRENT_SOURCE_DIR}/packaging/ios/LaunchScreen.storyboard"
    )

    set_target_properties(
      mixxx
      PROPERTIES
        MACOSX_BUNDLE true
        OUTPUT_NAME "${IOS_BUNDLE_NAME}"
        MACOSX_BUNDLE_BUNDLE_NAME "${IOS_BUNDLE_NAME}"
        MACOSX_BUNDLE_INFO_PLIST
          "${CMAKE_CURRENT_SOURCE_DIR}/packaging/ios/Info.plist.in"
        MACOSX_BUNDLE_GUI_IDENTIFIER "${IOS_BUNDLE_IDENTIFIER}"
        XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon"
    )
  elseif(MACOS_BUNDLE)
    install(
      FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/osx/application.icns"
      DESTINATION ${MIXXX_INSTALL_DATADIR}
    )

    set(MACOS_BUNDLE_VERSION "${CMAKE_PROJECT_VERSION}")
    set(MACOS_BUNDLE_SHORTVERSION "${CMAKE_PROJECT_VERSION}")

    set_target_properties(
      mixxx
      PROPERTIES
        MACOSX_BUNDLE true
        OUTPUT_NAME "${MACOS_BUNDLE_NAME}"
        MACOSX_BUNDLE_INFO_PLIST
          "${CMAKE_CURRENT_SOURCE_DIR}/packaging/macos/Info.plist.in"
    )
  endif()
endif()

if(EMSCRIPTEN)
  # Package resources for the web using preloading.
  # This will generate a mixxx.data file containing all the resources.
  # See https://emscripten.org/docs/porting/files/packaging_files.html
  # TODO: Strip this down by only including what we need (i.e. no macOS/Linux packaging, ...)
  target_link_options(
    mixxx-lib
    PUBLIC "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/res@/res"
  )
endif()

if(WIN32)
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "${MIXXX_INSTALL_BINDIR}")
  if(MSVC AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_INSTALL_UCRT_LIBRARIES true)
    set(CMAKE_INSTALL_DEBUG_LIBRARIES true)
    set(CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY true)
  endif()
  include(InstallRequiredSystemLibraries)
endif()

install(
  TARGETS mixxx
  RUNTIME DESTINATION "${MIXXX_INSTALL_BINDIR}"
  BUNDLE DESTINATION .
)

# Skins
install(
  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/res/skins"
  DESTINATION "${MIXXX_INSTALL_DATADIR}"
)

# Controller mappings
install(
  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/res/controllers"
  DESTINATION "${MIXXX_INSTALL_DATADIR}"
)

# Effect presets
install(
  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/res/effects"
  DESTINATION "${MIXXX_INSTALL_DATADIR}"
)

# Translation files
install(
  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/res/translations"
  DESTINATION "${MIXXX_INSTALL_DATADIR}"
  FILES_MATCHING
  PATTERN "*.qm"
)

# Font files
#
# Font installation is only enabled on Windows and macOS, because on Linux/BSD
# fonts should be installed via the package manager. Whenever a new font is
# added to Mixxx, its package name also needs to be added to
# tools/debian_buildenv.sh. If that font is not packaged on most distros, we
# need to re-enable font installation on Linux/BSD and exclude the packaged
# fonts here.
if(APPLE OR WIN32)
  install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/res/fonts"
    DESTINATION "${MIXXX_INSTALL_DATADIR}"
  )
endif()

# Keyboard mapping(s)
install(
  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/res/keyboard"
  DESTINATION "${MIXXX_INSTALL_DATADIR}"
)

# Licenses
install(
  FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
    "${CMAKE_CURRENT_SOURCE_DIR}/COPYING"
  DESTINATION "${MIXXX_INSTALL_LICENSEDIR}"
)

# Documentation
install(
  FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
    "${CMAKE_CURRENT_SOURCE_DIR}/res/Mixxx-Keyboard-Shortcuts.pdf"
  DESTINATION "${MIXXX_INSTALL_DOCDIR}"
)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/res/Mixxx-Manual.pdf")
  install(
    FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/Mixxx-Manual.pdf"
    DESTINATION "${MIXXX_INSTALL_DOCDIR}"
  )
endif()

# Additional Linux-only files
if(UNIX AND NOT APPLE)
  # .desktop file for KDE/GNOME menu
  install(
    FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/linux/org.mixxx.Mixxx.desktop"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/applications"
  )

  # Icon files for menu entry
  install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/res/images/icons/"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor"
    # This file is for Windows.
    PATTERN ic_mixxx.ico EXCLUDE
  )

  # .metainfo.xml file for KDE/GNOME AppStream initiative
  install(
    FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/linux/org.mixxx.Mixxx.metainfo.xml"
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/metainfo"
  )

  option(
    INSTALL_USER_UDEV_RULES
    "Install user udev rule file for USB HID and Bulk controllers"
    ON
  )
  if(INSTALL_USER_UDEV_RULES)
    set(MIXXX_UDEVDIR "${MIXXX_INSTALL_DATADIR}/udev")
    if(
      CMAKE_INSTALL_PREFIX STREQUAL "/usr"
      OR CMAKE_INSTALL_PREFIX STREQUAL "/"
    )
      # /usr and / install prefixes at treated by cmake GNUInstallDirs as
      # synonym for "system location". In this case we can look up the correct udevdir
      # using pkg-config.
      # See: https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html#special-cases
      find_package(PkgConfig)
      if(PKG_CONFIG_FOUND)
        pkg_check_modules(PKGCONFIG_UDEV udev)
        if(PKGCONFIG_UDEV_FOUND)
          execute_process(
            COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=udevdir udev
            OUTPUT_VARIABLE PKGCONFIG_UDEVDIR
            OUTPUT_STRIP_TRAILING_WHITESPACE
          )
          if(PKGCONFIG_UDEVDIR)
            file(TO_CMAKE_PATH "${PKGCONFIG_UDEVDIR}" MIXXX_UDEVDIR)
          endif()
        endif()
      endif()
    endif()
    if(MIXXX_UDEVDIR STREQUAL "${MIXXX_INSTALL_DATADIR}/udev")
      install(
        FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/linux/mixxx-usb-uaccess.rules"
        DESTINATION "${MIXXX_UDEVDIR}/rules.d"
      )
      install(
        CODE
          "
      message(STATUS \"Important Note: Installation of udev rules\n\"
          \"The udev rule file for USB HID and Bulk controller permissions have been\n\"
          \"installed to:\n\"
          \"    ${MIXXX_UDEVDIR}/rules.d.\n\"
          \"If you are installing Mixxx from source for your own use, copy\n\"
          \"mixxx-usb-uaccess.rules to /etc/udev/rules.d/ and run:\n\"
          \"    udevadm control --reload-rules && udevadm trigger\n\"
          \"as root to load the rules.\n\"
          \"If you are building a package for a distribution, the correct directory for\n\"
          \"system rules is either /lib/udev/rules.d (e.g. Debian, Fedora) or\n\"
          \"/usr/lib/udev/rules.d (e.g. Arch Linux) with an appropriate priority prefix.\n\"
          \"Adjust your package script accordingly and set -DINSTALL_USER_UDEV_RULES=OFF\")
      "
      )
    else()
      install(
        FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/linux/mixxx-usb-uaccess.rules"
        DESTINATION "${MIXXX_UDEVDIR}/rules.d"
        RENAME "69-mixxx-usb-uaccess.rules"
      )
    endif()
  endif()
endif()

if(MSVC)
  # install debug symbols if any were generated
  install(
    FILES $<TARGET_PDB_FILE:mixxx>
    CONFIGURATIONS Debug RelWithDebInfo
    DESTINATION "${MIXXX_INSTALL_BINDIR}"
    COMPONENT
      PDB # No spaces allowed
  )
endif()

if(WIN32 AND NOT QT6)
  # Qt 5 loads these ANGLE DLLs at runtime if the graphics driver is on the ignore list.
  # It does not work with Debug, because the debug version is compiled without the a d suffix
  find_package(unofficial-angle CONFIG REQUIRED)
  install(
    IMPORTED_RUNTIME_ARTIFACTS
      unofficial::angle::libEGL
      unofficial::angle::libGLESv2
      CONFIGURATIONS
      RelWithDebInfo
      Release
      DESTINATION
      "${MIXXX_INSTALL_BINDIR}"
      COMPONENT
      applocal
  )
  set(APPLOCAL_COMPONENT_DEFINED true)
endif()

#
# Tests
#

# Eve: to prevent multiple instances of winsock(2) being loaded
### add_definitions(-DWIN32_LEAN_AND_MEAN)

find_package(GTest CONFIG)
default_option(BUILD_TESTING "Build with Unittests" "GTest_FOUND")
if(BUILD_TESTING)
  if(GTest_FOUND)
    message(STATUS "Found GTest: Unittests enabled")
  else()
    message(FATAL_ERROR "GTest: not found")
  endif()
endif()

find_package(benchmark)
default_option(BUILD_BENCH "Build mixxx-benchmark" "benchmark_FOUND")
if(BUILD_BENCH AND BUILD_TESTING)
  if(benchmark_FOUND)
    message(STATUS "Found google-benchmark: mixxx-benchmark enabled")
  else()
    message(FATAL_ERROR "google-benchmark: not found")
  endif()
elseif(BUILD_BENCH AND NOT BUILD_TESTING)
  message(FATAL_ERROR "Benchmark needs Unittests (-DBUILD_TESTING=ON)")
endif()

# FFmpeg support
# FFmpeg is multimedia library that can be found http://ffmpeg.org/
find_package(FFMPEG COMPONENTS libavcodec libavformat libavutil libswresample)
default_option(FFMPEG "FFmpeg support (version 4.1.9 or later)" "FFMPEG_FOUND")
if(FFMPEG)
  if(NOT FFMPEG_FOUND)
    message(FATAL_ERROR "FFMPEG was not found")
  endif()

  # Check minimum required versions
  # Minimum library versions according to <https://ffmpeg.org/download.html>
  # Windows: Version numbers are not available!?
  # macOS: Untested
  if(
    FFMPEG_libavcodec_VERSION
    AND FFMPEG_libavcodec_VERSION VERSION_LESS 58.35.100
  )
    message(
      FATAL_ERROR
      "FFmpeg support requires at least version 58.35.100 of libavcodec (found: ${FFMPEG_libavcodec_VERSION})."
    )
  endif()
  if(
    FFMPEG_libavformat_VERSION
    AND FFMPEG_libavformat_VERSION VERSION_LESS 58.20.100
  )
    message(
      FATAL_ERROR
      "FFmpeg support requires at least version 58.20.100 of libavformat (found: ${FFMPEG_libavformat_VERSION})."
    )
  endif()
  if(
    FFMPEG_libavutil_VERSION
    AND FFMPEG_libavutil_VERSION VERSION_LESS 56.22.100
  )
    message(
      FATAL_ERROR
      "FFmpeg support requires at least version 56.22.100 of libavutil (found: ${FFMPEG_libavutil_VERSION})."
    )
  endif()
  if(
    FFMPEG_libswresample_VERSION
    AND FFMPEG_libswresample_VERSION VERSION_LESS 3.3.100
  )
    message(
      FATAL_ERROR
      "FFmpeg support requires at least version 3.3.100 of libswresample (found: ${FFMPEG_libswresample_VERSION})."
    )
  endif()

  target_sources(mixxx-lib PRIVATE src/sources/soundsourceffmpeg.cpp)
  target_compile_definitions(
    mixxx-lib
    PUBLIC
      __FFMPEG__
      # Needed to build new FFmpeg
      __STDC_CONSTANT_MACROS
      __STDC_LIMIT_MACROS
      __STDC_FORMAT_MACROS
  )
  target_link_libraries(mixxx-lib PRIVATE "${FFMPEG_LIBRARIES}")
  target_include_directories(mixxx-lib PUBLIC "${FFMPEG_INCLUDE_DIRS}")
endif()

# STEM file support
default_option(STEM "STEM file support" "FFMPEG_FOUND;FFMPEG")
if(STEM)
  if(NOT FFMPEG)
    message(FATAL_ERROR "STEM requires that also FFMPEG is enabled")
  endif()
  target_compile_definitions(mixxx-lib PUBLIC __STEM__)
  list(APPEND MIXXX_LIB_PRECOMPILED_HEADER src/track/steminfo.h)
  target_sources(
    mixxx-lib
    PRIVATE
      src/sources/soundsourcestem.cpp
      src/track/steminfoimporter.cpp
      src/track/steminfo.cpp
      src/widget/wtrackstemmenu.cpp
      src/widget/wstemlabel.cpp
  )
  if(QOPENGL)
    target_sources(
      mixxx-lib
      PRIVATE src/waveform/renderers/allshader/waveformrendererstem.cpp
    )
  endif()
endif()

if(BUILD_TESTING)
  set(
    src-mixxx-test
    src/test/analyserwaveformtest.cpp
    src/test/analyzersilence_test.cpp
    src/test/audiotaperpot_test.cpp
    src/test/autodjprocessor_test.cpp
    src/test/beatgridtest.cpp
    src/test/beatmaptest.cpp
    src/test/beatstest.cpp
    src/test/beatstranslatetest.cpp
    src/test/borrowabletest.cpp
    src/test/bpmtest.cpp
    src/test/bpmcontrol_test.cpp
    src/test/broadcastprofile_test.cpp
    src/test/broadcastsettings_test.cpp
    src/test/cache_test.cpp
    src/test/channelhandle_test.cpp
    src/test/chrono_clock_resolution_test.cpp
    src/test/colorconfig_test.cpp
    src/test/colormapperjsproxy_test.cpp
    src/test/colorpalette_test.cpp
    src/test/configobject_test.cpp
    src/test/controller_mapping_validation_test.cpp
    src/test/controller_mapping_settings_test.cpp
    src/test/controllers/controller_columnid_regression_test.cpp
    src/test/controllerscriptenginelegacy_test.cpp
    src/test/controlobjecttest.cpp
    src/test/controlobjectaliastest.cpp
    src/test/controlobjectscripttest.cpp
    src/test/controlpotmetertest.cpp
    src/test/coreservicestest.cpp
    src/test/coverartcache_test.cpp
    src/test/coverartutils_test.cpp
    src/test/cratestorage_test.cpp
    src/test/cue_test.cpp
    src/test/cuecontrol_test.cpp
    src/test/dbconnectionpool_test.cpp
    src/test/dbidtest.cpp
    src/test/directorydaotest.cpp
    src/test/duration_test.cpp
    src/test/durationutiltest.cpp
    #TODO: write useful tests for refactored effects system
    #src/test/effectchainslottest.cpp
    src/test/enginebufferscalelineartest.cpp
    src/test/enginebuffertest.cpp
    src/test/enginefilterbiquadtest.cpp
    src/test/enginemixertest.cpp
    src/test/enginemicrophonetest.cpp
    src/test/enginesynctest.cpp
    src/test/fileinfo_test.cpp
    src/test/frametest.cpp
    src/test/globaltrackcache_test.cpp
    src/test/hotcuecontrol_test.cpp
    src/test/hotcueorderbyposition_test.cpp
    src/test/imageutils_test.cpp
    src/test/indexrange_test.cpp
    src/test/itunesxmlimportertest.cpp
    src/test/keyfactorytest.cpp
    src/test/keyutilstest.cpp
    src/test/lcstest.cpp
    src/test/learningutilstest.cpp
    src/test/libraryscannertest.cpp
    src/test/librarytest.cpp
    src/test/looping_control_test.cpp
    src/test/main.cpp
    src/test/mathutiltest.cpp
    src/test/metadatatest.cpp
    #TODO: make this build again
    #src/test/metaknob_link_test.cpp
    src/test/midicontrollertest.cpp
    src/test/mixxxtest.cpp
    src/test/mock_networkaccessmanager.cpp
    src/test/musicbrainzrecordingstasktest.cpp
    #src/test/osc_liblo_test.cpp
    #src/test/osc_mixxx_integration_test.cpp
    src/test/performancetimer_test.cpp
    src/test/playcountertest.cpp
    src/test/playermanagertest.cpp
    src/test/playlisttest.cpp
    src/test/portmidicontroller_test.cpp
    src/test/portmidienumeratortest.cpp
    src/test/queryutiltest.cpp
    src/test/rangelist_test.cpp
    src/test/readaheadmanager_test.cpp
    src/test/replaygaintest.cpp
    src/test/rescalertest.cpp
    src/test/rgbcolor_test.cpp
    src/test/rotary_test.cpp
    src/test/samplebuffertest.cpp
    src/test/schemamanager_test.cpp
    src/test/searchqueryparsertest.cpp
    src/test/seratobeatgridtest.cpp
    src/test/seratomarkerstest.cpp
    src/test/seratomarkers2test.cpp
    src/test/seratotagstest.cpp
    src/test/signalpathtest.cpp
    src/test/skincontext_test.cpp
    src/test/softtakeover_test.cpp
    src/test/soundproxy_test.cpp
    src/test/soundsourceproviderregistrytest.cpp
    src/test/sqliteliketest.cpp
    src/test/synccontroltest.cpp
    src/test/synctrackmetadatatest.cpp
    src/test/tableview_test.cpp
    src/test/taglibtest.cpp
    src/test/trackdao_test.cpp
    src/test/trackexport_test.cpp
    src/test/trackmetadata_test.cpp
    src/test/trackmetadataexport_test.cpp
    src/test/tracknumberstest.cpp
    src/test/trackreftest.cpp
    src/test/trackupdate_test.cpp
    src/test/uuid_test.cpp
    src/test/wbatterytest.cpp
    src/test/wpushbutton_test.cpp
    src/test/wwidgetstack_test.cpp
    src/util/moc_included_test.cpp
    src/test/helpers/log_test.cpp
  )
  if(STEM)
    set(
      src-mixxx-test
      ${src-mixxx-test}
      src/test/stemtest.cpp
      src/test/steminfotest.cpp
      src/test/stemcontrolobjecttest.cpp
    )
  endif()
  if(BUILD_BENCH)
    set(
      src-mixxx-test
      ${src-mixxx-test}
      src/test/engineeffectsdelay_test.cpp
      src/test/movinginterquartilemean_test.cpp
      src/test/nativeeffects_test.cpp
      src/test/ringdelaybuffer_test.cpp
      src/test/sampleutiltest.cpp
      src/test/waveform_upgrade_test.cpp
    )
  endif()

  add_executable(mixxx-test ${src-mixxx-test})

  if(QML)
    target_sources(
      mixxx-test
      PRIVATE
        src/test/controller_mapping_file_handler_test.cpp
        src/test/controllerrenderingengine_test.cpp
    )
  endif()

  set_target_properties(mixxx-test PROPERTIES AUTOMOC ON)
  target_link_libraries(
    mixxx-test
    PRIVATE mixxx-lib mixxx-gitinfostore GTest::gtest GTest::gmock
  )

  if(BUILD_BENCH)
    add_compile_definitions(USE_BENCH)
    target_link_libraries(mixxx-test PRIVATE benchmark::benchmark)
  endif()

  # Test Suite
  include(CTest)
  include(GoogleTest)
  enable_testing()
  gtest_add_tests(
    TARGET mixxx-test
    EXTRA_ARGS --logLevel info
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    TEST_LIST testsuite
  )

  if(NOT WIN32)
    # Default to offscreen rendering during tests.
    # This is required if the build system like Fedora koji/mock does not
    # allow to pass environment variables into the ctest macro expansion.
    set_tests_properties(
      ${testsuite}
      PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )
  endif()

  if(BUILD_BENCH)
    # Benchmarking
    add_custom_target(
      mixxx-benchmark
      COMMAND $<TARGET_FILE:mixxx-test> --benchmark
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
      COMMENT "Mixxx Benchmarks"
      VERBATIM
    )
    add_dependencies(mixxx-benchmark mixxx-test)
  endif()
endif() # BUILD_TESTING

#
# Resources
#
# Add resources to mixxx and mixxx-test binaries, not the mixxx-lib static
# library. Doing this would require initialization using Q_INIT_RESOURCE()
# calls that are not present at the moment. Further information can be found
# at: https://doc.qt.io/qt5/resources.html#using-resources-in-a-library
option(DOWNLOAD_MANUAL "Download Manual PDF from Mixxx website" OFF)
if(
  DOWNLOAD_MANUAL
  AND NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/res/Mixxx-Manual.pdf"
)
  set(
    MANUAL_URL
    "https://downloads.mixxx.org/manual/${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}/mixxx-manual-${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}-en.pdf"
  )
  message(STATUS "Downloading manual from ${MANUAL_URL}...")
  file(
    DOWNLOAD "${MANUAL_URL}" "${CMAKE_CURRENT_BINARY_DIR}/res/Mixxx-Manual.pdf"
    SHOW_PROGRESS
    STATUS MANUAL_PDF_DOWNLOAD
    TLS_VERIFY ON
  )
  list(GET MANUAL_PDF_DOWNLOAD 0 MANUAL_PDF_DOWNLOAD_ERROR)
  if(NOT MANUAL_PDF_DOWNLOAD_ERROR EQUAL 0)
    list(GET MANUAL_PDF_DOWNLOAD 1 MANUAL_PDF_DOWNLOAD_MESSGAE)
    message(
      FATAL_ERROR
      "Manual PDF download failed with: "
      "${MANUAL_PDF_DOWNLOAD_MESSGAE} Code: ${MANUAL_PDF_DOWNLOAD_ERROR}. "
      "Either download it yourself and move it to "
      "'${CMAKE_CURRENT_SOURCE_DIR}/res/Mixxx-Manual.pdf' or "
      "reconfigure with -DDOWNLOAD_MANUAL=OFF to build without included "
      "manual."
    )
  endif()
  file(
    RENAME
    "${CMAKE_CURRENT_BINARY_DIR}/res/Mixxx-Manual.pdf"
    "${CMAKE_CURRENT_SOURCE_DIR}/res/Mixxx-Manual.pdf"
  )
endif()

target_sources(mixxx PRIVATE res/mixxx.qrc)
set_target_properties(mixxx PROPERTIES AUTORCC ON)
if(BUILD_TESTING)
  target_sources(mixxx-test PRIVATE res/mixxx.qrc)
  set_target_properties(mixxx-test PROPERTIES AUTORCC ON)
endif()

if(MIXXX_VERSION_PRERELEASE STREQUAL "")
  set(MIXXX_VERSION "${CMAKE_PROJECT_VERSION}")
else()
  set(MIXXX_VERSION "${CMAKE_PROJECT_VERSION}-${MIXXX_VERSION_PRERELEASE}")
endif()

get_target_property(MIXXX_BUILD_FLAGS mixxx-lib COMPILE_OPTIONS)

# uses CMAKE_PROJECT_VERSION MIXXX_VERSION_PRERELEASE MIXXX_BUILD_FLAGS
configure_file(src/version.h.in src/version.h @ONLY)

if(
  GIT_COMMIT_DATE
  AND
    NOT
      GIT_COMMIT_DATE
        MATCHES
        "^[0-9]*-[0-9]*-[0-9]*T[0-9]*\\:[0-9]*\\:[0-9]*[+-][0-9]*\\:[0-9]*$"
)
  message(
    FATAL_ERROR
    "GIT_COMMIT_DATE requires strict ISO 8601 format %Y-%m-%dT%H:%M:%SZ"
  )
endif()

add_custom_target(
  mixxx-gitinfo
  # Note: We don't quote the paths in the command since CMake already inserts
  # escapes (which, if quoted, lead to paths wrongly containing backslashes).
  # See https://stackoverflow.com/questions/8925396/why-does-cmake-prefixes-spaces-with-backslashes-when-executing-a-command
  COMMAND
    ${CMAKE_COMMAND} -DGIT_DESCRIBE=${GIT_DESCRIBE}
    -DGIT_COMMIT_DATE=${GIT_COMMIT_DATE}
    -DINPUT_FILE=${CMAKE_CURRENT_SOURCE_DIR}/src/gitinfo.h.in
    -DOUTPUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/src/gitinfo.h -P
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/scripts/gitinfo.cmake
  COMMENT "Update git version information in gitinfo.h"
  BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/src/gitinfo.h"
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

# Windows-only resource file
if(WIN32)
  # EVE
  ###add_definitions(-DWIN32_LEAN_AND_MEAN)
  string(TIMESTAMP MIXXX_YEAR "%Y")

  set(
    MIXXX_FILEVERSION
    "${CMAKE_PROJECT_VERSION_MAJOR},${CMAKE_PROJECT_VERSION_MINOR},${CMAKE_PROJECT_VERSION_PATCH}"
  )
  set(MIXXX_PRODUCTVERSION "${MIXXX_FILEVERSION}")

  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(MIXXX_DEBUG 1)
  else()
    set(MIXXX_DEBUG 0)
  endif()

  if(MIXXX_VERSION_PRERELEASE STREQUAL "")
    set(MIXXX_PRERELEASE 0)
  else()
    set(MIXXX_PRERELEASE 1)
  endif()

  # uses MIXXX_YEAR MIXXX_FILEVERSION MIXXX_PRODUCTVERSION MIXXX_VERSION MIXXX_DEBUG MIXXX_PRERELEASE
  configure_file("src/mixxx.rc.include.in" "src/mixxx.rc.include" @ONLY)
  add_dependencies(mixxx mixxx-gitinfo)

  target_sources(
    mixxx
    PRIVATE
      src/mixxx.rc
      "${CMAKE_CURRENT_BINARY_DIR}/src/mixxx.rc.include"
      "${CMAKE_CURRENT_BINARY_DIR}/src/gitinfo.h"
  )
  target_include_directories(mixxx PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

# Chromaprint
find_package(Chromaprint)
if(NOT Chromaprint_FOUND)
  # Fail verbose, because with Qt Creator, the verbose error message for Qt is bypassed.
  fatal_error_missing_env()
endif()
target_link_libraries(mixxx-lib PRIVATE Chromaprint::Chromaprint)

# Locale Aware Compare for SQLite
find_package(SQLite3)
# For LOCALECOMPARE we call directly sqlite functions to the database opened by
# Qt. It only works without crashing when Mixxx links to the same sqlite
# library as Qt.
# This is difficult on macOS where system the SQLite can be installed and linked
# dynamically from various locations. There is no issue in case of static
# linking which would result in a duplicate symbol error.
if(NOT SQLite3_FOUND)
  set(LOCALECOMPARE_DEFAULT OFF)
else()
  is_static_library(SQLite3_IS_STATIC SQLite::SQLite3)
  if(SQLite3_IS_STATIC OR NOT APPLE)
    set(LOCALECOMPARE_DEFAULT ON)
  else()
    set(LOCALECOMPARE_DEFAULT OFF)
  endif()
endif()
cmake_dependent_option(
  LOCALECOMPARE
  "Locale Aware Compare support for SQLite"
  ON
  "LOCALECOMPARE_DEFAULT"
  OFF
)
if(LOCALECOMPARE)
  if(NOT SQLite3_FOUND)
    message(
      FATAL_ERROR
      "Locale Aware Compare for SQLite requires libsqlite and its development headers."
    )
  endif()
  target_compile_definitions(mixxx-lib PUBLIC __SQLITE3__)
  target_link_libraries(mixxx-lib PRIVATE SQLite::SQLite3)
elseif(SQLite3_IS_STATIC)
  # in the static case we need to link SQLite3 uncoditionally
  target_link_libraries(mixxx-lib PRIVATE SQLite::SQLite3)
endif()

# Denon Engine Prime library export support (using libdjinterop)
option(ENGINEPRIME "Support for library export to Denon Engine Prime" ON)
if(ENGINEPRIME)
  # libdjinterop does not currently have a stable ABI, so we fetch sources for a specific tag, build here, and link
  # statically.  This situation should be reviewed once libdjinterop hits version 1.x.
  set(LIBDJINTEROP_VERSION 0.26.1)
  # Look whether an existing installation of libdjinterop matches the required version.
  find_package(DjInterop ${LIBDJINTEROP_VERSION} EXACT CONFIG)
  if(NOT DjInterop_FOUND)
    find_package(DjInterop ${LIBDJINTEROP_VERSION} EXACT MODULE)
  endif()

  if(DjInterop_FOUND)
    # An existing installation of djinterop is available.
    message(STATUS "STATIC link existing system installation of libdjinterop")
    target_link_libraries(mixxx-lib PUBLIC DjInterop::DjInterop)
  else()
    # On MacOS, Mixxx does not use system SQLite, so we will use libdjinterop's
    # embedded SQLite in such a case.
    if(APPLE AND NOT SQLite3_IS_STATIC)
      message(
        STATUS
        "Building libdjinterop sources (with embedded SQLite) fetched from GitHub"
      )
      set(DJINTEROP_SYSTEM_SQLITE OFF)
    else()
      message(
        STATUS
        "Building libdjinterop sources (with system SQLite) fetched from GitHub"
      )
      set(DJINTEROP_SYSTEM_SQLITE ON)
    endif()

    set(
      DJINTEROP_INSTALL_DIR
      "${CMAKE_CURRENT_BINARY_DIR}/lib/libdjinterop-install"
    )
    set(
      DJINTEROP_LIBRARY
      "lib/${CMAKE_STATIC_LIBRARY_PREFIX}djinterop${CMAKE_STATIC_LIBRARY_SUFFIX}"
    )

    # CMake does not pass lists of paths properly to external projects.
    # This is worked around by changing the list separator.
    string(
      REPLACE
      ";"
      "|"
      PIPE_DELIMITED_CMAKE_PREFIX_PATH
      "${CMAKE_PREFIX_PATH}"
    )

    # For offline builds download the archive file from the URL and
    # copy it into DOWNLOAD_DIR under DOWNLOAD_NAME prior to starting
    # the configuration.
    #
    # If you want to test (locally) an experimental fork/branch of libdjinterop,
    # you can comment out URL and URL_HASH and use GIT_REPOSITORY instead:
    #
    # GIT_REPOSITORY "https://github.com/abcd/your-fork-of-libdjinterop"
    # GIT_TAG "origin/name-of-your-branch"
    #
    ExternalProject_Add(
      libdjinterop
      URL
        "https://github.com/xsco/libdjinterop/archive/refs/tags/${LIBDJINTEROP_VERSION}.tar.gz"
        "https://launchpad.net/~xsco/+archive/ubuntu/djinterop/+sourcefiles/libdjinterop/${LIBDJINTEROP_VERSION}-0ubuntu1/libdjinterop_${LIBDJINTEROP_VERSION}.orig.tar.gz"
      URL_HASH
        SHA256=f4fbe728783c14acdc999b74ce3f03d680f9187e1ff676d6bf1233fdb64ae7b2
      DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/downloads"
      DOWNLOAD_NAME "libdjinterop-${LIBDJINTEROP_VERSION}.tar.gz"
      PREFIX "libdjinterop-${LIBDJINTEROP_VERSION}"
      INSTALL_DIR ${DJINTEROP_INSTALL_DIR}
      LIST_SEPARATOR "|"
      CMAKE_ARGS
        -DBUILD_SHARED_LIBS=OFF -DCMAKE_SKIP_INSTALL_ALL_DEPENDENCY=ON
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
        -DCMAKE_PREFIX_PATH=${PIPE_DELIMITED_CMAKE_PREFIX_PATH}
        -DCMAKE_INSTALL_LIBDIR:PATH=lib
        -DCMAKE_MODULE_PATH:PATH=${CMAKE_MODULE_PATH}
        -$<IF:$<BOOL:${CMAKE_TOOLCHAIN_FILE}>,D,U>CMAKE_TOOLCHAIN_FILE:PATH=${CMAKE_TOOLCHAIN_FILE}
        -$<IF:$<BOOL:${CMAKE_OSX_DEPLOYMENT_TARGET}>,D,U>CMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
        -$<IF:$<BOOL:${CMAKE_OSX_ARCHITECTURES}>,D,U>CMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
        -DCMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR}
        -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}
        -DSYSTEM_SQLITE=${DJINTEROP_SYSTEM_SQLITE}
      BUILD_COMMAND ${CMAKE_COMMAND} --build . --target DjInterop
      BUILD_BYPRODUCTS <INSTALL_DIR>/${DJINTEROP_LIBRARY}
      EXCLUDE_FROM_ALL TRUE
    )

    # Since we have built libdjinterop from sources as a static library, its
    # transitive dependencies are not automatically recognised.  libdjinterop
    # depends on zlib and optionally sqlite3.  If libdjinterop was configured
    # to depend on system SQLite, Mixxx will already have the dependency.
    # But it does not have zlib, so we explicitly add that here.
    find_package(ZLIB 1.2.8 REQUIRED)
    # The include folder needs to already exist, otherwise INTERFACE_INCLUDE_DIRECTORIES will not be propagated
    file(MAKE_DIRECTORY "${DJINTEROP_INSTALL_DIR}/include")

    if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
      # Required for 'CMakeFiles/mixxx-lib_autogen_timestamp_deps'
      # This tells 'make" that the libdjinterop is required for the djinterop library
      # BUILD_BYPRODUCTS work for Ninja only, while this workaround does not work for Ninja on macOS
      add_custom_command(
        OUTPUT "${DJINTEROP_INSTALL_DIR}/${DJINTEROP_LIBRARY}"
        DEPENDS libdjinterop
        COMMAND echo libdjinterop installed
        COMMENT
          "Tell 'make' that libdjinterop is required for the djinterop target"
      )
    endif()

    # Assemble a library based on the external project.
    add_library(mixxx-libdjinterop STATIC IMPORTED)
    set_target_properties(
      mixxx-libdjinterop
      PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${DJINTEROP_INSTALL_DIR}/include"
        INTERFACE_LINK_LIBRARIES ZLIB::ZLIB
        IMPORTED_LOCATION "${DJINTEROP_INSTALL_DIR}/${DJINTEROP_LIBRARY}"
    )
    add_dependencies(mixxx-libdjinterop libdjinterop)
    # Add the local include directory explicitly before linking the library
    # to make sure not the system provided headers are used
    target_include_directories(
      mixxx-lib
      BEFORE
      PRIVATE "${DJINTEROP_INSTALL_DIR}/include"
    )
    target_link_libraries(mixxx-lib PRIVATE mixxx-libdjinterop)
  endif()

  # Include conditional sources only required with Engine Prime export support.
  target_sources(
    mixxx-lib
    PRIVATE
      src/library/export/dlglibraryexport.cpp
      src/library/export/engineprimeexportjob.cpp
      src/library/export/libraryexporter.cpp
  )
  target_compile_definitions(mixxx-lib PUBLIC __ENGINEPRIME__)
endif()

# Ebur128
find_package(Ebur128 REQUIRED)
target_link_libraries(mixxx-lib PRIVATE Ebur128::Ebur128)

# FidLib
add_library(fidlib STATIC EXCLUDE_FROM_ALL lib/fidlib/fidlib.c)
if(MSVC)
  target_compile_definitions(fidlib PRIVATE T_MSVC)
  target_compile_definitions(fidlib PRIVATE _USE_MATH_DEFINES)
  target_compile_options(fidlib PRIVATE /W3)
elseif(MINGW)
  target_compile_definitions(fidlib PRIVATE T_MINGW)
  target_compile_options(
    fidlib
    PRIVATE
      -fno-finite-math-only
      -Wall
      -Wextra
      -Wfloat-conversion
      -Werror=return-type
  )
else()
  target_compile_definitions(fidlib PRIVATE T_LINUX)
  target_compile_options(
    fidlib
    PRIVATE
      -fno-finite-math-only
      -Wall
      -Wextra
      -Wfloat-conversion
      -Werror=return-type
  )
endif()
target_include_directories(mixxx-lib SYSTEM PUBLIC lib/fidlib)
target_link_libraries(mixxx-lib PRIVATE fidlib)

# Create a list from CMAKE_PREFIX_PATH with an alternate separator
# This is required to forward CMAKE_PREFIX_PATH in ExternalProject_Add()
# using the LIST_SEPARATOR option
string(REPLACE ";" "|" CMAKE_PREFIX_PATH_ALT_SEP "${CMAKE_PREFIX_PATH}")

# KeyFinder
option(KEYFINDER "KeyFinder support" ON)
if(KEYFINDER)
  set(MIN_LIBKEYFINDER_VERSION 2.2.4)
  set(FETCH_LIBKEYFINDER_VERSION 2.2.8)
  find_package(KeyFinder ${MIN_LIBKEYFINDER_VERSION})
  if(KeyFinder_FOUND)
    target_link_libraries(mixxx-lib PRIVATE KeyFinder::KeyFinder)
  else()
    # If KeyFinder is built statically, we need FFTW
    find_package(FFTW3 REQUIRED)
    set(
      KeyFinder_INSTALL_DIR
      "${CMAKE_CURRENT_BINARY_DIR}/lib/keyfinder-install"
    )
    set(
      KeyFinder_LIBRARY
      "${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}keyfinder${CMAKE_STATIC_LIBRARY_SUFFIX}"
    )

    # CMake does not pass lists of paths properly to external projects.
    # This is worked around by changing the list separator.
    string(
      REPLACE
      ";"
      "|"
      PIPE_DELIMITED_CMAKE_PREFIX_PATH
      "${CMAKE_PREFIX_PATH}"
    )

    # For offline builds download the archive file from the URL and
    # copy it into DOWNLOAD_DIR under DOWNLOAD_NAME prior to starting
    # the configuration.
    ExternalProject_Add(
      libkeyfinder
      URL
        "https://github.com/mixxxdj/libkeyfinder/archive/refs/tags/${FETCH_LIBKEYFINDER_VERSION}.zip"
      URL_HASH
        SHA256=4f10e9e5673d948776e47e78273fa4d61408155cb0e210af1538c83222f285d4
      DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/downloads"
      DOWNLOAD_NAME "libkeyfinder-${FETCH_LIBKEYFINDER_VERSION}.zip"
      PREFIX "libdjinterop-${FETCH_LIBKEYFINDER_VERSION}"
      INSTALL_DIR "${KeyFinder_INSTALL_DIR}"
      LIST_SEPARATOR "|"
      CMAKE_ARGS
        -DBUILD_SHARED_LIBS=OFF -DCMAKE_SKIP_INSTALL_ALL_DEPENDENCY=ON
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
        -DCMAKE_PREFIX_PATH=${PIPE_DELIMITED_CMAKE_PREFIX_PATH}
        -DCMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_LIBDIR}
        -$<IF:$<BOOL:${CMAKE_TOOLCHAIN_FILE}>,D,U>CMAKE_TOOLCHAIN_FILE:PATH=${CMAKE_TOOLCHAIN_FILE}
        -$<IF:$<BOOL:${CMAKE_OSX_DEPLOYMENT_TARGET}>,D,U>CMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
        -$<IF:$<BOOL:${CMAKE_OSX_ARCHITECTURES}>,D,U>CMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
        -DCMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR}
        -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME} -DBUILD_TESTING=OFF
      BUILD_COMMAND ${CMAKE_COMMAND} --build .
      BUILD_BYPRODUCTS <INSTALL_DIR>/${KeyFinder_LIBRARY}
      EXCLUDE_FROM_ALL TRUE
    )

    # This is a bit of a hack to make sure that the include directory actually
    # exists when configuring the build.
    # ExternalProject_Add() will create it
    # at compile time, but CMake already
    # checks that all directories passed to
    # target_include_directories() exist
    # during configuration and will throw
    # an error if not.
    file(MAKE_DIRECTORY "${KeyFinder_INSTALL_DIR}/include")

    if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
      # Required for 'CMakeFiles/mixxx-lib_autogen_timestamp_deps'
      # This tells 'make" that the libkeyfinder is required for the mixxx-keyfinder library
      # BUILD_BYPRODUCTS work for Ninja only, while this workaround does not work for Ninja on macOS
      add_custom_command(
        OUTPUT "${KeyFinder_INSTALL_DIR}/${KeyFinder_LIBRARY}"
        DEPENDS libkeyfinder
        COMMAND echo libkeyfinder installed
        COMMENT
          "Tell 'make' that libkeyfinder is required for the mixxx-keyfinder target"
      )
    endif()

    add_library(mixxx-keyfinder STATIC IMPORTED)
    add_dependencies(mixxx-keyfinder libkeyfinder)
    set_target_properties(
      mixxx-keyfinder
      PROPERTIES
        IMPORTED_LOCATION "${KeyFinder_INSTALL_DIR}/${KeyFinder_LIBRARY}"
    )
    target_link_libraries(mixxx-keyfinder INTERFACE FFTW3::fftw3)
    target_include_directories(
      mixxx-keyfinder
      INTERFACE "${KeyFinder_INSTALL_DIR}/include"
    )
    target_link_libraries(mixxx-lib PRIVATE mixxx-keyfinder)
  endif()

  target_sources(mixxx-lib PRIVATE src/analyzer/plugins/analyzerkeyfinder.cpp)
  target_compile_definitions(mixxx-lib PUBLIC __KEYFINDER__)
endif()

# FLAC
find_package(FLAC REQUIRED)
target_link_libraries(mixxx-lib PRIVATE FLAC::FLAC)

# FpClassify This is a wrapper around the fpclassify function that prevents
# inlining It is compiled without optimization and allows to use these function
# from -ffast-math optimized objects. The MSVC option /fp:fast does not suffer this issue
add_library(FpClassify STATIC EXCLUDE_FROM_ALL src/util/fpclassify.cpp)
# With gcc >= 14 and -flto some code is inlined to fast-math code and then
# optimized away. Disable it explicit for all compiler just in case.
# Note: This does not set -fno-lto to override an external provided -flto=auto like on
# Launchpad. Setting it explicit below.
set_target_properties(FpClassify PROPERTIES INTERPROCEDURAL_OPTIMIZATION OFF)

if(
  CMAKE_CXX_COMPILER_ID MATCHES "Clang"
  AND CMAKE_CXX_SIMULATE_ID MATCHES "MSVC"
)
  target_compile_options(FpClassify PRIVATE /fp:precise)
elseif(GNU_GCC OR LLVM_CLANG)
  # The option `-ffp-contract=on` must precede `-fno-fast-math`
  # to silence a warning on Clang 14
  target_compile_options(
    FpClassify
    PRIVATE -ffp-contract=on -fno-fast-math -fno-lto
  )
endif()
target_link_libraries(mixxx-lib PRIVATE FpClassify)

# LAME
find_package(mp3lame REQUIRED)
target_link_libraries(mixxx-lib PRIVATE mp3lame::mp3lame)

add_library(
  rekordbox_metadata
  STATIC
  EXCLUDE_FROM_ALL
  lib/rekordbox-metadata/rekordbox_pdb.cpp
  lib/rekordbox-metadata/rekordbox_anlz.cpp
)
target_include_directories(
  rekordbox_metadata
  SYSTEM
  PUBLIC lib/rekordbox-metadata
)
target_link_libraries(mixxx-lib PRIVATE rekordbox_metadata)

#silence "enumeration values not handled in switch" in generated code
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(rekordbox_metadata PRIVATE -Wno-switch)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  target_compile_options(rekordbox_metadata PRIVATE /w44063)
endif()

#eve osc
# liblo

if(WIN32 OR APPLE)
  message(STATUS "[Liblo] Windows or macOS, using vcpkg")
  find_package(liblo REQUIRED)
elseif(UNIX)
  message(STATUS "[Liblo] Linux, using pkg-config")
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(LIBLO REQUIRED liblo)

  add_library(liblo UNKNOWN IMPORTED)
  set_target_properties(
    liblo
    PROPERTIES
      IMPORTED_LOCATION ${LIBLO_LINK_LIBRARIES}
      INTERFACE_INCLUDE_DIRECTORIES ${LIBLO_INCLUDE_DIRS}
  )

  add_library(liblo::liblo ALIAS liblo)
endif()

include_directories(${LIBLO_INCLUDE_DIRS})
target_link_libraries(mixxx-lib PRIVATE liblo::liblo)

# Liblo Test for CI
# to check if liblo is included >> send some messages to itself
# if received >> test passes
if(BUILD_TESTING)
  add_executable(
    osc_mixxx_integration_test
    src/test/osc_mixxx_integration_test.cpp
  )
  target_link_libraries(osc_mixxx_integration_test PRIVATE liblo::liblo)
  add_test(NAME osc_mixxx_integration_test COMMAND osc_mixxx_integration_test)
endif()

# Kaitai for reading Rekordbox libraries
add_library(Kaitai STATIC EXCLUDE_FROM_ALL lib/kaitai/kaitai/kaitaistream.cpp)
target_include_directories(Kaitai SYSTEM PUBLIC lib/kaitai)
target_compile_definitions(Kaitai PRIVATE KS_STR_ENCODING_NONE)
target_link_libraries(rekordbox_metadata PRIVATE Kaitai)
target_link_libraries(mixxx-lib PRIVATE Kaitai)

# For determining MP3 timing offset cases in Rekordbox library feature
add_library(
  MP3GuessEnc
  STATIC
  EXCLUDE_FROM_ALL
  lib/mp3guessenc-0.27.4/mp3guessenc.c
  lib/mp3guessenc-0.27.4/tags.c
  lib/mp3guessenc-0.27.4/decode.c
  lib/mp3guessenc-0.27.4/bit_utils.c
)
if(WIN32)
  target_compile_definitions(
    MP3GuessEnc
    PRIVATE __WINDOWS__ _CRT_SECURE_NO_WARNINGS
  )
endif()
target_include_directories(MP3GuessEnc SYSTEM PUBLIC lib/mp3guessenc-0.27.4)
target_link_libraries(mixxx-lib PRIVATE MP3GuessEnc)

# OpenGL
option(QGLES2 "Use GLES 2.0 (or higher) instead of full OpenGL for UNIX" OFF)
if(IOS)
  target_link_libraries(mixxx-lib PRIVATE "-framework OpenGLES")
  target_compile_definitions(mixxx-lib PUBLIC QT_OPENGL_ES_2)
else()
  if(APPLE)
    # Prefer the system-provided OpenGL framework on macOS to avoid accidentally
    # linking some other implementation, e.g. /usr/X11R6/lib/libGL.dylib if the
    # user has XQuartz installed, due to vcpkg setting CMAKE_FIND_FRAMEWORK to
    # LAST (see https://cmake.org/cmake/help/latest/variable/CMAKE_FIND_FRAMEWORK.html)
    set(_previous_find_framework ${CMAKE_FIND_FRAMEWORK})
    set(CMAKE_FIND_FRAMEWORK FIRST)
  endif()
  set(OpenGL_GL_PREFERENCE "GLVND")
  if(EMSCRIPTEN)
    find_package(OpenGL REQUIRED)
    # Emscripten's FindOpenGL.cmake does not create OpenGL::GL
    target_link_libraries(mixxx-lib PRIVATE ${OPENGL_gl_LIBRARY})
    target_compile_definitions(mixxx-lib PUBLIC QT_OPENGL_ES_2)
    # Require WebGL 2.0 (for a WebGL-friendly subset of OpenGL ES 3.0) and
    # enable full OpenGL ES 2.0 emulation as per
    # https://emscripten.org/docs/porting/multimedia_and_graphics/OpenGL-support.html
    target_link_options(
      mixxx-lib
      PUBLIC -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sFULL_ES2=1
    )
  else()
    find_package(WrapOpenGL REQUIRED)
    if(OPENGL_opengl_LIBRARY)
      target_link_libraries(mixxx-lib PRIVATE OpenGL::OpenGL)
    else()
      target_link_libraries(mixxx-lib PRIVATE OpenGL::GL)
    endif()
  endif()
  if(UNIX AND QGLES2)
    target_compile_definitions(mixxx-lib PUBLIC QT_OPENGL_ES_2)
  endif()
  if(APPLE)
    # Restore the previous CMAKE_FIND_FRAMEWORK value
    set(CMAKE_FIND_FRAMEWORK ${_previous_find_framework})
  endif()
endif()

# Ogg
find_package(Ogg REQUIRED)
target_link_libraries(mixxx-lib PRIVATE Ogg::ogg)

# Vorbis
find_package(Vorbis REQUIRED)
target_link_libraries(
  mixxx-lib
  PRIVATE Vorbis::vorbis Vorbis::vorbisenc Vorbis::vorbisfile
)

# PortAudio
find_package(PortAudio REQUIRED)
target_link_libraries(mixxx-lib PUBLIC PortAudio::PortAudio)

# PortAudio Ring Buffer
add_library(
  PortAudioRingBuffer
  STATIC
  EXCLUDE_FROM_ALL
  lib/portaudio/pa_ringbuffer.c
)
target_include_directories(mixxx-lib SYSTEM PUBLIC lib/portaudio)
target_link_libraries(mixxx-lib PRIVATE PortAudioRingBuffer)

# PortMidi
option(PORTMIDI "Enable the PortMidi backend for MIDI controllers" ON)
if(PORTMIDI)
  target_compile_definitions(mixxx-lib PUBLIC __PORTMIDI__)
  find_package(PortMidi REQUIRED)
  target_include_directories(mixxx-lib SYSTEM PUBLIC ${PortMidi_INCLUDE_DIRS})
  target_link_libraries(mixxx-lib PRIVATE ${PortMidi_LIBRARIES})
  target_sources(
    mixxx-lib
    PRIVATE
      src/controllers/midi/portmidicontroller.cpp
      src/controllers/midi/portmidienumerator.cpp
  )
endif()

# Protobuf
add_subdirectory(src/proto)
target_link_libraries(mixxx-lib PUBLIC mixxx-proto)

# Rigtorp SPSC Queue
# https://github.com/rigtorp/SPSCQueue
target_include_directories(
  mixxx-lib
  SYSTEM
  PUBLIC lib/rigtorp/SPSCQueue/include
)

# Qt
set(
  QT_COMPONENTS
  Concurrent
  Core
  Gui
  Network
  OpenGL
  PrintSupport
  Qml # for QJSEngine
  Sql
  Svg
  Test
  Widgets
  Xml
)
set(QT_EXTRA_COMPONENTS "")
if(QT6)
  find_package(QT 6.2 NAMES Qt6 COMPONENTS Core REQUIRED)
  list(APPEND QT_EXTRA_COMPONENTS "ShaderTools")
  list(APPEND QT_EXTRA_COMPONENTS "SvgWidgets")
  list(APPEND QT_EXTRA_COMPONENTS "Core5Compat")
else()
  find_package(QT 5.12 NAMES Qt5 COMPONENTS Core REQUIRED)
endif()
if(QML)
  list(APPEND QT_EXTRA_COMPONENTS "Quick")
  list(APPEND QT_EXTRA_COMPONENTS "LabsQmlModels")
  list(APPEND QT_EXTRA_COMPONENTS "QuickControls2")
  list(APPEND QT_EXTRA_COMPONENTS "QuickControls2Impl")
  list(APPEND QT_EXTRA_COMPONENTS "QuickLayouts")
  list(APPEND QT_EXTRA_COMPONENTS "QuickShapesPrivate")
  list(APPEND QT_EXTRA_COMPONENTS "QuickTemplates2")
  list(APPEND QT_EXTRA_COMPONENTS "QuickWidgets")
  list(APPEND QT_EXTRA_COMPONENTS "ShaderTools")
  if(QT_VERSION VERSION_GREATER_EQUAL 6.7)
    list(APPEND QT_EXTRA_COMPONENTS "QuickControls2Basic")
    list(APPEND QT_EXTRA_COMPONENTS "QuickControls2BasicStyleImpl")
    list(APPEND QT_EXTRA_COMPONENTS "QuickControls2Fusion")
    list(APPEND QT_EXTRA_COMPONENTS "QuickControls2FusionStyleImpl")
  endif()
  if(QT_VERSION VERSION_LESS 6.8)
    # From Qt 6.8 integrated in Qml via QmlMeta
    list(APPEND QT_EXTRA_COMPONENTS "QmlWorkerScript")
  endif()
endif()
find_package(
  Qt${QT_VERSION_MAJOR}
  COMPONENTS ${QT_COMPONENTS} ${QT_EXTRA_COMPONENTS}
  REQUIRED
)
# PUBLIC is required below to find included headers
foreach(component ${QT_COMPONENTS})
  target_link_libraries(mixxx-lib PUBLIC Qt${QT_VERSION_MAJOR}::${component})
endforeach()
if(QT_EXTRA_COMPONENTS)
  foreach(component ${QT_EXTRA_COMPONENTS})
    target_link_libraries(mixxx-lib PUBLIC Qt${QT_VERSION_MAJOR}::${component})
  endforeach()
endif()

if(QML)
  if(QT_KNOWN_POLICY_QTP0004)
    # See: https://doc.qt.io/qt-6/qt-cmake-policy-qtp0004.html
    # OLD (Qt < 6.8) requires to import qml modules with a folder e.g.:
    # fragmentShader: "qrc:/shaders/rgbsignal_qml.frag.qsb"
    # For using NEW, we need to back-port:
    # https://github.com/qt/qtdeclarative/commit/6314d305ee0d9064ca848980ef2dab1793c191b8
    # until Qt 6.8 lands in all supported distros
    qt6_policy(SET QTP0004 OLD)
  endif()

  add_subdirectory(res/shaders)

  set(QT_QML_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/qml)
  qt_add_library(mixxx-qml-lib STATIC)

  if(WIN32)
    target_compile_definitions(mixxx-qml-lib PUBLIC __WINDOWS__)
  endif()

  if(ENGINEPRIME)
    target_compile_definitions(mixxx-qml-lib PUBLIC __ENGINEPRIME__)
  endif()

  foreach(component ${QT_COMPONENTS})
    target_link_libraries(
      mixxx-qml-lib
      PUBLIC Qt${QT_VERSION_MAJOR}::${component}
    )
  endforeach()
  if(QT_EXTRA_COMPONENTS)
    foreach(component ${QT_EXTRA_COMPONENTS})
      target_link_libraries(
        mixxx-qml-lib
        PUBLIC Qt${QT_VERSION_MAJOR}::${component}
      )
    endforeach()
  endif()
  set_target_properties(mixxx-qml-lib PROPERTIES AUTOMOC ON)
  qt_add_qml_module(mixxx-qml-lib
    URI Mixxx
    VERSION 1.0
    RESOURCE_PREFIX /mixxx.org/imports
    IMPORTS QtQuick
    QML_FILES
      res/qml/Mixxx/MathUtils.mjs
      res/qml/Mixxx/PlayerDropArea.qml
  )
  target_link_libraries(mixxx-lib PRIVATE mixxx-qml-lib)

  # FIXME: Currently we need to add these include directories due to
  # QTBUG-87221. We should figure out a better way to fix this.
  # See: https://bugreports.qt.io/browse/QTBUG-87221
  target_include_directories(mixxx-qml-lib PRIVATE src/control src/qml)
  target_include_directories(mixxx-qml-lib PUBLIC src/ ${CMAKE_BINARY_DIR}/src)
  target_include_directories(
    mixxx-qml-lib
    SYSTEM
    PUBLIC lib/rigtorp/SPSCQueue/include lib/portaudio
  )

  target_link_libraries(mixxx-qml-lib PUBLIC mixxx-proto)
  target_link_libraries(mixxx-qml-libplugin PUBLIC mixxx-proto)

  target_precompile_headers(
    mixxx-qml-lib
    PUBLIC ${MIXXX_COMMON_PRECOMPILED_HEADER}
  )

  target_link_libraries(mixxx-qml-lib PRIVATE mixxx-qml-libplugin)

  qt_add_library(mixxx-qml-mixxxcontrols STATIC)
  set_target_properties(mixxx-qml-mixxxcontrols PROPERTIES AUTOMOC ON)
  qt_add_qml_module(mixxx-qml-mixxxcontrols
    URI Mixxx.Controls
    VERSION 1.0
    RESOURCE_PREFIX /mixxx.org/imports
    OPTIONAL_IMPORTS Mixxx
    QML_FILES
      res/qml/Mixxx/Controls/Knob.qml
      res/qml/Mixxx/Controls/Slider.qml
      res/qml/Mixxx/Controls/Spinny.qml
      res/qml/Mixxx/Controls/WaveformOverviewHotcueMarker.qml
      res/qml/Mixxx/Controls/WaveformOverviewMarker.qml
      res/qml/Mixxx/Controls/WaveformOverview.qml
      res/qml/Mixxx/Controls/WaveformDisplay.qml
  )
  target_link_libraries(mixxx-qml-lib PRIVATE mixxx-qml-mixxxcontrolsplugin)

  target_sources(
    mixxx-qml-lib
    PRIVATE
      src/qml/asyncimageprovider.cpp
      src/qml/qmlapplication.cpp
      src/qml/qmlautoreload.cpp
      src/qml/qmlbeatsmodel.cpp
      src/qml/qmlchainpresetmodel.cpp
      src/qml/qmlconfigproxy.cpp
      src/qml/qmlcontrolproxy.cpp
      src/qml/qmlcuesmodel.cpp
      src/qml/qmldlgpreferencesproxy.cpp
      src/qml/qmleffectmanifestparametersmodel.cpp
      src/qml/qmleffectslotproxy.cpp
      src/qml/qmleffectsmanagerproxy.cpp
      src/qml/qmllibraryproxy.cpp
      src/qml/qmllibrarysource.cpp
      src/qml/qmllibrarysourcetree.cpp
      src/qml/qmllibrarytracklistmodel.cpp
      src/qml/qmlmixxxcontrollerscreen.cpp
      src/qml/qmlplayermanagerproxy.cpp
      src/qml/qmlplayerproxy.cpp
      src/qml/qmlsidebarmodelproxy.cpp
      src/qml/qmllibrarytracklistcolumn.cpp
      src/qml/qmltrackproxy.cpp
      src/qml/qmlvisibleeffectsmodel.cpp
      src/qml/qmlwaveformdisplay.cpp
      src/qml/qmlwaveformoverview.cpp
      src/qml/qmlwaveformrenderer.cpp
      src/qml/qmlsettingparameter.cpp
      src/qml/qmltrackproxy.cpp
      src/qml/qmlsoundmanagerproxy.cpp
      src/waveform/renderers/allshader/digitsrenderer.cpp
      src/waveform/renderers/allshader/waveformrenderbeat.cpp
      src/waveform/renderers/allshader/waveformrenderer.cpp
      src/waveform/renderers/allshader/waveformrendererendoftrack.cpp
      src/waveform/renderers/allshader/waveformrendererpreroll.cpp
      src/waveform/renderers/allshader/waveformrendererrgb.cpp
      src/waveform/renderers/allshader/waveformrenderersignalbase.cpp
      src/waveform/renderers/allshader/waveformrendermark.cpp
      src/waveform/renderers/allshader/waveformrendermarkrange.cpp
      src/waveform/renderers/allshader/waveformrendererslipmode.cpp
      src/waveform/renderers/allshader/waveformrendererfiltered.cpp
      src/waveform/renderers/allshader/waveformrendererhsv.cpp
      src/waveform/renderers/allshader/waveformrenderersimple.cpp
      # The following sources need to be in this target to get QML_ELEMENT properly interpreted
      src/control/controlmodel.cpp
      src/control/controlsortfiltermodel.cpp
      # needed for qml/qmlautoreload.cpp
      src/util/autofilereloader.cpp
  )

  if(STEM)
    target_compile_definitions(mixxx-qml-lib PUBLIC __STEM__)
    target_sources(
      mixxx-qml-lib
      PRIVATE
        src/waveform/renderers/allshader/waveformrendererstem.cpp
        src/qml/qmlstemsmodel.cpp
    )
  endif()

  # qt_finalize_target takes care that the resources :/mixxx.org/imports/Mixxx/
  # and :/mixxx.org/imports/Mixxx/Controls are placed into beginning of the binary
  qt_finalize_target(mixxx)

  install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/res/qml"
    DESTINATION "${MIXXX_INSTALL_DATADIR}"
  )
endif()

option(DEBUG_ASSERTIONS_FATAL "Fail if debug become true assertions" OFF)
if(DEBUG_ASSERTIONS_FATAL)
  target_compile_definitions(
    mixxx-lib
    PUBLIC MIXXX_DEBUG_ASSERTIONS_FATAL MIXXX_DEBUG_ASSERTIONS_ENABLED
  )
  if(QML)
    target_compile_definitions(
      mixxx-qml-lib
      PUBLIC MIXXX_DEBUG_ASSERTIONS_FATAL MIXXX_DEBUG_ASSERTIONS_ENABLED
    )
  endif()
  if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(
      STATUS
      "DEBUG_ASSERT statements have been enabled because DEBUG_ASSERTIONS_FATAL is ON."
    )
  endif()
endif()

if(EMSCRIPTEN)
  option(
    WASM_ASSERTIONS
    "Enable additional checks when targeting Emscripten/WebAssembly"
    OFF
  )
  if(WASM_ASSERTIONS)
    target_link_options(mixxx-lib PUBLIC -sASSERTIONS)
  endif()
endif()

target_compile_definitions(
  mixxx-lib
  PUBLIC QT_TABLET_SUPPORT QT_USE_QSTRINGBUILDER
)
is_static_library(Qt_IS_STATIC Qt${QT_VERSION_MAJOR}::Core)
if(Qt_IS_STATIC)
  # NOTE(rryan): If you are adding a plugin here, you must also
  # update src/mixxxapplication.cpp to define a Q_IMPORT_PLUGIN
  # for it. Not all imageformats plugins are built as .libs when
  # building Qt statically on Windows. Check the build environment
  # to see exactly what's available as a standalone .lib vs linked
  # into Qt .libs by default.

  target_link_libraries(
    mixxx-lib
    PRIVATE
      # imageformats plugins
      Qt${QT_VERSION_MAJOR}::QGifPlugin
      Qt${QT_VERSION_MAJOR}::QICOPlugin
      Qt${QT_VERSION_MAJOR}::QJpegPlugin
      Qt${QT_VERSION_MAJOR}::QSvgPlugin
      # sqldrivers
      Qt${QT_VERSION_MAJOR}::QSQLiteDriverPlugin
  )

  if(NOT IOS)
    target_link_libraries(
      mixxx-lib
      PRIVATE
        # network plugins
        Qt${QT_VERSION_MAJOR}::QTlsBackendOpenSSLPlugin
    )
  endif()

  if(EMSCRIPTEN)
    target_link_libraries(
      mixxx-lib
      PRIVATE Qt${QT_VERSION_MAJOR}::QWasmIntegrationPlugin
    )
  else()
    target_link_libraries(
      mixxx-lib
      PRIVATE
        # platform plugins
        Qt${QT_VERSION_MAJOR}::QOffscreenIntegrationPlugin
        Qt${QT_VERSION_MAJOR}::QMinimalIntegrationPlugin
    )
  endif()

  if(WIN32)
    target_link_libraries(
      mixxx-lib
      PRIVATE Qt${QT_VERSION_MAJOR}::QWindowsIntegrationPlugin
    )
    if(QT_VERSION VERSION_LESS 6.7)
      target_link_libraries(
        mixxx-lib
        PRIVATE Qt${QT_VERSION_MAJOR}::QWindowsVistaStylePlugin
      )
    else()
      target_link_libraries(
        mixxx-lib
        PRIVATE Qt${QT_VERSION_MAJOR}::QModernWindowsStylePlugin
      )
    endif()
  endif()

  if(APPLE)
    if(IOS)
      target_link_libraries(
        mixxx-lib
        PRIVATE Qt${QT_VERSION_MAJOR}::QIOSIntegrationPlugin
      )
    else()
      target_link_libraries(
        mixxx-lib
        PRIVATE
          Qt${QT_VERSION_MAJOR}::QCocoaIntegrationPlugin
          Qt${QT_VERSION_MAJOR}::QMacStylePlugin
      )
    endif()
  endif()
else()
  if(QT6 AND (WIN32 OR APPLE))
    # Qt6 does not automatically install plugins like in Qt 5

    find_package(libjpeg-turbo CONFIG REQUIRED)
    install(
      IMPORTED_RUNTIME_ARTIFACTS
        # QJpegPlugin dependency
        libjpeg-turbo::jpeg
        DESTINATION
        "${MIXXX_INSTALL_DATADIR}"
        COMPONENT
        applocal
    )

    install(
      IMPORTED_RUNTIME_ARTIFACTS
        # platform plugins
        Qt${QT_VERSION_MAJOR}::QOffscreenIntegrationPlugin
        Qt${QT_VERSION_MAJOR}::QMinimalIntegrationPlugin
        DESTINATION
        "${MIXXX_INSTALL_DATADIR}/platforms"
        COMPONENT
        applocal
    )

    install(
      IMPORTED_RUNTIME_ARTIFACTS
        Qt${QT_VERSION_MAJOR}::QGifPlugin
        Qt${QT_VERSION_MAJOR}::QICOPlugin
        Qt${QT_VERSION_MAJOR}::QJpegPlugin
        Qt${QT_VERSION_MAJOR}::QSvgPlugin
        DESTINATION
        "${MIXXX_INSTALL_DATADIR}/imageformats"
        COMPONENT
        applocal
    )

    install(
      IMPORTED_RUNTIME_ARTIFACTS
        Qt${QT_VERSION_MAJOR}::QSQLiteDriverPlugin
        DESTINATION
        "${MIXXX_INSTALL_DATADIR}/sqldrivers"
        COMPONENT
        applocal
    )

    if(NOT IOS)
      install(
        IMPORTED_RUNTIME_ARTIFACTS
          Qt${QT_VERSION_MAJOR}::QTlsBackendOpenSSLPlugin
          DESTINATION
          "${MIXXX_INSTALL_DATADIR}/tls"
          COMPONENT
          applocal
      )
    endif()

    if(QML)
      install(
        IMPORTED_RUNTIME_ARTIFACTS
          Qt${QT_VERSION_MAJOR}::LabsQmlModels
          DESTINATION
          "${MIXXX_INSTALL_DATADIR}"
          COMPONENT
          applocal
      )

      install(
        IMPORTED_RUNTIME_ARTIFACTS
          Qt${QT_VERSION_MAJOR}::QuickControls2Impl
          DESTINATION
          "${MIXXX_INSTALL_DATADIR}"
          COMPONENT
          applocal
      )

      if(QT_VERSION VERSION_GREATER_EQUAL 6.7)
        install(
          IMPORTED_RUNTIME_ARTIFACTS
            Qt${QT_VERSION_MAJOR}::QuickControls2Basic
            Qt${QT_VERSION_MAJOR}::QuickControls2BasicStyleImpl
            Qt${QT_VERSION_MAJOR}::QuickControls2Fusion
            Qt${QT_VERSION_MAJOR}::QuickControls2FusionStyleImpl
            DESTINATION
            "${MIXXX_INSTALL_DATADIR}"
            COMPONENT
            applocal
        )
      endif()

      if(QT_VERSION VERSION_LESS 6.8)
        # From Qt 6.8 integrated in Qml via QmlMeta
        install(
          IMPORTED_RUNTIME_ARTIFACTS
            Qt${QT_VERSION_MAJOR}::QmlWorkerScript
            DESTINATION
            "${MIXXX_INSTALL_DATADIR}"
            COMPONENT
            applocal
        )
      endif()

      install(
        IMPORTED_RUNTIME_ARTIFACTS
          Qt${QT_VERSION_MAJOR}::QuickLayouts
          DESTINATION
          "${MIXXX_INSTALL_DATADIR}"
          COMPONENT
          applocal
      )

      install(
        IMPORTED_RUNTIME_ARTIFACTS
          Qt${QT_VERSION_MAJOR}::QuickShapesPrivate
          DESTINATION
          "${MIXXX_INSTALL_DATADIR}"
          COMPONENT
          applocal
      )

      if(QT_VERSION VERSION_LESS 6.4)
        # From Qt 6.4 integrated in QuickControls2
        install(
          IMPORTED_RUNTIME_ARTIFACTS
            Qt${QT_VERSION_MAJOR}::QuickTemplates2
            DESTINATION
            "${MIXXX_INSTALL_DATADIR}"
            COMPONENT
            applocal
        )
      endif()

      install(
        IMPORTED_RUNTIME_ARTIFACTS
          Qt${QT_VERSION_MAJOR}::ShaderTools
          DESTINATION
          "${MIXXX_INSTALL_DATADIR}"
          COMPONENT
          applocal
      )

      #install qml6-module-qt5compat-graphicaleffects
      install(
        DIRECTORY
          "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}$<$<CONFIG:Debug>:/debug>/Qt6/qml/Qt5Compat/GraphicalEffects"
        DESTINATION "${MIXXX_INSTALL_DATADIR}/Qt6/qml/Qt5Compat"
        COMPONENT applocal
      )

      # install qml6-module-qtqml-workerscript
      install(
        DIRECTORY
          "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}$<$<CONFIG:Debug>:/debug>/Qt6/qml/QtQml/WorkerScript"
        DESTINATION "${MIXXX_INSTALL_DATADIR}/Qt6/qml/QtQml"
        COMPONENT applocal
      )

      # install qml6-module-qtquick-controls
      install(
        DIRECTORY
          "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}$<$<CONFIG:Debug>:/debug>/Qt6/qml/QtQuick/Controls"
        DESTINATION "${MIXXX_INSTALL_DATADIR}/Qt6/qml/QtQuick"
        COMPONENT applocal
      )

      # install qml6-module-qtquick-layouts
      install(
        DIRECTORY
          "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}$<$<CONFIG:Debug>:/debug>/Qt6/qml/QtQuick/Layouts"
        DESTINATION "${MIXXX_INSTALL_DATADIR}/Qt6/qml/QtQuick"
        COMPONENT applocal
      )

      # install qml6-module-qtquick-nativestyle
      install(
        DIRECTORY
          "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}$<$<CONFIG:Debug>:/debug>/Qt6/qml/QtQuick/NativeStyle"
        DESTINATION "${MIXXX_INSTALL_DATADIR}/Qt6/qml/QtQuick"
        COMPONENT applocal
      )

      # install qml6-module-qtquick-shapes
      install(
        DIRECTORY
          "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}$<$<CONFIG:Debug>:/debug>/Qt6/qml/QtQuick/Shapes"
        DESTINATION "${MIXXX_INSTALL_DATADIR}/Qt6/qml/QtQuick"
        COMPONENT applocal
      )

      # install qml6-module-qtquick-templates
      install(
        DIRECTORY
          "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}$<$<CONFIG:Debug>:/debug>/Qt6/qml/QtQuick/Templates"
        DESTINATION "${MIXXX_INSTALL_DATADIR}/Qt6/qml/QtQuick"
        COMPONENT applocal
      )

      # qml6-module-qtquick-window
      install(
        DIRECTORY
          "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}$<$<CONFIG:Debug>:/debug>/Qt6/qml/QtQuick/Window"
        DESTINATION "${MIXXX_INSTALL_DATADIR}/Qt6/qml/QtQuick"
        COMPONENT applocal
      )

      # install qml6-module-qt-labs-qmlmodels
      install(
        DIRECTORY
          "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}$<$<CONFIG:Debug>:/debug>/Qt6/qml/Qt/labs/qmlmodels"
        DESTINATION "${MIXXX_INSTALL_DATADIR}/Qt6/qml/Qt/labs"
        COMPONENT applocal
      )
    endif()

    if(WIN32)
      install(
        IMPORTED_RUNTIME_ARTIFACTS
          Qt${QT_VERSION_MAJOR}::QWindowsIntegrationPlugin
          DESTINATION
          "${MIXXX_INSTALL_DATADIR}/platforms"
          COMPONENT
          applocal
      )
      if(QT_VERSION VERSION_LESS 6.7)
        install(
          IMPORTED_RUNTIME_ARTIFACTS
            Qt${QT_VERSION_MAJOR}::QWindowsVistaStylePlugin
            DESTINATION
            "${MIXXX_INSTALL_DATADIR}/styles"
            COMPONENT
            applocal
        )
      else()
        install(
          IMPORTED_RUNTIME_ARTIFACTS
            Qt${QT_VERSION_MAJOR}::QModernWindowsStylePlugin
            DESTINATION
            "${MIXXX_INSTALL_DATADIR}/styles"
            COMPONENT
            applocal
        )
      endif()
    endif()
    if(APPLE)
      install(
        IMPORTED_RUNTIME_ARTIFACTS
          Qt${QT_VERSION_MAJOR}::QCocoaIntegrationPlugin
          DESTINATION
          "${MIXXX_INSTALL_DATADIR}/platforms"
          COMPONENT
          applocal
      )
      install(
        IMPORTED_RUNTIME_ARTIFACTS
          Qt${QT_VERSION_MAJOR}::QMacStylePlugin
          DESTINATION
          "${MIXXX_INSTALL_DATADIR}/styles"
          COMPONENT
          applocal
      )
    endif()

    set(APPLOCAL_COMPONENT_DEFINED true)
  endif()
endif()

if(APPLE)
  if(Qt_IS_STATIC OR QT6)
    target_link_libraries(
      mixxx-lib
      PRIVATE
        "-weak_framework Accelerate"
        "-weak_framework AudioToolbox"
        "-weak_framework AVFoundation"
        "-weak_framework CoreAudio"
        "-weak_framework CoreFoundation"
        "-weak_framework CoreImage"
        "-weak_framework CoreMedia"
        "-weak_framework CoreMidi"
        "-weak_framework CoreServices"
        "-weak_framework CoreVideo"
        "-weak_framework IOSurface"
        "-weak_framework VideoToolbox"
    )
    if(IOS)
      target_link_libraries(mixxx-lib PRIVATE "-weak_framework UIKit")
    elseif()
      target_link_libraries(
        mixxx-lib
        PRIVATE "-weak_framework AppKit" "-weak_framework AudioUnit"
      )
    endif()
  else()
    # Used for battery measurements and controlling the screensaver on macOS.
    target_link_libraries(mixxx-lib PRIVATE "-weak_framework IOKit")
  endif()
elseif(UNIX AND NOT APPLE AND NOT EMSCRIPTEN)
  if(QT6)
    find_package(X11)
  else()
    find_package(X11 REQUIRED)
    find_package(Qt5 COMPONENTS X11Extras REQUIRED)
    target_link_libraries(mixxx-lib PUBLIC Qt5::X11Extras)
  endif()
  if(${X11_FOUND})
    target_include_directories(mixxx-lib SYSTEM PUBLIC "${X11_INCLUDE_DIR}")
    target_link_libraries(mixxx-lib PRIVATE "${X11_LIBRARIES}")
    target_compile_definitions(mixxx-lib PUBLIC __X11__)
  endif()
  find_package(Qt${QT_VERSION_MAJOR} COMPONENTS DBus REQUIRED)
  target_link_libraries(mixxx-lib PUBLIC Qt${QT_VERSION_MAJOR}::DBus)
elseif(WIN32)
  if(Qt_IS_STATIC)
    target_link_libraries(
      mixxx-lib
      PRIVATE
        # Pulled from qt-4.8.2-source\mkspecs\win32-msvc2010\qmake.conf
        # QtCore
        kernel32
        user32 # QtGui, QtOpenGL, libHSS1394
        shell32
        uuid
        ole32 # QtGui,
        advapi32 # QtGui, portaudio, portmidi
        ws2_32 # QtGui, QtNetwork, libshout
        # QtGui
        gdi32 # QtOpenGL, libshout
        comdlg32
        oleaut32
        imm32
        winmm
        winspool
        # QtOpenGL
        glu32
        opengl32
        # QtNetwork openssl-linked
        crypt32
        dwmapi # qtwindows
        iphlpapi # qt5network
        mpr # qt5core
        netapi32 # qt5core
        userenv # qt5core
        uxtheme # ?
        version # ?
        wtsapi32 # ?
    )

    find_library(
      QTFONTDATABASESUPPORT_LIBRARY
      Qt${QT_VERSION_MAJOR}FontDatabaseSupport
    )
    target_link_libraries(mixxx-lib PRIVATE "${QTFONTDATABASESUPPORT_LIBRARY}")
    find_library(
      QTWINDOWSUIAUTOMATIONSUPPORT_LIBRARY
      Qt${QT_VERSION_MAJOR}WindowsUIAutomationSupport
    )
    target_link_libraries(
      mixxx-lib
      PRIVATE "${QTWINDOWSUIAUTOMATIONSUPPORT_LIBRARY}"
    )
    find_library(
      QTEVENTDISPATCHERSUPPORT_LIBRARY
      Qt${QT_VERSION_MAJOR}EventDispatcherSupport
    )
    target_link_libraries(
      mixxx-lib
      PRIVATE "${QTEVENTDISPATCHERSUPPORT_LIBRARY}"
    )
    find_library(QTTHEMESUPPORT_LIBRARY Qt${QT_VERSION_MAJOR}ThemeSupport)
    target_link_libraries(mixxx-lib PRIVATE "${QTTHEMESUPPORT_LIBRARY}")

    find_library(QTFREETYPE_LIBRARY qtfreetype)
    target_link_libraries(mixxx-lib PRIVATE "${QTFREETYPE_LIBRARY}")
    find_library(QTHARFBUZZ_LIBRARY qtharfbuzz)
    target_link_libraries(mixxx-lib PRIVATE "${QTHARFBUZZ_LIBRARY}")
    find_library(QTLIBPNG_LIBRARY qtlibpng)
    target_link_libraries(mixxx-lib PRIVATE "${QTLIBPNG_LIBRARY}")
    find_library(QTPCRE2_LIBRARY qtpcre2)
    target_link_libraries(mixxx-lib PRIVATE "${QTPCRE2_LIBRARY}")
  else()
    #libshout is always built statically
    target_link_libraries(
      mixxx-lib
      PRIVATE
        ws2_32 # libshout
        gdi32 # libshout
    )
  endif()
endif()

if(APPLE OR WIN32)
  # qt_de.qm is just one arbitrary file in the directory that needs to be located;
  # there is no particular reason to look for this file versus any other one in the directory.
  if(QT6)
    find_file(
      QT_TRANSLATION_FILE
      qt_de.qm
      PATHS "${Qt6_DIR}/../../translations/Qt6"
      REQUIRED
      NO_DEFAULT_PATH
    )
  else()
    find_file(
      QT_TRANSLATION_FILE
      qt_de.qm
      PATHS
        "${Qt5_DIR}/../../../translations"
        "${Qt5_DIR}/../../qt5/translations"
      REQUIRED
      NO_DEFAULT_PATH
    )
  endif()
  get_filename_component(QT_TRANSLATIONS ${QT_TRANSLATION_FILE} DIRECTORY)
  install(
    DIRECTORY "${QT_TRANSLATIONS}/"
    DESTINATION "${MIXXX_INSTALL_DATADIR}/translations"
    # QT 5 translations have been separated into several files, and most of the qt_xx.qm files
    # contain just shortcuts to load the qtbase, qtmultimedia etc files.
    FILES_MATCHING
    REGEX
    "qt_.+\.qm|qtbase_.*\.qm|qtmultimedia_.*\.qm|qtscript_.*\.qm|qtxmlpatterns_.*\.qm"
  )
endif()

add_library(
  mixxx-gitinfostore
  STATIC
  EXCLUDE_FROM_ALL
  src/util/gitinfostore.cpp
)
# QtCore for QString
target_link_libraries(mixxx-gitinfostore PUBLIC Qt${QT_VERSION_MAJOR}::Core)
target_include_directories(
  mixxx-gitinfostore
  PUBLIC src ${CMAKE_BINARY_DIR}/src
)
add_dependencies(mixxx-gitinfostore mixxx-gitinfo)

# Queen Mary DSP
add_library(
  QueenMaryDsp
  STATIC
  EXCLUDE_FROM_ALL
  # lib/qm-dsp/base/KaiserWindow.cpp
  lib/qm-dsp/base/Pitch.cpp
  # lib/qm-dsp/base/SincWindow.cpp
  lib/qm-dsp/dsp/chromagram/Chromagram.cpp
  lib/qm-dsp/dsp/chromagram/ConstantQ.cpp
  lib/qm-dsp/dsp/keydetection/GetKeyMode.cpp
  # lib/qm-dsp/dsp/mfcc/MFCC.cpp
  lib/qm-dsp/dsp/onsets/DetectionFunction.cpp
  lib/qm-dsp/dsp/onsets/PeakPicking.cpp
  lib/qm-dsp/dsp/phasevocoder/PhaseVocoder.cpp
  lib/qm-dsp/dsp/rateconversion/Decimator.cpp
  # lib/qm-dsp/dsp/rateconversion/DecimatorB.cpp
  # lib/qm-dsp/dsp/rateconversion/Resampler.cpp
  # lib/qm-dsp/dsp/rhythm/BeatSpectrum.cpp
  # lib/qm-dsp/dsp/segmentation/ClusterMeltSegmenter.cpp
  # lib/qm-dsp/dsp/segmentation/Segmenter.cpp
  # lib/qm-dsp/dsp/segmentation/cluster_melt.c
  # lib/qm-dsp/dsp/segmentation/cluster_segmenter.c
  lib/qm-dsp/dsp/signalconditioning/DFProcess.cpp
  lib/qm-dsp/dsp/signalconditioning/FiltFilt.cpp
  lib/qm-dsp/dsp/signalconditioning/Filter.cpp
  lib/qm-dsp/dsp/signalconditioning/Framer.cpp
  lib/qm-dsp/dsp/tempotracking/DownBeat.cpp
  lib/qm-dsp/dsp/tempotracking/TempoTrack.cpp
  lib/qm-dsp/dsp/tempotracking/TempoTrackV2.cpp
  lib/qm-dsp/dsp/tonal/ChangeDetectionFunction.cpp
  lib/qm-dsp/dsp/tonal/TCSgram.cpp
  lib/qm-dsp/dsp/tonal/TonalEstimator.cpp
  lib/qm-dsp/dsp/transforms/FFT.cpp
  # lib/qm-dsp/dsp/wavelet/Wavelet.cpp
  lib/qm-dsp/ext/kissfft/kiss_fft.c
  lib/qm-dsp/ext/kissfft/tools/kiss_fftr.c
  # lib/qm-dsp/hmm/hmm.c
  lib/qm-dsp/maths/Correlation.cpp
  # lib/qm-dsp/maths/CosineDistance.cpp
  lib/qm-dsp/maths/KLDivergence.cpp
  lib/qm-dsp/maths/MathUtilities.cpp
  # lib/qm-dsp/maths/pca/pca.c
  # lib/qm-dsp/thread/Thread.cpp
)

target_compile_definitions(QueenMaryDsp PRIVATE kiss_fft_scalar=double)
if(UNIX)
  target_compile_definitions(QueenMaryDsp PRIVATE USE_PTHREADS)
elseif(MSVC)
  # Causes the cmath headers to declare M_PI and friends.
  # http://msdn.microsoft.com/en-us/library/4hwaceh6.aspx We could define this
  # in our headers but then include order matters since headers we don't control
  # may include cmath first.
  target_compile_definitions(QueenMaryDsp PRIVATE _USE_MATH_DEFINES)
endif()
target_include_directories(
  QueenMaryDsp
  SYSTEM
  PUBLIC lib/qm-dsp lib/qm-dsp/include
)
target_link_libraries(mixxx-lib PRIVATE QueenMaryDsp)

# ReplayGain
add_library(ReplayGain STATIC EXCLUDE_FROM_ALL lib/replaygain/replaygain.cpp)
target_include_directories(mixxx-lib SYSTEM PRIVATE lib/replaygain)
target_link_libraries(mixxx-lib PRIVATE ReplayGain)

# Reverb
add_library(Reverb STATIC EXCLUDE_FROM_ALL lib/reverb/Reverb.cc)
if(MSVC)
  target_compile_definitions(Reverb PRIVATE _USE_MATH_DEFINES)
endif()
target_include_directories(Reverb PRIVATE src)
target_link_libraries(Reverb PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_include_directories(mixxx-lib SYSTEM PRIVATE lib/reverb)
target_link_libraries(mixxx-lib PRIVATE Reverb)

# Rubberband
option(RUBBERBAND "Enable the rubberband engine for pitch-bending" ON)
if(RUBBERBAND)
  find_package(rubberband REQUIRED)
  target_link_libraries(mixxx-lib PRIVATE rubberband::rubberband)
  target_compile_definitions(mixxx-lib PUBLIC __RUBBERBAND__)
  if(QML)
    target_compile_definitions(mixxx-qml-lib PUBLIC __RUBBERBAND__)
  endif()
  target_sources(
    mixxx-lib
    PRIVATE
      src/effects/backends/builtin/pitchshifteffect.cpp
      src/engine/bufferscalers/enginebufferscalerubberband.cpp
      src/engine/bufferscalers/rubberbandwrapper.cpp
      src/engine/bufferscalers/rubberbandtask.cpp
      src/engine/bufferscalers/rubberbandworkerpool.cpp
  )
endif()

# SndFile
find_package(SndFile REQUIRED)
target_link_libraries(mixxx-lib PRIVATE SndFile::sndfile)
target_compile_definitions(mixxx-lib PUBLIC __SNDFILE__)
if(SndFile_SUPPORTS_SET_COMPRESSION_LEVEL)
  target_compile_definitions(
    mixxx-lib
    PUBLIC SFC_SUPPORTS_SET_COMPRESSION_LEVEL
  )
endif()

# SoundTouch
find_package(SoundTouch 2.1.2 REQUIRED)
target_link_libraries(mixxx-lib PRIVATE SoundTouch::SoundTouch)

# TagLib
find_package(TagLib 1.11 REQUIRED)
target_link_libraries(mixxx-lib PUBLIC TagLib::TagLib)
if(QML)
  target_link_libraries(mixxx-qml-lib PUBLIC TagLib::TagLib)
endif()

# Threads
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(mixxx-lib PRIVATE Threads::Threads)

#
# Features
#

# Battery meter
#
# The battery meter is only available on Linux, macOS and Windows, therefore
# this option is forcibly set to OFF on all other platforms.
cmake_dependent_option(
  BATTERY
  "Battery meter support"
  ON
  "WIN32 OR UNIX"
  OFF
)
if(BATTERY)
  if(WIN32)
    target_sources(mixxx-lib PRIVATE src/util/battery/batterywindows.cpp)
  elseif(APPLE)
    if(IOS)
      message(FATAL_ERROR "Battery support is not implemented for iOS")
    else()
      target_sources(mixxx-lib PRIVATE src/util/battery/batterymac.cpp)
    endif()
  elseif(UNIX)
    if(EMSCRIPTEN)
      message(
        FATAL_ERROR
        "Battery support is not implemented for Emscripten (WebAssembly)"
      )
    endif()
    find_package(Upower REQUIRED)
    find_package(GLIB COMPONENTS gobject REQUIRED)
    target_include_directories(mixxx-lib SYSTEM PUBLIC ${GLIB_INCLUDE_DIRS})
    target_link_libraries(
      mixxx-lib
      PRIVATE Upower::Upower ${GLIB_LIBRARIES} ${GLIB_GOBJECT_LIBRARIES}
    )
    target_sources(mixxx-lib PRIVATE src/util/battery/batterylinux.cpp)
  else()
    message(
      FATAL_ERROR
      "Battery support is not implemented for the target platform."
    )
  endif()
  target_compile_definitions(mixxx-lib PUBLIC __BATTERY__)
endif()

# Build Time
option(BUILDTIME "Use __DATE__ and __TIME__" ON)
if(NOT BUILDTIME)
  # Distributions like openSUSE use tools (e. g. build-compare) to detect
  # whether a built binary differs from a former build to avoid unneeded
  # publishing of packages.
  # If __DATE__ and __TIME__ are used the built binary differs always but
  # the tools cannot detect the root and publish a new package although
  # the only change is caused by __DATE__ and __TIME__.
  target_compile_definitions(mixxx-lib PUBLIC DISABLE_BUILDTIME)
endif()

# Clang Color Diagnostics
option(CLANG_COLORDIAG "Clang color diagnostics" OFF)
if(CLANG_COLORDIAG)
  if(NOT LLVM_CLANG)
    message(
      FATAL_ERROR
      "Color Diagnostics are only available when using Clang."
    )
  endif()
  target_compile_options(mixxx-lib PUBLIC -fcolor-diagnostics)
endif()

# Clang Sanitizers
set(SANITIZERS "")
option(SANITIZE_ADDRESS "Address Sanitizer" OFF)
if(SANITIZE_ADDRESS)
  list(APPEND SANITIZERS "address")
endif()
option(SANITIZE_UNDEFINED "Clang Undefined Behaviour Sanitizer" OFF)
if(SANITIZE_UNDEFINED)
  list(APPEND SANITIZERS "undefined")
endif()
option(SANITIZE_THREAD "Clang Thread Sanitizer" OFF)
if(SANITIZE_THREAD)
  list(APPEND SANITIZERS "thread")
endif()
if(NOT SANITIZERS STREQUAL "")
  if(NOT (LLVM_CLANG OR GNU_GCC))
    message(FATAL_ERROR "Sanitizers are only available on Clang or GCC")
  endif()
  list(JOIN SANITIZERS "," SANITZERS_JOINED)
  target_compile_options(mixxx-lib PUBLIC -fsanitize=${SANITZERS_JOINED})
  target_link_options(mixxx-lib PUBLIC -fsanitize=${SANITZERS_JOINED})
endif()

# CoreAudio MP3/AAC Decoder
#
# The CoreAudio API is only available on macOS, therefore this option is
# forcibly set to OFF on all other platforms.
cmake_dependent_option(
  COREAUDIO
  "CoreAudio MP3/AAC Decoder"
  ON
  "APPLE"
  OFF
)
if(COREAUDIO)
  target_sources(
    mixxx-lib
    PRIVATE
      src/sources/soundsourcecoreaudio.cpp
      src/sources/v1/legacyaudiosourceadapter.cpp
      lib/apple/CAStreamBasicDescription.cpp
  )
  set_property(
    SOURCE lib/apple/CAStreamBasicDescription.cpp
    APPEND_STRING
    PROPERTY COMPILE_OPTIONS -Wno-deprecated-anon-enum-enum-conversion
  )
  target_compile_definitions(mixxx-lib PUBLIC __COREAUDIO__)
  target_include_directories(mixxx-lib SYSTEM PUBLIC lib/apple)
endif()

# FAAD AAC audio file decoder plugin
find_package(MP4)
find_package(MP4v2)
# It is enabled by default on Linux only, because other targets have other
# solutions. It requires MP4 or MP4v2.
default_option(FAAD "FAAD AAC audio file decoder support" "UNIX;NOT APPLE;MP4_FOUND OR MP4v2_FOUND")
if(FAAD)
  if(NOT MP4_FOUND AND NOT MP4v2_FOUND)
    message(
      FATAL_ERROR
      "FAAD AAC audio support requires libmp4 or libmp4v2 with development headers."
    )
  endif()
  target_sources(
    mixxx-lib
    PRIVATE src/sources/soundsourcem4a.cpp src/sources/libfaadloader.cpp
  )
  target_compile_definitions(mixxx-lib PUBLIC __FAAD__)
  if(MP4v2_FOUND)
    target_compile_definitions(mixxx-lib PUBLIC __MP4V2__)
    target_link_libraries(mixxx-lib PRIVATE MP4v2::MP4v2)
  else()
    target_link_libraries(mixxx-lib PRIVATE MP4::MP4)
  endif()
endif()

# FDK-AAC is loaded dynamically at runtime by EncoderFdkAac using QLibrary,
# so copy it into the Windows and macOS packages, but do not link to it.
if(APPLE AND MACOS_BUNDLE)
  find_library(FDK_AAC_LIBRARY fdk-aac)
  if(FDK_AAC_LIBRARY)
    message(STATUS "Found fdk-aac: ${FDK_AAC_LIBRARY}")
    file(
      COPY ${FDK_AAC_LIBRARY}
      DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/lib/fdk-aac-install"
      FOLLOW_SYMLINK_CHAIN
    )
    install(
      DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib/fdk-aac-install/"
      DESTINATION "${MIXXX_INSTALL_PREFIX}/Contents/Frameworks"
    )
  else()
    message(STATUS "Could NOT find libfdk-aac.dylib")
  endif()
elseif(WIN32)
  # On Windows find_library finds the .lib file, but the installer needs the .dll file.
  find_file(FDK_AAC_DLL fdk-aac.dll PATH_SUFFIXES ${CMAKE_INSTALL_BINDIR})
  if(FDK_AAC_DLL)
    message(STATUS "Found fdk-aac DLL: ${FDK_AAC_DLL}")
    install(FILES ${FDK_AAC_DLL} DESTINATION ${MIXXX_INSTALL_BINDIR})
  else()
    message(STATUS "Could NOT find fdk-aac.dll")
  endif()
endif()

# Google PerfTools
option(GPERFTOOLS "Google PerfTools libtcmalloc linkage" OFF)
option(GPERFTOOLSPROFILER "Google PerfTools libprofiler linkage" OFF)
if((BUILD_BENCH) AND (GPERFTOOLS OR GPERFTOOLSPROFILER))
  find_package(GPerfTools REQUIRED)
  if(GPERFTOOLS)
    target_link_libraries(mixxx-lib PRIVATE GPerfTools::tcmalloc)
  endif()
  if(PERFTOOLSPROFILER)
    target_link_libraries(mixxx-lib PRIVATE GPerfTools::profiler)
  endif()
endif()

# HSS1394 MIDI device
#
# The HSS1394 library is only available on macOS, therefore this option is
# forcibly set to OFF on all other platforms.
if(WIN32 OR APPLE)
  find_package(HSS1394)
else()
  set(HSS1394 OFF)
endif()
cmake_dependent_option(
  HSS1394
  "HSS1394 MIDI device support"
  "${HSS1394_FOUND}"
  "WIN32 OR APPLE"
  OFF
)
if(HSS1394)
  target_sources(
    mixxx-lib
    PRIVATE
      src/controllers/midi/hss1394controller.cpp
      src/controllers/midi/hss1394enumerator.cpp
  )
  target_compile_definitions(mixxx-lib PUBLIC __HSS1394__)
  if(NOT HSS1394_FOUND)
    message(
      FATAL_ERROR
      "HSS1394 MIDI device support requires the libhss1394 and its development headers."
    )
  endif()
  target_link_libraries(mixxx-lib PRIVATE HSS1394::HSS1394)
endif()

# Lilv (LV2)
find_package(lilv)
default_option(LILV "Lilv (LV2) support" "lilv_FOUND")
if(LILV)
  if(NOT lilv_FOUND)
    message(
      FATAL_ERROR
      "Lilv (LV2) support requires the liblilv-0 and LV2 libraries and development headers."
    )
  endif()
  target_sources(
    mixxx-lib
    PRIVATE
      src/effects/backends/lv2/lv2backend.cpp
      src/effects/backends/lv2/lv2effectprocessor.cpp
      src/effects/backends/lv2/lv2manifest.cpp
  )
  target_compile_definitions(mixxx-lib PUBLIC __LILV__)
  target_link_libraries(mixxx-lib PRIVATE lilv::lilv)
  if(BUILD_TESTING)
    target_link_libraries(mixxx-test PRIVATE lilv::lilv)
  endif()
endif()

# Live Broadcasting (Shoutcast)
cmake_dependent_option(
  BROADCAST
  "Live Broadcasting (Shoutcast) support"
  ON
  "NOT IOS"
  OFF
)
if(BROADCAST)
  find_package(Shoutidjc)
  # Check if system lib is at least 2.4.6 and not suffering bugs
  # https://github.com/mixxxdj/mixxx/issues/9681
  # https://github.com/mixxxdj/mixxx/issues/10305
  if(Shoutidjc_FOUND AND Shoutidjc_VERSION VERSION_LESS 2.4.4)
    message(
      STATUS
      "Installed libshout-idjc version: ${Shoutidjc_VERSION} is suffering from issue #9681"
    )
  elseif(Shoutidjc_FOUND AND Shoutidjc_VERSION VERSION_LESS 2.4.6)
    message(
      STATUS
      "Installed libshout version: ${Shout_VERSION} is suffering from issue #10305"
    )
  endif()
  if(NOT Shoutidjc_FOUND OR Shoutidjc_VERSION VERSION_LESS 2.4.6)
    # Fall back to internal library in the lib tree
    message(STATUS "Using internal libshout-idjc")
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/lib/libshout-idjc")
    target_include_directories(
      mixxx-lib
      SYSTEM
      PUBLIC lib/libshout-idjc/include
    )
    if(WIN32)
      target_compile_definitions(
        shout_mixxx
        PRIVATE __WINDOWS__ _CRT_NONSTDC_NO_WARNINGS
      )
    endif()
    target_link_libraries(mixxx-lib PRIVATE shout_mixxx)
  else()
    target_link_libraries(mixxx-lib PRIVATE Shoutidjc::Shoutidjc)
  endif()
  target_sources(
    mixxx-lib
    PRIVATE
      src/preferences/dialog/dlgprefbroadcastdlg.ui
      src/preferences/dialog/dlgprefbroadcast.cpp
      src/broadcast/broadcastmanager.cpp
      src/engine/sidechain/shoutconnection.cpp
      src/preferences/broadcastprofile.cpp
      src/preferences/broadcastsettings.cpp
      src/preferences/broadcastsettings_legacy.cpp
      src/preferences/broadcastsettingsmodel.cpp
      src/encoder/encoderbroadcastsettings.cpp
  )
  target_compile_definitions(mixxx-lib PUBLIC __BROADCAST__)
  if(QML)
    target_compile_definitions(mixxx-qml-lib PUBLIC __BROADCAST__)
  endif()
endif()

# Opus (RFC 6716)
find_package(OpusFile)
find_package(Opus)
default_option(OPUS "Opus (RFC 6716) support" "OpusFile_FOUND")
if(OPUS)
  if(NOT OpusFile_FOUND OR NOT Opus_FOUND)
    message(
      FATAL_ERROR
      "Opus support requires libopus and libopusfile with development headers."
    )
  endif()
  target_sources(
    mixxx-lib
    PRIVATE
      src/sources/soundsourceopus.cpp
      src/encoder/encoderopus.cpp
      src/encoder/encoderopussettings.cpp
  )
  target_compile_definitions(mixxx-lib PUBLIC __OPUS__)
  target_link_libraries(mixxx-lib PRIVATE OpusFile::OpusFile Opus::Opus)
  if(BUILD_TESTING)
    target_link_libraries(mixxx-test PRIVATE OpusFile::OpusFile Opus::Opus)
  endif()
endif()

# MAD MP3 Decoder
find_package(MAD)
find_package(ID3Tag)
default_option(MAD "MAD MP3 Decoder" "MAD_FOUND;ID3Tag_FOUND")
if(MAD)
  if(NOT MAD_FOUND)
    message(
      FATAL_ERROR
      "MAD support requires libmad and its development headers."
    )
  endif()
  if(NOT ID3Tag_FOUND)
    message(
      FATAL_ERROR
      "ID3Tag support requires libid3tag and its development headers."
    )
  endif()
  target_sources(mixxx-lib PRIVATE src/sources/soundsourcemp3.cpp)
  target_compile_definitions(mixxx-lib PUBLIC __MAD__)
  target_link_libraries(mixxx-lib PRIVATE MAD::MAD ID3Tag::ID3Tag)
endif()

# Media Foundation AAC Decoder Plugin
#
# The Media Foundtation API is only available on Windows, therefore this option
# is forcibly set to OFF on all other platforms.
cmake_dependent_option(
  MEDIAFOUNDATION
  "Media Foundation AAC decoder plugin"
  ON
  "WIN32"
  OFF
)
if(MEDIAFOUNDATION)
  find_package(MediaFoundation REQUIRED)
  target_sources(mixxx-lib PRIVATE src/sources/soundsourcemediafoundation.cpp)
  target_compile_definitions(mixxx-lib PUBLIC __MEDIAFOUNDATION__)
  target_include_directories(
    mixxx-lib
    SYSTEM
    PRIVATE ${MediaFoundation_INCLUDE_DIRS}
  )
  target_link_libraries(
    mixxx-lib
    PRIVATE ${MediaFoundation_LIBRARIES} Version.lib
  )
endif()

# Modplug support
find_package(Modplug)
default_option(MODPLUG "Modplug module decoder support" "Modplug_FOUND")
if(MODPLUG)
  if(NOT Modplug_FOUND)
    message(
      FATAL_ERROR
      "Modplug module decoder support requires libmodplug and its development headers."
    )
  endif()
  target_sources(
    mixxx-lib
    PRIVATE
      src/preferences/dialog/dlgprefmodplugdlg.ui
      src/sources/soundsourcemodplug.cpp
      src/preferences/dialog/dlgprefmodplug.cpp
  )
  target_compile_definitions(mixxx-lib PUBLIC __MODPLUG__)
  target_link_libraries(mixxx-lib PRIVATE Modplug::Modplug)
endif()

find_package(Microsoft.GSL CONFIG)
if(Microsoft.GSL_FOUND)
  target_link_libraries(mixxx-lib PRIVATE Microsoft.GSL::GSL)
  if(QML)
    target_link_libraries(mixxx-qml-lib PRIVATE Microsoft.GSL::GSL)
    target_link_libraries(mixxx-qml-libplugin PRIVATE Microsoft.GSL::GSL)
  endif()
else()
  # check if the headers have been installed without cmake config (< 3.1.0)
  check_include_file_cxx(gsl/gsl HAVE_GSL_GSL)
  if(NOT HAVE_GSL_GSL)
    unset(HAVE_GSL_GSL CACHE) # unset cache to re-evaluate this until it succeeds. check_include_file_cxx() has no REQUIRED flag.
    message(FATAL_ERROR "ms-gsl development headers (libmsgsl-dev) not found")
  endif()
endif()

# QtKeychain
option(
  QTKEYCHAIN
  "Secure credentials storage support for Live Broadcasting profiles"
  ON
)
if(QTKEYCHAIN)
  find_package(Qt${QT_VERSION_MAJOR}Keychain REQUIRED)
  target_compile_definitions(mixxx-lib PUBLIC __QTKEYCHAIN__)
  target_link_libraries(mixxx-lib PRIVATE ${QTKEYCHAIN_LIBRARIES})
  target_include_directories(mixxx-lib SYSTEM PUBLIC ${QTKEYCHAIN_INCLUDE_DIRS})
endif()

# USB HID or/and Bulk controller support
find_package(LibUSB)

# USB HID controller support
option(HID "USB HID controller support" ON)
if(HID)
  # hidapi 0.14.0 is the first release, that contains bus type information
  find_package(hidapi 0.14.0)
  if(NOT hidapi_FOUND)
    message(FATAL_ERROR "hidapi >= 0.14.0 not found!")
  else()
    # hidapi has two backends on Linux, one using the kernel's hidraw API and one using libusb.
    # libusb obviously does not support Bluetooth HID devices, so use the hidraw backend. The
    # libusb backend is the default, so hidraw needs to be selected explicitly at link time.
    if(CMAKE_SYSTEM_NAME STREQUAL Linux)
      target_link_libraries(mixxx-lib PRIVATE hidapi::hidraw)
    else()
      target_link_libraries(mixxx-lib PRIVATE hidapi::hidapi)
    endif()
  endif()
  target_sources(
    mixxx-lib
    PRIVATE
      src/controllers/hid/hidcontroller.cpp
      src/controllers/hid/hidiothread.cpp
      src/controllers/hid/hidioglobaloutputreportfifo.cpp
      src/controllers/hid/hidiooutputreport.cpp
      src/controllers/hid/hiddevice.cpp
      src/controllers/hid/hidenumerator.cpp
      src/controllers/hid/hidusagetables.cpp
      src/controllers/hid/legacyhidcontrollermapping.cpp
      src/controllers/hid/legacyhidcontrollermappingfilehandler.cpp
  )
  target_compile_definitions(mixxx-lib PUBLIC __HID__)
endif()

# USB Bulk controller support
default_option(BULK "USB Bulk controller support" "LibUSB_FOUND;NOT WIN32")
if(BULK)
  if(NOT LibUSB_FOUND)
    message(
      FATAL_ERROR
      "USB Bulk controller support requires libusb 1.0 and its development headers."
    )
  endif()
  target_sources(
    mixxx-lib
    PRIVATE
      src/controllers/bulk/bulkcontroller.cpp
      src/controllers/bulk/bulkenumerator.cpp
  )
  if(NOT HID)
    target_sources(
      mixxx-lib
      PRIVATE
        src/controllers/hid/legacyhidcontrollermapping.cpp
        src/controllers/hid/legacyhidcontrollermappingfilehandler.cpp
    )
  endif()
  target_compile_definitions(mixxx-lib PUBLIC __BULK__)
  target_link_libraries(mixxx-lib PRIVATE LibUSB::LibUSB)
endif()

# Vinyl Control
default_option(VINYLCONTROL "Vinyl Control support" "NOT MACAPPSTORE")
if(VINYLCONTROL)
  if(MACAPPSTORE)
    message(
      FATAL_ERROR
      "Mac App Store and Vinyl Control support are mutually exclusive due to licensing issues."
    )
  endif()

  target_sources(
    mixxx-lib
    PRIVATE
      src/vinylcontrol/vinylcontrol.cpp
      src/vinylcontrol/vinylcontrolxwax.cpp
      src/preferences/dialog/dlgprefvinyl.cpp
      src/vinylcontrol/vinylcontrolsignalwidget.cpp
      src/vinylcontrol/vinylcontrolmanager.cpp
      src/vinylcontrol/vinylcontrolprocessor.cpp
      src/vinylcontrol/steadypitch.cpp
      src/engine/controls/vinylcontrolcontrol.cpp
  )
  target_compile_definitions(mixxx-lib PUBLIC __VINYLCONTROL__)

  # Internal xwax library
  add_library(mixxx-xwax STATIC EXCLUDE_FROM_ALL)
  target_sources(
    mixxx-xwax
    PRIVATE
      lib/xwax/timecoder.c
      lib/xwax/timecoder_mk2.c
      lib/xwax/lut.c
      lib/xwax/lut_mk2.c
      lib/xwax/filters.c
      lib/xwax/delayline.c
      lib/xwax/pitch_kalman.c
  )
  target_include_directories(mixxx-xwax SYSTEM PUBLIC lib/xwax)
  target_link_libraries(mixxx-lib PRIVATE mixxx-xwax)
endif()

# rendergraph
add_subdirectory(src/rendergraph)
target_compile_definitions(
  rendergraph_gl
  PUBLIC $<$<CONFIG:Debug>:MIXXX_DEBUG_ASSERTIONS_ENABLED>
)
target_link_libraries(mixxx-lib PUBLIC rendergraph_gl)
target_compile_definitions(mixxx-lib PUBLIC rendergraph=rendergraph_gl)
target_compile_definitions(mixxx-lib PRIVATE allshader=allshader_gl)
if(BUILD_TESTING)
  target_compile_definitions(mixxx-test PRIVATE allshader=allshader_gl)
endif()
if(QML)
  target_compile_definitions(
    rendergraph_sg
    PUBLIC $<$<CONFIG:Debug>:MIXXX_DEBUG_ASSERTIONS_ENABLED>
  )
  target_link_libraries(mixxx-qml-lib PRIVATE rendergraph_sg)
  target_compile_definitions(mixxx-qml-lib PRIVATE rendergraph=rendergraph_sg)
  target_compile_definitions(mixxx-qml-lib PRIVATE __SCENEGRAPH__)
  target_compile_definitions(mixxx-qml-lib PRIVATE allshader=allshader_sg)
endif()

# WavPack audio file support
find_package(wavpack)
default_option(WAVPACK "WavPack audio file support" "wavpack_FOUND")
if(WAVPACK)
  if(NOT wavpack_FOUND)
    message(
      FATAL_ERROR
      "WavPack audio file support requires libwv and its development headers."
    )
  endif()
  target_sources(mixxx-lib PRIVATE src/sources/soundsourcewv.cpp)
  target_compile_definitions(mixxx-lib PUBLIC __WV__)
  target_link_libraries(mixxx-lib PRIVATE WavPack::wavpack)
endif()

target_precompile_headers(
  mixxx-lib
  PUBLIC ${MIXXX_LIB_PRECOMPILED_HEADER} ${MIXXX_COMMON_PRECOMPILED_HEADER}
)
if(BUILD_TESTING)
  target_precompile_headers(mixxx-test REUSE_FROM mixxx-lib)
endif()

# Configure file with build options
file(
  RELATIVE_PATH
  MIXXX_INSTALL_DOCDIR_RELATIVE_TO_DATADIR
  "${CMAKE_INSTALL_PREFIX}/${MIXXX_INSTALL_DATADIR}"
  "${CMAKE_INSTALL_PREFIX}/${MIXXX_INSTALL_DOCDIR}"
)
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/src/config.h"
  @ONLY
)

# Packaging
set(CPACK_PACKAGE_NAME "Mixxx")
set(CPACK_PACKAGE_VENDOR "Mixxx Project")
set(CPACK_PACKAGE_CONTACT "RJ Skerry-Ryan <rryan@mixxx.org>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Digital DJ Application")
set(
  CPACK_PACKAGE_DESCRIPTION_FILE
  "${CMAKE_CURRENT_SOURCE_DIR}/packaging/CPackPackageDescription.txt"
)
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Mixxx")
set(CPACK_PACKAGE_EXECUTABLES "mixxx;Mixxx")
set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/res/images/mixxx_install_logo.bmp")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://www.mixxx.org/")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_STRIP_FILES ON)
set(CPACK_CREATE_DESKTOP_LINKS "mixxx")
set(CPACK_MIXXX_VERSION "${MIXXX_VERSION}")
# Save GIT values just in case they have been set manual via cmake
set(CPACK_GIT_DESCRIBE "${GIT_DESCRIBE}")
set(CPACK_GIT_COMMIT_DATE ${GIT_COMMIT_DATE})

# Detailed version information, git info and package file name are set from
# CPackConfig.cmake, not here.

set(CPACK_SOURCE_IGNORE_FILES "\\\\.#;/#;.*~;\\\\.o$")
list(APPEND CPACK_SOURCE_IGNORE_FILES "/\\\\.git/")
list(APPEND CPACK_SOURCE_IGNORE_FILES "/\\\\.github/")
list(APPEND CPACK_SOURCE_IGNORE_FILES "/build/")
list(APPEND CPACK_SOURCE_IGNORE_FILES "${CMAKE_CURRENT_BINARY_DIR}/")
set(CPACK_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set(CPACK_DEBIAN_PACKAGE_SECTION "sound")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
if(QT6)
  set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "qt6-translations-l10n")
else()
  set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "qttranslations5-l10n")
endif()
set(CPACK_DEBIAN_PACKAGE_SUGGESTS "pdf-viewer, pulseaudio-utils")
set(CPACK_DEBIAN_PACKAGE_REPLACES "mixxx-data")

set(CPACK_DEBIAN_PACKAGE_DEPENDS "")
list(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS "fonts-open-sans")
list(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS "fonts-ubuntu")
if(QT6)
  list(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6sql6-sqlite")
  list(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS "qt6-qpa-plugins")
  if(QT_VERSION VERSION_GREATER_EQUAL 6.8)
    list(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS "qt6-svg-plugins")
  endif()
  if(QML)
    list(
      APPEND
      CPACK_DEBIAN_PACKAGE_DEPENDS
      "qml6-module-qt5compat-graphicaleffects"
    )
    list(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS "qml6-module-qtquick-controls")
    list(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS "qml6-module-qtquick-layouts")
    list(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS "qml6-module-qtquick-nativestyle")
    list(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS "qml6-module-qtquick-templates")
    list(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS "qml6-module-qtquick-window")
    list(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS "qml6-module-qt-labs-qmlmodels")
    list(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS "qml6-module-qtquick-shapes")
    list(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS "qml6-module-qtqml-workerscript")
  endif()
else()
  list(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5sql5-sqlite")
  if(QML)
    list(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS "qml-module-qtquick-controls")
    list(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS "qml-module-qtquick-controls2")
    list(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS "qml-module-qt-labs-qmlmodels")
    list(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS "qml-module-qtquick-shapes")
  endif()
endif()
list(JOIN CPACK_DEBIAN_PACKAGE_DEPENDS ", " CPACK_DEBIAN_PACKAGE_DEPENDS)

set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "${CPACK_PACKAGE_HOMEPAGE_URL}")
set(CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION TRUE)
file(READ ${CPACK_PACKAGE_DESCRIPTION_FILE} CPACK_DEBIAN_PACKAGE_DESCRIPTION)
set(
  CPACK_DEBIAN_PACKAGE_DESCRIPTION_MERGED
  "${CPACK_DEBIAN_PACKAGE_DESCRIPTION}"
)
string(
  PREPEND
  CPACK_DEBIAN_PACKAGE_DESCRIPTION_MERGED
  "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}"
  "\n"
)
string(
  REPLACE
  "\n\n"
  "\n.\n"
  CPACK_DEBIAN_PACKAGE_DESCRIPTION_MERGED
  "${CPACK_DEBIAN_PACKAGE_DESCRIPTION_MERGED}"
)
string(
  REPLACE
  "\n"
  "\n "
  CPACK_DEBIAN_PACKAGE_DESCRIPTION_MERGED
  "${CPACK_DEBIAN_PACKAGE_DESCRIPTION_MERGED}"
)

# This is the version of the package itself and can be advanced or set to
# something like 0ubuntu1 when building a new package from the same version
if(NOT CPACK_DEBIAN_PACKAGE_RELEASE)
  set(CPACK_DEBIAN_PACKAGE_RELEASE 1)
endif()

set(CPACK_DEBIAN_DISTRIBUTION_RELEASES noble plucky questing)
set(CPACK_DEBIAN_SOURCE_DIR ${CMAKE_SOURCE_DIR})
set(
  CPACK_DEBIAN_UPLOAD_PPA_SCRIPT
  "${CMAKE_CURRENT_SOURCE_DIR}/packaging/CPackDebUploadPPA.cmake"
)
set(
  CPACK_DEBIAN_INSTALL_SCRIPT
  "${CMAKE_CURRENT_SOURCE_DIR}/packaging/CPackDebInstall.cmake"
)

set(CPACK_WIX_UPGRADE_GUID "921DC99C-4DCF-478D-B950-50685CB9E6BE")
set(
  CPACK_WIX_LICENSE_RTF
  "${CMAKE_CURRENT_BINARY_DIR}/packaging/wix/LICENSE.rtf"
)
set(CPACK_WIX_PRODUCT_ICON "${CMAKE_SOURCE_DIR}/res/images/icons/ic_mixxx.ico")
set(CPACK_WIX_PROPERTY_ARPHELPLINK "${CPACK_PACKAGE_HOMEPAGE_URL}")
set(
  CPACK_WIX_UI_BANNER
  "${CMAKE_CURRENT_SOURCE_DIR}/packaging/wix/images/banner.bmp"
)
set(
  CPACK_WIX_UI_DIALOG
  "${CMAKE_CURRENT_SOURCE_DIR}/packaging/wix/images/dialog.bmp"
)

set(CPACK_PROJECT_CONFIG_FILE "${CMAKE_SOURCE_DIR}/packaging/CPackConfig.cmake")

if(WIN32)
  # override not working default NSIS
  set(CPACK_GENERATOR WIX)

  if(CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64")
    if(${CMAKE_VERSION} VERSION_LESS "3.24")
      message(FATAL_ERROR "WIX: Need CMake version >= 3.24 for ARM64")
    endif()
    set(CPACK_WIX_ARCHITECTURE "ARM64")
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x64|x86_64|AMD64)$")
    set(CPACK_WIX_ARCHITECTURE "x64")
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(i[3456]86|x86)$")
    set(CPACK_WIX_ARCHITECTURE "x86")
  else()
    message(
      FATAL_ERROR
      "WIX: Unsupported architecture: ${CMAKE_SYSTEM_PROCESSOR}"
    )
  endif()

  # uses CMAKE_PROJECT_VERSION
  configure_file(packaging/wix/LICENSE.rtf.in packaging/wix/LICENSE.rtf @ONLY)
endif()

include(CPack)

if(APPLOCAL_COMPONENT_DEFINED)
  cpack_add_component(applocal HIDDEN REQUIRED)

  # In order to run Mixx from the build directory install applocal components
  add_custom_command(
    TARGET mixxx
    POST_BUILD
    COMMAND
      "${CMAKE_COMMAND}" -DCOMPONENT=applocal
      -DCMAKE_INSTALL_PREFIX="${CMAKE_CURRENT_BINARY_DIR}" -P
      cmake_install.cmake
    COMMENT
      "Install applocal components to allow Mixxx to be run from the build directory."
  )
endif()

if(APPLE AND MACOS_BUNDLE)
  set(BUNDLE_NAME "${MIXXX_INSTALL_PREFIX}")
  foreach(PREFIX ${CMAKE_PREFIX_PATH})
    list(APPEND BUNDLE_DIRS "${PREFIX}/lib")
  endforeach()
  set(
    APPLE_CODESIGN_ENTITLEMENTS
    "${CMAKE_CURRENT_SOURCE_DIR}/packaging/macos/Mixxx.entitlements"
  )

  # Starting with arm64 macOS Apple will require ad-hoc code signatures,
  # which can be generated by setting the identity to a single dash (-).
  # These only include a checksum for verifying integrity, not an actual
  # signature.
  if(NOT APPLE_CODESIGN_IDENTITY)
    set(APPLE_CODESIGN_IDENTITY -)
  endif()

  configure_file(
    cmake/modules/BundleInstall.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/BundleInstall.cmake"
    @ONLY
  )
  install(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/BundleInstall.cmake")
endif()
