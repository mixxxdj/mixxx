/**
* @file bulksupported.h
* @author Neale Picket  neale@woozle.org
* @date Thu Jun 28 2012
* @brief A list of supported USB bulk devices
*/

#ifndef BULKSUPPORTED_H
#define BULKSUPPORTED_H

typedef struct bulk_supported {
    unsigned short vendor_id;
    unsigned short product_id;
    unsigned char in_epaddr;
    unsigned char out_epaddr;
} bulk_supported_t;

static bulk_supported_t bulk_supported[] = {
    {0x06f8, 0xb105, 0x82, 0x03},   // Hercules MP3e2
    {0x06f8, 0xb100, 0x86, 0x06},   // Hercules Mk2
    {0, 0, 0, 0}
};

#endif
