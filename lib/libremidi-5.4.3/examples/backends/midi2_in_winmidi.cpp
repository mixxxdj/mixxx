#include "backend_test_utils.hpp"

#include <libremidi/backends/winmidi.hpp>
#include <libremidi/libremidi.hpp>

using api = libremidi::winmidi::backend;

int main(void)
try
{
  winrt::init_apartment();
  std::cerr << "API: " << api::name << "\n";

  libremidi::observer_configuration obs_config{.track_any = false};
  api::midi_observer_configuration obs_api_config;
  libremidi::observer obs{obs_config, obs_api_config};

  auto ports = obs.get_input_ports();
  for (const auto& port : ports)
  {
    std::cerr << "Port: " << port.display_name << " : " << port.port_name << "; "
              << port.device_name << "\n";
  }

  {
    libremidi::ump_input_configuration in_config;
    in_config.on_message = [](const libremidi::ump& m) { std::cerr << m << "\n"; };
    api::midi_in_configuration in_api_config;
    libremidi::midi_in midiin{in_config, in_api_config};

    if (!ports.empty())
    {
      midiin.open_port(ports[0]);
      std::this_thread::sleep_for(std::chrono::seconds(5));
    }
  }

  // Bring your own shared MidiSession:
  {
    using namespace winrt::Microsoft::Windows::Devices::Midi2;
    auto my_session = MidiSession::Create(L"my app");

    std::vector<std::optional<libremidi::midi_in>> vec;
    vec.reserve(ports.size());
    static std::mutex mtx;

    for (int i = 0; i < ports.size(); i++)
    {
      libremidi::ump_input_configuration in_config;
      in_config.on_message = [i, ports](const libremidi::ump& m) {
        std::lock_guard _{mtx};
        std::cerr << ports[i].display_name << "(" << i << "): " << m << "\n";
      };
      api::midi_in_configuration in_api_config;
      in_api_config.context = &my_session;
      vec.emplace_back(std::in_place, in_config, in_api_config);
      vec.back()->open_port(ports[i]);
    }
    std::this_thread::sleep_for(std::chrono::seconds(25));
  }

  return 0;
}
catch (const std::exception& error)
{
  std::cerr << error.what() << std::endl;
  exit(EXIT_FAILURE);
}
