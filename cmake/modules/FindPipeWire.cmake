#[=======================================================================[.rst:
FindPipeWire
--------

Finds the PipeWire library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``PipeWire::PipeWire``
  The PipeWire library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``PipeWire_FOUND``
  True if the system has the PipeWire library.

#]=======================================================================]

find_package(PkgConfig REQUIRED)
pkg_check_modules(PIPEWIRE REQUIRED IMPORTED_TARGET libpipewire-0.3)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  PipeWire
  REQUIRED_VARS PIPEWIRE_FOUND
  VERSION_VAR PIPEWIRE_VERSION
)

if(PipeWire_FOUND AND NOT TARGET PipeWire::PipeWire)
  add_library(PipeWire::PipeWire INTERFACE IMPORTED)
  set_target_properties(
    PipeWire::PipeWire
    PROPERTIES INTERFACE_LINK_LIBRARIES PkgConfig::PIPEWIRE
  )
endif()
