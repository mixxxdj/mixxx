#pragma once
#include <libremidi/api.hpp>
#include <libremidi/config.hpp>
#include <libremidi/error.hpp>
#include <libremidi/types.hpp>

#include <compare>
#include <string>
#include <variant>

namespace libremidi
{

struct LIBREMIDI_EXPORT port_information
{
  // Compat
  using port_type = libremidi::transport_type;

  /// Which API is this port for. port_information objects are in general
  /// not useable for different APIs than the API of the observer that created them.
  libremidi::API api{};

  /// Handle to the API client object if the API provides one
  // Android: unused
  // ALSA Raw: unused
  // ALSA Seq: snd_seq_t*
  // CoreMIDI: MidiClientRef
  // JACK: jack_client_t*
  // PipeWire: unused // FIXME: pw_context? pw_main_loop?
  // WebMIDI: unused
  // WinMIDI: TODO
  // WinMM: unused
  // WinUWP: unused
  client_handle client = static_cast<client_handle>(-1);

  /// Container identifier if the API provides one
  // Android: unavailable
  // ALSA: device id (std::string), e.g. ID_PATH as returned by udev: "pci-0000:00:14.0-usb-0:12:1.0"
  // CoreMIDI: USBLocationID (int32_t)
  // JACK: unavailable
  // PipeWire: unavailable
  // WinMIDI: ContainerID GUID (bit_cast to a winapi or winrt::GUID ;
  //        this is not the string but the binary representation).
  // WinMM: unavailable
  // WinUWP: unavailable
  container_identifier container = libremidi_variant_alias::monostate{};

  /// Device identifier if the API provides one
  // Android: unavailable
  // ALSA: sysfs path (std::string), e.g. "/sys/devices/pci0000:00/0000:00:02.2/0000:02:00.0/sound/card0/controlC0"
  // CoreMIDI: USBVendorProduct (int32_t)
  // JACK: unavailable
  // PipeWire: unavailable
  // WinMIDI: EndpointDeviceId (std::string), e.g. "\\?\swd#midisrv#midiu_ksa..."
  // WinMM: MIDI{IN,OUT}CAPS mId / pId { uint16_t manufacturer_id, uint16_t product_id; }
  // WinUWP: unavailable
  device_identifier device = libremidi_variant_alias::monostate{};

  /// Handle to the port identifier if the API provides one
  // Android: index of the MIDI device in the list provided by the OS.
  // ALSA Raw: bit_cast to struct { uint16_t card, device, sub, padding; }.
  // ALSA Seq: bit_cast to struct { uint32_t client, port; }
  // CoreMIDI: MidiObjectRef's kMIDIPropertyUniqueID (uint32_t)
  // JACK: jack_port_id_t
  // PipeWire: "port.id" (uint32_t)
  // WebMIDI: index of the MIDI device in the list provided by the browser.
  // WinMIDI: uint64_t terminal_block_number; (MidiGroupTerminalBlock::Number(), index is 1-based)
  // WinMM: port index between 0 and midi{In,Out}GetNumDevs()
  // WinUWP: index of the MIDI device in the list provided by the OS.
  port_handle port = static_cast<port_handle>(-1);

  /// User-readable information
  // Android: unavailable
  // ALSA Raw: ID_VENDOR_FROM_DATABASE if provided by udev
  // ALSA Seq: ID_VENDOR_FROM_DATABASE if provided by udev
  // CoreMIDI: kMIDIPropertyManufacturer
  // JACK: unavailable
  // PipeWire: unavailable
  // WinMIDI: MidiEndpointDeviceInformation::GetTransportSuppliedInfo().ManufacturerName
  // WinMM: unavailable
  // WinUWP: unavailable
  std::string manufacturer{};

  // Android: unavailable
  // ALSA Raw: ID_MODEL_FROM_DATABASE if provided by udev
  // ALSA Seq: ID_MODEL_FROM_DATABASE if provided by udev
  // JACK: unavailable
  // PipeWire: unavailable
  // WinMIDI: MidiEndpointDeviceInformation::GetTransportSuppliedInfo().Name
  // WinUWP: unavailable
  std::string product{};

  /// "Unique" serial number. Note that this is super unreliable - pretty
  /// much no MIDI device manufacturer bothers with unique per-device serial number
  /// unlike most USB devices.
  // Android: unavailable
  // ALSA Raw: ID_USB_SERIAL if provided by udev.
  // ALSA Seq: ID_USB_SERIAL if provided by udev.
  // JACK: unavailable
  // PipeWire: unavailable
  // WinMIDI: MidiEndpointDeviceInformation::GetTransportSuppliedInfo().SerialNumber
  // WinUWP: unavailable
  std::string serial{};

  // Android: unavailable
  // ALSA Raw: Name returned by snd_rawmidi_info_get_name
  // ALSA Seq: Name returned by snd_seq_client_info_get_name
  // CoreMIDI: kMIDIPropertyModel
  // JACK: jack_get_client_name
  // PipeWire: first part of port_alias, e.g. "A" in A:B
  // WinMIDI: MidiEndpointDeviceInformation::Name
  // WinMM: unavailable
  // WinUWP: unavailable
  std::string device_name{};

  // Android: MidiDeviceInfo.getProperties().getString("name")
  // ALSA Raw: Name returned by snd_rawmidi_info_get_subdevice_name
  // ALSA Seq: Name returned by snd_seq_port_info_get_name
  // CoreMIDI: kMIDIPropertyName
  // JACK: jack_port_name
  // PipeWire: "port.name"
  // WinMIDI: MidiGroupTerminalBlock::Name
  // WinMM: szPname
  // WinUWP: DeviceInformation.Id()
  std::string port_name{};

  // CoreMIDI: kMIDIPropertyDisplayName
  // WinUWP: DeviceInformation.Name()
  // Otherwise: the closest to a unique name we can get
  std::string display_name{};

  /// Port type
  // Android: unavailable
  // ALSA Raw / Seq: available if udev is avaialble
  // CoreMIDI: available
  // JACK: unavailable
  // PipeWire: unavailable
  // WinMIDI: available
  // WinMM: unavailable
  // WinUWP: unavailable
  port_type type = port_type::unknown;

  // Equality and comparison operators are deleted as there is not one
  // single correct way to compare two port_information:
  // in some cases it may be useful to only compare the names, while in other cases
  // it is necessary to check whether this is the exact same low-level identifier.
  // Thus, the end-user must define their own custom equality operators
  // if using std:: containers or algorithms
  bool operator==(const port_information& other) const noexcept = delete;
  std::strong_ordering operator<=>(const port_information& other) const noexcept = delete;
};

struct input_port : port_information
{
  bool operator==(const input_port& other) const noexcept = delete;
  std::strong_ordering operator<=>(const input_port& other) const noexcept = delete;
};
struct output_port : port_information
{
  bool operator==(const output_port& other) const noexcept = delete;
  std::strong_ordering operator<=>(const output_port& other) const noexcept = delete;
};

}
