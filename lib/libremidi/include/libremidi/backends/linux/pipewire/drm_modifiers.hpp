#pragma once

#include <cstdint>

namespace libremidi::pipewire
{
constexpr std::uint64_t DRM_FORMAT_MOD_LINEAR_ = 0;
constexpr std::uint64_t DRM_FORMAT_MOD_INVALID_ = 0x00FFFFFFFFFFFFFFLL;

// DMA-BUF plane count for (fourcc, modifier). Vendor bits are
// modifier[56-63]. Conservatively returns 1 for unknown pairs;
// pipewire rejects mismatched counts so the producer can renegotiate.
constexpr std::uint32_t
drm_modifier_plane_count(std::uint32_t /*fourcc*/, std::uint64_t modifier) noexcept
{
  if (modifier == DRM_FORMAT_MOD_LINEAR_ || modifier == DRM_FORMAT_MOD_INVALID_)
    return 1;

  const std::uint8_t vendor = static_cast<std::uint8_t>((modifier >> 56) & 0xFFu);
  switch (vendor)
  {
    case 0x02: // AMD
      return 2;
    case 0x01: // Intel
      return 2;
    case 0x08: // ARM (AFBC)
      return 1;
    case 0x03: // NVIDIA
      return 1;
    default:
      return 1;
  }
}

} // namespace libremidi::pipewire
