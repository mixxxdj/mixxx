if(NOT WIN32)
  return()
endif()

# Adapted from vcpkg's cppwinrt portfile
if(NOT CMAKE_WINDOWS_KITS_10_DIR)
    get_filename_component(CMAKE_WINDOWS_KITS_10_DIR "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows\\v10.0;InstallationFolder]" ABSOLUTE CACHE)
    if ("${CMAKE_WINDOWS_KITS_10_DIR}" STREQUAL "/registry")
      set(CMAKE_WINDOWS_KITS_10_DIR "C:/Program Files (x86)/Windows Kits/10")
    endif()
endif()

set(WINSDK_PATH "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]")
# List all the SDKs manually as CMAKE_VS_blabla is only defined for VS generators
cmake_host_system_information(
    RESULT WINSDK_PATH
    QUERY WINDOWS_REGISTRY "HKLM/SOFTWARE/Microsoft/Windows Kits/Installed Roots"
          VALUE KitsRoot10)

file(GLOB WINSDK_GLOB RELATIVE "${WINSDK_PATH}Include/" "${WINSDK_PATH}Include/*")
set(WINSDK_LATEST "0")
set(WINSDK_LIST)
foreach(dir ${WINSDK_GLOB})
  if("${dir}" VERSION_GREATER "${WINSDK_LATEST}")
    set(WINSDK_LATEST "${dir}")
  endif()
  list(PREPEND WINSDK_LIST "${CMAKE_WINDOWS_KITS_10_DIR}/Include/${dir}/cppwinrt")
endforeach()

if(NOT WINSDK_LATEST)
  set(WINSDK_LATEST 10.0.26100.0)
endif()

if(NOT CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
  set(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION ${WINSDK_LATEST})
endif()

find_path(CPPWINRT_PATH "winrt/base.h"
  PATHS
    "${WINSDK_PATH}"
  PATH_SUFFIXES
    "${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/cppwinrt"
    "Include/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/cppwinrt"
    ${WINSDK_LIST}
)

if(MSVC AND LIBREMIDI_DOWNLOAD_CPPWINRT)
  if (NOT EXISTS "${CMAKE_WINDOWS_KITS_10_DIR}/Lib/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}.")
    message(FATAL_ERROR "Windows SDK not found. Install a Windows SDK and pass it to CMake with e.g.  -DCMAKE_GENERATOR_PLATFORM=x64,version=10.0.26100.0 -DCMAKE_SYSTEM_VERSION=10.0.26100.0")
  endif()

  # Download archive
  if(NOT EXISTS "${CMAKE_BINARY_DIR}/cppwinrt.zip")
    file(DOWNLOAD
      "https://www.nuget.org/api/v2/package/Microsoft.Windows.CppWinRT"
      "${CMAKE_BINARY_DIR}/cppwinrt.zip"
    )
    file(ARCHIVE_EXTRACT
      INPUT "${CMAKE_BINARY_DIR}/cppwinrt.zip"
      DESTINATION "${CMAKE_BINARY_DIR}/cppwinrt-nuget/"
    )
  endif()

  set(CPPWINRT_TOOL "${CMAKE_BINARY_DIR}/cppwinrt-nuget/bin/cppwinrt.exe")
else()
  find_path(WINRT_HEADER_PATH "winrt/Windows.Devices.Midi.h")
  if(NOT WINRT_HEADER_PATH)
    message(FATAL_ERROR "WinRT headers not found. On MSYS2, install the cppwinrt package")
  else()
    message("WinRT headers found: ${WINRT_HEADER_PATH}")
  endif()

  # Will be in /usr/bin/ with MSYS2
  find_program(CPPWINRT_TOOL "cppwinrt")
endif()

file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/cppwinrt/")
if(CPPWINRT_TOOL)
  # Enumerate winmd IDL files and store them in a response file
  file(TO_CMAKE_PATH "${CMAKE_WINDOWS_KITS_10_DIR}/References/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}" winsdk)
  file(GLOB winmds "${winsdk}/*/*/*.winmd")

  set(args "")
  foreach(winmd IN LISTS winmds)
    string(APPEND args "-input \"${winmd}\"\n")
  endforeach()

  # Recreate the sources
  file(WRITE "${CMAKE_BINARY_DIR}/cppwinrt-src/cppwinrt.rsp" "${args}")

  # Process the Windows API IDL files into headers
  execute_process(
    COMMAND "${CPPWINRT_TOOL}"
      "@${CMAKE_BINARY_DIR}/cppwinrt-src/cppwinrt.rsp"
      -output "${CMAKE_BINARY_DIR}/cppwinrt"
  )
endif()

