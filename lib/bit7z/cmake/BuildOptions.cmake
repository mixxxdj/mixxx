# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

# bit7z build options

option( BIT7Z_AUTO_FORMAT "Enable or disable auto format detection" )
message( STATUS "Auto format detection: ${BIT7Z_AUTO_FORMAT}" )
if( BIT7Z_AUTO_FORMAT )
    target_compile_definitions( ${LIB_TARGET} PUBLIC BIT7Z_AUTO_FORMAT )

    option( BIT7Z_DETECT_FROM_EXTENSION "Enable or disable detection of archive format from file extension" ON )
    message( STATUS "Detect format from file extension: ${BIT7Z_DETECT_FROM_EXTENSION}" )
    if( BIT7Z_DETECT_FROM_EXTENSION )
        target_compile_definitions( ${LIB_TARGET} PUBLIC BIT7Z_DETECT_FROM_EXTENSION )
    endif()
endif()

option( BIT7Z_REGEX_MATCHING "Enable or disable regex matching of archived files" )
message( STATUS "Regex matching extraction: ${BIT7Z_REGEX_MATCHING}" )
if( BIT7Z_REGEX_MATCHING )
    target_compile_definitions( ${LIB_TARGET} PUBLIC BIT7Z_REGEX_MATCHING )
endif()

option( BIT7Z_USE_STD_BYTE "Enable or disable using type safe byte type (like std::byte) for buffers" )
message( STATUS "Use std::byte: ${BIT7Z_USE_STD_BYTE}" )
if( BIT7Z_USE_STD_BYTE )
    target_compile_definitions( ${LIB_TARGET} PUBLIC BIT7Z_USE_STD_BYTE )
endif()

option( BIT7Z_USE_NATIVE_STRING "Enable or disable using the OS native string type
                                 (e.g., std::wstring on Windows, std::string elsewhere)" )
message( STATUS "Use native string: ${BIT7Z_USE_NATIVE_STRING}" )
if( BIT7Z_USE_NATIVE_STRING )
    target_compile_definitions( ${LIB_TARGET} PUBLIC BIT7Z_USE_NATIVE_STRING )
endif()

option( BIT7Z_GENERATE_PIC "Enable or disable generating Position Independent Code" )
message( STATUS "Generate Position Independent Code: ${BIT7Z_GENERATE_PIC}" )
if( BIT7Z_GENERATE_PIC )
    set_property( TARGET ${TARGET_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON )
endif()

option( BIT7Z_DISABLE_ZIP_ASCII_PWD_CHECK "Disable checking if password is ASCII when compressing using Zip format" )
message( STATUS "Disable Zip ASCII password check: ${BIT7Z_DISABLE_ZIP_ASCII_PWD_CHECK}" )
if( BIT7Z_DISABLE_ZIP_ASCII_PWD_CHECK )
    target_compile_definitions( ${LIB_TARGET} PRIVATE BIT7Z_DISABLE_ZIP_ASCII_PWD_CHECK )
endif()

option( BIT7Z_DISABLE_USE_STD_FILESYSTEM "Disable using the standard filesystem library (always use ghc::filesystem)" )
message( STATUS "Disable using std::filesystem: ${BIT7Z_DISABLE_USE_STD_FILESYSTEM}" )
if( BIT7Z_DISABLE_USE_STD_FILESYSTEM )
    target_compile_definitions( ${LIB_TARGET} PUBLIC BIT7Z_DISABLE_USE_STD_FILESYSTEM )
endif()

set( BIT7Z_CUSTOM_7ZIP_PATH "" CACHE STRING "A custom path to the 7-zip source code" )
if( NOT BIT7Z_CUSTOM_7ZIP_PATH STREQUAL "" )
    if( NOT EXISTS ${BIT7Z_CUSTOM_7ZIP_PATH}/CPP AND NOT EXISTS ${BIT7Z_CUSTOM_7ZIP_PATH}/DOC/readme.txt )
        message( FATAL_ERROR "Invalid or not existing custom path to 7-zip" )
    else()
        message( STATUS "7-zip custom path: ${BIT7Z_CUSTOM_7ZIP_PATH}" )
        add_library( 7-zip INTERFACE IMPORTED )
        target_include_directories( 7-zip INTERFACE "${BIT7Z_CUSTOM_7ZIP_PATH}/CPP/" )

        file( READ ${BIT7Z_CUSTOM_7ZIP_PATH}/DOC/readme.txt 7ZIP_README )
        if ( "${7ZIP_README}" MATCHES "^7-Zip ([0-9.]+)" )
            set( BIT7Z_7ZIP_VERSION "${CMAKE_MATCH_1}" )
            message( STATUS "Detected 7-zip version: ${BIT7Z_7ZIP_VERSION}" )
        endif()
    endif()
else()
    set( BIT7Z_7ZIP_VERSION "23.01" CACHE STRING "The version of 7-zip to be used by bit7z" )
    message( STATUS "7-zip version: ${BIT7Z_7ZIP_VERSION}" )
endif()

option( BIT7Z_BUILD_TESTS "Enable or disable building the testing executable" )
message( STATUS "Build tests: ${BIT7Z_BUILD_TESTS}" )

option( BIT7Z_BUILD_DOCS "Enable or disable building the documentation" )
message( STATUS "Build docs: ${BIT7Z_BUILD_DOCS}" )

if( CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
    option( BIT7Z_LINK_LIBCPP "Enable or disable linking to libc++" )
    message( STATUS "Link to libc++: ${BIT7Z_LINK_LIBCPP}" )
    if( BIT7Z_LINK_LIBCPP )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++" )
        set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi" )
    endif()
endif()

if( WIN32 )
    option( BIT7Z_AUTO_PREFIX_LONG_PATHS "Enable or disable automatically prepend paths with Windows long path prefix" )
    message( STATUS "Auto prefix long paths: ${BIT7Z_AUTO_PREFIX_LONG_PATHS}" )
    if( BIT7Z_AUTO_PREFIX_LONG_PATHS )
        target_compile_definitions( ${LIB_TARGET} PUBLIC BIT7Z_AUTO_PREFIX_LONG_PATHS )
    endif()

    option( BIT7Z_USE_SYSTEM_CODEPAGE "Enable or disable using the default Windows codepage for string conversions" )
    message( STATUS "Use the default codepage: ${BIT7Z_USE_SYSTEM_CODEPAGE}")
    if( BIT7Z_USE_SYSTEM_CODEPAGE )
        target_compile_definitions( ${LIB_TARGET} PUBLIC BIT7Z_USE_SYSTEM_CODEPAGE )
    endif()

    option( BIT7Z_PATH_SANITIZATION "Enable or disable path sanitization when extracting archives \
containing files with invalid Windows names" )
    message( STATUS "Path sanitization: ${BIT7Z_PATH_SANITIZATION}" )
    if( BIT7Z_PATH_SANITIZATION )
        target_compile_definitions( ${LIB_TARGET} PUBLIC BIT7Z_PATH_SANITIZATION )
    endif()
else()
    if( BIT7Z_7ZIP_VERSION VERSION_LESS "23.01" )
        set( BIT7Z_USE_LEGACY_IUNKNOWN ON )
    else()
        option( BIT7Z_USE_LEGACY_IUNKNOWN "Enable or disable building using the legacy version of IUnknown" )
    endif()

    message( STATUS "Use legacy IUnknown: ${BIT7Z_USE_LEGACY_IUNKNOWN}" )
    if( BIT7Z_USE_LEGACY_IUNKNOWN )
        target_compile_definitions( ${LIB_TARGET} PUBLIC BIT7Z_USE_LEGACY_IUNKNOWN )
    endif()
endif()