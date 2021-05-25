#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <unistd.h>

#include <string>

#include "ebur128.h"

static double malloc_fail_rate = 100.0;
static double malloc_fail_percentage = 0.995;

extern "C" void* my_malloc(size_t size) {
  if (size > 64 * 1024 * 1024) {
    return NULL;
  }

  if (!(rand() % ((int) malloc_fail_rate))) {
    return NULL;
  }

  malloc_fail_rate *= malloc_fail_percentage;
  if (malloc_fail_rate < 2) {
    malloc_fail_rate = 2;
  }

  return malloc(size);
}

extern "C" void* my_calloc(size_t nmemb, size_t size) {
  if (size * nmemb > 64 * 1024 * 1024) {
    return NULL;
  }

  if (!(rand() % ((int) malloc_fail_rate))) {
    return NULL;
  }

  malloc_fail_rate *= malloc_fail_percentage;
  if (malloc_fail_rate < 2) {
    malloc_fail_rate = 2;
  }

  return calloc(nmemb, size);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  unsigned int channels;
  unsigned long samplerate;
  int mode;
  unsigned int new_channels;
  unsigned long new_samplerate;
  unsigned long window;
  unsigned int random_seed;

  size_t data_offset = sizeof(channels) + sizeof(samplerate) + sizeof(mode) +
                       sizeof(new_channels) + sizeof(new_samplerate) +
                       sizeof(window) + sizeof(random_seed);

  if (size < data_offset) {
    return 0;
  }

  memcpy(&channels, data, sizeof(channels));
  data += sizeof(channels);
  memcpy(&samplerate, data, sizeof(samplerate));
  data += sizeof(samplerate);
  memcpy(&mode, data, sizeof(mode));
  data += sizeof(mode);
  memcpy(&new_channels, data, sizeof(new_channels));
  data += sizeof(new_channels);
  memcpy(&new_samplerate, data, sizeof(new_samplerate));
  data += sizeof(new_samplerate);
  memcpy(&window, data, sizeof(window));
  data += sizeof(window);
  memcpy(&random_seed, data, sizeof(random_seed));
  data += sizeof(random_seed);

  srand(random_seed);

  size -= data_offset;

  if (rand() % 5) {
    mode &= (1 << 7) - 1;
    mode |= EBUR128_MODE_TRUE_PEAK | EBUR128_MODE_HISTOGRAM;
    samplerate %= 96000;
    new_samplerate %= 96000;
    channels = 20;
    new_channels = 30;
  }

  if (new_channels < channels) {
    return 0;
  }

  ebur128_state* state = ebur128_init(channels, samplerate, mode);

  if (state) {
    if (ebur128_add_frames_int(state, (int const*) data,
                               size / channels / sizeof(int)) ||
        ebur128_set_max_window(state, window) ||
        ebur128_change_parameters(state, new_channels, new_samplerate) ||
        ebur128_add_frames_int(state, (int const*) data,
                               size / new_channels / sizeof(int))) {
      ebur128_destroy(&state);
      return 0;
    }

    ebur128_destroy(&state);
  }
  return 0;
}
