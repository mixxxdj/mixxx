#ifndef TIMECODER_MK2_H

#define TIMECODER_MK2_H

#include "lut_mk2.h"
#include "timecoder.h"

mk2bits_t fwd_mk2(mk2bits_t current, struct timecode_def *def);
mk2bits_t rev_mk2(mk2bits_t current, struct timecode_def *def);
int build_lookup_mk2(struct timecode_def *def);

void mk2_process_carrier(struct timecoder *tc, signed int primary, signed int secondary);
void mk2_process_timecode(struct timecoder *tc, signed int reading);

#endif /* end of include guard TIMECODER_MK2_H */
