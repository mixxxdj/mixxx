#ifndef __RECORDING_DEFS_H__
#define __RECORDING_DEFS_H__

#define RECORDING_PREF_KEY "[Recording]"
#define ENCODING_WAVE "WAV"
#define ENCODING_FLAC "FLAC"
#define ENCODING_AIFF "AIFF"
#define ENCODING_OGG  "OGG"
#define ENCODING_MP3 "MP3"

#define RECORD_READY 0.50f
#define RECORD_ON 1.0f
#define RECORD_OFF 0.0f

//File options for preferences Splitting
#define SPLIT_650MB "650 MB (CD)"
#define SPLIT_700MB "700 MB (CD)"
#define SPLIT_1024MB "1 GB"
#define SPLIT_2048MB "2 GB"
#define SPLIT_4096MB "4 GB"

// Byte conversions
// Instead of multiplying megabytes with 1024 to get kilobytes
// I use 1000
// Once the recording size has reached
// there's enough room to add closing frames by
// the encoder
#define SIZE_650MB  650000000   //bytes
#define SIZE_700MB  750000000   //bytes
#define SIZE_1GB    1000000000  //bytes
#define SIZE_2GB    2000000000  //bytes
#define SIZE_4GB    4000000000l //bytes

#endif
