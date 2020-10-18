# Protobuf
find_package(Protobuf)
protobuf_generate_cpp(
  PROTO_SOURCES
  PROTO_HEADERS
  beats.proto
  headers.proto
  keys.proto
  skin.proto
  waveform.proto
)

add_library(mixxx-proto STATIC ${PROTO_SOURCES} ${PROTO_HEADERS})

# Use uppercase PROTOBUF_ prefix for backwards compatibility for CMake < 3.6.0
target_include_directories(mixxx-proto SYSTEM PUBLIC ${PROTOBUF_INCLUDE_DIR})
if(PROTOBUF_INCLUDE_DIR AND PROTOBUF_LIBRARIES)
  target_link_libraries(mixxx-proto PUBLIC ${PROTOBUF_LIBRARIES})
elseif(PROTOBUF_INCLUDE_DIR AND PROTOBUF_LITE_LIBRARY)
  target_link_libraries(mixxx-proto PUBLIC ${PROTOBUF_LITE_LIBRARY})
else()
  message(FATAL_ERROR "Protobuf or Protobuf-lite libraries are required to compile Mixxx.")
endif()
