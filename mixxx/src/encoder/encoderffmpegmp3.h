#pragma once

#include "encoderffmpegcore.h"

class EncoderFfmpegCore;

// FFmpeg MP3 encoder
class EncoderFfmpegMp3 : public EncoderFfmpegCore {
public:
    EncoderFfmpegMp3(EncoderCallback* pCallback=NULL);
};
