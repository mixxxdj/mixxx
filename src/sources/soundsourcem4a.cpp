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

constexpr int kSampleBlockIdMin = 0;
constexpr int kSampleBlockIdInvalid = -1;

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

// http://www.iis.fraunhofer.de/content/dam/iis/de/doc/ame/wp/FraunhoferIIS_Application-Bulletin_AAC-Transport-Formats.pdf
// Footnote 13: "The usual frame length for AAC-LC is 1024 samples, but a 960 sample version
// is used for radio broadcasting, and 480 or 512 sample versions are used for the low-delay
// codecs AAC-LD and AAC-ELD."
constexpr int kDefaultFrameSize = 1024;

inline bool isValidMediaDataName(const char* mediaDataName) {
    return (nullptr != mediaDataName) &&
            (0 == strcasecmp(mediaDataName, "mp4a"));
}

QString formatErrorMessage(int errnum) {
    char errbuf[256];
    if (av_strerror(errnum, errbuf, sizeof(errbuf) / sizeof(errbuf[0]) == 0)) {
        return QString("%1 (%2)").arg(errbuf, errnum);
    } else {
        return QString("No description for error code (%1) found").arg(errnum);
    }
}

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

// Searches for the first audio track in the MP4 file that
// suits our needs.
int findFirstAudioTrackId(AVFormatContext* ic) {
    int nb_streams = static_cast<int>(ic->nb_streams);
    for (int i = 0; i < nb_streams; i++) {
        AVStream* st = ic->streams[i];
        DEBUG_ASSERT(st != nullptr);
        DEBUG_ASSERT(st->index == i);
        AVCodecParameters* par = st->codecpar;
        DEBUG_ASSERT(par != nullptr);
        if (par->codec_type != AVMEDIA_TYPE_AUDIO) {
            // non audio streams
            kLogger.warning() << "Unsupported stream type" << par->codec_type;
            kLogger.warning() << "Skipping stream" << i
                              << "of" << ic->nb_streams;
            continue;
        }
        if (par->codec_id != AV_CODEC_ID_AAC) {
            // no AAC
            kLogger.warning() << "No AAC stream" << par->codec_id;
            kLogger.warning() << "Skipping stream" << i
                              << "of" << ic->nb_streams;
            continue;
        }
        if (!par->channels || !par->sample_rate) {
            // Skip invalid or non audio streams
            kLogger.warning() << "Missing channels or sample rate";
            kLogger.warning() << "Skipping stream" << i
                              << "of" << ic->nb_streams;
            continue;
        }
        return i;
    }
    return -1;
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

void SoundSourceM4A::InputAVFormatContextPtr::take(
        AVFormatContext** ppAvInputFormatContext) {
    DEBUG_ASSERT(ppAvInputFormatContext != nullptr);
    if (m_pAvInputFormatContext != *ppAvInputFormatContext) {
        close();
        m_pAvInputFormatContext = *ppAvInputFormatContext;
        *ppAvInputFormatContext = nullptr;
    }
}

void SoundSourceM4A::InputAVFormatContextPtr::close() {
    if (m_pAvInputFormatContext != nullptr) {
        avformat_close_input(&m_pAvInputFormatContext);
        DEBUG_ASSERT(m_pAvInputFormatContext == nullptr);
    }
}

SoundSourceM4A::SoundSourceM4A(const QUrl& url)
        : SoundSource(url, "m4a"),
          m_pFaad(faad2::LibLoader::Instance()),
          m_frameSize(-1),
          m_maxSampleBlockId(kSampleBlockIdInvalid),
          m_pAvStream(nullptr),
          m_inputBufferLength(0),
          m_inputBufferOffset(0),
          m_hDecoder(nullptr),
          m_numberOfPrefetchSampleBlocks(0),
          m_curSampleBlockId(kSampleBlockIdInvalid),
          m_curFrameIndex(0) {
}

SoundSourceM4A::~SoundSourceM4A() {
    close();
}

SoundSource::OpenResult SoundSourceM4A::tryOpen(
        OpenMode mode,
        const OpenParams& params) {
    // Open input
    {
        AVFormatContext* pAvInputFormatContext =
                openInputFile(getLocalFileName());
        if (pAvInputFormatContext == nullptr) {
            kLogger.warning()
                    << "Failed to open input file"
                    << getLocalFileName();
            return OpenResult::Failed;
        }
        m_pAvInputFormatContext.take(&pAvInputFormatContext);
    }

    // Retrieve stream information
    const int avformat_find_stream_info_result =
            avformat_find_stream_info(m_pAvInputFormatContext, nullptr);
    if (avformat_find_stream_info_result != 0) {
        DEBUG_ASSERT(avformat_find_stream_info_result < 0);
        kLogger.warning()
                << "avformat_find_stream_info() failed:"
                << formatErrorMessage(avformat_find_stream_info_result).toLocal8Bit().constData();
        return OpenResult::Failed;
    }

    int streamIndex = findFirstAudioTrackId(m_pAvInputFormatContext);
    if (streamIndex < 0) {
        kLogger.warning() << "No AAC track found:" << getUrlString();
        return OpenResult::Aborted;
    }

    DEBUG_ASSERT(streamIndex < static_cast<int>(m_pAvInputFormatContext->nb_streams));

    // Select audio stream for decoding
    m_pAvStream = m_pAvInputFormatContext->streams[streamIndex];
    DEBUG_ASSERT(m_pAvStream);
    AVCodecParameters* par = m_pAvStream->codecpar;
    DEBUG_ASSERT(par);

    // Read frame size.  If the frame size is not is not
    // fixed (that is, if the number of frames per sample block varies
    // through the file), We can't currently handle these.
    m_frameSize = par->frame_size;
    if (m_frameSize <= 0) {
        kLogger.warning() << "Unable to determine the frame size of the track"
                          << streamIndex << "in file" << getUrlString();
        if (mode == OpenMode::Strict) {
            // Abort and give another decoder with lower priority
            // the chance to open the same file.
            // Fixes https://bugs.launchpad.net/mixxx/+bug/1504113
            return OpenResult::Aborted;
        } else {
            // Fallback: Use a default value
            kLogger.warning() << "Fallback: Using a default sample duration of"
                              << kDefaultFrameSize << "sample frames per block";
            m_frameSize = kDefaultFrameSize;
        }
    }

    int numberOfFrames = m_pAvStream->nb_frames;
    if (numberOfFrames <= 0) {
        kLogger.warning() << "Failed to read number of frames from file:" << getUrlString();
        return OpenResult::Failed;
    }
    m_maxSampleBlockId = numberOfFrames - 1;

    if (par->extradata_size <= 0 || !par->extradata) {
        kLogger.info() << "Failed to read the MP4 elementary stream "
                          "(ES) configuration";
    }

    if (kLogger.debugEnabled()) {
        kLogger.debug()
                << "Opened stream for decoding"
                << "{ index" << m_pAvStream->index
                << "| id" << m_pAvStream->id
                << "| codec_type" << m_pAvStream->codecpar->codec_type
                << "| codec_id" << m_pAvStream->codecpar->codec_id
                << "| channels" << m_pAvStream->codecpar->channels
                << "| channel_layout" << m_pAvStream->codecpar->channel_layout
                << "| format" << m_pAvStream->codecpar->format
                << "| sample_rate" << m_pAvStream->codecpar->sample_rate
                << "| bit_rate" << m_pAvStream->codecpar->bit_rate
                << "| frame_size" << m_pAvStream->codecpar->frame_size
                << "| initial_padding" << m_pAvStream->codecpar->initial_padding
                << "| trailing_padding" << m_pAvStream->codecpar->trailing_padding
                << "| seek_preroll" << m_pAvStream->codecpar->seek_preroll
                << "| start_time" << m_pAvStream->start_time
                << "| duration" << m_pAvStream->duration
                << "| nb_frames" << m_pAvStream->nb_frames
                << "| time_base" << m_pAvStream->time_base.num << '/' << m_pAvStream->time_base.den
                << '}';
    }

    m_inputBuffer.resize(m_frameSize, 0);

    m_openParams = params;

    if (openDecoder()) {
        return OpenResult::Succeeded;
    } else {
        return OpenResult::Failed;
    }
}

bool SoundSourceM4A::openDecoder() {
    DEBUG_ASSERT(m_hDecoder == nullptr); // not already opened

    if (!reopenDecoder()) {
        return false;
    }
    DEBUG_ASSERT(m_hDecoder);

    // Calculate how many sample blocks we need to decode in advance
    // of a random seek in order to get the recommended number of
    // prefetch frames
    m_numberOfPrefetchSampleBlocks =
            (kNumberOfPrefetchFrames + (m_frameSize - 1)) /
            m_frameSize;

    const SINT sampleBufferCapacity =
            getSignalInfo().frames2samples(m_frameSize);
    if (m_sampleBuffer.capacity() < sampleBufferCapacity) {
        m_sampleBuffer.adjustCapacity(sampleBufferCapacity);
    }

    // Discard all buffered input data
    m_curSampleBlockId = kSampleBlockIdInvalid;
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
        DEBUG_ASSERT(m_pAvStream);
        AVCodecParameters* par = m_pAvStream->codecpar;
        DEBUG_ASSERT(par);
        unsigned char* pMP4ESConfigBuffer = par->extradata;
        unsigned long sizeofMP4ESConfigBuffer = par->extradata_size;

        if (m_pFaad->Init2(
                    hNewDecoder,
                    pMP4ESConfigBuffer,
                    sizeofMP4ESConfigBuffer,
                    &sampleRate,
                    &channelCount) < 0) {
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
                        m_frameSize))) {
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
    m_pAvInputFormatContext.close();
}

bool SoundSourceM4A::isValidSampleBlockId(int sampleBlockId) const {
    return (sampleBlockId >= kSampleBlockIdMin) &&
            (sampleBlockId <= m_maxSampleBlockId);
}

void SoundSourceM4A::restartDecoding(int sampleBlockId) {
    DEBUG_ASSERT(sampleBlockId >= kSampleBlockIdMin);

    m_pFaad->PostSeekReset(m_hDecoder, sampleBlockId);
    m_curSampleBlockId = sampleBlockId;
    m_curFrameIndex = frameIndexMin() +
            (sampleBlockId - kSampleBlockIdMin) * m_frameSize;

    // Discard input buffer
    m_inputBufferLength = 0;
    m_inputBufferOffset = 0;

    // Discard previously decoded sample data
    m_sampleBuffer.clear();
}

int SoundSourceM4A::seekAndReadSampleBlock(
        int sampleBlockId,
        uint8_t** ppBytes,
        uint32_t* pNumBytes) {
    DEBUG_ASSERT(sampleBlockId >= 0 && sampleBlockId < m_pAvStream->nb_index_entries);

    int error = 0;
    char error_string[AV_ERROR_MAX_STRING_SIZE];

    error = avio_seek(m_pAvInputFormatContext->pb,
            m_pAvStream->index_entries[sampleBlockId].pos,
            SEEK_SET);

    if (error < 0) {
        av_make_error_string(error_string, AV_ERROR_MAX_STRING_SIZE, error);
        qWarning() << "Could not seek frame:" << error_string;
        return error;
    }

    // create an empty package
    AVPacket packet;
    av_init_packet(&packet);
    packet.data = nullptr;
    packet.size = 0;

    error = av_read_frame(m_pAvInputFormatContext, &packet);
    if (error < 0) {
        av_make_error_string(error_string, AV_ERROR_MAX_STRING_SIZE, error);
        qWarning() << "Could not read frame:" << error_string;
    } else if (packet.size <= static_cast<int>(*pNumBytes)) {
        memcpy(*ppBytes, packet.data, packet.size);
    } else {
        error = AVERROR(ENOMEM);
    }
    *pNumBytes = packet.size;
    av_packet_unref(&packet);
    return error;
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
            if ((m_curSampleBlockId >= kSampleBlockIdMin) &&
                    (firstFrameIndex < m_curFrameIndex) &&
                    (firstFrameIndex <=
                            (frameIndexMin() + kNumberOfPrefetchFrames))) {
                // Workaround: Discard remaining input data and reopen the decoder when
                // seeking near to the beginning of the stream while decoding.
                m_curSampleBlockId = kSampleBlockIdInvalid;
                m_inputBufferLength = 0;
                if (!reopenDecoder()) {
                    return {};
                }
            }

            int sampleBlockId = kSampleBlockIdMin +
                    (firstFrameIndex / m_frameSize);
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
                    uint8_t* pInputBuffer = m_inputBuffer.data();
                    uint32_t inputBufferLength = m_inputBuffer.size(); // in/out parameter
                    if (seekAndReadSampleBlock(
                                m_curSampleBlockId,
                                &pInputBuffer,
                                &inputBufferLength) < 0) {
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
                    AVCodecParameters* par = m_pAvStream->codecpar;
                    DEBUG_ASSERT(par != nullptr);
                    if (par->extradata && par->extradata_size > 0 &&
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
            // contains up to m_frameSize frames. Otherwise
            // we need to use a temporary buffer.
            CSAMPLE* pDecodeBuffer; // in/out parameter
            SINT decodeBufferCapacity;
            const SINT decodeBufferCapacityMin =
                    getSignalInfo().frames2samples(m_frameSize);
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

QString SoundSourceProviderM4A::getName() const {
    return "Nero FAAD2";
}

QStringList SoundSourceProviderM4A::getSupportedFileExtensions() const {
    QStringList supportedFileExtensions;
    if (faad2::LibLoader::Instance()->isLoaded()) {
        supportedFileExtensions.append("m4a");
        supportedFileExtensions.append("mp4");
    }
    return supportedFileExtensions;
}

SoundSourcePointer SoundSourceProviderM4A::newSoundSource(const QUrl& url) {
    return newSoundSourceFromUrl<SoundSourceM4A>(url);
}

} // namespace mixxx
