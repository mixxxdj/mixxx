# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set( BIT7Z_TARGET_ARCH_IS_64_BIT ON )
else()
    set( BIT7Z_TARGET_ARCH_IS_64_BIT OFF )
endif()

string( CONFIGURE [[
        #if defined( __arm__ ) || defined( _M_ARM )
        #   error CMAKE_TARGET_ARCH_arm
        #elif defined( __aarch64__ ) || defined( _M_ARM64 ) || defined(_M_ARM64EC)
        #   error CMAKE_TARGET_ARCH_arm64
        #elif defined( __i386__ ) || defined( _M_IX86 )
        #   error CMAKE_TARGET_ARCH_x86
        #elif defined( __x86_64__ ) || defined( _M_X64 )
        #   error CMAKE_TARGET_ARCH_x64
        #else
        #   error CMAKE_TARGET_ARCH_unknown
        #endif
]] architecture_check_code @ONLY )

file( WRITE "${CMAKE_BINARY_DIR}/arch_check.cpp" "${architecture_check_code}" )
try_run(
        run_result_unused
        compile_result_unused
        "${CMAKE_BINARY_DIR}"
        "${CMAKE_BINARY_DIR}/arch_check.cpp"
        COMPILE_OUTPUT_VARIABLE DETECTED_ARCH
)
string( REGEX MATCH "CMAKE_TARGET_ARCH_([a-zA-Z0-9_]+)" DETECTED_ARCH "${DETECTED_ARCH}" )
string( REPLACE "CMAKE_TARGET_ARCH_" "" DETECTED_ARCH "${DETECTED_ARCH}" )
set( BIT7Z_TARGET_ARCH_NAME "${DETECTED_ARCH}" )

if( "${BIT7Z_TARGET_ARCH_NAME}" STREQUAL "" )
    # If for some reason architecture detection failed, revert to using CMAKE_SYSTEM_PROCESSOR (not reliable on Windows)
    if ( CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)" OR CMAKE_GENERATOR_PLATFORM MATCHES "ARM64" )
        set( BIT7Z_TARGET_ARCH_NAME "arm64" )
    elseif( CMAKE_SYSTEM_PROCESSOR MATCHES "^arm" OR CMAKE_GENERATOR_PLATFORM MATCHES "ARM" )
        set( BIT7Z_TARGET_ARCH_NAME "arm" )
    elseif( BIT7Z_TARGET_ARCH_IS_64_BIT )
        set( BIT7Z_TARGET_ARCH_NAME "x64" )
    else()
        set( BIT7Z_TARGET_ARCH_NAME "x86" )
    endif()
endif()