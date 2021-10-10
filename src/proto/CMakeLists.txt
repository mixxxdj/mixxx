# Protobuf
find_package(Protobuf REQUIRED)

add_library(mixxx-proto OBJECT)

include(IsStaticLibrary)
if(TARGET protobuf::libprotobuf-lite)
    target_link_libraries(mixxx-proto PRIVATE protobuf::libprotobuf-lite)
elseif(TARGET protobuf::libprotobuf)
    target_link_libraries(mixxx-proto PRIVATE protobuf::libprotobuf)
else()
    message(FATAL_ERROR "Protobuf or Protobuf-lite libraries are required to compile Mixxx.")
endif()

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
