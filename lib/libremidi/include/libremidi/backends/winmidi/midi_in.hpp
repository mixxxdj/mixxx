#pragma once
#include <libremidi/backends/winmidi/config.hpp>
#include <libremidi/backends/winmidi/helpers.hpp>
#include <libremidi/backends/winmidi/observer.hpp>
#include <libremidi/detail/midi_in.hpp>
#include <libremidi/detail/midi_stream_decoder.hpp>

NAMESPACE_LIBREMIDI::winmidi
{
class midi_in_impl final
    : public midi2::in_api
    , public error_handler
    , public winmidi_shared_data
{
public:
  struct
      : libremidi::ump_input_configuration
      , winmidi::input_configuration
  {
  } configuration;

#if LIBREMIDI_WINMIDI_HAS_COM_EXTENSIONS
  struct raw_callback_type final : IMidiEndpointConnectionMessagesReceivedCallback
  {
    explicit raw_callback_type(midi_in_impl& self)
        : self{self}
    {

    }

    midi_in_impl& self;
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override
    {
      if (!ppvObject)
        return E_POINTER;

      if (riid == __uuidof(IUnknown) ||
          riid == libremidi::IID_IMidiEndpointConnectionMessagesReceivedCallback)
      {
        *ppvObject = static_cast<IMidiEndpointConnectionMessagesReceivedCallback*>(this);
        AddRef();
        return S_OK;
      }

      *ppvObject = nullptr;
      return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() override
    {
      return 1;
    }

    ULONG STDMETHODCALLTYPE Release() override
    {
      return 1;
    }

    HRESULT STDMETHODCALLTYPE MessagesReceived(
        GUID sessionId,
        GUID connectionId,
        UINT64 timestamp,
        UINT32 wordCount,
        UINT32 *messages) override {
      HRESULT res{};
      self.process_message(sessionId, connectionId, timestamp, wordCount, messages);
      return res;
    }
  } raw_callback{*this};
#endif

  explicit midi_in_impl(
      libremidi::ump_input_configuration&& conf, winmidi::input_configuration&& apiconf)
      : configuration{std::move(conf), std::move(apiconf)}
      , m_session{
            configuration.context ? *configuration.context
                                  : MidiSession::Create(to_hstring(configuration.client_name))}
  {
    this->client_open_ = stdx::error{};
  }

  ~midi_in_impl() override
  {
    close_port();
    this->client_open_ = std::errc::not_connected;
  }

  libremidi::API get_current_api() const noexcept override
  {
    return libremidi::API::WINDOWS_MIDI_SERVICES;
  }

  stdx::error open_port(const input_port& port, std::string_view) override
  {
    auto device_id = get_if<std::string>(&port.device);
    if (!device_id)
      return std::errc::invalid_argument;

    auto [ep, gp] = get_port(*device_id, port.port);
    if (!ep || !gp)
      return std::errc::address_not_available;

    // port.port is the Group Terminal Block Number(), which is not necessarily equal to
    // the UMP group index the block spans. For example a USB MIDI 1.0 device may expose
    // its input GTB as Number()==2 while its messages arrive on group 0. Using
    // `port.port - 1` as the group filter therefore silently drops all input from such
    // devices. Filter on the resolved block's actual first group instead.
    m_group_filter = gp.FirstGroup().Index();

    try
    {
      // TODO use a MidiGroupEndpointListener for the filtering
      m_endpoint = m_session.CreateEndpointConnection(ep.EndpointDeviceId());
      if (!m_endpoint)
        return std::errc::device_or_resource_busy;

  #if !LIBREMIDI_WINMIDI_HAS_COM_EXTENSIONS
      m_revoke_token = m_endpoint.MessageReceived(
          [this](
              const winrt::Microsoft::Windows::Devices::Midi2::IMidiMessageReceivedEventSource&,
              const winrt::Microsoft::Windows::Devices::Midi2::MidiMessageReceivedEventArgs& args) {
        process_message(args);
      });
  #else
      m_endpoint.as(libremidi::IID_IMidiEndpointConnectionRaw, m_raw_endpoint.put_void());

      m_raw_endpoint->SetMessagesReceivedCallback(
          &raw_callback
      );
  #endif

      m_endpoint.Open();

      return stdx::error{};
    }
    catch (...)
    {
      return std::errc::io_error;
    }
  }

#if LIBREMIDI_WINMIDI_HAS_VIRTUAL_DEVICE
  stdx::error open_virtual_port(std::string_view port_name) override
  {
    // Create endpoint information for the virtual device
    using namespace winrt::Microsoft::Windows::Devices::Midi2;
    using namespace winrt::Microsoft::Windows::Devices::Midi2::Endpoints::Virtual;

    auto conf = setup_virtualdevice_config(configuration.client_name, port_name, port_name, MidiFunctionBlockDirection::BlockInput);

    m_virtual = MidiVirtualDeviceManager::CreateVirtualDevice(conf);
    if (m_virtual == nullptr)
      return std::errc::device_or_resource_busy;

    try
    {
      // Create a connection to the device-side endpoint
      m_endpoint = m_session.CreateEndpointConnection(m_virtual.DeviceEndpointDeviceId());
      if (!m_endpoint)
        return std::errc::device_or_resource_busy;

      // Add the virtual device as a message processing plugin to receive messages
      m_endpoint.AddMessageProcessingPlugin(m_virtual);

      // Register message received event handler
      m_revoke_token = m_endpoint.MessageReceived(
          [this](
              const winrt::Microsoft::Windows::Devices::Midi2::IMidiMessageReceivedEventSource&,
              const winrt::Microsoft::Windows::Devices::Midi2::MidiMessageReceivedEventArgs& args) {
        process_message(args);
      });

      m_endpoint.Open();

      return stdx::error{};
    }
    catch (...)
    {
      return std::errc::io_error;
    }
  }
#endif

  void process_message(
      const winrt::Microsoft::Windows::Devices::Midi2::MidiMessageReceivedEventArgs& msg)
  {
    static constexpr timestamp_backend_info timestamp_info{
        .has_absolute_timestamps = true,
        .absolute_is_monotonic = false,
        .has_samples = false,
    };

    const auto& ump = msg.GetMessagePacket();
    auto pk = msg.PeekFirstWord();
    if (m_group_filter >= 0)
    {
      int group = cmidi2_ump_get_group(&pk);
      if (group != m_group_filter)
        return;
    }

    const auto& b = ump.GetAllWords();

    uint32_t ump_space[64];
    array_view<uint32_t> ref{ump_space};
    b.GetMany(0, ref);

    auto to_ns = [t = ump.Timestamp()] { return t; };
    m_processing.on_bytes(
        {ump_space, ump_space + b.Size()}, m_processing.timestamp<timestamp_info>(to_ns, 0));
  }

#if LIBREMIDI_WINMIDI_HAS_COM_EXTENSIONS
  void process_message(
      const GUID& /* sessionId */,
      const GUID& /* connectionId */,
      UINT64 timestamp,
      UINT32 wordCount,
      UINT32 *ump)
  {
    static constexpr timestamp_backend_info timestamp_info{
        .has_absolute_timestamps = true,
        .absolute_is_monotonic = false,
        .has_samples = false,
    };

    if(wordCount == 0)
      return;

    if (m_group_filter >= 0)
    {
      int group = cmidi2_ump_get_group(ump);
      if (group != m_group_filter)
        return;
    }

    auto to_ns = [t = timestamp] { return t; };
    m_processing.on_bytes(
        {ump, ump + wordCount}, m_processing.timestamp<timestamp_info>(to_ns, 0));
  }
#endif

  stdx::error close_port() override
  {
    if(!m_endpoint)
      return std::errc::not_connected;

#if LIBREMIDI_WINMIDI_HAS_COM_EXTENSIONS
    if(m_raw_endpoint) {
      m_raw_endpoint->RemoveMessagesReceivedCallback();
      m_raw_endpoint = nullptr;
    }
    // When no raw API: everything goes through revoke_token.
    // Otherwise: only virtual ports.
    else if(m_virtual)
#endif
      m_endpoint.MessageReceived(m_revoke_token);

    m_session.DisconnectEndpointConnection(m_endpoint.ConnectionId());

#if LIBREMIDI_WINMIDI_HAS_VIRTUAL_DEVICE
    if (m_virtual)
    {
      m_virtual.Cleanup();
      m_virtual = nullptr;
    }
  #endif
    return stdx::error{};
  }

  virtual timestamp absolute_timestamp() const noexcept override { return {}; }

private:
  MidiSession m_session;
  winrt::event_token m_revoke_token{};
  winrt::Microsoft::Windows::Devices::Midi2::MidiEndpointConnection m_endpoint{nullptr};
#if LIBREMIDI_WINMIDI_HAS_COM_EXTENSIONS
  winrt::impl::com_ref<IMidiEndpointConnectionRaw> m_raw_endpoint{};
#endif
#if LIBREMIDI_WINMIDI_HAS_VIRTUAL_DEVICE
  winrt::Microsoft::Windows::Devices::Midi2::Endpoints::Virtual::MidiVirtualDevice m_virtual{nullptr};
#endif
  midi2::input_state_machine m_processing{this->configuration};
  int m_group_filter = -1;
};
}
