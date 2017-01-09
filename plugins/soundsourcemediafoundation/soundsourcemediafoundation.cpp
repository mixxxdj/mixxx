#include "soundsourcemediafoundation.h"

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <propvarutil.h>

#include "util/sample.h"

namespace {

const SINT kBytesPerSample = sizeof(CSAMPLE);
const SINT kBitsPerSample = kBytesPerSample * 8;
const SINT kLeftoverSize = 4096; // in CSAMPLE's, this seems to be the size MF AAC

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

// Only read the first audio stream
const DWORD kStreamIndex = MF_SOURCE_READER_FIRST_AUDIO_STREAM;

/** Microsoft examples use this snippet often. */
template<class T> static void safeRelease(T **ppT) {
    if (*ppT) {
        (*ppT)->Release();
        *ppT = nullptr;
    }
}

} // anonymous namespace

namespace mixxx {

SoundSourceMediaFoundation::SoundSourceMediaFoundation(const QUrl& url)
        : SoundSourcePlugin(url, "m4a"),
          m_hrCoInitialize(E_FAIL),
          m_hrMFStartup(E_FAIL),
          m_pSourceReader(nullptr),
          m_currentFrameIndex(0),
          m_sampleBufferOffset(0),
          m_sampleBufferCount(0) {
}

SoundSourceMediaFoundation::~SoundSourceMediaFoundation() {
    close();
}

SoundSource::OpenResult SoundSourceMediaFoundation::tryOpen(const AudioSourceConfig& audioSrcCfg) {
    if (SUCCEEDED(m_hrCoInitialize)) {
        qWarning() << "Cannot reopen MediaFoundation file" << getUrlString();
        return OpenResult::FAILED;
    }

    const QString fileName(getLocalFileName());

    // Initialize the COM library.
    m_hrCoInitialize = CoInitializeEx(nullptr,
            COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(m_hrCoInitialize)) {
        qWarning() << "SSMF: failed to initialize COM";
        return OpenResult::FAILED;
    }
    // Initialize the Media Foundation platform.
    m_hrMFStartup = MFStartup(MF_VERSION);
    if (FAILED(m_hrCoInitialize)) {
        qWarning() << "SSMF: failed to initialize Media Foundation";
        return OpenResult::FAILED;
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
        qWarning() << "SSMF: Error opening input file:" << fileName;
        return OpenResult::FAILED;
    }

    if (!configureAudioStream(audioSrcCfg)) {
        qWarning() << "SSMF: Error configuring audio stream.";
        return OpenResult::FAILED;
    }

    m_streamUnitConverter = StreamUnitConverter(getSamplingRate());

    if (!readProperties()) {
        qWarning() << "SSMF::readProperties failed";
        return OpenResult::FAILED;
    }

    //Seek to position 0, which forces us to skip over all the header frames.
    //This makes sure we're ready to just let the Analyzer rip and it'll
    //get the number of samples it expects (ie. no header frames).
    seekSampleFrame(0);

    return OpenResult::SUCCEEDED;
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

SINT SoundSourceMediaFoundation::seekSampleFrame(
        SINT frameIndex) {
    DEBUG_ASSERT(isValidFrameIndex(frameIndex));

    if (m_currentFrameIndex < frameIndex) {
        // seeking forward
        SINT skipFramesCount = frameIndex - m_currentFrameIndex;
        if (skipFramesCount <= kNumberOfPrefetchFrames) {
            skipSampleFrames(skipFramesCount);
            DEBUG_ASSERT(m_currentFrameIndex == frameIndex);
        }
    }

    if (m_currentFrameIndex == frameIndex) {
        // already there
        return m_currentFrameIndex;
    }

    if (m_pSourceReader == nullptr) {
        // reader is dead -> jump to end of stream
        return getMaxFrameIndex();
    }

    HRESULT hr;

    hr = m_pSourceReader->Flush(kStreamIndex);
    if (FAILED(hr)) {
        qWarning() << "SoundSourceMediaFoundation:"
                << "Failed to flush reader before seeking"
                << hr;
    }
    // Flush left over buffer
    m_sampleBufferCount = 0;
    m_sampleBufferOffset = 0;

    // Invalidate current position
    m_currentFrameIndex = getMaxFrameIndex(); // invalidate

    // Jump to a position before the actual seeking position.
    // Prefetching a certain number of frames is necessary for
    // sample accurate decoding. The decoder needs to decode
    // some frames in advance to produce the same result at
    // each position in the stream.
    SINT seekIndex = std::max(SINT(frameIndex - kNumberOfPrefetchFrames), SINT(0));

    qint64 seekPos = m_streamUnitConverter.fromFrameIndex(seekIndex);
    DEBUG_ASSERT(seekPos >= 0);
    PROPVARIANT prop;
    hr = InitPropVariantFromInt64(seekPos, &prop);
    DEBUG_ASSERT(SUCCEEDED(hr)); // never fails
    hr = m_pSourceReader->SetCurrentPosition(GUID_NULL, prop);
    PropVariantClear(&prop);
    if (SUCCEEDED(hr)) {
        // Restart decoding by skipping over the prefetch samples
        DEBUG_ASSERT(seekIndex < frameIndex);
        SINT skipFramesCount = frameIndex - seekIndex;
        // NOTE(uklotzde): This loop is needed, because often the
        // target position has not been reached after the first
        // invocation of skipSampleFrames()!?!?
        while (skipFramesCount > 0) {
            skipSampleFrames(skipFramesCount);
            if (m_currentFrameIndex < frameIndex) {
                skipFramesCount = frameIndex - m_currentFrameIndex;
            } else {
                DEBUG_ASSERT(m_currentFrameIndex == frameIndex); // must not overshoot
                break;
            }
        }
        DEBUG_ASSERT(m_currentFrameIndex == frameIndex);
    } else {
        qWarning() << "IMFSourceReader::SetCurrentPosition() failed"
                << hr;
        safeRelease(&m_pSourceReader); // kill the reader
    }

    return m_currentFrameIndex;
}

SINT SoundSourceMediaFoundation::readSampleFrames(
        SINT numberOfFrames, CSAMPLE* sampleBuffer) {

    SINT numberOfFramesRemaining = numberOfFrames;
    CSAMPLE* pSampleBuffer = sampleBuffer;

    while (numberOfFramesRemaining > 0) {
        SINT copySamplesCount =
            std::min(frames2samples(numberOfFramesRemaining), m_sampleBufferCount);
        if (copySamplesCount > 0) {
            DEBUG_ASSERT(m_currentFrameIndex < getMaxFrameIndex());
            if (sampleBuffer != nullptr) {
                SampleUtil::copy(
                    pSampleBuffer,
                    m_sampleBuffer.data() + m_sampleBufferOffset,
                    copySamplesCount);
                pSampleBuffer += copySamplesCount;
            }
            m_sampleBufferOffset += copySamplesCount;
            m_sampleBufferCount -= copySamplesCount;
            if (m_sampleBufferCount == 0) {
                // all buffered samples have been read
                m_sampleBufferOffset = 0;
            }
            m_currentFrameIndex += samples2frames(copySamplesCount);
            numberOfFramesRemaining -= samples2frames(copySamplesCount);
        }
        if (numberOfFramesRemaining == 0) {
            break; // finished reading
        }

        // No more buffered frames available
        DEBUG_ASSERT(m_sampleBufferOffset == 0);
        DEBUG_ASSERT(m_sampleBufferCount == 0);

        if (m_pSourceReader == nullptr) {
            break; // abort (reader is dead)
        }

        DWORD dwFlags = 0;
        qint64 streamPos = 0;
        IMFSample* pSample = nullptr;
        HRESULT hr = m_pSourceReader->ReadSample(
            kStreamIndex, // [in] DWORD dwStreamIndex,
            0,                                 // [in] DWORD dwControlFlags,
            nullptr,                      // [out] DWORD *pdwActualStreamIndex,
            &dwFlags,                        // [out] DWORD *pdwStreamFlags,
            &streamPos,                     // [out] LONGLONG *pllTimestamp,
            &pSample);                         // [out] IMFSample **ppSample
        if (FAILED(hr)) {
            qWarning()
                << "IMFSourceReader::ReadSample() failed"
                << hr
                << "-> abort decoding";
            break; // abort
        }
        if (dwFlags & MF_SOURCE_READERF_ERROR) {
            DEBUG_ASSERT(pSample == nullptr);
            qWarning()
                    << "IMFSourceReader::ReadSample() detected stream errors"
                    << "(MF_SOURCE_READERF_ERROR)"
                    << "-> abort and stop decoding";
            safeRelease(&m_pSourceReader);
            break; // abort
        } else if (dwFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
            DEBUG_ASSERT(pSample == nullptr);
            break; // finished reading
        } else if (dwFlags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED) {
            DEBUG_ASSERT(pSample == nullptr);
            qWarning()
                    << "IMFSourceReader::ReadSample() detected that the media type has changed"
                    << "(MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)"
                    << "-> abort decoding";
            break; // abort
        }
        DEBUG_ASSERT(pSample != nullptr);
        SINT readerFrameIndex = m_streamUnitConverter.toFrameIndex(streamPos);
        DEBUG_ASSERT(
            (m_currentFrameIndex == getMaxFrameIndex()) ||
            (m_currentFrameIndex == readerFrameIndex));
        m_currentFrameIndex = readerFrameIndex;

        DWORD dwSampleBufferCount = 0;
        hr = pSample->GetBufferCount(&dwSampleBufferCount);
        if (FAILED(hr)) {
            qWarning()
                << "IMFSample::GetBufferCount() failed"
                << hr
                << "-> abort decoding";
            safeRelease(&pSample);
            break; // abort
        }

        DWORD dwSampleTotalLengthInBytes = 0;
        hr = pSample->GetTotalLength(&dwSampleTotalLengthInBytes);
        if (FAILED(hr)) {
            qWarning()
                << "IMFSample::GetTotalLength() failed"
                << hr
                << "-> abort decoding";
            safeRelease(&pSample);
            break; // abort
        }
        // Grow temporary buffer (if necessary)
        DEBUG_ASSERT((dwSampleTotalLengthInBytes % kBytesPerSample) == 0);
        SINT numberOfSamplesToBuffer =
            dwSampleTotalLengthInBytes / kBytesPerSample;
        SINT sampleBufferSize = m_sampleBuffer.size();
        DEBUG_ASSERT(sampleBufferSize > 0);
        while (sampleBufferSize < numberOfSamplesToBuffer) {
            sampleBufferSize *= 2;
        }
        if (m_sampleBuffer.size() < sampleBufferSize) {
            SampleBuffer(sampleBufferSize).swap(m_sampleBuffer);
        }

        DWORD dwSampleBufferIndex = 0;
        while (dwSampleBufferIndex < dwSampleBufferCount) {
            IMFMediaBuffer* pMediaBuffer = nullptr;
            hr = pSample->GetBufferByIndex(dwSampleBufferIndex, &pMediaBuffer);
            if (FAILED(hr)) {
                qWarning()
                    << "IMFSample::GetBufferByIndex() failed"
                    << hr
                    << "-> abort decoding";
                break; // prematurely exit buffer loop
            }

            CSAMPLE* pLockedSampleBuffer = nullptr;
            DWORD lockedSampleBufferLengthInBytes = 0;
            hr = pMediaBuffer->Lock(
                reinterpret_cast<quint8**>(&pLockedSampleBuffer),
                nullptr,
                &lockedSampleBufferLengthInBytes);
            if (FAILED(hr)) {
                qWarning()
                    << "IMFMediaBuffer::Lock() failed"
                    << hr
                    << "-> abort decoding";
                safeRelease(&pMediaBuffer);
                break; // prematurely exit buffer loop
            }

            DEBUG_ASSERT((lockedSampleBufferLengthInBytes % sizeof(pLockedSampleBuffer[0])) == 0);
            SINT lockedSampleBufferCount =
                lockedSampleBufferLengthInBytes / sizeof(pLockedSampleBuffer[0]);
            DEBUG_ASSERT(lockedSampleBufferCount
                <= (m_sampleBuffer.size() - m_sampleBufferOffset));
            SINT copySamplesCount =
                std::min(frames2samples(numberOfFramesRemaining), lockedSampleBufferCount);
            if (copySamplesCount > 0) {
                if (pSampleBuffer != nullptr) {
                    SampleUtil::copy(
                        pSampleBuffer,
                        pLockedSampleBuffer,
                        copySamplesCount);
                    pSampleBuffer += copySamplesCount;
                }
                pLockedSampleBuffer += copySamplesCount;
                lockedSampleBufferCount -= copySamplesCount;
                m_currentFrameIndex += samples2frames(copySamplesCount);
                numberOfFramesRemaining -= samples2frames(copySamplesCount);
            }
            // Buffer the remaining samples
            DEBUG_ASSERT(m_sampleBufferOffset == 0);
            SampleUtil::copy(
                m_sampleBuffer.data() + m_sampleBufferCount,
                pLockedSampleBuffer,
                lockedSampleBufferCount);
            m_sampleBufferCount += lockedSampleBufferCount;
            DEBUG_ASSERT_AND_HANDLE(SUCCEEDED(hr = pMediaBuffer->Unlock())) {
                qWarning()
                    << "IMFMediaBuffer::Unlock() failed"
                    << hr;
                // ignore and continue
            }
            safeRelease(&pMediaBuffer);
            ++dwSampleBufferIndex;
        }
        safeRelease(&pSample);
        if (dwSampleBufferIndex < dwSampleBufferCount) {
            // Failed to read data from all buffers -> kill the reader
            qWarning()
                << "Failed to read all buffered samples"
                << hr
                << "-> abort and stop decoding";
            safeRelease(&m_pSourceReader);
            break; // abort
        }
    }

    return numberOfFrames - numberOfFramesRemaining;
}

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
bool SoundSourceMediaFoundation::configureAudioStream(const AudioSourceConfig& audioSrcCfg) {
    HRESULT hr;

    // deselect all streams, we only want the first
    hr = m_pSourceReader->SetStreamSelection(
            MF_SOURCE_READER_ALL_STREAMS, false);
    if (FAILED(hr)) {
        qWarning() << "SSMF" << hr
                << "failed to deselect all streams";
        return false;
    }

    hr = m_pSourceReader->SetStreamSelection(
            kStreamIndex, true);
    if (FAILED(hr)) {
        qWarning() << "SSMF" << hr
                << "failed to select first audio stream";
        return false;
    }

    IMFMediaType* pAudioType = nullptr;

    hr = m_pSourceReader->GetCurrentMediaType(
            kStreamIndex, &pAudioType);
    if (FAILED(hr)) {
        qWarning() << "SSMF" << hr
                << "failed to get current media type from stream";
        return false;
    }

    hr = pAudioType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    if (FAILED(hr)) {
        qWarning() << "SSMF" << hr
                << "failed to set major type to audio";
        safeRelease(&pAudioType);
        return false;
    }

    hr = pAudioType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_Float);
    if (FAILED(hr)) {
        qWarning() << "SSMF" << hr
            << "failed to set subtype format to float";
        safeRelease(&pAudioType);
        return false;
    }

    hr = pAudioType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, true);
    if (FAILED(hr)) {
        qWarning() << "SSMF" << hr
            << "failed to set all samples independent";
        safeRelease(&pAudioType);
        return false;
    }

    hr = pAudioType->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, true);
    if (FAILED(hr)) {
        qWarning() << "SSMF" << hr
            << "failed to set fixed size samples";
        safeRelease(&pAudioType);
        return false;
    }

    hr = pAudioType->SetUINT32(
            MF_MT_AUDIO_BITS_PER_SAMPLE, kBitsPerSample);
    if (FAILED(hr)) {
        qWarning() << "SSMF" << hr
                << "failed to set bits per sample:"
                << kBitsPerSample;
        safeRelease(&pAudioType);
        return false;
    }

    const UINT sampleSize = kLeftoverSize * kBytesPerSample;
    hr = pAudioType->SetUINT32(
            MF_MT_SAMPLE_SIZE, sampleSize);
    if (FAILED(hr)) {
        qWarning() << "SSMF" << hr
                << "failed to set sample size:"
                << sampleSize;
        safeRelease(&pAudioType);
        return false;
    }

    UINT32 numChannels;
    hr = pAudioType->GetUINT32(
            MF_MT_AUDIO_NUM_CHANNELS, &numChannels);
    if (FAILED(hr)) {
        qWarning() << "SSMF" << hr
            << "failed to get actual number of channels";
        return false;
    } else {
        qDebug() << "Number of channels in input stream" << numChannels;
    }
    if (audioSrcCfg.hasValidChannelCount()) {
        numChannels = audioSrcCfg.getChannelCount();
        hr = pAudioType->SetUINT32(
                MF_MT_AUDIO_NUM_CHANNELS, numChannels);
        if (FAILED(hr)) {
            qWarning() << "SSMF" << hr
                << "failed to set number of channels:"
                << numChannels;
            safeRelease(&pAudioType);
            return false;
        }
        qDebug() << "Requested number of channels" << numChannels;
    }

    UINT32 samplesPerSecond;
    hr = pAudioType->GetUINT32(
            MF_MT_AUDIO_SAMPLES_PER_SECOND, &samplesPerSecond);
    if (FAILED(hr)) {
        qWarning() << "SSMF" << hr
            << "failed to get samples per second";
        return false;
    } else {
        qDebug() << "Samples per second in input stream" << samplesPerSecond;
    }
    if (audioSrcCfg.hasValidSamplingRate()) {
        samplesPerSecond = audioSrcCfg.getSamplingRate();
        hr = pAudioType->SetUINT32(
                MF_MT_AUDIO_SAMPLES_PER_SECOND, samplesPerSecond);
        if (FAILED(hr)) {
            qWarning() << "SSMF" << hr
                << "failed to set samples per second:"
                << samplesPerSecond;
            safeRelease(&pAudioType);
            return false;
        }
        qDebug() << "Requested samples per second" << samplesPerSecond;
    }

    // Set this type on the source reader. The source reader will
    // load the necessary decoder.
    hr = m_pSourceReader->SetCurrentMediaType(
            kStreamIndex, nullptr, pAudioType);
    if (FAILED(hr)) {
        qWarning() << "SSMF" << hr
            << "failed to set media type";
        safeRelease(&pAudioType);
        return false;
    }

    // Finally release the reference before reusing the pointer
    safeRelease(&pAudioType);

    // Get the resulting output format.
    hr = m_pSourceReader->GetCurrentMediaType(
            kStreamIndex, &pAudioType);
    if (FAILED(hr)) {
        qWarning() << "SSMF" << hr
            << "failed to retrieve completed media type";
        return false;
    }

    // Ensure the stream is selected.
    hr = m_pSourceReader->SetStreamSelection(
            kStreamIndex, true);
    if (FAILED(hr)) {
        qWarning() << "SSMF" << hr
            << "failed to select first audio stream (again)";
        return false;
    }

    hr = pAudioType->GetUINT32(
            MF_MT_AUDIO_NUM_CHANNELS, &numChannels);
    if (FAILED(hr)) {
        qWarning() << "SSMF" << hr
            << "failed to get actual number of channels";
        return false;
    }
    setChannelCount(numChannels);

    hr = pAudioType->GetUINT32(
            MF_MT_AUDIO_SAMPLES_PER_SECOND, &samplesPerSecond);
    if (FAILED(hr)) {
        qWarning() << "SSMF" << hr
            << "failed to get the actual sample rate";
        return false;
    }
    setSamplingRate(samplesPerSecond);

    UINT32 leftoverBufferSizeInBytes = 0;
    hr = pAudioType->GetUINT32(MF_MT_SAMPLE_SIZE, &leftoverBufferSizeInBytes);
    if (FAILED(hr)) {
        qWarning() << "SSMF" << hr
            << "failed to get sample buffer size (in bytes)";
        return false;
    }
    DEBUG_ASSERT((leftoverBufferSizeInBytes % kBytesPerSample) == 0);
    SampleBuffer(leftoverBufferSizeInBytes / kBytesPerSample).swap(m_sampleBuffer);
    DEBUG_ASSERT(m_sampleBuffer.size() > 0);
    qDebug() << "SoundSourceMediaFoundation: Sample buffer size" << m_sampleBuffer.size();

    // Finally release the reference
    safeRelease(&pAudioType);

    return true;
}

bool SoundSourceMediaFoundation::readProperties() {
    PROPVARIANT prop;
    HRESULT hr = S_OK;

    //Get the duration, provided as a 64-bit integer of 100-nanosecond units
    hr = m_pSourceReader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE,
        MF_PD_DURATION, &prop);
    if (FAILED(hr)) {
        qWarning() << "SSMF: error getting duration";
        return false;
    }
    setFrameCount(m_streamUnitConverter.toFrameIndex(prop.hVal.QuadPart));
    qDebug() << "SoundSourceMediaFoundation: Frame count" << getFrameCount();
    PropVariantClear(&prop);

    // presentation attribute MF_PD_AUDIO_ENCODING_BITRATE only exists for
    // presentation descriptors, one of which MFSourceReader is not.
    // Therefore, we calculate it ourselves.
    setBitrate(kBitsPerSample * frames2samples(getSamplingRate()));

    return true;
}

QString SoundSourceProviderMediaFoundation::getName() const {
    return "Microsoft Media Foundation";
}

QStringList SoundSourceProviderMediaFoundation::getSupportedFileExtensions() const {
    QStringList supportedFileExtensions;
    supportedFileExtensions.append("m4a");
    supportedFileExtensions.append("mp4");
    return supportedFileExtensions;
}

SoundSourcePointer SoundSourceProviderMediaFoundation::newSoundSource(const QUrl& url) {
    return newSoundSourcePluginFromUrl<SoundSourceMediaFoundation>(url);
}

} // namespace mixxx

extern "C" MIXXX_SOUNDSOURCEPLUGINAPI_EXPORT
mixxx::SoundSourceProvider* Mixxx_SoundSourcePluginAPI_createSoundSourceProvider() {
    // SoundSourceProviderMediaFoundation is stateless and a single instance
    // can safely be shared
    static mixxx::SoundSourceProviderMediaFoundation singleton;
    return &singleton;
}

extern "C" MIXXX_SOUNDSOURCEPLUGINAPI_EXPORT
void Mixxx_SoundSourcePluginAPI_destroySoundSourceProvider(mixxx::SoundSourceProvider*) {
    // The statically allocated instance must not be deleted!
}
