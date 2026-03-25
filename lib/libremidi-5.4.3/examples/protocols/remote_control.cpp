#include <libremidi/libremidi.hpp>
#include <libremidi/protocols/remote_control.hpp>

#if defined(_WIN32) && __has_include(<winrt/base.h>)
  #include <winrt/base.h>
#endif

#if __has_include(<magic_enum_all.hpp>)
  #include <magic_enum_all.hpp>
#else
  #include <iomanip>
  #include <iostream>
  #include <sstream>
namespace magic_enum
{
std::string enum_name(auto cmd)
{
  std::stringstream ss;
  ss << "0x" << std::setbase(16) << static_cast<uint32_t>(cmd);
  return ss.str();
}
}
#endif

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <thread>

int main()
{
#if defined(_WIN32) && __has_include(<winrt/base.h>)
  winrt::init_apartment();
#endif

  auto api = libremidi::API::ALSA_SEQ;
  libremidi::observer observer{{.track_any = true}, api};
  if (observer.get_input_ports().empty())
    return 1;
  if (observer.get_output_ports().empty())
    return 1;

  libremidi::input_port ip;
  libremidi::output_port op;

  // Tested with https://github.com/NicoG60/TouchMCU
  for (auto& p : observer.get_input_ports())
    if (p.port_name == "TouchOSC")
      ip = p;
  for (auto& p : observer.get_output_ports())
    if (p.port_name == "TouchOSC")
      op = p;

  if (ip.port_name.empty() || op.port_name.empty())
  {
    std::cerr << "No device found !";
    return 1;
  }

  // Create the midi out port
  libremidi::midi_out midi_out{{}, api};

  // Set-up the remote control API.
  // Here we only do some logging, this is where commands sqall be handled.
  libremidi::remote_control_processor rcp{{.midi_out = [&](libremidi::message&& msg) {
    midi_out.send_message(msg);
  }, .on_command = [](libremidi::remote_control_protocol::mixer_command cmd, bool pressed) {
    std::cerr << "command: " << magic_enum::enum_name(cmd) << " -> "
              << (pressed ? "pressed" : "released") << "\n";
  }, .on_control = [](libremidi::remote_control_protocol::mixer_control ctl, int v) {
    std::cerr << "control: " << magic_enum::enum_name(ctl) << " -> " << v << "\n";
  }, .on_fader = [](libremidi::remote_control_protocol::fader f, uint16_t v) {
    std::cerr << "fader: " << magic_enum::enum_name(f) << " -> " << v << "\n";
  }}};

  // Initialize the midi in port
  libremidi::midi_in midi_in{
      {.on_message = [&](const libremidi::message& message) { rcp.on_midi(message); }}, api};

  // Open the ports
  if (auto err = midi_in.open_port(ip); err != stdx::error{})
    err.throw_exception();

  if (auto err = midi_out.open_port(op); err != stdx::error{})
    err.throw_exception();

  // Start communication
  rcp.start();

  // Blast messages :)
  using proto = libremidi::remote_control_protocol;
  unsigned i = 0;
  for (;;)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::time_t result = std::time(nullptr);
    auto ctime = std::localtime(&result);
    rcp.update_timecode(ctime->tm_hour, ctime->tm_min, ctime->tm_sec, 0);
    rcp.update_lcd(std::string(1, '\0' + i % 127), i % 112);
    rcp.fader(static_cast<proto::fader>(i % 8), (200 * i) % 16384);
    i++;
  }

  return 0;
}
