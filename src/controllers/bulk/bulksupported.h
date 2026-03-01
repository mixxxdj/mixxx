#pragma once

#include <cstdint>
#include <optional>

struct bulk_device_id {
    std::uint16_t vendor_id;
    std::uint16_t product_id;
};

// A list of supported USB bulk devices

struct bulk_device_endpoints {
    std::uint8_t in_epaddr;
    std::uint8_t out_epaddr;
    // we may not know the interface, in which case we should not try to claim it.
    // these devices are likely unusable on windows without claiming the correct interface.
    std::optional<std::uint8_t> interface_number;
};

struct bulk_support_lookup {
    bulk_device_id key;
    bulk_device_endpoints endpoints;
};

constexpr static bulk_support_lookup bulk_supported[] = {
        {{0x06f8, 0xb105}, {0x82, 0x03, std::nullopt}}, // Hercules MP3e2
        {{0x06f8, 0xb107}, {0x83, 0x03, std::nullopt}}, // Hercules Mk4
        {{0x06f8, 0xb100}, {0x86, 0x06, std::nullopt}}, // Hercules Mk2
        {{0x06f8, 0xb120}, {0x82, 0x03, std::nullopt}}, // Hercules MP3 LE / Glow
        {{0x17cc, 0x1720}, {0x00, 0x03, 0x04}},         // Traktor NI S4 Mk3
};
