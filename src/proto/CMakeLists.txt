# Protobuf

# first try the package provided config
find_package(Protobuf CONFIG)
if(NOT Protobuf_FOUND)
  # Fall back to the CMake provide module
  find_package(Protobuf MODULE REQUIRED)
endif()

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
    target_link_libraries(mixxx-proto PRIVATE protobuf::libprotobuf-lite)
elseif(TARGET protobuf::libprotobuf)
    target_link_libraries(mixxx-proto PRIVATE protobuf::libprotobuf)
else()
    message(FATAL_ERROR "Protobuf or Protobuf-lite libraries are required to compile Mixxx.")
endif()
