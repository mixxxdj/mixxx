#include "backend_test_utils.hpp"

#include <libremidi/backends/alsa_raw_ump.hpp>
#include <libremidi/libremidi.hpp>

using api = libremidi::alsa_raw_ump::backend;

int main(void)
try
{
  std::cerr << "API: " << api::name << "\n";

  libremidi::observer_configuration obs_config{.track_any = true};
  api::midi_observer_configuration obs_api_config;
  libremidi::observer obs{obs_config, obs_api_config};

  auto ports = obs.get_output_ports();
  for (const auto& port : ports)
  {
    std::cerr << "Port: " << port.port_name << "\n";
  }

  libremidi::output_configuration out_config;
  api::midi_out_configuration out_api_config;
  libremidi::midi_out midiout{out_config, out_api_config};

  if (!ports.empty())
    midiout.open_port(ports[0]);

  return 0;
}
catch (const std::exception& error)
{
  std::cerr << error.what() << std::endl;
  exit(EXIT_FAILURE);
}
