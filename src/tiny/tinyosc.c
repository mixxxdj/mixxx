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

#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#if _WIN32
#include <winsock2.h>
#define tosc_strncpy(_dst, _src, _len) strncpy_s(_dst, _len, _src, _TRUNCATE)
#else
#include <netinet/in.h>
#define tosc_strncpy(_dst, _src, _len) strncpy(_dst, _src, _len)
#endif
#if __unix__ && !__APPLE__
#include <endian.h>
#define htonll(x) htobe64(x)
#define ntohll(x) be64toh(x)
#endif
#include "tinyosc.h"

#define BUNDLE_ID 0x2362756E646C6500L // "#bundle"

// http://opensoundcontrol.org/spec-1_0
int tosc_parseMessage(tosc_message *o, char *buffer, const int len) {
  // NOTE(mhroth): if there's a comma in the address, that's weird
  int i = 0;
  while (buffer[i] != '\0') ++i; // find the null-terimated address
  while (buffer[i] != ',') ++i; // find the comma which starts the format string
  if (i >= len) return -1; // error while looking for format string
  // format string is null terminated
  o->format = buffer + i + 1; // format starts after comma

  while (i < len && buffer[i] != '\0') ++i;
  if (i == len) return -2; // format string not null terminated

  i = (i + 4) & ~0x3; // advance to the next multiple of 4 after trailing '\0'
  o->marker = buffer + i;

  o->buffer = buffer;
  o->len = len;

  return 0;
}

// check if first eight bytes are '#bundle '
bool tosc_isBundle(const char *buffer) {
  return ((*(const int64_t *) buffer) == htonll(BUNDLE_ID));
}

void tosc_parseBundle(tosc_bundle *b, char *buffer, const int len) {
  b->buffer = (char *) buffer;
  b->marker = buffer + 16; // move past '#bundle ' and timetag fields
  b->bufLen = len;
  b->bundleLen = len;
}

uint64_t tosc_getTimetag(tosc_bundle *b) {
  return ntohll(*((uint64_t *) (b->buffer+8)));
}

uint32_t tosc_getBundleLength(tosc_bundle *b) {
  return b->bundleLen;
}

bool tosc_getNextMessage(tosc_bundle *b, tosc_message *o) {
  if ((b->marker - b->buffer) >= b->bundleLen) return false;
  uint32_t len = (uint32_t) ntohl(*((int32_t *) b->marker));
  tosc_parseMessage(o, b->marker+4, len);
  b->marker += (4 + len); // move marker to next bundle element
  return true;
}

char *tosc_getAddress(tosc_message *o) {
  return o->buffer;
}

char *tosc_getFormat(tosc_message *o) {
  return o->format;
}

uint32_t tosc_getLength(tosc_message *o) {
  return o->len;
}

int32_t tosc_getNextInt32(tosc_message *o) {
  // convert from big-endian (network btye order)
  const int32_t i = (int32_t) ntohl(*((uint32_t *) o->marker));
  o->marker += 4;
  return i;
}

int64_t tosc_getNextInt64(tosc_message *o) {
  const int64_t i = (int64_t) ntohll(*((uint64_t *) o->marker));
  o->marker += 8;
  return i;
}

uint64_t tosc_getNextTimetag(tosc_message *o) {
  return (uint64_t) tosc_getNextInt64(o);
}

float tosc_getNextFloat(tosc_message *o) {
  // convert from big-endian (network btye order)
  const uint32_t i = ntohl(*((uint32_t *) o->marker));
  o->marker += 4;
  return *((float *) (&i));
}

double tosc_getNextDouble(tosc_message *o) {
  const uint64_t i = ntohll(*((uint64_t *) o->marker));
  o->marker += 8;
  return *((double *) (&i));
}

const char *tosc_getNextString(tosc_message *o) {
  int i = (int) strlen(o->marker);
  if (o->marker + i >= o->buffer + o->len) return NULL;
  const char *s = o->marker;
  i = (i + 4) & ~0x3; // advance to next multiple of 4 after trailing '\0'
  o->marker += i;
  return s;
}

void tosc_getNextBlob(tosc_message *o, const char **buffer, int *len) {
  int i = (int) ntohl(*((uint32_t *) o->marker)); // get the blob length
  if (o->marker + 4 + i <= o->buffer + o->len) {
    *len = i; // length of blob
    *buffer = o->marker + 4;
    i = (i + 7) & ~0x3;
    o->marker += i;
  } else {
    *len = 0;
    *buffer = NULL;
  }
}

unsigned char *tosc_getNextMidi(tosc_message *o) {
  unsigned char *m = (unsigned char *) o->marker;
  o->marker += 4;
  return m;
}

tosc_message *tosc_reset(tosc_message *o) {
  int i = 0;
  while (o->format[i] != '\0') ++i;
  i = (i + 4) & ~0x3; // advance to the next multiple of 4 after trailing '\0'
  o->marker = o->format + i - 1; // -1 to account for ',' format prefix
  return o;
}

void tosc_writeBundle(tosc_bundle *b, uint64_t timetag, char *buffer, const int len) {
  *((uint64_t *) buffer) = htonll(BUNDLE_ID);
  *((uint64_t *) (buffer + 8)) = htonll(timetag);

  b->buffer = buffer;
  b->marker = buffer + 16;
  b->bufLen = len;
  b->bundleLen = 16;
}

// always writes a multiple of 4 bytes
static uint32_t tosc_vwrite(char *buffer, const int len,
    const char *address, const char *format, va_list ap) {
  memset(buffer, 0, len); // clear the buffer
  uint32_t i = (uint32_t) strlen(address);
  if (address == NULL || i >= len) return -1;
  tosc_strncpy(buffer, address, len);
  i = (i + 4) & ~0x3;
  buffer[i++] = ',';
  int s_len = (int) strlen(format);
  if (format == NULL || (i + s_len) >= len) return -2;
  tosc_strncpy(buffer+i, format, len-i-s_len);
  i = (i + 4 + s_len) & ~0x3;

  for (int j = 0; format[j] != '\0'; ++j) {
    switch (format[j]) {
      case 'b': {
        const uint32_t n = (uint32_t) va_arg(ap, int); // length of blob
        if (i + 4 + n > len) return -3;
        char *b = (char *) va_arg(ap, void *); // pointer to binary data
        *((uint32_t *) (buffer+i)) = htonl(n); i += 4;
        memcpy(buffer+i, b, n);
        i = (i + 3 + n) & ~0x3;
        break;
      }
      case 'f': {
        if (i + 4 > len) return -3;
        const float f = (float) va_arg(ap, double);
        *((uint32_t *) (buffer+i)) = htonl(*((uint32_t *) &f));
        i += 4;
        break;
      }
      case 'd': {
        if (i + 8 > len) return -3;
        const double f = (double) va_arg(ap, double);
        *((uint64_t *) (buffer+i)) = htonll(*((uint64_t *) &f));
        i += 8;
        break;
      }
      case 'i': {
        if (i + 4 > len) return -3;
        const uint32_t k = (uint32_t) va_arg(ap, int);
        *((uint32_t *) (buffer+i)) = htonl(k);
        i += 4;
        break;
      }
      case 'm': {
        if (i + 4 > len) return -3;
        const unsigned char *const k = (unsigned char *) va_arg(ap, void *);
        memcpy(buffer+i, k, 4);
        i += 4;
        break;
      }
      case 't':
      case 'h': {
        if (i + 8 > len) return -3;
        const uint64_t k = (uint64_t) va_arg(ap, long long);
        *((uint64_t *) (buffer+i)) = htonll(k);
        i += 8;
        break;
      }
      case 's': {
        const char *str = (const char *) va_arg(ap, void *);
        s_len = (int) strlen(str);
        if (i + s_len >= len) return -3;
        tosc_strncpy(buffer+i, str, len-i-s_len);
        i = (i + 4 + s_len) & ~0x3;
        break;
      }
      case 'T': // true
      case 'F': // false
      case 'N': // nil
      case 'I': // infinitum
          break;
      default: return -4; // unknown type
    }
  }

  return i; // return the total number of bytes written
}

uint32_t tosc_writeNextMessage(tosc_bundle *b,
    const char *address, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  if (b->bundleLen >= b->bufLen) return 0;
  const uint32_t i = tosc_vwrite(
      b->marker+4, b->bufLen-b->bundleLen-4, address, format, ap);
  va_end(ap);
  *((uint32_t *) b->marker) = htonl(i); // write the length of the message
  b->marker += (4 + i);
  b->bundleLen += (4 + i);
  return i;
}

uint32_t tosc_writeMessage(char *buffer, const int len,
    const char *address, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  const uint32_t i = tosc_vwrite(buffer, len, address, format, ap);
  va_end(ap);
  return i; // return the total number of bytes written
}

void tosc_printOscBuffer(char *buffer, const int len) {
  // parse the buffer contents (the raw OSC bytes)
  // a return value of 0 indicates no error
  tosc_message m;
  const int err = tosc_parseMessage(&m, buffer, len);
  if (err == 0) tosc_printMessage(&m);
  else printf("Error while reading OSC buffer: %i\n", err);
}

void tosc_printMessage(tosc_message *osc) {
  printf("[%i bytes] %s %s",
      osc->len, // the number of bytes in the OSC message
      tosc_getAddress(osc), // the OSC address string, e.g. "/button1"
      tosc_getFormat(osc)); // the OSC format string, e.g. "f"

  for (int i = 0; osc->format[i] != '\0'; i++) {
    switch (osc->format[i]) {
      case 'b': {
        const char *b = NULL; // will point to binary data
        int n = 0; // takes the length of the blob
        tosc_getNextBlob(osc, &b, &n);
        printf(" [%i]", n); // print length of blob
        for (int j = 0; j < n; ++j) printf("%02X", b[j] & 0xFF); // print blob bytes
        break;
      }
      case 'm': {
        unsigned char *m = tosc_getNextMidi(osc);
        printf(" 0x%02X%02X%02X%02X", m[0], m[1], m[2], m[3]);
        break;
      }
      case 'f': printf(" %g", tosc_getNextFloat(osc)); break;
      case 'd': printf(" %g", tosc_getNextDouble(osc)); break;
      case 'i': printf(" %d", tosc_getNextInt32(osc)); break;
      case 'h': printf(" %lld", tosc_getNextInt64(osc)); break;
      case 't': printf(" %lld", tosc_getNextTimetag(osc)); break;
      case 's': printf(" %s", tosc_getNextString(osc)); break;
      case 'F': printf(" false"); break;
      case 'I': printf(" inf"); break;
      case 'N': printf(" nil"); break;
      case 'T': printf(" true"); break;
      default: printf(" Unknown format: '%c'", osc->format[i]); break;
    }
  }
  printf("\n");
}
