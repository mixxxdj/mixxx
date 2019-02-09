#include "sources/soundsourceffmpeg4.h"

#include "util/sample.h"
#include "util/logger.h"

#include <limits>
#include <mutex>


namespace mixxx {

namespace {

constexpr AVSampleFormat kavSampleFormat = AV_SAMPLE_FMT_FLT;

constexpr uint64_t kavChannelLayoutUndefined = 0;

constexpr int64_t kavMinStartTime = 0;

// Use 0-based sample frame indexing
constexpr SINT kMinFrameIndex = 0;

constexpr SINT kFrameIndexInvalid = std::numeric_limits<SINT>::min();

constexpr SINT kFrameIndexUnknown = std::numeric_limits<SINT>::max();

constexpr SINT kSamplesPerMP3Frame = 1152;

// The minimum capacity for the internal sample buffer to
// store decoded data that has not been read/consumed yet.
// NOTE(2019-02-08, uklotzde): A capacity of 64 kB seem to
// be sufficient for most use cases when using FFmpeg 4.0.x.
constexpr SINT kMinSampleBufferCapacity = 65536;

const Logger kLogger("SoundSourceFFmpeg4");

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

inline
uint64_t getStreamChannelLayout(const AVStream& avStream) {
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

inline
int64_t getStreamStartTime(const AVStream& avStream) {
    auto start_time = avStream.start_time;
    if (start_time == AV_NOPTS_VALUE) {
        // This case is not unlikely, e.g. happens when decoding WAV files.
        /*
        kLogger.trace()
                << "Unknown start time -> using default value"
                << kavMinStartTime;
        */
        start_time = kavMinStartTime;
    }
    VERIFY_OR_DEBUG_ASSERT(start_time >= kavMinStartTime) {
        kLogger.warning()
                << "Adjusting start time:"
                << start_time
                << "->"
                << kavMinStartTime;
        return kavMinStartTime;
    }
    return start_time;
}

inline
int64_t getStreamEndTime(const AVStream& avStream) {
    auto start_time = avStream.start_time;
    if (start_time == AV_NOPTS_VALUE) {
        // This case is not unlikely, e.g. happens when decoding WAV files.
        /*
        kLogger.trace()
                << "Unknown start time -> using default value"
                << kavMinStartTime;
        */
        start_time = kavMinStartTime;
    }
    return start_time + avStream.duration;
}

inline
SINT convertStreamTimeToFrameIndex(const AVStream& avStream, int64_t pts) {
    return kMinFrameIndex +
            av_rescale_q(
                    pts - getStreamStartTime(avStream),
                    avStream.time_base,
                    (AVRational) {1, avStream.codecpar->sample_rate});
}

inline
int64_t convertFrameIndexToStreamTime(const AVStream& avStream, SINT frameIndex) {
    // See also: convertStreamTimeToFrameIndex(), e.g. pts = 0 at
    // frameIndex = kMinFrameIndex!
    return av_rescale_q(
                frameIndex - kMinFrameIndex,
                (AVRational) {1, avStream.codecpar->sample_rate},
                avStream.time_base);
}

IndexRange getStreamFrameIndexRange(const AVStream& avStream) {
    return IndexRange::forward(
            convertStreamTimeToFrameIndex(avStream, getStreamStartTime(avStream)),
            convertStreamTimeToFrameIndex(avStream, getStreamEndTime(avStream)));
}

SINT getStreamSeekPrerollFrameCount(const AVStream& avStream) {
    // Stream might not provide an appropriate value that is
    // sufficient for sample accurate decoding
    const SINT defaultSeekPrerollFrameCount =
            avStream.codecpar->seek_preroll;
    DEBUG_ASSERT(defaultSeekPrerollFrameCount >= 0);
    switch (avStream.codecpar->codec_id) {
    case AV_CODEC_ID_MP3:
    case AV_CODEC_ID_MP3ON4:
    {
        // In the worst case up to 29 MP3 frames need to be prerolled
        // for accurate seeking:
        // http://www.mars.org/mailman/public/mad-dev/2002-May/000634.html
        // But that would require to (re-)decode many frames after each seek
        // operation, which increases the chance that dropouts may occur.
        // As a compromise we will preroll only 9 instead of 29 frames.
        // Those 9 frames should at least drain the bit reservoir.
        DEBUG_ASSERT(avStream.codecpar->channels <= 2);
        const SINT mp3SeekPrerollFrameCount =
                9 * (kSamplesPerMP3Frame / avStream.codecpar->channels);
        return math_max(mp3SeekPrerollFrameCount, defaultSeekPrerollFrameCount);
    }
    case AV_CODEC_ID_AAC:
    case AV_CODEC_ID_AAC_LATM:
    {
        // "AAC Audio - Encoder Delay and Synchronization: The 2112 Sample Assumption"
        // https://developer.apple.com/library/ios/technotes/tn2258/_index.html
        // "It must also be assumed that without an explicit value, the playback
        // system will trim 2112 samples from the AAC decoder output when starting
        // playback from any point in the bistream."
        const SINT aacSeekPrerollFrameCount = 2112;
        return math_max(aacSeekPrerollFrameCount, defaultSeekPrerollFrameCount);
    }
    default:
        return defaultSeekPrerollFrameCount;
    }
}

QString formatErrorMessage(int errnum) {
    char errbuf[256];
    if (av_strerror(errnum, errbuf, sizeof(errbuf) / sizeof(errbuf[0]) == 0)) {
        return QString("%1 (%2)").arg(errbuf, errnum);
    } else {
        return QString("No description for error code (%1) found").arg(errnum);
    }
}

inline
void avTrace(const char* preamble, const AVStream& avStream) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << preamble
                << "{ index" << avStream.index
                << "| id" << avStream.id
                << "| codec_type" << avStream.codecpar->codec_type
                << "| codec_id" << avStream.codecpar->codec_id
                << "| channels" << avStream.codecpar->channels
                << "| channel_layout" << avStream.codecpar->channel_layout
                << "| channel_layout (fixed)" << getStreamChannelLayout(avStream)
                << "| format" << avStream.codecpar->format
                << "| sample_rate" << avStream.codecpar->sample_rate
                << "| bit_rate" << avStream.codecpar->bit_rate
                << "| frame_size" << avStream.codecpar->frame_size
                << "| initial_padding" << avStream.codecpar->initial_padding
                << "| trailing_padding" << avStream.codecpar->trailing_padding
                << "| seek_preroll" << avStream.codecpar->seek_preroll
                << "| start_time" << avStream.start_time
                << "| duration" << avStream.duration
                << "| nb_frames" << avStream.nb_frames
                << "| time_base" << avStream.time_base.num << '/' << avStream.time_base.den
                << '}';
    }
}

/*
inline
void avTrace(const char* preamble, const AVPacket& avPacket) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << preamble
            << "{ stream_index" << avPacket.stream_index
            << "| pos" << avPacket.pos
            << "| size" << avPacket.size
            << "| dst" << avPacket.dts
            << "| pts" << avPacket.pts
            << "| duration" << avPacket.duration
            << '}';
    }
}

inline
void avTrace(const char* preamble, const AVFrame& avFrame) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << preamble
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
}
*/

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
        kLogger.warning()
                << "avformat_open_input() failed:"
                << formatErrorMessage(avformat_open_input_result).toLocal8Bit().constData();
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
        kLogger.warning()
                << "avcodec_open2() failed:"
                << formatErrorMessage(avcodec_open2_result).toLocal8Bit().constData();
        return false;
    }
    return true;
}

} // anonymous namespace

void SoundSourceFFmpeg4::InputAVFormatContextPtr::take(
        AVFormatContext** ppavInputFormatContext) {
    DEBUG_ASSERT(ppavInputFormatContext != nullptr);
    if (m_pavInputFormatContext != *ppavInputFormatContext) {
        close();
        m_pavInputFormatContext = *ppavInputFormatContext;
        *ppavInputFormatContext = nullptr;
    }
}

void SoundSourceFFmpeg4::InputAVFormatContextPtr::close() {
    if (m_pavInputFormatContext != nullptr) {
        avformat_close_input(&m_pavInputFormatContext);
        DEBUG_ASSERT(m_pavInputFormatContext == nullptr);
    }
}

//static
SoundSourceFFmpeg4::AVCodecContextPtr
SoundSourceFFmpeg4::AVCodecContextPtr::alloc(
        const AVCodec* codec) {
    AVCodecContextPtr context(avcodec_alloc_context3(codec));
    if (!context) {
        kLogger.warning()
                << "avcodec_alloc_context3() failed for codec"
                << codec->name;
    }
    return context;
}

void SoundSourceFFmpeg4::AVCodecContextPtr::take(AVCodecContext** ppavCodecContext) {
    DEBUG_ASSERT(ppavCodecContext != nullptr);
    if (m_pavCodecContext != *ppavCodecContext) {
        close();
        m_pavCodecContext = *ppavCodecContext;
        *ppavCodecContext = nullptr;
    }
}

void SoundSourceFFmpeg4::AVCodecContextPtr::close() {
    if (m_pavCodecContext != nullptr) {
        avcodec_free_context(&m_pavCodecContext);
        m_pavCodecContext = nullptr;
    }
}

void SoundSourceFFmpeg4::SwrContextPtr::take(
        SwrContext** ppSwrContext) {
    DEBUG_ASSERT(m_pSwrContext != nullptr);
    if (m_pSwrContext != *ppSwrContext) {
        close();
        m_pSwrContext = *ppSwrContext;
        *ppSwrContext = nullptr;
    }
}

void SoundSourceFFmpeg4::SwrContextPtr::close() {
    if (m_pSwrContext != nullptr) {
        swr_free(&m_pSwrContext);
        DEBUG_ASSERT(m_pSwrContext == nullptr);
    }
}

SoundSourceProviderFFmpeg4::SoundSourceProviderFFmpeg4() {
    std::call_once(initFFmpegLibFlag, initFFmpegLib);
}

QStringList SoundSourceProviderFFmpeg4::getSupportedFileExtensions() const {
    QStringList list;

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
            } else if (!strcmp(pavInputFormat->name, "opus") ||
                    !strcmp(pavInputFormat->name, "libopus")) {
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
                // SoundSourceFFmpeg4 - av_seek_frame() failed: Operation not permitted
                list.append("flac");
                continue;
            } else if (!strcmp(pavInputFormat->name, "ogg")) {
                // Test failures that might be caused by FFmpeg bug:
                // https://trac.ffmpeg.org/ticket/3825
                list.append("ogg");
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
            } else if (!strcmp(pavInputFormat->name, "wma") ||
                       !strcmp(pavInputFormat->name, "xwma")) {
                list.append("wma");
                continue;
            */
            }
        }
        kLogger.info()
                << "Disabling untested input format:"
                << pavInputFormat->name;
        continue;
    }

    return list;
}

SoundSourceFFmpeg4::SoundSourceFFmpeg4(const QUrl& url)
    : SoundSource(url),
      m_pavStream(nullptr),
      m_pavDecodedFrame(nullptr),
      m_pavResampledFrame(nullptr),
      m_sampleBuffer(kMinSampleBufferCapacity),
      m_seekPrerollFrameCount(0),
      m_curFrameIndex(kFrameIndexInvalid) {
}

SoundSourceFFmpeg4::~SoundSourceFFmpeg4() {
    close();
}

SoundSource::OpenResult SoundSourceFFmpeg4::tryOpen(
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

    // Retrieve stream information
    const int avformat_find_stream_info_result =
            avformat_find_stream_info(m_pavInputFormatContext, nullptr);
    if (avformat_find_stream_info_result != 0) {
        DEBUG_ASSERT(avformat_find_stream_info_result < 0);
        kLogger.warning()
                << "avformat_find_stream_info() failed:"
                << formatErrorMessage(avformat_find_stream_info_result).toLocal8Bit().constData();
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
            kLogger.warning()
                    << "av_find_best_stream() failed:"
                    << formatErrorMessage(av_find_best_stream_result).toLocal8Bit().constData();
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
        kLogger.warning()
                << "avcodec_parameters_to_context() failed:"
                << formatErrorMessage(avcodec_parameters_to_context_result).toLocal8Bit().constData();
        return SoundSource::OpenResult::Aborted;
    }

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 18, 100)
    // Align the time base of the context with that of the selected stream
    av_codec_set_pkt_timebase(pavCodecContext, pavStream->time_base);
#endif

    // Request output format
    pavCodecContext->request_sample_fmt = kavSampleFormat;
    if (params.channelCount().valid()) {
        pavCodecContext->request_channel_layout = av_get_default_channel_layout(params.channelCount());
    }

    // Open decoding context
    if (!openDecodingContext(pavCodecContext)) {
        // early exit on any error
        return SoundSource::OpenResult::Failed;
    }

    // Initialize members
    m_pavCodecContext = std::move(pavCodecContext);
    m_pavStream = pavStream;

    avTrace("Opened stream for decoding", *m_pavStream);

    const auto streamChannelCount =
            ChannelCount(m_pavStream->codecpar->channels);
    m_avStreamChannelLayout =
            getStreamChannelLayout(*m_pavStream);
    const auto avStreamSampleFormat =
            m_pavCodecContext->sample_fmt;
    const auto sampleRate =
            SampleRate(m_pavStream->codecpar->sample_rate);
    const auto streamBitrate =
            Bitrate(m_pavStream->codecpar->bit_rate / 1000); // kbps
    const auto frameIndexRange =
            getStreamFrameIndexRange(*m_pavStream);

    DEBUG_ASSERT(!m_pavDecodedFrame);
    m_pavDecodedFrame = av_frame_alloc();

    // Init resampling
    const auto resampledChannelCount =
            // NOTE(uklotzde, 2017-09-26): Resampling to a different number of
            // channels like upsampling a mono to stereo signal breaks various
            // tests in the EngineBufferE2ETest suite!! SoundSource decoding tests
            // are unaffected, because there we always compare two signals produced
            // by the same decoder instead of a decoded with a reference signal. As
            // a workaround we decode the stream's channels as is and let Mixxx decide
            // how to handle this later.
            /*config.channelCount().valid() ? config.channelCount() :*/ streamChannelCount;
    m_avResampledChannelLayout =
            av_get_default_channel_layout(resampledChannelCount);
    const auto avResampledSampleFormat =
            kavSampleFormat;
    // NOTE(uklotzde): We prefer not to change adjust sample rate here, because
    // all the frame calculations while decoding use the frame information
    // from the underlying stream! We only need resampling for up-/downsampling
    // the channels and to transform the decoded audio data into the sample
    // format that is used by Mixxx.
    if ((resampledChannelCount != streamChannelCount) ||
            (m_avResampledChannelLayout != m_avStreamChannelLayout) ||
            (avResampledSampleFormat != avStreamSampleFormat)) {
        /*
        kLogger.trace()
                << "Decoded stream needs to be resampled"
                << ": channel count =" << resampledChannelCount
                << "| channel layout =" << m_avResampledChannelLayout
                << "| sample format =" << av_get_sample_fmt_name(avResampledSampleFormat);
        */
        m_pSwrContext = SwrContextPtr(swr_alloc_set_opts(
                nullptr,
                m_avResampledChannelLayout,
                avResampledSampleFormat,
                sampleRate,
                m_avStreamChannelLayout,
                avStreamSampleFormat,
                sampleRate,
                0,
                nullptr));
        if (!m_pSwrContext) {
            kLogger.warning()
                    << "Failed to allocate resampling context";
            return OpenResult::Failed;
        }
        const auto swr_init_result =
                swr_init(m_pSwrContext);
        if (swr_init_result < 0) {
            kLogger.warning()
                    << "swr_init() failed:"
                    << formatErrorMessage(swr_init_result).toLocal8Bit().constData();
            return OpenResult::Failed;
        }
        DEBUG_ASSERT(!m_pavResampledFrame);
        m_pavResampledFrame = av_frame_alloc();
    }

    if (!setChannelCount(resampledChannelCount)) {
        kLogger.warning()
                << "Failed to initialize number of channels"
                << resampledChannelCount;
        return OpenResult::Aborted;
    }

    if (!setSampleRate(sampleRate)) {
        kLogger.warning()
                << "Failed to initialize sampling rate"
                << sampleRate;
        return OpenResult::Aborted;
    }

    if (streamBitrate.valid() && !initBitrateOnce(streamBitrate)) {
        kLogger.warning()
                << "Failed to initialize bitrate"
                << streamBitrate;
        return OpenResult::Failed;
    }

    if (!initFrameIndexRangeOnce(frameIndexRange)) {
        kLogger.warning()
                << "Failed to initialize frame index range"
                << frameIndexRange;
        return OpenResult::Failed;
    }

    // A stream packet may produce multiple stream frames when decoded. Buffering
    // more than a few codec frames with samples in advance should be unlikely.
    const SINT codecSampleBufferCapacity = 4 * m_pavStream->codecpar->frame_size;
    if (m_sampleBuffer.capacity() < codecSampleBufferCapacity) {
        m_sampleBuffer.adjustCapacity(codecSampleBufferCapacity);
    }

    m_seekPrerollFrameCount = getStreamSeekPrerollFrameCount(*m_pavStream);
    //kLogger.trace() << "Seek preroll frame count:" << m_seekPrerollFrameCount;

    m_curFrameIndex = kMinFrameIndex;

    return OpenResult::Succeeded;
}

void SoundSourceFFmpeg4::close() {
    av_frame_free(&m_pavResampledFrame);
    DEBUG_ASSERT(!m_pavResampledFrame);
    av_frame_free(&m_pavDecodedFrame);
    DEBUG_ASSERT(!m_pavDecodedFrame);
    m_pSwrContext.close();
    m_pavCodecContext.close();
    m_pavInputFormatContext.close();
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
                    kLogger.debug()
                            << "EOF: Entering drain mode";
                    pavPacket->stream_index = pavStream->index;
                    pavPacket->data = nullptr;
                    pavPacket->size = 0;
                    return flushFrameIndex;
                } else {
                    kLogger.warning()
                            << "av_read_frame() failed:"
                            << formatErrorMessage(av_read_frame_result).toLocal8Bit().constData();
                    return kFrameIndexInvalid;
                }
            }
            //avTrace("Packet read from stream", *pavPacket);
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
                : kFrameIndexUnknown;
    }
}

ReadableSampleFrames SoundSourceFFmpeg4::readSampleFramesClamped(
        WritableSampleFrames writableSampleFrames) {
    CSAMPLE* pOutputSampleBuffer = writableSampleFrames.writableData();
    auto writableRange =
            writableSampleFrames.frameIndexRange();

    // Consume all buffered sample data before decoding any new data
    if (!m_sampleBuffer.empty()) {
        DEBUG_ASSERT(m_curFrameIndex != kFrameIndexInvalid);
        DEBUG_ASSERT(frameIndexRange().containsIndex(m_curFrameIndex));
        const auto bufferedRange =
                IndexRange::forward(
                        m_curFrameIndex,
                        samples2frames(m_sampleBuffer.readableLength()));
        const auto consumableRange =
                intersect(bufferedRange, writableRange);
        DEBUG_ASSERT(consumableRange.length() <= writableRange.length());
        if (!consumableRange.empty() &&
                (consumableRange.start() == writableRange.start())) {
            // Drop and skip preceding buffered samples
            DEBUG_ASSERT(m_curFrameIndex <= writableRange.start());
            const auto skippableRange =
                    IndexRange::between(
                            m_curFrameIndex,
                            writableRange.start());
            m_sampleBuffer.shrinkForReading(
                    frames2samples(skippableRange.length()));
            m_curFrameIndex += skippableRange.length();
            // Consume buffered samples
            DEBUG_ASSERT(m_curFrameIndex == writableRange.start());
            const SampleBuffer::ReadableSlice consumableSlice =
                    m_sampleBuffer.shrinkForReading(
                            frames2samples(consumableRange.length()));
            DEBUG_ASSERT(consumableSlice.length() ==
                    frames2samples(consumableRange.length()));
            //kLogger.trace() << "Consuming buffered samples" << consumableRange;
            if (pOutputSampleBuffer) {
                SampleUtil::copy(
                        pOutputSampleBuffer,
                        consumableSlice.data(),
                        consumableSlice.length());
                pOutputSampleBuffer += consumableSlice.length();
            }
            writableRange.shrinkFront(consumableRange.length());
            m_curFrameIndex += consumableRange.length();
            // Keep remaining buffered samples
            DEBUG_ASSERT(writableRange.empty() || m_sampleBuffer.empty());
        } else {
            // Discard all buffered sample data
            m_sampleBuffer.clear();
            m_curFrameIndex += bufferedRange.length();
        }
    }

    if (m_curFrameIndex != writableRange.start()) {
        // Need to seek before reading
        DEBUG_ASSERT(m_sampleBuffer.empty());
        auto seekFrameIndex =
                math_max(kMinFrameIndex, writableRange.start() - m_seekPrerollFrameCount);
        // Seek to codec frame boundaries if the frame size is fixed and known
        if (m_pavStream->codecpar->frame_size > 0) {
            seekFrameIndex -= (seekFrameIndex - kMinFrameIndex) % m_pavCodecContext->frame_size;
        }
        DEBUG_ASSERT(seekFrameIndex >= kMinFrameIndex);
        DEBUG_ASSERT(seekFrameIndex <= writableRange.start());
        if ((m_curFrameIndex == kFrameIndexInvalid) ||
                (m_curFrameIndex > writableRange.start()) ||
                (m_curFrameIndex < seekFrameIndex)) {
            // Flush internal decoder state
            avcodec_flush_buffers(m_pavCodecContext);
            // Invalidate current position
            m_curFrameIndex = kFrameIndexInvalid;
            // Seek to new position
            const int64_t seekTimestamp =
                    convertFrameIndexToStreamTime(*m_pavStream, seekFrameIndex);
            int av_seek_frame_result = av_seek_frame(
                    m_pavInputFormatContext,
                    m_pavStream->index,
                    seekTimestamp,
                    AVSEEK_FLAG_BACKWARD);
            if (av_seek_frame_result < 0) {
                kLogger.warning()
                        << "av_seek_frame() failed:"
                        << formatErrorMessage(av_seek_frame_result).toLocal8Bit().constData();
                m_curFrameIndex = kFrameIndexInvalid;
                return ReadableSampleFrames();
            }
            // Current position is unknown until reading from the stream
            m_curFrameIndex = kFrameIndexUnknown;
        }
    }
    DEBUG_ASSERT(m_curFrameIndex != kFrameIndexInvalid);

    AVPacket avPacket;
    av_init_packet(&avPacket);
    avPacket.data = nullptr;
    avPacket.size = 0;
    AVPacket* pavNextPacket = nullptr;
    auto readFrameIndex = m_curFrameIndex;
    while ((pavNextPacket || !writableRange.empty()) &&
            (m_curFrameIndex != kFrameIndexInvalid)) {
        // Read next packet from stream
        if (!pavNextPacket) {
            const SINT packetFrameIndex =
                    readNextPacket(
                            m_pavInputFormatContext,
                            m_pavStream,
                            &avPacket,
                            m_curFrameIndex);
            if (packetFrameIndex == kFrameIndexInvalid) {
                // Invalidate current position and abort reading
                m_curFrameIndex = kFrameIndexInvalid;
                break;
            }
            pavNextPacket = &avPacket;
        }
        DEBUG_ASSERT(pavNextPacket);

        // Consume raw packet data
        //avTrace("Sending packet to decoder", *pavNextPacket);
        const auto avcodec_send_packet_result =
                avcodec_send_packet(m_pavCodecContext, pavNextPacket);
        if (avcodec_send_packet_result == 0) {
            // Packet has been consumed completely
            //kLogger.trace() << "Packet has been consumed by decoder";
            // Release ownership on packet
            av_packet_unref(pavNextPacket);
            pavNextPacket = nullptr;
        } else {
            // Packet has not been consumed or only partially
            if (avcodec_send_packet_result == AVERROR(EAGAIN)) {
                // Keep and resend this packet to the decoder during the next round
                //kLogger.trace() << "Packet needs to be sent again to decoder";
            } else {
                kLogger.warning()
                        << "avcodec_send_packet() failed:"
                        << formatErrorMessage(avcodec_send_packet_result).toLocal8Bit().constData();
                // Release ownership on packet
                av_packet_unref(pavNextPacket);
                pavNextPacket = nullptr;
                // Invalidate current position and abort reading
                m_curFrameIndex = kFrameIndexInvalid;
                break;
            }
        }

        int avcodec_receive_frame_result;
        do {
            /*
            kLogger.trace()
                    << "m_curFrameIndex" << m_curFrameIndex
                    << "readFrameIndex" << readFrameIndex
                    << "writableRange" << writableRange
                    << "m_sampleBuffer.readableLength()" << m_sampleBuffer.readableLength();
            */

            DEBUG_ASSERT(writableRange.empty() || m_sampleBuffer.empty());

            SINT missingFrameCount = 0;
            const CSAMPLE* pDecodedSampleData = nullptr;

            // Decode next frame
            IndexRange decodedFrameRange;
            avcodec_receive_frame_result = avcodec_receive_frame(
                   m_pavCodecContext,
                   m_pavDecodedFrame);
            if (avcodec_receive_frame_result == 0) {
                //avTrace("Received decoded frame", *m_pavDecodedFrame);
                DEBUG_ASSERT(m_pavDecodedFrame->pts != AV_NOPTS_VALUE);
                const auto decodedFrameCount = m_pavDecodedFrame->nb_samples;
                DEBUG_ASSERT(decodedFrameCount > 0);
                decodedFrameRange = IndexRange::forward(
                        convertStreamTimeToFrameIndex(*m_pavStream, m_pavDecodedFrame->pts),
                        decodedFrameCount);
                if (readFrameIndex == kFrameIndexUnknown) {
                    readFrameIndex = decodedFrameRange.start();
                }
            } else if (avcodec_receive_frame_result == AVERROR(EAGAIN)) {
               //kLogger.trace() << "No more frames available until decoder is fed with more packets from stream";
               DEBUG_ASSERT(!pavNextPacket);
               break;
            } else if (avcodec_receive_frame_result == AVERROR_EOF) {
                DEBUG_ASSERT(!pavNextPacket);
                if (!writableRange.empty()) {
                    DEBUG_ASSERT(readFrameIndex < writableRange.end());
                    DEBUG_ASSERT(m_sampleBuffer.empty());
                    kLogger.debug()
                            << "Stream ends at sample frame"
                            << readFrameIndex
                            << "instead of"
                            << frameIndexRange().end()
                            << "-> padding with silence";
                    const auto clearSampleCount =
                            frames2samples(writableRange.length());
                    if (pOutputSampleBuffer) {
                        SampleUtil::clear(
                                pOutputSampleBuffer,
                                clearSampleCount);
                        pOutputSampleBuffer += clearSampleCount;
                    }
                    writableRange.shrinkFront(writableRange.length());
                }
                // Invalidate current position and abort reading
                m_curFrameIndex = kFrameIndexInvalid;
                break;
            } else {
                kLogger.warning()
                        << "avcodec_receive_frame() failed:"
                        << formatErrorMessage(avcodec_receive_frame_result).toLocal8Bit().constData();
                // Invalidate current position and abort reading
                m_curFrameIndex = kFrameIndexInvalid;
                break;
            }

            /*
            kLogger.trace()
                    << "After receiving decoded frame:"
                    << "m_curFrameIndex" << m_curFrameIndex
                    << "readFrameIndex" << readFrameIndex
                    << "decodedFrameRange" << decodedFrameRange
                    << "writableRange" << writableRange
                    << "missingFrameCount" << missingFrameCount;
            */
            DEBUG_ASSERT(readFrameIndex != kFrameIndexInvalid);
            DEBUG_ASSERT(readFrameIndex != kFrameIndexUnknown);

            if (decodedFrameRange.start() < readFrameIndex) {
                // The next frame starts BEFORE the current position
                const auto overlapRange =
                        IndexRange::between(
                                decodedFrameRange.start(),
                                readFrameIndex);
                // NOTE(2019-02-08, uklotzde): Overlapping frames at the
                // beginning of an audio stream before the first readable
                // sample frame at kMinFrameIndex are expected. For example
                // this happens when decoding 320kbps MP3 files where
                // decoding starts at position -1105 and the first 1105
                // decoded samples need to be skipped.
                if (readFrameIndex > kMinFrameIndex) {
                    kLogger.warning()
                            << "Overlapping sample frames in the stream:"
                            << overlapRange;
                }
                const auto consumedRange =
                        IndexRange::between(
                                writableSampleFrames.frameIndexRange().start(),
                                // We might still be decoding samples in preroll mode, i.e.
                                // readFrameIndex < writableSampleFrames.frameIndexRange().start()
                                math_max(readFrameIndex, writableSampleFrames.frameIndexRange().start()));
                auto rewindRange =
                        intersect(overlapRange, consumedRange);
                if (!rewindRange.empty()) {
                    DEBUG_ASSERT(rewindRange.end() == readFrameIndex);
                    kLogger.warning()
                            << "Rewinding current position:"
                            << readFrameIndex
                            << "->"
                            << rewindRange.start();
                    // Rewind internally buffered samples first...
                    const auto rewindSampleLength =
                            m_sampleBuffer.shrinkAfterWriting(
                                    frames2samples(rewindRange.length()));
                    rewindRange.shrinkBack(
                            samples2frames(rewindSampleLength));
                    // ...then rewind remaining samples from the output buffer
                    if (pOutputSampleBuffer) {
                        pOutputSampleBuffer -= frames2samples(rewindRange.length());
                    }
                    writableRange = IndexRange::between(rewindRange.start(), writableRange.end());
                }
                // Adjust read position
                readFrameIndex = decodedFrameRange.start();
            } else if (decodedFrameRange.start() > readFrameIndex) {
                // The next frame starts AFTER the current position, i.e.
                // some frames before decodedFrameRange.start() are missing.
                missingFrameCount = decodedFrameRange.start() - readFrameIndex;
            }

            /*
            kLogger.trace()
                    << "Before resampling:"
                    << "m_curFrameIndex" << m_curFrameIndex
                    << "readFrameIndex" << readFrameIndex
                    << "decodedFrameRange" << decodedFrameRange
                    << "writableRange" << writableRange
                    << "missingFrameCount" << missingFrameCount;
            */
            DEBUG_ASSERT(readFrameIndex <= decodedFrameRange.start());
            // NOTE: Decoding might start at a negative position for the first
            // frame of the file! In this case readFrameIndex < decodedFrameRange().start(),
            // i.e. the decoded frame starts outside of the track's valid range!
            // Consequently isValidFrameIndex(readFrameIndex) might return false.
            // This is expected behavior and will be compensated during 'preskip'
            // (see below).

            if (m_pSwrContext) {
                // Decoded frame must be resampled before reading
                m_pavResampledFrame->channel_layout = m_avResampledChannelLayout;
                m_pavResampledFrame->sample_rate = sampleRate();
                m_pavResampledFrame->format = kavSampleFormat;
                if (m_pavDecodedFrame->channel_layout == kavChannelLayoutUndefined) {
                    // Sometimes the channel layout is undefined.
                    m_pavDecodedFrame->channel_layout = m_avStreamChannelLayout;
                }
                //avTrace("Resampling decoded frame", *m_pavDecodedFrame);
                const auto swr_convert_frame_result =
                        swr_convert_frame(
                                m_pSwrContext,
                                m_pavResampledFrame,
                                m_pavDecodedFrame);
                if (swr_convert_frame_result != 0) {
                    kLogger.warning()
                            << "swr_convert_frame() failed:"
                            << formatErrorMessage(swr_convert_frame_result).toLocal8Bit().constData();
                    // Abort reading
                    av_frame_unref(m_pavDecodedFrame);
                    m_curFrameIndex = kFrameIndexInvalid;
                    break;
                }
                //avTrace("Received resampled frame", *m_pavResampledFrame);
                DEBUG_ASSERT(m_pavDecodedFrame->pts = m_pavResampledFrame->pts);
                DEBUG_ASSERT(m_pavDecodedFrame->nb_samples = m_pavResampledFrame->nb_samples);
                pDecodedSampleData = reinterpret_cast<const CSAMPLE*>(
                        m_pavResampledFrame->extended_data[0]);
            } else {
                pDecodedSampleData = reinterpret_cast<const CSAMPLE*>(
                        m_pavDecodedFrame->extended_data[0]);
            }

            // readFrameIndex
            //       |
            //       v
            //       | missingFrameCount |<- decodedFrameRange ->|

            VERIFY_OR_DEBUG_ASSERT(readFrameIndex <= writableRange.start()) {
                kLogger.critical()
                    << "Invalid decoding position"
                    << ": expected frame index =" << writableRange.start()
                    << ", actual frame index =" << readFrameIndex;
                m_curFrameIndex = kInvalidFrameIndex;
                break;
            }
            // Skip all missing/decoded ranges that do not overlap
            // with writableRange, i.e. that precede writableRange.
            DEBUG_ASSERT(pDecodedSampleData);
            const auto preskipMissingFrameCount =
                    math_min(missingFrameCount, writableRange.start() - readFrameIndex);
            missingFrameCount -= preskipMissingFrameCount;
            readFrameIndex += preskipMissingFrameCount;
            const auto preskipDecodedFrameCount =
                    math_min(decodedFrameRange.length(), writableRange.start() - readFrameIndex);
            pDecodedSampleData += frames2samples(preskipDecodedFrameCount);
            decodedFrameRange.shrinkFront(preskipDecodedFrameCount);
            readFrameIndex += preskipDecodedFrameCount;
            m_curFrameIndex = readFrameIndex;

            /*
            kLogger.trace()
                    << "Before writing:"
                    << "m_curFrameIndex" << m_curFrameIndex
                    << "readFrameIndex" << readFrameIndex
                    << "decodedFrameRange" << decodedFrameRange
                    << "writableRange" << writableRange
                    << "missingFrameCount" << missingFrameCount;
            */

            // Consume all sample data from missing/decoded ranges
            // that overlap with writableRange.
            if (readFrameIndex == writableRange.start()) {
                const auto writeMissingFrameCount =
                        math_min(missingFrameCount, writableRange.length());
                if (writeMissingFrameCount > 0) {
                    const auto clearSampleCount =
                            frames2samples(writeMissingFrameCount);
                    if (pOutputSampleBuffer) {
                        SampleUtil::clear(
                                pOutputSampleBuffer,
                                clearSampleCount);
                        pOutputSampleBuffer += clearSampleCount;
                    }
                    missingFrameCount -= writeMissingFrameCount;
                    writableRange.shrinkFront(writeMissingFrameCount);
                    readFrameIndex += writeMissingFrameCount;
                }
                DEBUG_ASSERT((missingFrameCount == 0) || writableRange.empty());
                const auto writeDecodedFrameCount =
                        math_min(decodedFrameRange.length(), writableRange.length());
                if (writeDecodedFrameCount > 0) {
                    const auto copySampleCount =
                            frames2samples(writeDecodedFrameCount);
                    if (pOutputSampleBuffer) {
                        SampleUtil::copy(
                                pOutputSampleBuffer,
                                pDecodedSampleData,
                                copySampleCount);
                        pOutputSampleBuffer += copySampleCount;
                    }
                    pDecodedSampleData += copySampleCount;
                    decodedFrameRange.shrinkFront(writeDecodedFrameCount);
                    writableRange.shrinkFront(writeDecodedFrameCount);
                    readFrameIndex += writeDecodedFrameCount;
                }
                DEBUG_ASSERT(decodedFrameRange.empty() || writableRange.empty());
                DEBUG_ASSERT(readFrameIndex == writableRange.start());
                m_curFrameIndex = readFrameIndex;
            }

            /*
            kLogger.trace()
                    << "Before buffering:"
                    << "m_curFrameIndex" << m_curFrameIndex
                    << "readFrameIndex" << readFrameIndex
                    << "decodedFrameRange" << decodedFrameRange
                    << "writableRange" << writableRange
                    << "missingFrameCount" << missingFrameCount;
            */

            // Buffer remaining unread sample data from
            // missing/decoded ranges
            if (readFrameIndex >= writableRange.start()) {
                const auto sampleBufferWriteLength =
                        frames2samples(missingFrameCount + decodedFrameRange.length());
                if (m_sampleBuffer.writableLength() < sampleBufferWriteLength) {
                    const auto sampleBufferCapacity =
                            m_sampleBuffer.readableLength() +
                            sampleBufferWriteLength;
                    kLogger.warning()
                            << "Adjusting capacity of internal sample buffer by reallocation:"
                            << m_sampleBuffer.capacity()
                            << "->"
                            << sampleBufferCapacity;
                    m_sampleBuffer.adjustCapacity(sampleBufferCapacity);
                }
                DEBUG_ASSERT(m_sampleBuffer.writableLength() >= sampleBufferWriteLength);
                if (missingFrameCount > 0) {
                    DEBUG_ASSERT(writableRange.empty());
                    const auto clearSampleCount =
                            frames2samples(missingFrameCount);
                    const SampleBuffer::WritableSlice writableSlice(
                            m_sampleBuffer.growForWriting(clearSampleCount));
                    DEBUG_ASSERT(writableSlice.length() == clearSampleCount);
                    SampleUtil::clear(
                            writableSlice.data(),
                            clearSampleCount);
                    readFrameIndex += missingFrameCount;
                }
                if (!decodedFrameRange.empty()) {
                    DEBUG_ASSERT(writableRange.empty());
                    const auto copySampleCount =
                            frames2samples(decodedFrameRange.length());
                    const SampleBuffer::WritableSlice writableSlice(
                            m_sampleBuffer.growForWriting(copySampleCount));
                    DEBUG_ASSERT(writableSlice.length() == copySampleCount);
                    SampleUtil::copy(
                            writableSlice.data(),
                            pDecodedSampleData,
                            copySampleCount);
                    readFrameIndex += decodedFrameRange.length();
                }
            }

            // Housekeeping before next decoding iteration
            av_frame_unref(m_pavDecodedFrame);
            av_frame_unref(m_pavResampledFrame);
        } while ((avcodec_receive_frame_result == 0) && (m_curFrameIndex != kFrameIndexInvalid));
    }
    DEBUG_ASSERT(!pavNextPacket);

    const auto readableRange =
            IndexRange::between(
                    writableSampleFrames.frameIndexRange().start(),
                    writableRange.start());
    return ReadableSampleFrames(
            readableRange,
            SampleBuffer::ReadableSlice(
                    writableSampleFrames.writableData(),
                    frames2samples(readableRange.length())));
}

QString SoundSourceProviderFFmpeg4::getName() const {
    return "FFmpeg4";
}

SoundSourceProviderPriority SoundSourceProviderFFmpeg4::getPriorityHint(
        const QString& /*supportedFileExtension*/) const {
    // TODO: Increase priority to HIGHER if FFmpeg should be used as the
    // default decoder instead of other SoundSources. Currently it is
    // only used as a fallback.
    return SoundSourceProviderPriority::LOWER;
}

} // namespace mixxx
