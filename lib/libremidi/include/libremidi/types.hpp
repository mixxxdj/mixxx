#pragma once
#include <libremidi/config.hpp>

#include <array>
#include <compare>
#include <cstdint>
#include <string>

NAMESPACE_LIBREMIDI
{
/// MIDI protocol supported by an endpoint or function block
enum class midi_protocol : uint8_t
{
  midi1 = (1 << 1),
  midi2 = (1 << 2),
  both = midi1 | midi2
};

enum transport_type : uint8_t
{
  unknown = 0,

  software = (1 << 1),
  loopback = (1 << 2),

  hardware = (1 << 3),
  usb = (1 << 4),
  bluetooth = (1 << 5),
  pci = (1 << 6),

  network = (1 << 7),
};

/// UMP version information
struct ump_version
{
  uint8_t major{1};
  uint8_t minor{1};

  bool operator==(const ump_version&) const noexcept = default;
  std::strong_ordering operator<=>(const ump_version&) const noexcept = default;
};

using client_handle = std::uint64_t;
using port_handle = std::uint64_t;

struct uuid
{
  std::array<uint8_t, 16> bytes;

  bool operator==(const uuid& other) const noexcept = default;
  std::strong_ordering operator<=>(const uuid& other) const noexcept = default;
};

struct usb_device_identifier
{
  uint16_t vendor_id;
  uint16_t product_id;

  bool operator==(const usb_device_identifier& other) const noexcept = default;
  std::strong_ordering operator<=>(const usb_device_identifier& other) const noexcept = default;
};

using container_identifier = libremidi_variant_alias::variant<
    libremidi_variant_alias::monostate, uuid, std::string, std::uint64_t>;
using device_identifier = libremidi_variant_alias::variant<
    libremidi_variant_alias::monostate, std::string, std::uint64_t, usb_device_identifier>;
using endpoint_identifier = libremidi_variant_alias::variant<
    libremidi_variant_alias::monostate, std::string, std::uint64_t>;
}
