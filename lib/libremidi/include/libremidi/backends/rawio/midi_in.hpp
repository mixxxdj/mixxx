#pragma once
#include <libremidi/backends/rawio/config.hpp>
#include <libremidi/detail/midi_in.hpp>
#include <libremidi/detail/midi_stream_decoder.hpp>

#include <chrono>

NAMESPACE_LIBREMIDI::rawio
{
class midi_in final
    : public midi1::in_api
    , public error_handler
{
public:
  using midi_api::client_open_;
  struct
      : input_configuration
      , rawio_input_configuration
  {
  } configuration;

  explicit midi_in(input_configuration&& conf, rawio_input_configuration&& apiconf)
      : configuration{std::move(conf), std::move(apiconf)}
  {
    client_open_ = stdx::error{};
  }

  ~midi_in() override { close_port(); }

  libremidi::API get_current_api() const noexcept override { return libremidi::API::RAW_IO; }

  stdx::error open_port(const input_port&, std::string_view) override
  {
    return open_virtual_port({});
  }

  stdx::error open_virtual_port(std::string_view) override
  {
    if (!configuration.set_receive_callback)
      return std::errc::function_not_supported;

    configuration.set_receive_callback(
        [this](std::span<const uint8_t> bytes, int64_t timestamp) {
      on_bytes(bytes, timestamp);
    });

    return stdx::error{};
  }

  stdx::error close_port() override
  {
    if (configuration.stop_receive)
      configuration.stop_receive();
    // Clear all callbacks to break reference cycles (prevents leaks in binding layers)
    configuration.set_receive_callback = nullptr;
    configuration.stop_receive = nullptr;
    return stdx::error{};
  }

  stdx::error set_port_name(std::string_view) override { return stdx::error{}; }

  timestamp absolute_timestamp() const noexcept override
  {
    return std::chrono::steady_clock::now().time_since_epoch().count();
  }

private:
  void on_bytes(std::span<const uint8_t> bytes, int64_t ts)
  {
    static constexpr timestamp_backend_info timestamp_info{
        .has_absolute_timestamps = true,
        .absolute_is_monotonic = false,
        .has_samples = false,
    };

    const auto to_ns = [ts]() { return ts; };

    m_processing.on_bytes_multi(
        bytes, m_processing.timestamp<timestamp_info>(to_ns, 0));
  }

  midi1::input_state_machine m_processing{this->configuration};
};
}
