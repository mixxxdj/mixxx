#pragma once
#include <libremidi/backends/kdmapi/config.hpp>
#include <libremidi/backends/kdmapi/helpers.hpp>
#include <libremidi/detail/observer.hpp>

namespace libremidi::kdmapi
{

class observer final : public observer_api
{
public:
  struct
      : libremidi::observer_configuration
      , kdmapi::observer_configuration
  {
  } configuration;

  explicit observer(
      libremidi::observer_configuration&& conf, kdmapi::observer_configuration&& apiconf)
      : configuration{std::move(conf), std::move(apiconf)}
  {
    if (!configuration.has_callbacks())
      return;

    auto& loader = kdmapi_loader::instance();
    if (!loader.is_available())
      return;

    // KDMAPI provides a single virtual output port
    if (configuration.notify_in_constructor)
    {
      if (configuration.output_added)
      {
        auto ports = get_output_ports();
        for (const auto& port : ports)
          configuration.output_added(port);
      }
    }
  }

  ~observer() = default;

  libremidi::API get_current_api() const noexcept override { return libremidi::API::KDMAPI; }

  std::vector<libremidi::input_port> get_input_ports() const noexcept override
  {
    // KDMAPI does not provide input ports
    return {};
  }

  std::vector<libremidi::output_port> get_output_ports() const noexcept override
  {
    auto& loader = kdmapi_loader::instance();
    if (!loader.is_available())
      return {};

    // Get KDMAPI version for display name
    std::string display_name = "OmniMIDI";
    if (loader.ReturnKDMAPIVer)
    {
      DWORD major{}, minor{}, build{}, rev{};
      if (loader.ReturnKDMAPIVer(&major, &minor, &build, &rev))
      {
        display_name += " (KDMAPI ";
        display_name += std::to_string(major);
        display_name += ".";
        display_name += std::to_string(minor);
        display_name += ".";
        display_name += std::to_string(build);
        display_name += ")";
      }
    }

    return {libremidi::output_port{
        {.api = libremidi::API::KDMAPI,
         .client = 0,
         .port = 0,
         .manufacturer = "KeppySoftware",
         .device_name = "OmniMIDI",
         .port_name = "OmniMIDI",
         .display_name = std::move(display_name)},
    }};
  }
};

}
