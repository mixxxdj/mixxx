#pragma once
#include <libremidi/backends/coremidi_ump/config.hpp>
#include <libremidi/backends/coremidi_ump/helpers.hpp>
#include <libremidi/detail/midi_in.hpp>
#include <libremidi/detail/midi_stream_decoder.hpp>

NAMESPACE_LIBREMIDI::coremidi_ump
{

class midi_in_impl final
    : public midi2::in_api
    , public coremidi_data
    , public error_handler
{
public:
  using midi_api::client_open_;
  struct
      : ump_input_configuration
      , coremidi_ump::input_configuration
  {
  } configuration;

  midi_in_impl(ump_input_configuration&& conf, coremidi_ump::input_configuration&& apiconf)
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

  ~midi_in_impl() override
  {
    // Close a connection if it exists.
    close_port();

    if (this->endpoint)
      MIDIEndpointDispose(this->endpoint);

    close_client(*this);
  }

  libremidi::API get_current_api() const noexcept override { return libremidi::API::COREMIDI_UMP; }

  stdx::error open_port(const input_port& info, std::string_view portName) override
  {
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, false);

    auto source = locate_object(*this, info, kMIDIObjectType_Source);
    if (source == 0)
      return std::errc::invalid_argument;

    // Create our local sink
    MIDIPortRef port;

    OSStatus result = MIDIInputPortCreateWithProtocol(
        this->client, toCFString(portName).get(), kMIDIProtocol_2_0, &port,
        ^(const MIDIEventList* evtlist, void* __nullable srcConnRefCon) {
            this->midiInputCallback(evtlist, srcConnRefCon);
        });

    if (result != noErr)
    {
      close_client(*this);
      libremidi_handle_error(
          this->configuration, "error creating macOS MIDI input port: " + std::to_string(result));
      return from_osstatus(result);
    }

    // Make the connection.
    if (result = MIDIPortConnectSource(port, source, nullptr); result != noErr)
    {
      MIDIPortDispose(port);
      close_client(*this);
      libremidi_handle_error(this->configuration, "error connecting macOS MIDI input port.");
      return from_osstatus(result);
    }

    // Save our api-specific port information.
    this->port = port;
    return stdx::error{};
  }

  stdx::error open_virtual_port(std::string_view portName) override
  {
    // Create a virtual MIDI input destination.
    MIDIEndpointRef endpoint;
    OSStatus result = MIDIDestinationCreateWithProtocol(
        this->client, toCFString(portName).get(), kMIDIProtocol_2_0, &endpoint,
        ^(const MIDIEventList* evtlist, void* __nullable srcConnRefCon) {
            this->midiInputCallback(evtlist, srcConnRefCon);
        });

    if (result != noErr)
    {
      libremidi_handle_error(
          this->configuration,
          "error creating virtual macOS MIDI "
          "destination.");
      return from_osstatus(result);
    }

    // Save our api-specific connection information.
    this->endpoint = endpoint;
    return stdx::error{};
  }

  stdx::error close_port() override { return coremidi_data::close_port(); }

  timestamp absolute_timestamp() const noexcept override
  {
    return coremidi_data::time_in_nanos(LIBREMIDI_AUDIO_GET_CURRENT_HOST_TIME());
  }

  void midiInputCallback(const MIDIEventList* list, void* /*srcRef*/)
  {
    static constexpr timestamp_backend_info timestamp_info{
        .has_absolute_timestamps = true,
        .absolute_is_monotonic = false,
        .has_samples = false,
    };

    const MIDIEventPacket* packet = &list->packet[0];
    for (unsigned int i = 0; i < list->numPackets; ++i)
    {
      if (packet->wordCount > 0)
      {
        auto to_ns = [packet] { return time_in_nanos(packet->timeStamp); };
        m_processing.on_bytes_multi(
            {packet->words, packet->words + packet->wordCount},
            m_processing.timestamp<timestamp_info>(to_ns, 0));
      }

      packet = MIDIEventPacketNext(packet);
    }
  }

  midi2::input_state_machine m_processing{this->configuration};
};

}
