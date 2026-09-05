#pragma once
#include <libremidi/backends/android/config.hpp>
#include <libremidi/backends/android/helpers.hpp>
#include <libremidi/detail/midi_in.hpp>
#include <libremidi/detail/midi_stream_decoder.hpp>

#include <atomic>
#include <chrono>
#include <thread>

NAMESPACE_LIBREMIDI
{
namespace android
{
class midi_in
    : public midi1::in_api
    , public error_handler
{
public:
  explicit midi_in(
      libremidi::input_configuration&& conf, libremidi::android::input_configuration aconf)
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

  ~midi_in() override { close_port(); }

  libremidi::API get_current_api() const noexcept override
  {
    return libremidi::API::ANDROID_AMIDI;
  }

  stdx::error open_port(const input_port& port, std::string_view /*name*/) override
  {
    if (is_port_open())
    {
      libremidi_handle_error(configuration, "midi_in::open_port: a port is already open");
      return std::errc{};
    }

    const unsigned int port_number = port.port;
    auto env = context::get_thread_env();
    if (!env)
    {
      libremidi_handle_error(configuration, "midi_in::open_port: failed to get JNI environment");
      return std::errc{};
    }

    auto ctx = context::get_context(env);
    if (!ctx)
    {
      libremidi_handle_error(configuration, "midi_in::open_port: failed to get Android context");
      return std::errc{};
    }

    context::refresh_midi_devices(env, ctx, false);

    if (port_number >= context::midi_devices.size())
    {
      libremidi_handle_error(configuration, "midi_in::open_port: invalid port number");
      return std::errc{};
    }

    port_open = true;
    running = true;
    context::open_device(context::midi_devices[port_number], this, false);
    return stdx::error{};
  }

  stdx::error close_port() override
  {
    if (!is_port_open())
      return std::errc{};

    running = false;

    if (poll_thread.joinable())
    {
      poll_thread.join();
    }

    if (midi_output_port)
    {
      AMidiOutputPort_close(midi_output_port);
      midi_output_port = nullptr;
    }

    if (receive_device)
    {
      AMidiDevice_release(receive_device);
      receive_device = nullptr;
    }

    port_open = false;
    return stdx::error{};
  }

  bool is_port_open() const noexcept { return port_open; }

  void start_midi_thread() { poll_thread = std::thread(&midi_in::poll_midi, this); }

  timestamp absolute_timestamp() const noexcept override { return system_ns(); }
  void poll_midi()
  {
    static constexpr timestamp_backend_info timestamp_info{
        .has_absolute_timestamps = false,
        .absolute_is_monotonic = false,
        .has_samples = false,
    };

    uint8_t buffer[1024];
    int32_t op_code;
    int64_t timestamp;
    size_t num_bytes;

    while (running)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));

      ssize_t num_messages = AMidiOutputPort_receive(
          midi_output_port, &op_code, buffer, sizeof(buffer), &num_bytes, &timestamp);

      if (num_messages < 0)
      {
        LOGE("Error receiving MIDI data: %d", (int)num_messages);
        continue;
      }

      if (num_messages > 0 && num_bytes > 0)
      {
        const auto to_ns = [=] { return timestamp; };

        m_processing.on_bytes_multi(
            {buffer, buffer + num_bytes},
            m_processing.timestamp<timestamp_info>(to_ns, timestamp));
      }
    }
  }

  static void open_callback(midi_in* self, AMidiDevice* device)
  {
    self->receive_device = device;
    AMidiOutputPort_open(device, 0, &self->midi_output_port);
    self->start_midi_thread();
  }

private:
  libremidi::input_configuration configuration;
  AMidiDevice* receive_device = nullptr;
  AMidiOutputPort* midi_output_port = nullptr;
  std::thread poll_thread;
  std::atomic<bool> port_open{false};
  std::atomic<bool> running{false};
  midi1::input_state_machine m_processing{this->configuration};
};
}
}
