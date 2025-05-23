# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

# checking if compiler supports the standard filesystem library

if( MINGW OR BIT7Z_DISABLE_USE_STD_FILESYSTEM )
    # Some versions of MinGW have a buggy std::filesystem that doesn't correctly handle paths with unicode characters,
    # so we are always using the ghc::filesystem library.
    set( USE_STANDARD_FILESYSTEM OFF )
else()
    set( CMAKE_CXX_STANDARD 17 )
    include( CheckIncludeFileCXX )
    check_include_file_cxx( "filesystem" USE_STANDARD_FILESYSTEM )
endif()

if( USE_STANDARD_FILESYSTEM )
    include( CheckCXXSourceCompiles )
    string( CONFIGURE [[
        #include <cstdlib>
        #include <filesystem>

        int main() {
            auto cwd = std::filesystem::current_path();
            printf("%s", cwd.c_str());
            return EXIT_SUCCESS;
        }
    ]] code @ONLY )
    check_cxx_source_compiles( "${code}" STANDARD_FILESYSTEM_COMPILES )
endif()

if( NOT USE_STANDARD_FILESYSTEM OR NOT STANDARD_FILESYSTEM_COMPILES )
    # if standard filesystem lib is not supported, revert to C++14 standard and use the ghc::filesystem library
    set( CMAKE_CXX_STANDARD 14 )
    message( STATUS "Standard filesystem: NO (using ghc::filesystem)" )
else()
    message( STATUS "Standard filesystem: YES" )
endif()
set( CMAKE_CXX_STANDARD_REQUIRED ON )