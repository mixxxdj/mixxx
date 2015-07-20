#include "soundsourcem4a.h"

#include "sampleutil.h"

#ifdef __WINDOWS__
#include <io.h>
#include <fcntl.h>
#endif

#ifdef _MSC_VER
#define S_ISDIR(mode) (mode & _S_IFDIR)
#define strcasecmp stricmp
#endif

// TODO(XXX): Do we still need this "hack" on the supported platforms?
#ifdef __M4AHACK__
typedef uint32_t SAMPLERATE_TYPE;
#else
typedef unsigned long SAMPLERATE_TYPE;
#endif

namespace Mixxx {

namespace {

// According to the specification each AAC sample block decodes
// to 1024 sample frames. The rarely used AAC-960 profile with
// a block size of 960 frames is not supported.
const SINT kFramesPerSampleBlock = 1024;

// MP4SampleId is 1-based
const MP4SampleId kSampleBlockIdMin = 1;

inline SINT getFrameIndexForSampleBlockId(
        MP4SampleId sampleBlockId) {
    return AudioSource::getMinFrameIndex() +
        (sampleBlockId - kSampleBlockIdMin) * kFramesPerSampleBlock;
}

inline SINT getFrameCountForSampleBlockId(
        MP4SampleId sampleBlockId) {
    return ((sampleBlockId - kSampleBlockIdMin) + 1) * kFramesPerSampleBlock;
}

// Decoding will be restarted one or more blocks of samples
// before the actual position after seeking randomly in the
// audio stream to avoid audible glitches.
//
// "AAC Audio - Encoder Delay and Synchronization: The 2112 Sample Assumption"
// https://developer.apple.com/library/ios/technotes/tn2258/_index.html
// "It must also be assumed that without an explicit value, the playback
// system will trim 2112 samples from the AAC decoder output when starting
// playback from any point in the bistream."
//
// 2112 frames = 2 * 1024 + 64 < 3 * 1024 = 3 sample blocks
//
// Decoding 3 blocks of samples in advance after seeking should
// compensate for the encoding delay and allow sample accurate
// seeking.
const MP4SampleId kNumberOfPrefetchSampleBlocks = 3;

// Searches for the first audio track in the MP4 file that
// suits our needs.
MP4TrackId findFirstAudioTrackId(MP4FileHandle hFile) {
    const MP4TrackId maxTrackId = MP4GetNumberOfTracks(hFile, nullptr, 0);
    for (MP4TrackId trackId = 1; trackId <= maxTrackId; ++trackId) {
        const char* trackType = MP4GetTrackType(hFile, trackId);
        if ((nullptr == trackType) || !MP4_IS_AUDIO_TRACK_TYPE(trackType)) {
            continue;
        }
        const char* mediaDataName = MP4GetTrackMediaDataName(hFile, trackId);
        if ((nullptr == mediaDataName) || (0 != strcasecmp(mediaDataName, "mp4a"))) {
            continue;
        }
        const u_int8_t audioType = MP4GetTrackEsdsObjectTypeId(hFile, trackId);
        if (MP4_INVALID_AUDIO_TYPE == audioType) {
            continue;
        }
        if (MP4_MPEG4_AUDIO_TYPE == audioType) {
            const u_int8_t audioMpeg4Type = MP4GetTrackAudioMpeg4Type(hFile,
                    trackId);
            if (MP4_IS_MPEG4_AAC_AUDIO_TYPE(audioMpeg4Type)) {
                return trackId;
            }
        } else if (MP4_IS_AAC_AUDIO_TYPE(audioType)) {
            return trackId;
        }
    }
    return MP4_INVALID_TRACK_ID;
}

} // anonymous namespace

SoundSourceM4A::SoundSourceM4A(const QUrl& url)
        : SoundSourcePlugin(url, "m4a"),
          m_hFile(MP4_INVALID_FILE_HANDLE),
          m_trackId(MP4_INVALID_TRACK_ID),
          m_maxSampleBlockId(MP4_INVALID_SAMPLE_ID),
          m_curSampleBlockId(MP4_INVALID_SAMPLE_ID),
          m_inputBufferLength(0),
          m_inputBufferOffset(0),
          m_hDecoder(nullptr),
          m_curFrameIndex(getMinFrameIndex()) {
}

SoundSourceM4A::~SoundSourceM4A() {
    close();
}

Result SoundSourceM4A::tryOpen(const AudioSourceConfig& audioSrcCfg) {
    DEBUG_ASSERT(MP4_INVALID_FILE_HANDLE == m_hFile);
    /* open MP4 file, check for >= ver 1.9.1 */
#if MP4V2_PROJECT_version_hex <= 0x00010901
    m_hFile = MP4Read(getLocalFileNameBytes().constData(), 0);
#else
    m_hFile = MP4Read(getLocalFileNameBytes().constData());
#endif
    if (MP4_INVALID_FILE_HANDLE == m_hFile) {
        qWarning() << "Failed to open file for reading:" << getUrlString();
        return ERR;
    }

    m_trackId = findFirstAudioTrackId(m_hFile);
    if (MP4_INVALID_TRACK_ID == m_trackId) {
        qWarning() << "No AAC track found:" << getUrlString();
        return ERR;
    }

    const MP4SampleId numberOfSamples =
            MP4GetTrackNumberOfSamples(m_hFile, m_trackId);
    if (0 >= numberOfSamples) {
        qWarning() << "Failed to read number of samples from file:" << getUrlString();
        return ERR;
    }
    m_maxSampleBlockId = kSampleBlockIdMin + (numberOfSamples - 1);

    // Determine the maximum input size (in bytes) of a
    // sample block for the selected track.
    const u_int32_t maxSampleBlockInputSize = MP4GetTrackMaxSampleSize(m_hFile,
            m_trackId);
    m_inputBuffer.resize(maxSampleBlockInputSize, 0);

    DEBUG_ASSERT(nullptr == m_hDecoder); // not already opened
    m_hDecoder = NeAACDecOpen();
    if (!m_hDecoder) {
        qWarning() << "Failed to open the AAC decoder!";
        return ERR;
    }
    NeAACDecConfigurationPtr pDecoderConfig = NeAACDecGetCurrentConfiguration(
            m_hDecoder);
    pDecoderConfig->outputFormat = FAAD_FMT_FLOAT;
    if ((kChannelCountMono == audioSrcCfg.channelCountHint) ||
            (kChannelCountStereo == audioSrcCfg.channelCountHint)) {
        pDecoderConfig->downMatrix = 1;
    } else {
        pDecoderConfig->downMatrix = 0;
    }

    pDecoderConfig->defObjectType = LC;
    if (!NeAACDecSetConfiguration(m_hDecoder, pDecoderConfig)) {
        qWarning() << "Failed to configure AAC decoder!";
        return ERR;
    }

    u_int8_t* configBuffer = nullptr;
    u_int32_t configBufferSize = 0;
    if (!MP4GetTrackESConfiguration(m_hFile, m_trackId, &configBuffer,
            &configBufferSize)) {
        /* failed to get mpeg-4 audio config... this is ok.
         * NeAACDecInit2() will simply use default values instead.
         */
        qWarning() << "Failed to read the MP4 audio configuration."
                << "Continuing with default values.";
    }

    SAMPLERATE_TYPE sampleRate;
    unsigned char channelCount;
    if (0 > NeAACDecInit2(m_hDecoder, configBuffer, configBufferSize,
                    &sampleRate, &channelCount)) {
        free(configBuffer);
        qWarning() << "Failed to initialize the AAC decoder!";
        return ERR;
    } else {
        free(configBuffer);
    }

    setChannelCount(channelCount);
    setFrameRate(sampleRate);
    setFrameCount(getFrameCountForSampleBlockId(m_maxSampleBlockId));

    // Resize temporary buffer for decoded sample data
    const SINT sampleBufferCapacity =
            frames2samples(kFramesPerSampleBlock);
    m_sampleBuffer.resetCapacity(sampleBufferCapacity);

    // Invalidate current position to enforce the following
    // seek operation
    m_curFrameIndex = getMaxFrameIndex();

    // (Re-)Start decoding at the beginning of the file
    seekSampleFrame(getMinFrameIndex());

    return OK;
}

void SoundSourceM4A::close() {
    if (m_hDecoder) {
        NeAACDecClose(m_hDecoder);
        m_hDecoder = nullptr;
    }
    if (MP4_INVALID_FILE_HANDLE != m_hFile) {
        MP4Close(m_hFile);
        m_hFile = MP4_INVALID_FILE_HANDLE;
    }
    m_inputBuffer.clear();
}

bool SoundSourceM4A::isValidSampleBlockId(MP4SampleId sampleBlockId) const {
    return (sampleBlockId >= kSampleBlockIdMin)
            && (sampleBlockId <= m_maxSampleBlockId);
}

void SoundSourceM4A::restartDecoding(MP4SampleId sampleBlockId) {
    DEBUG_ASSERT(MP4_INVALID_SAMPLE_ID != sampleBlockId);

    NeAACDecPostSeekReset(m_hDecoder, sampleBlockId);
    m_curSampleBlockId = sampleBlockId;
    m_curFrameIndex = getFrameIndexForSampleBlockId(m_curSampleBlockId);

    // Discard input buffer
    m_inputBufferLength = 0;

    // Discard previously decoded sample data
    m_sampleBuffer.reset();
}

SINT SoundSourceM4A::seekSampleFrame(SINT frameIndex) {
    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
    DEBUG_ASSERT(isValidFrameIndex(frameIndex));

    // Handle trivial case
    if (m_curFrameIndex == frameIndex) {
        // Nothing to do
        return m_curFrameIndex;
    }
    // Handle edge case
    if (getMaxFrameIndex() <= frameIndex) {
        // EOF reached
        m_curFrameIndex = getMaxFrameIndex();
        return m_curFrameIndex;
    }

    MP4SampleId sampleBlockId = kSampleBlockIdMin
            + (frameIndex / kFramesPerSampleBlock);
    DEBUG_ASSERT(isValidSampleBlockId(sampleBlockId));
    if ((frameIndex < m_curFrameIndex) || // seeking backwards?
            !isValidSampleBlockId(m_curSampleBlockId) || // invalid seek position?
            (sampleBlockId
                    > (m_curSampleBlockId + kNumberOfPrefetchSampleBlocks))) { // jumping forward?
        // Restart decoding one or more blocks of samples backwards
        // from the calculated starting block to avoid audible glitches.
        // Implementation note: The type MP4SampleId is unsigned so we
        // need to be careful when subtracting!
        if ((kSampleBlockIdMin + kNumberOfPrefetchSampleBlocks)
                < sampleBlockId) {
            sampleBlockId -= kNumberOfPrefetchSampleBlocks;
        } else {
            sampleBlockId = kSampleBlockIdMin;
        }
        restartDecoding(sampleBlockId);
        DEBUG_ASSERT(m_curSampleBlockId == sampleBlockId);
    }

    // Decoding starts before the actual target position
    DEBUG_ASSERT(m_curFrameIndex <= frameIndex);

    // Skip (= decode and discard) all samples up to the target position
    const SINT prefetchFrameCount = frameIndex - m_curFrameIndex;
    const SINT skipFrameCount = skipSampleFrames(prefetchFrameCount);
    DEBUG_ASSERT(skipFrameCount <= prefetchFrameCount);
    if (skipFrameCount < prefetchFrameCount) {
        qWarning() << "Failed to prefetch sample data while seeking"
                << skipFrameCount << "<" << prefetchFrameCount;
    }

    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
    return m_curFrameIndex;
}

SINT SoundSourceM4A::readSampleFrames(
        SINT numberOfFrames, CSAMPLE* sampleBuffer) {
    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));

    const SINT numberOfFramesTotal = math_min(
            numberOfFrames, getMaxFrameIndex() - m_curFrameIndex);
    const SINT numberOfSamplesTotal = frames2samples(numberOfFramesTotal);

    CSAMPLE* pSampleBuffer = sampleBuffer;
    SINT numberOfSamplesRemaining = numberOfSamplesTotal;
    while (0 < numberOfSamplesRemaining) {

        if (!m_sampleBuffer.isEmpty()) {
            // Consume previously decoded sample data
            const SampleBuffer::ReadableChunk readableChunk(
                    m_sampleBuffer.readFromHead(numberOfSamplesRemaining));
            if (pSampleBuffer) {
                SampleUtil::copy(pSampleBuffer, readableChunk.data(), readableChunk.size());
                pSampleBuffer += readableChunk.size();
            }
            m_curFrameIndex += samples2frames(readableChunk.size());
            DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
            DEBUG_ASSERT(numberOfSamplesRemaining >= readableChunk.size());
            numberOfSamplesRemaining -= readableChunk.size();
            if (0 == numberOfSamplesRemaining) {
                break; // exit loop
            }
        }
        // All previously decoded sample data has been consumed now
        DEBUG_ASSERT(m_sampleBuffer.isEmpty());

        if (0 == m_inputBufferLength) {
            // Fill input buffer from file
            if (isValidSampleBlockId(m_curSampleBlockId)) {
                // Read data for next sample block into input buffer
                u_int8_t* pInputBuffer = &m_inputBuffer[0];
                u_int32_t inputBufferLength = m_inputBuffer.size(); // in/out parameter
                if (!MP4ReadSample(m_hFile, m_trackId, m_curSampleBlockId,
                        &pInputBuffer, &inputBufferLength,
                        nullptr, nullptr, nullptr, nullptr)) {
                    qWarning()
                            << "Failed to read MP4 input data for sample block"
                            << m_curSampleBlockId << "(" << "min ="
                            << kSampleBlockIdMin << "," << "max ="
                            << m_maxSampleBlockId << ")";
                    break; // abort
                }
                ++m_curSampleBlockId;
                m_inputBufferLength = inputBufferLength;
                m_inputBufferOffset = 0;
            }
        }
        DEBUG_ASSERT(0 <= m_inputBufferLength);
        if (0 == m_inputBufferLength) {
            break; // EOF
        }

        // NOTE(uklotzde): The sample buffer for NeAACDecDecode2 has to
        // be big enough for a whole block of decoded samples, which
        // contains up to kFramesPerSampleBlock frames. Otherwise
        // we need to use a temporary buffer.
        CSAMPLE* pDecodeBuffer; // in/out parameter
        SINT decodeBufferCapacity;
        const SINT decodeBufferCapacityMin = frames2samples(kFramesPerSampleBlock);
        if (pSampleBuffer && (decodeBufferCapacityMin <= numberOfSamplesRemaining)) {
            // Decode samples directly into sampleBuffer
            pDecodeBuffer = pSampleBuffer;
            decodeBufferCapacity = numberOfSamplesRemaining;
        } else {
            // Decode next sample block into temporary buffer
            const SINT writeToTailCount = math_max(
                    numberOfSamplesRemaining, decodeBufferCapacityMin);
            const SampleBuffer::WritableChunk writableChunk(
                    m_sampleBuffer.writeToTail(writeToTailCount));
            pDecodeBuffer = writableChunk.data();
            decodeBufferCapacity = writableChunk.size();
        }
        DEBUG_ASSERT(decodeBufferCapacityMin <= decodeBufferCapacity);

        NeAACDecFrameInfo decFrameInfo;
        void* pDecodeResult = NeAACDecDecode2(
                m_hDecoder, &decFrameInfo,
                &m_inputBuffer[m_inputBufferOffset],
                m_inputBufferLength,
                reinterpret_cast<void**>(&pDecodeBuffer),
                decodeBufferCapacity * sizeof(*pDecodeBuffer));
        // Verify the decoding result
        if (0 != decFrameInfo.error) {
            qWarning() << "AAC decoding error:"
                    << decFrameInfo.error
                    << NeAACDecGetErrorMessage(decFrameInfo.error)
                    << getUrlString();
            break; // abort
        }
        DEBUG_ASSERT(pDecodeResult == pDecodeBuffer); // verify the in/out parameter

        // Verify the decoded sample data for consistency
        if (getChannelCount() != decFrameInfo.channels) {
            qWarning() << "Corrupt or unsupported AAC file:"
                    << "Unexpected number of channels" << decFrameInfo.channels
                    << "<>" << getChannelCount();
            break; // abort
        }
        if (getFrameRate() != SINT(decFrameInfo.samplerate)) {
            qWarning() << "Corrupt or unsupported AAC file:"
                    << "Unexpected sample rate" << decFrameInfo.samplerate
                    << "<>" << getFrameRate();
            break; // abort
        }

        // Consume input data
        m_inputBufferLength -= decFrameInfo.bytesconsumed;
        m_inputBufferOffset += decFrameInfo.bytesconsumed;

        // Consume decoded output data
        const SINT numberOfSamplesDecoded = decFrameInfo.samples;
        DEBUG_ASSERT(numberOfSamplesDecoded <= decodeBufferCapacity);
        SINT numberOfSamplesRead;
        if (pDecodeBuffer == pSampleBuffer) {
            numberOfSamplesRead = math_min(numberOfSamplesDecoded, numberOfSamplesRemaining);
            pSampleBuffer += numberOfSamplesRead;
        } else {
            m_sampleBuffer.readFromTail(decodeBufferCapacity - numberOfSamplesDecoded);
            const SampleBuffer::ReadableChunk readableChunk(
                    m_sampleBuffer.readFromHead(numberOfSamplesRemaining));
            numberOfSamplesRead = readableChunk.size();
            if (pSampleBuffer) {
                SampleUtil::copy(pSampleBuffer, readableChunk.data(), numberOfSamplesRead);
                pSampleBuffer += numberOfSamplesRead;
            }
        }
        // The decoder might decode more samples than actually needed
        // at the end of the file! When the end of the file has been
        // reached decoding can be restarted by seeking to a new
        // position.
        DEBUG_ASSERT(numberOfSamplesDecoded >= numberOfSamplesRead);
        m_curFrameIndex += samples2frames(numberOfSamplesRead);
        DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
        DEBUG_ASSERT(numberOfSamplesRemaining >= numberOfSamplesRead);
        numberOfSamplesRemaining -= numberOfSamplesRead;
    }

    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
    DEBUG_ASSERT(numberOfSamplesTotal >= numberOfSamplesRemaining);
    return samples2frames(numberOfSamplesTotal - numberOfSamplesRemaining);
}

QString SoundSourceProviderM4A::getName() const {
    return "Nero FAAD2";
}

QStringList SoundSourceProviderM4A::getSupportedFileExtensions() const {
    QStringList supportedFileExtensions;
    supportedFileExtensions.append("m4a");
    supportedFileExtensions.append("mp4");
    return supportedFileExtensions;
}

SoundSourcePointer SoundSourceProviderM4A::newSoundSource(const QUrl& url) {
    return exportSoundSourcePlugin(new SoundSourceM4A(url));
}

} // namespace Mixxx

namespace {

void deleteSoundSourceProviderSingleton(Mixxx::SoundSourceProvider*) {
    // The statically allocated instance must not be deleted!
}

} // anonymous namespace

extern "C" MIXXX_SOUNDSOURCEPLUGINAPI_EXPORT
Mixxx::SoundSourceProviderPointer Mixxx_SoundSourcePluginAPI_getSoundSourceProvider() {
    // SoundSourceProviderM4A is stateless and a single instance
    // can safely be shared
    static Mixxx::SoundSourceProviderM4A singleton;
    return Mixxx::SoundSourceProviderPointer(
            &singleton,
            deleteSoundSourceProviderSingleton);
}
