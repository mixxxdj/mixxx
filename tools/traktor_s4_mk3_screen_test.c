#include <libusb.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define VENDOR_ID 0x17cc
#define PRODUCT_ID 0x1720
#define IN_EPADDR 0x00
#define OUT_EPADDR 0x03

static const uint8_t header_data[] = {
        0x84, 0x0, 0x0, 0x21, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // Draw offset (y=0, x=0)
        0x1,
        0x40,
        0x0,
        0xf0, // Draw dimenssion (width=320, height=240)
};
static const uint8_t footer_data[] = {
        0x40, 0x00, 0x00, 0x00};

int main(int argc, char** argv) {
    libusb_context* context;
    int transferred;

    if (argc != 7) {
        fprintf(stderr, "Usage: %s <screen_idx> <x> <y> <width> <height> <color>\n", *argv);
        return -1;
    }

    libusb_init(&context);

    libusb_device_handle* handle = libusb_open_device_with_vid_pid(
            context, VENDOR_ID, PRODUCT_ID);

    if (!handle) {
        fprintf(stderr, "Unable to open USB Bulk device\n");
        return -1;
    }

    uint8_t screen_idx = atoi(argv[1]);

    if (screen_idx != 0 && screen_idx != 1) {
        fprintf(stderr, "Invalid screen ID %d\n", screen_idx);
        return -1;
    }

    uint16_t x = atoi(argv[2]);
    uint16_t y = atoi(argv[3]);
    uint16_t width = atoi(argv[4]);
    uint16_t height = atoi(argv[5]);
    uint16_t color = strtol(argv[6], NULL, 2);

    uint8_t* data = malloc(width * height * sizeof(uint16_t) +
            sizeof(header_data) + sizeof(footer_data));
    uint8_t* header = data;

    memcpy(header, header_data, sizeof(header_data));

    header[2] = screen_idx;

    header[8] = x >> 8;
    header[9] = x & 0xff;
    header[10] = y >> 8;
    header[11] = y & 0xff;

    header[12] = width >> 8;
    header[13] = width & 0xff;
    header[14] = height >> 8;
    header[15] = height & 0xff;

    printf("draw x=%d,y=%d,width=%d,height=%d with color %x\n", x, y, width, height, color);

    size_t payload_size = width * height * sizeof(uint16_t) +
            sizeof(header_data) + sizeof(footer_data);
    uint8_t* payload = data + sizeof(header_data);
    uint8_t* footer = payload + width * height * sizeof(uint16_t);

    for (int px = 0; px < width * height; px++) {
        payload[px * sizeof(uint16_t)] = color >> 8;
        payload[px * sizeof(uint16_t) + 1] = color & 0xff;
    }

    memcpy(footer, footer_data, sizeof(footer_data));

    footer[2] = screen_idx;

    clock_t start, end;
    double cpu_time_used;

    start = clock();
    int ret = libusb_bulk_transfer(handle, OUT_EPADDR, data, payload_size, &transferred, 0);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

    if (ret < 0) {
        fprintf(stderr, "Unable to send to USB Bulk device\n");

    } else {
        fprintf(stderr, "Sent %d bytes in %f ms\n", transferred, cpu_time_used);
    }

    libusb_close(handle);
    libusb_exit(context);

    return 0;
}
