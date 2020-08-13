// FFMPEG MP3 encoder for mixxx
#pragma once

#include "encoderffmpegcore.h"

class EncoderFfmpegCore;

class EncoderFfmpegMp3 : public EncoderFfmpegCore {
public:
    EncoderFfmpegMp3(EncoderCallback* pCallback=NULL);
};
