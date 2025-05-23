# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

if( CMAKE_CONFIGURATION_TYPES ) # enable only debug/release configurations for generated VS project file
    set( CMAKE_CONFIGURATION_TYPES Debug Release )
    set( CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
         "Reset the configurations to what we need" FORCE )
endif()

if( NOT CMAKE_BUILD_TYPE ) # by default, use release build
    set( CMAKE_BUILD_TYPE "Release" )
endif()

option( BIT7Z_VS_LIBNAME_OUTDIR_STYLE
        "Force using Visual Studio output library name and directory structure convention" )

get_property( BIT7Z_GENERATOR_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG )

include( cmake/TargetArchDetect.cmake )

# architecture-specific options
if( BIT7Z_TARGET_ARCH_IS_64_BIT )
    if( WIN32 )
        add_definitions( -DWIN64 )
    endif()
    if( NOT BIT7Z_GENERATOR_IS_MULTI_CONFIG AND NOT BIT7Z_VS_LIBNAME_OUTDIR_STYLE )
        set( ARCH_POSTFIX 64 )
    endif()
endif()

# Note: 7-zip supports only x86, x64, arm, and arm64
set( ARCH_DIR ${BIT7Z_TARGET_ARCH_NAME} )

if( NOT BIT7Z_GENERATOR_IS_MULTI_CONFIG AND BIT7Z_VS_LIBNAME_OUTDIR_STYLE )
    # forcing output directory to ${BIT7Z_DIR}/lib/${ARCH_DIR}/${CMAKE_BUILD_TYPE} (e.g. ./lib/x64/Release)
    set( LIB_OUTPUT_DIR lib/${ARCH_DIR}/${CMAKE_BUILD_TYPE}/ )
    set( BIN_OUTPUT_DIR bin/${ARCH_DIR}/${CMAKE_BUILD_TYPE}/ )
else()
    # forcing output directory to ${BIT7Z_DIR}/lib/${ARCH_DIR}/ (e.g. ./lib/x64/)
    # Note: Visual Studio will append ${CMAKE_BUILD_TYPE} to ${OUTPUT_DIR}.
    set( LIB_OUTPUT_DIR lib/${ARCH_DIR}/ )
    set( BIN_OUTPUT_DIR bin/${ARCH_DIR}/ )
endif()

# Note: not applied in generated Visual Studio project files (e.g., .vcxproj)
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/${LIB_OUTPUT_DIR}" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/${LIB_OUTPUT_DIR}" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/${BIN_OUTPUT_DIR}" )

if( NOT BIT7Z_GENERATOR_IS_MULTI_CONFIG AND NOT BIT7Z_VS_LIBNAME_OUTDIR_STYLE )
    set( CMAKE_DEBUG_POSTFIX "_d" ) # debug library file name should end with "_d" (e.g. bit7z_d.lib)
endif()