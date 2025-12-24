#include "encoder/encoderffmpegvorbis.h"

// Constructor
#if LIBAVCODEC_VERSION_INT > 3544932
EncoderFfmpegVorbis::EncoderFfmpegVorbis(EncoderCallback* pCallback) : EncoderFfmpegCore(pCallback, AV_CODEC_ID_VORBIS)
#else
EncoderFfmpegVorbis::EncoderFfmpegVorbis(EncoderCallback* pCallback) : EncoderFfmpegCore(pCallback, CODEC_ID_VORBIS)
#endif
{
}
