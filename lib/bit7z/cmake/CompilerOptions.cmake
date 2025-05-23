# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

# compiler-specific options

if( MSVC )
    # setting a pdb file name for debug builds (otherwise it is not generated!)
    set_target_properties( ${LIB_TARGET} PROPERTIES COMPILE_PDB_NAME_DEBUG ${LIB_TARGET}${CMAKE_DEBUG_POSTFIX} )

    # release builds should be optimized (e.g., for size)
    target_compile_options( ${LIB_TARGET} PRIVATE "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:/Os>" )
    target_compile_options( ${LIB_TARGET} PRIVATE "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:/Oi>" )
    target_compile_options( ${LIB_TARGET} PRIVATE "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:/GS>" )
    target_compile_options( ${LIB_TARGET} PRIVATE "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:/Gy>" )

    option( BIT7Z_ANALYZE_CODE "Enable or disable code analysis" )
    message( STATUS "Code analysis: ${BIT7Z_ANALYZE_CODE}" )
    if( BIT7Z_ANALYZE_CODE )
        target_compile_options( ${LIB_TARGET} PRIVATE "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:/analyze>" )
    endif()

    target_compile_options( ${LIB_TARGET} PRIVATE /EHsc )

    # remove CMake default warning level
    string( REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )
    string( REGEX REPLACE "/GR" "/GR-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )

    # warning flags (as suggested in https://lefticus.gitbooks.io/cpp-best-practices/)
    target_compile_options( ${LIB_TARGET} PRIVATE /W4 /w14640 /w14242 /w14254 /w14263 /w14265 /w14287 /we4289 /w14296
                            /w14311 /w14545 /w14546 /w14547 /w14549 /w14555 /w14619 /w14640 /w14826 /w14905 /w14906
                            /w14928 )

    # C++ standard conformance options of MSVC
    target_compile_options( ${LIB_TARGET} PRIVATE /fp:precise /Zc:wchar_t /Zc:rvalueCast /Zc:inline
                            /Zc:forScope /Zc:strictStrings /Zc:throwingNew /Zc:referenceBinding )

    if( CMAKE_GENERATOR MATCHES "Visual Studio" )
        target_compile_options( ${LIB_TARGET} PRIVATE /MP$ENV{NUMBER_OF_PROCESSORS} )
    endif()

    # linker flags
    set( CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /nologo" )

    # options only for specific MSVC versions
    if( MSVC_VERSION GREATER_EQUAL 1914 ) # MSVC >= 15.7 (VS 2017)
        target_compile_options( ${LIB_TARGET} PRIVATE /Zc:__cplusplus )
    endif()
    if( MSVC_VERSION GREATER_EQUAL 1910 ) # MSVC >= 15.0 (VS 2017)
        # treating warnings as errors
        target_compile_options( ${LIB_TARGET} PRIVATE /WX )
    else() # MSVC < 15.0 (i.e., <= VS 2015)
        # ignoring C4127 warning
        target_compile_options( ${LIB_TARGET} PRIVATE /wd4127 )
    endif()

    # static runtime option
    option( BIT7Z_STATIC_RUNTIME "Enable or disable using /MT MSVC flag" )
    message( STATUS "Static runtime: ${BIT7Z_STATIC_RUNTIME}" )
    if( BIT7Z_STATIC_RUNTIME )
        set( CompilerFlags
             CMAKE_CXX_FLAGS_DEBUG
             CMAKE_CXX_FLAGS_RELEASE
             CMAKE_C_FLAGS_DEBUG
             CMAKE_C_FLAGS_RELEASE )
        foreach( CompilerFlag ${CompilerFlags} )
            string( REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}" )
            set( ${CompilerFlag} "${${CompilerFlag}}" CACHE STRING "msvc compiler flags" FORCE )
            message( STATUS "MSVC flags: ${CompilerFlag}:${${CompilerFlag}}" )
        endforeach()
    endif()
else()
    target_compile_options( ${LIB_TARGET} PRIVATE -Wall -Wextra -Werror -Wconversion -Wsign-conversion )
endif()

# Extra warning flags for Clang
if( CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
    if( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 3.6 )
        target_compile_options( ${LIB_TARGET} PRIVATE -Wno-inconsistent-missing-override )
    endif()
    if( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 3.8 )
        target_compile_options( ${LIB_TARGET} PRIVATE -Wdouble-promotion )
    endif()
    if( CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6.0 )
        target_compile_options( ${LIB_TARGET} PRIVATE -Wno-missing-braces -Wmissing-field-initializers )
    endif()
endif()

if( APPLE )
    set( CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>" )
    set( CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>" )
endif()

# Extra warning flags for GCC
if( CMAKE_CXX_COMPILER_ID MATCHES "GNU" )
    target_compile_options( ${LIB_TARGET} PRIVATE -Wshadow -Wcast-align -Wunused
                            -Woverloaded-virtual -Wformat=2 -Wdouble-promotion -Wlogical-op )
    if( NOT MINGW AND BIT7Z_USE_VIRTUAL_DESTRUCTOR_IN_IUNKNOWN )
        target_compile_options( ${LIB_TARGET} PRIVATE -Wnon-virtual-dtor )
    endif()
    if( MINGW )
        # Some versions of MinGW might complain that the library is too big when linking to it.
        # Using -Wa,-mbig-obj fixes the linking error.
        # (https://digitalkarabela.com/mingw-w64-how-to-fix-file-too-big-too-many-sections/).
        target_compile_options( ${LIB_TARGET} PUBLIC -Wa,-mbig-obj )
        target_compile_options( ${LIB_TARGET} PRIVATE -Wno-error=non-virtual-dtor )
        if( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8.0 )
            target_compile_options( ${LIB_TARGET} PRIVATE -Wno-cast-function-type )
        endif()
    endif()
    if( CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0 )
        target_compile_options( ${LIB_TARGET} PRIVATE
                                -Wno-missing-field-initializers
                                -Wno-shadow
                                -Wno-unused-parameter )
    endif()
    if( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 6.0 )
        target_compile_options( ${LIB_TARGET} PRIVATE
                                # GCC 6.0+ complains on 7-zip macros using misleading indentation,
                                # disabling the warning to make it compile.
                                -Wno-misleading-indentation
                                # Extra warning flags for GCC 6.0+
                                -Wduplicated-cond -Wnull-dereference )
    endif()
    if( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 7.0 )
        # Extra warning flags for GCC 7.0+
        target_compile_options( ${LIB_TARGET} PRIVATE -Wduplicated-branches -Wrestrict )
    endif()
endif()