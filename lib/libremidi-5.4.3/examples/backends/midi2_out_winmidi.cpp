#include "backend_test_utils.hpp"

#include <libremidi/backends/winmidi.hpp>
#include <libremidi/libremidi.hpp>
using api = libremidi::winmidi::backend;

int main(void)
try
{
  winrt::init_apartment();

  libremidi::observer_configuration obs_config{.track_any = false};
  api::midi_observer_configuration obs_api_config;
  libremidi::observer obs{obs_config, obs_api_config};

  auto ports = obs.get_output_ports();
  for (const auto& port : ports)
  {
    std::cerr << "Port: " << port.display_name << " : " << port.port_name << "; "
              << port.device_name << "\n";
  }

  {
    libremidi::output_configuration config;
    api::midi_out_configuration api_config;
    libremidi::midi_out midiout{config, api_config};

    if (!ports.empty())
    {
      midiout.open_port(ports[0]);
      for (int n = 0; n < 100; n++)
      {
        for (int i = 81; i < 89; i++)
          midiout.send_message(libremidi::channel_events::control_change(1, i, rand()));
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    }
  }

  return 0;
}
catch (const std::exception& error)
{
  std::cerr << error.what() << std::endl;
  exit(EXIT_FAILURE);
}
