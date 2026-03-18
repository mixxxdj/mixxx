#pragma once
#include <libremidi/backends/pipewire/config.hpp>
#include <libremidi/backends/pipewire/helpers.hpp>
#include <libremidi/detail/observer.hpp>

#include <unordered_set>

NAMESPACE_LIBREMIDI
{
class observer_pipewire final
    : public observer_api
    , private pipewire_helpers
    , private error_handler
{
public:
  struct
      : observer_configuration
      , pipewire_observer_configuration
  {
  } configuration;

  explicit observer_pipewire(
      observer_configuration&& conf, pipewire_observer_configuration&& apiconf)
      : configuration{std::move(conf), std::move(apiconf)}
  {
    create_context(*this);

    // FIXME port rename callback
    {
      this->add_callbacks<libremidi::API::PIPEWIRE>("midi", configuration);
      this->start_thread();
    }

    if (configuration.notify_in_constructor)
    {
      if (configuration.input_added)
        for (const auto& p : get_input_ports())
          configuration.input_added(p);

      if (configuration.output_added)
        for (const auto& p : get_output_ports())
          configuration.output_added(p);
    }
  }

  libremidi::API get_current_api() const noexcept override { return libremidi::API::PIPEWIRE; }

  std::vector<libremidi::input_port> get_input_ports() const noexcept override
  {
    return get_ports<SPA_DIRECTION_OUTPUT, libremidi::API::PIPEWIRE>(
        "midi", this->configuration, *this->global_context);
  }

  std::vector<libremidi::output_port> get_output_ports() const noexcept override
  {
    return get_ports<SPA_DIRECTION_INPUT, libremidi::API::PIPEWIRE>(
        "midi", this->configuration, *this->global_context);
  }

  ~observer_pipewire()
  {
    stop_thread();
    destroy_context();
  }

  std::unordered_set<std::string> seen_input_ports;
  std::unordered_set<std::string> seen_output_ports;
};
}
