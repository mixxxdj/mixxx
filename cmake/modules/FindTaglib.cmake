# - Try to find the Taglib library
# Once done this will define
#
#  TAGLIB_FOUND - system has the taglib library
#  TAGLIB_CFLAGS - the taglib cflags
#  TAGLIB_LIBRARIES - The libraries needed to use taglib

# Copyright (c) 2006, Laurent Montel, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT TAGLIB_MIN_VERSION)
    set(TAGLIB_MIN_VERSION "1.7")
endif()

if(NOT WIN32)
    find_program(TAGLIBCONFIG_EXECUTABLE
        NAMES taglib-config
        PATHS ${BIN_INSTALL_DIR}
    )
endif()

#reset vars
set(TAGLIB_LIBRARIES)
set(TAGLIB_CFLAGS)

# if taglib-config has been found
if(TAGLIBCONFIG_EXECUTABLE)

    exec_program(${TAGLIBCONFIG_EXECUTABLE} ARGS --version RETURN_VALUE _return_VALUE OUTPUT_VARIABLE TAGLIB_VERSION)

    if(TAGLIB_VERSION VERSION_LESS "${TAGLIB_MIN_VERSION}")
        message(STATUS "TagLib version too old: version searched :${TAGLIB_MIN_VERSION}, found ${TAGLIB_VERSION}")
        set(TAGLIB_FOUND FALSE)
    else()

        exec_program(${TAGLIBCONFIG_EXECUTABLE} ARGS --libs RETURN_VALUE _return_VALUE OUTPUT_VARIABLE TAGLIB_LIBRARIES)

        exec_program(${TAGLIBCONFIG_EXECUTABLE} ARGS --cflags RETURN_VALUE _return_VALUE OUTPUT_VARIABLE TAGLIB_CFLAGS)

    if(TAGLIB_LIBRARIES AND TAGLIB_CFLAGS)
        set(TAGLIB_FOUND TRUE)
    endif()

    string(REGEX REPLACE " *-I" ";" TAGLIB_INCLUDE_DIRS "${TAGLIB_CFLAGS}")
    string(SUBSTRING ${TAGLIB_INCLUDE_DIRS} 0 -1 TAGLIB_INCLUDE_DIRS) #we remove the initial ;
endif()

mark_as_advanced(TAGLIB_CFLAGS TAGLIB_LIBRARIES TAGLIB_INCLUDES)

else()

    find_path(TAGLIB_INCLUDE_DIRS
        NAMES tag.h
        PATH_SUFFIXES taglib
        PATHS ${INCLUDE_INSTALL_DIR}
    )

    if(NOT WIN32)
        # on non-win32 we don't need to take care about WIN32_DEBUG_POSTFIX
        find_library(TAGLIB_LIBRARIES tag PATHS ${LIB_INSTALL_DIR})

    else()

        # 1. get all possible libnames
        set(args PATHS ${LIB_INSTALL_DIR})
        set(newargs "")
        set(libnames_release "")
        set(libnames_debug "")

        list(LENGTH args listCount)

        # just one name
        list(APPEND libnames_release "tag")
        list(APPEND libnames_debug   "tagd")

        set(newargs ${args})

        # search the release lib
        find_library(TAGLIB_LIBRARIES_RELEASE
                     NAMES ${libnames_release}
                     ${newargs}
        )

        # search the debug lib
        find_library(TAGLIB_LIBRARIES_DEBUG
                     NAMES ${libnames_debug}
                     ${newargs}
        )

        if(TAGLIB_LIBRARIES_RELEASE AND TAGLIB_LIBRARIES_DEBUG)

            # both libs found
            set(TAGLIB_LIBRARIES optimized ${TAGLIB_LIBRARIES_RELEASE}
                                 debug     ${TAGLIB_LIBRARIES_DEBUG}
            )

        else()

            if(TAGLIB_LIBRARIES_RELEASE)
                # only release found
                set(TAGLIB_LIBRARIES ${TAGLIB_LIBRARIES_RELEASE})

            else()
                # only debug (or nothing) found
                set(TAGLIB_LIBRARIES ${TAGLIB_LIBRARIES_DEBUG})

            endif()

        endif()

        mark_as_advanced(TAGLIB_LIBRARIES_RELEASE)
        mark_as_advanced(TAGLIB_LIBRARIES_DEBUG)

    endif()

    include(FindPackageMessage)
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(Taglib DEFAULT_MSG TAGLIB_INCLUDE_DIRS_ TAGLIB_LIBRARIES)

endif()


if(TAGLIB_FOUND)
    if(NOT Taglib_FIND_QUIETLY AND TAGLIBCONFIG_EXECUTABLE)
        message(STATUS "Taglib found: ${TAGLIB_LIBRARIES}")
    endif()
else()
    if(Taglib_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find Taglib")
    endif()
endif()
