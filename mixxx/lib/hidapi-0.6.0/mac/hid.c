/*******************************************************
 HIDAPI - Multi-Platform library for
 communication with HID devices.
 
 Alan Ott
 Signal 11 Software

 2010-07-03

 Copyright 2010, All Rights Reserved.
 
 At the discretion of the user of this library,
 this software may be licensed under the terms of the
 GNU Public License v3, a BSD-Style license, or the
 original HIDAPI license as outlined in the LICENSE.txt,
 LICENSE-gpl3.txt, LICENSE-bsd.txt, and LICENSE-orig.txt
 files located at the root of the source distribution.
 These files may also be found in the public source
 code repository located at:
        http://github.com/signal11/hidapi .
********************************************************/


#include <IOKit/hid/IOHIDManager.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <wchar.h>
#include <locale.h>
#include <pthread.h>

#include "hidapi.h"

/* Linked List of input reports received from the device. */
struct input_report {
	uint8_t *data;
	size_t len;
	struct input_report *next;
};

struct hid_device_ {
	IOHIDDeviceRef device_handle;
	int blocking;
	int uses_numbered_reports;
	int disconnected;
	CFStringRef run_loop_mode;
	uint8_t *input_report_buf;
	struct input_report *input_reports;
	pthread_mutex_t mutex;
	
	hid_device *next;
};

/* Static list of all the devices open. This way when a device gets
   disconnected, its hid_device structure can be marked as disconnected
   from hid_device_removal_callback(). */
static hid_device *device_list = NULL;

static hid_device *new_hid_device(void)
{
	hid_device *dev = calloc(1, sizeof(hid_device));
	dev->device_handle = NULL;
	dev->blocking = 1;
	dev->uses_numbered_reports = 0;
	dev->disconnected = 0;
	dev->run_loop_mode = NULL;
	dev->input_report_buf = NULL;
	dev->input_reports = NULL;
	dev->next = NULL;

	pthread_mutex_init(&dev->mutex, NULL);
	
	/* Add the new record to the device_list. */
	if (!device_list)
		device_list = dev;
	else {
		hid_device *d = device_list;
		while (d) {
			if (!d->next) {
				d->next = dev;
				break;
			}
			d = d->next;
		}
	}
	
	return dev;
}

static void free_hid_device(hid_device *dev)
{
	if (!dev)
		return;
	
	/* Delete any input reports still left over. */
	struct input_report *rpt = dev->input_reports;
	while (rpt) {
		struct input_report *next = rpt->next;
		free(rpt->data);
		free(rpt);
		rpt = next;
	}

	/* Free the string and the report buffer. The check for NULL
	   is necessary here as CFRelease() doesn't handle NULL like
	   free() and others do. */
	if (dev->run_loop_mode)
		CFRelease(dev->run_loop_mode);
	free(dev->input_report_buf);

	pthread_mutex_destroy(&dev->mutex);

	/* Remove it from the device list. */
	hid_device *d = device_list;
	if (d == dev) {
		device_list = d->next;
	}
	else {
		while (d) {
			if (d->next == dev) {
				d->next = d->next->next;
				break;
			}
			
			d = d->next;
		}
	}

	/* Free the structure itself. */
	free(dev);
	
}

static 	IOHIDManagerRef hid_mgr = 0x0;


#if 0
static void register_error(hid_device *device, const char *op)
{

}
#endif


static int32_t get_int_property(IOHIDDeviceRef device, CFStringRef key)
{
	CFTypeRef ref;
	int32_t value;
	
	ref = IOHIDDeviceGetProperty(device, key);
	if (ref) {
		if (CFGetTypeID(ref) == CFNumberGetTypeID()) {
			CFNumberGetValue((CFNumberRef) ref, kCFNumberSInt32Type, &value);
			return value;
		}
	}
	return 0;
}

static unsigned short get_vendor_id(IOHIDDeviceRef device)
{
	return get_int_property(device, CFSTR(kIOHIDVendorIDKey));
}

static unsigned short get_product_id(IOHIDDeviceRef device)
{
	return get_int_property(device, CFSTR(kIOHIDProductIDKey));
}


static int32_t get_max_report_length(IOHIDDeviceRef device)
{
	return get_int_property(device, CFSTR(kIOHIDMaxInputReportSizeKey));
}

static int get_string_property(IOHIDDeviceRef device, CFStringRef prop, wchar_t *buf, size_t len)
{
	CFStringRef str = IOHIDDeviceGetProperty(device, prop);

	buf[0] = 0x0000;

	if (str) {
		CFRange range;
		range.location = 0;
		range.length = len;
		CFIndex used_buf_len;
		CFStringGetBytes(str,
			range,
			kCFStringEncodingUTF32LE,
			(char)'?',
			FALSE,
			(UInt8*)buf,
			len,
			&used_buf_len);
		buf[len-1] = 0x00000000;
		return used_buf_len;
	}
	else
		return 0;
		
}

static int get_string_property_utf8(IOHIDDeviceRef device, CFStringRef prop, char *buf, size_t len)
{
	CFStringRef str = IOHIDDeviceGetProperty(device, prop);

	buf[0] = 0x0000;

	if (str) {
		CFRange range;
		range.location = 0;
		range.length = len;
		CFIndex used_buf_len;
		CFStringGetBytes(str,
			range,
			kCFStringEncodingUTF8,
			(char)'?',
			FALSE,
			(UInt8*)buf,
			len,
			&used_buf_len);
		buf[len-1] = 0x00000000;
		return used_buf_len;
	}
	else
		return 0;
		
}


static int get_serial_number(IOHIDDeviceRef device, wchar_t *buf, size_t len)
{
	return get_string_property(device, CFSTR(kIOHIDSerialNumberKey), buf, len);
}

static int get_manufacturer_string(IOHIDDeviceRef device, wchar_t *buf, size_t len)
{
	return get_string_property(device, CFSTR(kIOHIDManufacturerKey), buf, len);
}

static int get_product_string(IOHIDDeviceRef device, wchar_t *buf, size_t len)
{
	return get_string_property(device, CFSTR(kIOHIDProductKey), buf, len);
}


/* Implementation of wcsdup() for Mac. */
static wchar_t *dup_wcs(const wchar_t *s)
{
	size_t len = wcslen(s);
	wchar_t *ret = malloc((len+1)*sizeof(wchar_t));
	wcscpy(ret, s);

	return ret;
}


static int make_path(IOHIDDeviceRef device, char *buf, size_t len)
{
	int res;
	unsigned short vid, pid;
	char transport[32];

	buf[0] = '\0';

	res = get_string_property_utf8(
		device, CFSTR(kIOHIDTransportKey),
		transport, sizeof(transport));
	
	if (!res)
		return -1;

	vid = get_vendor_id(device);
	pid = get_product_id(device);

	res = snprintf(buf, len, "%s_%04hx_%04hx_%p",
	                   transport, vid, pid, device);
	
	
	buf[len-1] = '\0';
	return res+1;
}

static void init_hid_manager(void)
{
	/* Initialize all the HID Manager Objects */
	hid_mgr = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
	IOHIDManagerSetDeviceMatching(hid_mgr, NULL);
	IOHIDManagerScheduleWithRunLoop(hid_mgr, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	IOHIDManagerOpen(hid_mgr, kIOHIDOptionsTypeNone);
}


struct hid_device_info  HID_API_EXPORT *hid_enumerate(unsigned short vendor_id, unsigned short product_id)
{
	struct hid_device_info *root = NULL; // return object
	struct hid_device_info *cur_dev = NULL;
	CFIndex num_devices;
	int i;
	
	setlocale(LC_ALL,"");

	/* Set up the HID Manager if it hasn't been done */
	if (!hid_mgr)
		init_hid_manager();
	
	/* Get a list of the Devices */
	CFSetRef device_set = IOHIDManagerCopyDevices(hid_mgr);

	/* Convert the list into a C array so we can iterate easily. */	
	num_devices = CFSetGetCount(device_set);
	IOHIDDeviceRef *device_array = calloc(num_devices, sizeof(IOHIDDeviceRef));
	CFSetGetValues(device_set, (const void **) device_array);

	/* Iterate over each device, making an entry for it. */	
	for (i = 0; i < num_devices; i++) {
		unsigned short dev_vid;
		unsigned short dev_pid;
		#define BUF_LEN 256
		wchar_t buf[BUF_LEN];
		char cbuf[BUF_LEN];

		IOHIDDeviceRef dev = device_array[i];

		dev_vid = get_vendor_id(dev);
		dev_pid = get_product_id(dev);

		/* Check the VID/PID against the arguments */
		if ((vendor_id == 0x0 && product_id == 0x0) ||
		    (vendor_id == dev_vid && product_id == dev_pid)) {
			struct hid_device_info *tmp;
			size_t len;

		    	/* VID/PID match. Create the record. */
			tmp = malloc(sizeof(struct hid_device_info));
			if (cur_dev) {
				cur_dev->next = tmp;
			}
			else {
				root = tmp;
			}
			cur_dev = tmp;

			// Get the Usage Page and Usage for this device.
			cur_dev->usage_page = get_int_property(dev, CFSTR(kIOHIDPrimaryUsagePageKey));
			cur_dev->usage = get_int_property(dev, CFSTR(kIOHIDPrimaryUsageKey));

			/* Fill out the record */
			cur_dev->next = NULL;
			len = make_path(dev, cbuf, sizeof(cbuf));
			cur_dev->path = strdup(cbuf);

			/* Serial Number */
			get_serial_number(dev, buf, BUF_LEN);
			cur_dev->serial_number = dup_wcs(buf);

			/* Manufacturer and Product strings */
			get_manufacturer_string(dev, buf, BUF_LEN);
			cur_dev->manufacturer_string = dup_wcs(buf);
			get_product_string(dev, buf, BUF_LEN);
			cur_dev->product_string = dup_wcs(buf);
			
			/* VID/PID */
			cur_dev->vendor_id = dev_vid;
			cur_dev->product_id = dev_pid;

			/* Release Number */
			cur_dev->release_number = get_int_property(dev, CFSTR(kIOHIDVersionNumberKey));

			/* Interface Number (Unsupported on Mac)*/
			cur_dev->interface_number = -1;
		}
	}
	
	free(device_array);
	CFRelease(device_set);
	
	return root;
}

void  HID_API_EXPORT hid_free_enumeration(struct hid_device_info *devs)
{
	/* This function is identical to the Linux version. Platform independent. */
	struct hid_device_info *d = devs;
	while (d) {
		struct hid_device_info *next = d->next;
		free(d->path);
		free(d->serial_number);
		free(d->manufacturer_string);
		free(d->product_string);
		free(d);
		d = next;
	}
}

hid_device * HID_API_EXPORT hid_open(unsigned short vendor_id, unsigned short product_id, wchar_t *serial_number)
{
	/* This function is identical to the Linux version. Platform independent. */
	struct hid_device_info *devs, *cur_dev;
	const char *path_to_open = NULL;
	hid_device * handle = NULL;
	
	devs = hid_enumerate(vendor_id, product_id);
	cur_dev = devs;
	while (cur_dev) {
		if (cur_dev->vendor_id == vendor_id &&
		    cur_dev->product_id == product_id) {
			if (serial_number) {
				if (wcscmp(serial_number, cur_dev->serial_number) == 0) {
					path_to_open = cur_dev->path;
					break;
				}
			}
			else {
				path_to_open = cur_dev->path;
				break;
			}
		}
		cur_dev = cur_dev->next;
	}

	if (path_to_open) {
		/* Open the device */
		handle = hid_open_path(path_to_open);
	}

	hid_free_enumeration(devs);
	
	return handle;
}

static void hid_device_removal_callback(void *context, IOReturn result,
                                        void *sender, IOHIDDeviceRef dev_ref)
{
	hid_device *d = device_list;
	while (d) {
		if (d->device_handle == dev_ref) {
			d->disconnected = 1;
		}
		
		d = d->next;
	}
}

/* The Run Loop calls this function for each input report received.
   This function puts the data into a linked list to be picked up by
   hid_read(). */
static void hid_report_callback(void *context, IOReturn result, void *sender,
                         IOHIDReportType report_type, uint32_t report_id,
                         uint8_t *report, CFIndex report_length)
{
	struct input_report *rpt;
	hid_device *dev = context;
	
	/* Make a new Input Report object */
	rpt = calloc(1, sizeof(struct input_report));
	rpt->data = calloc(1, report_length);
	memcpy(rpt->data, report, report_length);
	rpt->len = report_length;
	rpt->next = NULL;
	
	/* Attach the new report object to the end of the list. */
	if (dev->input_reports == NULL) {
		/* The list is empty. Put it at the root. */
		dev->input_reports = rpt;
	}
	else {
		/* Find the end of the list and attach. */
		struct input_report *cur = dev->input_reports;
		while (cur->next != NULL)
			cur = cur->next;
		cur->next = rpt;
	}
	
	/* Stop the Run Loop. This is mostly used for when blocking is
	   enabled, but it doesn't hurt for non-blocking as well.  */
	CFRunLoopStop(CFRunLoopGetCurrent());
}

hid_device * HID_API_EXPORT hid_open_path(const char *path)
{
  	int i;
	hid_device *dev = NULL;
	CFIndex num_devices;
	
	dev = new_hid_device();

	/* Set up the HID Manager if it hasn't been done */
	if (!hid_mgr)
		init_hid_manager();

	CFSetRef device_set = IOHIDManagerCopyDevices(hid_mgr);
	
	num_devices = CFSetGetCount(device_set);
	IOHIDDeviceRef *device_array = calloc(num_devices, sizeof(IOHIDDeviceRef));
	CFSetGetValues(device_set, (const void **) device_array);	
	for (i = 0; i < num_devices; i++) {
		char cbuf[BUF_LEN];
		size_t len;
		IOHIDDeviceRef os_dev = device_array[i];
		
		len = make_path(os_dev, cbuf, sizeof(cbuf));
		if (!strcmp(cbuf, path)) {
			// Matched Paths. Open this Device.
			IOReturn ret = IOHIDDeviceOpen(os_dev, kIOHIDOptionsTypeNone);
			if (ret == kIOReturnSuccess) {
				char str[32];
				CFIndex max_input_report_len;

				free(device_array);
				CFRelease(device_set);
				dev->device_handle = os_dev;
				
				/* Create the buffers for receiving data */
				max_input_report_len = (CFIndex) get_max_report_length(os_dev);
				dev->input_report_buf = calloc(max_input_report_len, sizeof(uint8_t));
				
				/* Create the Run Loop Mode for this device.
				   printing the reference seems to work. */
				sprintf(str, "%p", os_dev);
				dev->run_loop_mode = 
					CFStringCreateWithCString(NULL, str, kCFStringEncodingASCII);
				
				/* Attach the device to a Run Loop */
				IOHIDDeviceScheduleWithRunLoop(os_dev, CFRunLoopGetCurrent(), dev->run_loop_mode);
				IOHIDDeviceRegisterInputReportCallback(
					os_dev, dev->input_report_buf, max_input_report_len,
					&hid_report_callback, dev);
				IOHIDManagerRegisterDeviceRemovalCallback(hid_mgr, hid_device_removal_callback, NULL);
				
				
				return dev;
			}
			else {
				goto return_error;
			}
		}
	}

return_error:
	free(device_array);
	CFRelease(device_set);
	free_hid_device(dev);
	return NULL;
}

static int set_report(hid_device *dev, IOHIDReportType type, const unsigned char *data, size_t length)
{
	const unsigned char *data_to_send;
	size_t length_to_send;
	IOReturn res;

	/* Return if the device has been disconnected. */
   	if (dev->disconnected)
   		return -1;

	if (data[0] == 0x0) {
		/* Not using numbered Reports.
		   Don't send the report number. */
		data_to_send = data+1;
		length_to_send = length-1;
	}
	else {
		/* Using numbered Reports.
		   Send the Report Number */
		data_to_send = data;
		length_to_send = length;
	}
	
	if (!dev->disconnected) {
		res = IOHIDDeviceSetReport(dev->device_handle,
					   type,
					   data[0], /* Report ID*/
					   data_to_send, length_to_send);
	
		if (res == kIOReturnSuccess) {
			return length;
		}
		else
			return -1;
	}
	
	return -1;
}

int HID_API_EXPORT hid_write(hid_device *dev, const unsigned char *data, size_t length)
{
	return set_report(dev, kIOHIDReportTypeOutput, data, length);
}

/* Helper function, so that this isn't duplicated in hid_read(). */
static int return_data(hid_device *dev, unsigned char *data, size_t length)
{
	/* Copy the data out of the linked list item (rpt) into the
	   return buffer (data), and delete the liked list item. */
	struct input_report *rpt = dev->input_reports;
	size_t len = (length < rpt->len)? length: rpt->len;
	memcpy(data, rpt->data, len);
	dev->input_reports = rpt->next;
	free(rpt->data);
	free(rpt);
	return len;
}

int HID_API_EXPORT hid_read(hid_device *dev, unsigned char *data, size_t length)
{
	int ret_val = -1;

	/* Lock this function */
	pthread_mutex_lock(&dev->mutex);
	
	/* There's an input report queued up. Return it. */
	if (dev->input_reports) {
		/* Return the first one */
		ret_val = return_data(dev, data, length);
		goto ret;
	}

	/* Return if the device has been disconnected. */
   	if (dev->disconnected) {
   		ret_val = -1;
   		goto ret;
	}
	
	/* There are no input reports queued up.
	   Need to get some from the OS. */

	/* Move the device's run loop to this thread. */
	IOHIDDeviceScheduleWithRunLoop(dev->device_handle, CFRunLoopGetCurrent(), dev->run_loop_mode);
	
	if (dev->blocking) {
		/* Run the Run Loop until it stops timing out. In other
		   words, until something happens. This is necessary because
		   there is no INFINITE timeout value. */
		SInt32 code;
		while (1) {
			code = CFRunLoopRunInMode(dev->run_loop_mode, 1000, TRUE);
			
			/* Return if the device has been disconnected */
			if (code == kCFRunLoopRunFinished) {
				dev->disconnected = 1;
				ret_val = -1;
				goto ret;
			}


			/* Return if some data showed up. */
			if (dev->input_reports)
				break;
			
			/* Break if The Run Loop returns Finished or Stopped. */
			if (code != kCFRunLoopRunTimedOut &&
			    code != kCFRunLoopRunHandledSource)
				break;
		}
		
		/* See if the run loop and callback gave us any reports. */
		if (dev->input_reports) {
			ret_val = return_data(dev, data, length);
			goto ret;
		}
		else {
			dev->disconnected = 1;
			ret_val = -1; /* An error occured (maybe CTRL-C?). */
			goto ret;
		}
	}
	else {
		/* Non-blocking. See if the OS has any reports to give. */
		SInt32 code;
		code = CFRunLoopRunInMode(dev->run_loop_mode, 0, TRUE);
		if (code == kCFRunLoopRunFinished) {
			/* The run loop is finished, indicating an error
			   or the device had been disconnected. */
			dev->disconnected = 1;
			ret_val = -1;
			goto ret;
		}
		if (dev->input_reports) {
			/* Return the first one */
			ret_val = return_data(dev, data, length);
			goto ret;
		}
		else {
			ret_val = 0; /* No data*/
			goto ret;
		}
	}

ret:
	/* Unlock */
	pthread_mutex_unlock(&dev->mutex);
	return ret_val;
}

int HID_API_EXPORT hid_set_nonblocking(hid_device *dev, int nonblock)
{
	/* All Nonblocking operation is handled by the library. */
	dev->blocking = !nonblock;
	
	return 0;
}

int HID_API_EXPORT hid_send_feature_report(hid_device *dev, const unsigned char *data, size_t length)
{
	return set_report(dev, kIOHIDReportTypeFeature, data, length);
}

int HID_API_EXPORT hid_get_feature_report(hid_device *dev, unsigned char *data, size_t length)
{
	CFIndex len = length;
	IOReturn res;

	/* Return if the device has been unplugged. */
	if (dev->disconnected)
		return -1;

	res = IOHIDDeviceGetReport(dev->device_handle,
	                           kIOHIDReportTypeFeature,
	                           data[0], /* Report ID */
	                           data, &len);
	if (res == kIOReturnSuccess)
		return len;
	else
		return -1;
}


void HID_API_EXPORT hid_close(hid_device *dev)
{
	if (!dev)
		return;
	
	/* Close the OS handle to the device, but only if it's not
	   been unplugged. If it's been unplugged, then calling
	   IOHIDDeviceClose() will crash. */
	if (!dev->disconnected) {
		IOHIDDeviceClose(dev->device_handle, kIOHIDOptionsTypeNone);
	}

	free_hid_device(dev);
}

int HID_API_EXPORT_CALL hid_get_manufacturer_string(hid_device *dev, wchar_t *string, size_t maxlen)
{
	return get_manufacturer_string(dev->device_handle, string, maxlen);
}

int HID_API_EXPORT_CALL hid_get_product_string(hid_device *dev, wchar_t *string, size_t maxlen)
{
	return get_product_string(dev->device_handle, string, maxlen);
}

int HID_API_EXPORT_CALL hid_get_serial_number_string(hid_device *dev, wchar_t *string, size_t maxlen)
{
	return get_serial_number(dev->device_handle, string, maxlen);
}

int HID_API_EXPORT_CALL hid_get_indexed_string(hid_device *dev, int string_index, wchar_t *string, size_t maxlen)
{
	// TODO:

	return 0;
}


HID_API_EXPORT const wchar_t * HID_API_CALL  hid_error(hid_device *dev)
{
	// TODO:

	return NULL;
}


#if 0
static int32_t get_location_id(IOHIDDeviceRef device)
{
	return get_int_property(device, CFSTR(kIOHIDLocationIDKey));
}

static int32_t get_usage(IOHIDDeviceRef device)
{
	int32_t res;
	res = get_int_property(device, CFSTR(kIOHIDDeviceUsageKey));
	if (!res)
		res = get_int_property(device, CFSTR(kIOHIDPrimaryUsageKey));
	return res;
}

static int32_t get_usage_page(IOHIDDeviceRef device)
{
	int32_t res;
	res = get_int_property(device, CFSTR(kIOHIDDeviceUsagePageKey));
	if (!res)
		res = get_int_property(device, CFSTR(kIOHIDPrimaryUsagePageKey));
	return res;
}

static int get_transport(IOHIDDeviceRef device, wchar_t *buf, size_t len)
{
	return get_string_property(device, CFSTR(kIOHIDTransportKey), buf, len);
}


int main(void)
{
	IOHIDManagerRef mgr;
	int i;
	
	mgr = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
	IOHIDManagerSetDeviceMatching(mgr, NULL);
	IOHIDManagerOpen(mgr, kIOHIDOptionsTypeNone);
	
	CFSetRef device_set = IOHIDManagerCopyDevices(mgr);
	
	CFIndex num_devices = CFSetGetCount(device_set);
	IOHIDDeviceRef *device_array = calloc(num_devices, sizeof(IOHIDDeviceRef));
	CFSetGetValues(device_set, (const void **) device_array);
	
	setlocale(LC_ALL, "");
	
	for (i = 0; i < num_devices; i++) {
		IOHIDDeviceRef dev = device_array[i];
		printf("Device: %p\n", dev);
		printf("  %04hx %04hx\n", get_vendor_id(dev), get_product_id(dev));

		wchar_t serial[256], buf[256];
		char cbuf[256];
		get_serial_number(dev, serial, 256);

		
		printf("  Serial: %ls\n", serial);
		printf("  Loc: %ld\n", get_location_id(dev));
		get_transport(dev, buf, 256);
		printf("  Trans: %ls\n", buf);
		make_path(dev, cbuf, 256);
		printf("  Path: %s\n", cbuf);
		
	}
	
	return 0;
}
#endif
