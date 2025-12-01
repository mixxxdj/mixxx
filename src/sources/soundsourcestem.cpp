#include "sources/soundsourcestem.h"

#include "sources/readaheadframebuffer.h"

extern "C" {

#include <libavutil/avutil.h>
#if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(57, 28, 100) // FFmpeg 5.1
#include <libavutil/channel_layout.h>
#endif

} // extern "C"

#include "util/assert.h"
#include "util/logger.h"
#include "util/sample.h"

#if !defined(VERBOSE_DEBUG_LOG)
#define VERBOSE_DEBUG_LOG false
#endif

namespace mixxx {

namespace {

// STEM constants
constexpr int kNumStreams = 5;
constexpr int kRequiredStreamCount = kNumStreams - 1; // Stem count doesn't include the main mix

const Logger kLogger("SoundSourceSTEM");

} // anonymous namespace

const QString SoundSourceProviderSTEM::kDisplayName = QStringLiteral("STEM with FFmpeg");

QStringList SoundSourceProviderSTEM::getSupportedFileTypes() const {
    return {"stem.mp4", "stem.m4a"};
}

SoundSourceProviderPriority SoundSourceProviderSTEM::getPriorityHint(
        const QString& supportedFileType) const {
    Q_UNUSED(supportedFileType)
    return SoundSourceProviderPriority::Higher;
}

QString SoundSourceProviderSTEM::getVersionString() const {
    return QString::fromUtf8(av_version_info());
}

SoundSourceSingleSTEM::SoundSourceSingleSTEM(const QUrl& url, unsigned int streamIdx)
        : SoundSourceFFmpeg(url), m_streamIdx(streamIdx) {
}

SoundSource::OpenResult SoundSourceSingleSTEM::tryOpen(
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

    if (m_pavInputFormatContext->nb_streams <= m_streamIdx) {
        kLogger.warning().noquote()
                << "cannot find stream" << m_streamIdx;
        return OpenResult::Failed;
    }

    if (m_pavInputFormatContext->streams[m_streamIdx]->codecpar->codec_type !=
            AVMEDIA_TYPE_AUDIO) {
        kLogger.warning().noquote()
                << "selected stream isn't a valid audio stream";
        return OpenResult::Failed;
    }

    AVStream* selectedAudioStream = m_pavInputFormatContext->streams[m_streamIdx];

    // Open the decoder for these streams
    const AVCodec* pDecoder = avcodec_find_decoder(selectedAudioStream->codecpar->codec_id);
    if (!pDecoder) {
        kLogger.warning()
                << "av_find_best_stream() failed to find a decoder for any audio stream";
        return SoundSource::OpenResult::Aborted;
    }

    DEBUG_ASSERT(pDecoder);

    if (pDecoder->id == AV_CODEC_ID_AAC ||
            pDecoder->id == AV_CODEC_ID_AAC_LATM) {
        // We only allow AAC decoders that pass our seeking tests
        if (std::strcmp(pDecoder->name, "aac") != 0 && std::strcmp(pDecoder->name, "aac_at") != 0) {
            const AVCodec* pAacDecoder = avcodec_find_decoder_by_name("aac");
            if (pAacDecoder) {
                pDecoder = pAacDecoder;
            } else {
                kLogger.warning()
                        << "Internal aac decoder not found in your FFmpeg "
                           "build."
                        << "To enable AAC support, please install an FFmpeg "
                           "version with the internal aac decoder enabled."
                           "Note 1: The libfdk_aac decoder is no working properly "
                           "with Mixxx, FFmpeg's internal AAC decoder does."
                        << "Note 2: AAC decoding may be subject to patent "
                           "restrictions, depending on your country.";
            }
        }
    }

    kLogger.debug() << "using FFmpeg decoder:" << pDecoder->long_name;

    // Select the main mix stream for decoding
    AVStream* pavStream = selectedAudioStream;
    DEBUG_ASSERT(pavStream != nullptr);

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

    // Request output format
    pavCodecContext->request_sample_fmt = s_avSampleFormat;
    if (params.getSignalInfo().getChannelCount().isValid()) {
        // A dedicated number of channels for the output signal
        // has been requested. Forward this to FFmpeg to avoid
        // manual resampling or post-processing after decoding.
#if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(57, 28, 100) // FFmpeg 5.1
        av_channel_layout_default(&pavCodecContext->ch_layout,
                params.getSignalInfo().getChannelCount());
#else
        pavCodecContext->request_channel_layout =
                av_get_default_channel_layout(params.getSignalInfo().getChannelCount());
#endif
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
#if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(57, 28, 100) // FFmpeg 5.1
        AVChannelLayout fixedChannelLayout;
        initChannelLayoutFromStream(&fixedChannelLayout, *m_pavStream);
#endif
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
#if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(57, 28, 100) // FFmpeg 5.1
                << "| ch_layout.nb_channels" << m_pavStream->codecpar->ch_layout.nb_channels
                << "| ch_layout.order" << m_pavStream->codecpar->ch_layout.order
                << "| ch_layout.order (fixed)" << fixedChannelLayout.order
#else
                << "| channels" << m_pavStream->codecpar->channels
                << "| channel_layout" << m_pavStream->codecpar->channel_layout
                << "| channel_layout (fixed)" << getStreamChannelLayout(*m_pavStream)
#endif
                << "| format" << m_pavStream->codecpar->format
                << "| sample_rate" << m_pavStream->codecpar->sample_rate
                << "| bit_rate" << m_pavStream->codecpar->bit_rate
                << "| frame_size" << m_pavStream->codecpar->frame_size
                << "| seek_preroll" << m_pavStream->codecpar->seek_preroll
                << "| initial_padding" << m_pavStream->codecpar->initial_padding
                << "| trailing_padding" << m_pavStream->codecpar->trailing_padding
                << '}';
#if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(57, 28, 100) // FFmpeg 5.1
        av_channel_layout_uninit(&fixedChannelLayout);
#endif
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
    // range of the stream onto our internal range starting at FrameIndex 0.
    // See also the discussion regarding cue point shift/offset:
    // https://mixxx.zulipchat.com/#narrow/stream/109171-development/topic/Cue.20shift.2Foffset
    const auto frameIndexRange = IndexRange::forward(
            0,
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

SoundSourceSTEM::SoundSourceSTEM(const QUrl& url)
        : SoundSource(url) {
}

SoundSource::OpenResult SoundSourceSTEM::tryOpen(
        OpenMode /*mode*/,
        const OpenParams& params) {
    // Ensure that the source isn't yet opened
    VERIFY_OR_DEBUG_ASSERT(!m_requestedChannelCount.isValid()) {
        return OpenResult::Failed;
    }
    // Open input
    AVFormatContext* pavInputFormatContext =
            SoundSourceFFmpeg::openInputFile(getLocalFileName());
    if (pavInputFormatContext == nullptr) {
        kLogger.warning()
                << "Failed to open input file"
                << getLocalFileName();
        return OpenResult::Failed;
    }
#if VERBOSE_DEBUG_LOG
    kLogger.debug()
            << "AVFormatContext"
            << "{ nb_streams" << pavInputFormatContext->nb_streams
            << "| start_time" << pavInputFormatContext->start_time
            << "| duration" << pavInputFormatContext->duration
            << "| bit_rate" << pavInputFormatContext->bit_rate
            << "| packet_size" << pavInputFormatContext->packet_size
            << "| audio_codec_id" << pavInputFormatContext->audio_codec_id
            << "| output_ts_offset" << pavInputFormatContext->output_ts_offset
            << '}';
#endif

    // Retrieve stream information
    const int avformat_find_stream_info_result =
            avformat_find_stream_info(pavInputFormatContext, nullptr);
    if (avformat_find_stream_info_result != 0) {
        DEBUG_ASSERT(avformat_find_stream_info_result < 0);
        kLogger.warning().noquote()
                << "avformat_find_stream_info() failed:"
                << SoundSourceFFmpeg::formatErrorString(avformat_find_stream_info_result);
        return OpenResult::Failed;
    }

    bool foundPremixedStream = false;
    AVStream* firstStem = nullptr;
    int stemCount = 0;
    uint selectedStemMask = params.stemMask();
    VERIFY_OR_DEBUG_ASSERT(selectedStemMask <= 2 << mixxx::kMaxSupportedStems) {
        kLogger.warning().noquote()
                << "Invalid selected stem mask" << selectedStemMask;
        return OpenResult::Failed;
    }
    OpenParams stemParam = params;
    stemParam.setChannelCount(mixxx::audio::ChannelCount::stereo());
    for (unsigned int streamIdx = 0; streamIdx < pavInputFormatContext->nb_streams; streamIdx++) {
        if (pavInputFormatContext->streams[streamIdx]->codecpar->codec_type !=
                AVMEDIA_TYPE_AUDIO) {
            continue;
        }

#if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(57, 28, 100) // FFmpeg 5.1
        if (pavInputFormatContext->streams[streamIdx]->codecpar->ch_layout.nb_channels !=
                mixxx::audio::ChannelCount::stereo()) {
#else
        if (pavInputFormatContext->streams[streamIdx]->codecpar->channels !=
                mixxx::audio::ChannelCount::stereo()) {
#endif
            kLogger.warning().noquote()
                    << "stream at position" << streamIdx << "is not in stereo";
            return OpenResult::Failed;
        }

        if (!foundPremixedStream) {
            // We can currently allow this, as we NEVER LOAD the pre-mastered
            // track, as we do not have analyzer data for it.
            // This is because we only support one set of metadata for the whole
            // STEM file, where we determine the track parameters from an
            // on-the-fly mix of all 4 stems. Especially the replaygain differs
            // between the on-the-fly mix and the pre-mastered track, because we
            // do not apply DSP (limiter, equalizer, compressor) to the
            // on-the-fly mix. If this ever gets changed, we should set
            // `initSampleRateOnce` and `initBitrateOnce` to match the stem
            // sample rate and bit rate, such that
            // SoundSourceFFmpeg::resampleDecodedAVFrame will take care to
            // resample the main stream, in order to use the same time scale and
            // keep a working grid/cue definition
            foundPremixedStream = true;
            continue;
        }

        stemCount++;

        if (!firstStem) {
            // We always keep track of the stem to verify that stem stream properties are matching
            firstStem = pavInputFormatContext->streams[streamIdx];
        } else {
            if (pavInputFormatContext->streams[streamIdx]->codecpar->codec_id !=
                    firstStem->codecpar->codec_id) {
                kLogger.warning().noquote()
                        << "Stem at position" << streamIdx << "is using a different codec";
                return OpenResult::Failed;
            }

            if (pavInputFormatContext->streams[streamIdx]
                            ->codecpar->sample_rate !=
                    firstStem->codecpar->sample_rate) {
                kLogger.warning().noquote()
                        << "Stem at position" << streamIdx << "is using a different sample rate";
                return OpenResult::Failed;
            }
        }

        // StemIdx is equal to StreamIdx -1 (the main mix)
        if (selectedStemMask && !(selectedStemMask & 1 << (streamIdx - 1))) {
            continue;
        }

        m_pStereoStreams.emplace_back(std::make_unique<SoundSourceSingleSTEM>(getUrl(), streamIdx));
        if (m_pStereoStreams.back()->open(OpenMode::Strict /*Unused*/,
                    stemParam) != OpenResult::Succeeded) {
            return OpenResult::Failed;
        }
    }

    if (stemCount != kRequiredStreamCount) {
        kLogger.warning().noquote()
                << "expected to find" << kRequiredStreamCount
                << "stem but found" << stemCount;
        close();
        return OpenResult::Failed;
    }

    VERIFY_OR_DEBUG_ASSERT(!m_pStereoStreams.empty()) {
        kLogger.warning().noquote()
                << "no stem track were selected";
        close();
        return OpenResult::Failed;
    }

    if (params.getSignalInfo().getChannelCount() ==
                    mixxx::audio::ChannelCount::stereo() ||
            selectedStemMask) {
        // Requesting a stereo stream (used for samples and preview decks)
        m_requestedChannelCount = mixxx::audio::ChannelCount::stereo();
        initChannelCountOnce(mixxx::audio::ChannelCount::stereo());
    } else {
        // No special channel format request
        m_requestedChannelCount = mixxx::audio::ChannelCount::stem();
        initChannelCountOnce(
                static_cast<int>(mixxx::audio::ChannelCount::stereo() *
                        m_pStereoStreams.size()));
    }

    initSampleRateOnce(m_pStereoStreams.front()->getSignalInfo().getSampleRate());
    initBitrateOnce(m_pStereoStreams.front()->getBitrate());
    initFrameIndexRangeOnce(m_pStereoStreams.front()->frameIndexRange());

    return OpenResult::Succeeded;
}

void SoundSourceSTEM::close() {
    for (auto& stream : m_pStereoStreams) {
        stream->close();
    }
}

ReadableSampleFrames SoundSourceSTEM::readSampleFramesClamped(
        const WritableSampleFrames& globalSampleFrames) {
    VERIFY_OR_DEBUG_ASSERT(m_requestedChannelCount.isValid()) {
        return ReadableSampleFrames();
    }

    VERIFY_OR_DEBUG_ASSERT(globalSampleFrames.writableLength() %
                    m_requestedChannelCount ==
            0) {
        return ReadableSampleFrames();
    };

    SINT stemSampleLength = m_pStereoStreams.front()->getSignalInfo().frames2samples(
            globalSampleFrames.frameLength());

    // The same buffer is reused between requests tp prevent reallocation, but
    // it will be reallocated if a larger chunk is requested and will keep the
    // new maximum size
    if (stemSampleLength > m_buffer.size()) {
        m_buffer = SampleBuffer(stemSampleLength);
    }

    ReadableSampleFrames read(globalSampleFrames.frameIndexRange(),
            SampleBuffer::ReadableSlice(
                    globalSampleFrames.writableData(),
                    globalSampleFrames.writableLength()));
    std::size_t stemCount = m_pStereoStreams.size();
    CSAMPLE* pBuffer = globalSampleFrames.writableData();

    if (m_requestedChannelCount == mixxx::audio::ChannelCount::stereo() && stemCount != 1) {
        SampleUtil::clear(pBuffer, globalSampleFrames.writableLength());
    } else {
        DEBUG_ASSERT(stemSampleLength * static_cast<SINT>(stemCount) ==
                globalSampleFrames.writableLength());
    }

    if (stemCount == 1) {
        m_pStereoStreams[0]->readSampleFrames(globalSampleFrames);
        return read;
    }

    for (std::size_t streamIdx = 0; streamIdx < stemCount; streamIdx++) {
        WritableSampleFrames currentStemFrame = WritableSampleFrames(
                globalSampleFrames.frameIndexRange(),
                SampleBuffer::WritableSlice(
                        m_buffer.data(),
                        stemSampleLength));
        m_pStereoStreams[streamIdx]->readSampleFrames(currentStemFrame);

        // TODO(XXX): currently, stem samples are interleaved and packed
        // next to each other as such:
        //    1L1R1L1R1L1R...2L2R2L2R2L2R2L2R......3L3R3L3R3L3R3L3R......4L4R4L4R4L4R4L4R....
        //    Can FFmpeg decode as without having to use a decoder per
        //    channel? 1LLLLLLLLLLLLLL....1RRRRRRRRR...2LLLLLLL...?
        if (m_requestedChannelCount != mixxx::audio::ChannelCount::stereo()) {
            // Change the sample layout to interleave all channels together
            for (SINT i = 0; i < stemSampleLength / 2; i++) {
                pBuffer[2 * stemCount * i + 2 * streamIdx] = m_buffer[2 * i];
                pBuffer[2 * stemCount * i + 2 * streamIdx + 1] = m_buffer[2 * i + 1];
            }
        } else {
            // Change the sample layout to mix all channels together
            for (SINT i = 0; i < stemSampleLength / 2; i++) {
                pBuffer[2 * i] += m_buffer[2 * i];
                pBuffer[2 * i + 1] += m_buffer[2 * i + 1];
            }
        }
    }

    return read;
}

} // namespace mixxx
