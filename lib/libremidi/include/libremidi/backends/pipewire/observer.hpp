#pragma once
#include <libremidi/backends/pipewire/config.hpp>
#include <libremidi/backends/pipewire/helpers.hpp>
#include <libremidi/detail/observer.hpp>

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
    if (auto err = create_context(*this); err != stdx::error{})
      return;

    // Hold thread_loop_lock so a port arriving between subscription
    // install and the snapshot walk doesn't fire twice.
    this->ctx->with_lock([this] {
      this->add_callbacks<libremidi::API::PIPEWIRE>(
          libremidi::pipewire::media_class::midi, configuration);

      if (configuration.notify_in_constructor)
      {
        if (configuration.input_added)
          for (const auto& p : get_input_ports())
            configuration.input_added(p);

        if (configuration.output_added)
          for (const auto& p : get_output_ports())
            configuration.output_added(p);
      }
    });
  }

  libremidi::API get_current_api() const noexcept override { return libremidi::API::PIPEWIRE; }

  std::vector<libremidi::input_port> get_input_ports() const noexcept override
  {
    if (!this->ctx)
      return {};
    return get_ports<SPA_DIRECTION_OUTPUT, libremidi::API::PIPEWIRE>(
        libremidi::pipewire::media_class::midi, this->configuration, *this->ctx);
  }

  std::vector<libremidi::output_port> get_output_ports() const noexcept override
  {
    if (!this->ctx)
      return {};
    return get_ports<SPA_DIRECTION_INPUT, libremidi::API::PIPEWIRE>(
        libremidi::pipewire::media_class::midi, this->configuration, *this->ctx);
  }

  ~observer_pipewire() { destroy_context(); }
};
}
