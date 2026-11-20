# Installs (or removes) a desktop entry and the hicolor icons for a Mixxx
# binary that is run from the build directory, so that desktop shells which
# derive the window icon from the desktop entry matching the Wayland app_id
# (e.g. GNOME) show the Mixxx icon instead of a generic one.
#
# Required variables:
#   ACTION       install | uninstall
#   DATA_HOME    target data directory, usually ~/.local/share
#   SOURCE_DIR   Mixxx source directory
# Required for ACTION=install:
#   MIXXX_BINARY absolute path of the mixxx executable

set(DESKTOP_FILE "${DATA_HOME}/applications/org.mixxx.Mixxx.desktop")
set(ICON_SOURCE_DIR "${SOURCE_DIR}/res/images/icons")
file(
  GLOB_RECURSE ICON_FILES
  RELATIVE "${ICON_SOURCE_DIR}"
  "${ICON_SOURCE_DIR}/*/apps/*"
)

if(ACTION STREQUAL "install")
  file(READ "${SOURCE_DIR}/res/linux/org.mixxx.Mixxx.desktop" DESKTOP_ENTRY)
  string(
    REGEX REPLACE
    "\nExec=[^\n]*"
    "\nExec=\"${MIXXX_BINARY}\""
    DESKTOP_ENTRY
    "${DESKTOP_ENTRY}"
  )
  string(
    REGEX REPLACE
    "\nName=Mixxx\n"
    "\nName=Mixxx (build directory)\n"
    DESKTOP_ENTRY
    "${DESKTOP_ENTRY}"
  )
  file(WRITE "${DESKTOP_FILE}" "${DESKTOP_ENTRY}")
  message(STATUS "Installed ${DESKTOP_FILE}")
  foreach(ICON_FILE ${ICON_FILES})
    get_filename_component(ICON_DIR "${ICON_FILE}" DIRECTORY)
    file(
      COPY "${ICON_SOURCE_DIR}/${ICON_FILE}"
      DESTINATION "${DATA_HOME}/icons/hicolor/${ICON_DIR}"
    )
  endforeach()
  message(STATUS "Installed Mixxx icons to ${DATA_HOME}/icons/hicolor")
elseif(ACTION STREQUAL "uninstall")
  set(REMOVE_FILES "${DESKTOP_FILE}")
  foreach(ICON_FILE ${ICON_FILES})
    list(APPEND REMOVE_FILES "${DATA_HOME}/icons/hicolor/${ICON_FILE}")
  endforeach()
  file(REMOVE ${REMOVE_FILES})
  message(STATUS "Removed ${DESKTOP_FILE} and Mixxx icons")
else()
  message(FATAL_ERROR "Unknown ACTION '${ACTION}'")
endif()
