#include "sources/soundsourceopus.h"

#include <QFileInfo>

#include "util/logger.h"

namespace mixxx {

namespace {

const Logger kLogger("SoundSourceOpus");

// Decoded output of opusfile has a fixed sample rate of 48 kHz (fullband)
constexpr audio::SampleRate kSampleRate = audio::SampleRate(48000);

// http://opus-codec.org
//  - Sample rate 48 kHz (fullband)
//  - Frame sizes from 2.5 ms to 60 ms
//   => Up to 48000 kHz * 0.06 s = 2880 sample frames per data frame
// Prefetching 2 * 2880 sample frames while seeking limits the decoding
// errors to kMaxDecodingError during our tests.
//
// According to the API documentation of op_pcm_seek():
// "...decoding after seeking may not return exactly the same
// values as would be obtained by decoding the stream straight
// through. However, such differences are expected to be smaller
// than the loss introduced by Opus's lossy compression."
// This implementation internally uses prefetching to compensate
// those differences, although not completely. The following
// constant indicates the maximum expected difference for
// testing purposes.
constexpr SINT kNumberOfPrefetchFrames = 2 * 2880;

// Parameter for op_channel_count()
// See also: https://mf4.xiph.org/jenkins/view/opus/job/opusfile-unix/ws/doc/html/group__stream__info.html
constexpr int kCurrentStreamLink = -1; // get ... of the current (stream) link

// Parameter for op_pcm_total() and op_bitrate()
// See also: https://mf4.xiph.org/jenkins/view/opus/job/opusfile-unix/ws/doc/html/group__stream__info.html
constexpr int kEntireStreamLink = -1; // get ... of the whole/entire stream

class OggOpusFileOwner {
  public:
    explicit OggOpusFileOwner(OggOpusFile* pFile)
            : m_pFile(pFile) {
    }
    OggOpusFileOwner(OggOpusFileOwner&&) = delete;
    OggOpusFileOwner(const OggOpusFileOwner&) = delete;
    ~OggOpusFileOwner() {
        if (m_pFile) {
            op_free(m_pFile);
        }
    }
    operator OggOpusFile*() const {
        return m_pFile;
    }
    OggOpusFile* release() {
        OggOpusFile* pFile = m_pFile;
        m_pFile = nullptr;
        return pFile;
    }

  private:
    OggOpusFile* m_pFile;
};

} // anonymous namespace

//static
const QString SoundSourceProviderOpus::kDisplayName = QStringLiteral("Xiph.org libopusfile");

//static
const QStringList SoundSourceProviderOpus::kSupportedFileTypes = {
        QStringLiteral("opus"),
};

SoundSourceProviderPriority SoundSourceProviderOpus::getPriorityHint(
        const QString& supportedFileType) const {
    Q_UNUSED(supportedFileType)
    // This reference decoder is supposed to produce more accurate
    // and reliable results than any other DEFAULT provider.
    return SoundSourceProviderPriority::Higher;
}

SoundSourceOpus::SoundSourceOpus(const QUrl& url)
        : SoundSource(url),
          m_pOggOpusFile(nullptr),
          m_curFrameIndex(0) {
}

SoundSourceOpus::~SoundSourceOpus() {
    close();
}

SoundSource::OpenResult SoundSourceOpus::tryOpen(
        OpenMode /*mode*/,
        const OpenParams& params) {
    // From opus/opusfile.h
    // On Windows, this string must be UTF-8 (to allow access to
    // files whose names cannot be represented in the current
    // MBCS code page).
    // All other systems use the native character encoding.
#ifdef _WIN32
    QByteArray qBAFilename = getLocalFileName().toUtf8();
#else
    QByteArray qBAFilename = QFile::encodeName(getLocalFileName());
#endif

    int errorCode = 0;
    OggOpusFileOwner pOggOpusFile(
            op_open_file(qBAFilename.constData(), &errorCode));
    if (!pOggOpusFile || (errorCode != 0)) {
        kLogger.warning()
                << "Opening of OggOpusFile failed with error"
                << errorCode
                << ":"
                << getLocalFileName();
        return OpenResult::Failed;
    }
    if (!op_seekable(pOggOpusFile)) {
        kLogger.warning()
                << "Stream in"
                << getUrlString()
                << "is not seekable";
        return OpenResult::Aborted;
    }
    DEBUG_ASSERT(!m_pOggOpusFile);
    m_pOggOpusFile = pOggOpusFile.release();

    const int streamChannelCount = op_channel_count(m_pOggOpusFile, kCurrentStreamLink);
    if (0 < streamChannelCount) {
        // opusfile supports to enforce stereo decoding
        bool enforceStereoDecoding =
                params.getSignalInfo().getChannelCount().isValid() &&
                (params.getSignalInfo().getChannelCount() <= 2) &&
                // preserve mono signals if stereo signal is not requested explicitly
                ((params.getSignalInfo().getChannelCount() == 2) || (streamChannelCount > 2));
        if (enforceStereoDecoding) {
            initChannelCountOnce(2);
        } else {
            initChannelCountOnce(streamChannelCount);
        }
    } else {
        kLogger.warning()
                << "Failed to read channel configuration of OggOpus file:"
                << getUrlString();
        return OpenResult::Failed;
    }

    // Reserve enough capacity for buffering a stereo signal!
    const auto prefetchChannelCount = std::min(getSignalInfo().getChannelCount(), audio::ChannelCount(2));
    SampleBuffer(prefetchChannelCount * kNumberOfPrefetchFrames).swap(m_prefetchSampleBuffer);

    const ogg_int64_t pcmTotal = op_pcm_total(m_pOggOpusFile, kEntireStreamLink);
    if (0 <= pcmTotal) {
        initFrameIndexRangeOnce(IndexRange::forward(0, pcmTotal));
    } else {
        kLogger.warning()
                << "Failed to read total length of OggOpus file:"
                << getUrlString();
        return OpenResult::Failed;
    }

    const opus_int32 bitrate = op_bitrate(m_pOggOpusFile, kEntireStreamLink);
    if (0 < bitrate) {
        initBitrateOnce(bitrate / 1000);
    } else {
        kLogger.warning()
                << "Failed to determine bitrate of OggOpus file:"
                << getUrlString();
        return OpenResult::Failed;
    }

    initSampleRateOnce(kSampleRate);

    m_curFrameIndex = frameIndexMin();

    return OpenResult::Succeeded;
}

void SoundSourceOpus::close() {
    if (m_pOggOpusFile) {
        op_free(m_pOggOpusFile);
        m_pOggOpusFile = nullptr;
    }
}

ReadableSampleFrames SoundSourceOpus::readSampleFramesClamped(
        const WritableSampleFrames& writableSampleFrames) {
    const SINT firstFrameIndex = writableSampleFrames.frameIndexRange().start();

    if (m_curFrameIndex != firstFrameIndex) {
        // Prefer skipping over seeking if the seek position is up to
        // 2 * kNumberOfPrefetchFrames in front of the current position
        if ((m_curFrameIndex > firstFrameIndex) ||
                ((firstFrameIndex - m_curFrameIndex) > 2 * kNumberOfPrefetchFrames)) {
            SINT seekIndex = std::max(firstFrameIndex - kNumberOfPrefetchFrames, frameIndexMin());
            int seekResult = op_pcm_seek(m_pOggOpusFile, seekIndex);
            if (0 == seekResult) {
                m_curFrameIndex = seekIndex;
            } else {
                kLogger.warning() << "Failed to seek OggOpus file:" << seekResult;
                const ogg_int64_t pcmOffset = op_pcm_tell(m_pOggOpusFile);
                if (0 <= pcmOffset) {
                    m_curFrameIndex = pcmOffset;
                } else {
                    // Reset to EOF
                    m_curFrameIndex = frameIndexMax();
                }
                // Abort
                return ReadableSampleFrames(
                        IndexRange::between(
                                m_curFrameIndex,
                                m_curFrameIndex));
            }
        }
        // Decoding starts before the actual target position
        // -> skip decoded samples until reaching the target position
        DEBUG_ASSERT(m_curFrameIndex <= firstFrameIndex);
        const auto precedingFrames =
                IndexRange::between(m_curFrameIndex, firstFrameIndex);
        if (!precedingFrames.empty() && (precedingFrames != readSampleFramesClamped(WritableSampleFrames(precedingFrames)).frameIndexRange())) {
            kLogger.warning()
                    << "Failed to skip preceding frames"
                    << precedingFrames;
            return ReadableSampleFrames(IndexRange::between(m_curFrameIndex, m_curFrameIndex));
        }
    }
    DEBUG_ASSERT(m_curFrameIndex == firstFrameIndex);

    const SINT numberOfFramesTotal = writableSampleFrames.frameLength();

    // pSampleBuffer might be null while skipping (see above)
    CSAMPLE* pSampleBuffer = writableSampleFrames.writableData();
    SINT numberOfFramesRemaining = numberOfFramesTotal;
    while (0 < numberOfFramesRemaining) {
        SINT numberOfSamplesToRead =
                getSignalInfo().frames2samples(numberOfFramesRemaining);
        if (!writableSampleFrames.writableData()) {
            // NOTE(uklotzde): The opusfile API does not provide any
            // functions for skipping samples in the audio stream. Calling
            // API functions with a nullptr buffer does not return. Since
            // seeking in Opus files requires prefetching + skipping we
            // need to skip sample frames by reading into a temporary
            // buffer
            pSampleBuffer = m_prefetchSampleBuffer.data();
            if (numberOfSamplesToRead > m_prefetchSampleBuffer.size()) {
                numberOfSamplesToRead = m_prefetchSampleBuffer.size();
            }
        }
        int readResult;
        if (getSignalInfo().getChannelCount() == 2) {
            readResult = op_read_float_stereo(
                    m_pOggOpusFile,
                    pSampleBuffer,
                    numberOfSamplesToRead);
        } else {
            readResult = op_read_float(
                    m_pOggOpusFile,
                    pSampleBuffer,
                    numberOfSamplesToRead,
                    nullptr);
        }
        if (0 < readResult) {
            m_curFrameIndex += readResult;
            pSampleBuffer += getSignalInfo().frames2samples(readResult);
            numberOfFramesRemaining -= readResult;
        } else {
            kLogger.warning() << "Failed to read sample data from OggOpus file:"
                              << readResult;
            break; // abort
        }
    }

    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
    DEBUG_ASSERT(numberOfFramesTotal >= numberOfFramesRemaining);
    const SINT numberOfFrames = numberOfFramesTotal - numberOfFramesRemaining;
    return ReadableSampleFrames(
            IndexRange::forward(firstFrameIndex, numberOfFrames),
            SampleBuffer::ReadableSlice(
                    writableSampleFrames.writableData(),
                    std::min(writableSampleFrames.writableLength(), getSignalInfo().frames2samples(numberOfFrames))));
}

} // namespace mixxx
