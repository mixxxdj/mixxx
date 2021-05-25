set(BUILD_STATIC_LIBS       ON  CACHE BOOL "Build static library")
set(WITH_STATIC_PIC         OFF CACHE BOOL "Compile static library with -fPIC flag")

#### queue.h
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/queuetest.c
"#include <sys/queue.h>\nLIST_HEAD(listhead, entry) head;\nint main() { return 0; }")
try_compile(HAS_QUEUE ${CMAKE_CURRENT_BINARY_DIR}
                      ${CMAKE_CURRENT_BINARY_DIR}/queuetest.c)

set(SUMMARY_HAS_QUEUE ${HAS_QUEUE} CACHE INTERNAL "")

include(CMakeDependentOption)
cmake_dependent_option(ENABLE_INTERNAL_QUEUE_H "Use own queue.h" OFF "HAS_QUEUE" ON)
# PARENT_SCOPE set necessary due to status print in parent CMakeLists.txt
set(ENABLE_INTERNAL_QUEUE_H ${ENABLE_INTERNAL_QUEUE_H} PARENT_SCOPE)

if(ENABLE_INTERNAL_QUEUE_H)
  include_directories(SYSTEM queue)
endif()


if(MSVC)
  add_definitions(-D_USE_MATH_DEFINES)
  if(CMAKE_SIZEOF_VOID_P LESS 8)
    add_definitions(/arch:SSE2)
  endif()
endif()


set(EBUR128_VERSION_MAJOR 1)
set(EBUR128_VERSION 1.2.6)

add_library(ebur128 ebur128.c)
set_target_properties(ebur128 PROPERTIES
  SOVERSION ${EBUR128_VERSION_MAJOR}
  VERSION ${EBUR128_VERSION}
)

if(BUILD_SHARED_LIBS)
  if(MSVC)
    target_sources(ebur128 PRIVATE ebur128.def)
  endif()
else()
  if(WITH_STATIC_PIC)
    set_property(TARGET ebur128 PROPERTY POSITION_INDEPENDENT_CODE ON)
  endif()
endif()

# Link with Math library if available
find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
  target_link_libraries(ebur128 ${MATH_LIBRARY})
endif()

if(ENABLE_FUZZER)
  target_compile_options(ebur128 PUBLIC "${FUZZER_FLAGS}")
  target_compile_definitions(ebur128 PRIVATE malloc=my_malloc calloc=my_calloc)
  target_link_libraries(ebur128 "${SANITIZER_FLAGS}")
endif()

set(EBUR128_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")

install(FILES ebur128.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS ebur128
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

#### pkg-config
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libebur128.pc.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/libebur128.pc @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libebur128.pc"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
