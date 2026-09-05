if(CMAKE_VERSION VERSION_GREATER 3.30)
    set(CMAKE_FETCHCONTENT_SYSTEM_KEYWORD SYSTEM)
endif()

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2
    GIT_TAG        v3.15.3
    ${CMAKE_FETCHCONTENT_SYSTEM_KEYWORD}
)

FetchContent_MakeAvailable(Catch2)
if(NOT TARGET Catch2::Catch2WithMain)
    message(WARNING "libremidi: Catch2::Catch2WithMain target not found")
    return()
endif()

message(STATUS "libremidi: compiling tests")
if(LIBREMIDI_CI)
    target_compile_definitions(libremidi ${_public} LIBREMIDI_CI)
endif()

add_executable(conversion_test tests/unit/conversion.cpp)
target_link_libraries(conversion_test PRIVATE libremidi Catch2::Catch2WithMain)

add_executable(error_test tests/unit/error.cpp)
target_link_libraries(error_test PRIVATE libremidi Catch2::Catch2WithMain)

add_executable(midiin_test tests/unit/midi_in.cpp)
target_link_libraries(midiin_test PRIVATE libremidi Catch2::Catch2WithMain)

add_executable(midiout_test tests/unit/midi_out.cpp)
target_link_libraries(midiout_test PRIVATE libremidi Catch2::Catch2WithMain)

add_executable(midifile_read_test tests/unit/midifile_read.cpp)
target_link_libraries(midifile_read_test PRIVATE libremidi Catch2::Catch2WithMain)
target_compile_definitions(midifile_read_test PRIVATE "LIBREMIDI_TEST_CORPUS=\"${CMAKE_CURRENT_SOURCE_DIR}/tests/corpus\"")

add_executable(midifile_write_test tests/unit/midifile_write.cpp)
target_link_libraries(midifile_write_test PRIVATE libremidi Catch2::Catch2WithMain)
target_compile_definitions(midifile_write_test PRIVATE "LIBREMIDI_TEST_CORPUS=\"${CMAKE_CURRENT_SOURCE_DIR}/tests/corpus\"")

add_executable(midifile_write_tracks_test tests/integration/midifile_write_tracks.cpp)
target_link_libraries(midifile_write_tracks_test PRIVATE libremidi Catch2::Catch2WithMain)

add_executable(protocols_test tests/unit/protocols.cpp)
target_link_libraries(protocols_test PRIVATE libremidi Catch2::Catch2WithMain)

add_executable(midi_stream_decoder_test tests/unit/midi_stream_decoder.cpp)
target_link_libraries(midi_stream_decoder_test PRIVATE libremidi Catch2::Catch2WithMain)

add_executable(midi_timing_test tests/unit/midi_timing.cpp)
target_link_libraries(midi_timing_test PRIVATE libremidi Catch2::Catch2WithMain)

add_executable(rawio_test tests/unit/rawio.cpp)
target_link_libraries(rawio_test PRIVATE libremidi Catch2::Catch2WithMain)

include(CTest)
add_test(NAME conversion_test COMMAND conversion_test)
add_test(NAME error_test COMMAND error_test)
add_test(NAME midiin_test COMMAND midiin_test --allow-running-no-tests)
add_test(NAME midiout_test COMMAND midiout_test --allow-running-no-tests)
add_test(NAME midifile_read_test COMMAND midifile_read_test)
add_test(NAME midifile_write_test COMMAND midifile_write_test)
add_test(NAME midifile_write_tracks_test COMMAND midifile_write_tracks_test)
add_test(NAME protocols_test COMMAND protocols_test)
add_test(NAME midi_stream_decoder_test COMMAND midi_stream_decoder_test)
add_test(NAME midi_timing_test COMMAND midi_timing_test)
add_test(NAME rawio_test COMMAND rawio_test)

# PipeWire shared-context regression tests. Standalone programs (no Catch2):
# each skips with exit 0 when no daemon is reachable and arms a watchdog so a
# lock-corruption regression fails instead of hanging.
if(LIBREMIDI_HAS_PIPEWIRE)
  foreach(_pwtest pipewire_context_sync pipewire_context_reconnect pipewire_context_subscriptions
                  pipewire_context_error_scope pipewire_context_shared_teardown)
    add_executable(${_pwtest}_test tests/integration/${_pwtest}.cpp)
    target_link_libraries(${_pwtest}_test PRIVATE libremidi)
    add_test(NAME ${_pwtest}_test COMMAND ${_pwtest}_test)
  endforeach()
endif()
