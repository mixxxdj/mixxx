cmake_minimum_required(VERSION 3.16)

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

project(mixxx VERSION 2.4.0)
# Work around missing version suffixes support https://gitlab.kitware.com/cmake/cmake/-/issues/16716
set(MIXXX_VERSION_PRERELEASE "alpha-pre") # set to "alpha-pre" "beta" or ""

set(CMAKE_PROJECT_HOMEPAGE_URL "https://www.mixxx.org")
set(CMAKE_PROJECT_DESCRIPTION "Mixxx is Free DJ software that gives you everything you need to perform live mixes.")

# Used for force control of color output
set(BUILD_COLORS "auto" CACHE STRING "Try to use colors auto/always/no")
# Option to disable symlinks
set(USE_SYMLINKS ON CACHE BOOL "Use symlinks in build directory when possible")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
include(CMakeDependentOption)
include(CheckSymbolExists)
include(ExternalProject)
include(GNUInstallDirs)
include(DefaultOption)

#######################################################################
# Compilers and toolchains

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # GNU is GNU GCC
  set(GNU_GCC true)
else()
  set(GNU_GCC false)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # using regular Clang or AppleClang
  set(LLVM_CLANG true)
else()
  set(LLVM_CLANG false)
endif()

if(APPLE AND CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS 10.14)
  add_compile_options(-fno-aligned-allocation)
endif()

# CMake implicitly sets the variable MSVC to true for Microsoft
# Visual C++ or another compiler simulating Visual C++.
# https://cmake.org/cmake/help/latest/variable/MSVC.html

#######################################################################

set(CMAKE_CXX_STANDARD 17)
if(MSVC)
  # Ensure MSVC populates __cplusplus correctly.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")
endif()

# Speed up builds on HDDs and prevent wearing of SDDs
#
# This is only applies to gcc/clang, therefore this option is forcibly set to
# ON on all other compilers.
cmake_dependent_option(BUILD_LOW_MEMORY "Store temporary build files on disk by disabling the build option -pipe" OFF "GNU_GCC OR LLVM_CLANG" ON)
if(NOT BUILD_LOW_MEMORY)
  add_compile_options(-pipe)
endif()

# Profiling
#
# This is only available on Linux, therefore this option is forcibly set to OFF
# on all other platforms.
cmake_dependent_option(PROFILING "Profiling (e.g. gprof) support" OFF "UNIX;NOT APPLE" OFF)
if(PROFILING)
  add_compile_options(-pg)
  add_link_options(-pg)
endif()

#
# Optimizations
#

set(OPTIMIZE "portable" CACHE STRING "Optimization and Tuning (set to off, portable, native, legacy)")
string(TOLOWER "${OPTIMIZE}" OPTIMIZE)
message(STATUS "Optimization level: ${OPTIMIZE}")

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
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # Target architecture is x64 -> x64 has alsways SSE and SSE2 instruction sets
    message(STATUS "x64 Enabling SS2 CPU optimizations (>= Pentium 4)")
    # Define gcc/clang style defines for SSE and SSE2 for compatibility
    add_compile_definitions("__SSE__" "__SSE2__")
  endif()

  # Needed for sccache
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    string(REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REPLACE "/Zi" "/Z7" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
  elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    string(REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    string(REPLACE "/Zi" "/Z7" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
  elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    string(REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
    string(REPLACE "/Zi" "/Z7" CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
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
      #optimize Debug Builds as well, to have "normal" behaviour of mixxx during development
      string(REPLACE "/Od" "" CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG}")
      string(REPLACE "/Od" "" CMAKE_C_FLAGS_DEBUG  "${CMAKE_C_FLAGS}")
      string(REPLACE "/Ob0" "" CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG}")
      string(REPLACE "/Ob0" "" CMAKE_C_FLAGS_DEBUG  "${CMAKE_C_FLAGS}")

      add_compile_options(/O2) # this implies /Od2
      # Remove /RTC1 flag set by CMAKE by default (conflicts with /O2)
      string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG}")
      string(REPLACE "/RTC1" "" CMAKE_C_FLAGS_DEBUG  "${CMAKE_C_FLAGS_DEBUG}")
    elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
      # Reduce the size of the binary in RelWithDebInfo builds
      # Do not use /OPT:ICF because it has no effect.
      # https://github.com/mixxxdj/mixxx/pull/3660#pullrequestreview-600137258
      add_link_options(/OPT:REF)

      # /INCREMENTAL is incompatible with /OPT:REF, but it's the CMake default for RelWithDebInfo
      # The CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO can be defined by the user in the GUI or in CMakeSettings.json,
      # therefore we can't rely on the default.
      string(FIND CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/INCREMENTAL:NO" INCREMENTAL_NO_POSITION)
      if(INCREMENTAL_NO_POSITION EQUAL -1)
        message(STATUS "Overwriting /INCREMENTAL by /INCREMENTAL:NO to allow link time code optimization")
        string(REPLACE "/INCREMENTAL" "/INCREMENTAL:NO" CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}")
      endif()
      # Note: CMAKE_INTERPROCEDURAL_OPTIMIZATION sets the /GL and /LTCG flags for us
    elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
      # Settings for MinSizeRel builds
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
      # Settings for Release builds
    endif()

    if(OPTIMIZE STREQUAL "portable")
      if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        # Target architecture is x86 with SSE and SSE2
        message(STATUS "x86 Enabling SS2 CPU optimizations (>= Pentium 4)")
        # Define gcc/clang style defines for SSE and SSE2 for compatibility
        add_compile_definitions("__SSE__" "__SSE2__")
        # Set compiler option for SSE/SSE2
        add_compile_options(/arch:SSE2)
      endif()
    elseif(OPTIMIZE STREQUAL "native")
      message("Enabling optimizations for native system, specified by user")
      if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        # Target architecture is x86 with SSE and SSE2
        message(STATUS "x86 Enabling SS2 CPU optimizations (>= Pentium 4)")
        # Define gcc/clang style defines for SSE and SSE2 for compatibility
        add_compile_definitions("__SSE__" "__SSE2__")
      endif()
      # Define the target processor instruction and other compiler optimization flags here:
      # https://docs.microsoft.com/en-us/cpp/build/reference/arch-x64?view=msvc-160
      # add_compile_options(/arch:AVX512)
      message(FATAL_ERROR "User need to set the MSVC compiler flags for the native processor here!")
      add_compile_options("/favor:${CMAKE_SYSTEM_PROCESSOR}")
    elseif(OPTIMIZE STREQUAL "legacy")
      if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        message("Enabling pure x64 instruction set (without AVX etc.)")
      else()
        message("Enabling pure i386 instruction set (without SSE/SSE2 etc.)")
      endif()
    else()
      message(FATAL_ERROR "Invalid value passed to OPTIMIZE option: ${OPTIMIZE}")
    endif()
  else()
    # OPTIMIZE=off
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
      #Remove optimize flags set by cmake defaults
      string(REPLACE "/O2" "" CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE}")
      string(REPLACE "/O2" "" CMAKE_C_FLAGS_RELEASE  "${CMAKE_C_FLAGS_RELEASE}")
      string(REPLACE "/Ob2" "" CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE}")
      string(REPLACE "/Ob2" "" CMAKE_C_FLAGS_RELEASE  "${CMAKE_C_FLAGS_RELEASE}")
      add_compile_options(/Od) # this implies /Ob0
      add_compile_options(/RTC1)
    elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
      #Remove optimize flags set by cmake defaults
      string(REPLACE "/O2" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO  "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
      string(REPLACE "/O2" "" CMAKE_C_FLAGS_RELWITHDEBINFO  "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
      string(REPLACE "/Ob2" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO  "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
      string(REPLACE "/Ob2" "" CMAKE_C_FLAGS_RELWITHDEBINFO  "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
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
    add_compile_options(
      -O3
      -ffast-math
      -funroll-loops
    )
    # set -fomit-frame-pointer when we don't profile and are not using
    # Clang sanitizers.
    # Note: It is only included in -O on machines where it does not
    # interfere with debugging
    if(NOT PROFILING AND NOT CLANG_SANITIZERS)
      add_compile_options(-fomit-frame-pointer)
    endif()

    if(OPTIMIZE STREQUAL "portable")
      # portable: sse2 CPU (>= Pentium 4)
      if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(i[3456]86|x86|x64|x86_64|AMD64)$")
        message(STATUS "Enabling SS2 CPU optimizations (>= Pentium 4)")
        add_compile_options(-mtune=generic)
        # -mtune=generic picks the most common, but compatible options.
        # on arm platforms equivalent to -march=arch
        if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
          # the sse flags are not set by default on 32 bit builds
          # but are not supported on arm builds
          add_compile_options(
            -msse2
            -mfpmath=sse)
        endif()
        # TODO(rryan): macOS can use SSE3, and possibly SSE 4.1 once
        # we require macOS 10.12.
        # https://stackoverflow.com/questions/45917280/mac-osx-minumum-support-sse-version
      elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm|armv7.*)$") # but not armv8
        add_compile_options(
          -mfloat-abi=hard
          -mfpu=neon
        )
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
        add_compile_options(
          -mfloat-abi=hard
          -mfpu=neon
        )
      endif()
    elseif(OPTIMIZE STREQUAL "legacy")
      if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(i[3456]86|x86|x64|x86_64|AMD64)$")
        message("Enabling pure i386 code")
        add_compile_options(-mtune=generic)
        # -mtune=generic pick the most common, but compatible options.
        # on arm platforms equivalent to -march=arch
      endif()
    else()
      message(FATAL_ERROR "Invalid value passed to OPTIMIZE option: ${OPTIMIZE}")
    endif()
  endif()
endif()

if(WIN32)
  # Add support for lib prefix on Windows
  set(CMAKE_FIND_LIBRARY_PREFIXES "" "lib")
endif()

if(MSVC)
  # sccache support
  find_program(SCCACHE_EXECUTABLE "sccache")
  if(SCCACHE_EXECUTABLE)
    message(STATUS "Found sccache: ${SCCACHE_EXECUTABLE}")
  else()
    message(STATUS "Could NOT find sccache (missing executable)")
  endif()
  default_option(SCCACHE_SUPPORT "Enable sccache support" "SCCACHE_EXECUTABLE")
  message(STATUS "Support for sccache: ${SCCACHE_SUPPORT}")
  if(SCCACHE_SUPPORT)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "sccache")
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
  if(CCACHE_SUPPORT)
    if(GNU_GCC OR LLVM_CLANG)
      # without this compiler messages in `make` backend would be uncolored
      set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fdiagnostics-color=${BUILD_COLORS}")
    endif()
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "ccache")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "ccache")
  endif()
  message(STATUS "Support for ccache: ${CCACHE_SUPPORT}")
endif()

if(CMAKE_VERSION VERSION_LESS "3.7.0")
  set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(CLANG_TIDY "" CACHE STRING "CMAKE_CXX_CLANG_TIDY equivalent that only applies to mixxx sources, not bundled dependencies")

# Mixxx itself
add_library(mixxx-lib STATIC EXCLUDE_FROM_ALL
  src/analyzer/analyzerbeats.cpp
  src/analyzer/analyzerebur128.cpp
  src/analyzer/analyzergain.cpp
  src/analyzer/analyzerkey.cpp
  src/analyzer/analyzersilence.cpp
  src/analyzer/analyzerthread.cpp
  src/analyzer/analyzerwaveform.cpp
  src/analyzer/plugins/analyzerqueenmarybeats.cpp
  src/analyzer/plugins/analyzerqueenmarykey.cpp
  src/analyzer/plugins/analyzersoundtouchbeats.cpp
  src/analyzer/plugins/buffering_utils.cpp
  src/analyzer/trackanalysisscheduler.cpp
  src/audio/types.cpp
  src/audio/signalinfo.cpp
  src/audio/streaminfo.cpp
  src/control/control.cpp
  src/control/controlaudiotaperpot.cpp
  src/control/controlbehavior.cpp
  src/control/controleffectknob.cpp
  src/control/controlencoder.cpp
  src/control/controlindicator.cpp
  src/control/controllinpotmeter.cpp
  src/control/controllogpotmeter.cpp
  src/control/controlmodel.cpp
  src/control/controlobject.cpp
  src/control/controlobjectscript.cpp
  src/control/controlpotmeter.cpp
  src/control/controlproxy.cpp
  src/control/controlpushbutton.cpp
  src/control/controlttrotary.cpp
  src/controllers/controller.cpp
  src/controllers/controllerdebug.cpp
  src/controllers/controllerenumerator.cpp
  src/controllers/controllerinputmappingtablemodel.cpp
  src/controllers/controllerlearningeventfilter.cpp
  src/controllers/controllermanager.cpp
  src/controllers/controllermappingtablemodel.cpp
  src/controllers/controllermappinginfo.cpp
  src/controllers/controllermappinginfoenumerator.cpp
  src/controllers/controlleroutputmappingtablemodel.cpp
  src/controllers/controlpickermenu.cpp
  src/controllers/legacycontrollermappingfilehandler.cpp
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
  src/controllers/scripting/controllerscriptenginebase.cpp
  src/controllers/scripting/controllerscriptmoduleengine.cpp
  src/controllers/scripting/colormapper.cpp
  src/controllers/scripting/colormapperjsproxy.cpp
  src/controllers/scripting/legacy/controllerscriptenginelegacy.cpp
  src/controllers/scripting/legacy/controllerscriptinterfacelegacy.cpp
  src/controllers/scripting/legacy/scriptconnection.cpp
  src/controllers/scripting/legacy/scriptconnectionjsproxy.cpp
  src/controllers/keyboard/keyboardeventfilter.cpp
  src/controllers/learningutils.cpp
  src/controllers/midi/legacymidicontrollermapping.cpp
  src/controllers/midi/legacymidicontrollermappingfilehandler.cpp
  src/controllers/midi/midicontroller.cpp
  src/controllers/midi/midienumerator.cpp
  src/controllers/midi/midimessage.cpp
  src/controllers/midi/midioutputhandler.cpp
  src/controllers/midi/midiutils.cpp
  src/controllers/midi/portmidicontroller.cpp
  src/controllers/midi/portmidienumerator.cpp
  src/controllers/softtakeover.cpp
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
  src/effects/builtin/autopaneffect.cpp
  src/effects/builtin/balanceeffect.cpp
  src/effects/builtin/bessel4lvmixeqeffect.cpp
  src/effects/builtin/bessel8lvmixeqeffect.cpp
  src/effects/builtin/biquadfullkilleqeffect.cpp
  src/effects/builtin/bitcrushereffect.cpp
  src/effects/builtin/builtinbackend.cpp
  src/effects/builtin/echoeffect.cpp
  src/effects/builtin/filtereffect.cpp
  src/effects/builtin/flangereffect.cpp
  src/effects/builtin/graphiceqeffect.cpp
  src/effects/builtin/linkwitzriley8eqeffect.cpp
  src/effects/builtin/loudnesscontoureffect.cpp
  src/effects/builtin/metronomeeffect.cpp
  src/effects/builtin/whitenoiseeffect.cpp
  src/effects/builtin/moogladder4filtereffect.cpp
  src/effects/builtin/parametriceqeffect.cpp
  src/effects/builtin/phasereffect.cpp
  src/effects/builtin/reverbeffect.cpp
  src/effects/builtin/threebandbiquadeqeffect.cpp
  src/effects/builtin/tremoloeffect.cpp
  src/effects/effect.cpp
  src/effects/effectbuttonparameterslot.cpp
  src/effects/effectchain.cpp
  src/effects/effectchainmanager.cpp
  src/effects/effectchainslot.cpp
  src/effects/effectmanifest.cpp
  src/effects/effectmanifestparameter.cpp
  src/effects/effectparameter.cpp
  src/effects/effectparameterslot.cpp
  src/effects/effectparameterslotbase.cpp
  src/effects/effectrack.cpp
  src/effects/effectsbackend.cpp
  src/effects/effectslot.cpp
  src/effects/effectsmanager.cpp
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
  src/engine/bufferscalers/enginebufferscalerubberband.cpp
  src/engine/bufferscalers/enginebufferscalest.cpp
  src/engine/cachingreader/cachingreader.cpp
  src/engine/cachingreader/cachingreaderchunk.cpp
  src/engine/cachingreader/cachingreaderworker.cpp
  src/engine/channelmixer_autogen.cpp
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
  src/engine/effects/engineeffectrack.cpp
  src/engine/effects/engineeffectsmanager.cpp
  src/engine/enginebuffer.cpp
  src/engine/enginedelay.cpp
  src/engine/enginemaster.cpp
  src/engine/engineobject.cpp
  src/engine/enginepregain.cpp
  src/engine/enginesidechaincompressor.cpp
  src/engine/enginetalkoverducking.cpp
  src/engine/enginevumeter.cpp
  src/engine/engineworker.cpp
  src/engine/engineworkerscheduler.cpp
  src/engine/enginexfader.cpp
  src/engine/filters/enginefilter.cpp
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
  src/library/analysisfeature.cpp
  src/library/analysislibrarytablemodel.cpp
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
  src/library/bpmdelegate.cpp
  src/library/browse/browsefeature.cpp
  src/library/browse/browsetablemodel.cpp
  src/library/browse/browsethread.cpp
  src/library/browse/foldertreemodel.cpp
  src/library/colordelegate.cpp
  src/library/columncache.cpp
  src/library/coverart.cpp
  src/library/coverartcache.cpp
  src/library/coverartdelegate.cpp
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
  src/library/dlganalysis.cpp
  src/library/dlganalysis.ui
  src/library/dlgcoverartfullsize.cpp
  src/library/dlgcoverartfullsize.ui
  src/library/dlghidden.cpp
  src/library/dlghidden.ui
  src/library/dlgmissing.cpp
  src/library/dlgmissing.ui
  src/library/dlgtagfetcher.cpp
  src/library/dlgtagfetcher.ui
  src/library/dlgtrackinfo.cpp
  src/library/dlgtrackinfo.ui
  src/library/dlgtrackmetadataexport.cpp
  src/library/export/dlgtrackexport.ui
  src/library/export/trackexportdlg.cpp
  src/library/export/trackexportwizard.cpp
  src/library/export/trackexportworker.cpp
  src/library/externaltrackcollection.cpp
  src/library/hiddentablemodel.cpp
  src/library/itunes/itunesfeature.cpp
  src/library/library.cpp
  src/library/librarycontrol.cpp
  src/library/libraryfeature.cpp
  src/library/librarytablemodel.cpp
  src/library/locationdelegate.cpp
  src/library/missingtablemodel.cpp
  src/library/mixxxlibraryfeature.cpp
  src/library/parser.cpp
  src/library/parsercsv.cpp
  src/library/parserm3u.cpp
  src/library/parserpls.cpp
  src/library/playlisttablemodel.cpp
  src/library/previewbuttondelegate.cpp
  src/library/proxytrackmodel.cpp
  src/library/recording/dlgrecording.cpp
  src/library/recording/dlgrecording.ui
  src/library/recording/recordingfeature.cpp
  src/library/rekordbox/rekordbox_anlz.cpp
  src/library/rekordbox/rekordbox_pdb.cpp
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
  src/library/stardelegate.cpp
  src/library/stareditor.cpp
  src/library/starrating.cpp
  src/library/tableitemdelegate.cpp
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
  src/coreservices.cpp
  src/mixxx.cpp
  src/mixxxapplication.cpp
  src/musicbrainz/chromaprinter.cpp
  src/musicbrainz/crc.cpp
  src/musicbrainz/gzip.cpp
  src/musicbrainz/musicbrainz.cpp
  src/musicbrainz/musicbrainzxml.cpp
  src/musicbrainz/tagfetcher.cpp
  src/musicbrainz/web/acoustidlookuptask.cpp
  src/musicbrainz/web/musicbrainzrecordingstask.cpp
  src/network/jsonwebtask.cpp
  src/network/networktask.cpp
  src/network/webtask.cpp
  src/preferences/configobject.cpp
  src/preferences/dialog/dlgprefautodj.cpp
  src/preferences/dialog/dlgprefautodjdlg.ui
  src/preferences/dialog/dlgprefbeats.cpp
  src/preferences/dialog/dlgprefbeatsdlg.ui
  src/preferences/dialog/dlgprefcolors.cpp
  src/preferences/dialog/dlgprefcolorsdlg.ui
  src/preferences/dialog/dlgprefcrossfader.cpp
  src/preferences/dialog/dlgprefcrossfaderdlg.ui
  src/preferences/dialog/dlgprefdeck.cpp
  src/preferences/dialog/dlgprefdeckdlg.ui
  src/preferences/dialog/dlgprefeffects.cpp
  src/preferences/dialog/dlgprefeffectsdlg.ui
  src/preferences/dialog/dlgprefeq.cpp
  src/preferences/dialog/dlgprefeqdlg.ui
  src/preferences/dialog/dlgpreferencepage.cpp
  src/preferences/dialog/dlgpreferences.cpp
  src/preferences/dialog/dlgpreferencesdlg.ui
  src/preferences/dialog/dlgprefinterface.cpp
  src/preferences/dialog/dlgprefinterfacedlg.ui
  src/preferences/dialog/dlgprefkey.cpp
  src/preferences/dialog/dlgprefkeydlg.ui
  src/preferences/dialog/dlgpreflibrary.cpp
  src/preferences/dialog/dlgpreflibrarydlg.ui
  src/preferences/dialog/dlgpreflv2dlg.ui
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
  src/preferences/effectsettingsmodel.cpp
  src/preferences/colorpaletteeditor.cpp
  src/preferences/colorpaletteeditormodel.cpp
  src/preferences/colorpalettesettings.cpp
  src/preferences/replaygainsettings.cpp
  src/preferences/settingsmanager.cpp
  src/preferences/upgrade.cpp
  src/recording/recordingmanager.cpp
  src/skin/legacy/colorschemeparser.cpp
  src/skin/legacy/imgcolor.cpp
  src/skin/legacy/imginvert.cpp
  src/skin/legacy/imgloader.cpp
  src/skin/legacy/launchimage.cpp
  src/skin/legacy/legacyskinparser.cpp
  src/skin/legacy/pixmapsource.cpp
  src/skin/legacy/legacyskin.cpp
  src/skin/qml/qmlcontrolproxy.cpp
  src/skin/qml/qmlskin.cpp
  src/skin/legacy/skincontext.cpp
  src/skin/legacy/tooltips.cpp
  src/skin/skinloader.cpp
  src/soundio/sounddevice.cpp
  src/soundio/sounddevicenetwork.cpp
  src/soundio/sounddeviceportaudio.cpp
  src/soundio/soundmanager.cpp
  src/soundio/soundmanagerconfig.cpp
  src/soundio/soundmanagerutil.cpp
  src/sources/audiosource.cpp
  src/sources/audiosourcestereoproxy.cpp
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
  src/track/beatgrid.cpp
  src/track/beatmap.cpp
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
  src/track/serato/cueinfoimporter.cpp
  src/track/serato/markers.cpp
  src/track/serato/markers2.cpp
  src/track/serato/tags.cpp
  src/track/track.cpp
  src/track/trackinfo.cpp
  src/track/trackmetadata.cpp
  src/track/tracknumbers.cpp
  src/track/trackrecord.cpp
  src/track/trackref.cpp
  src/track/taglib/trackmetadata_ape.cpp
  src/track/taglib/trackmetadata_common.cpp
  src/track/taglib/trackmetadata_file.cpp
  src/track/taglib/trackmetadata_id3v2.cpp
  src/track/taglib/trackmetadata_mp4.cpp
  src/track/taglib/trackmetadata_riff.cpp
  src/track/taglib/trackmetadata_xiph.cpp
  src/util/battery/battery.cpp
  src/util/cache.cpp
  src/util/cmdlineargs.cpp
  src/util/color/color.cpp
  src/util/color/colorpalette.cpp
  src/util/color/predefinedcolorpalettes.cpp
  src/util/console.cpp
  src/util/db/dbconnection.cpp
  src/util/db/dbconnectionpool.cpp
  src/util/db/dbconnectionpooled.cpp
  src/util/db/dbconnectionpooler.cpp
  src/util/db/dbid.cpp
  src/util/db/fwdsqlquery.cpp
  src/util/db/fwdsqlqueryselectresult.cpp
  src/util/db/sqllikewildcardescaper.cpp
  src/util/db/sqlite.cpp
  src/util/db/sqlqueryfinisher.cpp
  src/util/db/sqlstringformatter.cpp
  src/util/db/sqltransaction.cpp
  src/util/desktophelper.cpp
  src/util/dnd.cpp
  src/util/duration.cpp
  src/util/experiment.cpp
  src/util/fileaccess.cpp
  src/util/fileinfo.cpp
  src/util/imageutils.cpp
  src/util/indexrange.cpp
  src/util/logger.cpp
  src/util/logging.cpp
  src/util/mac.cpp
  src/util/movinginterquartilemean.cpp
  src/util/performancetimer.cpp
  src/util/rangelist.cpp
  src/util/readaheadsamplebuffer.cpp
  src/util/rlimit.cpp
  src/util/rotary.cpp
  src/util/sample.cpp
  src/util/samplebuffer.cpp
  src/util/sandbox.cpp
  src/util/semanticversion.cpp
  src/util/screensaver.cpp
  src/util/sleepableqthread.cpp
  src/util/stat.cpp
  src/util/statmodel.cpp
  src/util/statsmanager.cpp
  src/util/tapfilter.cpp
  src/util/task.cpp
  src/util/taskmonitor.cpp
  src/util/threadcputimer.cpp
  src/util/time.cpp
  src/util/timer.cpp
  src/util/valuetransformer.cpp
  src/util/versionstore.cpp
  src/util/widgethelper.cpp
  src/util/widgetrendertimer.cpp
  src/util/workerthread.cpp
  src/util/workerthreadscheduler.cpp
  src/util/xml.cpp
  src/waveform/guitick.cpp
  src/waveform/renderers/glslwaveformrenderersignal.cpp
  src/waveform/renderers/glvsynctestrenderer.cpp
  src/waveform/renderers/glwaveformrendererfilteredsignal.cpp
  src/waveform/renderers/glwaveformrendererrgb.cpp
  src/waveform/renderers/glwaveformrenderersimplesignal.cpp
  src/waveform/renderers/qtvsynctestrenderer.cpp
  src/waveform/renderers/qtwaveformrendererfilteredsignal.cpp
  src/waveform/renderers/qtwaveformrenderersimplesignal.cpp
  src/waveform/renderers/waveformmark.cpp
  src/waveform/renderers/waveformmarkrange.cpp
  src/waveform/renderers/waveformmarkset.cpp
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
  src/waveform/widgets/glrgbwaveformwidget.cpp
  src/waveform/widgets/glsimplewaveformwidget.cpp
  src/waveform/widgets/glslwaveformwidget.cpp
  src/waveform/widgets/glvsynctestwidget.cpp
  src/waveform/widgets/glwaveformwidget.cpp
  src/waveform/widgets/hsvwaveformwidget.cpp
  src/waveform/widgets/qthsvwaveformwidget.cpp
  src/waveform/widgets/qtrgbwaveformwidget.cpp
  src/waveform/widgets/qtsimplewaveformwidget.cpp
  src/waveform/widgets/qtvsynctestwidget.cpp
  src/waveform/widgets/qtwaveformwidget.cpp
  src/waveform/widgets/rgbwaveformwidget.cpp
  src/waveform/widgets/softwarewaveformwidget.cpp
  src/waveform/widgets/waveformwidgetabstract.cpp
  src/widget/controlwidgetconnection.cpp
  src/widget/hexspinbox.cpp
  src/widget/paintable.cpp
  src/widget/wanalysislibrarytableview.cpp
  src/widget/wbasewidget.cpp
  src/widget/wbattery.cpp
  src/widget/wbeatspinbox.cpp
  src/widget/wcolorpicker.cpp
  src/widget/wcolorpickeraction.cpp
  src/widget/wcombobox.cpp
  src/widget/wcoverart.cpp
  src/widget/wcoverartlabel.cpp
  src/widget/wcoverartmenu.cpp
  src/widget/wcuemenupopup.cpp
  src/widget/wdisplay.cpp
  src/widget/weffect.cpp
  src/widget/weffectbuttonparameter.cpp
  src/widget/weffectchain.cpp
  src/widget/weffectparameter.cpp
  src/widget/weffectparameterbase.cpp
  src/widget/weffectparameterknob.cpp
  src/widget/weffectparameterknobcomposed.cpp
  src/widget/weffectpushbutton.cpp
  src/widget/weffectselector.cpp
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
  src/widget/wnumber.cpp
  src/widget/wnumberdb.cpp
  src/widget/wnumberpos.cpp
  src/widget/wnumberrate.cpp
  src/widget/woverview.cpp
  src/widget/woverviewhsv.cpp
  src/widget/woverviewlmh.cpp
  src/widget/woverviewrgb.cpp
  src/widget/wpixmapstore.cpp
  src/widget/wpushbutton.cpp
  src/widget/wrecordingduration.cpp
  src/widget/wsearchlineedit.cpp
  src/widget/wsearchrelatedtracksmenu.cpp
  src/widget/wsingletoncontainer.cpp
  src/widget/wsizeawarestack.cpp
  src/widget/wskincolor.cpp
  src/widget/wslidercomposed.cpp
  src/widget/wspinny.cpp
  src/widget/wsplitter.cpp
  src/widget/wstarrating.cpp
  src/widget/wstatuslight.cpp
  src/widget/wtime.cpp
  src/widget/wtrackmenu.cpp
  src/widget/wtrackproperty.cpp
  src/widget/wtracktableview.cpp
  src/widget/wtracktableviewheader.cpp
  src/widget/wtracktext.cpp
  src/widget/wtrackwidgetgroup.cpp
  src/widget/wvumeter.cpp
  src/widget/wwaveformviewer.cpp
  src/widget/wwidget.cpp
  src/widget/wwidgetgroup.cpp
  src/widget/wwidgetstack.cpp
  src/widget/wraterange.cpp
)
set_target_properties(mixxx-lib PROPERTIES AUTOMOC ON AUTOUIC ON CXX_CLANG_TIDY "${CLANG_TIDY}")
target_include_directories(mixxx-lib PUBLIC src "${CMAKE_CURRENT_BINARY_DIR}/src")
if(UNIX AND NOT APPLE)
  set(MIXXX_SETTINGS_PATH ".mixxx/")
endif()

# Disable warnings in generated source files
set_property(
  SOURCE src/library/rekordbox/rekordbox_anlz.cpp
  APPEND_STRING
  PROPERTY CXX_CLANG_TIDY ""
)
set_property(
  SOURCE src/library/rekordbox/rekordbox_pdb.cpp
  APPEND_STRING
  PROPERTY CXX_CLANG_TIDY ""
)
if(GNU_GCC OR LLVM_CLANG)
  set_property(
    SOURCE src/library/rekordbox/rekordbox_anlz.cpp
    APPEND_STRING
    PROPERTY COMPILE_OPTIONS -Wno-unused-parameter
  )
  set_property(
    SOURCE src/library/rekordbox/rekordbox_pdb.cpp
    APPEND_STRING
    PROPERTY COMPILE_OPTIONS -Wno-unused-parameter -Wno-switch
  )
elseif(MSVC)
  set_property(
    SOURCE src/library/rekordbox/rekordbox_anlz.cpp
    APPEND_STRING
    PROPERTY COMPILE_OPTIONS /w
  )
  set_property(
    SOURCE src/library/rekordbox/rekordbox_pdb.cpp
    APPEND_STRING
    PROPERTY COMPILE_OPTIONS /w
  )
endif()

option(WARNINGS_PEDANTIC "Let the compiler show even more warnings" OFF)
if(MSVC)
  if(WARNINGS_PEDANTIC)
    target_compile_options(mixxx-lib PUBLIC /W4)
  else()
    target_compile_options(mixxx-lib PUBLIC /W3)
    target_compile_definitions(mixxx-lib PUBLIC _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING _CRT_SECURE_NO_WARNINGS)
  endif()
else()
  target_compile_options(mixxx-lib PUBLIC -Wall -Wextra -Woverloaded-virtual -Wfloat-conversion -Werror=return-type -Wformat=2 -Wundef)
  if(WARNINGS_PEDANTIC)
    target_compile_options(mixxx-lib PUBLIC -pedantic)
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

option(DEBUG_ASSERTIONS_FATAL "Fail if debug become true assertions" OFF)
if(DEBUG_ASSERTIONS_FATAL)
  target_compile_definitions(mixxx-lib PUBLIC MIXXX_DEBUG_ASSERTIONS_FATAL MIXXX_DEBUG_ASSERTIONS_ENABLED)
  if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "DEBUG_ASSERT statements have been enabled because DEBUG_ASSERTIONS_FATAL is ON.")
  endif()
endif()

target_compile_definitions(mixxx-lib PUBLIC
  "${CMAKE_SYSTEM_PROCESSOR}"
  $<$<CONFIG:Debug>:MIXXX_BUILD_DEBUG>
  $<$<CONFIG:Debug>:MIXXX_DEBUG_ASSERTIONS_ENABLED>
  $<$<NOT:$<CONFIG:Debug>>:MIXXX_BUILD_RELEASE>
  # Disable assert.h assertions in release mode. Some libraries use
  # this as a signal for when to enable code that should be disabled
  # in release mode.
  $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
)

# Mac-specific options
#
# These options are OFF by default, and since they are only available on macOS,
# they are forcibly set to OFF on all other platforms.
cmake_dependent_option(MACOS_BUNDLE "Install files to proper locations to make an .app bundle" OFF "APPLE" OFF)
cmake_dependent_option(MACAPPSTORE "Build for Mac App Store" OFF "APPLE" OFF)
if(MACAPPSTORE)
  target_compile_definitions(mixxx-lib PUBLIC __MACAPPSTORE__)
endif()

# Windows-specific options
if(WIN32)
  # https://docs.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt
  # _WIN32_WINNT_WIN7 = 0x0601
  target_compile_definitions(mixxx-lib PUBLIC WINVER=0x0601)
  target_compile_definitions(mixxx-lib PUBLIC _WIN32_WINNT=0x0601)
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
if (APPLE AND MACOS_BUNDLE)
  set(MIXXX_INSTALL_BINDIR "${CMAKE_INSTALL_BINDIR}")
  set(MACOS_BUNDLE_NAME mixxx)
  set(MIXXX_INSTALL_PREFIX "${MACOS_BUNDLE_NAME}.app")
  set(MIXXX_INSTALL_DATADIR "${MIXXX_INSTALL_PREFIX}/Contents/Resources")
  set(MIXXX_INSTALL_DOCDIR "${MIXXX_INSTALL_DATADIR}")
  set(MIXXX_INSTALL_LICENSEDIR "${MIXXX_INSTALL_DATADIR}/licenses")
elseif (UNIX)
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

  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_compile_definitions(mixxx-lib PUBLIC WIN32)
  else()
    target_compile_definitions(mixxx-lib PUBLIC WIN64)
  endif()

  target_link_libraries(mixxx-lib PRIVATE shell32)

  if(MSVC)
    if(NOT STATIC_DEPS OR CMAKE_BUILD_TYPE STREQUAL "Debug")
      target_link_options(mixxx-lib PUBLIC /nodefaultlib:LIBCMT.lib /nodefaultlib:LIBCMTd.lib)
    endif()
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
add_executable(mixxx WIN32 src/main.cpp)
set_target_properties(mixxx-lib PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY}")
target_link_libraries(mixxx PRIVATE mixxx-lib mixxx-gitinfostore)

#
# Installation and Packaging
#
if (APPLE)
  if (MACOS_BUNDLE)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/osx/application.icns" DESTINATION ${MIXXX_INSTALL_DATADIR})

    set(MACOS_BUNDLE_VERSION "${CMAKE_PROJECT_VERSION}")
    set(MACOS_BUNDLE_SHORTVERSION "${CMAKE_PROJECT_VERSION}")

    set_target_properties(mixxx PROPERTIES
        MACOSX_BUNDLE true
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/packaging/macos/bundle.plist.in"
    )
  endif()
endif()

if(WIN32)
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "${MIXXX_INSTALL_BINDIR}")
  if(MSVC AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_INSTALL_DEBUG_LIBRARIES true)
  endif()
  include(InstallRequiredSystemLibraries)
endif()

if(WIN32)
  option(WINDOWS_CODESIGN "Sign Windows executables and libraries with digital certificate" OFF)
  mark_as_advanced(WINDOWS_CODESIGN)
  if(WINDOWS_CODESIGN)
    set(WINDOWS_CODESIGN_CERTIFICATE_PATH "$ENV{WINDOWS_CODESIGN_CERTIFICATE_PATH}" CACHE STRING "Path to signtool certificate")
    set(WINDOWS_CODESIGN_CERTIFICATE_PASSWORD "$ENV{WINDOWS_CODESIGN_CERTIFICATE_PASSWORD}" CACHE STRING "Password of signtool certificate")
    if("${WINDOWS_CODESIGN_CERTIFICATE_PATH}" STREQUAL "" AND "${WINDOWS_CODESIGN_CERTIFICATE_PASSWORD}" STREQUAL "")
        set(WINDOWS_CODESIGN_ARGS /a /t http://timestamp.verisign.com/scripts/timstamp.dll CACHE STRING "parameters for signtool (list)")
    else()
        set(WINDOWS_CODESIGN_ARGS /f ${WINDOWS_CODESIGN_CERTIFICATE_PATH} /p ${WINDOWS_CODESIGN_CERTIFICATE_PASSWORD} CACHE STRING "parameters for signtool (list)")
    endif()
    find_program(SIGNTOOL_EXECUTABLE signtool)
    if(NOT SIGNTOOL_EXECUTABLE)
      message(FATAL_ERROR "signtool is not found. Signing executables not possible")
    endif()
    message(STATUS "Found signtool: ${SIGNTOOL_EXECUTABLE}")

    # Check if we're able to sign an executable
    if(NOT DEFINED WINDOWS_CODESIGN_OK)
      file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/testsign.c "int main(){return 0;}")
      file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testsign)
      try_compile(
        RESULT ${CMAKE_CURRENT_BINARY_DIR}/testsign ${CMAKE_CURRENT_BINARY_DIR}/testsign.c
        COPY_FILE ${CMAKE_CURRENT_BINARY_DIR}/testsign.exe
      )
      execute_process(
        COMMAND ${SIGNTOOL_EXECUTABLE} sign ${WINDOWS_CODESIGN_ARGS} ${CMAKE_CURRENT_BINARY_DIR}/testsign.exe
        RESULT_VARIABLE ERR OUTPUT_QUIET
      )
      if(ERR EQUAL 0)
        message(STATUS "Windows codesigning via signtool is working")
        set(WINDOWS_CODESIGN_OK 1 CACHE INTERNAL "Can sign executables")
      else()
        message(FATAL_ERROR "Could NOT codesign test sample (signtool failed)")
        set(WINDOWS_CODESIGN_OK 0 CACHE INTERNAL "Invalid or missing certificate")
      endif()
    endif()
    mark_as_advanced(SIGNTOOL_EXECUTABLE SIGNTOOL_ARGS)
  endif()

  macro(windows_codesign_target CODESIGN_TARGET)
    add_custom_command(
        TARGET "${CODESIGN_TARGET}" POST_BUILD
        COMMAND ${SIGNTOOL_EXECUTABLE} sign ${WINDOWS_CODESIGN_ARGS} $<TARGET_FILE:${CODESIGN_TARGET}>
        COMMENT "Signining target ${CODESIGN_TARGET}"
        VERBATIM
    )
  endmacro()

  if(WINDOWS_CODESIGN)
    windows_codesign_target(mixxx)
  endif()
endif()


install(
  TARGETS
    mixxx
  RUNTIME DESTINATION
    "${MIXXX_INSTALL_BINDIR}"
  BUNDLE DESTINATION
    .
)

# Skins
install(
  DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}/res/skins"
  DESTINATION
    "${MIXXX_INSTALL_DATADIR}"
)

# Controller mappings
install(
  DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}/res/controllers"
  DESTINATION
    "${MIXXX_INSTALL_DATADIR}"
)

# Translation files
install(
  DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}/res/translations"
  DESTINATION
    "${MIXXX_INSTALL_DATADIR}"
  FILES_MATCHING PATTERN
    "*.qm"
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
    DIRECTORY
      "${CMAKE_CURRENT_SOURCE_DIR}/res/fonts"
    DESTINATION
      "${MIXXX_INSTALL_DATADIR}"
  )
endif()

# Keyboard mapping(s)
install(
  DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}/res/keyboard"
  DESTINATION
    "${MIXXX_INSTALL_DATADIR}"
)

# Licenses
install(
  FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
    "${CMAKE_CURRENT_SOURCE_DIR}/COPYING"
  DESTINATION
    "${MIXXX_INSTALL_LICENSEDIR}"
)

# Documentation
install(
  FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
    "${CMAKE_CURRENT_SOURCE_DIR}/res/Mixxx-Keyboard-Shortcuts.pdf"
  DESTINATION
    "${MIXXX_INSTALL_DOCDIR}"
)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/res/Mixxx-Manual.pdf")
  install(
    FILES
      "${CMAKE_CURRENT_SOURCE_DIR}/res/Mixxx-Manual.pdf"
    DESTINATION
      "${MIXXX_INSTALL_DOCDIR}"
  )
endif()

# Additional Linux-only files
if(UNIX AND NOT APPLE)
  # .desktop file for KDE/GNOME menu
  install(
    FILES
      "${CMAKE_CURRENT_SOURCE_DIR}/res/linux/mixxx.desktop"
    DESTINATION
      "${CMAKE_INSTALL_DATADIR}/applications"
  )

  # Icon files for menu entry
  install(
    FILES
      "${CMAKE_CURRENT_SOURCE_DIR}/res/images/icons/mixxx.svg"
    DESTINATION
      "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps"
  )
  install(
    FILES
      "${CMAKE_CURRENT_SOURCE_DIR}/res/images/icons/mixxx.png"
    DESTINATION
      "${CMAKE_INSTALL_DATADIR}/icons/hicolor/32x32/apps"
  )

  # .metainfo.xml file for KDE/GNOME AppStream initiative
  install(
    FILES
      "${CMAKE_CURRENT_SOURCE_DIR}/res/linux/mixxx.metainfo.xml"
    DESTINATION
      "${CMAKE_INSTALL_DATAROOTDIR}/metainfo"
  )

  option(INSTALL_USER_UDEV_RULES "Install user udev rule file for USB HID and Bulk controllers" ON)
  if (INSTALL_USER_UDEV_RULES)
    install(
      FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/res/linux/mixxx-usb-uaccess.rules"
      DESTINATION
        "${MIXXX_INSTALL_DATADIR}/udev/rules.d"
    )
    install(CODE "
    message(STATUS \"Important Note: Installation of udev rules\n\"
        \"The udev rule file for USB HID and Bulk controller permissions will be\n\"
        \"installed to:\n\"
        \"    ${CMAKE_INSTALL_PREFIX}/${MIXXX_INSTALL_DATADIR}/udev/rules.d.\n\"
        \"If you are installing Mixxx from source for your own use, copy\n\"
        \"mixxx-usb-uaccess.rules to /etc/udev/rules.d/ and run:\n\"
        \"    udevadm control --reload-rules && udevadm trigger\n\"
        \"as root to load the rules.\n\"
        \"If you are building a package for a distribution, the correct directory for\n\"
        \"system rules is either /lib/udev/rules.d (e.g. Debian, Fedora) or\n\"
        \"/usr/lib/udev/rules.d (e.g. Arch Linux) with an appropriate priority prefix.\n\"
        \"Adjust your package script accordingly and set -DINSTALL_USER_UDEV_RULES=OFF\")
    ")
  endif()
endif()

if(MSVC)
  # install debug symbols if any were generated
  install(
    FILES $<TARGET_PDB_FILE:mixxx>
    CONFIGURATIONS Debug RelWithDebInfo
    DESTINATION "${MIXXX_INSTALL_BINDIR}"
    COMPONENT PDB # No spaces allowed
  )
endif()

if(WIN32)
  # qt_de.qm is just one arbitrary file in the directory that needs to be located;
  # there is no particular reason to look for this file versus any other one in the directory.
  find_file(QT_TRANSLATION_FILE qt_de.qm PATH_SUFFIXES share/qt5/translations)
  get_filename_component(QT_TRANSLATIONS ${QT_TRANSLATION_FILE} DIRECTORY)
  install(
    DIRECTORY "${QT_TRANSLATIONS}"
    DESTINATION "${MIXXX_INSTALL_DATADIR}"
    # QT 5 translations have been separated into several files, and most of the qt_xx.qm files
    # contain just shortcuts to load the qtbase, qtmultimedia etc files.
    FILES_MATCHING REGEX
      "qt_.+\.qm|qtbase_.*\.qm|qtmultimedia_.*\.qm|qtscript_.*\.qm|qtxmlpatterns_.*\.qm"
  )

  # Qt 5 loads these ANGLE DLLs at runtime if the graphics driver is blocklisted.
  # Qt does not link these and vcpkg does not build them as a dependency of Qt,
  # so copy them manually.
  find_file(EGL_DLL libEGL.dll PATH_SUFFIXES ${CMAKE_INSTALL_BINDIR})
  find_file(GLESv2_DLL libGLESv2.dll PATH_SUFFIXES ${CMAKE_INSTALL_BINDIR})
  install(FILES ${EGL_DLL} ${GLESv2_DLL} DESTINATION "${MIXXX_INSTALL_BINDIR}")
endif()

#
# Tests
#

# CMake + googletest: https://crascit.com/2015/07/25/cmake-gtest/

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "Pass Mixxx compiler/linker options to GoogleTest" FORCE)

# Prevent installation of GoogleTest libraries
set(INSTALL_GTEST OFF CACHE BOOL "Disable installation of GoogleTest" FORCE)

# Add googletest directly to our build. This adds the following targets:
# gtest, gtest_main, gmock and gmock_main
add_subdirectory(
  "${CMAKE_CURRENT_SOURCE_DIR}/lib/googletest"
  "${CMAKE_CURRENT_BINARY_DIR}/lib/googletest"
)

add_executable(mixxx-test
  src/test/analyserwaveformtest.cpp
  src/test/analyzersilence_test.cpp
  src/test/audiotaperpot_test.cpp
  src/test/autodjprocessor_test.cpp
  src/test/baseeffecttest.cpp
  src/test/beatgridtest.cpp
  src/test/beatmaptest.cpp
  src/test/beatstranslatetest.cpp
  src/test/bpmcontrol_test.cpp
  src/test/broadcastprofile_test.cpp
  src/test/broadcastsettings_test.cpp
  src/test/cache_test.cpp
  src/test/channelhandle_test.cpp
  src/test/colorconfig_test.cpp
  src/test/colormapperjsproxy_test.cpp
  src/test/colorpalette_test.cpp
  src/test/compatibility_test.cpp
  src/test/configobject_test.cpp
  src/test/controller_mapping_validation_test.cpp
  src/test/controllerscriptenginelegacy_test.cpp
  src/test/controlobjecttest.cpp
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
  src/test/effectchainslottest.cpp
  src/test/effectslottest.cpp
  src/test/effectsmanagertest.cpp
  src/test/enginebufferscalelineartest.cpp
  src/test/enginebuffertest.cpp
  src/test/enginefilterbiquadtest.cpp
  src/test/enginemastertest.cpp
  src/test/enginemicrophonetest.cpp
  src/test/enginesynctest.cpp
  src/test/fileinfo_test.cpp
  src/test/globaltrackcache_test.cpp
  src/test/hotcuecontrol_test.cpp
  src/test/imageutils_test.cpp
  src/test/indexrange_test.cpp
  src/test/keyutilstest.cpp
  src/test/lcstest.cpp
  src/test/learningutilstest.cpp
  src/test/libraryscannertest.cpp
  src/test/librarytest.cpp
  src/test/looping_control_test.cpp
  src/test/main.cpp
  src/test/mathutiltest.cpp
  src/test/metadatatest.cpp
  src/test/metaknob_link_test.cpp
  src/test/midicontrollertest.cpp
  src/test/mixxxtest.cpp
  src/test/movinginterquartilemean_test.cpp
  src/test/nativeeffects_test.cpp
  src/test/performancetimer_test.cpp
  src/test/playcountertest.cpp
  src/test/playlisttest.cpp
  src/test/portmidicontroller_test.cpp
  src/test/portmidienumeratortest.cpp
  src/test/queryutiltest.cpp
  src/test/rangelist_test.cpp
  src/test/readaheadmanager_test.cpp
  src/test/replaygaintest.cpp
  src/test/rescalertest.cpp
  src/test/rgbcolor_test.cpp
  src/test/samplebuffertest.cpp
  src/test/sampleutiltest.cpp
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
  src/test/tableview_test.cpp
  src/test/taglibtest.cpp
  src/test/trackdao_test.cpp
  src/test/trackexport_test.cpp
  src/test/trackmetadata_test.cpp
  src/test/tracknumberstest.cpp
  src/test/trackreftest.cpp
  src/test/trackupdate_test.cpp
  src/test/wbatterytest.cpp
  src/test/wpushbutton_test.cpp
  src/test/wwidgetstack_test.cpp
)
set_target_properties(mixxx-test PROPERTIES AUTOMOC ON)
target_link_libraries(mixxx-test PRIVATE mixxx-lib mixxx-gitinfostore gtest gmock)

#
# Benchmark tests
#

# Disable testing of google/benchmark
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)

# Prevent installation of google/benchmark artifacts
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "Disable installation of google/benchmark" FORCE)

add_subdirectory(
  "${CMAKE_CURRENT_SOURCE_DIR}/lib/benchmark"
  "${CMAKE_CURRENT_BINARY_DIR}/lib/benchmark"
)
target_link_libraries(mixxx-test PRIVATE benchmark)

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
if (NOT WIN32)
  # Default to offscreen rendering during tests.
  # This is required if the build system like Fedora koji/mock does not
  # allow to pass environment variables into the ctest macro expansion.
  set_tests_properties(${testsuite} PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif()

# Benchmarking
add_custom_target(mixxx-benchmark
  COMMAND $<TARGET_FILE:mixxx-test> --benchmark
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  COMMENT "Mixxx Benchmarks"
  VERBATIM
)
add_dependencies(mixxx-benchmark mixxx-test)

if(UNIX AND USE_SYMLINKS)
  add_custom_target(mixxx-testdata
    COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_CURRENT_SOURCE_DIR}/src/test" "${CMAKE_CURRENT_BINARY_DIR}/src/test"
    COMMENT "Symlinking test data to build directory..."
  )
elseif(WIN32)
  file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src/test" CMAKE_CURRENT_SOURCE_TESTDATA_DIR_NATIVE)
  file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/src/test/" CMAKE_CURRENT_BINARY_TESTDATA_DIR_NATIVE)
  add_custom_target(mixxx-testdata
    COMMAND xcopy ${CMAKE_CURRENT_SOURCE_TESTDATA_DIR_NATIVE} ${CMAKE_CURRENT_BINARY_TESTDATA_DIR_NATIVE} /s /d /q /y
    COMMENT "Copying missing or modified test data files to build directory..."
  )
else()
  add_custom_target(mixxx-testdata
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/src/test" "${CMAKE_CURRENT_BINARY_DIR}/src/test"
    COMMENT "Copying all test data files to build directory..."
  )
endif()
add_dependencies(mixxx-test mixxx-testdata)

#
# Resources
#
# Add resources to mixxx and mixxx-test binaries, not the mixxx-lib static
# library. Doing this would require initialization using Q_INIT_RESOURCE()
# calls that are not present at the moment. Further information can be found
# at: https://doc.qt.io/qt5/resources.html#using-resources-in-a-library
option(DOWNLOAD_MANUAL "Download Manual PDF from Mixxx website" OFF)
if(DOWNLOAD_MANUAL AND NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/res/Mixxx-Manual.pdf")
  set(MANUAL_URL "https://downloads.mixxx.org/manual/${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}/mixxx-manual-${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}-en.pdf")
  message(STATUS "Downloading manual from ${MANUAL_URL}...")
  file(DOWNLOAD
    "${MANUAL_URL}"
    "${CMAKE_CURRENT_BINARY_DIR}/res/Mixxx-Manual.pdf"
    SHOW_PROGRESS
    STATUS MANUAL_PDF_DOWNLOAD
    TLS_VERIFY ON
  )
  if(NOT MANUAL_PDF_DOWNLOAD EQUAL 0)
    message(FATAL_ERROR "Manual PDF download failed. Either download it yourself "
      "and move it to '${CMAKE_CURRENT_SOURCE_DIR}/res/Mixxx-Manual.pdf' or "
      "reconfigure with -DDOWNLOAD_MANUAL=OFF to build without included "
      "manual.")
  endif()
  file(RENAME "${CMAKE_CURRENT_BINARY_DIR}/res/Mixxx-Manual.pdf" "${CMAKE_CURRENT_SOURCE_DIR}/res/Mixxx-Manual.pdf")
endif()

target_sources(mixxx PRIVATE res/mixxx.qrc)
set_target_properties(mixxx PROPERTIES AUTORCC ON)
target_sources(mixxx-test PRIVATE res/mixxx.qrc)
set_target_properties(mixxx-test PROPERTIES AUTORCC ON)

if (MIXXX_VERSION_PRERELEASE STREQUAL "")
   set(MIXXX_VERSION "${CMAKE_PROJECT_VERSION}")
else()
   set(MIXXX_VERSION "${CMAKE_PROJECT_VERSION}-${MIXXX_VERSION_PRERELEASE}")
endif()

get_target_property(MIXXX_BUILD_FLAGS mixxx-lib COMPILE_OPTIONS)

# uses CMAKE_PROJECT_VERSION MIXXX_VERSION_PRERELEASE MIXXX_BUILD_FLAGS
configure_file(src/version.h.in src/version.h @ONLY)

if(GIT_COMMIT_DATE AND NOT GIT_COMMIT_DATE MATCHES "^[0-9]*-[0-9]*-[0-9]*T[0-9]*\\:[0-9]*\\:[0-9]*[+-][0-9]*\\:[0-9]*$")
  message(FATAL_ERROR "GIT_COMMIT_DATE requires strict ISO 8601 format %Y-%m-%dT%H:%M:%SZ")
endif()

add_custom_target(mixxx-gitinfo
  COMMAND ${CMAKE_COMMAND}
    -DGIT_DESCRIBE="${GIT_DESCRIBE}"
    -DGIT_COMMIT_DATE="${GIT_COMMIT_DATE}"
    -DINPUT_FILE="${CMAKE_CURRENT_SOURCE_DIR}/src/gitinfo.h.in"
    -DOUTPUT_FILE="${CMAKE_CURRENT_BINARY_DIR}/src/gitinfo.h"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/scripts/gitinfo.cmake"
  COMMENT "Update git version information in gitinfo.h"
  BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/src/gitinfo.h"
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_library(mixxx-gitinfostore STATIC EXCLUDE_FROM_ALL
    src/util/gitinfostore.cpp
)
target_include_directories(mixxx-gitinfostore PUBLIC src ${CMAKE_BINARY_DIR}/src)
add_dependencies(mixxx-gitinfostore mixxx-gitinfo)

# Windows-only resource file
if(WIN32)
  string(TIMESTAMP MIXXX_YEAR "%Y")

  set(MIXXX_FILEVERSION "${CMAKE_PROJECT_VERSION_MAJOR},${CMAKE_PROJECT_VERSION_MINOR},${CMAKE_PROJECT_VERSION_PATCH}")
  set(MIXXX_PRODUCTVERSION "${MIXXX_FILEVERSION}")

  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(MIXXX_DEBUG 1)
  else()
    set(MIXXX_DEBUG 0)
  endif()

  if (MIXXX_VERSION_PRERELEASE STREQUAL "")
    set(MIXXX_PRERELEASE 0)
  else()
    set(MIXXX_PRERELEASE 1)
  endif()

  # uses MIXXX_YEAR MIXXX_FILEVERSION MIXXX_PRODUCTVERSION MIXXX_VERSION MIXXX_DEBUG MIXXX_PRERELEASE
  configure_file(
    "src/mixxx.rc.include.in"
    "src/mixxx.rc.include"
    @ONLY
  )
  add_dependencies(mixxx mixxx-gitinfo)

  target_sources(mixxx PRIVATE
    src/mixxx.rc
    "${CMAKE_CURRENT_BINARY_DIR}/src/mixxx.rc.include"
    "${CMAKE_CURRENT_BINARY_DIR}/src/gitinfo.h"
  )
  # sccache fails with RC files
  # https://github.com/mozilla/sccache/issues/947
  if(SCCACHE_SUPPORT)
    set_target_properties(mixxx PROPERTIES RULE_LAUNCH_COMPILE "")
  endif()
  target_include_directories(mixxx PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

#
# Dependencies
#
option(STATIC_DEPS "Link dependencies statically" OFF)

# Chromaprint
find_package(Chromaprint REQUIRED)
target_link_libraries(mixxx-lib PRIVATE Chromaprint::Chromaprint)
if(WIN32)
  if(STATIC_DEPS)
    target_compile_definitions(mixxx-lib PUBLIC CHROMAPRINT_NODLL)
  endif()
   # Chromaprint is always built statically and needs fftw.
  find_package(FFTW REQUIRED)
  target_link_libraries(mixxx-lib PRIVATE FFTW::FFTW)
endif()

# Denon Engine Prime library export support (using libdjinterop)
option(ENGINEPRIME "Support for library export to Denon Engine Prime" ON)
if(ENGINEPRIME)
  # Look for an existing installation of libdjinterop and use that if available.
  # Otherwise, download and build from GitHub.
  find_package(DjInterop)
  if(DjInterop_FOUND)
    # An existing installation of djinterop is available.
    message(STATUS "Using existing system installation of libdjinterop")
    target_include_directories(mixxx-lib PUBLIC ${DjInterop_INCLUDE_DIRS})
    target_link_libraries(mixxx-lib PRIVATE DjInterop::DjInterop)
  else()
    # Fetch djinterop sources from GitHub and build them statically.

    # On MacOS, Mixxx does not use system SQLite, so we will use libdjinterop's
    # embedded SQLite in such a case.
    if (APPLE)
      message(STATUS "Building libdjinterop sources (with embedded SQLite) fetched from GitHub")
      set(DJINTEROP_SYSTEM_SQLITE OFF)
    else()
      message(STATUS "Building libdjinterop sources (with system SQLite) fetched from GitHub")
      set(DJINTEROP_SYSTEM_SQLITE ON)
    endif()

    set(DJINTEROP_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/lib/libdjinterop-install")
    set(DJINTEROP_LIBRARY "lib/${CMAKE_STATIC_LIBRARY_PREFIX}djinterop${CMAKE_STATIC_LIBRARY_SUFFIX}")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/download")

    # CMake does not pass lists of paths properly to external projects.
    # This is worked around by changing the list separator.
    string(REPLACE ";" "|" PIPE_DELIMITED_CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}")

    # External project sources must be downloaded as an archive into DOWNLOAD_DIR
    # from an URL to keep offline builds like for Fedora/RPM Fusion working!
    ExternalProject_Add(libdjinterop
      URL "https://github.com/xsco/libdjinterop/archive/0.15.2.tar.gz"
      URL_HASH SHA256=66f64c0b38a53936ba1b471524905e35afd605bc05d29e7aa6fa7ea0005c7d54
      DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/download/libdjinterop"
      INSTALL_DIR ${DJINTEROP_INSTALL_DIR}
      LIST_SEPARATOR "|"
      CMAKE_ARGS
        -DBUILD_SHARED_LIBS=OFF
        -DCMAKE_SKIP_INSTALL_ALL_DEPENDENCY=ON
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
        -DCMAKE_PREFIX_PATH=${PIPE_DELIMITED_CMAKE_PREFIX_PATH}
        -DCMAKE_INSTALL_LIBDIR:PATH=lib
        -DCMAKE_MODULE_PATH:PATH=${CMAKE_MODULE_PATH}
        -DSYSTEM_SQLITE=${DJINTEROP_SYSTEM_SQLITE}
      BUILD_BYPRODUCTS <INSTALL_DIR>/${DJINTEROP_LIBRARY}
      EXCLUDE_FROM_ALL TRUE
    )

    # Assemble a library based on the external project.
    add_library(mixxx-libdjinterop STATIC IMPORTED)
    add_dependencies(mixxx-libdjinterop libdjinterop)
    set(DJINTEROP_INCLUDE_DIR "${DJINTEROP_INSTALL_DIR}/include")
    set(DJINTEROP_LIBRARY_PATH "${DJINTEROP_INSTALL_DIR}/${DJINTEROP_LIBRARY}")
    set_target_properties(mixxx-libdjinterop PROPERTIES IMPORTED_LOCATION "${DJINTEROP_LIBRARY_PATH}")
    target_include_directories(mixxx-lib PUBLIC ${DJINTEROP_INCLUDE_DIR})
    target_link_libraries(mixxx-lib PRIVATE mixxx-libdjinterop)

    # Since we have built libdjinterop from sources as a static library, its
    # transitive dependencies are not automatically recognised.  libdjinterop
    # depends on zlib and optionally sqlite3.  If libdjinterop was configured
    # to depend on system SQLite, Mixxx will already have the dependency.
    # But it does not have zlib, so we explicitly add that here.
    find_package(ZLIB 1.2.8 REQUIRED)
    target_link_libraries(mixxx-lib PRIVATE ${ZLIB_LIBRARIES})
  endif()

  # Include conditional sources only required with Engine Prime export support.
  target_sources(mixxx-lib PRIVATE
    src/library/export/dlglibraryexport.cpp
    src/library/export/engineprimeexportjob.cpp
    src/library/export/libraryexporter.cpp)
  target_compile_definitions(mixxx-lib PUBLIC __ENGINEPRIME__)
endif()

# Ebur128
find_package(Ebur128)
default_option(EBUR128_STATIC "Link libebur128 statically" "NOT Ebur128_FOUND")
if(EBUR128_STATIC)
  message(STATUS "Preparing internal Ebur128")
  set(EBUR128_CMAKE_ARGS "-DBUILD_STATIC_LIBS=ON")
  check_symbol_exists(STAILQ_HEAD sys/queue.h HAVE_STAILQ)
  if(NOT HAVE_STAILQ)
    list(APPEND EBUR128_CMAKE_ARGS "-DENABLE_INTERNAL_QUEUE_H=ON")
  endif()
  if(MSVC)
    set(EBUR128_LIBRARY "${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}ebur128_static${CMAKE_STATIC_LIBRARY_SUFFIX}")
  else()
    set(EBUR128_LIBRARY "${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}ebur128${CMAKE_STATIC_LIBRARY_SUFFIX}")
  endif()
  ExternalProject_Add(libebur128
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/libebur128"
    BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/lib/libebur128"
    INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/lib/libebur128-install"
    CMAKE_ARGS "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}" -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> ${EBUR128_CMAKE_ARGS}
    BUILD_BYPRODUCTS <INSTALL_DIR>/${EBUR128_LIBRARY}
  )
  set_target_properties(libebur128 PROPERTIES EXCLUDE_FROM_ALL TRUE)

  add_library(mixxx-libebur128 STATIC IMPORTED)
  add_dependencies(mixxx-libebur128 libebur128)
  set(EBUR128_LIBRARY_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/lib/libebur128-install/${EBUR128_LIBRARY}")
  message(STATUS "Linking internal libebur128 statically: ${EBUR128_LIBRARY_LOCATION}")
  set_target_properties(mixxx-libebur128 PROPERTIES
    IMPORTED_LOCATION "${EBUR128_LIBRARY_LOCATION}"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/lib/libebur128/ebur128"
  )
  target_link_libraries(mixxx-lib PRIVATE mixxx-libebur128)
else()
  message(STATUS "Linking libebur128 dynamically")
  target_link_libraries(mixxx-lib PRIVATE Ebur128::Ebur128)
endif()

# FidLib
add_library(fidlib STATIC EXCLUDE_FROM_ALL lib/fidlib/fidlib.c)
if(MSVC)
  target_compile_definitions(fidlib PRIVATE T_MSVC)
  target_compile_definitions(fidlib PRIVATE _USE_MATH_DEFINES)
elseif(MINGW)
  target_compile_definitions(fidlib PRIVATE T_MINGW)
else()
  target_compile_definitions(fidlib PRIVATE T_LINUX)
endif()
target_include_directories(mixxx-lib SYSTEM PUBLIC lib/fidlib)
target_link_libraries(mixxx-lib PRIVATE fidlib)

# KeyFinder
find_package(KeyFinder)
option(KEYFINDER "KeyFinder support" ON)
if(KEYFINDER)
  if (KeyFinder_FOUND)
    target_link_libraries(mixxx-lib PRIVATE KeyFinder::KeyFinder)
  else()
    # If KeyFinder is built statically, we need FFTW
    find_package(FFTW REQUIRED)
    set(KeyFinder_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/lib/keyfinder-install")
    set(KeyFinder_LIBRARY "${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}keyfinder${CMAKE_STATIC_LIBRARY_SUFFIX}")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/download")
    # External project sources must be downloaded as an archive into DOWNLOAD_DIR
    # from an URL to keep offline builds like for Fedora/RPM Fusion working!
    ExternalProject_Add(libkeyfinder
      URL "https://github.com/mixxxdj/libkeyfinder/archive/v2.2.4.zip"
      URL_HASH SHA256=cb3fea8c7213257281b6c7006b2430809f466ad8a1d485663324378d6fc0782c
      DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/download/libkeyfinder"
      INSTALL_DIR "${KeyFinder_INSTALL_DIR}"
      CMAKE_ARGS
        -DBUILD_SHARED_LIBS=OFF
        -DCMAKE_SKIP_INSTALL_ALL_DEPENDENCY=ON
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
        -DCMAKE_PREFIX_PATH:PATH="${CMAKE_PREFIX_PATH}"
        -DBUILD_TESTING=OFF
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

    add_library(mixxx-keyfinder STATIC IMPORTED)
    add_dependencies(mixxx-keyfinder libkeyfinder)
    set_target_properties(mixxx-keyfinder PROPERTIES IMPORTED_LOCATION "${KeyFinder_INSTALL_DIR}/${KeyFinder_LIBRARY}")
    target_link_libraries(mixxx-keyfinder INTERFACE FFTW::FFTW)
    target_include_directories(mixxx-keyfinder INTERFACE "${KeyFinder_INSTALL_DIR}/include")
    target_link_libraries(mixxx-lib PRIVATE mixxx-keyfinder)
  endif()

  target_sources(mixxx-lib PRIVATE src/analyzer/plugins/analyzerkeyfinder.cpp)
  target_compile_definitions(mixxx-lib PUBLIC __KEYFINDER__)
endif()

# FLAC
find_package(FLAC REQUIRED)
target_link_libraries(mixxx-lib PRIVATE FLAC::FLAC)
if(WIN32 AND STATIC_DEPS)
  target_compile_definitions(mixxx-lib PUBLIC FLAC__NO_DLL)
endif()

# FpClassify This is a wrapper around the fpclassify function that prevents
# inlining It is compiled without optimization and allows to use these function
# from -ffast-math optimized objects. The MSVC option /fp:fast does not suffer this issue
add_library(FpClassify STATIC EXCLUDE_FROM_ALL src/util/fpclassify.cpp)
if(GNU_GCC OR LLVM_CLANG)
  target_compile_options(FpClassify PRIVATE -fno-fast-math)
endif()
target_link_libraries(mixxx-lib PRIVATE FpClassify)

# googletest
# Required to use the macro FRIEND_TEST from <gtest/gtest_prod.h>
# in production code
target_include_directories(mixxx-lib SYSTEM PUBLIC "${gtest_SOURCE_DIR}/include")

# LAME
find_package(mp3lame REQUIRED)
target_link_libraries(mixxx-lib PRIVATE mp3lame::mp3lame)

# Kaitai for reading Rekordbox libraries
add_library(Kaitai STATIC EXCLUDE_FROM_ALL
  lib/kaitai/kaitaistream.cpp
)
target_include_directories(Kaitai SYSTEM PUBLIC lib/kaitai)
target_compile_definitions(Kaitai PRIVATE KS_STR_ENCODING_NONE)
target_link_libraries(mixxx-lib PRIVATE Kaitai)

# For determining MP3 timing offset cases in Rekordbox library feature
add_library(MP3GuessEnc STATIC EXCLUDE_FROM_ALL
  lib/mp3guessenc-0.27.4/mp3guessenc.c
  lib/mp3guessenc-0.27.4/tags.c
  lib/mp3guessenc-0.27.4/decode.c
  lib/mp3guessenc-0.27.4/bit_utils.c
)
if(WIN32)
  target_compile_definitions(MP3GuessEnc PRIVATE __WINDOWS__)
endif()
target_include_directories(MP3GuessEnc SYSTEM PUBLIC lib/mp3guessenc-0.27.4)
target_link_libraries(mixxx-lib PRIVATE MP3GuessEnc)

# OpenGL
set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)
target_link_libraries(mixxx-lib PRIVATE OpenGL::GL)

# Ogg
find_package(Ogg REQUIRED)
target_link_libraries(mixxx-lib PRIVATE Ogg::ogg)

# Vorbis
find_package(Vorbis REQUIRED)
target_link_libraries(mixxx-lib PRIVATE Vorbis::vorbis Vorbis::vorbisenc Vorbis::vorbisfile)

# PortAudio
find_package(PortAudio REQUIRED)
target_include_directories(mixxx-lib SYSTEM PUBLIC ${PortAudio_INCLUDE_DIRS})
target_link_libraries(mixxx-lib PRIVATE ${PortAudio_LIBRARIES})

# PortAudio Ring Buffer
add_library(PortAudioRingBuffer STATIC EXCLUDE_FROM_ALL
  lib/portaudio/pa_ringbuffer.c
)
target_include_directories(mixxx-lib SYSTEM PUBLIC lib/portaudio)
target_link_libraries(mixxx-lib PRIVATE PortAudioRingBuffer)

# PortMidi
find_package(PortMidi REQUIRED)
target_include_directories(mixxx-lib SYSTEM PUBLIC ${PortMidi_INCLUDE_DIRS})
target_link_libraries(mixxx-lib PRIVATE ${PortMidi_LIBRARIES})

# Protobuf
if(STATIC_DEPS)
  set(Protobuf_USE_STATIC_LIBS ON)
  mark_as_advanced(Protobuf_USE_STATIC_LIBS)
endif()
add_subdirectory(src/proto)
target_link_libraries(mixxx-lib PRIVATE mixxx-proto)

# Rigtorp SPSC Queue
# https://github.com/rigtorp/SPSCQueue
target_include_directories(mixxx-lib SYSTEM PUBLIC lib/rigtorp/SPSCQueue/include)

# Qt
find_package(Qt5
  COMPONENTS
    Concurrent
    Core
    Gui
    Network
    OpenGL
    Qml
    QuickWidgets
    Sql
    Svg
    Test
    Widgets
    Xml
  REQUIRED
)
target_link_libraries(mixxx-lib PUBLIC
  Qt5::Concurrent
  Qt5::Core
  Qt5::Gui
  Qt5::Network
  Qt5::OpenGL
  Qt5::Qml
  Qt5::QuickWidgets
  Qt5::Sql
  Qt5::Svg
  Qt5::Test
  Qt5::Widgets
  Qt5::Xml)
target_compile_definitions(mixxx-lib PUBLIC QT_TABLET_SUPPORT QT_USE_QSTRINGBUILDER)
if(UNIX AND NOT APPLE)
  find_package(X11 REQUIRED)
  find_package(Qt5 COMPONENTS X11Extras DBus REQUIRED)
  target_include_directories(mixxx-lib SYSTEM PUBLIC "${X11_INCLUDE_DIR}")
  target_link_libraries(mixxx-lib PRIVATE
    "${X11_LIBRARIES}"
    Qt5::X11Extras
    Qt5::DBus
  )
elseif(WIN32)
  get_target_property(QT5_TYPE Qt5::Core TYPE)
  if(QT5_TYPE STREQUAL "STATIC_LIBRARY")
    target_compile_definitions(mixxx-lib PUBLIC QT_NODLL)
    target_link_libraries(mixxx-lib PRIVATE
      # Pulled from qt-4.8.2-source\mkspecs\win32-msvc2010\qmake.conf
      # QtCore
      kernel32
      user32      # QtGui, QtOpenGL, libHSS1394
      shell32
      uuid
      ole32       # QtGui,
      advapi32    # QtGui, portaudio, portmidi
      ws2_32      # QtGui, QtNetwork, libshout
      # QtGui
      gdi32       # QtOpenGL, libshout
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

      # New libraries required by Qt5.
      dwmapi      # qtwindows
      iphlpapi    # qt5network
      mpr         # qt5core
      netapi32    # qt5core
      userenv     # qt5core
      uxtheme     # ?
      version     # ?
      wtsapi32    # ?

      # NOTE(rryan): If you are adding a plugin here, you must also
      # update src/mixxxapplication.cpp to define a Q_IMPORT_PLUGIN
      # for it. Not all imageformats plugins are built as .libs when
      # building Qt statically on Windows. Check the build environment
      # to see exactly what's available as a standalone .lib vs linked
      # into Qt .libs by default.

      # iconengines plugins
      Qt5::QSvgIconPlugin

      # imageformats plugins
      Qt5::QGifPlugin
      Qt5::QICOPlugin
      Qt5::QJpegPlugin
      Qt5::QSvgPlugin
      Qt5::QTgaPlugin

      # platform plugins (new in Qt5 for Windows)
      Qt5::QWindowsIntegrationPlugin

      # styles (new in Qt5 for Windows)
      Qt5::QWindowsVistaStylePlugin

      # sqldrivers (new in Qt5? or did we just start enabling them)
      Qt5::QSQLiteDriverPlugin
    )

    find_library(QT5FONTDATABASESUPPORT_LIBRARY Qt5FontDatabaseSupport)
    target_link_libraries(mixxx-lib PRIVATE "${QT5FONTDATABASESUPPORT_LIBRARY}")
    find_library(QT5WINDOWSUIAUTOMATIONSUPPORT_LIBRARY Qt5WindowsUIAutomationSupport)
    target_link_libraries(mixxx-lib PRIVATE "${QT5WINDOWSUIAUTOMATIONSUPPORT_LIBRARY}")
    find_library(QT5EVENTDISPATCHERSUPPORT_LIBRARY Qt5EventDispatcherSupport)
    target_link_libraries(mixxx-lib PRIVATE "${QT5EVENTDISPATCHERSUPPORT_LIBRARY}")
    find_library(QT5THEMESUPPORT_LIBRARY Qt5ThemeSupport)
    target_link_libraries(mixxx-lib PRIVATE "${QT5THEMESUPPORT_LIBRARY}")

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
    target_link_libraries(mixxx-lib PRIVATE
      ws2_32      # libshout
      gdi32       # libshout
    )
  endif()
endif()

# Queen Mary DSP
add_library(QueenMaryDsp STATIC EXCLUDE_FROM_ALL
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
  lib/qm-dsp/maths/KLDivergence.cpp lib/qm-dsp/maths/MathUtilities.cpp
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
target_include_directories(QueenMaryDsp SYSTEM PUBLIC lib/qm-dsp lib/qm-dsp/include)
target_link_libraries(mixxx-lib PRIVATE QueenMaryDsp)

# ReplayGain
add_library(ReplayGain STATIC EXCLUDE_FROM_ALL
  lib/replaygain/replaygain.cpp
)
target_include_directories(mixxx-lib SYSTEM PRIVATE lib/replaygain)
target_link_libraries(mixxx-lib PRIVATE ReplayGain)

# Reverb
add_library(Reverb STATIC EXCLUDE_FROM_ALL lib/reverb/Reverb.cc)
if(MSVC)
  target_compile_definitions(Reverb PRIVATE _USE_MATH_DEFINES)
endif()
target_include_directories(Reverb PRIVATE src)
target_link_libraries(Reverb PRIVATE Qt5::Core)
target_include_directories(mixxx-lib SYSTEM PRIVATE lib/reverb)
target_link_libraries(mixxx-lib PRIVATE Reverb)

# Rubberband
find_package(rubberband REQUIRED)
target_link_libraries(mixxx-lib PRIVATE rubberband::rubberband)

# SndFile
find_package(SndFile REQUIRED)
target_link_libraries(mixxx-lib PRIVATE SndFile::sndfile)
target_compile_definitions(mixxx-lib PUBLIC __SNDFILE__)
if(SndFile_SUPPORTS_SET_COMPRESSION_LEVEL)
  target_compile_definitions(mixxx-lib PUBLIC SFC_SUPPORTS_SET_COMPRESSION_LEVEL)
endif()
if(WIN32 AND STATIC_DEPS)
  find_package(G72X REQUIRED)
  target_link_libraries(mixxx-lib PRIVATE G72X::G72X)
endif()

# SoundTouch
find_package(SoundTouch)
default_option(SoundTouch_STATIC "Link libSoundTouch statically" "NOT SoundTouch_FOUND")
if(SoundTouch_STATIC)
  message(STATUS "Preparing internal libSoundTouch")
  add_library(SoundTouch STATIC EXCLUDE_FROM_ALL
    lib/soundtouch/AAFilter.cpp
    lib/soundtouch/BPMDetect.cpp
    lib/soundtouch/FIFOSampleBuffer.cpp
    lib/soundtouch/FIRFilter.cpp
    lib/soundtouch/InterpolateCubic.cpp
    lib/soundtouch/InterpolateLinear.cpp
    lib/soundtouch/InterpolateShannon.cpp
    lib/soundtouch/PeakFinder.cpp
    lib/soundtouch/RateTransposer.cpp
    lib/soundtouch/SoundTouch.cpp
    lib/soundtouch/TDStretch.cpp
    lib/soundtouch/cpu_detect_x86.cpp
    lib/soundtouch/mmx_optimized.cpp
    lib/soundtouch/sse_optimized.cpp
  )
  target_include_directories(SoundTouch SYSTEM PUBLIC lib)
  target_link_libraries(mixxx-lib PRIVATE SoundTouch)
else()
  message(STATUS "Linking libSoundTouch dynamically")
  target_link_libraries(mixxx-lib PRIVATE SoundTouch::SoundTouch)
endif()

# TagLib
find_package(TagLib REQUIRED)
target_link_libraries(mixxx-lib PRIVATE TagLib::TagLib)
if(WIN32 AND STATIC_DEPS)
  target_compile_definitions(mixxx-lib PUBLIC TAGLIB_STATIC)
endif()

# Threads
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(mixxx-lib PRIVATE Threads::Threads)

# Upower
if(UNIX AND NOT APPLE)
  find_package(GLIB COMPONENTS gobject REQUIRED)
  find_package(Upower REQUIRED)
  target_include_directories(mixxx-lib SYSTEM PUBLIC ${GLIB_INCLUDE_DIRS})
  target_link_libraries(mixxx-lib PRIVATE ${GLIB_LIBRARIES} ${GLIB_GOBJECT_LIBRARIES})
  target_link_libraries(mixxx-lib PRIVATE Upower::Upower)
endif()

# iOS/OS X Frameworks
if(APPLE)
  find_library(COREFOUNDATION_LIBRARY CoreFoundation REQUIRED)
  target_link_libraries(mixxx-lib PRIVATE ${COREFOUNDATION_LIBRARY})

  # The iOS/OS X security framework is used to implement sandboxing.
  find_library(SECURITY_LIBRARY Security REQUIRED)
  target_link_libraries(mixxx-lib PRIVATE ${SECURITY_LIBRARY})

  find_library(CORESERVICES_LIBRARY CoreServices REQUIRED)
  target_link_libraries(mixxx-lib PRIVATE ${CORESERVICES_LIBRARY})

  find_library(FOUNDATION_LIBRARY Foundation REQUIRED)
  target_link_libraries(mixxx-lib PRIVATE ${FOUNDATION_LIBRARY})

  # Used for battery measurements and controlling the screensaver on OS X and iOS.
  find_library(IOKIT_LIBRARY IOKit REQUIRED)
  target_link_libraries(mixxx-lib PRIVATE ${IOKIT_LIBRARY})
endif()

#
# Features
#

# Battery meter
#
# The battery meter is only available on Linux, macOS and Windows, therefore
# this option is forcibly set to OFF on all other platforms.
cmake_dependent_option(BATTERY "Battery meter support" ON "WIN32 OR UNIX" OFF)
if(BATTERY)
  if(WIN32)
    target_sources(mixxx-lib PRIVATE src/util/battery/batterywindows.cpp)
  elseif(APPLE)
    target_sources(mixxx-lib PRIVATE src/util/battery/batterymac.cpp)
  elseif(UNIX)
    target_sources(mixxx-lib PRIVATE src/util/battery/batterylinux.cpp)
  else()
    message(FATAL_ERROR "Battery support is not implemented for the target platform.")
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
    message(FATAL_ERROR "Color Diagnostics are only available when using Clang.")
  endif()
  target_compile_options(mixxx-lib PUBLIC -fcolor-diagnostics)
endif()

# Clang Sanitizers
set(CLANG_SANITIZERS "")
option(CLANG_ASAN "Clang Address Sanitizer" OFF)
if(CLANG_ASAN)
  list(APPEND CLANG_SANITIZERS "address")
endif()
option(CLANG_UBSAN "Clang Undefined Behaviour Sanitizer" OFF)
if(CLANG_UBSAN)
  list(APPEND CLANG_SANITIZERS "undefined")
endif()
option(CLANG_TSAN "Clang Thread Sanitizer" OFF)
if(CLANG_TSAN)
  list(APPEND CLANG_SANITIZERS "thread")
endif()
if(NOT CLANG_SANITIZERS STREQUAL "")
  if(NOT LLVM_CLANG)
    message(FATAL_ERROR "Clang Sanitizers are only available when using Clang.")
  endif()
  list(JOIN CLANG_SANITIZERS "," CLANG_SANITZERS_JOINED)
  target_compile_options(mixxx-lib PUBLIC -fsanitize=${CLANG_SANITZERS_JOINED})
  target_link_options(mixxx-lib PUBLIC -fsanitize=${CLANG_SANITZERS_JOINED})
endif()

# CoreAudio MP3/AAC Decoder
#
# The CoreAudio API is only available on macOS, therefore this option is
# forcibly set to OFF on all other platforms.
cmake_dependent_option(COREAUDIO "CoreAudio MP3/AAC Decoder" ON "APPLE" OFF)
if(COREAUDIO)
  target_sources(mixxx-lib PRIVATE
    src/sources/soundsourcecoreaudio.cpp
    src/sources/v1/legacyaudiosourceadapter.cpp
    lib/apple/CAStreamBasicDescription.cpp
  )
  find_library(AUDIOTOOLBOX_LIBRARY AudioToolbox REQUIRED)
  target_link_libraries(mixxx-lib PRIVATE ${AUDIOTOOLBOX_LIBRARY})
  target_compile_definitions(mixxx-lib PRIVATE __COREAUDIO__)
  target_include_directories(mixxx-lib SYSTEM PUBLIC lib/apple)
endif()

# FAAD AAC audio file decoder plugin
find_package(MP4)
find_package(MP4v2)
# It is enabled by default on Linux only, because other targets have other
# solutions. It requires MP4 or MP4v2.
default_option(FAAD "FAAD AAC audio file decoder support" "UNIX;NOT APPLE;(MP4_FOUND OR MP4v2_FOUND)")
if(FAAD)
  if(NOT MP4_FOUND AND NOT MP4v2_FOUND)
    message(FATAL_ERROR "FAAD AAC audio support requires libmp4 or libmp4v2 with development headers.")
  endif()
  target_sources(mixxx-lib PRIVATE
    src/sources/soundsourcem4a.cpp
    src/sources/libfaadloader.cpp
  )
  target_compile_definitions(mixxx-lib PRIVATE __FAAD__)
  if(MP4v2_FOUND)
    target_compile_definitions(mixxx-lib PRIVATE __MP4V2__)
    target_link_libraries(mixxx-lib PRIVATE MP4v2::MP4v2)
  else()
    target_link_libraries(mixxx-lib PRIVATE MP4::MP4)
  endif()
endif()

# FDK-AAC is loaded dynamically at runtime by EncoderFdkAac using QLibrary,
# so copy it into the Windows and macOS packages, but do not link to it.
if(APPLE AND MACOS_BUNDLE)
  find_library(FDK_AAC_LIBRARY fdk-aac.2)
  if(FDK_AAC_LIBRARY)
    message(STATUS "Found fdk-aac: ${FDK_AAC_LIBRARY}")
    install(FILES ${FDK_AAC_LIBRARY} DESTINATION ${MIXXX_INSTALL_PREFIX}/Contents/Frameworks)
  else()
    message(STATUS "Could NOT find libfdk-aac.2.dylib")
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

# FFmpeg 4.x support
# FFmpeg is multimedia library that can be found http://ffmpeg.org/
find_package(FFMPEG COMPONENTS libavcodec libavformat libavutil libswresample)
default_option(FFMPEG "FFmpeg 4.x support" "FFMPEG_FOUND")
if(FFMPEG)
  if(NOT FFMPEG_FOUND)
    message(FATAL_ERROR "FFMPEG was not found")
  endif()

  # Check minimum required versions
  # Windows: Version numbers are not available!?
  # macOS: Untested
  if(FFMPEG_libavcodec_VERSION AND FFMPEG_libavcodec_VERSION VERSION_LESS 58)
    message(FATAL_ERROR "FFmpeg support requires at least version 58 of libavcodec (found: ${FFMPEG_libavcodec_VERSION}).")
  endif()
  if(FFMPEG_libavformat_VERSION AND FFMPEG_libavformat_VERSION VERSION_LESS 58)
    message(FATAL_ERROR "FFmpeg support requires at least version 58 of libavformat (found: ${FFMPEG_libavformat_VERSION}).")
  endif()
  if(FFMPEG_libavutil_VERSION AND FFMPEG_libavutil_VERSION VERSION_LESS 56)
    message(FATAL_ERROR "FFmpeg support requires at least version 56 of libavutil (found: ${FFMPEG_libavutil_VERSION}).")
  endif()
  if(FFMPEG_libswresample_VERSION AND FFMPEG_libswresample_VERSION VERSION_LESS 3.1)
    message(FATAL_ERROR "FFmpeg support requires at least version 3.1 of libswresample (found: ${FFMPEG_libswresample_VERSION}).")
  endif()

  target_sources(mixxx-lib PRIVATE src/sources/soundsourceffmpeg.cpp)
  target_compile_definitions(mixxx-lib PUBLIC
    __FFMPEG__
    # Needed to build new FFmpeg
    __STDC_CONSTANT_MACROS
    __STDC_LIMIT_MACROS
    __STDC_FORMAT_MACROS
  )
  target_link_libraries(mixxx-lib PRIVATE "${FFMPEG_LIBRARIES}")
  target_include_directories(mixxx-lib PUBLIC "${FFMPEG_INCLUDE_DIRS}")
endif()

# Google PerfTools
option(GPERFTOOLS "Google PerfTools libtcmalloc linkage" OFF)
option(GPERFTOOLSPROFILER "Google PerfTools libprofiler linkage" OFF)
if(GPERFTOOLS OR GPERFTOOLSPROFILER)
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
cmake_dependent_option(HSS1394 "HSS1394 MIDI device support" "${HSS1394_FOUND}" "WIN32 OR APPLE" OFF)
if(HSS1394)
  target_sources(mixxx-lib PRIVATE
    src/controllers/midi/hss1394controller.cpp
    src/controllers/midi/hss1394enumerator.cpp
  )
  target_compile_definitions(mixxx-lib PUBLIC __HSS1394__)
  if(NOT HSS1394_FOUND)
    message(FATAL_ERROR "HSS1394 MIDI device support requires the libhss1394 and its development headers.")
  endif()
  target_link_libraries(mixxx-lib PRIVATE HSS1394::HSS1394)
endif()

# Lilv (LV2)
find_package(lilv)
default_option(LILV "Lilv (LV2) support" "lilv_FOUND")
if(LILV)
  if(NOT TARGET lilv::lilv)
    message(FATAL_ERROR "Lilv (LV2) support requires the liblilv-0 and its development headers.")
  endif()
  target_sources(mixxx-lib PRIVATE
    src/effects/lv2/lv2backend.cpp
    src/effects/lv2/lv2effectprocessor.cpp
    src/effects/lv2/lv2manifest.cpp
    src/preferences/dialog/dlgpreflv2.cpp
  )
  target_compile_definitions(mixxx-lib PUBLIC __LILV__)
  target_link_libraries(mixxx-lib PRIVATE lilv::lilv)
  target_link_libraries(mixxx-test PRIVATE lilv::lilv)
endif()

# Live Broadcasting (Shoutcast)
option(BROADCAST "Live Broadcasting (Shoutcast) support" ON)
if(BROADCAST)
  find_package(Shoutidjc)
  # Check if system lib is at least 2.4.6 and not suffering bugs
  # https://bugs.launchpad.net/mixxx/+bug/1833225
  # https://bugs.launchpad.net/mixxx/+bug/1913579
  if(Shoutidjc_FOUND AND Shoutidjc_VERSION VERSION_LESS 2.4.4)
      message(STATUS "Installed libshout-idjc version: ${Shoutidjc_VERSION} is suffering from bug lp1833225")
  elseif(Shoutidjc_FOUND AND Shoutidjc_VERSION VERSION_LESS 2.4.6)
      message(STATUS "Installed libshout version: ${Shout_VERSION} is suffering from bug lp1913579")
  endif()
  if(NOT Shoutidjc_FOUND OR Shoutidjc_VERSION VERSION_LESS 2.4.6)
    # Fall back to internal libraray in the lib tree
    message(STATUS "Using internal libshout-idjc")
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/lib/libshout-idjc")
    target_include_directories(mixxx-lib SYSTEM PUBLIC lib/libshout-idjc/include)
    target_link_libraries(mixxx-lib PRIVATE shout_mixxx)
  else()
    target_link_libraries(mixxx-lib PRIVATE Shoutidjc::Shoutidjc)
  endif()
  target_sources(mixxx-lib PRIVATE
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
endif()

# Locale Aware Compare for SQLite
find_package(SQLite3)
# FIXME: It is difficult to get qmake to link Qt to a custom built SQLite on
# macOS instead of the system SQLite, which results in a crash on startup when
# LOCALECOMPARE is enabled, therefore this option is forcibly set to OFF on
# macOS.
cmake_dependent_option(LOCALECOMPARE "Locale Aware Compare support for SQLite" "${SQLite3_FOUND}" "NOT APPLE" OFF)
if(LOCALECOMPARE)
  if(NOT SQLite3_FOUND)
    message(FATAL_ERROR "Locale Aware Compare for SQLite requires libsqlite and its development headers.")
  endif()
  target_compile_definitions(mixxx-lib PUBLIC __SQLITE3__)
  target_include_directories(mixxx-lib SYSTEM PRIVATE ${SQLite3_INCLUDE_DIRS})
  target_link_libraries(mixxx-lib PRIVATE ${SQLite3_LIBRARIES})
endif()

# Opus (RFC 6716)
find_package(Opus)
default_option(OPUS "Opus (RFC 6716) support" "Opus_FOUND")
if(OPUS)
  if(NOT Opus_FOUND)
    message(FATAL_ERROR "Opus support requires libopus and libopusfile with development headers.")
  endif()
  target_sources(mixxx-lib PRIVATE
    src/sources/soundsourceopus.cpp
    src/encoder/encoderopus.cpp
    src/encoder/encoderopussettings.cpp
  )
  target_compile_definitions(mixxx-lib PUBLIC __OPUS__)
  target_include_directories(mixxx-lib SYSTEM PUBLIC ${Opus_INCLUDE_DIRS})
  target_link_libraries(mixxx-lib PRIVATE ${Opus_LIBRARIES})
  if(WIN32 AND STATIC_DEPS)
    find_package(Celt)
    if(NOT Celt_FOUND)
      message(FATAL_ERROR "Opus support with static dependencies requires the celt library.")
    endif()
    target_link_libraries(mixxx-lib PRIVATE Celt::Celt)

    find_package(Silk)
    if(NOT Silk_FOUND)
      message(FATAL_ERROR "Opus support with static dependencies requires the silk library.")
    endif()
    target_link_libraries(mixxx-lib PRIVATE Silk::Float)
  endif()
endif()

# MAD MP3 Decoder
find_package(MAD)
find_package(ID3Tag)
default_option(MAD "MAD MP3 Decoder" "MAD_FOUND;ID3Tag_FOUND")
if(MAD)
  if(NOT MAD_FOUND)
    message(FATAL_ERROR "MAD support requires libmad and its development headers.")
  endif()
  if(NOT ID3Tag_FOUND)
    message(FATAL_ERROR "ID3Tag support requires libid3tag and its development headers.")
  endif()
  target_sources(mixxx-lib PRIVATE src/sources/soundsourcemp3.cpp)
  target_compile_definitions(mixxx-lib PUBLIC __MAD__)
  target_link_libraries(mixxx-lib PRIVATE MAD::MAD ID3Tag::ID3Tag)
endif()

# Media Foundation AAC Decoder Plugin
#
# The Media Foundtation API is only available on Windows, therefore this option
# is forcibly set to OFF on all other platforms.
cmake_dependent_option(MEDIAFOUNDATION "Media Foundation AAC decoder plugin" ON "WIN32" OFF)
if(MEDIAFOUNDATION)
  find_package(MediaFoundation REQUIRED)
  target_sources(mixxx-lib PRIVATE
    src/sources/soundsourcemediafoundation.cpp
  )
  target_compile_definitions(mixxx-lib PUBLIC __MEDIAFOUNDATION__)
  target_include_directories(mixxx-lib SYSTEM PRIVATE
    ${MediaFoundation_INCLUDE_DIRS}
  )
  target_link_libraries(mixxx-lib PRIVATE
    ${MediaFoundation_LIBRARIES}
  )
endif()

# Modplug support
find_package(Modplug)
default_option(MODPLUG "Modplug module decoder support" "Modplug_FOUND")
if(MODPLUG)
  if(NOT Modplug_FOUND)
    message(FATAL_ERROR "Modplug module decoder support requires libmodplug and its development headers.")
  endif()
  target_sources(mixxx-lib PRIVATE
    src/preferences/dialog/dlgprefmodplugdlg.ui
    src/sources/soundsourcemodplug.cpp
    src/preferences/dialog/dlgprefmodplug.cpp
  )
  target_compile_definitions(mixxx-lib PUBLIC __MODPLUG__)
  target_link_libraries(mixxx-lib PRIVATE Modplug::Modplug)
endif()

# QtKeychain
find_package(Qt5Keychain QUIET)
option(QTKEYCHAIN "Secure credentials storage support for Live Broadcasting profiles" ON)
if(QTKEYCHAIN)
  if(NOT Qt5Keychain_FOUND)
    message(FATAL_ERROR "Secure credential storage support requires the Qt5::Keychain component.")
  endif()
  target_compile_definitions(mixxx-lib PUBLIC __QTKEYCHAIN__)
  target_link_libraries(mixxx-lib PRIVATE ${QTKEYCHAIN_LIBRARIES})
  target_include_directories(mixxx-lib SYSTEM PUBLIC ${QTKEYCHAIN_INCLUDE_DIRS})
endif()

# USB HID or/and Bulk controller support
find_package(LibUSB)

# USB HID controller support
option(HID "USB HID controller support" ON)
if(HID)
  find_package(hidapi)
  # hidapi_VERSION is only available starting with 0.10.0
  if(NOT hidapi_FOUND OR NOT hidapi_VERSION OR hidapi_VERSION VERSION_LESS 0.10.1)
    message(STATUS "Linking internal libhidapi statically")
    add_library(mixxx-hidapi STATIC EXCLUDE_FROM_ALL)
    target_include_directories(mixxx-hidapi SYSTEM PUBLIC lib/hidapi/hidapi)
    if(WIN32)
        target_sources(mixxx-hidapi PRIVATE lib/hidapi/windows/hid.c)
        find_library(Setupapi_LIBRARY Setupapi REQUIRED)
        target_link_libraries(mixxx-hidapi PUBLIC ${Setupapi_LIBRARY})
    elseif(APPLE)
        target_sources(mixxx-hidapi PRIVATE lib/hidapi/mac/hid.c)
        find_library(AppKit_LIBRARY AppKit REQUIRED)
        target_link_libraries(mixxx-hidapi PUBLIC ${AppKit_LIBRARY})
    elseif(UNIX)
      if(NOT LibUSB_FOUND)
        message(FATAL_ERROR "USB HID controller support on Unix with statically linked libhidapi-libusb requires libusb 1.0 and its development headers.")
      endif()
      target_sources(mixxx-hidapi PRIVATE lib/hidapi/libusb/hid.c)
      target_link_libraries(mixxx-hidapi PRIVATE LibUSB::LibUSB)
    else()
      message(FATAL_ERROR "USB HID controller support only possible on Windows/Mac OS/Linux/BSD.")
    endif()
    target_link_libraries(mixxx-lib PRIVATE mixxx-hidapi)
  else()
    message(STATUS "Linking libhidapi dynamically")
    if(NOT HIDAPI_FOUND)
      message(FATAL_ERROR "USB HID controller support requires libhidapi-libusb and its development headers.")
    endif()
    target_link_libraries(mixxx-lib PRIVATE hidapi::hidapi)
  endif()
  target_sources(mixxx-lib PRIVATE
    src/controllers/hid/hidcontroller.cpp
    src/controllers/hid/hiddevice.cpp
    src/controllers/hid/hidenumerator.cpp
    src/controllers/hid/legacyhidcontrollermapping.cpp
    src/controllers/hid/legacyhidcontrollermappingfilehandler.cpp
  )
  target_compile_definitions(mixxx-lib PUBLIC __HID__)
endif()

# USB Bulk controller support
default_option(BULK "USB Bulk controller support" "LibUSB_FOUND")
if(BULK)
  if(NOT LibUSB_FOUND)
    message(FATAL_ERROR "USB Bulk controller support requires libusb 1.0 and its development headers.")
  endif()
  target_sources(mixxx-lib PRIVATE
    src/controllers/bulk/bulkcontroller.cpp
    src/controllers/bulk/bulkenumerator.cpp
  )
  if(NOT HID)
    target_sources(mixxx-lib PRIVATE
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
    message(FATAL_ERROR "Mac App Store and Vinyl Control support are mutually exclusive due to licensing issues.")
  endif()

  target_sources(mixxx-lib PRIVATE
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
  if(WIN32)
    target_sources(mixxx-xwax PRIVATE lib/xwax/timecoder_win32.cpp lib/xwax/lut_win32.cpp)
  else()
    target_sources(mixxx-xwax PRIVATE lib/xwax/timecoder.c lib/xwax/lut.c)
  endif()
  target_include_directories(mixxx-xwax SYSTEM PUBLIC lib/xwax)
  target_link_libraries(mixxx-lib PRIVATE mixxx-xwax)
endif()

# WavPack audio file support
find_package(wavpack)
default_option(WAVPACK "WavPack audio file support" "wavpack_FOUND")
if(WAVPACK)
  if(NOT wavpack_FOUND)
    message(FATAL_ERROR "WavPack audio file support requires libwv and its development headers.")
  endif()
  target_sources(mixxx-lib PRIVATE src/sources/soundsourcewv.cpp)
  target_compile_definitions(mixxx-lib PUBLIC __WV__)
  target_link_libraries(mixxx-lib PRIVATE WavPack::wavpack)
endif()

# Configure file with build options
file(RELATIVE_PATH MIXXX_INSTALL_DOCDIR_RELATIVE_TO_DATADIR "${CMAKE_INSTALL_PREFIX}/${MIXXX_INSTALL_DATADIR}" "${CMAKE_INSTALL_PREFIX}/${MIXXX_INSTALL_DOCDIR}")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/src/config.h" @ONLY)

# Packaging
set(CPACK_PACKAGE_VENDOR "Mixxx Project")
set(CPACK_PACKAGE_CONTACT "RJ Skerry-Ryan <rryan@mixxx.org>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Digital DJ Application")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/packaging/CPackPackageDescription.txt")
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

set(CPACK_SOURCE_IGNORE_FILES  "\\\\.#;/#;.*~;\\\\.o$")
list(APPEND CPACK_SOURCE_IGNORE_FILES "/\\\\.git/")
list(APPEND CPACK_SOURCE_IGNORE_FILES "/\\\\.github/")
list(APPEND CPACK_SOURCE_IGNORE_FILES "/cache/")
list(APPEND CPACK_SOURCE_IGNORE_FILES "/build/")
list(APPEND CPACK_SOURCE_IGNORE_FILES "/.*_build/")
list(APPEND CPACK_SOURCE_IGNORE_FILES "/\\\\.sconf_temp/")
list(APPEND CPACK_SOURCE_IGNORE_FILES "${CMAKE_CURRENT_BINARY_DIR}/")
set(CPACK_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set(CPACK_DEBIAN_PACKAGE_SECTION "sound")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_SUGGESTS "pdf-viewer, pulseaudio-utils")
set(CPACK_DEBIAN_PACKAGE_REPLACES "mixxx-data")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5sql5-sqlite, fonts-open-sans, fonts-ubuntu")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "${CPACK_PACKAGE_HOMEPAGE_URL}")
set(CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION TRUE)
file(READ ${CPACK_PACKAGE_DESCRIPTION_FILE} CPACK_DEBIAN_PACKAGE_DESCRIPTION)
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION_MERGED "${CPACK_DEBIAN_PACKAGE_DESCRIPTION}")
string(PREPEND CPACK_DEBIAN_PACKAGE_DESCRIPTION_MERGED "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}" "\n")
string(REPLACE "\n\n" "\n.\n" CPACK_DEBIAN_PACKAGE_DESCRIPTION_MERGED "${CPACK_DEBIAN_PACKAGE_DESCRIPTION_MERGED}")
string(REPLACE "\n" "\n " CPACK_DEBIAN_PACKAGE_DESCRIPTION_MERGED "${CPACK_DEBIAN_PACKAGE_DESCRIPTION_MERGED}")

# This is the version of the package itself and can be advanced or set to
# something like 0ubuntu1 when building a new package from the same version
if (NOT CPACK_DEBIAN_PACKAGE_RELEASE)
   set(CPACK_DEBIAN_PACKAGE_RELEASE 1)
endif()

set(CPACK_DEBIAN_DISTRIBUTION_RELEASES focal groovy hirsute impish)
set(CPACK_DEBIAN_SOURCE_DIR ${CMAKE_SOURCE_DIR})
set(CPACK_DEBIAN_UPLOAD_PPA_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/packaging/CPackDebUploadPPA.cmake")
set(CPACK_DEBIAN_INSTALL_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/packaging/CPackDebInstall.cmake")

set(CPACK_WIX_UPGRADE_GUID "921DC99C-4DCF-478D-B950-50685CB9E6BE")
set(CPACK_WIX_LICENSE_RTF "${CMAKE_CURRENT_SOURCE_DIR}/packaging/wix/LICENSE.rtf")
set(CPACK_WIX_PRODUCT_ICON "${CMAKE_SOURCE_DIR}/res/images/icons/ic_mixxx.ico")
set(CPACK_WIX_PROPERTY_ARPHELPLINK "${CPACK_PACKAGE_HOMEPAGE_URL}")
set(CPACK_WIX_UI_BANNER "${CMAKE_CURRENT_SOURCE_DIR}/packaging/wix/images/banner.bmp")
set(CPACK_WIX_UI_DIALOG "${CMAKE_CURRENT_SOURCE_DIR}/packaging/wix/images/dialog.bmp")

set(CPACK_PROJECT_CONFIG_FILE "${CMAKE_SOURCE_DIR}/packaging/CPackConfig.cmake" )

include(CPack)

if(APPLE AND MACOS_BUNDLE)
    macro(install_qt5_plugin _qt_plugin_name _qt_plugins_var _prefix)
        get_target_property(_qt_plugin_path "${_qt_plugin_name}" LOCATION)
        if(EXISTS "${_qt_plugin_path}")
            get_filename_component(_qt_plugin_file "${_qt_plugin_path}" NAME)
            get_filename_component(_qt_plugin_type "${_qt_plugin_path}" PATH)
            get_filename_component(_qt_plugin_type "${_qt_plugin_type}" NAME)
            set(_qt_plugin_dest "${_prefix}/Contents/PlugIns/${_qt_plugin_type}")
            install(FILES "${_qt_plugin_path}"
                DESTINATION "${_qt_plugin_dest}")
            set(${_qt_plugins_var}
                "${${_qt_plugins_var}};\$ENV{DEST_DIR}\${CMAKE_INSTALL_PREFIX}/${_qt_plugin_dest}/${_qt_plugin_file}")
        else()
            message(FATAL_ERROR "QT plugin ${_qt_plugin_name} not found")
        endif()
    endmacro()


    install_qt5_plugin(Qt5::QCocoaIntegrationPlugin BUNDLE_LIBS "${MIXXX_INSTALL_PREFIX}")
    install_qt5_plugin(Qt5::QSQLiteDriverPlugin BUNDLE_LIBS "${MIXXX_INSTALL_PREFIX}")
    install_qt5_plugin(Qt5::QMacStylePlugin BUNDLE_LIBS "${MIXXX_INSTALL_PREFIX}")
    install_qt5_plugin(Qt5::QSvgPlugin BUNDLE_LIBS "${MIXXX_INSTALL_PREFIX}")
    install_qt5_plugin(Qt5::QSvgIconPlugin BUNDLE_LIBS "${MIXXX_INSTALL_PREFIX}")
    install_qt5_plugin(Qt5::QJpegPlugin BUNDLE_LIBS "${MIXXX_INSTALL_PREFIX}")
    install_qt5_plugin(Qt5::QGifPlugin BUNDLE_LIBS "${MIXXX_INSTALL_PREFIX}")

    set(BUNDLE_NAME "${MIXXX_INSTALL_PREFIX}")
    set(BUNDLE_DIRS "${CMAKE_PREFIX_PATH}/lib;${Qt5Widgets_DIR}/../..")
    set(APPLE_CODESIGN_ENTITLEMENTS "${CMAKE_CURRENT_SOURCE_DIR}/packaging/macos/entitlements.plist")

    configure_file(cmake/modules/BundleInstall.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/BundleInstall.cmake" @ONLY)
    install(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/BundleInstall.cmake")
endif()
