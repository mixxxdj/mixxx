# - Find MediaFoundation
# Find the Windows SDK MediaFoundation libraries
#
# MediaFoundation_LIBRARIES   - List of libraries when using MediaFoundation
# MediaFoundation_FOUND       - True if MediaFoundation found

IF (MSVC)
  SET( MediaFoundation_LIBRARIES mf.lib mfplat.lib mfreadwrite.lib mfuuid.lib strmiids.lib )
  SET( MediaFoundation_FOUND true )
ENDIF (MSVC)

IF (MediaFoundation_FOUND)
  IF (NOT MediaFoundation_FIND_QUIETLY)
    MESSAGE(STATUS "Found MediaFoundation: ${MediaFoundation_LIBRARIES}")
  ENDIF (NOT MediaFoundation_FIND_QUIETLY)
ELSE (MediaFoundation_FOUND)
  IF (MediaFoundation_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find MediaFoundation")
  ENDIF (MediaFoundation_FIND_REQUIRED)
ENDIF (MediaFoundation_FOUND)
