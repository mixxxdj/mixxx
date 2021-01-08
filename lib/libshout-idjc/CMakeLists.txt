cmake_minimum_required(VERSION 3.0)
project(shout_mixxx C)
 
find_package(OpenSSL)

include_directories(
  .
  src
  include
  src/common
)

add_library(shout_mixxx STATIC
  src/common/avl/avl.c
  src/common/net/sock.c
  src/common/net/resolver.c
  src/common/timing/timing.c
  src/common/httpp/httpp.c
  src/common/httpp/encoding.c
  src/common/thread/thread.c
  src/shout.c
  src/util.c
  src/queue.c
  src/proto_http.c
  src/proto_xaudiocast.c
  src/proto_icy.c
  src/proto_roaraudio.c
  src/format_ogg.c
  src/format_webm.c
  src/format_mp3.c
  src/codec_vorbis.c
  src/codec_opus.c
  src/tls.c
)

target_compile_definitions(shout_mixxx PRIVATE HAVE_CONFIG_H)
target_compile_options(shout_mixxx PRIVATE -pthread -Wall)

target_link_libraries(shout_mixxx ogg vorbis OpenSSL::SSL OpenSSL::Crypto)


