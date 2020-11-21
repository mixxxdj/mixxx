# Protobuf
find_package(Protobuf REQUIRED)
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
if(TARGET protobuf::libprotobuf-lite)
    target_link_libraries(mixxx-proto PUBLIC protobuf::libprotobuf-lite)
else()
    target_link_libraries(mixxx-proto PUBLIC protobuf::libprotobuf)
endif()
