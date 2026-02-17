/* -*- c-basic-offset: 8; -*-
 * example.c: Demonstration of the libshout API.
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if !(defined(WIN32) && !defined(__MINGW64__) && !defined(__MINGW32__))
#include <time.h>
#endif

#include <shoutidjc/shout.h>

int main()
{
    shout_t *shout;
    unsigned char buff[4096];
    size_t read, total;
    int ret;

    shout_init();

    if (!(shout = shout_new())) {
        printf("Could not allocate shout_t\n");
        return 1;
    }

    if (shout_set_host(shout, "127.0.0.1") != SHOUTERR_SUCCESS) {
        printf("Error setting hostname: %s\n", shout_get_error(shout));
        return 1;
    }

    if (shout_set_protocol(shout, SHOUT_PROTOCOL_HTTP) != SHOUTERR_SUCCESS) {
        printf("Error setting protocol: %s\n", shout_get_error(shout));
        return 1;
    }

    if (shout_set_port(shout, 8000) != SHOUTERR_SUCCESS) {
        printf("Error setting port: %s\n", shout_get_error(shout));
        return 1;
    }

    if (shout_set_password(shout, "hackme") != SHOUTERR_SUCCESS) {
        printf("Error setting password: %s\n", shout_get_error(shout));
        return 1;
    }
    if (shout_set_mount(shout, "/example.ogg") != SHOUTERR_SUCCESS) {
        printf("Error setting mount: %s\n", shout_get_error(shout));
        return 1;
    }

    if (shout_set_user(shout, "source") != SHOUTERR_SUCCESS) {
        printf("Error setting user: %s\n", shout_get_error(shout));
        return 1;
    }

    if (shout_set_content_format(shout, SHOUT_FORMAT_OGG, SHOUT_USAGE_UNKNOWN, NULL) != SHOUTERR_SUCCESS) {
        printf("Error setting user: %s\n", shout_get_error(shout));
        return 1;
    }

    if (shout_set_nonblocking(shout, 1) != SHOUTERR_SUCCESS) {
        printf("Error setting non-blocking mode: %s\n", shout_get_error(shout));
        return 1;
    }

    ret = shout_open(shout);
    if (ret == SHOUTERR_SUCCESS)
        ret = SHOUTERR_CONNECTED;

    if (ret == SHOUTERR_BUSY)
        printf("Connection pending...\n");

    while (ret == SHOUTERR_BUSY || ret == SHOUTERR_RETRY) {
#if !(defined(WIN32) && !defined(__MINGW64__) && !defined(__MINGW32__))
        static const struct timespec ts = {.tv_sec = 0, .tv_nsec = 10 * 1000 * 1000};
        nanosleep(&ts, NULL);
#else
        usleep(10000);
#endif
        ret = shout_get_connected(shout);
    }

    if (ret == SHOUTERR_CONNECTED) {
        printf("Connected to server...\n");
        total = 0;
        while (1) {
            read = fread(buff, 1, sizeof(buff), stdin);
            total = total + read;

            if (read > 0) {
                ret = shout_send(shout, buff, read);
                if (ret != SHOUTERR_SUCCESS) {
                    printf("DEBUG: Send error: %s\n", shout_get_error(shout));
                    break;
                }
            } else {
                break;
            }
            if (shout_queuelen(shout) > 0)
                printf("DEBUG: queue length: %d\n",
                        (int)shout_queuelen(shout));

            shout_sync(shout);
        }
    } else {
        printf("Error connecting: %s\n", shout_get_error(shout));
    }

    shout_close(shout);

    shout_shutdown();

    return 0;
}
