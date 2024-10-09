#pragma once

#include <cstdint>

// A list of supported USB bulk devices

static constexpr std::uint32_t kInvalidInterfaceNumber = 0;
struct bulk_supported_t {
    std::uint16_t vendor_id;
    std::uint16_t product_id;
    std::uint8_t in_epaddr;
    std::uint8_t out_epaddr;
    std::uint32_t interface_number = kInvalidInterfaceNumber;
};

constexpr bool operator==(const bulk_supported_t& lhs, const bulk_supported_t& rhs) {
    return lhs.vendor_id == rhs.vendor_id && lhs.product_id == rhs.product_id &&
            lhs.in_epaddr == rhs.in_epaddr && lhs.out_epaddr == rhs.out_epaddr
#if defined(__WINDOWS__) || defined(__APPLE__)
            && lhs.interface_number == rhs.interface_number
#endif
            ;
}

constexpr bool operator!=(const bulk_supported_t& lhs, const bulk_supported_t& rhs) {
    return !(lhs == rhs);
}

constexpr static bulk_supported_t bulk_supported[] = {
        {0x06f8, 0xb105, 0x82, 0x03}, // Hercules MP3e2
        {0x06f8, 0xb107, 0x83, 0x03}, // Hercules Mk4
        {0x06f8, 0xb100, 0x86, 0x06}, // Hercules Mk2
        {0x06f8, 0xb120, 0x82, 0x03}, // Hercules MP3 LE / Glow
};
