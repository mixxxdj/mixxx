#define _DEFAULT_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <linux/uhid.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#ifndef VENDOR_ID
#define VENDOR_ID 0xDEAD
#endif
#ifndef PRODUCT_ID
#define PRODUCT_ID 0xBEAF
#endif
#ifndef DEVICE_NAME
#define DEVICE_NAME "test-uhid-device"
#endif

static volatile u_int32_t done;

static unsigned char rdesc[] = {
        0x06, 0x02, 0xFF, /*  Usage Page (FF02h),                     */
        0x09,
        0x00, /*  Usage (00h),                            */
        0xA1,
        0x01, /*  Collection (Application),               */
        0x09,
        0x01, /*      Usage (01h),                        */
        0xA1,
        0x02, /*      Collection (Logical),               */
        0x85,
        0x01, /*          Report ID (1),                  */
        0x09,
        0x02, /*          Usage (02h),                    */
        0x15,
        0x00, /*          Logical Minimum (0),            */
        0x25,
        0x01, /*          Logical Maximum (1),            */
        0x75,
        0x01, /*          Report Size (1),                */
        0x95,
        0x88, /*          Report Count (136),             */
        0x81,
        0x02, /*          Input (Variable),               */
        0x09,
        0x07, /*          Usage (07h),                    */
        0x15,
        0x00, /*          Logical Minimum (0),            */
        0x25,
        0x01, /*          Logical Maximum (1),            */
        0x75,
        0x01, /*          Report Size (1),                */
        0x95,
        0x10, /*          Report Count (16),              */
        0x81,
        0x02, /*          Input (Variable),               */
        0x09,
        0x03, /*          Usage (03h),                    */
        0x15,
        0x00, /*          Logical Minimum (0),            */
        0x25,
        0x0F, /*          Logical Maximum (15),           */
        0x75,
        0x04, /*          Report Size (4),                */
        0x95,
        0x06, /*          Report Count (6),               */
        0x81,
        0x02, /*          Input (Variable),               */
        0xC0, /*      End Collection,                     */
        0xC0  /*  End Collection                          */
};

void sighndlr(int signal) {
    done = 1;
    printf("\n");
}

static int uhid_write(int fd, const struct uhid_event* ev) {
    ssize_t ret;

    ret = write(fd, ev, sizeof(*ev));
    if (ret < 0) {
        fprintf(stderr, "Cannot write to uhid: %s\n", strerror(errno));
        return -errno;
    } else if (ret != sizeof(*ev)) {
        fprintf(stderr, "Wrong size written to uhid: %zd != %zu\n", ret, sizeof(ev));
        return -1;
    } else {
        return 0;
    }
}

static int create(int fd) {
    struct uhid_event ev;

    memset(&ev, 0, sizeof(ev));
    ev.type = UHID_CREATE;
    strcpy((char*)ev.u.create.name, DEVICE_NAME);
    ev.u.create.rd_data = rdesc;
    ev.u.create.rd_size = sizeof(rdesc);
    ev.u.create.bus = BUS_USB;
    ev.u.create.vendor = VENDOR_ID;
    ev.u.create.product = PRODUCT_ID;
    ev.u.create.version = 0;
    ev.u.create.country = 0;

    return uhid_write(fd, &ev);
}

static void destroy(int fd) {
    struct uhid_event ev;

    memset(&ev, 0, sizeof(ev));
    ev.type = UHID_DESTROY;

    uhid_write(fd, &ev);
}

/* This parses raw output reports sent by the kernel to the device. A normal
 * uhid program shouldn't do this but instead just forward the raw report.
 * However, for ducomentational purposes, we try to detect LED events here and
 * print debug messages for it. */
static void handle_output(struct uhid_event* ev) {
    if (ev->u.output.rtype != UHID_OUTPUT_REPORT)
        return;

    fprintf(stderr,
            "Output received of type %d and size %d: %s\n",
            ev->u.output.rtype,
            ev->u.output.size,
            ev->u.output.data);
    for (int i = 0; i < ev->u.output.size; i++) {
        printf("%02X ", ev->u.output.data[i]);
    }
    printf("\n");
    return;
}
static void handle_report(struct uhid_event* ev, int fd) {
    fprintf(stderr, "RType is %d\n", ev->u.get_report.rtype);
    if (ev->u.get_report.rtype != UHID_START)
        return;

    fprintf(stderr, "Get report for %d\n", ev->u.get_report.rnum);

    struct uhid_event rep;

    memset(&rep, 0, sizeof(rep));
    rep.type = UHID_GET_REPORT_REPLY;

    rep.u.get_report_reply.id = ev->u.get_report.id;
    rep.u.get_report_reply.err = 0;
    rep.u.get_report_reply.size = 255;
    memset(rep.u.get_report_reply.data, 0, UHID_DATA_MAX);

    uhid_write(fd, &rep);
    return;
}

static int event(int fd) {
    struct uhid_event ev;
    ssize_t ret;

    memset(&ev, 0, sizeof(ev));
    ret = read(fd, &ev, sizeof(ev));
    if (ret == 0) {
        fprintf(stderr, "Read HUP on uhid-cdev\n");
        return -1;
    } else if (ret < 0) {
        fprintf(stderr, "Cannot read uhid-cdev\n");
        return -errno;
    } else if (ret != sizeof(ev)) {
        fprintf(stderr, "Invalid size read from uhid-dev: %zd != %zu\n", ret, sizeof(ev));
        return -1;
    }

    switch (ev.type) {
    case UHID_START:
        fprintf(stderr, "UHID_START from uhid-dev\n");
        break;
    case UHID_STOP:
        fprintf(stderr, "UHID_STOP from uhid-dev\n");
        break;
    case UHID_OPEN:
        fprintf(stderr, "UHID_OPEN from uhid-dev\n");
        break;
    case UHID_CLOSE:
        fprintf(stderr, "UHID_CLOSE from uhid-dev\n");
    case UHID_OUTPUT:
        fprintf(stderr, "UHID_OUTPUT from uhid-dev\n");
        handle_output(&ev);
        break;
    case UHID_OUTPUT_EV:
        fprintf(stderr, "UHID_OUTPUT_EV from uhid-dev\n");
        break;
    case UHID_GET_REPORT:
        fprintf(stderr, "UHID_GET_REPORT from uhid-dev\n");
        handle_report(&ev, fd);
        break;
    default:
        fprintf(stderr, "Invalid event from uhid-dev: %u\n", ev.type);
    }

    return 0;
}

int main(int argc, char** argv) {
    signal(SIGINT, sighndlr);

    // UHID
    int fd = open("/dev/uhid", O_RDWR);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    fprintf(stderr, "Create uhid device\n");
    if (create(fd)) {
        close(fd);
        return EXIT_FAILURE;
    }

    struct pollfd pfds;
    pfds.fd = fd;
    pfds.events = POLLIN;

    while (!done) {
        int ret = poll(&pfds, 1, 10);
        if (ret < 0) {
            fprintf(stderr, "Cannot poll for fds: %m\n");
            break;
        }
        if (pfds.revents & POLLHUP) {
            fprintf(stderr, "Received HUP on uhid-cdev\n");
            break;
        }
        if (pfds.revents & POLLIN) {
            ret = event(fd);
            if (ret)
                break;
        }
    }

    destroy(fd);

    return 0;
}
