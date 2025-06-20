#pragma once

// TODO: unify this with the invalid interfacenumber from the bulkenumerator
constexpr static int kInvalidInterfaceNumber = -1;
constexpr static unsigned short kAnyValue = 0x0;

struct hid_denylist_t {
    unsigned short vendor_id;
    unsigned short product_id;
    unsigned short usage_page;
    unsigned short usage;
    int interface_number = kInvalidInterfaceNumber;
};

/// USB HID device that should not be recognized as controllers
constexpr static hid_denylist_t hid_denylisted[] = {
        {0x1157, 0x300, 0x1, 0x2},                  // EKS Otus mouse pad (OS/X,windows)
        {0x1157, 0x300, kAnyValue, kAnyValue, 0x3}, // EKS Otus mouse pad (linux)
        {0x04f3, 0x2d26, kAnyValue, kAnyValue},     // ELAN2D26:00 Touch screen
        {0x046d, 0xc539, kAnyValue, kAnyValue},     // Logitech G Pro Wireless
        // The following rules have been created using the official USB HID page
        // spec as specified at https://usb.org/sites/default/files/hut1_4.pdf
        {
                kAnyValue,
                kAnyValue,
                0x0D,
                0x04,
        }, // Touch Screen
        {
                kAnyValue,
                kAnyValue,
                0x0D,
                0x22,
        }, // Finger
};
