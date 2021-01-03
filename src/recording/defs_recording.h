#pragma once

#define RECORDING_PREF_KEY "[Recording]"
#define ENCODING_WAVE "WAV"
#define ENCODING_FLAC "FLAC"
#define ENCODING_AIFF "AIFF"
#define ENCODING_OGG "OGG"
#define ENCODING_MP3 "MP3"
#define ENCODING_OPUS "Opus"

#define RECORD_OFF 0.0
#define RECORD_READY 1.0
#define RECORD_ON 2.0
#define RECORD_SPLIT_CONTINUE 3.0

//File options for preferences Splitting
#define SPLIT_650MB "650 MB (CD)"
#define SPLIT_700MB "700 MB (CD)"
#define SPLIT_1024MB "1 GB"
#define SPLIT_2048MB "2 GB"
#define SPLIT_4096MB "4 GB"
#define SPLIT_60MIN "60 Minutes"
#define SPLIT_74MIN "74 Minutes (CD)"
#define SPLIT_80MIN "80 Minutes (CD)"
#define SPLIT_120MIN "120 Minutes"

// Byte conversions. Slightly rounded to leave enough room to add
// closing frames by the encoder. All sizes are in bytes.
#define SIZE_650MB Q_UINT64_C(680'000'000)
#define SIZE_700MB Q_UINT64_C(730'000'000)
#define SIZE_1GB Q_UINT64_C(1'070'000'000)
#define SIZE_2GB Q_UINT64_C(2'140'000'000)
#define SIZE_4GB Q_UINT64_C(4'280'000'000)
