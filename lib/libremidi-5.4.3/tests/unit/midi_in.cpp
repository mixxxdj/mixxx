#include "../include_catch.hpp"

#include <libremidi/backends/keyboard/config.hpp>
#include <libremidi/libremidi.hpp>

#include <chrono>
#include <mutex>
#include <thread>

#if defined(LIBREMIDI_JACK)
  #include <libremidi/backends/jack/config.hpp>

  #include <jack/jack.h>
#endif

TEST_CASE("creation", "[midi_in]")
{
#if defined(LIBREMIDI_CI)
  SKIP("GH runners do not have MIDI support");
#endif

  GIVEN("A default midi input")
  {
    libremidi::midi_in in(libremidi::input_configuration{.on_message = [](auto) { }});
    THEN("created with the default MIDI 1 api for the platform")
    {
      REQUIRE(in.get_current_api() == libremidi::midi1::default_api());
    }
  }

  GIVEN("A default ump input")
  {
    libremidi::ump_input_configuration conf{.on_message = [](auto) { }};
    libremidi::midi_in in(conf);
    THEN("created with the default MIDI 2 api for the platform")
    {
      REQUIRE(in.get_current_api() == libremidi::midi2::default_api());
    }
  }

  GIVEN("A midi input with an explicitly unspecified API")
  {
    libremidi::midi_in in(
        libremidi::input_configuration{.on_message = [](auto) { }}, libremidi::API::UNSPECIFIED);
    THEN("created with the default api")
    {
      REQUIRE(in.get_current_api() == libremidi::midi1::default_api());
    }
  }

  GIVEN("A midi input with an empty API")
  {
    libremidi::midi_in in(
        libremidi::input_configuration{.on_message = [](auto) { }},
        libremidi::input_api_configuration{});
    THEN("created with defaultapi")
    {
      REQUIRE(in.get_current_api() == libremidi::midi1::default_api());
    }
  }
  GIVEN("A midi input with an explicit API")
  {
    libremidi::midi_in in(
        libremidi::input_configuration{.on_message = [](auto) { }}, libremidi::API::KEYBOARD);
    THEN("created with that api")
    {
      REQUIRE(in.get_current_api() == libremidi::API::KEYBOARD);
    }
  }

  GIVEN("A midi input with a proper API")
  {
    libremidi::midi_in in(
        libremidi::input_configuration{.on_message = [](auto) { }},
        libremidi::kbd_input_configuration{});
    THEN("created with the correct api")
    {
      REQUIRE(in.get_current_api() == libremidi::API::KEYBOARD);
    }
  }

  GIVEN("A midi 2 input with a proper midi1 API")
  {
    libremidi::midi_in in(
        libremidi::ump_input_configuration{.on_message = [](auto) { }},
        libremidi::kbd_input_configuration{});
    THEN("created with the correct api")
    {
      REQUIRE(in.get_current_api() == libremidi::API::KEYBOARD);
    }
  }
}

TEST_CASE("poly aftertouch", "[midi_in]")
{
#if defined(LIBREMIDI_JACK)
  #if defined(LIBREMIDI_CI)
  SKIP("GH runners do not have MIDI support");
  #endif

  std::vector<libremidi::message> queue;
  std::mutex qmtx;

  libremidi::midi_out midi_out{
      {}, libremidi::jack_output_configuration{.client_name = "libremidi-test-out"}};
  midi_out.open_virtual_port("port");

  libremidi::midi_in midi{
      libremidi::input_configuration{
          .on_message =
              [&](libremidi::message&& msg) {
    std::lock_guard _{qmtx};
    queue.push_back(std::move(msg));
  }},
      libremidi::jack_input_configuration{.client_name = "libremidi-test"}};
  midi.open_virtual_port("port");

  jack_options_t opt = JackNullOption;
  jack_status_t status;
  auto jack_client = jack_client_open("libremidi-tester", opt, &status);
  int ret = jack_activate(jack_client);
  if (ret != 0)
    return;

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  ret = jack_connect(jack_client, "libremidi-test-out:port", "libremidi-test:port");
  REQUIRE(ret == 0);

  // Flush potentially initial messages
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  {
    std::lock_guard _{qmtx};
    queue.clear();
  }

  // Send a message
  midi_out.send_message(libremidi::channel_events::poly_pressure(0, 60, 100));
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Check that we receive it correctly
  {
    std::lock_guard _{qmtx};
    REQUIRE(queue.size() == 1);
    libremidi::message mess = queue.back();
    REQUIRE(mess.bytes == libremidi::channel_events::poly_pressure(0, 60, 100).bytes);
  }
#endif
}
