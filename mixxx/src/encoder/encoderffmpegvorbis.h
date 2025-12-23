#pragma once

#include "encoderffmpegcore.h"

class EncoderFfmpegCore;

/// FFmpeg OGG/Vorbis encoder
class EncoderFfmpegVorbis : public EncoderFfmpegCore {
public:
    EncoderFfmpegVorbis(EncoderCallback* pCallback=NULL);
};
