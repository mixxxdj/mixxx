/**
 * Copyright (c) 2015-2018, Martin Roth (mhroth@gmail.com)
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _TINY_OSC_
#define _TINY_OSC_

#include <stdbool.h>
#include <stdint.h>

#define TINYOSC_TIMETAG_IMMEDIATELY 1L

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tosc_message {
  char *format;  // a pointer to the format field
  char *marker;  // the current read head
  char *buffer;  // the original message data (also points to the address)
  uint32_t len;  // length of the buffer data
} tosc_message;

typedef struct tosc_bundle {
  char *marker; // the current write head (where the next message will be written)
  char *buffer; // the original buffer
  uint32_t bufLen; // the byte length of the original buffer
  uint32_t bundleLen; // the byte length of the total bundle
} tosc_bundle;



/**
 * Returns true if the buffer refers to a bundle of OSC messages. False otherwise.
 */
bool tosc_isBundle(const char *buffer);

/**
 * Reads a buffer containing a bundle of OSC messages.
 */
void tosc_parseBundle(tosc_bundle *b, char *buffer, const int len);

/**
 * Returns the timetag of an OSC bundle.
 */
uint64_t tosc_getTimetag(tosc_bundle *b);

/**
 * Parses the next message in a bundle. Returns true if successful.
 * False otherwise.
 */
bool tosc_getNextMessage(tosc_bundle *b, tosc_message *o);

/**
 * Returns a point to the address block of the OSC buffer.
 * This is also the start of the buffer.
 */
char *tosc_getAddress(tosc_message *o);

/**
 * Returns a pointer to the format block of the OSC buffer.
 */
char *tosc_getFormat(tosc_message *o);

/**
 * Returns the length in bytes of this message.
 */
uint32_t tosc_getLength(tosc_message *o);

/**
 * Returns the next 32-bit int. Does not check buffer bounds.
 */
int32_t tosc_getNextInt32(tosc_message *o);

/**
 * Returns the next 64-bit int. Does not check buffer bounds.
 */
int64_t tosc_getNextInt64(tosc_message *o);

/**
 * Returns the next 64-bit timetag. Does not check buffer bounds.
 */
uint64_t tosc_getNextTimetag(tosc_message *o);

/**
 * Returns the next 32-bit float. Does not check buffer bounds.
 */
float tosc_getNextFloat(tosc_message *o);

/**
 * Returns the next 64-bit float. Does not check buffer bounds.
 */
double tosc_getNextDouble(tosc_message *o);

/**
 * Returns the next string, or NULL if the buffer length is exceeded.
 */
const char *tosc_getNextString(tosc_message *o);

/**
 * Points the given buffer pointer to the next blob.
 * The len pointer is set to the length of the blob.
 * Returns NULL and 0 if the OSC buffer bounds are exceeded.
 */
void tosc_getNextBlob(tosc_message *o, const char **buffer, int *len);

/**
 * Returns the next set of midi bytes. Does not check bounds.
 * Bytes from MSB to LSB are: port id, status byte, data1, data2.
 */
unsigned char *tosc_getNextMidi(tosc_message *o);

/**
 * Resets the read head to the first element.
 *
 * @return  The same tosc_message pointer.
 */
tosc_message *tosc_reset(tosc_message *o);

/**
 * Parse a buffer containing an OSC message.
 * The contents of the buffer are NOT copied.
 * The tosc_message struct only points at relevant parts of the original buffer.
 * Returns 0 if there is no error. An error code (a negative number) otherwise.
 */
int tosc_parseMessage(tosc_message *o, char *buffer, const int len);

/**
 * Starts writing a bundle to the given buffer with length.
 */
void tosc_writeBundle(tosc_bundle *b, uint64_t timetag, char *buffer, const int len);

/**
 * Write a message to a bundle buffer. Returns the number of bytes written.
 */
uint32_t tosc_writeNextMessage(tosc_bundle *b,
    const char *address, const char *format, ...);

/**
 * Returns the length in bytes of the bundle.
 */
uint32_t tosc_getBundleLength(tosc_bundle *b);

/**
 * Writes an OSC packet to a buffer. Returns the total number of bytes written.
 * The entire buffer is cleared before writing.
 */
uint32_t tosc_writeMessage(char *buffer, const int len, const char *address,
    const char *fmt, ...);

/**
 * A convenience function to (non-destructively) print a buffer containing
 * an OSC message to stdout.
 */
void tosc_printOscBuffer(char *buffer, const int len);

/**
 * A convenience function to (non-destructively) print a pre-parsed OSC message
 * to stdout.
 */
void tosc_printMessage(tosc_message *o);

#ifdef __cplusplus
}
#endif

#endif // _TINY_OSC_
