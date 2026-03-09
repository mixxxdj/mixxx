#pragma once
#include <libremidi/backends/coremidi/config.hpp>
#include <libremidi/backends/coremidi/helpers.hpp>
#include <libremidi/detail/observer.hpp>

#include <CoreMIDI/CoreMIDI.h>

#include <bit>

NAMESPACE_LIBREMIDI
{
class observer_core
    : public observer_api
    , public error_handler
{
public:
  struct
      : observer_configuration
      , coremidi_observer_configuration
  {
  } configuration;

  explicit observer_core(observer_configuration&& conf, coremidi_observer_configuration&& apiconf)
      : configuration{std::move(conf), std::move(apiconf)}
  {
  }

  void finish_init()
  {
    if (configuration.client_name.empty())
      configuration.client_name = "libremidi observer";

    if (!configuration.has_callbacks())
      return;

    auto result = MIDIClientCreate(
        toCFString(configuration.client_name).get(),
        +[](const MIDINotification* message, void* ctx) {
      ((observer_core*)ctx)->notify(message);
    }, this, &client);

    if (result != noErr)
    {
      libremidi_handle_error(
          this->configuration, "error creating MIDI client object: " + std::to_string(result));
      return;
    }

    if (configuration.on_create_context)
      configuration.on_create_context(client);

    if (configuration.notify_in_constructor)
    {
      if (this->configuration.input_added)
        for (auto& p : get_input_ports())
          this->configuration.input_added(p);

      if (this->configuration.output_added)
        for (auto& p : get_output_ports())
          this->configuration.output_added(p);
    }
  }

  libremidi::API get_current_api() const noexcept override { return libremidi::API::COREMIDI; }

  template <bool Input>
  auto to_port_info(MIDIObjectRef obj) const noexcept
      -> std::optional<std::conditional_t<Input, input_port, output_port>>
  {
    MIDIEntityRef e{};
    MIDIEndpointGetEntity(obj, &e);
    bool physical = bool(e);

    bool ok = this->configuration.track_any;
    if (physical && this->configuration.track_hardware)
      ok |= true;
    else if ((!physical) && this->configuration.track_virtual)
      ok |= true;

    if (!ok)
      return {};

    // Get the MIDI device from the entity
    libremidi::transport_type type{};
    libremidi::container_identifier usb_location_id{};
    libremidi::device_identifier usb_vendor_product{};
    {
      MIDIDeviceRef device{};
      if (MIDIEntityGetDevice(e, &device) == noErr)
      {
        if (device)
        {
          using enum libremidi::transport_type;
          SInt32 devid_res{};
          if (MIDIObjectGetIntegerProperty(device, CFSTR("USBLocationID"), &devid_res) == noErr)
          {
            type = (libremidi::transport_type)(hardware | usb);
            usb_location_id = (uint64_t)devid_res;
          }

          SInt32 vendor_res{};
          if (MIDIObjectGetIntegerProperty(device, CFSTR("USBVendorProduct"), &vendor_res)
              == noErr)
          {
            type = (libremidi::transport_type)(hardware | usb);
            usb_vendor_product = (uint64_t)vendor_res;
          }

          const auto driver = get_string_property(device, kMIDIPropertyDriverOwner);
          if (driver == "com.apple.AppleMIDIUSBDriver")
            type = (libremidi::transport_type)(hardware | usb);
          if (driver == "com.apple.AppleMIDIBluetoothDriver")
            type = (libremidi::transport_type)(hardware | bluetooth);
          if (driver == "com.apple.AppleMIDIIACDriver")
            type = (libremidi::transport_type)(software);
          if (driver == "com.apple.AppleMIDIRTPDriver")
            type = (libremidi::transport_type)(network);
        }
      }
    }

    return std::conditional_t<Input, input_port, output_port>{
        {.api = get_current_api(),
         .client = (std::uintptr_t)this->client,
         .container = usb_location_id,
         .device = usb_vendor_product,
         .port = std::bit_cast<uint32_t>(get_int_property(obj, kMIDIPropertyUniqueID)),
         .manufacturer = get_string_property(obj, kMIDIPropertyManufacturer),
         .device_name = get_string_property(obj, kMIDIPropertyModel),
         .port_name = get_string_property(obj, kMIDIPropertyName),
         .display_name = get_string_property(obj, kMIDIPropertyDisplayName),
         .type = type}};
  }

  std::vector<libremidi::input_port> get_input_ports() const noexcept override
  {
    std::vector<libremidi::input_port> ret;

    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, false);
    for (ItemCount i = 0; i < MIDIGetNumberOfSources(); i++)
    {
      if (auto p = to_port_info<true>(MIDIGetSource(i)))
        ret.push_back(std::move(*p));
    }

    return ret;
  }

  std::vector<libremidi::output_port> get_output_ports() const noexcept override
  {
    std::vector<libremidi::output_port> ret;

    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, false);
    for (ItemCount i = 0; i < MIDIGetNumberOfDestinations(); i++)
    {
      if (auto p = to_port_info<false>(MIDIGetDestination(i)))
        ret.push_back(std::move(*p));
    }
    return ret;
  }

  void notify(const MIDINotification* message)
  {
    switch (message->messageID)
    {
      case kMIDIMsgObjectAdded: {
        auto obj = reinterpret_cast<const MIDIObjectAddRemoveNotification*>(message);

        switch (obj->childType)
        {
          case kMIDIObjectType_Source:
            if (auto& cb = configuration.input_added)
              if (auto p = to_port_info<true>(obj->child))
                cb(std::move(*p));
            break;
          case kMIDIObjectType_Destination:
            if (auto& cb = configuration.output_added)
              if (auto p = to_port_info<false>(obj->child))
                cb(std::move(*p));
            break;
          default:
            break;
        }

        break;
      }

      case kMIDIMsgObjectRemoved: {
        auto obj = reinterpret_cast<const MIDIObjectAddRemoveNotification*>(message);

        switch (obj->childType)
        {
          case kMIDIObjectType_Source:
            if (auto& cb = configuration.input_removed)
              if (auto p = to_port_info<true>(obj->child))
                cb(std::move(*p));
            break;
          case kMIDIObjectType_Destination:
            if (auto& cb = configuration.output_removed)
              if (auto p = to_port_info<false>(obj->child))
                cb(std::move(*p));
            break;
          default:
            break;
        }

        break;
      }

      default:
        break;
    }
  }

  ~observer_core() { MIDIClientDispose(this->client); }

private:
  MIDIClientRef client{};
};
}

NAMESPACE_LIBREMIDI::coremidi
{
struct observer_impl : observer_core
{
  observer_impl(observer_configuration&& conf, coremidi_observer_configuration&& apiconf)
      : observer_core{std::move(conf), std::move(apiconf)}
  {
    finish_init();
  }

  libremidi::API get_current_api() const noexcept override { return libremidi::API::COREMIDI; }
};
}
