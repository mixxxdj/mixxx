#pragma once
#include <libremidi/backends/winuwp/config.hpp>
#include <libremidi/backends/winuwp/helpers.hpp>
#include <libremidi/backends/winuwp/observer.hpp>
#include <libremidi/detail/midi_in.hpp>
#include <libremidi/detail/midi_stream_decoder.hpp>

NAMESPACE_LIBREMIDI
{

class midi_in_winuwp final
    : public midi1::in_api
    , public error_handler
{
public:
  struct
      : input_configuration
      , winuwp_input_configuration
  {
  } configuration;

  explicit midi_in_winuwp(input_configuration&& conf, winuwp_input_configuration&& apiconf)
      : configuration{std::move(conf), std::move(apiconf)}
  {
    winrt_init();
    this->client_open_ = stdx::error{};
  }

  ~midi_in_winuwp() override
  {
    close_port();
    this->client_open_ = std::errc::not_connected;
  }

  libremidi::API get_current_api() const noexcept override { return libremidi::API::WINDOWS_UWP; }

  stdx::error open_port(const input_port& port, std::string_view) override
  {
    const auto id = winrt::to_hstring(port.port_name);
    if (id.empty())
      return std::errc::invalid_argument;

    port_ = get(MidiInPort::FromIdAsync(id));
    if (!port_)
      return std::errc::io_error;

    midi_start_timestamp = std::chrono::steady_clock::now();

    port_.MessageReceived(
        [this](
            const winrt::Windows::Devices::Midi::IMidiInPort&,
            const winrt::Windows::Devices::Midi::MidiMessageReceivedEventArgs& args) {
      this->process_message(args.Message());
    });

    return stdx::error{};
  }

  void process_message(const winrt::Windows::Devices::Midi::IMidiMessage& msg)
  {
    static constexpr timestamp_backend_info timestamp_info{
        .has_absolute_timestamps = true,
        .absolute_is_monotonic = false,
        .has_samples = false,
    };

    auto reader = DataReader::FromBuffer(msg.RawData());
    auto begin = msg.RawData().data();
    auto end = begin + msg.RawData().Length();

    const auto to_ns = [&msg] { return msg.Timestamp().count() * 1'000'000; };
    m_processing.on_bytes({begin, end}, m_processing.timestamp<timestamp_info>(to_ns, 0));
  }

  stdx::error close_port() override
  {
    if (port_)
    {
      port_.Close();
      port_ = nullptr;
    }
    return stdx::error{};
  }

  timestamp absolute_timestamp() const noexcept override
  {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
               std::chrono::steady_clock::now() - midi_start_timestamp)
        .count();
  }

private:
  winrt::Windows::Devices::Midi::IMidiInPort port_{nullptr};
  std::chrono::steady_clock::time_point midi_start_timestamp;

  midi1::input_state_machine m_processing{this->configuration};
};
}
