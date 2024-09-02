#pragma once

#include <cstdint>

// A list of supported USB bulk devices

struct bulk_supported_t {
    std::uint16_t vendor_id;
    std::uint16_t product_id;
    std::uint8_t in_epaddr;
    std::uint8_t out_epaddr;
    std::uint8_t interface_number;
};

constexpr static bulk_supported_t bulk_supported[] = {
        {0x06f8, 0xb105, 0x82, 0x03, 0}, // Hercules MP3e2
        {0x06f8, 0xb107, 0x83, 0x03, 0}, // Hercules Mk4
        {0x06f8, 0xb100, 0x86, 0x06, 0}, // Hercules Mk2
        {0x06f8, 0xb120, 0x82, 0x03, 0}, // Hercules MP3 LE / Glow
};
