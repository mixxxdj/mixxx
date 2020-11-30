#include "sources/soundsourceffmpeg.h"

#include <mutex>

#include "util/logger.h"
#include "util/sample.h"

#if !defined(VERBOSE_DEBUG_LOG)
#define VERBOSE_DEBUG_LOG false
#endif

namespace mixxx {

namespace {

// FFmpeg constants

constexpr AVSampleFormat kavSampleFormat = AV_SAMPLE_FMT_FLT;

constexpr uint64_t kavChannelLayoutUndefined = 0;

constexpr int64_t kavStreamDefaultStartTime = 0;

// https://ffmpeg.org/doxygen/trunk/structAVPacket.html#details
// "For audio it may contain several compressed frames."
// A stream packet may produce multiple stream frames when decoded.
// Buffering more than a few codec frames with samples in advance
// should be unlikely.
// This is just a best guess that needs to be increased once
// warnings about reallocation of the internal sample buffer
// appear in the logs!
constexpr uint64_t kavMaxDecodedFramesPerPacket = 16;

// 0.5 sec @ 96 kHz / 1 sec @ 48 kHz / 1.09 sec @ 44.1 kHz
constexpr FrameCount kDefaultFrameBufferCapacity = 48000;

constexpr FrameCount kMinFrameBufferCapacity = kDefaultFrameBufferCapacity;

inline FrameCount frameBufferCapacityForStream(
        const AVStream& avStream) {
    DEBUG_ASSERT(kMinFrameBufferCapacity <= kDefaultFrameBufferCapacity);
    if (avStream.codecpar->frame_size > 0) {
        return math_max(
                static_cast<FrameCount>(
                        avStream.codecpar->frame_size *
                        kavMaxDecodedFramesPerPacket),
                kMinFrameBufferCapacity);
    }
    return kDefaultFrameBufferCapacity;
}

// "AAC Audio - Encoder Delay and Synchronization: The 2112 Sample Assumption"
// https://developer.apple.com/library/ios/technotes/tn2258/_index.html
// "It must also be assumed that without an explicit value, the playback
// system will trim 2112 samples from the AAC decoder output when starting
// playback from any point in the bitsream."
// See also: https://developer.apple.com/library/archive/documentation/QuickTime/QTFF/QTFFAppenG/QTFFAppenG.html
constexpr int64_t kavStreamDecoderFrameDelayAAC = 2112;

// Use 0-based sample frame indexing
constexpr SINT kMinFrameIndex = 0;

constexpr SINT kSamplesPerMP3Frame = 1152;

const Logger kLogger("SoundSourceFFmpeg");

std::once_flag initFFmpegLibFlag;

// FFmpeg API Changes:
// https://github.com/FFmpeg/FFmpeg/blob/master/doc/APIchanges

// This function must be called once during startup.
void initFFmpegLib() {
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 9, 100)
    av_register_all();
#endif
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 10, 100)
    avcodec_register_all();
#endif
}

inline int64_t getStreamChannelLayout(const AVStream& avStream) {
    auto channel_layout = avStream.codecpar->channel_layout;
    if (channel_layout == kavChannelLayoutUndefined) {
        // Workaround: FFmpeg sometimes fails to determine the channel
        // layout, e.g. for a mono WAV files with a single channel!
        channel_layout = av_get_default_channel_layout(avStream.codecpar->channels);
        kLogger.info()
                << "Unknown channel layout -> using default layout"
                << channel_layout
                << "for"
                << avStream.codecpar->channels
                << "channel(s)";
    }
    return channel_layout;
}

inline int64_t getStreamStartTime(const AVStream& avStream) {
    auto start_time = avStream.start_time;
    if (start_time == AV_NOPTS_VALUE) {
        // This case is not unlikely, e.g. happens when decoding WAV files.
        switch (avStream.codecpar->codec_id) {
        case AV_CODEC_ID_AAC:
        case AV_CODEC_ID_AAC_LATM: {
            // Account for the expected decoder delay instead of simply
            // using the default start time.
            // Not all M4A files encode the start_time correctly, e.g.
            // the test file cover-test-itunes-12.7.0-aac.m4a has a valid
            // start_time of 0. Unfortunately, this special case is cannot
            // detected and compensated.
            start_time = math_max(kavStreamDefaultStartTime, kavStreamDecoderFrameDelayAAC);
            break;
        }
        default:
            start_time = kavStreamDefaultStartTime;
        }
#if VERBOSE_DEBUG_LOG
        kLogger.debug()
                << "Unknown start time -> using default value"
                << start_time;
#endif
    }
    DEBUG_ASSERT(start_time != AV_NOPTS_VALUE);
    return start_time;
}

inline int64_t getStreamEndTime(const AVStream& avStream) {
    // The "duration" contains actually the end time of the
    // stream.
    VERIFY_OR_DEBUG_ASSERT(getStreamStartTime(avStream) <= avStream.duration) {
        // assume that the stream is empty
        return getStreamStartTime(avStream);
    }
    return avStream.duration;
}

inline SINT convertStreamTimeToFrameIndex(const AVStream& avStream, int64_t pts) {
    // getStreamStartTime(avStream) -> 1st audible frame at kMinFrameIndex
    return kMinFrameIndex +
            av_rescale_q(
                    pts - getStreamStartTime(avStream),
                    avStream.time_base,
                    (AVRational){1, avStream.codecpar->sample_rate});
}

inline int64_t convertFrameIndexToStreamTime(const AVStream& avStream, SINT frameIndex) {
    // Inverse mapping of convertStreamTimeToFrameIndex()
    return getStreamStartTime(avStream) +
            av_rescale_q(
                    frameIndex - kMinFrameIndex,
                    (AVRational){1, avStream.codecpar->sample_rate},
                    avStream.time_base);
}

IndexRange getStreamFrameIndexRange(const AVStream& avStream) {
    const auto frameIndexRange = IndexRange::between(
            convertStreamTimeToFrameIndex(avStream, getStreamStartTime(avStream)),
            convertStreamTimeToFrameIndex(avStream, getStreamEndTime(avStream)));
    DEBUG_ASSERT(frameIndexRange.orientation() != IndexRange::Orientation::Backward);
    return frameIndexRange;
}

SINT getStreamSeekPrerollFrameCount(const AVStream& avStream) {
    // Stream might not provide an appropriate value that is
    // sufficient for sample accurate decoding
    const SINT defaultSeekPrerollFrameCount =
            avStream.codecpar->seek_preroll;
    DEBUG_ASSERT(defaultSeekPrerollFrameCount >= 0);
    switch (avStream.codecpar->codec_id) {
    case AV_CODEC_ID_MP3:
    case AV_CODEC_ID_MP3ON4: {
        // In the worst case up to 29 MP3 frames need to be prerolled
        // for accurate seeking:
        // http://www.mars.org/mailman/public/mad-dev/2002-May/000634.html
        // But that would require to (re-)decode many frames after each seek
        // operation, which increases the chance that dropouts may occur.
        // As a compromise we will preroll only 9 instead of 29 frames.
        // Those 9 frames should at least drain the bit reservoir.
        //
        // NOTE(2019-09-08): Executing the decoding test with various VBR/CBR
        // MP3 files always produced exact results with only 9 preroll frames.
        // Thus increasing this number is not required and would increase
        // the risk for drop outs when jumping to a new position within
        // the file. Audible drop outs are considered more harmful than
        // slight deviations from the exact signal!
        DEBUG_ASSERT(avStream.codecpar->channels <= 2);
        const SINT mp3SeekPrerollFrameCount =
                9 * (kSamplesPerMP3Frame / avStream.codecpar->channels);
        return math_max(mp3SeekPrerollFrameCount, defaultSeekPrerollFrameCount);
    }
    case AV_CODEC_ID_AAC:
    case AV_CODEC_ID_AAC_LATM: {
        const SINT aacSeekPrerollFrameCount = kavStreamDecoderFrameDelayAAC;
        return math_max(aacSeekPrerollFrameCount, defaultSeekPrerollFrameCount);
    }
    default:
        return defaultSeekPrerollFrameCount;
    }
}

inline QString formatErrorString(int errnum) {
    // Allocate a static buffer on the stack and initialize it
    // with a `\0` terminator for extra safety if av_strerror()
    // unexpectedly fails and does nothing.
    char errbuf[AV_ERROR_MAX_STRING_SIZE]{0};
    // The result value if av_strerror() does not need to be handled:
    // "Even in case of failure av_strerror() will print a generic error
    // message indicating the errnum provided to errbuf."
    av_strerror(errnum, errbuf, sizeof(errbuf) / sizeof(errbuf[0]));
    return QString::fromLocal8Bit(errbuf);
}

#if VERBOSE_DEBUG_LOG
inline void avTrace(const char* preamble, const AVPacket& avPacket) {
    kLogger.debug()
            << preamble
            << "{ stream_index" << avPacket.stream_index
            << "| pos" << avPacket.pos
            << "| size" << avPacket.size
            << "| dst" << avPacket.dts
            << "| pts" << avPacket.pts
            << "| duration" << avPacket.duration
            << '}';
}

inline void avTrace(const char* preamble, const AVFrame& avFrame) {
    kLogger.debug()
            << preamble
            << "{ channels" << avFrame.channels
            << "| channel_layout" << avFrame.channel_layout
            << "| format" << avFrame.format
            << "| sample_rate" << avFrame.sample_rate
            << "| pkt_dts" << avFrame.pkt_dts
            << "| pkt_duration" << avFrame.pkt_duration
            << "| pts" << avFrame.pts
            << "| nb_samples" << avFrame.nb_samples
            << '}';
}
#endif // VERBOSE_DEBUG_LOG

AVFormatContext* openInputFile(
        const QString& fileName) {
    // Will be allocated implicitly when opening the input file
    AVFormatContext* pavInputFormatContext = nullptr;

    // Open input file and allocate/initialize AVFormatContext
    const int avformat_open_input_result =
            avformat_open_input(
                    &pavInputFormatContext, fileName.toLocal8Bit().constData(), nullptr, nullptr);
    if (avformat_open_input_result != 0) {
        DEBUG_ASSERT(avformat_open_input_result < 0);
        kLogger.warning().noquote()
                << "avformat_open_input() failed:"
                << formatErrorString(avformat_open_input_result);
        DEBUG_ASSERT(pavInputFormatContext == nullptr);
    }
    return pavInputFormatContext;
}

bool openDecodingContext(
        AVCodecContext* pavCodecContext) {
    DEBUG_ASSERT(pavCodecContext != nullptr);

    const int avcodec_open2_result = avcodec_open2(pavCodecContext, pavCodecContext->codec, nullptr);
    if (avcodec_open2_result != 0) {
        DEBUG_ASSERT(avcodec_open2_result < 0);
        kLogger.warning().noquote()
                << "avcodec_open2() failed:"
                << formatErrorString(avcodec_open2_result);
        return false;
    }
    return true;
}

} // anonymous namespace

void SoundSourceFFmpeg::InputAVFormatContextPtr::take(
        AVFormatContext** ppavInputFormatContext) {
    DEBUG_ASSERT(ppavInputFormatContext != nullptr);
    if (m_pavInputFormatContext != *ppavInputFormatContext) {
        close();
        m_pavInputFormatContext = *ppavInputFormatContext;
        *ppavInputFormatContext = nullptr;
    }
}

void SoundSourceFFmpeg::InputAVFormatContextPtr::close() {
    if (m_pavInputFormatContext != nullptr) {
        avformat_close_input(&m_pavInputFormatContext);
        DEBUG_ASSERT(m_pavInputFormatContext == nullptr);
    }
}

//static
SoundSourceFFmpeg::AVCodecContextPtr
SoundSourceFFmpeg::AVCodecContextPtr::alloc(
        const AVCodec* codec) {
    AVCodecContextPtr context(avcodec_alloc_context3(codec));
    if (!context) {
        kLogger.warning()
                << "avcodec_alloc_context3() failed for codec"
                << codec->name;
    }
    return context;
}

void SoundSourceFFmpeg::AVCodecContextPtr::take(AVCodecContext** ppavCodecContext) {
    DEBUG_ASSERT(ppavCodecContext != nullptr);
    if (m_pavCodecContext != *ppavCodecContext) {
        close();
        m_pavCodecContext = *ppavCodecContext;
        *ppavCodecContext = nullptr;
    }
}

void SoundSourceFFmpeg::AVCodecContextPtr::close() {
    if (m_pavCodecContext != nullptr) {
        avcodec_free_context(&m_pavCodecContext);
        m_pavCodecContext = nullptr;
    }
}

void SoundSourceFFmpeg::SwrContextPtr::take(
        SwrContext** ppSwrContext) {
    DEBUG_ASSERT(m_pSwrContext != nullptr);
    if (m_pSwrContext != *ppSwrContext) {
        close();
        m_pSwrContext = *ppSwrContext;
        *ppSwrContext = nullptr;
    }
}

void SoundSourceFFmpeg::SwrContextPtr::close() {
    if (m_pSwrContext != nullptr) {
        swr_free(&m_pSwrContext);
        DEBUG_ASSERT(m_pSwrContext == nullptr);
    }
}

const QString SoundSourceProviderFFmpeg::kDisplayName = QStringLiteral("FFmpeg");

SoundSourceProviderFFmpeg::SoundSourceProviderFFmpeg() {
    std::call_once(initFFmpegLibFlag, initFFmpegLib);
}

QStringList SoundSourceProviderFFmpeg::getSupportedFileExtensions() const {
    QStringList list;
    QStringList disabledInputFormats;

    // Collect all supported formats (whitelist)
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 9, 100)
    AVInputFormat* pavInputFormat = nullptr;
    while ((pavInputFormat = av_iformat_next(pavInputFormat))) {
#else
    const AVInputFormat* pavInputFormat = nullptr;
    void* iInputFormat = 0;
    while ((pavInputFormat = av_demuxer_iterate(&iInputFormat))) {
#endif
        if (pavInputFormat->flags | AVFMT_SEEK_TO_PTS) {
            ///////////////////////////////////////////////////////////
            // Whitelist of tested codecs (including variants)
            ///////////////////////////////////////////////////////////
            if (!strcmp(pavInputFormat->name, "aac")) {
                list.append("aac");
                continue;
            } else if (!strcmp(pavInputFormat->name, "aiff")) {
                list.append("aif");
                list.append("aiff");
                continue;
            } else if (!strcmp(pavInputFormat->name, "mp3")) {
                list.append("mp3");
                continue;
            } else if (!strcmp(pavInputFormat->name, "mp4")) {
                list.append("mp4");
                continue;
            } else if (!strcmp(pavInputFormat->name, "m4v")) {
                list.append("m4v");
                continue;
            } else if (!strcmp(pavInputFormat->name, "mov,mp4,m4a,3gp,3g2,mj2")) {
                list.append("mov");
                list.append("mp4");
                list.append("m4a");
                list.append("3gp");
                list.append("3g2");
                list.append("mj2");
                continue;
            } else if (!strcmp(pavInputFormat->name, "opus") || !strcmp(pavInputFormat->name, "libopus")) {
                list.append("opus");
                continue;
            } else if (!strcmp(pavInputFormat->name, "wav")) {
                list.append("wav");
                continue;
            } else if (!strcmp(pavInputFormat->name, "wv")) {
                list.append("wv");
                continue;
                ///////////////////////////////////////////////////////////
                // Codecs with failing tests
                ///////////////////////////////////////////////////////////
                /*
            } else if (!strcmp(pavInputFormat->name, "flac")) {
                // FFmpeg failure causes test failure:
                // [flac @ 0x2ef2060] read_timestamp() failed in the middle
                // SoundSourceFFmpeg - av_seek_frame() failed: Operation not permitted
                list.append("flac");
                continue;
            } else if (!strcmp(pavInputFormat->name, "ogg")) {
                // Test failures that might be caused by FFmpeg bug:
                // https://trac.ffmpeg.org/ticket/3825
                list.append("ogg");
                continue;
            } else if (!strcmp(pavInputFormat->name, "wma") ||
                    !strcmp(pavInputFormat->name, "xwma")) {
                list.append("wma");
                continue;
            */
                ///////////////////////////////////////////////////////////
                // Untested codecs
                ///////////////////////////////////////////////////////////
                /*
            } else if (!strcmp(pavInputFormat->name, "ac3")) {
                list.append("ac3");
                continue;
            } else if (!strcmp(pavInputFormat->name, "caf")) {
                list.append("caf");
                continue;
            } else if (!strcmp(pavInputFormat->name, "mpc")) {
                list.append("mpc");
                continue;
            } else if (!strcmp(pavInputFormat->name, "mpeg")) {
                list.append("mpeg");
                continue;
            } else if (!strcmp(pavInputFormat->name, "tak")) {
                list.append("tak");
                continue;
            } else if (!strcmp(pavInputFormat->name, "tta")) {
                list.append("tta");
                continue;
            */
            }
        }
        disabledInputFormats.append(pavInputFormat->name);
        continue;
    }

    if (!disabledInputFormats.isEmpty()) {
        kLogger.info().noquote()
                << "Disabling untested input formats:"
                << disabledInputFormats.join(QStringLiteral(", "));
    }

    return list;
}

SoundSourceProviderPriority SoundSourceProviderFFmpeg::getPriorityHint(
        const QString& supportedFileExtension) const {
    Q_UNUSED(supportedFileExtension)
    // TODO: Increase priority to Default or even Higher for all
    // supported and tested file extension?
    // Currently it is only used as a fallback after all other
    // SoundSources failed to open a file or are otherwise unavailable.
    return SoundSourceProviderPriority::Lowest;
}

SoundSourceFFmpeg::SoundSourceFFmpeg(const QUrl& url)
        : SoundSource(url),
          m_pavStream(nullptr),
          m_pavDecodedFrame(nullptr),
          m_pavResampledFrame(nullptr),
          m_seekPrerollFrameCount(0) {
}

SoundSourceFFmpeg::~SoundSourceFFmpeg() {
    close();
}

SoundSource::OpenResult SoundSourceFFmpeg::tryOpen(
        OpenMode /*mode*/,
        const OpenParams& params) {
    // Open input
    {
        AVFormatContext* pavInputFormatContext =
                openInputFile(getLocalFileName());
        if (pavInputFormatContext == nullptr) {
            kLogger.warning()
                    << "Failed to open input file"
                    << getLocalFileName();
            return OpenResult::Failed;
        }
        m_pavInputFormatContext.take(&pavInputFormatContext);
    }
#if VERBOSE_DEBUG_LOG
    kLogger.debug()
            << "AVFormatContext"
            << "{ nb_streams" << m_pavInputFormatContext->nb_streams
            << "| start_time" << m_pavInputFormatContext->start_time
            << "| duration" << m_pavInputFormatContext->duration
            << "| bit_rate" << m_pavInputFormatContext->bit_rate
            << "| packet_size" << m_pavInputFormatContext->packet_size
            << "| audio_codec_id" << m_pavInputFormatContext->audio_codec_id
            << "| output_ts_offset" << m_pavInputFormatContext->output_ts_offset
            << '}';
#endif

    // Retrieve stream information
    const int avformat_find_stream_info_result =
            avformat_find_stream_info(m_pavInputFormatContext, nullptr);
    if (avformat_find_stream_info_result != 0) {
        DEBUG_ASSERT(avformat_find_stream_info_result < 0);
        kLogger.warning().noquote()
                << "avformat_find_stream_info() failed:"
                << formatErrorString(avformat_find_stream_info_result);
        return OpenResult::Failed;
    }

    // Find the best stream
    AVCodec* pDecoder = nullptr;
    const int av_find_best_stream_result = av_find_best_stream(
            m_pavInputFormatContext,
            AVMEDIA_TYPE_AUDIO,
            /*wanted_stream_nb*/ -1,
            /*related_stream*/ -1,
            &pDecoder,
            /*flags*/ 0);
    if (av_find_best_stream_result < 0) {
        switch (av_find_best_stream_result) {
        case AVERROR_STREAM_NOT_FOUND:
            kLogger.warning()
                    << "av_find_best_stream() failed to find an audio stream";
            break;
        case AVERROR_DECODER_NOT_FOUND:
            kLogger.warning()
                    << "av_find_best_stream() failed to find a decoder for any audio stream";
            break;
        default:
            kLogger.warning().noquote()
                    << "av_find_best_stream() failed:"
                    << formatErrorString(av_find_best_stream_result);
        }
        return SoundSource::OpenResult::Aborted;
    }
    DEBUG_ASSERT(pDecoder);

    // Select audio stream for decoding
    AVStream* pavStream = m_pavInputFormatContext->streams[av_find_best_stream_result];
    DEBUG_ASSERT(pavStream != nullptr);
    DEBUG_ASSERT(pavStream->index == av_find_best_stream_result);

    // Allocate decoding context
    AVCodecContextPtr pavCodecContext = AVCodecContextPtr::alloc(pDecoder);
    if (!pavCodecContext) {
        return SoundSource::OpenResult::Aborted;
    }

    // Configure decoding context
    const int avcodec_parameters_to_context_result =
            avcodec_parameters_to_context(pavCodecContext, pavStream->codecpar);
    if (avcodec_parameters_to_context_result != 0) {
        DEBUG_ASSERT(avcodec_parameters_to_context_result < 0);
        kLogger.warning().noquote()
                << "avcodec_parameters_to_context() failed:"
                << formatErrorString(avcodec_parameters_to_context_result);
        return SoundSource::OpenResult::Aborted;
    }

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 18, 100)
    // Align the time base of the context with that of the selected stream
    av_codec_set_pkt_timebase(pavCodecContext, pavStream->time_base);
#endif

    // Request output format
    pavCodecContext->request_sample_fmt = kavSampleFormat;
    if (params.getSignalInfo().getChannelCount().isValid()) {
        // A dedicated number of channels for the output signal
        // has been requested. Forward this to FFmpeg to avoid
        // manual resampling or post-processing after decoding.
        pavCodecContext->request_channel_layout =
                av_get_default_channel_layout(params.getSignalInfo().getChannelCount());
    }

    // Open decoding context
    if (!openDecodingContext(pavCodecContext)) {
        // early exit on any error
        return SoundSource::OpenResult::Failed;
    }

    // Initialize members
    m_pavCodecContext = std::move(pavCodecContext);
    m_pavStream = pavStream;

    if (kLogger.debugEnabled()) {
        kLogger.debug()
                << "AVStream"
                << "{ index" << m_pavStream->index
                << "| id" << m_pavStream->id
                << "| time_base" << m_pavStream->time_base.num << '/' << m_pavStream->time_base.den
                << "| start_time" << m_pavStream->start_time
                << "| duration" << m_pavStream->duration
                << "| nb_frames" << m_pavStream->nb_frames
                << "| codec_type" << m_pavStream->codecpar->codec_type
                << "| codec_id" << m_pavStream->codecpar->codec_id
                << "| channels" << m_pavStream->codecpar->channels
                << "| channel_layout" << m_pavStream->codecpar->channel_layout
                << "| channel_layout (fixed)" << getStreamChannelLayout(*m_pavStream)
                << "| format" << m_pavStream->codecpar->format
                << "| sample_rate" << m_pavStream->codecpar->sample_rate
                << "| bit_rate" << m_pavStream->codecpar->bit_rate
                << "| frame_size" << m_pavStream->codecpar->frame_size
                << "| seek_preroll" << m_pavStream->codecpar->seek_preroll
                << "| initial_padding" << m_pavStream->codecpar->initial_padding
                << "| trailing_padding" << m_pavStream->codecpar->trailing_padding
                << '}';
    }

    audio::ChannelCount channelCount;
    audio::SampleRate sampleRate;
    if (!initResampling(&channelCount, &sampleRate)) {
        return OpenResult::Failed;
    }
    if (!initChannelCountOnce(channelCount)) {
        kLogger.warning()
                << "Failed to initialize number of channels"
                << channelCount;
        return OpenResult::Aborted;
    }
    if (!initSampleRateOnce(sampleRate)) {
        kLogger.warning()
                << "Failed to initialize sample rate"
                << sampleRate;
        return OpenResult::Aborted;
    }

    const auto streamBitrate =
            audio::Bitrate(m_pavStream->codecpar->bit_rate / 1000); // kbps
    if (streamBitrate.isValid() && !initBitrateOnce(streamBitrate)) {
        kLogger.warning()
                << "Failed to initialize bitrate"
                << streamBitrate;
        return OpenResult::Failed;
    }

    if (m_pavStream->duration == AV_NOPTS_VALUE) {
        // Streams with unknown or unlimited duration are
        // not (yet) supported.
        kLogger.warning()
                << "Unknown or unlimited stream duration";
        return OpenResult::Failed;
    }
    const auto streamFrameIndexRange =
            getStreamFrameIndexRange(*m_pavStream);
    VERIFY_OR_DEBUG_ASSERT(streamFrameIndexRange.start() <= streamFrameIndexRange.end()) {
        kLogger.warning()
                << "Stream with unsupported or invalid frame index range"
                << streamFrameIndexRange;
        return OpenResult::Failed;
    }

    // Decoding MP3/AAC files manually into WAV using the ffmpeg CLI and
    // comparing the audio data revealed that we need to map the nominal
    // range of the stream onto our internal range starting at kMinFrameIndex.
    // See also the discussion regarding cue point shift/offset:
    // https://mixxx.zulipchat.com/#narrow/stream/109171-development/topic/Cue.20shift.2Foffset
    const auto frameIndexRange = IndexRange::forward(
            kMinFrameIndex,
            streamFrameIndexRange.length());
    if (!initFrameIndexRangeOnce(frameIndexRange)) {
        kLogger.warning()
                << "Failed to initialize frame index range"
                << frameIndexRange;
        return OpenResult::Failed;
    }

    DEBUG_ASSERT(!m_pavDecodedFrame);
    m_pavDecodedFrame = av_frame_alloc();

    // FFmpeg does not provide sample-accurate decoding after random seeks
    // in the stream out of the box. Depending on the actual codec we need
    // to account for this and start decoding before the target position.
    m_seekPrerollFrameCount = getStreamSeekPrerollFrameCount(*m_pavStream);
#if VERBOSE_DEBUG_LOG
    kLogger.debug() << "Seek preroll frame count:" << m_seekPrerollFrameCount;
#endif

    m_frameBuffer = ReadAheadFrameBuffer(
            getSignalInfo(),
            frameBufferCapacityForStream(*m_pavStream));
#if VERBOSE_DEBUG_LOG
    kLogger.debug() << "Frame buffer capacity:" << m_frameBuffer.capacity();
#endif

    return OpenResult::Succeeded;
}

bool SoundSourceFFmpeg::initResampling(
        audio::ChannelCount* pResampledChannelCount,
        audio::SampleRate* pResampledSampleRate) {
    const auto avStreamChannelLayout =
            getStreamChannelLayout(*m_pavStream);
    const auto streamChannelCount =
            audio::ChannelCount(m_pavStream->codecpar->channels);
    // NOTE(uklotzde, 2017-09-26): Resampling to a different number of
    // channels like upsampling a mono to stereo signal breaks various
    // tests in the EngineBufferE2ETest suite!! SoundSource decoding tests
    // are unaffected, because there we always compare two signals produced
    // by the same decoder instead of a decoded with a reference signal. As
    // a workaround we decode the stream's channels as is and let Mixxx decide
    // how to handle this later.
    const auto resampledChannelCount =
            /*config.getSignalInfo().getChannelCount().isValid() ? config.getSignalInfo().getChannelCount() :*/ streamChannelCount;
    const auto avResampledChannelLayout =
            av_get_default_channel_layout(resampledChannelCount);
    const auto avStreamSampleFormat =
            m_pavCodecContext->sample_fmt;
    const auto avResampledSampleFormat =
            kavSampleFormat;
    // NOTE(uklotzde): We prefer not to change adjust sample rate here, because
    // all the frame calculations while decoding use the frame information
    // from the underlying stream! We only need resampling for up-/downsampling
    // the channels and to transform the decoded audio data into the sample
    // format that is used by Mixxx.
    const auto streamSampleRate =
            audio::SampleRate(m_pavStream->codecpar->sample_rate);
    const auto resampledSampleRate = streamSampleRate;
    if ((resampledChannelCount != streamChannelCount) ||
            (avResampledChannelLayout != avStreamChannelLayout) ||
            (avResampledSampleFormat != avStreamSampleFormat)) {
#if VERBOSE_DEBUG_LOG
        kLogger.debug()
                << "Decoded stream needs to be resampled"
                << ": channel count =" << resampledChannelCount
                << "| channel layout =" << avResampledChannelLayout
                << "| sample format =" << av_get_sample_fmt_name(avResampledSampleFormat);
#endif
        m_pSwrContext = SwrContextPtr(swr_alloc_set_opts(
                nullptr,
                avResampledChannelLayout,
                avResampledSampleFormat,
                resampledSampleRate,
                avStreamChannelLayout,
                avStreamSampleFormat,
                streamSampleRate,
                0,
                nullptr));
        if (!m_pSwrContext) {
            kLogger.warning()
                    << "Failed to allocate resampling context";
            return false;
        }
        const auto swr_init_result =
                swr_init(m_pSwrContext);
        if (swr_init_result < 0) {
            kLogger.warning().noquote()
                    << "swr_init() failed:"
                    << formatErrorString(swr_init_result);
            return false;
        }
        DEBUG_ASSERT(!m_pavResampledFrame);
        m_pavResampledFrame = av_frame_alloc();
    }
    // Finish initialization
    m_avStreamChannelLayout = avStreamChannelLayout;
    m_avResampledChannelLayout = avResampledChannelLayout;
    // Write output parameters
    DEBUG_ASSERT(pResampledChannelCount);
    *pResampledChannelCount = resampledChannelCount;
    DEBUG_ASSERT(pResampledSampleRate);
    *pResampledSampleRate = resampledSampleRate;
    return true;
}

void SoundSourceFFmpeg::close() {
    av_frame_free(&m_pavResampledFrame);
    DEBUG_ASSERT(!m_pavResampledFrame);
    av_frame_free(&m_pavDecodedFrame);
    DEBUG_ASSERT(!m_pavDecodedFrame);
    m_pSwrContext.close();
    m_pavCodecContext.close();
    m_pavInputFormatContext.close();
    m_pavStream = nullptr;
}

namespace {
SINT readNextPacket(
        AVFormatContext* pavFormatContext,
        AVStream* pavStream,
        AVPacket* pavPacket,
        SINT flushFrameIndex) {
    while (true) {
        const auto av_read_frame_result =
                av_read_frame(
                        pavFormatContext,
                        pavPacket);
        if (av_read_frame_result < 0) {
            if (av_read_frame_result == AVERROR_EOF) {
                // Enter drain mode: Flush the decoder with a final empty packet
#if VERBOSE_DEBUG_LOG
                kLogger.debug()
                        << "EOF: Entering drain mode";
#endif
                pavPacket->stream_index = pavStream->index;
                pavPacket->data = nullptr;
                pavPacket->size = 0;
                return flushFrameIndex;
            } else {
                kLogger.warning().noquote()
                        << "av_read_frame() failed:"
                        << formatErrorString(av_read_frame_result);
                return ReadAheadFrameBuffer::kInvalidFrameIndex;
            }
        }
#if VERBOSE_DEBUG_LOG
        avTrace("Packet read from stream", *pavPacket);
#endif
        DEBUG_ASSERT(pavPacket->data);
        DEBUG_ASSERT(pavPacket->size > 0);
        if (pavPacket->stream_index == pavStream->index) {
            // Found a packet for the stream
            break;
        } else {
            av_packet_unref(pavPacket);
        }
    }
    DEBUG_ASSERT(pavPacket->stream_index == pavStream->index);
    return (pavPacket->pts != AV_NOPTS_VALUE)
            ? convertStreamTimeToFrameIndex(*pavStream, pavPacket->pts)
            : ReadAheadFrameBuffer::kUnknownFrameIndex;
}
} // namespace

bool SoundSourceFFmpeg::adjustCurrentPosition(SINT startIndex) {
    DEBUG_ASSERT(frameIndexRange().containsIndex(startIndex));

    if (m_frameBuffer.tryContinueReadingFrom(startIndex)) {
        // Already buffered
        return true;
    }

    // Need to seek to a new position before continue reading. For
    // sample accurate decoding the actual seek position must be
    // placed BEFORE the position where reading continues.
    auto seekIndex =
            math_max(kMinFrameIndex, startIndex - m_seekPrerollFrameCount);
    // Seek to codec frame boundaries if the frame size is fixed and known
    if (m_pavStream->codecpar->frame_size > 0) {
        seekIndex -=
                (seekIndex - kMinFrameIndex) % m_pavCodecContext->frame_size;
    }
    DEBUG_ASSERT(seekIndex >= kMinFrameIndex);
    DEBUG_ASSERT(seekIndex <= startIndex);

    if (m_frameBuffer.tryContinueReadingFrom(seekIndex)) {
        // No need to perform a costly seek operation. Just skip some buffered
        // samples and continue decoding at the current position.
        return true;
    }

    // Flush internal decoder state before seeking
    avcodec_flush_buffers(m_pavCodecContext);

    // Seek to new position
    const int64_t seekTimestamp =
            convertFrameIndexToStreamTime(*m_pavStream, seekIndex);
    int av_seek_frame_result = av_seek_frame(
            m_pavInputFormatContext,
            m_pavStream->index,
            seekTimestamp,
            AVSEEK_FLAG_BACKWARD);
    if (av_seek_frame_result < 0) {
        // Unrecoverable seek error: Invalidate the current position and abort
        kLogger.warning().noquote()
                << "av_seek_frame() failed:"
                << formatErrorString(av_seek_frame_result);
        m_frameBuffer.invalidate();
        return false;
    }
    // The current position remains unknown until actually reading data
    // from the stream
    m_frameBuffer.reset();

    return true;
}

bool SoundSourceFFmpeg::consumeNextAVPacket(
        AVPacket* pavPacket, AVPacket** ppavNextPacket) {
    DEBUG_ASSERT(pavPacket);
    DEBUG_ASSERT(ppavNextPacket);
    if (!*ppavNextPacket) {
        // Read next packet from stream
        const SINT packetFrameIndex = readNextPacket(
                m_pavInputFormatContext,
                m_pavStream,
                pavPacket,
                m_frameBuffer.writeIndex());
        if (packetFrameIndex == ReadAheadFrameBuffer::kInvalidFrameIndex) {
            // Invalidate current position and abort reading
            m_frameBuffer.invalidate();
            return false;
        }
        *ppavNextPacket = pavPacket;
    }
    auto pavNextPacket = *ppavNextPacket;

    // Consume raw packet data
#if VERBOSE_DEBUG_LOG
    avTrace("Sending packet to decoder", *pavNextPacket);
#endif
    const auto avcodec_send_packet_result =
            avcodec_send_packet(m_pavCodecContext, pavNextPacket);
    if (avcodec_send_packet_result == 0) {
        // Packet has been consumed completely
#if VERBOSE_DEBUG_LOG
        kLogger.debug() << "Packet has been consumed by decoder";
#endif
        // Release ownership on packet
        av_packet_unref(pavNextPacket);
        *ppavNextPacket = nullptr;
    } else {
        // Packet has not been consumed or only partially
        if (avcodec_send_packet_result == AVERROR(EAGAIN)) {
            // Keep and resend this packet to the decoder during the next round
#if VERBOSE_DEBUG_LOG
            kLogger.debug() << "Packet needs to be sent again to decoder";
#endif
        } else {
            kLogger.warning().noquote()
                    << "avcodec_send_packet() failed:"
                    << formatErrorString(avcodec_send_packet_result);
            // Release ownership on packet
            av_packet_unref(pavNextPacket);
            *ppavNextPacket = nullptr;
            // Invalidate current position and abort reading
            m_frameBuffer.invalidate();
            return false;
        }
    }
    return true;
}

const CSAMPLE* SoundSourceFFmpeg::resampleDecodedAVFrame() {
    if (m_pSwrContext) {
        // Decoded frame must be resampled before reading
        m_pavResampledFrame->channel_layout = m_avResampledChannelLayout;
        m_pavResampledFrame->sample_rate = getSignalInfo().getSampleRate();
        m_pavResampledFrame->format = kavSampleFormat;
        if (m_pavDecodedFrame->channel_layout == kavChannelLayoutUndefined) {
            // Sometimes the channel layout is undefined.
            m_pavDecodedFrame->channel_layout = m_avStreamChannelLayout;
        }
#if VERBOSE_DEBUG_LOG
        avTrace("Resampling decoded frame", *m_pavDecodedFrame);
#endif
        const auto swr_convert_frame_result = swr_convert_frame(
                m_pSwrContext, m_pavResampledFrame, m_pavDecodedFrame);
        if (swr_convert_frame_result != 0) {
            kLogger.warning().noquote()
                    << "swr_convert_frame() failed:"
                    << formatErrorString(swr_convert_frame_result);
            // Discard decoded frame and abort after unrecoverable error
            av_frame_unref(m_pavDecodedFrame);
            return nullptr;
        }
#if VERBOSE_DEBUG_LOG
        avTrace("Received resampled frame", *m_pavResampledFrame);
#endif
        DEBUG_ASSERT(m_pavDecodedFrame->pts = m_pavResampledFrame->pts);
        DEBUG_ASSERT(m_pavDecodedFrame->nb_samples =
                             m_pavResampledFrame->nb_samples);
        return reinterpret_cast<const CSAMPLE*>(
                m_pavResampledFrame->extended_data[0]);
    } else {
        return reinterpret_cast<const CSAMPLE*>(
                m_pavDecodedFrame->extended_data[0]);
    }
}

ReadableSampleFrames SoundSourceFFmpeg::readSampleFramesClamped(
        const WritableSampleFrames& originalWritableSampleFrames) {
    DEBUG_ASSERT(m_frameBuffer.signalInfo() == getSignalInfo());
    const SINT readableStartIndex =
            originalWritableSampleFrames.frameIndexRange().start();
    const CSAMPLE* readableData = originalWritableSampleFrames.writableData();

#if VERBOSE_DEBUG_LOG
    kLogger.debug() << "readSampleFramesClamped:"
                    << "originalWritableSampleFrames.frameIndexRange()"
                    << originalWritableSampleFrames.frameIndexRange();
#endif
    WritableSampleFrames writableSampleFrames = originalWritableSampleFrames;

    // Consume all buffered sample data before decoding any new data
    if (m_frameBuffer.isReady()) {
        writableSampleFrames = m_frameBuffer.drainBuffer(writableSampleFrames);
#if VERBOSE_DEBUG_LOG
        kLogger.debug() << "After consuming buffered sample data:"
                        << "writableSampleFrames.frameIndexRange()"
                        << writableSampleFrames.frameIndexRange();
#endif
    }

    // Skip decoding if all data has been read
    auto writableFrameRange = writableSampleFrames.frameIndexRange();
    DEBUG_ASSERT(writableFrameRange.isSubrangeOf(frameIndexRange()));
    if (writableFrameRange.empty()) {
        auto readableRange = IndexRange::between(
                readableStartIndex, writableFrameRange.start());
        DEBUG_ASSERT(readableRange.orientation() != IndexRange::Orientation::Backward);
        const auto readableSampleCount =
                getSignalInfo().frames2samples(readableRange.length());
        return ReadableSampleFrames(readableRange,
                SampleBuffer::ReadableSlice(readableData, readableSampleCount));
    }

    // Adjust the current position
    if (!adjustCurrentPosition(writableFrameRange.start())) {
        // Abort reading on seek errors
        return ReadableSampleFrames();
    }
    DEBUG_ASSERT(m_frameBuffer.isValid());

    // Start decoding into the output buffer from the current position
    CSAMPLE* pOutputSampleBuffer = writableSampleFrames.writableData();

    AVPacket avPacket;
    av_init_packet(&avPacket);
    avPacket.data = nullptr;
    avPacket.size = 0;
    AVPacket* pavNextPacket = nullptr;
    while (m_frameBuffer.isValid() &&                         // no decoding error occurred
            (pavNextPacket || !writableFrameRange.empty()) && // not yet finished
            consumeNextAVPacket(&avPacket, &pavNextPacket)) { // next packet consumed
        int avcodec_receive_frame_result;
        // One or more AV packets are required for decoding the next AV frame
        do {
#if VERBOSE_DEBUG_LOG
            kLogger.debug()
                    << "Before decoding next AVPacket:"
                    << "m_frameBuffer.bufferedRange()" << m_frameBuffer.bufferedRange()
                    << "writableFrameRange" << writableFrameRange;
#endif

            // This invariant is valid during the whole loop!
            DEBUG_ASSERT(writableFrameRange.empty() || m_frameBuffer.isEmpty());

            // Decode next frame
            IndexRange decodedFrameRange;
            avcodec_receive_frame_result =
                    avcodec_receive_frame(m_pavCodecContext, m_pavDecodedFrame);
            if (avcodec_receive_frame_result == 0) {
#if VERBOSE_DEBUG_LOG
                avTrace("Received decoded frame", *m_pavDecodedFrame);
#endif
                DEBUG_ASSERT(m_pavDecodedFrame->pts != AV_NOPTS_VALUE);
                const auto decodedFrameCount = m_pavDecodedFrame->nb_samples;
                DEBUG_ASSERT(decodedFrameCount > 0);
                auto streamFrameIndex =
                        convertStreamTimeToFrameIndex(
                                *m_pavStream, m_pavDecodedFrame->pts);
                decodedFrameRange = IndexRange::forward(
                        streamFrameIndex,
                        decodedFrameCount);
            } else if (avcodec_receive_frame_result == AVERROR(EAGAIN)) {
#if VERBOSE_DEBUG_LOG
                kLogger.debug()
                        << "No more frames available until decoder is fed with "
                           "more packets from stream";
#endif
                DEBUG_ASSERT(!pavNextPacket);
                break;
            } else if (avcodec_receive_frame_result == AVERROR_EOF) {
                // Due to the lead-in with a start_time > 0 some encoded
                // files are shorter then actually reported. This may vary
                // depending on the encoder version, because sometimes the
                // lead-in is included in the stream's duration and sometimes
                // not. Short periods of silence at the end of a track are
                // acceptable in favor of a consistent handling of the lead-in,
                // because they may affect only the position of the outro end
                // point and not any other position markers!
                if (m_frameBuffer.isReady()) {
                    // Current position is known
                    DEBUG_ASSERT(m_frameBuffer.isEmpty());
                    DEBUG_ASSERT(m_frameBuffer.writeIndex() < frameIndexRange().end());
                    kLogger.info()
                            << "Stream ends at sample frame"
                            << m_frameBuffer.writeIndex()
                            << "instead of"
                            << frameIndexRange().end();
                }
                if (!writableFrameRange.empty()) {
                    const auto clearSampleCount =
                            getSignalInfo().frames2samples(
                                    writableFrameRange.length());
                    kLogger.debug()
                            << "Padding remaining output buffer with silence"
                            << writableFrameRange;
                    if (pOutputSampleBuffer) {
                        SampleUtil::clear(
                                pOutputSampleBuffer, clearSampleCount);
                        pOutputSampleBuffer += clearSampleCount;
                    }
                    writableFrameRange.shrinkFront(writableFrameRange.length());
                }
                DEBUG_ASSERT(writableFrameRange.empty());
                DEBUG_ASSERT(!pavNextPacket);
                break;
            } else {
                kLogger.warning().noquote()
                        << "avcodec_receive_frame() failed:"
                        << formatErrorString(avcodec_receive_frame_result);
                // Invalidate the current position and abort reading
                m_frameBuffer.invalidate();
                break;
            }
            DEBUG_ASSERT(!decodedFrameRange.empty());

#if VERBOSE_DEBUG_LOG
            kLogger.debug()
                    << "After receiving decoded sample data:"
                    << "m_frameBuffer.bufferedRange()" << m_frameBuffer.bufferedRange()
                    << "writableFrameRange" << writableFrameRange
                    << "decodedFrameRange" << decodedFrameRange;
#endif

            const CSAMPLE* pDecodedSampleData = resampleDecodedAVFrame();
            if (!pDecodedSampleData) {
                // Invalidate current position and abort reading after unrecoverable error
                m_frameBuffer.invalidate();
                // Housekeeping before aborting to avoid memory leaks
                av_frame_unref(m_pavDecodedFrame);
                break;
            }

            // The decoder may provide some lead-in and lead-out frames
            // before the start position and after the end of the stream.
            // Those frames need to be cut-off before consumption.
            if (decodedFrameRange.start() < frameIndexRange().start()) {
                const auto leadinRange = IndexRange::between(
                        decodedFrameRange.start(),
                        math_min(frameIndexRange().start(), decodedFrameRange.end()));
                DEBUG_ASSERT(leadinRange.orientation() != IndexRange::Orientation::Backward);
                if (leadinRange.orientation() == IndexRange::Orientation::Forward) {
#if VERBOSE_DEBUG_LOG
                    kLogger.debug()
                            << "Cutting off lead-in"
                            << leadinRange
                            << "before"
                            << frameIndexRange();
#endif
                    pDecodedSampleData += getSignalInfo().frames2samples(leadinRange.length());
                    decodedFrameRange.shrinkFront(leadinRange.length());
                }
            }
            if (decodedFrameRange.end() > frameIndexRange().end()) {
                const auto leadoutRange = IndexRange::between(
                        math_max(frameIndexRange().end(), decodedFrameRange.start()),
                        decodedFrameRange.end());
                DEBUG_ASSERT(leadoutRange.orientation() != IndexRange::Orientation::Backward);
                if (leadoutRange.orientation() == IndexRange::Orientation::Forward) {
#if VERBOSE_DEBUG_LOG
                    kLogger.debug()
                            << "Cutting off lead-out"
                            << leadoutRange
                            << "beyond"
                            << frameIndexRange();
#endif
                    decodedFrameRange.shrinkBack(leadoutRange.length());
                }
            }

#if VERBOSE_DEBUG_LOG
            kLogger.debug()
                    << "Before consuming decoded sample data:"
                    << "m_frameBuffer.bufferedRange()" << m_frameBuffer.bufferedRange()
                    << "writableFrameRange" << writableFrameRange
                    << "decodedFrameRange" << decodedFrameRange;
#endif

            const auto decodedSampleFrames = ReadableSampleFrames(
                    decodedFrameRange,
                    SampleBuffer::ReadableSlice(
                            pDecodedSampleData,
                            getSignalInfo().frames2samples(decodedFrameRange.length())));
            auto outputSampleFrames = WritableSampleFrames(
                    writableFrameRange,
                    SampleBuffer::WritableSlice(
                            pOutputSampleBuffer,
                            getSignalInfo().frames2samples(writableFrameRange.length())));
            outputSampleFrames = m_frameBuffer.consumeAndFillBuffer(
                    decodedSampleFrames,
                    outputSampleFrames,
                    writableSampleFrames.frameIndexRange().start());
            pOutputSampleBuffer = outputSampleFrames.writableData();
            writableFrameRange = outputSampleFrames.frameIndexRange();

#if VERBOSE_DEBUG_LOG
            kLogger.debug()
                    << "After consuming decoded sample data:"
                    << "m_frameBuffer.bufferedRange()" << m_frameBuffer.bufferedRange()
                    << "writableFrameRange" << writableFrameRange;
#endif

            // Housekeeping before next decoding iteration
            av_frame_unref(m_pavDecodedFrame);
            av_frame_unref(m_pavResampledFrame);
        } while (avcodec_receive_frame_result == 0 &&
                m_frameBuffer.isValid());
    }
    DEBUG_ASSERT(!pavNextPacket);

    auto readableRange =
            IndexRange::between(
                    readableStartIndex,
                    writableFrameRange.start());
    return ReadableSampleFrames(
            readableRange,
            SampleBuffer::ReadableSlice(
                    readableData,
                    getSignalInfo().frames2samples(readableRange.length())));
}

} // namespace mixxx
