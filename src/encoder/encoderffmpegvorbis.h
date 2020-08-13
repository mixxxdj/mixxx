// FFMPEG OGG/Vorbis encoder for mixxx
#pragma once

#include "encoderffmpegcore.h"

class EncoderFfmpegCore;

class EncoderFfmpegVorbis : public EncoderFfmpegCore {
public:
    EncoderFfmpegVorbis(EncoderCallback* pCallback=NULL);
};
