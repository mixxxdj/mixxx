#pragma once
#include <libremidi/backends/winmidi/config.hpp>
#include <libremidi/backends/winmidi/helpers.hpp>
#include <libremidi/detail/observer.hpp>

NAMESPACE_LIBREMIDI::winmidi
{
struct port_info
{
  winrt::hstring id;
  winrt::hstring name;
};

class observer_impl final
    : public observer_api
    , public winmidi_shared_data
{
public:
  struct
      : libremidi::observer_configuration
      , winmidi::observer_configuration
  {
  } configuration;

  MidiEndpointDeviceWatcher watcher = nullptr;
  MidiEndpointDeviceWatcher::Added_revoker m_addHandler;
  MidiEndpointDeviceWatcher::Updated_revoker m_updHandler;
  MidiEndpointDeviceWatcher::Removed_revoker m_delHandler;
  std::map<winrt::hstring, std::vector<input_port>> m_known_input_devices;
  std::map<winrt::hstring, std::vector<output_port>> m_known_output_devices;
  std::mutex m_devices_mtx;
  std::atomic_bool m_in_constructor{true};

  explicit observer_impl(
      libremidi::observer_configuration&& conf, winmidi::observer_configuration&& apiconf)
      : configuration{std::move(conf), std::move(apiconf)}
  {
    struct on_delete { observer_impl& self; ~on_delete() { self.m_in_constructor = false; } } on_delete{*this};

    if (!configuration.has_callbacks())
      return;

    // Note: winmidi also notifies for existing devices, so the "notify in constructor"
    // is handled in the callbacks
    if ((watcher = MidiEndpointDeviceWatcher::Create()))
    {
      namespace enumeration = winrt::Windows::Devices::Enumeration;
      auto addHandler = foundation::TypedEventHandler<
          MidiEndpointDeviceWatcher, MidiEndpointDeviceInformationAddedEventArgs>(
          this, &observer_impl::on_device_added);
      auto updHandler = foundation::TypedEventHandler<
          MidiEndpointDeviceWatcher, MidiEndpointDeviceInformationUpdatedEventArgs>(
          this, &observer_impl::on_device_updated);
      auto delHandler = foundation::TypedEventHandler<
          MidiEndpointDeviceWatcher, MidiEndpointDeviceInformationRemovedEventArgs>(
          this, &observer_impl::on_device_removed);

      m_addHandler = watcher.Added(winrt::auto_revoke, addHandler);
      m_updHandler = watcher.Updated(winrt::auto_revoke, updHandler);
      m_delHandler = watcher.Removed(winrt::auto_revoke, delHandler);

      watcher.Start();
    }
  }

  ~observer_impl()
  {
    if (!configuration.has_callbacks())
      return;

    m_addHandler.revoke();
    m_updHandler.revoke();
    m_delHandler.revoke();
  }

  libremidi::API get_current_api() const noexcept override
  {
    return libremidi::API::WINDOWS_MIDI_SERVICES;
  }

  static transport_type code_to_type(std::string_view str) noexcept
  {
    using enum transport_type;

    if (str.starts_with("KS"))
      return hardware;
    if (str == "BLE")
      return transport_type(hardware | bluetooth);
    if (str == "VPB" || str == "APP")
      return software;
    if (str == "LOOP")
      return transport_type(software | loopback);
    if (str.starts_with("NET"))
      return network;
    return unknown;
  }

  template <bool Input>
  auto to_port_info(const MidiEndpointDeviceInformation& p, const MidiGroupTerminalBlock& gp)
      const noexcept -> std::conditional_t<Input, input_port, output_port>
  {
    const auto& tinfo = p.GetTransportSuppliedInfo();

    return {
        {.api = libremidi::API::WINDOWS_MIDI_SERVICES,
         .client = 0,
         .container = std::bit_cast<libremidi::uuid>(p.ContainerId()),
         .device = to_string(p.EndpointDeviceId()),
         .port = gp.Number(),
         .manufacturer = to_string(tinfo.ManufacturerName),
         .product = to_string(tinfo.Name),
         .serial = to_string(tinfo.SerialNumber),
         .device_name = to_string(p.Name()),
         .port_name = to_string(gp.Name()),
         .display_name = to_string(gp.Name()) + " " + std::to_string(gp.Number()),
         .type = code_to_type(to_string(p.GetTransportSuppliedInfo().TransportCode))}};
  }

  std::vector<libremidi::input_port> get_input_ports() const noexcept override
  {
    std::vector<libremidi::input_port> ret;

    for (const auto& ep : MidiEndpointDeviceInformation::FindAll())
    {
      if (ep.Name().starts_with(L"Diagnostics"))
      {
        continue;
      }

      for (const auto& gp : ep.GetGroupTerminalBlocks())
      {
        if (gp.Direction() != MidiGroupTerminalBlockDirection::BlockOutput)
          ret.emplace_back(to_port_info<true>(ep, gp));
      }
    }

    return ret;
  }

  std::vector<libremidi::output_port> get_output_ports() const noexcept override
  {
    std::vector<libremidi::output_port> ret;

    for (const auto& ep : MidiEndpointDeviceInformation::FindAll())
    {
      if (ep.Name().starts_with(L"Diagnostics"))
      {
        continue;
      }

      for (const auto& gp : ep.GetGroupTerminalBlocks())
      {
        if (gp.Direction() != MidiGroupTerminalBlockDirection::BlockInput)
          ret.emplace_back(to_port_info<false>(ep, gp));
      }
    }

    return ret;
  }

  // Note: these callbacks are called from some random thread!
  void on_device_added(
      const MidiEndpointDeviceWatcher&, const MidiEndpointDeviceInformationAddedEventArgs& result)
  {
    add_device(result.AddedDevice());
  }

  void on_device_updated(
      const MidiEndpointDeviceWatcher&,
      const MidiEndpointDeviceInformationUpdatedEventArgs& result)
  {
    // OPTIMIZEME
    remove_device(result.EndpointDeviceId());

    add_device(
        MidiEndpointDeviceInformation::CreateFromEndpointDeviceId(result.EndpointDeviceId()));
  }

  void on_device_removed(
      const MidiEndpointDeviceWatcher&,
      const MidiEndpointDeviceInformationRemovedEventArgs& result)
  {
    remove_device(result.EndpointDeviceId());
  }

  void add_device(const MidiEndpointDeviceInformation& ep)
  {
    for (const auto& gp : ep.GetGroupTerminalBlocks())
    {
      MidiGroupTerminalBlockDirection direction = gp.Direction();
      switch (direction)
      {
        case MidiGroupTerminalBlockDirection::Bidirectional: {
          if (configuration.input_added)
          {
            auto ip = to_port_info<true>(ep, gp);
            {
              std::lock_guard _{m_devices_mtx};
              m_known_input_devices[ep.EndpointDeviceId()].push_back(ip);
            }

            if(!m_in_constructor || configuration.notify_in_constructor)
              configuration.input_added(std::move(ip));
          }
          if (configuration.output_added)
          {
            auto op = to_port_info<false>(ep, gp);
            {
              std::lock_guard _{m_devices_mtx};
              m_known_output_devices[ep.EndpointDeviceId()].push_back(op);
            }

            if(!m_in_constructor || configuration.notify_in_constructor)
              configuration.output_added(std::move(op));
          }
          break;
        }
        case MidiGroupTerminalBlockDirection::BlockInput:
          if (configuration.input_added)
          {
            auto ip = to_port_info<true>(ep, gp);
            {
              std::lock_guard _{m_devices_mtx};
              m_known_input_devices[ep.EndpointDeviceId()].push_back(ip);
            }

            if(!m_in_constructor || configuration.notify_in_constructor)
              configuration.input_added(std::move(ip));
          }
          break;
        case MidiGroupTerminalBlockDirection::BlockOutput:
          if (configuration.output_added)
          {
            auto op = to_port_info<false>(ep, gp);
            {
              std::lock_guard _{m_devices_mtx};
              m_known_output_devices[ep.EndpointDeviceId()].push_back(op);
            }

            if(!m_in_constructor || configuration.notify_in_constructor)
              configuration.output_added(std::move(op));
          }
          break;
      }
    }
  }

  void remove_device(winrt::hstring eid)
  {
    std::vector<input_port> to_remove_in;
    std::vector<output_port> to_remove_out;

    {
      std::lock_guard _{m_devices_mtx};
      if (auto it = m_known_input_devices.find(eid); it != m_known_input_devices.end())
      {
        for (auto& ip : it->second)
        {
          to_remove_in.push_back(ip);
        }
        m_known_input_devices.erase(it);
      }
      if (auto it = m_known_output_devices.find(eid); it != m_known_output_devices.end())
      {
        for (auto& op : it->second)
        {
          to_remove_out.push_back(op);
        }
        m_known_output_devices.erase(it);
      }
    }

    if (configuration.input_removed)
      for (auto& port : to_remove_in)
        configuration.input_removed(port);
    if (configuration.output_removed)
      for (auto& port : to_remove_out)
        configuration.output_removed(port);
  }
};

}
