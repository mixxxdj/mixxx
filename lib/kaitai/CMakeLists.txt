cmake_minimum_required (VERSION 3.11)
project (kaitai_struct_cpp_stl_runtime CXX)
enable_testing()

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)
option(BUILD_TESTS "Build tests" ON)

set (CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(ZLIB)
find_package(Iconv)

set (HEADERS
    kaitai/kaitaistream.h
    kaitai/kaitaistruct.h
    kaitai/exceptions.h
)

set (SOURCES
    kaitai/kaitaistream.cpp
)

set(STRING_ENCODING_TYPE "ICONV" CACHE STRING "Set the way strings have to be encoded (ICONV|WIN32API|NONE|...)")

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library (${PROJECT_NAME} ${HEADERS} ${SOURCES})
set_property(TARGET ${PROJECT_NAME} PROPERTY PUBLIC_HEADER ${HEADERS})
target_include_directories(${PROJECT_NAME} INTERFACE ${PROJECT_SOURCE_DIR})

if (ZLIB_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB)
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DKS_ZLIB)
endif()

if(Iconv_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE Iconv::Iconv)
endif()

include(Common.cmake)

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/kaitai
)

# Add the tests
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
