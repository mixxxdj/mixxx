#pragma once

typedef struct hid_denylist {
    unsigned short vendor_id;
    unsigned short product_id;
    unsigned short usage_page;
    unsigned short usage;
    int interface_number;
} hid_denylist_t;

/// USB HID device that should not be recognized as controllers
constexpr hid_denylist_t hid_denylisted[] = {
        {0x1157, 0x300, 0x1, 0x2, -1},  // EKS Otus mouse pad (OS/X,windows)
        {0x1157, 0x300, 0x0, 0x0, 0x3}, // EKS Otus mouse pad (linux)
        {0x04f3, 0x2d26, 0x0, 0x0, -1}, // ELAN2D26:00 Touch screen
        {0x046d, 0xc539, 0x0, 0x0, -1}, // Logitech G Pro Wireless
        // The following rules have been created using the official USB HID page
        // spec as specified at https://usb.org/sites/default/files/hut1_4.pdf
        {0x0, 0x0, 0x0D, 0x04, -1}, // Touch Screen
        {0x0, 0x0, 0x0D, 0x22, -1}, // Finger
};
