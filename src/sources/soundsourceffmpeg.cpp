#include "sources/soundsourceffmpeg.h"

#include "encoder/encoderffmpegresample.h"

#include "util/logger.h"

#include <mutex>
#include <vector>

#define LIBAVCODEC_HAS_AV_PACKET_UNREF \
    (LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 8, 0))

#define AUDIOSOURCEFFMPEG_CACHESIZE 1000
#define AUDIOSOURCEFFMPEG_MIXXXFRAME_TO_BYTEOFFSET(numFrames) (frames2samples(numFrames) * sizeof(CSAMPLE))
#define AUDIOSOURCEFFMPEG_BYTEOFFSET_TO_MIXXXFRAME(byteOffset) (samples2frames(byteOffset / sizeof(CSAMPLE)))
#define AUDIOSOURCEFFMPEG_FILL_FROM_CURRENTPOS -1

namespace mixxx {

namespace {

const Logger kLogger("SoundSourceFFmpeg");

std::once_flag initFFmpegLibFlag;

// This function must be called once during startup.
void initFFmpegLib() {
    av_register_all();
    avcodec_register_all();
}

// More than 2 channels are currently not supported
const SINT kMaxChannelCount = 2;

inline
AVMediaType getMediaTypeOfStream(AVStream* pStream) {
    return m_pAVStreamWrapper.getMediaTypeOfStream(pStream);
}

AVStream* findFirstAudioStream(AVFormatContext* pFormatCtx) {
    DEBUG_ASSERT(pFormatCtx != nullptr);
    // Start search at the first stream
    unsigned int iNextStream = 0;
    while (iNextStream < pFormatCtx->nb_streams) {
        AVStream* pNextStream = pFormatCtx->streams[iNextStream];
        if (getMediaTypeOfStream(pNextStream) == AVMEDIA_TYPE_AUDIO) {
            return pNextStream;
        } else {
            // Continue search at the next stream
            ++iNextStream;
        }
    }
    return nullptr;
}

inline
AVCodec* findDecoderForStream(AVStream* pStream) {
    return m_pAVStreamWrapper.findDecoderForStream(pStream);
}

inline
mixxx::AudioSignal::ChannelCount getChannelCountOfStream(AVStream* pStream) {
    return mixxx::AudioSignal::ChannelCount(
            m_pAVStreamWrapper.getChannelCountOfStream(pStream));
}

inline
mixxx::AudioSignal::SampleRate getSampleRateOfStream(AVStream* pStream) {
    return mixxx::AudioSignal::SampleRate(
            m_pAVStreamWrapper.getSampleRateOfStream(pStream));
}

inline
bool getFrameIndexRangeOfStream(AVStream* pStream, mixxx::IndexRange* pFrameIndexRange) {
    // NOTE(uklotzde): Use 64-bit integer instead of floating point
    // calculations to minimize rounding errors
    DEBUG_ASSERT(pFrameIndexRange);
    DEBUG_ASSERT(pStream->duration >= 0);
    int64_t int64val = pStream->duration;
    if (int64val <= 0) {
        // Empty stream
        *pFrameIndexRange = mixxx::IndexRange();
        return true;
    }
    DEBUG_ASSERT(getSampleRateOfStream(pStream) > 0);
    int64val *= getSampleRateOfStream(pStream);
    VERIFY_OR_DEBUG_ASSERT(int64val > 0) {
        // Integer overflow
        kLogger.warning()
                << "Integer overflow during calculation of frame count";
        return false;
    }
    DEBUG_ASSERT(pStream->time_base.num > 0);
    int64val *= pStream->time_base.num;
    VERIFY_OR_DEBUG_ASSERT(int64val > 0) {
        // Integer overflow
        kLogger.warning()
                << "Integer overflow during calculation of frame count";
        return false;
    }
    DEBUG_ASSERT(pStream->time_base.den > 0);
    int64val /= pStream->time_base.den;
    SINT frameCount = int64val;
    VERIFY_OR_DEBUG_ASSERT(static_cast<int64_t>(frameCount) == int64val) {
        // Integer truncation
        kLogger.warning()
                << "Integer truncation during calculation of frame count";
        return false;
    }
    *pFrameIndexRange = mixxx::IndexRange::forward(0, frameCount);
    return true;
}

inline
AVSampleFormat getSampleFormatOfStream(AVStream* pStream) {
  return m_pAVStreamWrapper.getSampleFormatOfStream(pStream);
}

} // anonymous namespace

SoundSourceProviderFFmpeg::SoundSourceProviderFFmpeg() {
    std::call_once(initFFmpegLibFlag, initFFmpegLib);
}

QStringList SoundSourceProviderFFmpeg::getSupportedFileExtensions() const {
    QStringList list;
    AVInputFormat *l_SInputFmt  = nullptr;

    while ((l_SInputFmt = av_iformat_next(l_SInputFmt))) {
        if (l_SInputFmt->name == nullptr) {
            break; // exit loop
        }

        kLogger.debug() << "FFmpeg input format:" << l_SInputFmt->name;

        if (!strcmp(l_SInputFmt->name, "ac3")) {
            list.append("ac3");
        } else if (!strcmp(l_SInputFmt->name, "aiff")) {
                list.append("aif");
                list.append("aiff");
        } else if (!strcmp(l_SInputFmt->name, "caf")) {
            list.append("caf");
        } else if (!strcmp(l_SInputFmt->name, "flac")) {
            list.append("flac");
        } else if (!strcmp(l_SInputFmt->name, "ogg")) {
            list.append("ogg");
        } else if (!strcmp(l_SInputFmt->name, "mov,mp4,m4a,3gp,3g2,mj2")) {
            list.append("m4a");
            list.append("mp4");
        } else if (!strcmp(l_SInputFmt->name, "mp4")) {
            list.append("mp4");
        } else if (!strcmp(l_SInputFmt->name, "mp3")) {
            list.append("mp3");
        } else if (!strcmp(l_SInputFmt->name, "aac")) {
            list.append("aac");
        } else if (!strcmp(l_SInputFmt->name, "opus") ||
                   !strcmp(l_SInputFmt->name, "libopus")) {
            list.append("opus");
        } else if (!strcmp(l_SInputFmt->name, "tak")) {
            list.append("tak");
        } else if (!strcmp(l_SInputFmt->name, "tta")) {
            list.append("tta");
        } else if (!strcmp(l_SInputFmt->name, "wav")) {
            list.append("wav");
        } else if (!strcmp(l_SInputFmt->name, "wma") or
                   !strcmp(l_SInputFmt->name, "xwma")) {
            list.append("wma");
        } else if (!strcmp(l_SInputFmt->name, "wv")) {
            list.append("wv");
        }
    }

    return list;
}

//static
AVFormatContext* SoundSourceFFmpeg::openInputFile(
        const QString& fileName) {
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55, 69, 0)
    // TODO(XXX): Why do we need to allocate and partially initialize
    // the AVFormatContext struct before opening the input file???
    AVFormatContext *pInputFormatContext = avformat_alloc_context();
    if (pInputFormatContext == nullptr) {
        kLogger.warning()
                << "avformat_alloc_context() failed";
        return nullptr;
    }
    pInputFormatContext->max_analyze_duration = 999999999;
#else
    // Will be allocated implicitly when opening the input file
    AVFormatContext *pInputFormatContext = nullptr;
#endif

    // libav replaces open() with ff_win32_open() which accepts a
    // Utf8 path
    // see: avformat/os_support.h
    // The old method defining an URL_PROTOCOL is deprecated
#if defined(_WIN32) && !defined(__MINGW32CE__)
    const QByteArray qBAFilename(
            avformat_version() >= AV_VERSION_INT(52, 0, 0) ?
                    fileName.toUtf8() :
                    QFile::encodeName(fileName));
#else
    const QByteArray qBAFilename(QFile::encodeName(fileName));
#endif

    // Open input file and allocate/initialize AVFormatContext
    const int avformat_open_input_result =
            avformat_open_input(
                    &pInputFormatContext, qBAFilename.constData(), nullptr, nullptr);
    if (avformat_open_input_result != 0) {
        kLogger.warning()
                << "avformat_open_input() failed and returned"
                << avformat_open_input_result;
        DEBUG_ASSERT(pInputFormatContext == nullptr);
    }
    return pInputFormatContext;
}

void SoundSourceFFmpeg::ClosableInputAVFormatContextPtr::take(
        AVFormatContext** ppClosableInputFormatContext) {
    DEBUG_ASSERT(ppClosableInputFormatContext != nullptr);
    if (m_pClosableInputFormatContext != *ppClosableInputFormatContext) {
        close();
        m_pClosableInputFormatContext = *ppClosableInputFormatContext;
        *ppClosableInputFormatContext = nullptr;
    }
}

void SoundSourceFFmpeg::ClosableInputAVFormatContextPtr::close() {
    if (m_pClosableInputFormatContext != nullptr) {
        avformat_close_input(&m_pClosableInputFormatContext);
        DEBUG_ASSERT(m_pClosableInputFormatContext == nullptr);
    }
}

//static
SoundSource::OpenResult SoundSourceFFmpeg::openAudioStream(
        AVCodecContext* pCodecContext, AVCodec *pDecoder) {
    DEBUG_ASSERT(pCodecContext != nullptr);

    const int avcodec_open2_result = avcodec_open2(pCodecContext, pDecoder, nullptr);
    if (avcodec_open2_result < 0) {
        kLogger.warning()
                << "avcodec_open2() failed and returned"
                << avcodec_open2_result;
        return SoundSource::OpenResult::Failed;
    }
    return SoundSource::OpenResult::Succeeded;
}

void SoundSourceFFmpeg::ClosableAVStreamPtr::take(AVStream** ppClosableStream) {
    DEBUG_ASSERT(ppClosableStream != nullptr);
    if (m_pClosableStream != *ppClosableStream) {
        close();
        m_pClosableStream = *ppClosableStream;
        *ppClosableStream = nullptr;
    }
}

void SoundSourceFFmpeg::ClosableAVStreamPtr::close() {
    if (m_pClosableStream != nullptr) {
#if ! AVSTREAM_FROM_API_VERSION_3_1
        const int avcodec_close_result = avcodec_close(m_pClosableStream->codec);
        if (avcodec_close_result != 0) {
            kLogger.warning()
                    << "avcodec_close() failed and returned"
                    << avcodec_close_result;
            // ignore error and continue
        }
#endif
        m_pClosableStream = nullptr;
    }
}

#if AVSTREAM_FROM_API_VERSION_3_1
void SoundSourceFFmpeg::ClosableAVCodecContextPtr::take(AVCodecContext** ppClosableContext) {
    DEBUG_ASSERT(ppClosableContext != nullptr);
    if (m_pClosableContext != *ppClosableContext) {
        close();
        m_pClosableContext = *ppClosableContext;
        *ppClosableContext = nullptr;
    }
}

void SoundSourceFFmpeg::ClosableAVCodecContextPtr::close() {
    if (m_pClosableContext != nullptr) {
        avcodec_free_context(&m_pClosableContext);
        m_pClosableContext = nullptr;
    }
}
#endif

SoundSourceFFmpeg::SoundSourceFFmpeg(const QUrl& url)
    : SoundSource(url),
      m_pResample(nullptr),
      m_currentMixxxFrameIndex(0),
      m_bIsSeeked(false),
      m_lCacheFramePos(0),
      m_lCacheStartFrame(0),
      m_lCacheEndFrame(0),
      m_lCacheLastPos(0),
      m_lLastStoredPos(0),
      m_lStoreCount(0),
      m_lStoredSeekPoint(-1),
      m_SStoredJumpPoint(nullptr) {
}

SoundSourceFFmpeg::~SoundSourceFFmpeg() {
    close();
}

SoundSource::OpenResult SoundSourceFFmpeg::tryOpen(
        OpenMode /*mode*/,
        const OpenParams& /*config*/) {
    AVFormatContext *pInputFormatContext =
            openInputFile(getLocalFileName());
    if (pInputFormatContext == nullptr) {
        kLogger.warning()
                << "Failed to open input file"
                << getLocalFileName();
        return OpenResult::Failed;
    }
    m_pInputFormatContext.take(&pInputFormatContext);

    // Retrieve stream information
    const int avformat_find_stream_info_result =
            avformat_find_stream_info(m_pInputFormatContext, nullptr);
    if (avformat_find_stream_info_result < 0) {
        kLogger.warning()
                << "avformat_find_stream_info() failed and returned"
                << avformat_find_stream_info_result;
        return OpenResult::Failed;
    }

    //debug only (Enable if needed)
    //av_dump_format(m_pInputFormatContext, 0, qBAFilename.constData(), false);

    // Find and open audio stream for decoding
    AVStream* pAudioStream = findFirstAudioStream(m_pInputFormatContext);
    if (pAudioStream == nullptr) {
        kLogger.warning()
                << "No audio stream found";
        return OpenResult::Aborted;
    }

    // Find codec to decode stream or pass out
    AVCodec* pDecoder = findDecoderForStream(pAudioStream);
    if (pDecoder == nullptr) {
        kLogger.warning()
                << "Failed to find a decoder for stream"
                << pAudioStream->index;
        return SoundSource::OpenResult::Aborted;
    }

#if AVSTREAM_FROM_API_VERSION_3_1
    AVCodecContext *pCodecContext = avcodec_alloc_context3(pDecoder);

    if (pCodecContext == nullptr) {
        kLogger.warning()
                << "Failed to allocate codec context"
                << pAudioStream->index;
        return SoundSource::OpenResult::Aborted;
    }

    // Add stream parameters to context
    if (avcodec_parameters_to_context(pCodecContext,pAudioStream->codecpar)) {
        kLogger.warning()
                << "Failed to find to set Code parameter for AVCodecContext"
                << pAudioStream->index;
        return SoundSource::OpenResult::Aborted;
    }

    // Se timebase correct
    av_codec_set_pkt_timebase(pCodecContext, pAudioStream->time_base);

    // Make sure that Codecs are identical or avcodec_open2 fails.
    pCodecContext->codec_id = pDecoder->id;

    const OpenResult openAudioStreamResult = openAudioStream(pCodecContext, pDecoder);

    m_pAudioContext.take(&pCodecContext);
#else
    const OpenResult openAudioStreamResult = openAudioStream(pAudioStream->codec, pDecoder);
#endif


    if (openAudioStreamResult != OpenResult::Succeeded) {
        return openAudioStreamResult; // early exit on any error
    }

    // Now set the member, because the audio stream has been opened
    // successfully and needs to be closed eventually.
    m_pAudioStream.take(&pAudioStream);

    const auto channelCount = getChannelCountOfStream(m_pAudioStream);
    if (!channelCount.valid()) {
        kLogger.warning()
                << "Stream has invalid or unsupported number of channels:"
                << channelCount;
        return OpenResult::Aborted;
    }

    const auto sampleRate = getSampleRateOfStream(m_pAudioStream);
    if (!sampleRate.valid()) {
        kLogger.warning()
                << "Stream has invalid or unsupported sample rate:"
                << sampleRate;
        return OpenResult::Aborted;
    }

    mixxx::IndexRange frameIndexRange;
    if (!getFrameIndexRangeOfStream(m_pAudioStream, &frameIndexRange)) {
        kLogger.warning()
                << "Failed to get frame index range for stream";
        return OpenResult::Failed;
    }

    setChannelCount(channelCount);
    setSampleRate(sampleRate);
    initFrameIndexRangeOnce(frameIndexRange);

#if AVSTREAM_FROM_API_VERSION_3_1
    m_pResample = std::make_unique<EncoderFfmpegResample>(m_pAudioContext);
#else
    m_pResample = std::make_unique<EncoderFfmpegResample>(m_pAudioStream->codec);
#endif
    m_pResample->openMixxx(getSampleFormatOfStream(m_pAudioStream), AV_SAMPLE_FMT_FLT);

    return OpenResult::Succeeded;
}

void SoundSourceFFmpeg::close() {
    clearCache();

    m_pResample.reset();

    while (m_SJumpPoints.size() > 0) {
        ffmpegLocationObject* l_SRmJmp = m_SJumpPoints[0];
        m_SJumpPoints.remove(0);
        free(l_SRmJmp);
    }

#if AVSTREAM_FROM_API_VERSION_3_1
    m_pAudioContext.close();
#endif
    m_pAudioStream.close();
    m_pInputFormatContext.close();
}

void SoundSourceFFmpeg::clearCache() {
    while (m_SCache.size() > 0) {
        struct ffmpegCacheObject* l_SRmObj = m_SCache[0];
        m_SCache.remove(0);
        free(l_SRmObj->bytes);
        free(l_SRmObj);
    }
}

bool SoundSourceFFmpeg::readFramesToCache(unsigned int count, SINT offset) {
    unsigned int l_iCount = count;
    qint32 l_iRet = 0;
    AVPacket l_SPacket;
    l_SPacket.data = nullptr;
    l_SPacket.size = 0;
    AVFrame *l_pFrame = nullptr;
    bool l_bStop = false;
#if ! AVSTREAM_FROM_API_VERSION_3_1
    int l_iFrameFinished = 0;
#endif
    struct ffmpegCacheObject *l_SObj = nullptr;
    struct ffmpegCacheObject *l_SRmObj = nullptr;
    qint64 l_lLastPacketPos = -1;
    int l_iError = 0;
    int l_iFrameCount = 0;

    while (l_iCount > 0) {
        if (l_pFrame != nullptr) {
            l_iFrameCount--;
// FFMPEG 2.2 and beyond
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(54, 86, 100)
            av_frame_free(&l_pFrame);
// FFMPEG 0.11 and below
#elif LIBAVCODEC_VERSION_INT <= AV_VERSION_INT(54, 23, 100)
            av_free(l_pFrame);
// FFMPEG 1.0 - 2.1
#else
            avcodec_free_frame(&l_pFrame);
#endif
            l_pFrame = nullptr;

        }

        if (l_bStop) {
            break;
        }
        l_iFrameCount++;
        av_init_packet(&l_SPacket);
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55, 52, 0)
        l_pFrame = avcodec_alloc_frame();
#else
        l_pFrame = av_frame_alloc();
#endif

        if (l_pFrame == nullptr) {
            kLogger.debug() << "readFramesToCache: Can't alloc memory!";
            return false;
        }

        // Read one frame (which has nothing to do with Mixxx Frame)
        // it's some packed audio data from container like MP3, Ogg or MP4
        if (av_read_frame(m_pInputFormatContext, &l_SPacket) >= 0) {
            // Are we on correct audio stream. Currently we are always
            // Using first audio stream but in future there should be
            // possibility to choose which to use
            // If Pos is -1 it meand FFmpeg/AVConv doesn't know it
            // So then we use pts instead
            if (l_SPacket.stream_index == m_pAudioStream->index &&
                    (l_SPacket.pos >= 0 || l_SPacket.pos == -1)) {

                // Codecs like Wavpack does it like this
                // They work but you can say about position nothing
                if (l_SPacket.pos == -1)
                {
                   l_SPacket.pos = l_SPacket.pts;
                }
                if (m_lStoredSeekPoint > 0) {
                    struct ffmpegLocationObject *l_STestObj = nullptr;
                    if (m_SJumpPoints.size() > 0) {
                        l_STestObj = m_SJumpPoints.first();

                        if (l_STestObj->pos > l_SPacket.pos) {
                            continue;
                        }
                    }

                    // Seek for correct jump point
                    if (m_lStoredSeekPoint > l_SPacket.pos) {
#if (LIBAVCODEC_HAS_AV_PACKET_UNREF)
                        av_packet_unref(&l_SPacket);
#else
                        av_free_packet(&l_SPacket);
#endif
                        l_SPacket.data = nullptr;
                        l_SPacket.size = 0;
                        continue;
                    }
                    m_lStoredSeekPoint = -1;
                    m_SStoredJumpPoint = nullptr;
                }

#if AVSTREAM_FROM_API_VERSION_3_1
                l_iRet = avcodec_send_packet(m_pAudioContext, &l_SPacket);

                // AVERROR(EAGAIN) means that we need to feed more
                // That we can decode Frame or Packet
                if (l_iRet == AVERROR(EAGAIN)) {
                  kLogger.warning() << "readFramesToCache: Need more packets to decode!";
                  continue;
                }

                if (l_iRet == AVERROR_EOF || l_iRet == AVERROR(EINVAL)) {
                      kLogger.warning() << "readFramesToCache: Warning can't decode frame!";
                }

                l_iRet = avcodec_receive_frame(m_pAudioContext, l_pFrame);

                // AVERROR(EAGAIN) means that we need to feed more
                // That we can decode Frame or Packet
                if (l_iRet == AVERROR(EAGAIN)) {
                  kLogger.warning() << "readFramesToCache: Need more packets to decode!";
                  continue;
                }

                if (l_iRet == AVERROR_EOF || l_iRet == AVERROR(EINVAL)) {
                      kLogger.warning() << "readFramesToCache: Warning can't decode frame!";
                }

                if (l_iRet == AVERROR_EOF || l_iRet < 0) {
#else
                // Decode audio bytes (These can be S16P or FloatP [P is Planar])
                l_iRet = avcodec_decode_audio4(m_pAudioStream->codec,l_pFrame,&l_iFrameFinished,
                                               &l_SPacket);
                if (l_iRet <= 0) {
#endif
                    // An error or EOF occurred,index break out and return what
                    // we have so far.
                    kLogger.debug() << "readFramesToCache: EOF or uncoverable error!";
                    l_bStop = true;
                    continue;
                } else {
                    l_iRet = 0;
                    l_SObj = (struct ffmpegCacheObject *)malloc(sizeof(struct ffmpegCacheObject));
                    if (l_SObj == nullptr) {
                        kLogger.debug() << "readFramesToCache: Not enough memory!";
                        l_bStop = true;
                        continue;
                    }
                    memset(l_SObj, 0x00, sizeof(struct ffmpegCacheObject));

                    // Try to convert it to Mixxx understand output format
                    // which is pure Stereo Float
                    l_iRet = m_pResample->reSampleMixxx(l_pFrame, &l_SObj->bytes);

                    if (l_iRet > 0) {
                        // Remove from cache
                        if (m_SCache.size() >= (AUDIOSOURCEFFMPEG_CACHESIZE - 10)) {
                            l_SRmObj = m_SCache[0];
                            m_SCache.remove(0);
                            free(l_SRmObj->bytes);
                            free(l_SRmObj);
                        }

                        // Add to cache and store byte place to memory
                        m_SCache.append(l_SObj);
                        l_SObj->startFrame = m_lCacheFramePos;
                        l_SObj->length = l_iRet;
                        m_lCacheFramePos += AUDIOSOURCEFFMPEG_BYTEOFFSET_TO_MIXXXFRAME(l_iRet);

                        // Ogg/Opus have packages pos that have many
                        // audio frames so seek next unique pos..
                        if (l_SPacket.pos != l_lLastPacketPos) {
                            l_lLastPacketPos = l_SPacket.pos;
                            m_lStoreCount++;
                        }

                        // If we are over last storepos and we have read more than jump point need is
                        // and pos is unique we store it to memory
                        //
                        // So why number 32? Sorry I just made that up from the hat there is no
                        // math behind it. Number 32 it's not too big nor jumps points are not
                        // too close each other. Mainly it's ugly compromise with MP3,MP4,OGG and WMA
                        // different codec frame sizes
                        if (m_lStoreCount == 32) {
                            struct ffmpegLocationObject *l_STestObj = nullptr;

                            if (m_SJumpPoints.size() > 0) {
                                l_STestObj = m_SJumpPoints.last();
                            }
                            // Check whether we have this jumppoint stored already or not
                            // We should have jumppoints below that pos
                            if (l_STestObj == nullptr || l_STestObj->pos < l_SPacket.pos) {
                                struct ffmpegLocationObject  *l_SJmp = (struct ffmpegLocationObject  *)malloc(
                                        sizeof(struct ffmpegLocationObject));
                                m_lLastStoredPos = m_lCacheFramePos;
                                l_SJmp->startFrame = m_lCacheFramePos;
                                l_SJmp->pos = l_SPacket.pos;
                                l_SJmp->pts = l_SPacket.pts;
                                m_SJumpPoints.append(l_SJmp);
                            }
                            m_lStoreCount = 0;
                        }

                        if (offset < 0 || offset <= m_lCacheFramePos) {
                            l_iCount--;
                        }
                    } else {
                        free(l_SObj);
                        l_SObj = nullptr;
                        kLogger.debug() << "readFramesToCache: General error in audio decode:" <<
                                 l_iRet;
                    }
                }

#if (LIBAVCODEC_HAS_AV_PACKET_UNREF)
                av_packet_unref(&l_SPacket);
#else
                av_free_packet(&l_SPacket);
#endif
                l_SPacket.data = nullptr;
                l_SPacket.size = 0;

            } else {
                l_iError++;
                if (l_iError == 5) {
                    // Stream end and we couldn't read enough frames
                    l_bStop = true;
                }
            }


        } else {
            kLogger.debug() << "readFramesToCache: Packet too big or File end";
            l_bStop = true;
        }

    }

    if (l_pFrame != nullptr) {
        l_iFrameCount--;
// FFMPEG 2.2 and beyond
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(54, 86, 100)
        av_frame_unref(l_pFrame);
        av_frame_free(&l_pFrame);
// FFMPEG 0.11 and below
#elif LIBAVCODEC_VERSION_INT <= AV_VERSION_INT(54, 23, 100)
        av_free(l_pFrame);
// FFMPEG 1.0 - 2.1
#else
        avcodec_free_frame(&l_pFrame);
#endif
        l_pFrame = nullptr;

    }

    if (l_iFrameCount > 0) {
        kLogger.debug() << "readFramesToCache(): Frame balance is not 0 it is: " <<
                 l_iFrameCount;
    }

    if (m_SCache.isEmpty()) {
        kLogger.debug() << "readFramesToCache(): Can't read frames. Cache empty!";
        return false;
    }

    l_SObj = m_SCache.first();
    m_lCacheStartFrame = l_SObj->startFrame;
    l_SObj = m_SCache.last();
    m_lCacheEndFrame = (l_SObj->startFrame + l_SObj->length);

    if (!l_iCount) {
        return true;
    } else {
        return false;
    }
}

bool SoundSourceFFmpeg::getBytesFromCache(CSAMPLE* buffer, SINT offset,
        SINT size) {
    struct ffmpegCacheObject *l_SObj = nullptr;
    qint32 l_lPos = 0;
    quint32 l_lLeft = AUDIOSOURCEFFMPEG_MIXXXFRAME_TO_BYTEOFFSET(size);
    quint32 l_lOffset = 0;
    quint32 l_lBytesToCopy = 0;
    bool l_bEndOfFile = false;

    char *l_pBuffer = (char *)buffer;

    // If cache is empty then return without crash.
    if (m_SCache.isEmpty()) {
        kLogger.debug() << "getBytesFromCache: Cache is empty can't return bytes";
        if (l_pBuffer != nullptr)
        {
            memset(l_pBuffer, 0x00, l_lLeft);
        }
        return false;
    }

    // Is offset bigger than start of cache
    if (offset >= m_lCacheStartFrame) {
        int l_lTmpLen = 0;
        // If last pos is (which it shouldn't) use caches end
        if (m_lCacheLastPos == 0) {
            m_lCacheLastPos = m_SCache.size() - 1;
        }

        // Seek to correct FrameIndex (Minus 5 for faster seek)
        //
        // This could be done per steps but because there
        // Jump points can be far away and codec frames are small
        // just jump to point where is safe to start.
        for (l_lPos = m_lCacheLastPos; l_lPos >= 0; l_lPos -= 5) {
            l_SObj = m_SCache[l_lPos];

            // Because length is in byte we have to convert it to Frames
            l_lTmpLen = AUDIOSOURCEFFMPEG_BYTEOFFSET_TO_MIXXXFRAME(l_SObj->length);

            if ((l_SObj->startFrame + l_lTmpLen) < offset) {
                break;
            }
        }

        // Because we step 5 backward we can end up to below zero
        // We can't go further so hope for the best
        if (l_lPos < 0) {
            l_lPos = 0;
        }

        // This shouldn't never happen.. because it's nearly impossible
        // but because it can happen double check
        if (l_lPos >= m_SCache.size()) {
            l_lPos = m_SCache.size() - 1;
        }

        // Use this Cache object as starting point
        l_SObj = m_SCache[l_lPos];

        if (l_SObj == nullptr) {
            kLogger.debug() << "getBytesFromCache: Cache object nullptr";
            return false;
        }

        if (l_pBuffer == nullptr) {
            kLogger.debug() << "getBytesFromCache: Out buffer nullptr";
            return false;
        }

        while (l_lLeft > 0) {
            // If Cache is running low read more
            if ((l_lPos + 5) > m_SCache.size() && l_bEndOfFile == false) {
                offset = l_SObj->startFrame;
                // Read 50 frames from current pos. If we hit file end before that
                // exit
                if (readFramesToCache(50, AUDIOSOURCEFFMPEG_FILL_FROM_CURRENTPOS) == false) {
                    // File has ended.. don't try to cache anymore
                    // or some fatal error has occurred so.. just don't
                    l_bEndOfFile = true;
                }

                // Seek back to correct place
                for (l_lPos = (m_SCache.size() - 50); l_lPos > 0; l_lPos--) {
                    l_SObj = m_SCache[l_lPos];
                    if ((l_SObj->startFrame + l_SObj->length) < offset) {
                        break;
                    }
                }

                if (l_lPos < m_SCache.size() && l_lPos >= 0) {
                    l_SObj = m_SCache[l_lPos];
                    continue;
                } else if (l_lPos < 0) {
                    l_lPos = 0;
                } else {
                    l_SObj = m_SCache.last();
                    l_lPos = l_lPos < m_SCache.size() - 1;
                }
            }

            // If Cache object ain't correct then calculate offset
            if (l_SObj->startFrame <= offset) {
                // We have to convert again it to bytes
                l_lOffset = AUDIOSOURCEFFMPEG_MIXXXFRAME_TO_BYTEOFFSET(offset - l_SObj->startFrame);
            }

            // Okay somehow offset is bigger than our Cache object have bytes
            if (l_lOffset >= l_SObj->length) {
                if ((l_lPos + 1) < m_SCache.size()) {
                    l_SObj = m_SCache[++ l_lPos];
                    continue;
                } else {
                    kLogger.debug() << "getBytesFromCache: Buffer run out. Shouldn't happen!";
                    if (l_pBuffer != nullptr)
                    {
                        memset(l_pBuffer, 0x00, l_lLeft);
                    }
                    return false;
                }
            }

            // If bytes left is bigger than FFmpeg frame bytes available
            // then copy to buffer end and then jump to next FFmpeg frame
            // to understand this here are some examples
            //   * MP3 have size 2304 * 4
            //   * OGG/Opus size 256 - 1024
            //   * WMA size 32767 - 131070
            // and all these are separated in packets nor solid stream of bytes
            // that just can be copied to buffer
            // so that's why this kind of abstraction is needed
            if (l_lLeft > (l_SObj->length - l_lOffset)) {
                // calculate start point of copy
                l_lBytesToCopy = l_SObj->length - l_lOffset;
                memcpy(l_pBuffer, (l_SObj->bytes + l_lOffset), l_lBytesToCopy);
                l_lOffset = 0;
                l_pBuffer += l_lBytesToCopy;
                l_lLeft -= l_lBytesToCopy;
            } else {
                memcpy(l_pBuffer, (l_SObj->bytes + l_lOffset), l_lLeft);
                l_lLeft = 0;
            }

            // If we have more items of cache use them
            // or after that just zero buffer..
            if ((l_lPos + 1) < m_SCache.size()) {
                l_SObj = m_SCache[++ l_lPos];
            } else {
                // With MP3 VBR length of audio is just a guess
                // it's near good as it can get but it can be too long
                // so fill buffer with 0x00 (zero) that we don't get ugly
                // noise at the end of the file
                memset(l_pBuffer, 0x00, l_lLeft);
                l_lLeft = 0;
            }
        }

        m_lCacheLastPos = --l_lPos;
        return true;
    }

    return false;
}

ReadableSampleFrames SoundSourceFFmpeg::readSampleFramesClamped(
        WritableSampleFrames writableSampleFrames) {

    const SINT firstFrameIndex = writableSampleFrames.frameIndexRange().start();

    const SINT seekFrameIndex = firstFrameIndex;
    if ((m_currentMixxxFrameIndex != seekFrameIndex) || (m_SCache.size() == 0)) {
        int ret = 0;
        qint64 i = 0;
        struct ffmpegLocationObject *l_STestObj = nullptr;

        if (seekFrameIndex < m_lCacheStartFrame) {
            // Seek to set (start of the stream which is FFmpeg frame 0)
            // because we are dealing with compressed audio FFmpeg takes
            // best of to seek that point (in this case 0 Is always there)
            // in every other case we should provide MIN and MAX tolerance
            // which we can take.
            // FFmpeg just just can't take zero as MAX tolerance so we try to
            // just make some tolerable (which is never used because zero point
            // should always be there) some number (which is 0xffff 65535)
            // that is chosen because in WMA frames can be that big and if it's
            // smaller than the frame we are seeking we can get into error
            ret = avformat_seek_file(m_pInputFormatContext,
                                     m_pAudioStream->index,
                                     0,
                                     0,
                                     0xffff,
                                     AVSEEK_FLAG_BACKWARD);

            if (ret < 0) {
                kLogger.warning() << "seek: Can't seek to 0 byte!";
                return ReadableSampleFrames();
            }

            clearCache();
            m_lCacheStartFrame = 0;
            m_lCacheEndFrame = 0;
            m_lCacheLastPos = 0;
            m_lCacheFramePos = 0;
            m_lStoredSeekPoint = -1;


            // Try to find some jump point near to
            // where we are located so we don't needed
            // to try guess it
            if (m_SJumpPoints.size() > 0) {
                l_STestObj = m_SJumpPoints.first();

                if (seekFrameIndex > l_STestObj->startFrame) {
                    for (i = 0; i < m_SJumpPoints.size(); i++) {
                        if (m_SJumpPoints[i]->startFrame >= seekFrameIndex) {
                            if (i > 0) {
                                i--;
                            }

                            m_lCacheFramePos = m_SJumpPoints[i]->startFrame;
                            m_lStoredSeekPoint = m_SJumpPoints[i]->pos;
                            m_SStoredJumpPoint = m_SJumpPoints[i];
                            break;
                        }
                    }
                }
            }

            if (seekFrameIndex == frameIndexMin()) {
                // Because we are in the beginning just read cache full
                // but leave 50 of just in case
                // -1 one means we are seeking from current position and
                // filling the cache
                readFramesToCache((AUDIOSOURCEFFMPEG_CACHESIZE - 50),
                                  AUDIOSOURCEFFMPEG_FILL_FROM_CURRENTPOS);
            }
        }

        if (m_lCacheEndFrame <= seekFrameIndex) {
            // Cache tries to read until it gets to frameIndex
            // after that we still read 100 FFmpeg frames to memory
            // so we have good cache to go forward (100) and backward (900)
            // from the point
            readFramesToCache(100, seekFrameIndex);
        }

        m_currentMixxxFrameIndex = seekFrameIndex;

        m_bIsSeeked = m_currentMixxxFrameIndex != frameIndexMin();
    }
    DEBUG_ASSERT(m_currentMixxxFrameIndex == seekFrameIndex);

    const SINT numberOfFrames = writableSampleFrames.frameLength();

    DEBUG_ASSERT(m_currentMixxxFrameIndex == firstFrameIndex);
    DEBUG_ASSERT(m_SCache.size() > 0);
    getBytesFromCache(
            writableSampleFrames.writableData(),
            m_currentMixxxFrameIndex, numberOfFrames);
    m_currentMixxxFrameIndex += numberOfFrames;
    m_bIsSeeked = false;

    return ReadableSampleFrames(
            IndexRange::forward(firstFrameIndex, numberOfFrames),
            SampleBuffer::ReadableSlice(
                    writableSampleFrames.writableData(),
                    std::min(writableSampleFrames.writableLength(), frames2samples(numberOfFrames))));
}

} // namespace mixxx
