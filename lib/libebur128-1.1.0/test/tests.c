/* See COPYING file for copyright and license details. */

#include <math.h>
#include <sndfile.h>
#include <string.h>
#include <stdlib.h>

#include "ebur128.h"

double test_global_loudness(const char* filename) {
  SF_INFO file_info;
  SNDFILE* file;
  sf_count_t nr_frames_read;

  ebur128_state* st = NULL;
  double gated_loudness;
  double* buffer;

  memset(&file_info, '\0', sizeof(file_info));
  file = sf_open(filename, SFM_READ, &file_info);
  if (!file) {
    fprintf(stderr, "Could not open file %s!\n", filename);
    return 0.0;
  }
  st = ebur128_init((size_t) file_info.channels,
                    (size_t) file_info.samplerate,
                    EBUR128_MODE_I);
  if (file_info.channels == 5) {
    ebur128_set_channel(st, 0, EBUR128_LEFT);
    ebur128_set_channel(st, 1, EBUR128_RIGHT);
    ebur128_set_channel(st, 2, EBUR128_CENTER);
    ebur128_set_channel(st, 3, EBUR128_LEFT_SURROUND);
    ebur128_set_channel(st, 4, EBUR128_RIGHT_SURROUND);
  }
  buffer = (double*) malloc(st->samplerate * st->channels * sizeof(double));
  while ((nr_frames_read = sf_readf_double(file, buffer,
                                           (sf_count_t) st->samplerate))) {
    ebur128_add_frames_double(st, buffer, (size_t) nr_frames_read);
  }

  ebur128_loudness_global(st, &gated_loudness);

  /* clean up */
  ebur128_destroy(&st);

  free(buffer);
  buffer = NULL;
  if (sf_close(file)) {
    fprintf(stderr, "Could not close input file!\n");
  }
  return gated_loudness;
}

double test_loudness_range(const char* filename) {
  SF_INFO file_info;
  SNDFILE* file;
  sf_count_t nr_frames_read;

  ebur128_state* st = NULL;
  double loudness_range;
  double* buffer;

  memset(&file_info, '\0', sizeof(file_info));
  file = sf_open(filename, SFM_READ, &file_info);
  if (!file) {
    fprintf(stderr, "Could not open file %s!\n", filename);
    return 0.0;
  }
  st = ebur128_init((size_t) file_info.channels,
                    (size_t) file_info.samplerate,
                    EBUR128_MODE_LRA);
  if (file_info.channels == 5) {
    ebur128_set_channel(st, 0, EBUR128_LEFT);
    ebur128_set_channel(st, 1, EBUR128_RIGHT);
    ebur128_set_channel(st, 2, EBUR128_CENTER);
    ebur128_set_channel(st, 3, EBUR128_LEFT_SURROUND);
    ebur128_set_channel(st, 4, EBUR128_RIGHT_SURROUND);
  }
  buffer = (double*) malloc(st->samplerate * st->channels * sizeof(double));
  while ((nr_frames_read = sf_readf_double(file, buffer,
                                           (sf_count_t) st->samplerate))) {
    ebur128_add_frames_double(st, buffer, (size_t) nr_frames_read);
  }

  ebur128_loudness_range(st, &loudness_range);

  /* clean up */
  ebur128_destroy(&st);

  free(buffer);
  buffer = NULL;
  if (sf_close(file)) {
    fprintf(stderr, "Could not close input file!\n");
  }
  return loudness_range;
}

double test_true_peak(const char* filename) {
  SF_INFO file_info;
  SNDFILE* file;
  sf_count_t nr_frames_read;

  ebur128_state* st = NULL;
  double true_peak;
  double* buffer;

  memset(&file_info, '\0', sizeof(file_info));
  file = sf_open(filename, SFM_READ, &file_info);
  if (!file) {
    fprintf(stderr, "Could not open file %s!\n", filename);
    return 0.0;
  }
  st = ebur128_init((size_t) file_info.channels,
                    (size_t) file_info.samplerate,
                    EBUR128_MODE_LRA);
  if (file_info.channels == 5) {
    ebur128_set_channel(st, 0, EBUR128_LEFT);
    ebur128_set_channel(st, 1, EBUR128_RIGHT);
    ebur128_set_channel(st, 2, EBUR128_CENTER);
    ebur128_set_channel(st, 3, EBUR128_LEFT_SURROUND);
    ebur128_set_channel(st, 4, EBUR128_RIGHT_SURROUND);
  }
  buffer = (double*) malloc(st->samplerate * st->channels * sizeof(double));
  while ((nr_frames_read = sf_readf_double(file, buffer,
                                           (sf_count_t) st->samplerate))) {
    ebur128_add_frames_double(st, buffer, (size_t) nr_frames_read);
  }

  ebur128_loudness_range(st, &true_peak);

  /* clean up */
  ebur128_destroy(&st);

  free(buffer);
  buffer = NULL;
  if (sf_close(file)) {
    fprintf(stderr, "Could not close input file!\n");
  }
  return true_peak;
}

double gr[] = {-23.0,
               -33.0,
               -23.0,
               -23.0,
               -23.0,
               -23.0,
               -23.0,
               -23.0,
               -23.0};
double gre[] = {-2.2953556442089987e+01,
                -3.2959860397340044e+01,
                -2.2995899818255047e+01,
                -2.3035918615414182e+01,
                -2.2949997446096436e+01,
                -2.3017157781104373e+01,
                -2.3017157781104373e+01,
                -2.2980242495081757e+01,
                -2.3009077718930545e+01};
double lra[] = {10.0,
                 5.0,
                20.0,
                15.0,
                 5.0,
                15.0};
double lrae[] = {1.0001105488329134e+01,
                 4.9993734051522178e+00,
                 1.9995064067783115e+01,
                 1.4999273937723455e+01,
                 4.9747585878473721e+00,
                 1.4993650849123316e+01};


int main() {
  double result;

  fprintf(stderr, "Note: the tests do not have to pass with EXACT_PASSED.\n"
                  "Passing these tests does not mean that the library is "
                  "100%% EBU R 128 compliant!\n\n");

#define TEST_GLOBAL_LOUDNESS(filename, i)                                      \
  result = test_global_loudness(filename);                                     \
  if (result == result) {                                                      \
    printf("%s, %s - %s: %1.16e\n",                                            \
       (result <= gr[i] + 0.1 && result >= gr[i] - 0.1) ? "PASSED" : "FAILED", \
       (result == gre[i]) ?  "EXACT_PASSED" : "EXACT_FAILED",                  \
       filename, result);                                                      \
  }

  TEST_GLOBAL_LOUDNESS("seq-3341-1-16bit.wav", 0)
  TEST_GLOBAL_LOUDNESS("seq-3341-2-16bit.wav", 1)
  TEST_GLOBAL_LOUDNESS("seq-3341-3-16bit-v02.wav", 2)
  TEST_GLOBAL_LOUDNESS("seq-3341-4-16bit-v02.wav", 3)
  TEST_GLOBAL_LOUDNESS("seq-3341-5-16bit-v02.wav", 4)
  TEST_GLOBAL_LOUDNESS("seq-3341-6-5channels-16bit.wav", 5)
  TEST_GLOBAL_LOUDNESS("seq-3341-6-6channels-WAVEEX-16bit.wav", 6)
  TEST_GLOBAL_LOUDNESS("seq-3341-7_seq-3342-5-24bit.wav", 7)
  TEST_GLOBAL_LOUDNESS("seq-3341-2011-8_seq-3342-6-24bit-v02.wav", 8)


#define TEST_LRA(filename, i)                                                  \
  result = test_loudness_range(filename);                                      \
  if (result == result) {                                                      \
    printf("%s, %s - %s: %1.16e\n",                                            \
       (result <= lra[i] + 1 && result >= lra[i] - 1) ? "PASSED" : "FAILED",   \
       (result == lrae[i]) ?  "EXACT_PASSED" : "EXACT_FAILED",                 \
       filename, result);                                                      \
  }

  TEST_LRA("seq-3342-1-16bit.wav", 0)
  TEST_LRA("seq-3342-2-16bit.wav", 1)
  TEST_LRA("seq-3342-3-16bit.wav", 2)
  TEST_LRA("seq-3342-4-16bit.wav", 3)
  TEST_LRA("seq-3341-7_seq-3342-5-24bit.wav", 4)
  TEST_LRA("seq-3341-2011-8_seq-3342-6-24bit-v02.wav", 5)

  return 0;
}
