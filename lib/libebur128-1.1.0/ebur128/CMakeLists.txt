set(BUILD_STATIC_LIBS       ON  CACHE BOOL "Build static library")
set(WITH_STATIC_PIC         OFF CACHE BOOL "Compile static library with -fPIC flag")
set(ENABLE_INTERNAL_QUEUE_H OFF CACHE BOOL "Use own queue.h")
set(DISABLE_SPEEXDSP        OFF CACHE BOOL "Don't build with speexdsp")

#### queue.h
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/queuetest.c
"#include <sys/queue.h>\nLIST_HEAD(listhead, entry) head;\nint main() { return 0; }")
try_compile(HAS_QUEUE ${CMAKE_CURRENT_BINARY_DIR}
                      ${CMAKE_CURRENT_BINARY_DIR}/queuetest.c)

set(SUMMARY_HAS_QUEUE ${HAS_QUEUE} CACHE INTERNAL "")

if(ENABLE_INTERNAL_QUEUE_H)
  include_directories(SYSTEM queue)
endif()


if(MSVC)
  add_definitions(-D_USE_MATH_DEFINES)
endif()


set(EBUR128_VERSION_MAJOR 1)
set(EBUR128_VERSION 1.1.0)

#### static
if(BUILD_STATIC_LIBS)
  add_library(ebur128_static STATIC ebur128.c)
  set_property(TARGET ebur128_static PROPERTY OUTPUT_NAME ebur128)
endif()

if(WITH_STATIC_PIC)
  set_property(TARGET ebur128_static PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

#### shared
add_library(ebur128 SHARED ebur128.c)
set_target_properties(ebur128 PROPERTIES
    SOVERSION ${EBUR128_VERSION_MAJOR}
    VERSION ${EBUR128_VERSION})

#### speexdsp
if(NOT DISABLE_SPEEXDSP)
  find_pkg_config(SPEEXDSP speexdsp)
    if(SPEEXDSP_FOUND)
      target_include_directories(ebur128 PRIVATE
                                 ${SPEEXDSP_INCLUDE_DIRS})
      target_compile_definitions(ebur128 PRIVATE
                                 -DUSE_SPEEX_RESAMPLER
                                 -DHAVE_STDINT_H)
    endif()
endif()

if(UNIX)
  target_link_libraries(ebur128 m)
endif()

if(SPEEXDSP_FOUND AND NOT DISABLE_SPEEXDSP)
    if(BUILD_STATIC_LIBS)
      target_compile_options(ebur128_static PRIVATE ${SPEEXDSP_CFLAGS})
    endif()
  target_compile_options(ebur128 PRIVATE ${SPEEXDSP_CFLAGS})
  target_link_libraries(ebur128 ${SPEEXDSP_LIBRARIES})
endif()

set(SUMMARY_SPEEXDSP_FOUND ${SPEEXDSP_FOUND} CACHE INTERNAL "")
set(EBUR128_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")

install(FILES ebur128.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
if(BUILD_STATIC_LIBS)
  install(TARGETS ebur128 ebur128_static DESTINATION ${CMAKE_INSTALL_LIBDIR})
else()
  install(TARGETS ebur128 DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
