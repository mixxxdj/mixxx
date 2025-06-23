# Protobuf

# first try the package provided config
find_package(Protobuf CONFIG)
if(NOT Protobuf_FOUND)
  # Fall back to the CMake provide module
  find_package(Protobuf MODULE REQUIRED)
endif()

add_library(mixxx-proto OBJECT)

if(TARGET protobuf::libprotobuf-lite)
    target_link_libraries(mixxx-proto PUBLIC protobuf::libprotobuf-lite)
elseif(TARGET protobuf::libprotobuf)
    target_link_libraries(mixxx-proto PUBLIC protobuf::libprotobuf)
else()
    message(FATAL_ERROR "Protobuf or Protobuf-lite libraries are required to compile Mixxx.")
endif()

if(EMSCRIPTEN)
  # If we try linking in a proto lib built without -pthread we get
  #   wasm-ld: error: --shared-memory is disallowed by keys.pb.cc.o
  #   because it was not compiled with 'atomics' or 'bulk-memory' features.
  # See https://groups.google.com/g/emscripten-discuss/c/G4nwFprZFYo
  target_compile_options(mixxx-proto PRIVATE -pthread)
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
