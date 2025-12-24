# - Find MediaFoundation
# Find the Windows SDK MediaFoundation libraries
#
# MediaFoundation_LIBRARIES   - List of libraries when using MediaFoundation
# MediaFoundation_FOUND       - True if MediaFoundation found
#
# lint_cmake: -package/consistency

if(MSVC)
  set(
    MediaFoundation_LIBRARIES
    mf.lib
    mfplat.lib
    mfreadwrite.lib
    mfuuid.lib
    strmiids.lib
  )
  set(MediaFoundation_FOUND true)
endif()

if(MediaFoundation_FOUND)
  if(NOT MediaFoundation_FIND_QUIETLY)
    message(STATUS "Found MediaFoundation: ${MediaFoundation_LIBRARIES}")
  endif()
else()
  if(MediaFoundation_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find MediaFoundation")
  endif()
endif()
