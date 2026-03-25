#pragma once
#include <libremidi/backends/winuwp/config.hpp>
#include <libremidi/backends/winuwp/helpers.hpp>
#include <libremidi/backends/winuwp/observer.hpp>
#include <libremidi/detail/midi_out.hpp>

NAMESPACE_LIBREMIDI
{

class midi_out_winuwp final
    : public midi1::out_api
    , public error_handler
{
public:
  struct
      : output_configuration
      , winuwp_output_configuration
  {
  } configuration;

  midi_out_winuwp(output_configuration&& conf, winuwp_output_configuration&& apiconf)
      : configuration{std::move(conf), std::move(apiconf)}
  {
    winrt_init();
    this->client_open_ = stdx::error{};
  }

  ~midi_out_winuwp() override
  {
    close_port();
    this->client_open_ = std::errc::not_connected;
  }

  libremidi::API get_current_api() const noexcept override { return libremidi::API::WINDOWS_UWP; }

  stdx::error open_port(const output_port& port, std::string_view) override
  {
    const auto id = winrt::to_hstring(port.port_name);
    if (id.empty())
      return std::errc::invalid_argument;

    port_ = get(MidiOutPort::FromIdAsync(id));
    if (!bool(port_))
      return std::errc::io_error;

    return stdx::error{};
  }

  stdx::error close_port() override
  {
    if (port_)
    {
      port_.Close();
      port_ = {};
    }
    return stdx::error{};
  }

  stdx::error send_message(const unsigned char* message, size_t size) override
  {
    if (!port_)
      return std::errc::not_connected;

    InMemoryRandomAccessStream str;
    DataWriter rb(str);
    rb.WriteBytes(
        winrt::array_view<const uint8_t>{(const uint8_t*)message, (const uint8_t*)message + size});
    port_.SendBuffer(rb.DetachBuffer());

    return stdx::error{};
  }

private:
  winrt::Windows::Devices::Midi::IMidiOutPort port_{nullptr};
};

}
