#include "sources/soundsourcemediafoundation.h"

#include <mfapi.h>
#include <mferror.h>
#include <propvarutil.h>

#include "util/logger.h"
#include "util/sample.h"

namespace {

const mixxx::Logger kLogger("SoundSourceMediaFoundation");

constexpr SINT kUnknownFrameIndex = -1;

constexpr SINT kBytesPerSample = sizeof(CSAMPLE);
constexpr SINT kBitsPerSample = kBytesPerSample * 8;
constexpr SINT kLeftoverSize = 4096; // in CSAMPLE's, this seems to be the size MF AAC

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

// Only read the first audio stream
constexpr DWORD kStreamIndex = MF_SOURCE_READER_FIRST_AUDIO_STREAM;

/** Microsoft examples use this snippet often. */
template<class T>
static void safeRelease(T** ppT) {
    if (*ppT) {
        (*ppT)->Release();
        *ppT = nullptr;
    }
}

} // anonymous namespace

namespace mixxx {

//static
const QString SoundSourceProviderMediaFoundation::kDisplayName =
        QStringLiteral("Microsoft Media Foundation");

//static
const QStringList SoundSourceProviderMediaFoundation::kSupportedFileTypes = {
        QStringLiteral("aac"),
        QStringLiteral("m4a"),
        QStringLiteral("mp4"),
};

SoundSourceProviderPriority SoundSourceProviderMediaFoundation::getPriorityHint(
        const QString& supportedFileType) const {
    Q_UNUSED(supportedFileType)
    // On Windows SoundSourceMediaFoundation is the preferred decoder for all
    // supported audio formats.
    return SoundSourceProviderPriority::Higher;
}

SoundSourcePointer SoundSourceProviderMediaFoundation::newSoundSource(const QUrl& url) {
    return newSoundSourceFromUrl<SoundSourceMediaFoundation>(url);
}

QString SoundSourceProviderMediaFoundation::getVersionString() const {
    HMODULE mfModule = GetModuleHandle(L"mfplat.dll");
    VERIFY_OR_DEBUG_ASSERT(mfModule) {
        return QString();
    }
    wchar_t dllPath[MAX_PATH];
    DWORD pathLength = GetModuleFileName(mfModule, dllPath, MAX_PATH);
    DWORD versionInfoSize = GetFileVersionInfoSize(dllPath, nullptr);
    VERIFY_OR_DEBUG_ASSERT(versionInfoSize > 0) {
        qWarning() << "failed to read" << dllPath << "error:" << GetLastError();
        return QString();
    }
    QVarLengthArray<BYTE> info(static_cast<int>(versionInfoSize));
    if (GetFileVersionInfo(dllPath, 0, versionInfoSize, info.data())) {
        UINT size = 0;
        VS_FIXEDFILEINFO* pVerInfo = nullptr;
        if (VerQueryValue(info.data(), L"\\", reinterpret_cast<LPVOID*>(&pVerInfo), &size) &&
                pVerInfo != nullptr &&
                size >= sizeof(VS_FIXEDFILEINFO)) {
            return QStringLiteral("%1.%2.%3.%4")
                    .arg(QString::number(HIWORD(pVerInfo->dwProductVersionMS)),
                            QString::number(
                                    LOWORD(pVerInfo->dwProductVersionMS)),
                            QString::number(
                                    HIWORD(pVerInfo->dwProductVersionLS)),
                            QString::number(
                                    LOWORD(pVerInfo->dwProductVersionLS)));
        }
    }
    qWarning() << "failed to read version from" << dllPath << "error:" << GetLastError();
    return QString();
}

SoundSourceMediaFoundation::SoundSourceMediaFoundation(const QUrl& url)
        : SoundSource(url),
          m_hrCoInitialize(E_FAIL),
          m_hrMFStartup(E_FAIL),
          m_pSourceReader(nullptr),
          m_currentFrameIndex(0) {
}

SoundSourceMediaFoundation::~SoundSourceMediaFoundation() {
    close();
}

SoundSource::OpenResult SoundSourceMediaFoundation::tryOpen(
        OpenMode /*mode*/,
        const OpenParams& params) {
    VERIFY_OR_DEBUG_ASSERT(!SUCCEEDED(m_hrCoInitialize)) {
        kLogger.warning()
                << "Cannot reopen file"
                << getUrlString();
        return OpenResult::Failed;
    }

    const QString fileName(getLocalFileName());

    // Initialize the COM library.
    m_hrCoInitialize = CoInitializeEx(nullptr,
            COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(m_hrCoInitialize)) {
        kLogger.warning()
                << "failed to initialize COM";
        return OpenResult::Failed;
    }
    // Initialize the Media Foundation platform.
    m_hrMFStartup = MFStartup(MF_VERSION);
    if (FAILED(m_hrCoInitialize)) {
        kLogger.warning()
                << "failed to initialize Media Foundation";
        return OpenResult::Failed;
    }

    // Create the source reader to read the input file.
    // Note: we cannot use QString::toStdWString since QT 4 is compiled with
    // '/Zc:wchar_t-' flag and QT 5 not
    const ushort* const fileNameUtf16 = fileName.utf16();
    static_assert(sizeof(wchar_t) == sizeof(ushort), "QString::utf16(): wchar_t and ushort have different sizes");
    HRESULT hr = MFCreateSourceReaderFromURL(
            reinterpret_cast<const wchar_t*>(fileNameUtf16),
            nullptr,
            &m_pSourceReader);

    if (FAILED(hr)) {
        kLogger.warning()
                << "Error opening input file:"
                << fileName;
        return OpenResult::Failed;
    }

    if (!configureAudioStream(params)) {
        kLogger.warning()
                << "Failed to configure audio stream";
        return OpenResult::Failed;
    }

    m_streamUnitConverter = StreamUnitConverter(this);

    if (!readProperties()) {
        kLogger.warning()
                << "Failed to read file properties";
        return OpenResult::Failed;
    }

    //Seek to first position, which forces us to skip over all the header frames.
    //This makes sure we're ready to just let the Analyzer rip and it'll
    //get the number of samples it expects (ie. no header frames).
    seekSampleFrame(frameIndexMin());

    return OpenResult::Succeeded;
}

void SoundSourceMediaFoundation::close() {
    safeRelease(&m_pSourceReader);

    if (SUCCEEDED(m_hrMFStartup)) {
        MFShutdown();
        m_hrMFStartup = E_FAIL;
    }
    if (SUCCEEDED(m_hrCoInitialize)) {
        CoUninitialize();
        m_hrCoInitialize = E_FAIL;
    }
}

void SoundSourceMediaFoundation::seekSampleFrame(SINT frameIndex) {
    DEBUG_ASSERT(isValidFrameIndex(frameIndex));

    if (isValidFrameIndex(m_currentFrameIndex) && (m_currentFrameIndex < frameIndex)) {
        // seeking forward
        const auto skipFrames = IndexRange::between(m_currentFrameIndex, frameIndex);
        // When to prefer skipping over seeking:
        // 1) The sample buffer would be discarded before seeking anyway and
        //    skipping those already decoded samples effectively costs nothing
        // 2) After seeking we need to decode at least kNumberOfPrefetchFrames
        //    before reaching the actual target position -> Only seek if we
        //    need to decode more than  2 * kNumberOfPrefetchFrames frames
        //    while skipping
        SINT skipFramesCountMax =
                getSignalInfo().samples2frames(m_sampleBuffer.readableLength()) +
                2 * kNumberOfPrefetchFrames;
        if (skipFrames.length() <= skipFramesCountMax) {
            if (skipFrames != readSampleFramesClamped(WritableSampleFrames(skipFrames)).frameIndexRange()) {
                kLogger.warning()
                        << "Failed to skip frames before decoding"
                        << skipFrames;
                return; // abort
            }
        }
    }
    if (m_currentFrameIndex != frameIndex) {
        // Discard decoded samples
        m_sampleBuffer.clear();

        // Invalidate current position (end of stream to prevent further reading)
        m_currentFrameIndex = frameIndexMax();

        if (m_pSourceReader == nullptr) {
            // reader is dead
            return; // abort
        }

        // Jump to a position before the actual seeking position.
        // Prefetching a certain number of frames is necessary for
        // sample accurate decoding. The decoder needs to decode
        // some frames in advance to produce the same result at
        // each position in the stream.
        SINT seekIndex = std::max(SINT(frameIndex - kNumberOfPrefetchFrames), frameIndexMin());
        DEBUG_ASSERT(isValidFrameIndex(seekIndex));

        LONGLONG seekPos = m_streamUnitConverter.fromFrameIndex(seekIndex);
        DEBUG_ASSERT(seekPos >= 0);
        PROPVARIANT prop;
        HRESULT hrInitPropVariantFromInt64 =
                InitPropVariantFromInt64(seekPos, &prop);
        DEBUG_ASSERT(SUCCEEDED(hrInitPropVariantFromInt64)); // never fails
        HRESULT hrSetCurrentPosition =
                m_pSourceReader->SetCurrentPosition(GUID_NULL, prop);
        PropVariantClear(&prop);
        if (SUCCEEDED(hrSetCurrentPosition)) {
            // NOTE(uklotzde): After SetCurrentPosition() the actual position
            // of the stream is unknown until reading the next samples from
            // the reader. Please note that the first sample decoded after
            // SetCurrentPosition() may start BEFORE the actual target position.
            // See also: https://msdn.microsoft.com/en-us/library/windows/desktop/dd374668(v=vs.85).aspx
            //   "The SetCurrentPosition method does not guarantee exact seeking." ...
            //   "After seeking, the application should call IMFSourceReader::ReadSample
            //    and advance to the desired position.
            auto skipFrames = IndexRange::between(seekIndex, frameIndex);
            if (skipFrames.empty()) {
                // We are at the beginning of the stream and don't need
                // to skip any frames. Calling IMFSourceReader::ReadSample
                // is not necessary in this special case.
                DEBUG_ASSERT(frameIndex == frameIndexMin());
                m_currentFrameIndex = frameIndex;
            } else {
                // We need to fetch at least 1 sample from the reader to obtain the
                // current position!
                m_currentFrameIndex = kUnknownFrameIndex; // prevent further seeking
                DEBUG_ASSERT(!isValidFrameIndex(m_currentFrameIndex));
                if (skipFrames != readSampleFramesClamped(WritableSampleFrames(skipFrames)).frameIndexRange()) {
                    kLogger.warning()
                            << "Failed to skip frames while seeking"
                            << skipFrames;
                    return; // abort
                }
                // Now m_currentFrameIndex reflects the actual position of the reader
                if (m_currentFrameIndex < frameIndex) {
                    // Skip more frames if the seek has taken us to a position before
                    // the requested target position (see comment above about the behavior
                    // of SetCurrentPosition()
                    skipFrames = IndexRange::between(m_currentFrameIndex, frameIndex);
                    // Skip more samples if frameIndex has not yet been reached
                    if (skipFrames != readSampleFramesClamped(WritableSampleFrames(skipFrames)).frameIndexRange()) {
                        kLogger.warning()
                                << "Failed to skip frames while seeking"
                                << skipFrames;
                        return; // abort
                    }
                }
                if (m_currentFrameIndex != frameIndex) {
                    kLogger.warning()
                            << "Seeking to frame"
                            << frameIndex
                            << "failed";
                    // Jump to end of stream to prevent further reading
                    m_currentFrameIndex = frameIndexMax();
                }
            }
        } else {
            kLogger.warning()
                    << "IMFSourceReader::SetCurrentPosition() failed"
                    << hrSetCurrentPosition;
            safeRelease(&m_pSourceReader); // kill the reader
        }
    }
}

ReadableSampleFrames SoundSourceMediaFoundation::readSampleFramesClamped(
        const WritableSampleFrames& writableSampleFrames) {
    const SINT firstFrameIndex = writableSampleFrames.frameIndexRange().start();
    if (m_currentFrameIndex != kUnknownFrameIndex) {
        seekSampleFrame(firstFrameIndex);
        if (m_currentFrameIndex != firstFrameIndex) {
            kLogger.warning()
                    << "Failed to position reader at beginning of decoding range"
                    << writableSampleFrames.frameIndexRange();
            // Abort
            return ReadableSampleFrames(
                    mixxx::IndexRange::between(
                            m_currentFrameIndex,
                            m_currentFrameIndex));
        }
        DEBUG_ASSERT(m_currentFrameIndex == firstFrameIndex);
    } else {
        // Unknown position should only occur after seeking
        // when all temporary buffers are empty
        DEBUG_ASSERT(!isValidFrameIndex(m_currentFrameIndex));
        DEBUG_ASSERT(m_sampleBuffer.empty());
    }

    const SINT numberOfFramesTotal = writableSampleFrames.frameLength();

    CSAMPLE* pSampleBuffer = writableSampleFrames.writableData();
    SINT numberOfFramesRemaining = numberOfFramesTotal;
    while (numberOfFramesRemaining > 0) {
        SampleBuffer::ReadableSlice readableSlice(
                m_sampleBuffer.shrinkForReading(
                        getSignalInfo().frames2samples(numberOfFramesRemaining)));
        DEBUG_ASSERT(readableSlice.length() <= getSignalInfo().frames2samples(numberOfFramesRemaining));
        if (readableSlice.length() > 0) {
            DEBUG_ASSERT(isValidFrameIndex(m_currentFrameIndex));
            DEBUG_ASSERT(m_currentFrameIndex < frameIndexMax());
            if (pSampleBuffer) {
                SampleUtil::copy(
                        pSampleBuffer,
                        readableSlice.data(),
                        readableSlice.length());
                pSampleBuffer += readableSlice.length();
            }
            m_currentFrameIndex += getSignalInfo().samples2frames(readableSlice.length());
            numberOfFramesRemaining -= getSignalInfo().samples2frames(readableSlice.length());
        }
        if (numberOfFramesRemaining == 0) {
            break; // finished reading
        }

        // No more decoded sample frames available
        DEBUG_ASSERT(m_sampleBuffer.empty());

        if (m_pSourceReader == nullptr) {
            break; // abort if reader is dead
        }

        DWORD dwFlags = 0;
        LONGLONG streamPos = 0;
        IMFSample* pSample = nullptr;
        HRESULT hrReadSample =
                m_pSourceReader->ReadSample(
                        kStreamIndex, // [in]  DWORD dwStreamIndex,
                        0,            // [in]  DWORD dwControlFlags,
                        nullptr,      // [out] DWORD *pdwActualStreamIndex,
                        &dwFlags,     // [out] DWORD *pdwStreamFlags,
                        &streamPos,   // [out] LONGLONG *pllTimestamp,
                        &pSample);    // [out] IMFSample **ppSample
        if (FAILED(hrReadSample)) {
            kLogger.warning()
                    << "IMFSourceReader::ReadSample() failed"
                    << hrReadSample
                    << "-> abort decoding";
            DEBUG_ASSERT(pSample == nullptr);
            break; // abort
        }
        if (dwFlags & MF_SOURCE_READERF_ERROR) {
            kLogger.warning()
                    << "IMFSourceReader::ReadSample()"
                    << "detected stream errors"
                    << "(MF_SOURCE_READERF_ERROR)"
                    << "-> abort and stop decoding";
            DEBUG_ASSERT(pSample == nullptr);
            safeRelease(&m_pSourceReader); // kill the reader
            break;                         // abort
        } else if (dwFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
            DEBUG_ASSERT(pSample == nullptr);
            break; // finished reading
        } else if (dwFlags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED) {
            kLogger.warning()
                    << "IMFSourceReader::ReadSample()"
                    << "detected that the media type has changed"
                    << "(MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)"
                    << "-> abort decoding";
            DEBUG_ASSERT(pSample == nullptr);
            break; // abort
        }
        DEBUG_ASSERT(pSample != nullptr);
        SINT readerFrameIndex = m_streamUnitConverter.toFrameIndex(streamPos);
        // TODO: Fix debug assertion in else arm. It has been commented
        // out deliberately to prevent crashes in debug builds.
        // https://github.com/mixxxdj/mixxx/issues/10160
        if (m_currentFrameIndex == kUnknownFrameIndex) {
            // Unknown position after seeking
            m_currentFrameIndex = readerFrameIndex;
            //     kLogger.debug()
            //             << "Stream position (in sample frames) after seeking:"
            //             << "target =" << writableSampleFrames.frameIndexRange().end()
            //             << "current =" << readerFrameIndex;
            // } else {
            //     // Both positions should match, otherwise readerFrameIndex
            //     // is inaccurate due to rounding errors after conversion from
            //     // stream units to frames! But if this ever happens we better
            //     // trust m_currentFrameIndex that is continuously updated while
            //     // reading in forward direction.
            //     VERIFY_OR_DEBUG_ASSERT(m_currentFrameIndex == readerFrameIndex) {
            //         kLogger.debug()
            //                 << "streamPos [100 ns] =" << streamPos
            //                 << ", sampleRate =" << getSignalInfo().getSampleRate();
            //         kLogger.warning()
            //                 << "Stream position (in sample frames) while reading is inaccurate:"
            //                 << "expected =" << m_currentFrameIndex
            //                 << "actual =" << readerFrameIndex;
            //     }
        }

        DWORD dwSampleBufferCount = 0;
        HRESULT hrGetBufferCount =
                pSample->GetBufferCount(&dwSampleBufferCount);
        if (FAILED(hrGetBufferCount)) {
            kLogger.warning()
                    << "IMFSample::GetBufferCount() failed"
                    << hrGetBufferCount
                    << "-> abort decoding";
            safeRelease(&pSample);
            break; // abort
        }

        DWORD dwSampleTotalLengthInBytes = 0;
        HRESULT hrGetTotalLength = pSample->GetTotalLength(&dwSampleTotalLengthInBytes);
        if (FAILED(hrGetTotalLength)) {
            kLogger.warning()
                    << "IMFSample::GetTotalLength() failed"
                    << hrGetTotalLength
                    << "-> abort decoding";
            safeRelease(&pSample);
            break; // abort
        }
        // Enlarge temporary buffer (if necessary)
        DEBUG_ASSERT((dwSampleTotalLengthInBytes % kBytesPerSample) == 0);
        SINT numberOfSamplesToBuffer =
                dwSampleTotalLengthInBytes / kBytesPerSample;
        SINT sampleBufferCapacity = m_sampleBuffer.capacity();
        DEBUG_ASSERT(sampleBufferCapacity > 0);
        while (sampleBufferCapacity < numberOfSamplesToBuffer) {
            sampleBufferCapacity *= 2;
        }
        if (m_sampleBuffer.capacity() < sampleBufferCapacity) {
            kLogger.debug()
                    << "Enlarging sample buffer capacity"
                    << m_sampleBuffer.capacity()
                    << "->"
                    << sampleBufferCapacity;
            m_sampleBuffer.adjustCapacity(sampleBufferCapacity);
        }

        DWORD dwSampleBufferIndex = 0;
        while (dwSampleBufferIndex < dwSampleBufferCount) {
            IMFMediaBuffer* pMediaBuffer = nullptr;
            HRESULT hrGetBufferByIndex = pSample->GetBufferByIndex(dwSampleBufferIndex, &pMediaBuffer);
            if (FAILED(hrGetBufferByIndex)) {
                kLogger.warning()
                        << "IMFSample::GetBufferByIndex() failed"
                        << hrGetBufferByIndex
                        << "-> abort decoding";
                DEBUG_ASSERT(pMediaBuffer == nullptr);
                break; // prematurely exit buffer loop
            }

            CSAMPLE* pLockedSampleBuffer = nullptr;
            DWORD lockedSampleBufferLengthInBytes = 0;
            HRESULT hrLock = pMediaBuffer->Lock(
                    reinterpret_cast<quint8**>(&pLockedSampleBuffer),
                    nullptr,
                    &lockedSampleBufferLengthInBytes);
            if (FAILED(hrLock)) {
                kLogger.warning()
                        << "IMFMediaBuffer::Lock() failed"
                        << hrLock
                        << "-> abort decoding";
                safeRelease(&pMediaBuffer);
                break; // prematurely exit buffer loop
            }

            DEBUG_ASSERT((lockedSampleBufferLengthInBytes % sizeof(pLockedSampleBuffer[0])) == 0);
            SINT lockedSampleBufferCount =
                    lockedSampleBufferLengthInBytes / sizeof(pLockedSampleBuffer[0]);
            SINT copySamplesCount = std::min(
                    getSignalInfo().frames2samples(numberOfFramesRemaining),
                    lockedSampleBufferCount);
            if (copySamplesCount > 0) {
                // Copy samples directly into output buffer if possible
                if (pSampleBuffer != nullptr) {
                    SampleUtil::copy(
                            pSampleBuffer,
                            pLockedSampleBuffer,
                            copySamplesCount);
                    pSampleBuffer += copySamplesCount;
                }
                pLockedSampleBuffer += copySamplesCount;
                lockedSampleBufferCount -= copySamplesCount;
                m_currentFrameIndex += getSignalInfo().samples2frames(copySamplesCount);
                numberOfFramesRemaining -= getSignalInfo().samples2frames(copySamplesCount);
            }
            // Buffer the remaining samples
            SampleBuffer::WritableSlice writableSlice(
                    m_sampleBuffer.growForWriting(lockedSampleBufferCount));
            // The required capacity has been calculated in advance (see above)
            DEBUG_ASSERT(writableSlice.length() == lockedSampleBufferCount);
            SampleUtil::copy(
                    writableSlice.data(),
                    pLockedSampleBuffer,
                    writableSlice.length());
            HRESULT hrUnlock = pMediaBuffer->Unlock();
            VERIFY_OR_DEBUG_ASSERT(SUCCEEDED(hrUnlock)) {
                kLogger.warning()
                        << "IMFMediaBuffer::Unlock() failed"
                        << hrUnlock;
                // ignore and continue
            }
            safeRelease(&pMediaBuffer);
            ++dwSampleBufferIndex;
        }
        safeRelease(&pSample);
        if (dwSampleBufferIndex < dwSampleBufferCount) {
            // Failed to read data from all buffers -> kill the reader
            kLogger.warning()
                    << "Failed to read all buffered samples"
                    << "-> abort and stop decoding";
            safeRelease(&m_pSourceReader);
            break; // abort
        }
    }

    DEBUG_ASSERT(isValidFrameIndex(m_currentFrameIndex));
    DEBUG_ASSERT(numberOfFramesTotal >= numberOfFramesRemaining);
    const SINT numberOfFrames = numberOfFramesTotal - numberOfFramesRemaining;
    return ReadableSampleFrames(
            IndexRange::forward(firstFrameIndex, numberOfFrames),
            SampleBuffer::ReadableSlice(
                    writableSampleFrames.writableData(),
                    std::min(writableSampleFrames.writableLength(), getSignalInfo().frames2samples(numberOfFrames))));
}

namespace {

bool configureMediaType(
        IMFSourceReader* pSourceReader,
        SINT* pBitrate,
        const AudioSource::OpenParams& params = AudioSource::OpenParams()) {
    DEBUG_ASSERT(pSourceReader);
    DEBUG_ASSERT(pBitrate);

    HRESULT hr;

    IMFMediaType* pAudioType = nullptr;
    hr = pSourceReader->GetCurrentMediaType(
            kStreamIndex, &pAudioType);
    if (FAILED(hr)) {
        kLogger.warning() << hr
                          << "failed to get current media type from stream";
        return false;
    }

    //------ Get bitrate from the file, before we change it to get uncompressed audio
    UINT32 avgBytesPerSecond;
    hr = pAudioType->GetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, &avgBytesPerSecond);
    if (FAILED(hr)) {
        kLogger.warning() << hr
                          << "error getting MF_MT_AUDIO_AVG_BYTES_PER_SECOND";
        safeRelease(&pAudioType);
        return false;
    }
    *pBitrate = (avgBytesPerSecond * 8) / 1000;

    hr = pAudioType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    if (FAILED(hr)) {
        kLogger.warning() << hr
                          << "failed to set major type to audio";
        safeRelease(&pAudioType);
        return false;
    }

    hr = pAudioType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_Float);
    if (FAILED(hr)) {
        kLogger.warning() << hr
                          << "failed to set subtype format to float";
        safeRelease(&pAudioType);
        return false;
    }

    hr = pAudioType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, true);
    if (FAILED(hr)) {
        kLogger.warning() << hr
                          << "failed to set all samples independent";
        safeRelease(&pAudioType);
        return false;
    }

    hr = pAudioType->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, true);
    if (FAILED(hr)) {
        kLogger.warning() << hr
                          << "failed to set fixed size samples";
        safeRelease(&pAudioType);
        return false;
    }

    hr = pAudioType->SetUINT32(
            MF_MT_AUDIO_BITS_PER_SAMPLE, kBitsPerSample);
    if (FAILED(hr)) {
        kLogger.warning() << hr
                          << "failed to set bits per sample:"
                          << kBitsPerSample;
        safeRelease(&pAudioType);
        return false;
    }

    const UINT sampleSize = kLeftoverSize * kBytesPerSample;
    hr = pAudioType->SetUINT32(
            MF_MT_SAMPLE_SIZE, sampleSize);
    if (FAILED(hr)) {
        kLogger.warning() << hr
                          << "failed to set sample size:"
                          << sampleSize;
        safeRelease(&pAudioType);
        return false;
    }

    UINT32 numChannels;
    hr = pAudioType->GetUINT32(
            MF_MT_AUDIO_NUM_CHANNELS, &numChannels);
    if (FAILED(hr)) {
        kLogger.warning() << hr
                          << "failed to get actual number of channels";
        safeRelease(&pAudioType);
        return false;
    }
    kLogger.debug() << "Number of channels in input stream" << numChannels;
    if (params.getSignalInfo().getChannelCount().isValid()) {
        numChannels = std::min(params.getSignalInfo().getChannelCount(),
                mixxx::kMaxEngineChannelInputCount);
        hr = pAudioType->SetUINT32(
                MF_MT_AUDIO_NUM_CHANNELS, numChannels);
        if (FAILED(hr)) {
            kLogger.warning() << hr
                              << "failed to set number of channels:"
                              << numChannels;
            safeRelease(&pAudioType);
            return false;
        }
        kLogger.debug() << "Requested number of channels" << numChannels;
    }

    UINT32 samplesPerSecond;
    hr = pAudioType->GetUINT32(
            MF_MT_AUDIO_SAMPLES_PER_SECOND, &samplesPerSecond);
    if (FAILED(hr)) {
        kLogger.warning() << hr
                          << "failed to get samples per second";
        safeRelease(&pAudioType);
        return false;
    }
    kLogger.debug() << "Samples per second in input stream" << samplesPerSecond;
    if (params.getSignalInfo().getSampleRate().isValid()) {
        samplesPerSecond = params.getSignalInfo().getSampleRate();
        hr = pAudioType->SetUINT32(
                MF_MT_AUDIO_SAMPLES_PER_SECOND, samplesPerSecond);
        if (FAILED(hr)) {
            kLogger.warning() << hr
                              << "failed to set samples per second:"
                              << samplesPerSecond;
            safeRelease(&pAudioType);
            return false;
        }
        kLogger.debug() << "Requested samples per second" << samplesPerSecond;
    }

    // Set this type on the source reader. The source reader will
    // load the necessary decoder.
    hr = pSourceReader->SetCurrentMediaType(
            kStreamIndex, nullptr, pAudioType);
    if (FAILED(hr)) {
        kLogger.warning() << hr
                          << "failed to set media type";
        safeRelease(&pAudioType);
        return false;
    }

    safeRelease(&pAudioType);
    return true;
}

} // anonymous namespace

//-------------------------------------------------------------------
// configureAudioStream
//
// Selects an audio stream from the source file, and configures the
// stream to deliver decoded PCM audio.
//-------------------------------------------------------------------

/** Cobbled together from:
 http://msdn.microsoft.com/en-us/library/dd757929(v=vs.85).aspx
 and http://msdn.microsoft.com/en-us/library/dd317928(VS.85).aspx
 -- Albert
 If anything in here fails, just bail. I'm not going to decode HRESULTS.
 -- Bill
 */
bool SoundSourceMediaFoundation::configureAudioStream(const OpenParams& openParams) {
    HRESULT hr;

    // deselect all streams, we only want the first
    hr = m_pSourceReader->SetStreamSelection(
            MF_SOURCE_READER_ALL_STREAMS, false);
    if (FAILED(hr)) {
        kLogger.warning() << hr
                          << "failed to deselect all streams";
        return false;
    }

    hr = m_pSourceReader->SetStreamSelection(
            kStreamIndex, true);
    if (FAILED(hr)) {
        kLogger.warning() << hr
                          << "failed to select first audio stream";
        return false;
    }

    SINT bitrate;
    if (!configureMediaType(m_pSourceReader, &bitrate, openParams)) {
        // Fallback: Ignore custom params
        if (!configureMediaType(m_pSourceReader, &bitrate)) {
            kLogger.warning() << "Unsupported media type";
            return false;
        }
    }
    initBitrateOnce(bitrate);

    // Get the resulting output format.
    IMFMediaType* pAudioType = nullptr;
    hr = m_pSourceReader->GetCurrentMediaType(
            kStreamIndex, &pAudioType);
    if (FAILED(hr)) {
        kLogger.warning() << hr
                          << "failed to retrieve completed media type";
        safeRelease(&pAudioType);
        return false;
    }

    // Ensure the stream is selected.
    hr = m_pSourceReader->SetStreamSelection(
            kStreamIndex, true);
    if (FAILED(hr)) {
        kLogger.warning() << hr
                          << "failed to select first audio stream (again)";
        safeRelease(&pAudioType);
        return false;
    }

    UINT32 numChannels;
    hr = pAudioType->GetUINT32(
            MF_MT_AUDIO_NUM_CHANNELS, &numChannels);
    if (FAILED(hr)) {
        kLogger.warning() << hr
                          << "failed to get actual number of channels";
        safeRelease(&pAudioType);
        return false;
    }
    initChannelCountOnce(numChannels);

    UINT32 samplesPerSecond;
    hr = pAudioType->GetUINT32(
            MF_MT_AUDIO_SAMPLES_PER_SECOND, &samplesPerSecond);
    if (FAILED(hr)) {
        kLogger.warning() << hr
                          << "failed to get the actual sample rate";
        safeRelease(&pAudioType);
        return false;
    }
    initSampleRateOnce(samplesPerSecond);

    UINT32 leftoverBufferSizeInBytes = 0;
    hr = pAudioType->GetUINT32(MF_MT_SAMPLE_SIZE, &leftoverBufferSizeInBytes);
    if (FAILED(hr)) {
        kLogger.warning() << hr
                          << "failed to get sample buffer size (in bytes)";
        safeRelease(&pAudioType);
        return false;
    }
    DEBUG_ASSERT((leftoverBufferSizeInBytes % kBytesPerSample) == 0);
    const SINT sampleBufferCapacity =
            leftoverBufferSizeInBytes / kBytesPerSample;
    if (m_sampleBuffer.capacity() < sampleBufferCapacity) {
        m_sampleBuffer.adjustCapacity(sampleBufferCapacity);
    }
    DEBUG_ASSERT(m_sampleBuffer.capacity() > 0);
    kLogger.debug()
            << "Sample buffer capacity"
            << m_sampleBuffer.capacity();

    safeRelease(&pAudioType);
    return true;
}

bool SoundSourceMediaFoundation::readProperties() {
    PROPVARIANT prop;
    HRESULT hr = S_OK;

    //Get the duration, provided as a 64-bit integer of 100-nanosecond units
    hr = m_pSourceReader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE,
            MF_PD_DURATION,
            &prop);
    if (FAILED(hr)) {
        kLogger.warning() << "error getting duration";
        return false;
    }
    initFrameIndexRangeOnce(
            mixxx::IndexRange::forward(
                    0,
                    m_streamUnitConverter.toFrameIndex(prop.hVal.QuadPart)));
    kLogger.debug() << "Frame index range" << frameIndexRange();
    PropVariantClear(&prop);

    return true;
}

} // namespace mixxx
