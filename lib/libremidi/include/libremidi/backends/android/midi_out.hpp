#pragma once
#include <libremidi/backends/android/config.hpp>
#include <libremidi/backends/android/helpers.hpp>
#include <libremidi/detail/midi_out.hpp>

NAMESPACE_LIBREMIDI
{
namespace android
{
class midi_out final
    : public midi1::out_api
    , public error_handler
{
public:
  explicit midi_out(
      libremidi::output_configuration&& conf, libremidi::android::output_configuration aconf)
      : configuration{std::move(conf)}
  {
    if (!context::client_name.empty() && context::client_name != aconf.client_name)
    {
      libremidi_handle_error(
          configuration, "Android backend only supports one client name per process");
      return;
    }
    context::client_name = std::string(aconf.client_name);
    client_open_ = stdx::error{};
  }

  ~midi_out() override { close_port(); }

  libremidi::API get_current_api() const noexcept override
  {
    return libremidi::API::ANDROID_AMIDI;
  }

  stdx::error open_port(const output_port& port, std::string_view /*name*/) override
  {
    if (is_port_open())
    {
      libremidi_handle_error(configuration, "midi_out::open_port: a port is already open");
      return std::errc{};
    }

    const unsigned int port_number = port.port;

    auto env = context::get_thread_env();
    if (!env)
    {
      libremidi_handle_error(configuration, "midi_out::open_port: failed to get JNI environment");
      return std::errc{};
    }

    auto ctx = context::get_context(env);
    if (!ctx)
    {
      libremidi_handle_error(configuration, "midi_out::open_port: failed to get Android context");
      return std::errc{};
    }

    context::refresh_midi_devices(env, ctx, true);

    if (port_number >= context::midi_devices.size())
    {
      libremidi_handle_error(configuration, "midi_out::open_port: invalid port number");
      return std::errc{};
    }

    port_open = true;
    context::open_device(context::midi_devices[port_number], this, true);
    return stdx::error{};
  }

  stdx::error close_port() override
  {
    if (!is_port_open())
      return std::errc{};

    if (midi_input_port)
    {
      AMidiInputPort_close(midi_input_port);
      midi_input_port = nullptr;
    }

    if (send_device)
    {
      AMidiDevice_release(send_device);
      send_device = nullptr;
    }

    port_open = false;
    return stdx::error{};
  }

  bool is_port_open() const noexcept { return port_open; }

  stdx::error send_message(const unsigned char* message, size_t size) override
  {
    if (!is_port_open() || !midi_input_port)
    {
      libremidi_handle_error(configuration, "midi_out::send_message: port is not open");
      return std::errc{};
    }

    ssize_t result = AMidiInputPort_send(midi_input_port, message, size);
    if (result < 0)
    {
      libremidi_handle_error(configuration, "midi_out::send_message: failed to send MIDI data");
      return std::errc{};
    }
    return stdx::error{};
  }

  static void open_callback(midi_out* self, AMidiDevice* device)
  {
    self->send_device = device;
    AMidiInputPort_open(device, 0, &self->midi_input_port);
  }

private:
  libremidi::output_configuration configuration;
  AMidiDevice* send_device = nullptr;
  AMidiInputPort* midi_input_port = nullptr;
  bool port_open = false;
};
}
}
