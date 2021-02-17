# Protobuf
find_package(Protobuf)
add_library(mixxx-proto OBJECT)
protobuf_generate(
  LANGUAGE cpp
  TARGET mixxx-proto
  PROTOS
    beats.proto
    headers.proto
    keys.proto
    skin.proto
    waveform.proto
)

if(TARGET protobuf::libprotobuf-lite)
    target_link_libraries(mixxx-proto PUBLIC protobuf::libprotobuf-lite)
elseif(TARGET protobuf::libprotobuf)
    target_link_libraries(mixxx-proto PUBLIC protobuf::libprotobuf)
else()
    message(FATAL_ERROR "Protobuf or Protobuf-lite libraries are required to compile Mixxx.")
endif()
