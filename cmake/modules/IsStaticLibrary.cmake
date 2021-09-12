#[=======================================================================[.rst:
IsStaticLibrary
-------------

Macros to set a given variable to true or false whether the library is static

Usage:

.. code-block:: cmake

  is_static_library(<VAR> target)

Where ``<VAR>`` is set to TRUE if the target is a static library

Example invocation:

.. code-block:: cmake

  is_static_library(LIB_IS_STATIC Lib::Lib)

#]=======================================================================]

macro(IS_STATIC_LIBRARY var target)
  get_target_property(_target_type ${target} TYPE)
  if(${_target_type} STREQUAL "STATIC_LIBRARY")
    set(${var} TRUE)
  elseif(${_target_type} STREQUAL "UNKNOWN_LIBRARY")
    get_target_property(_target_location ${target} LOCATION)
    get_filename_component(_target_extension ${_target_location} EXT)
    if(${_target_extension} STREQUAL ${CMAKE_STATIC_LIBRARY_SUFFIX})
      set(${var} TRUE)
    else()
      set(${var} FALSE)
    endif()
    unset(_target_location)
    unset(_target_extension)
  else()
    set(${var} FALSE)
  endif()
  unset(_target_type)
endmacro()
