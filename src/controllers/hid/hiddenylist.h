#pragma once

#include <cstdint>

// TODO: unify this with the invalid interfacenumber from the bulkenumerator
constexpr static std::int32_t kInvalidInterfaceNumber = -1;
constexpr static std::int32_t kBlanketValue = 0x0;

struct hid_denylist_t {
    std::uint16_t vendor_id;
    std::uint16_t product_id;
    std::uint16_t usage_page;
    std::uint16_t usage;
    std::int32_t interface_number = kInvalidInterfaceNumber;
};

/// USB HID device that should not be recognized as controllers
constexpr static hid_denylist_t hid_denylisted[] = {
        {0x1157, 0x300, 0x1, 0x2},                          // EKS Otus mouse pad (OS/X,windows)
        {0x1157, 0x300, kBlanketValue, kBlanketValue, 0x3}, // EKS Otus mouse pad (linux)
        {0x04f3, 0x2d26, kBlanketValue, kBlanketValue},     // ELAN2D26:00 Touch screen
        {0x046d, 0xc539, kBlanketValue, kBlanketValue},     // Logitech G Pro Wireless
        // The following rules have been created using the official USB HID page
        // spec as specified at https://usb.org/sites/default/files/hut1_4.pdf
        {
                kBlanketValue,
                kBlanketValue,
                0x0D,
                0x04,
        }, // Touch Screen
        {
                kBlanketValue,
                kBlanketValue,
                0x0D,
                0x22,
        }, // Finger
};
