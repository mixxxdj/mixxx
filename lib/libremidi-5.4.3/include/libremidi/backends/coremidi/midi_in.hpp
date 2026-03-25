#pragma once
#include <libremidi/backends/coremidi/config.hpp>
#include <libremidi/backends/coremidi/helpers.hpp>
#include <libremidi/detail/midi_in.hpp>
#include <libremidi/detail/midi_stream_decoder.hpp>

NAMESPACE_LIBREMIDI
{
class midi_in_core final
    : public midi1::in_api
    , public coremidi_data
    , public error_handler
{
public:
  using midi_api::client_open_;
  struct
      : input_configuration
      , coremidi_input_configuration
  {
  } configuration;

  midi_in_core(input_configuration&& conf, coremidi_input_configuration&& apiconf)
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

  ~midi_in_core() override
  {
    // Close a connection if it exists.
    midi_in_core::close_port();

    if (this->endpoint)
      MIDIEndpointDispose(this->endpoint);

    close_client(*this);
  }

  libremidi::API get_current_api() const noexcept override { return libremidi::API::COREMIDI; }

  stdx::error open_port(const input_port& info, std::string_view portName) override
  {
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, false);

    auto source = locate_object(*this, info, kMIDIObjectType_Source);
    if (source == 0)
      return std::errc::invalid_argument;

    // Create our local sink
    MIDIPortRef port;
    OSStatus result = MIDIInputPortCreate(
        this->client, toCFString(portName).get(), midiInputCallback, (void*)this, &port);

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
    OSStatus result = MIDIDestinationCreate(
        this->client, toCFString(portName).get(), midiInputCallback, (void*)this, &endpoint);

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

  static void midiInputCallback(const MIDIPacketList* list, void* procRef, void* /*srcRef*/)
  {
    static constexpr timestamp_backend_info timestamp_info{
        .has_absolute_timestamps = true,
        .absolute_is_monotonic = false,
        .has_samples = false,
    };

    auto& self = *(midi_in_core*)procRef;

    const MIDIPacket* packet = &list->packet[0];
    for (unsigned int i = 0; i < list->numPackets; ++i)
    {
      // My interpretation of the CoreMIDI documentation: all message
      // types, except sysex, are complete within a packet and there may
      // be several of them in a single packet.  Sysex messages can be
      // broken across multiple packets and PacketLists but are bundled
      // alone within each packet (these packets do not contain other
      // message types).  If sysex messages are split across multiple
      // MIDIPacketLists, they must be handled by multiple calls to this
      // function.

      if (packet->length > 0)
      {
        auto to_ns = [packet] { return time_in_nanos(packet->timeStamp); };
        self.m_processing.on_bytes_multi(
            {packet->data, packet->data + packet->length},
            self.m_processing.timestamp<timestamp_info>(to_ns, 0));
      }

      packet = MIDIPacketNext(packet);
    }
  }

  midi1::input_state_machine m_processing{this->configuration};
};
}
