// A list of supported USB bulk devices

#pragma once

typedef struct bulk_supported {
    unsigned short vendor_id;
    unsigned short product_id;
    unsigned char in_epaddr;
    unsigned char out_epaddr;
} bulk_supported_t;

static bulk_supported_t bulk_supported[] = {
    {0x06f8, 0xb105, 0x82, 0x03}, // Hercules MP3e2
    {0x06f8, 0xb107, 0x83, 0x03}, // Hercules Mk4
    {0x06f8, 0xb100, 0x86, 0x06}, // Hercules Mk2
    {0x06f8, 0xb120, 0x82, 0x03}, // Hercules MP3 LE / Glow
    {0, 0, 0, 0}
};
