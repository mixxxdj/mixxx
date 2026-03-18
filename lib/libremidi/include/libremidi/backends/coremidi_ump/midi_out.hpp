#pragma once
#include <libremidi/backends/coremidi_ump/config.hpp>
#include <libremidi/backends/coremidi_ump/helpers.hpp>
#include <libremidi/cmidi2.hpp>
#include <libremidi/detail/midi_out.hpp>
#include <libremidi/detail/ump_stream.hpp>

NAMESPACE_LIBREMIDI::coremidi_ump
{
class midi_out_impl final
    : public midi2::out_api
    , public coremidi_data
    , public error_handler
{
public:
  using midi_api::client_open_;
  struct
      : libremidi::output_configuration
      , coremidi_ump::output_configuration
  {
  } configuration;

  midi_out_impl(
      libremidi::output_configuration&& conf, coremidi_ump::output_configuration&& apiconf)
      : configuration{std::move(conf), std::move(apiconf)}
  {
    if (auto result = init_client(configuration); result != noErr)
    {
      libremidi_handle_error(
          this->configuration, "error creating MIDI client object: " + std::to_string(result));
      client_open_ = from_osstatus(result);
      return;
    }

    client_open_ = stdx::error{};
  }

  ~midi_out_impl()
  {
    midi_out_impl::close_port();

    if (this->endpoint)
      MIDIEndpointDispose(this->endpoint);

    close_client(*this);
  }

  libremidi::API get_current_api() const noexcept override { return libremidi::API::COREMIDI_UMP; }

  stdx::error open_port(const output_port& info, std::string_view portName) override
  {
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, false);

    // Find where we want to send
    auto destination = locate_object(*this, info, kMIDIObjectType_Destination);
    if (destination == 0)
      return std::errc::invalid_argument;

    // Create our local source
    MIDIPortRef port;
    OSStatus result = MIDIOutputPortCreate(this->client, toCFString(portName).get(), &port);
    if (result != noErr)
    {
      close_client(*this);
      libremidi_handle_error(this->configuration, "error creating macOS MIDI output port.");
      return from_osstatus(result);
    }

    // Save our api-specific connection information.
    this->port = port;
    this->destinationId = destination;

    return stdx::error{};
  }

  stdx::error open_virtual_port(std::string_view portName) override
  {
    // Create a virtual MIDI output source.
    OSStatus result = MIDISourceCreateWithProtocol(
        this->client, toCFString(portName).get(), kMIDIProtocol_2_0, &this->endpoint);

    if (result != noErr)
    {
      this->endpoint = 0;
      libremidi_handle_error(this->configuration, "error creating macOS virtual MIDI source.");

      return from_osstatus(result);
    }

    return stdx::error{};
  }

  stdx::error close_port() override { return coremidi_data::close_port(); }

  stdx::error send_ump(const uint32_t* ump_stream, std::size_t count) override
  {
    MIDIEventList* eventList = reinterpret_cast<MIDIEventList*>(m_eventListBuffer);
    MIDIEventPacket* packet = MIDIEventListInit(eventList, kMIDIProtocol_2_0);
    const MIDITimeStamp ts = LIBREMIDI_AUDIO_GET_CURRENT_HOST_TIME();

    auto write_fun = [ts, &packet, &eventList](const uint32_t* ump, int bytes) -> std::errc {
      packet = MIDIEventListAdd(eventList, event_list_max_size, packet, ts, bytes / 4, ump);
      if (packet)
        return std::errc{0};
      else
        return std::errc::not_enough_memory;
    };

    auto realloc_fun = [this, &packet, &eventList] {
      push_event_list(eventList);
      packet = MIDIEventListInit(eventList, kMIDIProtocol_2_0);
    };

    segment_ump_stream(ump_stream, count, write_fun, realloc_fun);
    return push_event_list(eventList);
  }

  stdx::error push_event_list(MIDIEventList* eventList)
  {
    if (this->endpoint)
    {
      auto result = MIDIReceivedEventList(this->endpoint, eventList);
      if (result != noErr)
      {
        libremidi_handle_warning(
            this->configuration,
            "error sending MIDI to virtual "
            "destinations.");
        return std::errc::io_error;
      }
    }

    if (this->destinationId != 0)
    {
      auto result = MIDISendEventList(this->port, this->destinationId, eventList);
      if (result != noErr)
      {
        libremidi_handle_warning(this->configuration, "error sending MIDI message to port.");
        return std::errc::io_error;
      }
    }
    return stdx::error{};
  }

  MIDIEndpointRef destinationId{};

  static constexpr int event_list_max_size = 65535;
  unsigned char m_eventListBuffer[sizeof(MIDIEventList) + event_list_max_size];
};
}
