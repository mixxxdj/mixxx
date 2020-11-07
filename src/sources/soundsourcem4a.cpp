#include "sources/soundsourcem4a.h"

#include "util/logger.h"
#include "util/sample.h"

#ifdef __WINDOWS__
#include <fcntl.h>
#include <io.h>
#endif

#ifdef _MSC_VER
#define S_ISDIR(mode) (mode & _S_IFDIR)
#define strcasecmp stricmp
#endif

typedef unsigned long SAMPLERATE_TYPE;

namespace mixxx {

namespace {

const Logger kLogger("SoundSourceM4A");

// MP4SampleId is 1-based
constexpr MP4SampleId kSampleBlockIdMin = 1;

// Decoding will be restarted one or more blocks of samples
// before the actual position after seeking randomly in the
// audio stream to avoid audible glitches.
//
// "AAC Audio - Encoder Delay and Synchronization: The 2112 Sample Assumption"
// https://developer.apple.com/library/ios/technotes/tn2258/_index.html
// "It must also be assumed that without an explicit value, the playback
// system will trim 2112 samples from the AAC decoder output when starting
// playback from any point in the bitstream."
constexpr SINT kNumberOfPrefetchFrames = 2112;

// The TrackId is a 1-based index of the tracks in an MP4 file
constexpr u_int32_t kMinTrackId = 1;

// http://www.iis.fraunhofer.de/content/dam/iis/de/doc/ame/wp/FraunhoferIIS_Application-Bulletin_AAC-Transport-Formats.pdf
// Footnote 13: "The usual frame length for AAC-LC is 1024 samples, but a 960 sample version
// is used for radio broadcasting, and 480 or 512 sample versions are used for the low-delay
// codecs AAC-LD and AAC-ELD."
constexpr MP4Duration kDefaultFramesPerSampleBlock = 1024;

// According to various references DecoderConfigDescriptor.bufferSizeDB
// is a 24-bit unsigned integer value.
// MP4 atom:
//   trak.mdia.minf.stbl.stsd.*.esds.decConfigDescr.bufferSizeDB
// References:
//   https://github.com/sannies/mp4parser/blob/master/isoparser/src/main/java/org/mp4parser/boxes/iso14496/part1/objectdescriptors/DecoderConfigDescriptor.java
//   http://mutagen-specs.readthedocs.io/en/latest/mp4/
//   http://perso.telecom-paristech.fr/~dufourd/mpeg-4/tools.html
constexpr u_int32_t kMaxSampleBlockInputSizeLimit = (u_int32_t(1) << 24) - 1;

inline u_int32_t getMaxTrackId(MP4FileHandle hFile) {
    // The maximum TrackId equals the number of all tracks
    // in an MP4 file. We pass nullptr and 0 as arguments
    // to avoid any type/subtype filtering at this point!
    // Otherwise the previous assumption would no longer
    // be valid!
    return MP4GetNumberOfTracks(hFile, nullptr, 0);
}

inline bool isValidTrackType(const char* trackType) {
    return (nullptr != trackType) &&
            MP4_IS_AUDIO_TRACK_TYPE(trackType);
}

inline bool isValidMediaDataName(const char* mediaDataName) {
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

// Either 7 (without CRC) or 9 (with CRC) bytes
// https://wiki.multimedia.cx/index.php/ADTS
constexpr size_t kMinADTSHeaderLength = 7;

size_t getADTSHeaderLength(
        const u_int8_t* pInputBuffer,
        size_t sizeofInputBuffer) {
    // The size of the raw data block must be strictly greater than
    // the size of only ADTS header alone, i.e. additional sample
    // data must be present.
    if (sizeofInputBuffer > kMinADTSHeaderLength &&
            // ADTS header starts with syncword 0xFFF + 0-bit (Layer) + 0-bit (MPEG4)
            pInputBuffer[0] == 0xff &&
            (pInputBuffer[1] & 0xf6) == 0xf0) {
        const auto numberOfAacFramesMinusOne = pInputBuffer[6] & 0x03;
        VERIFY_OR_DEBUG_ASSERT(numberOfAacFramesMinusOne == 0) {
            // See also: https://wiki.multimedia.cx/index.php/ADTS
            kLogger.warning()
                    << "Multiple AAC frames (RDBs) per ADTS "
                       "frame are not supported";
        }
        // 2 bytes for CRC are optional
        size_t actualADTSHeaderLength = kMinADTSHeaderLength + (pInputBuffer[1] & 0x01) ? 0 : 2;
        if (sizeofInputBuffer > actualADTSHeaderLength) {
            return actualADTSHeaderLength;
        }
    }
    // No ADTS header found
    return 0;
}

inline bool startsWithADTSHeader(
        const u_int8_t* pInputBuffer,
        size_t sizeofInputBuffer) {
    return 0 < getADTSHeaderLength(pInputBuffer, sizeofInputBuffer);
}

} // anonymous namespace

//static
const QString SoundSourceProviderM4A::kDisplayName = QStringLiteral("Nero FAAD2");

//static
const QStringList SoundSourceProviderM4A::kSupportedFileExtensions = {
        QStringLiteral("m4a"),
        QStringLiteral("mp4"),
};

QStringList SoundSourceProviderM4A::getSupportedFileExtensions() const {
    if (faad2::LibLoader::Instance()->isLoaded()) {
        return kSupportedFileExtensions;
    } else {
        return QStringList(); // none available
    }
}

SoundSourcePointer SoundSourceProviderM4A::newSoundSource(const QUrl& url) {
    return newSoundSourceFromUrl<SoundSourceM4A>(url);
}

SoundSourceM4A::SoundSourceM4A(const QUrl& url)
        : SoundSource(url),
          m_pFaad(faad2::LibLoader::Instance()),
          m_hFile(MP4_INVALID_FILE_HANDLE),
          m_trackId(MP4_INVALID_TRACK_ID),
          m_framesPerSampleBlock(MP4_INVALID_DURATION),
          m_maxSampleBlockId(MP4_INVALID_SAMPLE_ID),
          m_inputBufferLength(0),
          m_inputBufferOffset(0),
          m_hDecoder(nullptr),
          m_numberOfPrefetchSampleBlocks(0),
          m_curSampleBlockId(MP4_INVALID_SAMPLE_ID),
          m_curFrameIndex(0) {
}

SoundSourceM4A::~SoundSourceM4A() {
    close();
}

SoundSource::OpenResult SoundSourceM4A::tryOpen(
        OpenMode mode,
        const OpenParams& params) {
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
        return OpenResult::Failed;
    }

    m_trackId = findFirstAudioTrackId(m_hFile, getLocalFileName());
    if (MP4_INVALID_TRACK_ID == m_trackId) {
        kLogger.warning() << "No AAC track found:" << getUrlString();
        return OpenResult::Aborted;
    }

    // Read fixed sample duration.  If the sample duration is not
    // fixed (that is, if the number of frames per sample block varies
    // through the file), the call returns MP4_INVALID_DURATION. We
    // can't currently handle these.
    m_framesPerSampleBlock = MP4GetTrackFixedSampleDuration(m_hFile, m_trackId);
    if (MP4_INVALID_DURATION == m_framesPerSampleBlock) {
        kLogger.warning() << "Unable to determine the fixed sample duration of track"
                          << m_trackId << "in file" << getUrlString();
        if (mode == OpenMode::Strict) {
            // Abort and give another decoder with lower priority
            // the chance to open the same file.
            // Fixes https://bugs.launchpad.net/mixxx/+bug/1504113
            return OpenResult::Aborted;
        } else {
            // Fallback: Use a default value
            kLogger.warning() << "Fallback: Using a default sample duration of"
                              << kDefaultFramesPerSampleBlock << "sample frames per block";
            m_framesPerSampleBlock = kDefaultFramesPerSampleBlock;
        }
    }

    const MP4SampleId numberOfSamples =
            MP4GetTrackNumberOfSamples(m_hFile, m_trackId);
    if (0 >= numberOfSamples) {
        kLogger.warning() << "Failed to read number of samples from file:" << getUrlString();
        return OpenResult::Failed;
    }
    m_maxSampleBlockId = kSampleBlockIdMin + (numberOfSamples - 1);

    // Determine the maximum input size (in bytes) of a
    // sample block for the selected track.
    const u_int32_t maxSampleBlockInputSize = MP4GetTrackMaxSampleSize(m_hFile,
            m_trackId);
    if (maxSampleBlockInputSize == 0) {
        kLogger.warning() << "Failed to read MP4 DecoderConfigDescriptor.bufferSizeDB:"
                          << getUrlString();
        return OpenResult::Failed;
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
        return OpenResult::Aborted;
    }
    m_inputBuffer.resize(maxSampleBlockInputSize, 0);

    m_openParams = params;

    if (openDecoder()) {
        return OpenResult::Succeeded;
    } else {
        return OpenResult::Failed;
    }
}

bool SoundSourceM4A::openDecoder() {
    DEBUG_ASSERT(m_hDecoder == nullptr); // not already opened

    DEBUG_ASSERT(!m_pMP4ESConfigBuffer);
    m_sizeofMP4ESConfigBuffer = 0;
    if (!MP4GetTrackESConfiguration(m_hFile,
                m_trackId,
                &m_pMP4ESConfigBuffer,
                &m_sizeofMP4ESConfigBuffer)) {
        // Failed to get mpeg-4 audio config... this is ok.
        // Init2() will then simply use default values instead.
        kLogger.info() << "Failed to read the MP4 elementary stream "
                          "(ES) configuration";
        DEBUG_ASSERT(!m_pMP4ESConfigBuffer);
        DEBUG_ASSERT(m_sizeofMP4ESConfigBuffer == 0);
    }

    if (!reopenDecoder()) {
        return false;
    }
    DEBUG_ASSERT(m_hDecoder);

    // Calculate how many sample blocks we need to decode in advance
    // of a random seek in order to get the recommended number of
    // prefetch frames
    m_numberOfPrefetchSampleBlocks =
            (kNumberOfPrefetchFrames + (m_framesPerSampleBlock - 1)) /
            m_framesPerSampleBlock;

    const SINT sampleBufferCapacity =
            getSignalInfo().frames2samples(m_framesPerSampleBlock);
    if (m_sampleBuffer.capacity() < sampleBufferCapacity) {
        m_sampleBuffer.adjustCapacity(sampleBufferCapacity);
    }

    // Discard all buffered input data
    m_curSampleBlockId = MP4_INVALID_SAMPLE_ID;
    m_inputBufferLength = 0;
    m_inputBufferOffset = 0;

    // Invalidate current stream position
    m_curFrameIndex = frameIndexMax();

    return true;
}

bool SoundSourceM4A::reopenDecoder() {
    auto hNewDecoder = m_pFaad->Open();
    if (!hNewDecoder) {
        kLogger.warning() << "Failed to open the AAC decoder";
        return false;
    }

    if (!replaceDecoder(hNewDecoder)) {
        return false;
    }
    DEBUG_ASSERT(m_hDecoder == hNewDecoder);

    return true;
}

bool SoundSourceM4A::replaceDecoder(
        faad2::DecoderHandle hNewDecoder) {
    const faad2::Configuration* pOldConfig = nullptr;
    if (m_hDecoder) {
        pOldConfig = m_pFaad->GetCurrentConfiguration(m_hDecoder);
        if (!pOldConfig) {
            kLogger.warning()
                    << "Failed to get the current (old) AAC decoder configuration";
            return false;
        }
        if (kLogger.traceEnabled()) {
            kLogger.trace()
                    << "Old AAC decoder configuration"
                    << *pOldConfig;
        }
    }

    faad2::Configuration* pNewConfig =
            m_pFaad->GetCurrentConfiguration(hNewDecoder);
    if (!pNewConfig) {
        kLogger.warning()
                << "Failed to get the current (new) AAC decoder configuration";
        return false;
    }
    if (kLogger.traceEnabled()) {
        kLogger.trace()
                << "Current AAC decoder configuration"
                << *pNewConfig;
    }

    if (pOldConfig) {
        if (pOldConfig->defSampleRate > 0) {
            pNewConfig->defSampleRate = pOldConfig->defSampleRate;
        }
        pNewConfig->defObjectType = pOldConfig->defObjectType;
        pNewConfig->outputFormat = pOldConfig->outputFormat;
        pNewConfig->downMatrix = pOldConfig->downMatrix;
    } else {
        pNewConfig->outputFormat = faad2::FMT_FLOAT;
        const auto desiredChannelCount =
                getSignalInfo().getChannelCount().isValid()
                ? getSignalInfo().getChannelCount()
                : m_openParams.getSignalInfo().getChannelCount();
        if (desiredChannelCount == 1 || desiredChannelCount == 2) {
            pNewConfig->downMatrix = 1;
        } else {
            pNewConfig->downMatrix = 0;
        }
    }

    if (kLogger.traceEnabled()) {
        kLogger.trace()
                << "Desired AAC decoder configuration"
                << *pNewConfig;
    }
    if (!m_pFaad->SetConfiguration(hNewDecoder, pNewConfig)) {
        kLogger.warning()
                << "Failed to configure AAC decoder"
                << *pNewConfig;
        return false;
    }

    SAMPLERATE_TYPE sampleRate;
    unsigned char channelCount;
    if (startsWithADTSHeader(m_inputBuffer.data(), m_inputBufferLength)) {
        DEBUG_ASSERT(m_hDecoder);
        kLogger.debug()
                << "Reinitializing decoder from AAC stream";
        if (m_pFaad->Init(hNewDecoder,
                    m_inputBuffer.data(),
                    m_inputBufferLength,
                    &sampleRate,
                    &channelCount) < 0) {
            kLogger.warning() << "Failed to initialize the AAC decoder from "
                                 "AAC stream (ADTS/ADIF)";
            return false;
        }
    } else {
        if (static_cast<signed char>(m_pFaad->Init2(
                    hNewDecoder,
                    m_pMP4ESConfigBuffer,
                    m_sizeofMP4ESConfigBuffer,
                    &sampleRate,
                    &channelCount)) < 0) {
            free(m_pMP4ESConfigBuffer);
            m_pMP4ESConfigBuffer = nullptr;
            kLogger.warning() << "Failed to initialize the AAC decoder from "
                                 "MP4 elementary stream (ES) configuration";
            return false;
        }
    }
    if (!initChannelCountOnce(channelCount)) {
        return false;
    }
    if (!initSampleRateOnce(sampleRate)) {
        return false;
    }
    if (!initFrameIndexRangeOnce(mixxx::IndexRange::forward(0,
                ((m_maxSampleBlockId - kSampleBlockIdMin) + 1) *
                        m_framesPerSampleBlock))) {
        return false;
    }

    if (m_hDecoder) {
        m_pFaad->Close(m_hDecoder);
    }
    m_hDecoder = hNewDecoder;

    return true;
}

void SoundSourceM4A::closeDecoder() {
    if (!m_hDecoder) {
        return;
    }
    m_pFaad->Close(m_hDecoder);
    m_hDecoder = nullptr;
}

void SoundSourceM4A::close() {
    closeDecoder();
    m_sampleBuffer.clear();
    m_inputBuffer.clear();
    if (m_pMP4ESConfigBuffer) {
        free(m_pMP4ESConfigBuffer);
        m_pMP4ESConfigBuffer = nullptr;
    }
    if (MP4_INVALID_FILE_HANDLE != m_hFile) {
        MP4Close(m_hFile);
        m_hFile = MP4_INVALID_FILE_HANDLE;
    }
}

bool SoundSourceM4A::isValidSampleBlockId(MP4SampleId sampleBlockId) const {
    return (sampleBlockId >= kSampleBlockIdMin) &&
            (sampleBlockId <= m_maxSampleBlockId);
}

void SoundSourceM4A::restartDecoding(MP4SampleId sampleBlockId) {
    DEBUG_ASSERT(sampleBlockId >= kSampleBlockIdMin);

    m_pFaad->PostSeekReset(m_hDecoder, sampleBlockId);
    m_curSampleBlockId = sampleBlockId;
    m_curFrameIndex = frameIndexMin() +
            (sampleBlockId - kSampleBlockIdMin) * m_framesPerSampleBlock;

    // Discard input buffer
    m_inputBufferLength = 0;
    m_inputBufferOffset = 0;

    // Discard previously decoded sample data
    m_sampleBuffer.clear();
}

ReadableSampleFrames SoundSourceM4A::readSampleFramesClamped(
        WritableSampleFrames writableSampleFrames) {
    const SINT firstFrameIndex = writableSampleFrames.frameIndexRange().start();

    bool retryAfterReopeningDecoder = false;
    do {
        while (m_curFrameIndex != firstFrameIndex) {
            // NOTE(uklotzde): Resetting the decoder near to the beginning
            // of the stream when seeking backwards produces invalid sample
            // values! As a consequence the seeking test fails.
            if ((m_curSampleBlockId != MP4_INVALID_SAMPLE_ID) &&
                    (firstFrameIndex < m_curFrameIndex) &&
                    (firstFrameIndex <=
                            (frameIndexMin() + kNumberOfPrefetchFrames))) {
                // Workaround: Discard remaining input data and reopen the decoder when
                // seeking near to the beginning of the stream while decoding.
                m_curSampleBlockId = MP4_INVALID_SAMPLE_ID;
                m_inputBufferLength = 0;
                if (!reopenDecoder()) {
                    return {};
                }
            }

            MP4SampleId sampleBlockId = kSampleBlockIdMin +
                    (firstFrameIndex / m_framesPerSampleBlock);
            DEBUG_ASSERT(isValidSampleBlockId(sampleBlockId));
            if ((firstFrameIndex < m_curFrameIndex) || // seeking backwards?
                    !isValidSampleBlockId(
                            m_curSampleBlockId) || // invalid seek position?
                    (sampleBlockId >
                            (m_curSampleBlockId +
                                    m_numberOfPrefetchSampleBlocks))) { // jumping forward?
                // Restart decoding one or more blocks of samples backwards
                // from the calculated starting block to avoid audible glitches.
                // Implementation note: The type MP4SampleId is unsigned so we
                // need to be careful when subtracting!
                if ((kSampleBlockIdMin + m_numberOfPrefetchSampleBlocks) <
                        sampleBlockId) {
                    sampleBlockId -= m_numberOfPrefetchSampleBlocks;
                } else {
                    sampleBlockId = kSampleBlockIdMin;
                }
                restartDecoding(sampleBlockId);
                DEBUG_ASSERT(m_curSampleBlockId == sampleBlockId);
            }

            // Decoding starts before the actual target position
            DEBUG_ASSERT(m_curFrameIndex <= firstFrameIndex);
            const auto precedingFrames =
                    IndexRange::between(m_curFrameIndex, firstFrameIndex);
            if (!precedingFrames.empty() &&
                    (precedingFrames !=
                            readSampleFramesClamped(
                                    WritableSampleFrames(precedingFrames))
                                    .frameIndexRange())) {
                kLogger.warning()
                        << "Failed to skip preceding frames" << precedingFrames;
                // Abort
                return ReadableSampleFrames(
                        IndexRange::between(m_curFrameIndex, m_curFrameIndex));
            }
        }
        DEBUG_ASSERT(m_curFrameIndex == firstFrameIndex);

        const SINT numberOfSamplesTotal = getSignalInfo().frames2samples(
                writableSampleFrames.frameLength());

        SINT numberOfSamplesRemaining = numberOfSamplesTotal;
        SINT outputSampleOffset = 0;
        while (0 < numberOfSamplesRemaining) {
            if (!m_sampleBuffer.empty()) {
                // Consume previously decoded sample data
                const SampleBuffer::ReadableSlice readableSlice(
                        m_sampleBuffer.shrinkForReading(
                                numberOfSamplesRemaining));
                if (writableSampleFrames.writableData()) {
                    SampleUtil::copy(writableSampleFrames.writableData(
                                             outputSampleOffset),
                            readableSlice.data(),
                            readableSlice.length());
                    outputSampleOffset += readableSlice.length();
                }
                m_curFrameIndex +=
                        getSignalInfo().samples2frames(readableSlice.length());
                DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
                DEBUG_ASSERT(
                        numberOfSamplesRemaining >= readableSlice.length());
                numberOfSamplesRemaining -= readableSlice.length();
                if (0 == numberOfSamplesRemaining) {
                    break; // exit loop
                }
            }
            // All previously decoded sample data has been consumed now
            DEBUG_ASSERT(m_sampleBuffer.empty());

            DEBUG_ASSERT(m_inputBufferLength >= m_inputBufferOffset);
            if (m_inputBufferLength <= m_inputBufferOffset) {
                // Fill input buffer from file
                if (isValidSampleBlockId(m_curSampleBlockId)) {
                    // Read data for next sample block into input buffer
                    u_int8_t* pInputBuffer = m_inputBuffer.data();
                    u_int32_t inputBufferLength = m_inputBuffer.size(); // in/out parameter
                    if (!MP4ReadSample(m_hFile,
                                m_trackId,
                                m_curSampleBlockId,
                                &pInputBuffer,
                                &inputBufferLength,
                                nullptr,
                                nullptr,
                                nullptr,
                                nullptr)) {
                        kLogger.warning()
                                << "Failed to read MP4 input data for sample "
                                   "block"
                                << m_curSampleBlockId << "("
                                << "min =" << kSampleBlockIdMin << ","
                                << "max =" << m_maxSampleBlockId << ")";
                        break; // abort
                    }
                    DEBUG_ASSERT(pInputBuffer == m_inputBuffer.data());
                    DEBUG_ASSERT(inputBufferLength <= m_inputBuffer.size());
                    ++m_curSampleBlockId;
                    m_inputBufferLength = inputBufferLength;
                    m_inputBufferOffset = 0;
                    // Skip ADTS header if we have a decoder specific ES config
                    if (m_pMP4ESConfigBuffer &&
                            startsWithADTSHeader(m_inputBuffer.data(), m_inputBufferLength)) {
                        m_inputBufferOffset +=
                                getADTSHeaderLength(m_inputBuffer.data(), m_inputBufferLength);
                    }
                }
            }
            DEBUG_ASSERT(m_inputBufferLength >= m_inputBufferOffset);
            if (m_inputBufferLength <= m_inputBufferOffset) {
                break; // EOF
            }

            // NOTE(uklotzde): The sample buffer for Decode2 has to
            // be big enough for a whole block of decoded samples, which
            // contains up to m_framesPerSampleBlock frames. Otherwise
            // we need to use a temporary buffer.
            CSAMPLE* pDecodeBuffer; // in/out parameter
            SINT decodeBufferCapacity;
            const SINT decodeBufferCapacityMin =
                    getSignalInfo().frames2samples(m_framesPerSampleBlock);
            if (writableSampleFrames.writableData() &&
                    (decodeBufferCapacityMin <= numberOfSamplesRemaining)) {
                // Decode samples directly into the output buffer
                pDecodeBuffer =
                        writableSampleFrames.writableData(outputSampleOffset);
                decodeBufferCapacity = numberOfSamplesRemaining;
            } else {
                // Decode next sample block into temporary buffer
                const SINT maxWriteLength = math_max(
                        numberOfSamplesRemaining, decodeBufferCapacityMin);
                const SampleBuffer::WritableSlice writableSlice(
                        m_sampleBuffer.growForWriting(maxWriteLength));
                pDecodeBuffer = writableSlice.data();
                decodeBufferCapacity = writableSlice.length();
            }
            DEBUG_ASSERT(decodeBufferCapacityMin <= decodeBufferCapacity);

            faad2::FrameInfo decFrameInfo;
            DEBUG_ASSERT(m_inputBufferLength >= m_inputBufferOffset);
            void* pDecodeResult = m_pFaad->Decode2(m_hDecoder,
                    &decFrameInfo,
                    &m_inputBuffer[m_inputBufferOffset],
                    m_inputBufferLength - m_inputBufferOffset,
                    reinterpret_cast<void**>(&pDecodeBuffer),
                    decodeBufferCapacity * sizeof(*pDecodeBuffer));
            if (decFrameInfo.error != 0) {
                // A decoding error has occurred
                if (retryAfterReopeningDecoder) {
                    // At this point we have failed to decode the current sample
                    // block twice and need to discard it. The content of the
                    // sample block is unknown and we simply continue with the
                    // next block. This is just a workaround! The reason why FAAD2
                    // v2.9.2 rejects these blocks is unknown.
                    kLogger.warning()
                            << "Skipping block"
                            << m_curSampleBlockId
                            << "of length"
                            << m_inputBufferLength
                            << "after an AAC decoding error occurred";
                    // Reset the retry flag before continuing with the next block
                    retryAfterReopeningDecoder = false;
                    m_inputBufferLength = 0;
                    m_inputBufferOffset = 0;
                    continue;
                } else {
                    const auto frameError = faad2::FrameError(decFrameInfo.error);
                    if (frameError == faad2::FrameError::InvalidNumberOfChannels ||
                            frameError == faad2::FrameError::InvalidChannelConfiguration) {
                        kLogger.debug()
                                << "Reopening decoder after AAC decoding error"
                                << decFrameInfo.error
                                << m_pFaad->GetErrorMessage(decFrameInfo.error)
                                << getUrlString();
                        // Assumption: All samples from the preceding blocks have been
                        // decoded and consumed before decoding continues with the new,
                        // reopened decoder. Otherwise the decoded stream of samples
                        // might be discontinuous, but we can't do anything about it.
                        retryAfterReopeningDecoder = reopenDecoder();
                        // If reopening the decoder failed retrying the same sample
                        // block with the same decoder instance will fail again. In
                        // this case we will simply abort the decoding of the stream
                        // immediately, see below.
                    }
                }
                if (!retryAfterReopeningDecoder) {
                    // A decoding error occurred and no retry is pending
                    kLogger.warning()
                            << "AAC decoding error:" << decFrameInfo.error
                            << m_pFaad->GetErrorMessage(decFrameInfo.error)
                            << getUrlString();
                    // In turn the decoding will be aborted
                }
                // Either abort or retry by exiting the inner loop
                break;
            } else {
                // Reset the retry flag after succesfully decoding a block
                retryAfterReopeningDecoder = false;
            }
            // Upon a pending retry the inner loop is exited immediately and
            // we must never get to this point.
            DEBUG_ASSERT(!retryAfterReopeningDecoder);

            Q_UNUSED(pDecodeResult); // only used in DEBUG_ASSERT
            DEBUG_ASSERT(pDecodeResult ==
                    pDecodeBuffer); // verify the in/out parameter

            // Verify the decoded sample data for consistency
            VERIFY_OR_DEBUG_ASSERT(getSignalInfo().getChannelCount() ==
                    decFrameInfo.channels) {
                kLogger.critical() << "Corrupt or unsupported AAC file:"
                                   << "Unexpected number of channels"
                                   << decFrameInfo.channels << "<>"
                                   << getSignalInfo().getChannelCount();
                break; // abort
            }
            VERIFY_OR_DEBUG_ASSERT(getSignalInfo().getSampleRate() ==
                    SINT(decFrameInfo.samplerate)) {
                kLogger.critical()
                        << "Corrupt or unsupported AAC file:"
                        << "Unexpected sample rate" << decFrameInfo.samplerate
                        << "<>" << getSignalInfo().getSampleRate();
                break; // abort
            }

            // Consume input data
            m_inputBufferOffset += decFrameInfo.bytesconsumed;

            // Consume decoded output data
            const SINT numberOfSamplesDecoded = decFrameInfo.samples;
            DEBUG_ASSERT(numberOfSamplesDecoded <= decodeBufferCapacity);
            SINT numberOfSamplesRead;
            if (writableSampleFrames.writableData() &&
                    (pDecodeBuffer == writableSampleFrames.writableData(outputSampleOffset))) {
                // Decoded in-place
                DEBUG_ASSERT(numberOfSamplesDecoded <= numberOfSamplesRemaining);
                numberOfSamplesRead = numberOfSamplesDecoded;
                outputSampleOffset += numberOfSamplesRead;
            } else {
                // Decoded into temporary buffer
                DEBUG_ASSERT(numberOfSamplesDecoded <= decodeBufferCapacity);
                // Shrink the size of the buffer to the samples that have
                // actually been decoded, i.e. dropping unneeded samples
                // from the back of the buffer.
                m_sampleBuffer.shrinkAfterWriting(decodeBufferCapacity - numberOfSamplesDecoded);
                DEBUG_ASSERT(m_sampleBuffer.readableLength() == numberOfSamplesDecoded);
                // Read from the buffer's head
                numberOfSamplesRead =
                        std::min(numberOfSamplesDecoded, numberOfSamplesRemaining);
                const SampleBuffer::ReadableSlice readableSlice(
                        m_sampleBuffer.shrinkForReading(numberOfSamplesRead));
                DEBUG_ASSERT(readableSlice.length() == numberOfSamplesRead);
                if (writableSampleFrames.writableData()) {
                    SampleUtil::copy(
                            writableSampleFrames.writableData(outputSampleOffset),
                            readableSlice.data(),
                            readableSlice.length());
                    outputSampleOffset += numberOfSamplesRead;
                }
            }
            // The decoder might decode more samples than actually needed
            // at the end of the file! When the end of the file has been
            // reached decoding can be restarted by seeking to a new
            // position.
            m_curFrameIndex += getSignalInfo().samples2frames(numberOfSamplesRead);
            numberOfSamplesRemaining -= numberOfSamplesRead;
        }
        DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
        if (retryAfterReopeningDecoder) {
            // Continue by retrying to decode the current sample block again
            // with a new decoder instance after errors occurred.
            continue;
        }
        // The current sample block has been decoded successfully
        DEBUG_ASSERT(numberOfSamplesTotal >= numberOfSamplesRemaining);
        const SINT numberOfSamples = numberOfSamplesTotal - numberOfSamplesRemaining;
        return ReadableSampleFrames(
                IndexRange::forward(
                        firstFrameIndex,
                        getSignalInfo().samples2frames(numberOfSamples)),
                SampleBuffer::ReadableSlice(
                        writableSampleFrames.writableData(),
                        std::min(writableSampleFrames.writableLength(), numberOfSamples)));
    } while (retryAfterReopeningDecoder);
    DEBUG_ASSERT(!"unreachable");
    return {};
}

} // namespace mixxx
