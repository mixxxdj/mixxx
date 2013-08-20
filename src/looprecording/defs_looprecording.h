//  defs_looprecording.h
//  Created by Carl Pillot on 6/22/13.

#ifndef __LOOPRECORDING_DEFS_H__
#define __LOOPRECORDING_DEFS_H__

#define LOOP_RECORDING_PREF_KEY "[Loop_Recording]"
#define LOOP_ENCODING_WAVE "WAV"
#define LOOP_ENCODING_AIFF "AIFF"

#define LOOP_RECORD_OFF 0.0f
#define LOOP_RECORD_READY 1.0f
#define LOOP_RECORD_ON 2.0f
#define LOOP_RECORD_CLEAR 3.0f

// Arbitrary loop buffer length
#define LOOP_BUFFER_LENGTH 500000

#define INPUT_MASTER 0.0f
#define INPUT_HEAD 1.0f
#define INPUT_MICROPHONE 2.0f
#define INPUT_PT1 3.0f
#define INPUT_PT2 4.0f
#define INPUT_DECK_BASE 100.0f
#define INPUT_SAMPLER_BASE 200.f

#define LOOP_RECORDING_DIR "/temp"

#endif
