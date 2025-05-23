# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

cmake_minimum_required( VERSION 3.14 )

project( bit7z
         VERSION 4.0.9
         DESCRIPTION "A C++ static library offering a clean and simple interface to the 7-zip/p7zip shared libraries"
         HOMEPAGE_URL "https://github.com/rikyoz/bit7z/" )
set( CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON" )

#add_definitions(-DBIT7Z_STATIC_LIB)

cmake_policy( SET CMP0022 NEW )
if( POLICY CMP0076 )
    cmake_policy( SET CMP0076 NEW )
endif()

# public headers
set( PUBLIC_HEADERS
     include/bit7z/bit7z.hpp
     include/bit7z/bit7zlibrary.hpp
     include/bit7z/bitabstractarchivecreator.hpp
     include/bit7z/bitabstractarchivehandler.hpp
     include/bit7z/bitabstractarchiveopener.hpp
     include/bit7z/bitarchiveeditor.hpp
     include/bit7z/bitarchiveitem.hpp
     include/bit7z/bitarchiveiteminfo.hpp
     include/bit7z/bitarchiveitemoffset.hpp
     include/bit7z/bitarchivereader.hpp
     include/bit7z/bitarchivewriter.hpp
     include/bit7z/bitcompressionlevel.hpp
     include/bit7z/bitcompressionmethod.hpp
     include/bit7z/bitcompressor.hpp
     include/bit7z/bitdefines.hpp
     include/bit7z/biterror.hpp
     include/bit7z/bitexception.hpp
     include/bit7z/bitextractor.hpp
     include/bit7z/bitfilecompressor.hpp
     include/bit7z/bitfileextractor.hpp
     include/bit7z/bitformat.hpp
     include/bit7z/bitfs.hpp
     include/bit7z/bitgenericitem.hpp
     include/bit7z/bitinputarchive.hpp
     include/bit7z/bititemsvector.hpp
     include/bit7z/bitmemcompressor.hpp
     include/bit7z/bitmemextractor.hpp
     include/bit7z/bitoutputarchive.hpp
     include/bit7z/bitpropvariant.hpp
     include/bit7z/bitstreamcompressor.hpp
     include/bit7z/bitstreamextractor.hpp
     include/bit7z/bittypes.hpp
     include/bit7z/bitwindows.hpp )

# header files
set( HEADERS
     src/internal/archiveproperties.hpp
     src/internal/bufferextractcallback.hpp
     src/internal/bufferitem.hpp
     src/internal/bufferutil.hpp
     src/internal/callback.hpp
     src/internal/cbufferinstream.hpp
     src/internal/cbufferoutstream.hpp
     src/internal/cfileinstream.hpp
     src/internal/cfileoutstream.hpp
     src/internal/cfixedbufferoutstream.hpp
     src/internal/cmultivolumeinstream.hpp
     src/internal/cmultivolumeoutstream.hpp
     src/internal/com.hpp
     src/internal/cstdinstream.hpp
     src/internal/cstdoutstream.hpp
     src/internal/csymlinkinstream.hpp
     src/internal/cvolumeinstream.hpp
     src/internal/cvolumeoutstream.hpp
     src/internal/dateutil.hpp
     src/internal/extractcallback.hpp
     src/internal/failuresourcecategory.hpp
     src/internal/fileextractcallback.hpp
     src/internal/fixedbufferextractcallback.hpp
     src/internal/formatdetect.hpp
     src/internal/fsindexer.hpp
     src/internal/fsitem.hpp
     src/internal/fsutil.hpp
     src/internal/fs.hpp
     src/internal/genericinputitem.hpp
     src/internal/guiddef.hpp
     src/internal/guids.hpp
     src/internal/hresultcategory.hpp
     src/internal/internalcategory.hpp
     src/internal/macros.hpp
     src/internal/opencallback.hpp
     src/internal/operationcategory.hpp
     src/internal/operationresult.hpp
     src/internal/processeditem.hpp
     src/internal/renameditem.hpp
     src/internal/stdinputitem.hpp
     src/internal/streamextractcallback.hpp
     src/internal/streamutil.hpp
     src/internal/stringutil.hpp
     src/internal/updatecallback.hpp
     src/internal/util.hpp
     src/internal/windows.hpp )

# source files
set( SOURCES
     src/bit7zlibrary.cpp
     src/bitabstractarchivecreator.cpp
     src/bitabstractarchivehandler.cpp
     src/bitabstractarchiveopener.cpp
     src/bitarchiveeditor.cpp
     src/bitarchiveitem.cpp
     src/bitarchiveiteminfo.cpp
     src/bitarchiveitemoffset.cpp
     src/bitarchivereader.cpp
     src/bitarchivewriter.cpp
     src/biterror.cpp
     src/bitexception.cpp
     src/bitfilecompressor.cpp
     src/bitformat.cpp
     src/bitinputarchive.cpp
     src/bititemsvector.cpp
     src/bitoutputarchive.cpp
     src/bitpropvariant.cpp
     src/bittypes.cpp
     src/internal/bufferextractcallback.cpp
     src/internal/bufferitem.cpp
     src/internal/bufferutil.cpp
     src/internal/callback.cpp
     src/internal/cbufferinstream.cpp
     src/internal/cbufferoutstream.cpp
     src/internal/cfileinstream.cpp
     src/internal/cfileoutstream.cpp
     src/internal/cfixedbufferoutstream.cpp
     src/internal/cmultivolumeinstream.cpp
     src/internal/cmultivolumeoutstream.cpp
     src/internal/cstdinstream.cpp
     src/internal/cstdoutstream.cpp
     src/internal/csymlinkinstream.cpp
     src/internal/cvolumeinstream.cpp
     src/internal/cvolumeoutstream.cpp
     src/internal/dateutil.cpp
     src/internal/extractcallback.cpp
     src/internal/failuresourcecategory.cpp
     src/internal/fileextractcallback.cpp
     src/internal/fixedbufferextractcallback.cpp
     src/internal/formatdetect.cpp
     src/internal/fsindexer.cpp
     src/internal/fsitem.cpp
     src/internal/fsutil.cpp
     src/internal/genericinputitem.cpp
     src/internal/guids.cpp
     src/internal/hresultcategory.cpp
     src/internal/internalcategory.cpp
     src/internal/opencallback.cpp
     src/internal/operationcategory.cpp
     src/internal/operationresult.cpp
     src/internal/processeditem.cpp
     src/internal/renameditem.cpp
     src/internal/stdinputitem.cpp
     src/internal/streamextractcallback.cpp
     src/internal/stringutil.cpp
     src/internal/updatecallback.cpp
     src/internal/windows.cpp )

# library output file name options
include( cmake/OutputOptions.cmake )

# detecting if the std::filesystem is available and usable
include( cmake/FilesystemDetect.cmake )

# printing library and compiler information (useful for debugging)
message( STATUS "Target Version: ${CMAKE_PROJECT_VERSION}" )
message( STATUS "Compiler ID: ${CMAKE_CXX_COMPILER_ID}" )
message( STATUS "Compiler Version: ${CMAKE_CXX_COMPILER_VERSION}" )
message( STATUS "Architecture: ${BIT7Z_TARGET_ARCH_NAME}" )
message( STATUS "Build Type: ${CMAKE_BUILD_TYPE}" )
message( STATUS "Language Standard for bit7z: C++${CMAKE_CXX_STANDARD}" )

# library target options
set( LIB_TARGET bit7z${ARCH_POSTFIX} )
add_library( ${LIB_TARGET} STATIC )
target_sources( ${LIB_TARGET}
                PUBLIC ${PUBLIC_HEADERS}
                PRIVATE ${HEADERS} ${SOURCES} )

# additional target without the architecture suffix in the name
if( NOT "${ARCH_POSTFIX}" STREQUAL "" )
    add_library( bit7z ALIAS ${LIB_TARGET} )
endif()

# bit7z build options
include( cmake/BuildOptions.cmake )

# compiler-specific options
include( cmake/CompilerOptions.cmake )

# dependencies
include( cmake/Dependencies.cmake )

# 7-zip source code
target_link_libraries( ${LIB_TARGET} PRIVATE 7-zip )

# filesystem library (needed if std::filesystem is not available)
if( ghc_filesystem_ADDED )
    target_link_libraries( ${LIB_TARGET} PRIVATE ghc_filesystem )
endif()

# public includes
target_include_directories( ${LIB_TARGET} PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
                                                 "$<INSTALL_INTERFACE:include>" )

# private includes
target_include_directories( ${LIB_TARGET} PRIVATE "${PROJECT_SOURCE_DIR}/include/bit7z"
                                                  "${PROJECT_SOURCE_DIR}/src" )

# 7-zip compilation definitions
target_compile_definitions( ${LIB_TARGET} PRIVATE UNICODE _UNICODE )
if( WIN32 )
    target_compile_definitions( ${LIB_TARGET} PRIVATE _WINDOWS )
    target_link_libraries( ${LIB_TARGET} PUBLIC oleaut32 )
endif()

if( MINGW )
    target_link_libraries( ${LIB_TARGET} PUBLIC uuid )
endif()

if( UNIX )
    target_compile_definitions( ${LIB_TARGET} PRIVATE _LARGEFILE64_SOURCE _LARGEFILE_SOURCE _REENTRANT
                                                      EXTERNAL_CODECS ENV_UNIX BREAK_HANDLER USE_WIN_FILE )
    target_compile_definitions( ${LIB_TARGET} PUBLIC _TIME_BITS=64 )
    if ( ( NOT ANDROID ) OR ( ANDROID_PLATFORM GREATER_EQUAL 24 ) )
        target_compile_definitions( ${LIB_TARGET} PUBLIC _FILE_OFFSET_BITS=64 )
    endif()
    target_link_libraries( ${LIB_TARGET} PUBLIC ${CMAKE_DL_LIBS} )
endif()

# sanitizers
include( cmake/Sanitizers.cmake )

# tests
if( BIT7Z_BUILD_TESTS )
    enable_testing()
    add_subdirectory( tests )
endif()

# docs
if( BIT7Z_BUILD_DOCS )
    add_subdirectory( docs )
endif()