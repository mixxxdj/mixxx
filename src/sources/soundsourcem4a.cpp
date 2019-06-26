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
const MP4SampleId kSampleBlockIdMin = 1;

// Decoding will be restarted one or more blocks of samples
// before the actual position after seeking randomly in the
// audio stream to avoid audible glitches.
//
// "AAC Audio - Encoder Delay and Synchronization: The 2112 Sample Assumption"
// https://developer.apple.com/library/ios/technotes/tn2258/_index.html
// "It must also be assumed that without an explicit value, the playback
// system will trim 2112 samples from the AAC decoder output when starting
// playback from any point in the bitstream."
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

} // anonymous namespace

SoundSourceM4A::SoundSourceM4A(const QUrl& url)
        : SoundSource(url, "m4a"),
          m_hFile(MP4_INVALID_FILE_HANDLE),
          m_trackId(MP4_INVALID_TRACK_ID),
          m_framesPerSampleBlock(MP4_INVALID_DURATION),
          m_maxSampleBlockId(MP4_INVALID_SAMPLE_ID),
          m_inputBufferLength(0),
          m_inputBufferOffset(0),
          m_hDecoder(nullptr),
          m_numberOfPrefetchSampleBlocks(0),
          m_curSampleBlockId(MP4_INVALID_SAMPLE_ID),
          m_curFrameIndex(0),
          m_pFaad(LibFaadLoader::Instance()) {
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

    m_hDecoder = m_pFaad->Open();
    if (m_hDecoder == nullptr) {
        kLogger.warning() << "Failed to open the AAC decoder!";
        return false;
    }
    LibFaadLoader::Configuration* pDecoderConfig = m_pFaad->GetCurrentConfiguration(
            m_hDecoder);
    pDecoderConfig->outputFormat = FAAD_FMT_FLOAT;
    if ((m_openParams.channelCount() == 1) ||
            (m_openParams.channelCount() == 2)) {
        pDecoderConfig->downMatrix = 1;
    } else {
        pDecoderConfig->downMatrix = 0;
    }

    pDecoderConfig->defObjectType = LC;
    if (!m_pFaad->SetConfiguration(m_hDecoder, pDecoderConfig)) {
        kLogger.warning() << "Failed to configure AAC decoder!";
        return false;
    }

    u_int8_t* configBuffer = nullptr;
    u_int32_t configBufferSize = 0;
    if (!MP4GetTrackESConfiguration(m_hFile, m_trackId, &configBuffer, &configBufferSize)) {
        // Failed to get mpeg-4 audio config... this is ok.
        // Init2() will simply use default values instead.
        kLogger.warning() << "Failed to read the MP4 audio configuration."
                          << "Continuing with default values.";
    }

    SAMPLERATE_TYPE sampleRate;
    unsigned char channelCount;
    if (0 > m_pFaad->Init2(m_hDecoder, configBuffer, configBufferSize, &sampleRate, &channelCount)) {
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
    setSampleRate(sampleRate);
    initFrameIndexRangeOnce(
            mixxx::IndexRange::forward(
                    0,
                    ((m_maxSampleBlockId - kSampleBlockIdMin) + 1) * m_framesPerSampleBlock));

    const SINT sampleBufferCapacity =
            frames2samples(m_framesPerSampleBlock);
    if (m_sampleBuffer.capacity() < sampleBufferCapacity) {
        m_sampleBuffer.adjustCapacity(sampleBufferCapacity);
    }

    // Discard all buffered samples
    m_inputBufferLength = 0;

    // Invalidate current position(s)
    m_curSampleBlockId = MP4_INVALID_SAMPLE_ID;
    m_curFrameIndex = frameIndexMax();

    return true;
}

void SoundSourceM4A::closeDecoder() {
    if (m_hDecoder != nullptr) {
        m_pFaad->Close(m_hDecoder);
        m_hDecoder = nullptr;
    }
}

bool SoundSourceM4A::reopenDecoder() {
    closeDecoder();
    return openDecoder();
}

void SoundSourceM4A::close() {
    closeDecoder();
    m_sampleBuffer.clear();
    m_inputBuffer.clear();
    if (MP4_INVALID_FILE_HANDLE != m_hFile) {
        MP4Close(m_hFile);
        m_hFile = MP4_INVALID_FILE_HANDLE;
    }
}

bool SoundSourceM4A::isValidSampleBlockId(MP4SampleId sampleBlockId) const {
    return (sampleBlockId >= kSampleBlockIdMin) && (sampleBlockId <= m_maxSampleBlockId);
}

void SoundSourceM4A::restartDecoding(MP4SampleId sampleBlockId) {
    DEBUG_ASSERT(sampleBlockId >= kSampleBlockIdMin);

    m_pFaad->PostSeekReset(m_hDecoder, sampleBlockId);
    m_curSampleBlockId = sampleBlockId;
    m_curFrameIndex = frameIndexMin() +
            (sampleBlockId - kSampleBlockIdMin) * m_framesPerSampleBlock;

    // Discard input buffer
    m_inputBufferLength = 0;

    // Discard previously decoded sample data
    m_sampleBuffer.clear();
}

ReadableSampleFrames SoundSourceM4A::readSampleFramesClamped(
        WritableSampleFrames writableSampleFrames) {
    const SINT firstFrameIndex = writableSampleFrames.frameIndexRange().start();

    if (m_curFrameIndex != firstFrameIndex) {
        // NOTE(uklotzde): Resetting the decoder near to the beginning
        // of the stream when seeking backwards produces invalid sample
        // values! As a consequence the seeking test fails.
        if ((m_curSampleBlockId != MP4_INVALID_SAMPLE_ID) &&
                (firstFrameIndex < m_curFrameIndex) &&
                (firstFrameIndex <= (frameIndexMin() + kNumberOfPrefetchFrames))) {
            // Workaround: Reset the decoder when seeking near to the beginning
            // of the stream while decoding.
            reopenDecoder();
        }

        MP4SampleId sampleBlockId = kSampleBlockIdMin + (firstFrameIndex / m_framesPerSampleBlock);
        DEBUG_ASSERT(isValidSampleBlockId(sampleBlockId));
        if ((firstFrameIndex < m_curFrameIndex) ||                                         // seeking backwards?
                !isValidSampleBlockId(m_curSampleBlockId) ||                               // invalid seek position?
                (sampleBlockId > (m_curSampleBlockId + m_numberOfPrefetchSampleBlocks))) { // jumping forward?
            // Restart decoding one or more blocks of samples backwards
            // from the calculated starting block to avoid audible glitches.
            // Implementation note: The type MP4SampleId is unsigned so we
            // need to be careful when subtracting!
            if ((kSampleBlockIdMin + m_numberOfPrefetchSampleBlocks) < sampleBlockId) {
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
        if (!precedingFrames.empty() && (precedingFrames != readSampleFramesClamped(WritableSampleFrames(precedingFrames)).frameIndexRange())) {
            kLogger.warning()
                    << "Failed to skip preceding frames"
                    << precedingFrames;
            // Abort
            return ReadableSampleFrames(
                    IndexRange::between(
                            m_curFrameIndex,
                            m_curFrameIndex));
        }
    }
    DEBUG_ASSERT(m_curFrameIndex == firstFrameIndex);

    const SINT numberOfSamplesTotal = frames2samples(writableSampleFrames.frameLength());

    SINT numberOfSamplesRemaining = numberOfSamplesTotal;
    SINT outputSampleOffset = 0;
    while (0 < numberOfSamplesRemaining) {
        if (!m_sampleBuffer.empty()) {
            // Consume previously decoded sample data
            const SampleBuffer::ReadableSlice readableSlice(
                    m_sampleBuffer.shrinkForReading(numberOfSamplesRemaining));
            if (writableSampleFrames.writableData()) {
                SampleUtil::copy(
                        writableSampleFrames.writableData(outputSampleOffset),
                        readableSlice.data(),
                        readableSlice.length());
                outputSampleOffset += readableSlice.length();
            }
            m_curFrameIndex += samples2frames(readableSlice.length());
            DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
            DEBUG_ASSERT(numberOfSamplesRemaining >= readableSlice.length());
            numberOfSamplesRemaining -= readableSlice.length();
            if (0 == numberOfSamplesRemaining) {
                break; // exit loop
            }
        }
        // All previously decoded sample data has been consumed now
        DEBUG_ASSERT(m_sampleBuffer.empty());

        if (0 == m_inputBufferLength) {
            // Fill input buffer from file
            if (isValidSampleBlockId(m_curSampleBlockId)) {
                // Read data for next sample block into input buffer
                u_int8_t* pInputBuffer = &m_inputBuffer[0];
                u_int32_t inputBufferLength = m_inputBuffer.size(); // in/out parameter
                if (!MP4ReadSample(m_hFile, m_trackId, m_curSampleBlockId, &pInputBuffer, &inputBufferLength, nullptr, nullptr, nullptr, nullptr)) {
                    kLogger.warning()
                            << "Failed to read MP4 input data for sample block"
                            << m_curSampleBlockId << "("
                            << "min ="
                            << kSampleBlockIdMin << ","
                            << "max ="
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

        // NOTE(uklotzde): The sample buffer for Decode2 has to
        // be big enough for a whole block of decoded samples, which
        // contains up to m_framesPerSampleBlock frames. Otherwise
        // we need to use a temporary buffer.
        CSAMPLE* pDecodeBuffer; // in/out parameter
        SINT decodeBufferCapacity;
        const SINT decodeBufferCapacityMin = frames2samples(m_framesPerSampleBlock);
        if (writableSampleFrames.writableData() &&
                (decodeBufferCapacityMin <= numberOfSamplesRemaining)) {
            // Decode samples directly into the output buffer
            pDecodeBuffer = writableSampleFrames.writableData(outputSampleOffset);
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

        LibFaadLoader::FrameInfo decFrameInfo;
        void* pDecodeResult = m_pFaad->Decode2(
                m_hDecoder, &decFrameInfo, &m_inputBuffer[m_inputBufferOffset], m_inputBufferLength, reinterpret_cast<void**>(&pDecodeBuffer), decodeBufferCapacity * sizeof(*pDecodeBuffer));
        // Verify the decoding result
        if (0 != decFrameInfo.error) {
            kLogger.warning() << "AAC decoding error:"
                              << decFrameInfo.error
                              << m_pFaad->GetErrorMessage(decFrameInfo.error)
                              << getUrlString();
            break; // abort
        }
        Q_UNUSED(pDecodeResult); // only used in DEBUG_ASSERT
        DEBUG_ASSERT(pDecodeResult == pDecodeBuffer); // verify the in/out parameter

        // Verify the decoded sample data for consistency
        VERIFY_OR_DEBUG_ASSERT(channelCount() == decFrameInfo.channels) {
            kLogger.critical()
                    << "Corrupt or unsupported AAC file:"
                    << "Unexpected number of channels" << decFrameInfo.channels
                    << "<>" << channelCount();
            break; // abort
        }
        VERIFY_OR_DEBUG_ASSERT(sampleRate() == SINT(decFrameInfo.samplerate)) {
            kLogger.critical()
                    << "Corrupt or unsupported AAC file:"
                    << "Unexpected sample rate" << decFrameInfo.samplerate
                    << "<>" << sampleRate();
            break; // abort
        }

        // Consume input data
        m_inputBufferLength -= decFrameInfo.bytesconsumed;
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
        m_curFrameIndex += samples2frames(numberOfSamplesRead);
        numberOfSamplesRemaining -= numberOfSamplesRead;
    }

    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
    DEBUG_ASSERT(numberOfSamplesTotal >= numberOfSamplesRemaining);
    const SINT numberOfSamples = numberOfSamplesTotal - numberOfSamplesRemaining;
    return ReadableSampleFrames(
            IndexRange::forward(firstFrameIndex, samples2frames(numberOfSamples)),
            SampleBuffer::ReadableSlice(
                    writableSampleFrames.writableData(),
                    std::min(writableSampleFrames.writableLength(), numberOfSamples)));
}

QString SoundSourceProviderM4A::getName() const {
    return "Nero FAAD2";
}

QStringList SoundSourceProviderM4A::getSupportedFileExtensions() const {
    QStringList supportedFileExtensions;
    if (LibFaadLoader::Instance()->isLoaded()) {
        supportedFileExtensions.append("m4a");
        supportedFileExtensions.append("mp4");
    }
    return supportedFileExtensions;
}

SoundSourcePointer SoundSourceProviderM4A::newSoundSource(const QUrl& url) {
    return newSoundSourceFromUrl<SoundSourceM4A>(url);
}

} // namespace mixxx
