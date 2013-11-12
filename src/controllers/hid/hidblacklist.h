// Blacklisted HID devices

#ifndef HIDBLACKLIST_H
#define HIDBLACKLIST_H

typedef struct hid_blacklist {
    unsigned short vendor_id;
    unsigned short product_id;
    unsigned short usage_page;
    unsigned short usage;
    int interface_number;
} hid_blacklist_t;

hid_blacklist_t hid_blacklisted[] = {
    {0x5ac, 0x253, 0xff00, 0x1, -1}, // Apple laptop chassis
    {0x5ac, 0x8242, 0xc, 0x1, -1},   // Apple IR Remote Controller
    {0x1157, 0x300, 0x1, 0x2, -1},   // EKS Otus mouse pad (OS/X,windows)
    {0x1157, 0x300, 0x0, 0x0, 0x3},  // EKS Otus mouse pad (linux)
};

#endif
