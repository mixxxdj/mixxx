HID API for Windows, Linux, and Mac OS X

About
------

HIDAPI is a multi-platform library which allows an application to interface
with USB and Bluetooth HID-Class devices on Windows, Linux, and Mac OS X. 
On Windows, a DLL is built.  On other platforms (and optionally on Windows),
the single source file can simply be dropped into a target application.

HIDAPI has four back-ends:
	* Windows (using hid.dll)
	* Linux/hidraw (using the Kernel's hidraw driver)
	* Linux/libusb (using libusb-1.0)
	* Mac (using IOHidManager)

On Linux, either the hidraw or the libusb back-end can be used. There are
tradeoffs, and the functionality supported is slightly different.

Linux/hidraw (linux/hid.c):
This back-end uses the hidraw interface in the Linux kernel.  While this
back-end will support both USB and Bluetooth, it has some limitations on
kernels prior to 2.6.39, including the inability to send or receive feature
reports.  In addition, it will only communicate with devices which have
hidraw nodes associated with them.  Keyboards, mice, and some other devices
which are blacklisted from having hidraw nodes will not work. Fortunately,
for nearly all the uses of hidraw, this is not a problem.

Linux/libusb (linux/hid-libusb.c):
This back-end uses libusb-1.0 to communicate directly to a USB device. This
back-end will of course not work with Bluetooth devices.

What Does the API Look Like?
-----------------------------
The API provides the the most commonly used HID functions including sending
and receiving of input, output, and feature reports.  The sample program,
which communicates with a heavily modified version the USB Generic HID
sample which is part of the Microchip Application Library (in folder
"Microchip Solutions\USB Device - HID - Custom Demos\Generic HID - Firmware"
when the Microchip Application Framework is installed), looks like this
(with error checking removed for simplicity):

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "hidapi.h"

#define MAX_STR 255

int main(int argc, char* argv[])
{
	int res;
	unsigned char buf[65];
	wchar_t wstr[MAX_STR];
	hid_device *handle;
	int i;

	// Open the device using the VID, PID,
	// and optionally the Serial number.
	handle = hid_open(0x4d8, 0x3f, NULL);

	// Read the Manufacturer String
	res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
	wprintf(L"Manufacturer String: %s\n", wstr);

	// Read the Product String
	res = hid_get_product_string(handle, wstr, MAX_STR);
	wprintf(L"Product String: %s\n", wstr);

	// Read the Serial Number String
	res = hid_get_serial_number_string(handle, wstr, MAX_STR);
	wprintf(L"Serial Number String: (%d) %s\n", wstr[0], wstr);

	// Read Indexed String 1
	res = hid_get_indexed_string(handle, 1, wstr, MAX_STR);
	wprintf(L"Indexed String 1: %s\n", wstr);

	// Toggle LED (cmd 0x80). The first byte is the report number (0x0).
	buf[0] = 0x0;
	buf[1] = 0x80;
	res = hid_write(handle, buf, 65);

	// Request state (cmd 0x81). The first byte is the report number (0x0).
	buf[0] = 0x0;
	buf[1] = 0x81;
	res = hid_write(handle, buf, 65);

	// Read requested state
	hid_read(handle, buf, 65);

	// Print out the returned buffer.
	for (i = 0; i < 4; i++)
		printf("buf[%d]: %d\n", i, buf[i]);

	return 0;
}

License
--------
HIDAPI may be used by one of three licenses as outlined in LICENSE.txt.

Download
---------
It can be downloaded from github
	git clone git://github.com/signal11/hidapi.git

Build Instructions
-------------------
To build the console test program:
  Windows:
    Build the .sln file in the windows/ directory.
  Linux:
    cd to the linux/ directory and run make.
  Mac OS X:
    cd to the mac/ directory and run make.

To build the Test GUI:
  The test GUI uses Fox toolkit, available from www.fox-toolkit.org.
  On Debian-based systems such as Ubuntu, install Fox using the following:
	sudo apt-get install libfox-1.6-dev
  On Mac OSX, install Fox from ports:
	sudo port install fox
  On Windows, download the hidapi-externals.zip file from the main download
  site and extract it just outside of hidapi, so that hidapi-externals and
  hidapi are on the same level, as shown:

     Parent_Folder
       |
       +hidapi
       +hidapi-externals

  Then to build:
    On Windows, build the .sln file in the testgui/ directory.
    On Linux and Mac, run make from the testgui/ directory.

To build using the DDK (old method):

   1. Install the Windows Driver Kit (WDK) from Microsoft.
   2. From the Start menu, in the Windows Driver Kits folder, select Build
      Environments, then your operating system, then the x86 Free Build
      Environment (or one that is appropriate for your system).
   3. From the console, change directory to the windows/ddk_build/ directory,
      which is part of the HIDAPI distribution.
   4. Type build.
   5. You can find the output files (DLL and LIB) in a subdirectory created
      by the build system which is appropriate for your environment. On
      Windows XP, this directory is objfre_wxp_x86/i386.

--------------------------------

Signal 11 Software - 2010-04-11
                     2010-07-28
                     2011-09-10
