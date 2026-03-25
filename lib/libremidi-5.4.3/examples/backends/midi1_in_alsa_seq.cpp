#include "backend_test_utils.hpp"

#include <libremidi/backends/alsa_seq.hpp>
#include <libremidi/libremidi.hpp>

using api = libremidi::alsa_seq::backend;

int main(void)
try
{
  std::cerr << "API: " << api::name << "\n";

  libremidi::observer_configuration obs_config{.track_any = true};
  api::midi_observer_configuration obs_api_config;
  libremidi::observer obs{obs_config, obs_api_config};

  auto ports = obs.get_input_ports();
  for (const auto& port : ports)
  {
    std::cerr << "Port: " << port.port_name << "\n";
  }

  {
    libremidi::input_configuration in_config;
    in_config.on_message = [](const libremidi::message& m) { std::cerr << m << "\n"; };
    api::midi_in_configuration in_api_config;
    libremidi::midi_in midiin{in_config, in_api_config};

    if (!ports.empty())
    {
      midiin.open_port(ports[0]);
      sleep(10);
    }
  }

  return 0;
}
catch (const std::exception& error)
{
  std::cerr << error.what() << std::endl;
  exit(EXIT_FAILURE);
}
