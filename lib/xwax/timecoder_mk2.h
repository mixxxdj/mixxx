#ifndef TIMECODER_MK2_H

#define TIMECODER_MK2_H

#include "timecoder.h"

int build_lookup_mk2(struct timecode_def *def);
int lut_load_mk2(struct timecode_def *def, const char *lut_dir_path);
int lut_store_mk2(struct timecode_def *def, const char *lut_dir_path);

void init_mk2_channel(struct timecoder_channel *ch);
void mk2_subcode_init(struct mk2_subcode *sc);

void mk2_process_bitstream(struct timecoder *tc, signed int reading);
void mk2_process_carrier(struct timecoder *tc,
        signed int primary, signed int secondary);

#endif /* end of include guard TIMECODER_MK2_H */

