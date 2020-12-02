#include "encoder/encoderffmpegmp3.h"

// Constructor
#if LIBAVCODEC_VERSION_INT > 3544932
EncoderFfmpegMp3::EncoderFfmpegMp3(EncoderCallback* pCallback) : EncoderFfmpegCore(pCallback, AV_CODEC_ID_MP3)
#else
EncoderFfmpegMp3::EncoderFfmpegMp3(EncoderCallback* pCallback) : EncoderFfmpegCore(pCallback, CODEC_ID_MP3)
#endif
{
}
