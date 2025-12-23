/* example.c: Demonstration of the libshout API.
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <shoutidjc/shout.h>

int main()
{
	shout_t *shout;
	unsigned char buff[4096];
	long read, ret, total;

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

	if (shout_set_format(shout, SHOUT_FORMAT_OGG) != SHOUTERR_SUCCESS) {
		printf("Error setting user: %s\n", shout_get_error(shout));
		return 1;
	}

	if (shout_open(shout) == SHOUTERR_SUCCESS) {
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

			shout_sync(shout);
		}
	} else {
		printf("Error connecting: %s\n", shout_get_error(shout));
	}

	shout_close(shout);

	shout_shutdown();

	return 0;
}
