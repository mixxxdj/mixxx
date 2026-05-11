#include "sources/soundsourcestem.h"

extern "C" {

#include <libavutil/avutil.h>
#if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(57, 28, 100) // FFmpeg 5.1
#include <libavutil/channel_layout.h>
#endif

} // extern "C"

#include <memory>

#include "sources/soundsourceffmpeg.h"
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

// Local RAII for AVFormatContext; SoundSourceFFmpeg's wrapper is private.
struct AVFormatContextDeleter {
    void operator()(AVFormatContext* ctx) const {
        if (ctx) {
            avformat_close_input(&ctx);
        }
    }
};
using AVFormatContextPtr =
        std::unique_ptr<AVFormatContext, AVFormatContextDeleter>;

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
    // Open input. RAII handles cleanup on every return path.
    AVFormatContextPtr pavInputFormatContextGuard(
            SoundSourceFFmpeg::openInputFile(getLocalFileName()));
    AVFormatContext* pavInputFormatContext = pavInputFormatContextGuard.get();
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

        m_pStereoStreams.emplace_back(std::make_unique<SoundSourceFFmpeg>(getUrl(), streamIdx));
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
            SampleUtil::add(pBuffer, m_buffer.data(), stemSampleLength);
        }
    }

    return read;
}

} // namespace mixxx
