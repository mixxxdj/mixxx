### Create the library ###
if(LIBREMIDI_MODULE_BUILD)
  add_library(libremidi)
  set(_public PUBLIC)
  set(_private PRIVATE)
  target_sources(libremidi
    PUBLIC
      FILE_SET CXX_MODULES
      FILES
        "src/libremidi.ixx"
  )
  target_compile_options(libremidi PRIVATE
    $<$<CXX_COMPILER_ID:Clang>:-Wno-include-angled-in-module-purview>
    $<$<CXX_COMPILER_ID:AppleClang>:-Wno-include-angled-in-module-purview>
  )
elseif(LIBREMIDI_HEADER_ONLY)
  add_library(libremidi INTERFACE)
  set(_public INTERFACE)
  set(_private INTERFACE)
  target_compile_definitions(libremidi ${_public} LIBREMIDI_HEADER_ONLY)
else()
  add_library(libremidi)
  include(libremidi.sources)

  set(_public PUBLIC)
  set(_private PRIVATE)

  set_target_properties(libremidi PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION}
  )
endif()
add_library(libremidi::libremidi ALIAS libremidi)
target_compile_features(libremidi ${_public} cxx_std_${CMAKE_CXX_STANDARD})

### Library configuration ###
include(libremidi.warnings)

if(LIBREMIDI_SLIM_MESSAGE GREATER 0)
  target_compile_definitions(libremidi ${_public} LIBREMIDI_SLIM_MESSAGE=${LIBREMIDI_SLIM_MESSAGE})
endif()

if(LIBREMIDI_NO_BOOST)
  target_compile_definitions(libremidi ${_public} LIBREMIDI_NO_BOOST)
  message(STATUS "libremidi: Using std::vector for libremidi::message")
else()
  # Use of boost is public as it changes the ABI of libremidi::message
  if(TARGET Boost::boost)
    target_compile_definitions(libremidi ${_public} LIBREMIDI_USE_BOOST)
    target_link_libraries(libremidi ${_public} $<BUILD_INTERFACE:Boost::boost>)
    message(STATUS "libremidi: Using boost::small_vector for libremidi::message")
  elseif(Boost_INCLUDE_DIR)
    target_compile_definitions(libremidi ${_public} LIBREMIDI_USE_BOOST)
    target_include_directories(libremidi SYSTEM ${_public} $<BUILD_INTERFACE:${Boost_INCLUDE_DIR}>)
    message(STATUS "libremidi: Using boost::small_vector for libremidi::message")
  else()
    message(STATUS "libremidi: Using std::vector for libremidi::message")
  endif()
endif()

if(NOT LIBREMIDI_NO_EXPORTS)
  target_compile_definitions(libremidi ${_private} LIBREMIDI_EXPORTS)
endif()

if(LIBREMIDI_NI_MIDI2)
  target_compile_definitions(libremidi ${_public} LIBREMIDI_USE_NI_MIDI2)
  target_link_libraries(libremidi ${_public} $<BUILD_INTERFACE:ni::midi2>)
endif()

if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(libremidi ${_public} ${CMAKE_THREAD_LIBS_INIT})
endif()

if(PROJECT_IS_TOP_LEVEL)
  target_include_directories(libremidi ${_public}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  )
  target_include_directories(libremidi SYSTEM ${_public}
    $<INSTALL_INTERFACE:include>
  )
else()
  target_include_directories(libremidi SYSTEM ${_public}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  )
endif()

if(NOT LIBREMIDI_HEADER_ONLY)
  target_include_directories(libremidi ${_private} # Makes any above set warnings apply normally, without leaking out to clients
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  )
endif()
