#include "soundsourcem4a.h"

#include "util/sample.h"
#include "util/logger.h"

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

namespace mixxx {

namespace {

const Logger kLogger("SoundSourceM4A");

// MP4SampleId is 1-based
const MP4SampleId kSampleBlockIdMin = 1;

// Decoding will be restarted one or more blocks of samples
// before the actual position after seeking randomly in the
// audio stream to avoid audible glitches.
//
// "AAC Audio - Encoder Delay and Synchronization: The 2112 Sample Assumption"
// https://developer.apple.com/library/ios/technotes/tn2258/_index.html
// "It must also be assumed that without an explicit value, the playback
// system will trim 2112 samples from the AAC decoder output when starting
// playback from any point in the bistream."
const SINT kNumberOfPrefetchFrames = 2112;

// The TrackId is a 1-based index of the tracks in an MP4 file
const u_int32_t kMinTrackId = 1;

// http://www.iis.fraunhofer.de/content/dam/iis/de/doc/ame/wp/FraunhoferIIS_Application-Bulletin_AAC-Transport-Formats.pdf
// Footnote 13: "The usual frame length for AAC-LC is 1024 samples, but a 960 sample version
// is used for radio broadcasting, and 480 or 512 sample versions are used for the low-delay
// codecs AAC-LD and AAC-ELD."
const MP4Duration kDefaultFramesPerSampleBlock = 1024;

// According to various references DecoderConfigDescriptor.bufferSizeDB
// is a 24-bit unsigned integer value.
// MP4 atom:
//   trak.mdia.minf.stbl.stsd.*.esds.decConfigDescr.bufferSizeDB
// References:
//   https://github.com/sannies/mp4parser/blob/master/isoparser/src/main/java/org/mp4parser/boxes/iso14496/part1/objectdescriptors/DecoderConfigDescriptor.java
//   http://mutagen-specs.readthedocs.io/en/latest/mp4/
//   http://perso.telecom-paristech.fr/~dufourd/mpeg-4/tools.html
const u_int32_t kMaxSampleBlockInputSizeLimit = (u_int32_t(1) << 24) - 1;

inline
u_int32_t getMaxTrackId(MP4FileHandle hFile) {
    // The maximum TrackId equals the number of all tracks
    // in an MP4 file. We pass nullptr and 0 as arguments
    // to avoid any type/subtype filtering at this point!
    // Otherwise the previous assumption would no longer
    // be valid!
    return MP4GetNumberOfTracks(hFile, nullptr, 0);
}

inline
bool isValidTrackType(const char* trackType) {
    return (nullptr != trackType) &&
            MP4_IS_AUDIO_TRACK_TYPE(trackType);
}

inline
bool isValidMediaDataName(const char* mediaDataName) {
    return (nullptr != mediaDataName) &&
            (0 == strcasecmp(mediaDataName, "mp4a"));
}

// Searches for the first audio track in the MP4 file that
// suits our needs.
MP4TrackId findFirstAudioTrackId(MP4FileHandle hFile, const QString& fileName) {
    const u_int32_t maxTrackId = getMaxTrackId(hFile);
    for (u_int32_t trackId = kMinTrackId; trackId <= maxTrackId; ++trackId) {
        const char* trackType = MP4GetTrackType(hFile, trackId);
        if (!isValidTrackType(trackType)) {
            kLogger.warning() << "Unsupported track type"
                    << QString((trackType == nullptr) ? "" : trackType);
            kLogger.warning() << "Skipping track"
                    << trackId
                    << "of"
                    << maxTrackId
                    << "in file"
                    << fileName;
            continue;
        }
        const char* mediaDataName = MP4GetTrackMediaDataName(hFile, trackId);
        if (!isValidMediaDataName(mediaDataName)) {
            kLogger.warning() << "Unsupported media data name"
                    << QString((mediaDataName == nullptr) ? "" : mediaDataName);
            kLogger.warning() << "Skipping track"
                    << trackId
                    << "of"
                    << maxTrackId
                    << "in file"
                    << fileName;
            continue;
        }
        const u_int8_t audioType = MP4GetTrackEsdsObjectTypeId(hFile, trackId);
        if (MP4_IS_AAC_AUDIO_TYPE(audioType)) {
            if (MP4_MPEG4_AUDIO_TYPE == audioType) {
                const u_int8_t mpeg4AudioType =
                        MP4GetTrackAudioMpeg4Type(hFile, trackId);
                if (MP4_IS_MPEG4_AAC_AUDIO_TYPE(mpeg4AudioType)) {
                    return trackId;
                } else {
                    kLogger.warning() << "Unsupported MPEG4 audio type"
                            << int(mpeg4AudioType);
                    kLogger.warning() << "Skipping track"
                            << trackId
                            << "of"
                            << maxTrackId
                            << "in file"
                            << fileName;
                    continue;
                }
            } else {
                return trackId;
            }
        } else {
            kLogger.warning() << "Unsupported audio type"
                    << int(audioType);
            kLogger.warning() << "Skipping track"
                    << trackId
                    << "of"
                    << maxTrackId
                    << "in file"
                    << fileName;
            continue;
        }
        VERIFY_OR_DEBUG_ASSERT(!"unreachable code") {
            kLogger.warning() << "Skipping track"
                    << trackId
                    << "of"
                    << maxTrackId
                    << "in file"
                    << fileName;
        }
    }
    return MP4_INVALID_TRACK_ID;
}

} // anonymous namespace

SoundSourceM4A::SoundSourceM4A(const QUrl& url)
        : SoundSourcePlugin(url, "m4a"),
          m_hFile(MP4_INVALID_FILE_HANDLE),
          m_trackId(MP4_INVALID_TRACK_ID),
          m_framesPerSampleBlock(MP4_INVALID_DURATION),
          m_maxSampleBlockId(MP4_INVALID_SAMPLE_ID),
          m_inputBufferLength(0),
          m_inputBufferOffset(0),
          m_hDecoder(nullptr),
          m_numberOfPrefetchSampleBlocks(0),
          m_curSampleBlockId(MP4_INVALID_SAMPLE_ID),
          m_curFrameIndex(getMinFrameIndex()) {
}

SoundSourceM4A::~SoundSourceM4A() {
    close();
}

SoundSource::OpenResult SoundSourceM4A::tryOpen(const AudioSourceConfig& audioSrcCfg) {
    DEBUG_ASSERT(MP4_INVALID_FILE_HANDLE == m_hFile);
    // open MP4 file, check for >= ver 1.9.1
    // From mp4v2/file.h:
    //  * On Windows, this should be a UTF-8 encoded string.
    //  * On other platforms, it should be an 8-bit encoding that is
    //  * appropriate for the platform, locale, file system, etc.
    //  * (prefer to use UTF-8 when possible).
#if MP4V2_PROJECT_version_hex <= 0x00010901
    m_hFile = MP4Read(getLocalFileName().toUtf8().constData(), 0);
#else
    m_hFile = MP4Read(getLocalFileName().toUtf8().constData());
#endif
    if (MP4_INVALID_FILE_HANDLE == m_hFile) {
        kLogger.warning() << "Failed to open file for reading:" << getUrlString();
        return OpenResult::FAILED;
    }

    m_trackId = findFirstAudioTrackId(m_hFile, getLocalFileName());
    if (MP4_INVALID_TRACK_ID == m_trackId) {
        kLogger.warning() << "No AAC track found:" << getUrlString();
        return OpenResult::ABORTED;
    }

    // Read fixed sample duration.  If the sample duration is not
    // fixed (that is, if the number of frames per sample block varies
    // through the file), the call returns MP4_INVALID_DURATION. We
    // can't currently handle these.
    m_framesPerSampleBlock = MP4GetTrackFixedSampleDuration(m_hFile, m_trackId);
    if (MP4_INVALID_DURATION == m_framesPerSampleBlock) {
      kLogger.warning() << "Unable to determine the fixed sample duration of track"
              << m_trackId << "in file" << getUrlString();
      // TODO(XXX): The following check for FFmpeg or any another available
      // AAC decoder should be done at runtime and not at compile time.
      // Only if this is the last available decoder for handling the file
      // the default value should be used as a fallback and last resort.
      // Unfortunately our API currently does not provide this information
      // for making such a decision at runtime.
#ifdef __FFMPEGFILE__
      // Give up and instead let FFmpeg (or any other AAC decoder with
      // lower priority) handle this file.
      // Fixes https://bugs.launchpad.net/mixxx/+bug/1504113
      return OpenResult::ABORTED;
#else
      // Fallback: Use a default value if FFmpeg is not available (checked
      // at compile time).
      kLogger.warning() << "Fallback: Using a default sample duration of"
              << kDefaultFramesPerSampleBlock << "sample frames per block";
      m_framesPerSampleBlock = kDefaultFramesPerSampleBlock;
#endif // __FFMPEGFILE__
    }

    const MP4SampleId numberOfSamples =
            MP4GetTrackNumberOfSamples(m_hFile, m_trackId);
    if (0 >= numberOfSamples) {
        kLogger.warning() << "Failed to read number of samples from file:" << getUrlString();
        return OpenResult::FAILED;
    }
    m_maxSampleBlockId = kSampleBlockIdMin + (numberOfSamples - 1);

    // Determine the maximum input size (in bytes) of a
    // sample block for the selected track.
    const u_int32_t maxSampleBlockInputSize = MP4GetTrackMaxSampleSize(m_hFile,
            m_trackId);
    if (maxSampleBlockInputSize == 0) {
        kLogger.warning() << "Failed to read MP4 DecoderConfigDescriptor.bufferSizeDB:"
                << getUrlString();
        return OpenResult::FAILED;
    }
    if (maxSampleBlockInputSize > kMaxSampleBlockInputSizeLimit) {
        // Workaround for a possible bug in libmp4v2 2.0.0 (Ubuntu 16.04)
        // that returns 4278190742 when opening a corrupt file.
        // https://bugs.launchpad.net/mixxx/+bug/1594169
        kLogger.warning() << "MP4 DecoderConfigDescriptor.bufferSizeDB ="
                << maxSampleBlockInputSize
                << ">"
                << kMaxSampleBlockInputSizeLimit
                << "exceeds limit:"
                << getUrlString();
        return OpenResult::ABORTED;
    }
    m_inputBuffer.resize(maxSampleBlockInputSize, 0);

    m_audioSrcCfg = audioSrcCfg;

    if (openDecoder()) {
        return OpenResult::SUCCEEDED;
    } else {
        return OpenResult::FAILED;
    }
}

bool SoundSourceM4A::openDecoder() {
    DEBUG_ASSERT(m_hDecoder == nullptr); // not already opened

    m_hDecoder = NeAACDecOpen();
    if (m_hDecoder == nullptr) {
        kLogger.warning() << "Failed to open the AAC decoder!";
        return false;
    }
    NeAACDecConfigurationPtr pDecoderConfig = NeAACDecGetCurrentConfiguration(
            m_hDecoder);
    pDecoderConfig->outputFormat = FAAD_FMT_FLOAT;
    if ((kChannelCountMono == m_audioSrcCfg.getChannelCount()) ||
            (kChannelCountStereo == m_audioSrcCfg.getChannelCount())) {
        pDecoderConfig->downMatrix = 1;
    } else {
        pDecoderConfig->downMatrix = 0;
    }

    pDecoderConfig->defObjectType = LC;
    if (!NeAACDecSetConfiguration(m_hDecoder, pDecoderConfig)) {
        kLogger.warning() << "Failed to configure AAC decoder!";
        return false;
    }

    u_int8_t* configBuffer = nullptr;
    u_int32_t configBufferSize = 0;
    if (!MP4GetTrackESConfiguration(m_hFile, m_trackId, &configBuffer,
            &configBufferSize)) {
        // Failed to get mpeg-4 audio config... this is ok.
        // NeAACDecInit2() will simply use default values instead.
        kLogger.warning() << "Failed to read the MP4 audio configuration."
                << "Continuing with default values.";
    }

    SAMPLERATE_TYPE samplingRate;
    unsigned char channelCount;
    if (0 > NeAACDecInit2(m_hDecoder, configBuffer, configBufferSize,
                    &samplingRate, &channelCount)) {
        free(configBuffer);
        kLogger.warning() << "Failed to initialize the AAC decoder!";
        return false;
    } else {
        free(configBuffer);
    }

    // Calculate how many sample blocks we need to decode in advance
    // of a random seek in order to get the recommended number of
    // prefetch frames
    m_numberOfPrefetchSampleBlocks =
            (kNumberOfPrefetchFrames + (m_framesPerSampleBlock - 1)) /
            m_framesPerSampleBlock;

    setChannelCount(channelCount);
    setSamplingRate(samplingRate);
    setFrameCount(((m_maxSampleBlockId - kSampleBlockIdMin) + 1) * m_framesPerSampleBlock);

    // Resize temporary buffer for decoded sample data
    const SINT sampleBufferCapacity =
            frames2samples(m_framesPerSampleBlock);
    m_sampleBuffer.resetCapacity(sampleBufferCapacity);

    // Discard all buffered samples
    m_inputBufferLength = 0;

    // Invalidate current position(s) for the following seek operation
    m_curSampleBlockId = MP4_INVALID_SAMPLE_ID;
    m_curFrameIndex = getMaxFrameIndex();

    // (Re-)Start decoding at the beginning of the file
    seekSampleFrame(getMinFrameIndex());

    return m_curFrameIndex == getMinFrameIndex();
}

void SoundSourceM4A::closeDecoder() {
    if (m_hDecoder != nullptr) {
        NeAACDecClose(m_hDecoder);
        m_hDecoder = nullptr;
    }
}

bool SoundSourceM4A::reopenDecoder() {
    closeDecoder();
    return openDecoder();
}

void SoundSourceM4A::close() {
    closeDecoder();
    m_sampleBuffer.reset();
    m_inputBuffer.clear();
    if (MP4_INVALID_FILE_HANDLE != m_hFile) {
        MP4Close(m_hFile);
        m_hFile = MP4_INVALID_FILE_HANDLE;
    }
}

bool SoundSourceM4A::isValidSampleBlockId(MP4SampleId sampleBlockId) const {
    return (sampleBlockId >= kSampleBlockIdMin)
            && (sampleBlockId <= m_maxSampleBlockId);
}

void SoundSourceM4A::restartDecoding(MP4SampleId sampleBlockId) {
    DEBUG_ASSERT(sampleBlockId >= kSampleBlockIdMin);

    NeAACDecPostSeekReset(m_hDecoder, sampleBlockId);
    m_curSampleBlockId = sampleBlockId;
    m_curFrameIndex = AudioSource::getMinFrameIndex() +
            (sampleBlockId - kSampleBlockIdMin) * m_framesPerSampleBlock;

    // Discard input buffer
    m_inputBufferLength = 0;

    // Discard previously decoded sample data
    m_sampleBuffer.reset();
}

SINT SoundSourceM4A::seekSampleFrame(SINT frameIndex) {
    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));

    if (frameIndex >= getMaxFrameIndex()) {
        // EOF
        m_curFrameIndex = getMaxFrameIndex();
        return m_curFrameIndex;
    }

    // NOTE(uklotzde): Resetting the decoder near to the beginning
    // of the stream when seeking backwards produces invalid sample
    // values! As a consequence the seeking test fails.
    if (isValidSampleBlockId(m_curSampleBlockId) &&
            (frameIndex <= kNumberOfPrefetchFrames)) {
        // Workaround: Reset the decoder when seeking near to the beginning
        // of the stream while decoding.
        reopenDecoder();
        skipSampleFrames(frameIndex);
    }
    if (frameIndex == m_curFrameIndex) {
        return m_curFrameIndex;
    }

    MP4SampleId sampleBlockId = kSampleBlockIdMin
            + (frameIndex / m_framesPerSampleBlock);
    DEBUG_ASSERT(isValidSampleBlockId(sampleBlockId));
    if ((frameIndex < m_curFrameIndex) || // seeking backwards?
            !isValidSampleBlockId(m_curSampleBlockId) || // invalid seek position?
            (sampleBlockId
                    > (m_curSampleBlockId + m_numberOfPrefetchSampleBlocks))) { // jumping forward?
        // Restart decoding one or more blocks of samples backwards
        // from the calculated starting block to avoid audible glitches.
        // Implementation note: The type MP4SampleId is unsigned so we
        // need to be careful when subtracting!
        if ((kSampleBlockIdMin + m_numberOfPrefetchSampleBlocks)
                < sampleBlockId) {
            sampleBlockId -= m_numberOfPrefetchSampleBlocks;
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
        kLogger.warning() << "Failed to prefetch sample data while seeking"
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
                    kLogger.warning()
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
        // contains up to m_framesPerSampleBlock frames. Otherwise
        // we need to use a temporary buffer.
        CSAMPLE* pDecodeBuffer; // in/out parameter
        SINT decodeBufferCapacity;
        const SINT decodeBufferCapacityMin = frames2samples(m_framesPerSampleBlock);
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
            kLogger.warning() << "AAC decoding error:"
                    << decFrameInfo.error
                    << NeAACDecGetErrorMessage(decFrameInfo.error)
                    << getUrlString();
            break; // abort
        }
        DEBUG_ASSERT(pDecodeResult == pDecodeBuffer); // verify the in/out parameter

        // Verify the decoded sample data for consistency
        if (getChannelCount() != decFrameInfo.channels) {
            kLogger.warning() << "Corrupt or unsupported AAC file:"
                    << "Unexpected number of channels" << decFrameInfo.channels
                    << "<>" << getChannelCount();
            break; // abort
        }
        if (getSamplingRate() != SINT(decFrameInfo.samplerate)) {
            kLogger.warning() << "Corrupt or unsupported AAC file:"
                    << "Unexpected sampling rate" << decFrameInfo.samplerate
                    << "<>" << getSamplingRate();
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
    return newSoundSourcePluginFromUrl<SoundSourceM4A>(url);
}

} // namespace mixxx

extern "C" MIXXX_SOUNDSOURCEPLUGINAPI_EXPORT
mixxx::SoundSourceProvider* Mixxx_SoundSourcePluginAPI_createSoundSourceProvider() {
    // SoundSourceProviderM4A is stateless and a single instance
    // can safely be shared
    static mixxx::SoundSourceProviderM4A singleton;
    return &singleton;
}

extern "C" MIXXX_SOUNDSOURCEPLUGINAPI_EXPORT
void Mixxx_SoundSourcePluginAPI_destroySoundSourceProvider(mixxx::SoundSourceProvider*) {
    // The statically allocated instance must not be deleted!
}
