# This file is part of Mixxx, Digital DJ'ing software.
# Copyright (C) 2001-2020 Mixxx Development Team
# Distributed under the GNU General Public Licence (GPL) version 2 or any later
# later version. See the LICENSE file for details.

#[=======================================================================[.rst:
WixMultiLanguagePackage
------------------------

This builds separate MSIs for each supported locale, then generates MST files
(transforms) of the diffs for each installer and finally injects the transforms
into the main installer.

See this `article`_ for details.

.. _article: http://www.codeproject.com/Articles/103749/Creating-a-Localized-Windows-Installer-Bootstrap

Input Variables
^^^^^^^^^^^^^^^

This will use the following variables as input:

``WIX_MULTILANGUAGE_INSTALLER_LOCALELCIDS``
  A list of installer locale/LCID entries to use. Each entry contains the
  locale and LCID, separated by a colon (e.g. ``en-us:1033``).
``WIX_MULTILANGUAGE_INSTALLER_MAINLOCALE``
  The locale of the main installer (e.g. ``en-us``).

Build Targets
^^^^^^^^^^^^^

This module provides the following build targets:

``wix-multilanguage-package``
  Build a multi-language MSI package using WIX.

#]=======================================================================]

# Find the Windows SDK directory.
find_package(WindowsSDK REQUIRED)
find_program(MSWSWDEVKIT_CSCRIPT "cscript")
if (NOT MSWSWDEVKIT_CSCRIPT)
  message(FATAL_ERROR "Microsoft Windows Scripting Host cscript.exe not found!")
endif()
file(GLOB WIX_PATH "C:/Program Files*/WiX Toolset v*/")
find_program(WIX_TORCH_EXECUTABLE "torch" PATHS "${WIX_PATH}" ENV WIX PATH_SUFFIXES bin DOC "WiX Toolset torch.exe location")
if (NOT WIX_TORCH_EXECUTABLE)
  message(FATAL_ERROR "WIX torch.exe not found!")
endif()
find_file(MSWSWDEVKIT_WISUBSTG "WiSubStg.vbs" PATHS "${WINDOWSSDK_LATEST_DIR}/Samples/sysmgmt/msi/scripts")
if (NOT MSWSWDEVKIT_WISUBSTG)
  message(FATAL_ERROR "Window SDK WiSubStg.vbs not found!")
endif()
find_file(MSWSWDEVKIT_WILANGID "WiLangId.vbs" PATHS "${WINDOWSSDK_LATEST_DIR}/Samples/sysmgmt/msi/scripts")
if (NOT MSWSWDEVKIT_WILANGID)
  message(FATAL_ERROR "Window SDK WiLangId.vbs not found!")
endif()
set(WIX_MULTILANGUAGE_INSTALLER_BATCHFILE "${PROJECT_BINARY_DIR}/WixMultiLanguagePackage.bat")

# Parse Locales
set(WIX_MULTILANGUAGE_INSTALLER_LOCALES "")
foreach(WIX_MULTILANGUAGE_INSTALLER_LOCALELCID ${WIX_MULTILANGUAGE_INSTALLER_LOCALELCIDS})
  if(NOT "${WIX_MULTILANGUAGE_INSTALLER_LOCALELCID}" MATCHES "^([a-zA-Z-]+):([0-9]+)$")
      message(FATAL_ERROR "Failed to parse locale/LCID \"${WIX_MULTILANGUAGE_INSTALLER_LOCALELCID}\"")
  endif()
  list(APPEND WIX_MULTILANGUAGE_INSTALLER_LOCALES "${CMAKE_MATCH_1}")
endforeach()

file(WRITE "${WIX_MULTILANGUAGE_INSTALLER_BATCHFILE}" "@echo on" \n)

# Build localized installers
foreach(WIX_MULTILANGUAGE_INSTALLER_LOCALE ${WIX_MULTILANGUAGE_INSTALLER_LOCALES})
  file(APPEND ${WIX_MULTILANGUAGE_INSTALLER_BATCHFILE} ":: Building the installer with ${WIX_MULTILANGUAGE_INSTALLER_LOCALE} locale" \n)
  file(APPEND ${WIX_MULTILANGUAGE_INSTALLER_BATCHFILE} "\"${CMAKE_CPACK_COMMAND}\" -G WIX --config ${CPACK_OUTPUT_CONFIG_FILE} -DCPACK_WIX_CULTURES:STRING=${WIX_MULTILANGUAGE_INSTALLER_LOCALE}" \n)
  file(APPEND ${WIX_MULTILANGUAGE_INSTALLER_BATCHFILE} "if %ERRORLEVEL% neq 0 goto exit" \n)
  file(APPEND ${WIX_MULTILANGUAGE_INSTALLER_BATCHFILE} "del ${WIX_MULTILANGUAGE_INSTALLER_FILENAME}-${WIX_MULTILANGUAGE_INSTALLER_LOCALE}.msi" \n)
  file(APPEND ${WIX_MULTILANGUAGE_INSTALLER_BATCHFILE} "ren ${WIX_MULTILANGUAGE_INSTALLER_FILENAME}.msi ${WIX_MULTILANGUAGE_INSTALLER_FILENAME}-${WIX_MULTILANGUAGE_INSTALLER_LOCALE}.msi" \n)
  file(APPEND ${WIX_MULTILANGUAGE_INSTALLER_BATCHFILE} "if %ERRORLEVEL% neq 0 goto exit" \n)
endforeach()
file(APPEND ${WIX_MULTILANGUAGE_INSTALLER_BATCHFILE} ":: Renaming main installer (${WIX_MULTILANGUAGE_INSTALLER_MAINLOCALE})" \n)
file(APPEND ${WIX_MULTILANGUAGE_INSTALLER_BATCHFILE} "ren ${WIX_MULTILANGUAGE_INSTALLER_FILENAME}-${WIX_MULTILANGUAGE_INSTALLER_MAINLOCALE}.msi ${WIX_MULTILANGUAGE_INSTALLER_FILENAME}.msi" \n)
file(APPEND ${WIX_MULTILANGUAGE_INSTALLER_BATCHFILE} "if %ERRORLEVEL% neq 0 goto exit" \n)

# Make diffs and generate the transforms
file(APPEND ${WIX_MULTILANGUAGE_INSTALLER_BATCHFILE} ":: Generating the language transforms from the diffs" \n)
foreach(WIX_MULTILANGUAGE_INSTALLER_LOCALE ${WIX_MULTILANGUAGE_INSTALLER_LOCALES})
  if("${WIX_MULTILANGUAGE_INSTALLER_LOCALE}" STREQUAL "${WIX_MULTILANGUAGE_INSTALLER_MAINLOCALE}")
    continue()
  endif()
  file(APPEND ${WIX_MULTILANGUAGE_INSTALLER_BATCHFILE} "\"${WIX_TORCH_EXECUTABLE}\" ${WIX_MULTILANGUAGE_INSTALLER_FILENAME}.msi ${WIX_MULTILANGUAGE_INSTALLER_FILENAME}-${WIX_MULTILANGUAGE_INSTALLER_LOCALE}.msi -o ${WIX_MULTILANGUAGE_INSTALLER_FILENAME}-${WIX_MULTILANGUAGE_INSTALLER_LOCALE}.mst" \n)
  file(APPEND ${WIX_MULTILANGUAGE_INSTALLER_BATCHFILE} "if %ERRORLEVEL% neq 0 goto exit" \n)
endforeach()

# Inject localization information into the main installer
file(APPEND ${WIX_MULTILANGUAGE_INSTALLER_BATCHFILE} ":: Injecting localization information into the main installer" \n)
set(INSTALLER_LCIDS "")
foreach(INSTALLER_LOCALELCID ${WIX_MULTILANGUAGE_INSTALLER_LOCALELCIDS})
  if(NOT "${INSTALLER_LOCALELCID}" MATCHES "^([a-zA-Z-]+):([0-9]+)$")
    message(FATAL_ERROR "Failed to parse locale/LCID \"${WIX_MULTILANGUAGE_INSTALLER_LOCALELCID}\"")
  endif()
  set(WIX_MULTILANGUAGE_INSTALLER_LOCALE "${CMAKE_MATCH_1}")
  set(WIX_MULTILANGUAGE_INSTALLER_LCID "${CMAKE_MATCH_2}")
  list(APPEND INSTALLER_LCIDS "${WIX_MULTILANGUAGE_INSTALLER_LCID}")
  if("${WIX_MULTILANGUAGE_INSTALLER_LOCALE}" STREQUAL "${WIX_MULTILANGUAGE_INSTALLER_MAINLOCALE}")
    continue()
  endif()
  file(APPEND ${WIX_MULTILANGUAGE_INSTALLER_BATCHFILE} "\"${MSWSWDEVKIT_CSCRIPT}\" \"${MSWSWDEVKIT_WISUBSTG}\" ${WIX_MULTILANGUAGE_INSTALLER_FILENAME}.msi ${WIX_MULTILANGUAGE_INSTALLER_FILENAME}-${WIX_MULTILANGUAGE_INSTALLER_LOCALE}.mst ${WIX_MULTILANGUAGE_INSTALLER_LCID}" \n)
  file(APPEND ${WIX_MULTILANGUAGE_INSTALLER_BATCHFILE} "if %ERRORLEVEL% neq 0 goto exit" \n)
  file(APPEND ${WIX_MULTILANGUAGE_INSTALLER_BATCHFILE} "del ${WIX_MULTILANGUAGE_INSTALLER_FILENAME}-${WIX_MULTILANGUAGE_INSTALLER_LOCALE}.msi" \n)
  file(APPEND ${WIX_MULTILANGUAGE_INSTALLER_BATCHFILE} "if %ERRORLEVEL% neq 0 goto exit" \n)
endforeach()

# Update supported Locale IDs (LCIDs) of the installer
file(APPEND ${WIX_MULTILANGUAGE_INSTALLER_BATCHFILE} ":: Updating supported locale IDs (LCIDs) of the installer" \n)
list(JOIN INSTALLER_LCIDS "," INSTALLER_LCIDS_STR)
file(APPEND ${WIX_MULTILANGUAGE_INSTALLER_BATCHFILE} "\"${MSWSWDEVKIT_CSCRIPT}\" \"${MSWSWDEVKIT_WILANGID}\" ${WIX_MULTILANGUAGE_INSTALLER_FILENAME}.msi Package ${INSTALLER_LCIDS_STR}" \n)
file(APPEND ${WIX_MULTILANGUAGE_INSTALLER_BATCHFILE} "if %ERRORLEVEL% neq 0 goto exit" \n)
file(APPEND ${WIX_MULTILANGUAGE_INSTALLER_BATCHFILE} ":exit" \n)

add_custom_target(wix-multilanguage-package
  COMMAND "${WIX_MULTILANGUAGE_INSTALLER_BATCHFILE}"
  WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
  COMMENT "Building multi-language installer using WIX"
)
