#pragma once
#include <libremidi/backends/winmidi/config.hpp>
#include <libremidi/backends/winmidi/helpers.hpp>
#include <libremidi/backends/winmidi/observer.hpp>
#include <libremidi/detail/midi_out.hpp>
#include <libremidi/detail/ump_stream.hpp>

NAMESPACE_LIBREMIDI::winmidi
{

class midi_out_impl final
    : public midi2::out_api
    , public error_handler
    , public winmidi_shared_data
{
public:
  struct
      : libremidi::output_configuration
      , winmidi::output_configuration
  {
  } configuration;

  midi_out_impl(libremidi::output_configuration&& conf, winmidi::output_configuration&& apiconf)
      : configuration{std::move(conf), std::move(apiconf)}
      , m_session{
            configuration.context ? *configuration.context
                                  : MidiSession::Create(to_hstring(configuration.client_name))}
  {
    this->client_open_ = stdx::error{};
  }

  ~midi_out_impl() override { close_port(); }

  libremidi::API get_current_api() const noexcept override
  {
    return libremidi::API::WINDOWS_MIDI_SERVICES;
  }

  stdx::error open_port(const output_port& port, std::string_view) override
  {
    auto device_id = get_if<std::string>(&port.device);
    if (!device_id)
      return std::errc::invalid_argument;

    auto [ep, gp] = get_port(*device_id, port.port);
    if (!ep || !gp)
      return std::errc::address_not_available;

    m_endpoint = m_session.CreateEndpointConnection(ep.EndpointDeviceId());
#if LIBREMIDI_WINMIDI_HAS_COM_EXTENSIONS
    m_endpoint.as(libremidi::IID_IMidiEndpointConnectionRaw, m_raw_endpoint.put_void());
  #endif
    m_endpoint.Open();

    return stdx::error{};
  }

  stdx::error close_port() override
  {
    if(!m_endpoint)
      return std::errc::not_connected;

    m_session.DisconnectEndpointConnection(m_endpoint.ConnectionId());
    return stdx::error{};
  }

  stdx::error send_ump(const uint32_t* message, size_t size) override
  {
    auto write_func = [this](const uint32_t* ump, int64_t bytes) -> std::errc {

#if !LIBREMIDI_WINMIDI_HAS_COM_EXTENSIONS
      MidiSendMessageResults ret{};
      switch (bytes / 4)
      {
        case 1:
          ret = m_endpoint.SendSingleMessagePacket(MidiMessage32(0, ump[0]));
          break;
        case 2:
          ret = m_endpoint.SendSingleMessagePacket(MidiMessage64(0, ump[0], ump[1]));
          break;
        case 3:
          ret = m_endpoint.SendSingleMessagePacket(MidiMessage96(0, ump[0], ump[1], ump[2]));
          break;
        case 4:
          ret = m_endpoint.SendSingleMessagePacket(
              MidiMessage128(0, ump[0], ump[1], ump[2], ump[3]));
          break;
        default:
          return std::errc::bad_message;
      }
      if (ret != MidiSendMessageResults::Succeeded)
        return std::errc::bad_message;
  #else
      HRESULT ret{};
      switch (bytes / 4)
      {
        case 1:
          assert( m_raw_endpoint->ValidateBufferHasOnlyCompleteUmps(1, (UINT32*)ump));
          ret = m_raw_endpoint->SendMidiMessagesRaw(0, 1, (UINT32*)ump);
          break;
        case 2:
          assert( m_raw_endpoint->ValidateBufferHasOnlyCompleteUmps(2, (UINT32*)ump));
          ret = m_raw_endpoint->SendMidiMessagesRaw(0, 2, (UINT32*)ump);
          break;
        case 3:
          assert( m_raw_endpoint->ValidateBufferHasOnlyCompleteUmps(3, (UINT32*)ump));
          ret = m_raw_endpoint->SendMidiMessagesRaw(0, 3, (UINT32*)ump);
          break;
        case 4:
          assert( m_raw_endpoint->ValidateBufferHasOnlyCompleteUmps(4, (UINT32*)ump));
          ret = m_raw_endpoint->SendMidiMessagesRaw(0, 4, (UINT32*)ump);
          break;
        default:
          return std::errc::bad_message;
      }
  #endif
      if(ret < 0)
        return std::errc::io_error;
      return std::errc{0};
    };

    return segment_ump_stream(message, size, write_func, []() { });
  }

private:
  MidiSession m_session;
  winrt::Microsoft::Windows::Devices::Midi2::MidiEndpointConnection m_endpoint{nullptr};
#if LIBREMIDI_WINMIDI_HAS_COM_EXTENSIONS
  winrt::impl::com_ref<IMidiEndpointConnectionRaw> m_raw_endpoint{};
#endif
};

}
