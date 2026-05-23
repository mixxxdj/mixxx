#pragma once
#include <libremidi/backends/android/config.hpp>
#include <libremidi/backends/android/helpers.hpp>
#include <libremidi/detail/observer.hpp>

NAMESPACE_LIBREMIDI
{
namespace android
{
class observer final
    : public libremidi::observer_api
    , public error_handler
{
public:
  explicit observer(
      libremidi::observer_configuration&& conf, libremidi::android::observer_configuration aconf)
      : configuration{std::move(conf)}
  {
    if (!context::client_name.empty() && context::client_name != aconf.client_name)
    {
      LOGW("Android backend only supports one client name per process");
    }
    context::client_name = std::string(aconf.client_name);
  }

  ~observer()
  {
    auto env = context::get_thread_env();
    if (env)
    {
      context::cleanup_devices(env);
    }
  }

  libremidi::API get_current_api() const noexcept override
  {
    return libremidi::API::ANDROID_AMIDI;
  }

  std::vector<libremidi::input_port> get_input_ports() const noexcept override
  {
    auto env = context::get_thread_env();
    if (!env)
      return {};

    auto ctx = context::get_context(env);
    if (!ctx)
      return {};

    context::refresh_midi_devices(env, ctx, false);

    std::vector<libremidi::input_port> ports;
    for (size_t i = 0; i < context::midi_devices.size(); ++i)
    {
      libremidi::input_port port;
      port.api = libremidi::API::ANDROID_AMIDI;
      port.port_name = context::port_name(env, i);
      port.port = i;
      ports.push_back(std::move(port));
    }

    return ports;
  }

  std::vector<libremidi::output_port> get_output_ports() const noexcept override
  {
    auto env = context::get_thread_env();
    if (!env)
      return {};

    auto ctx = context::get_context(env);
    if (!ctx)
      return {};

    context::refresh_midi_devices(env, ctx, true);

    std::vector<libremidi::output_port> ports;
    for (size_t i = 0; i < context::midi_devices.size(); ++i)
    {
      libremidi::output_port port;
      port.port_name = context::port_name(env, i);
      port.port = i;
      ports.push_back(std::move(port));
    }

    return ports;
  }

private:
  libremidi::observer_configuration configuration;
};
}
}
