#include "sources/soundsourcecoreaudio.h"
#include "sources/mp3decoding.h"

#include "engine/engine.h"
#include "util/logger.h"
#include "util/math.h"

namespace mixxx {

namespace {

const Logger kLogger("SoundSourceCoreAudio");

// The maximum number of samples per MP3 frame
constexpr SINT kMp3MaxFrameSize = 1152;

// NOTE(rryan): For every MP3 seek we jump back kMp3MaxSeekPrefetchFrames frames from
// the seek position and read forward to allow the decoder to stabilize. The
// cover-test.mp3 file needs this otherwise SoundSourceProxyTest.seekForward
// fails. I can't find any good documentation on how to figure out the
// appropriate amount to pre-fetch from the ExtAudioFile API. Oddly, the "prime"
// information -- which AIUI is supposed to tell us this information -- is zero
// for this file. We use the same frame pre-fetch count from SoundSourceMp3.
constexpr SINT kMp3MaxSeekPrefetchFrames =
        kMp3SeekFramePrefetchCount * kMp3MaxFrameSize;

} // namespace

//static
const QString SoundSourceProviderCoreAudio::kDisplayName = QStringLiteral("Apple Core Audio");

//static
const QStringList SoundSourceProviderCoreAudio::kSupportedFileExtensions = {
        QStringLiteral("aac"),
        QStringLiteral("m4a"),
        QStringLiteral("mp4"),
        QStringLiteral("mp3"),
        QStringLiteral("mp2"),
        // Can add mp3, mp2, ac3, and others here if you want:
        // http://developer.apple.com/library/mac/documentation/MusicAudio/Reference/AudioFileConvertRef/Reference/reference.html#//apple_ref/doc/c_ref/AudioFileTypeID
};

SoundSourceProviderPriority SoundSourceProviderCoreAudio::getPriorityHint(
        const QString& supportedFileExtension) const {
    Q_UNUSED(supportedFileExtension)
    // On macOS SoundSourceCoreAudio is the preferred decoder for all
    // supported audio formats.
    return SoundSourceProviderPriority::Higher;
}

SoundSourceCoreAudio::SoundSourceCoreAudio(QUrl url)
        : SoundSource(url),
          LegacyAudioSourceAdapter(this, this),
          m_bFileIsMp3(false),
          m_leadingFrames(0),
          m_seekPrefetchFrames(0) {
}

SoundSourceCoreAudio::~SoundSourceCoreAudio() {
    close();
}

// soundsource overrides
SoundSource::OpenResult SoundSourceCoreAudio::tryOpen(
        OpenMode /*mode*/,
        const OpenParams& params) {
    const QString fileName(getLocalFileName());

    //Open the audio file.
    OSStatus err;

    /** This code blocks works with OS X 10.5+ only. DO NOT DELETE IT for now. */
    CFStringRef urlStr = CFStringCreateWithCharacters(0,
            reinterpret_cast<const UniChar*>(fileName.unicode()),
            fileName.size());
    CFURLRef urlRef = CFURLCreateWithFileSystemPath(nullptr, urlStr, kCFURLPOSIXPathStyle, false);
    err = ExtAudioFileOpenURL(urlRef, &m_audioFile);
    CFRelease(urlStr);
    CFRelease(urlRef);

    /** TODO: Use FSRef for compatibility with 10.4 Tiger.
     Note that ExtAudioFileOpen() is deprecated above Tiger, so we must maintain
     both code paths if someone finishes this part of the code.
     FSRef fsRef;
     CFURLGetFSRef(reinterpret_cast<CFURLRef>(url.get()), &fsRef);
     err = ExtAudioFileOpen(&fsRef, &m_audioFile);
     */

    if (err != noErr) {
        kLogger.warning()
                << "Failed to open file"
                << fileName
                << err;
        return OpenResult::Failed;
    }

    // get the input file format
    UInt32 inputFormatSize = sizeof(m_inputFormat);
    err = ExtAudioFileGetProperty(m_audioFile,
            kExtAudioFileProperty_FileDataFormat,
            &inputFormatSize,
            &m_inputFormat);
    if (err != noErr) {
        kLogger.warning()
                << "Failed to determine file format"
                << fileName
                << err;
        return OpenResult::Aborted;
    }
    m_bFileIsMp3 = m_inputFormat.mFormatID == kAudioFormatMPEGLayer3;

    // create the output format
    const UInt32 numChannels =
            params.getSignalInfo().getChannelCount().isValid() ?
            params.getSignalInfo().getChannelCount() :
            mixxx::kEngineChannelCount;
    m_outputFormat = CAStreamBasicDescription(m_inputFormat.mSampleRate,
            numChannels,
            CAStreamBasicDescription::kPCMFormatFloat32,
            true);

    // set the client format
    err = ExtAudioFileSetProperty(m_audioFile,
            kExtAudioFileProperty_ClientDataFormat,
            sizeof(m_outputFormat),
            &m_outputFormat);
    if (err != noErr) {
        kLogger.warning()
                << "Failed to set output format"
                << fileName
                << err;
        return OpenResult::Failed;
    }

    //get the total length in frames of the audio file - copypasta: http://discussions.apple.com/thread.jspa?threadID=2364583&tstart=47
    SInt64 totalFrameCount;
    UInt32 totalFrameCountSize = sizeof(totalFrameCount);
    err = ExtAudioFileGetProperty(m_audioFile,
            kExtAudioFileProperty_FileLengthFrames,
            &totalFrameCountSize,
            &totalFrameCount);
    if (err != noErr) {
        kLogger.warning()
                << "Failed to read file length in sample frames"
                << fileName
                << err;
        return OpenResult::Failed;
    }

    AudioConverterRef acRef;
    UInt32 acrsize = sizeof(AudioConverterRef);
    err = ExtAudioFileGetProperty(m_audioFile,
            kExtAudioFileProperty_AudioConverter,
            &acrsize,
            &acRef);
    VERIFY_OR_DEBUG_ASSERT(err == noErr) {
        kLogger.warning()
                << "Failed to obtain AudioConverterRef"
                << fileName
                << err;
        return OpenResult::Failed;
    }

    AudioConverterPrimeInfo primeInfo;
    UInt32 piSize = sizeof(AudioConverterPrimeInfo);
    memset(&primeInfo, 0, piSize);
    err = AudioConverterGetProperty(acRef, kAudioConverterPrimeInfo, &piSize, &primeInfo);
    switch (err) {
    case noErr:
        VERIFY_OR_DEBUG_ASSERT(primeInfo.trailingFrames == 0) {
            kLogger.warning()
                    << "Unsupported audio converter property: trailingFrames ="
                    << primeInfo.trailingFrames;
        }
        // See also: https://developer.apple.com/documentation/audiotoolbox/audioconverterprimeinfo/1501803-leadingframes
        m_leadingFrames = primeInfo.leadingFrames;
        break;
    case kAudioConverterErr_PropertyNotSupported:
        break;
    default:
        kLogger.warning()
                << "Failed to get number of leading/trailing frames"
                << fileName
                << err;
        return OpenResult::Failed;
    }

    initChannelCountOnce(m_outputFormat.NumberChannels());
    DEBUG_ASSERT(std::round(m_inputFormat.mSampleRate) == m_inputFormat.mSampleRate);
    initSampleRateOnce(static_cast<SINT>(m_inputFormat.mSampleRate));
    // TODO(XXX): Reduce totalFrameCount by m_leadingFrames???
    initFrameIndexRangeOnce(IndexRange::forward(m_leadingFrames, totalFrameCount));

    if (m_bFileIsMp3) {
        // Use the maximum value for MP3 files to ensure that all decoded samples
        // are accurate. Otherwise the decoding tests for MP3 files fail!
        m_seekPrefetchFrames = math_max(m_leadingFrames, kMp3MaxSeekPrefetchFrames);
    } else {
        m_seekPrefetchFrames = m_leadingFrames;
    }
    m_seekPrefetchBuffer.resize(getSignalInfo().frames2samples(m_seekPrefetchFrames));

    // Seek to the first position, skipping over all header frames
    seekSampleFrame(frameIndexMin());

    return OpenResult::Succeeded;
} // namespace mixxx

void SoundSourceCoreAudio::close() {
    ExtAudioFileDispose(m_audioFile);
}

SINT SoundSourceCoreAudio::seekSampleFrame(SINT frameIndex) {
    DEBUG_ASSERT(isValidFrameIndex(frameIndex));

    // Prefetch frames for sample-accurate decoding
    const SINT prefetchFrames = math_min(frameIndex, m_seekPrefetchFrames);
    OSStatus err = ExtAudioFileSeek(m_audioFile, frameIndex - prefetchFrames);
    if (err != noErr) {
        kLogger.warning()
                << "Seeking to frame position"
                << frameIndex
                << "failed"
                << err;
    }
    // Decode and discard prefetched frames
    if (prefetchFrames > 0) {
        DEBUG_ASSERT(getSignalInfo().frames2samples(prefetchFrames) <= SINT(m_seekPrefetchBuffer.size()));
        const auto prefetchedFrames = readSampleFrames(prefetchFrames, m_seekPrefetchBuffer.data());
        DEBUG_ASSERT(prefetchedFrames <= prefetchFrames);
        if (prefetchedFrames < prefetchFrames) {
            kLogger.warning()
                << "Failed to skip prefetched frames while seeking:"
                << prefetchedFrames
                << "<"
                << prefetchFrames;
            // Adjust the frame index to reflect the current position
            frameIndex -= prefetchFrames - prefetchedFrames;
        }
    }
    return frameIndex;
}

SINT SoundSourceCoreAudio::readSampleFrames(
        SINT numberOfFrames, CSAMPLE* sampleBuffer) {
    DEBUG_ASSERT(numberOfFrames >= 0);
    if (numberOfFrames <= 0) {
        return 0;
    }

    // Handle special case: Skipping instead of reading
    if (!sampleBuffer) {
        SInt64 frameOffset = 0;
        const OSStatus err = ExtAudioFileTell(m_audioFile, &frameOffset);
        if (err != noErr) {
            kLogger.warning()
                    << "Failed to determine the current position for skipping"
                    << numberOfFrames
                    << "sample frames"
                    << err;
            return 0; // abort
        }
        const SINT frameIndexBefore = frameIndexMin() + frameOffset;
        const SINT frameIndexAfter = seekSampleFrame(frameIndexBefore + numberOfFrames);
        DEBUG_ASSERT(frameIndexBefore <= frameIndexAfter);
        return frameIndexAfter - frameIndexBefore;
    }

    SINT numFramesRead = 0;
    while (numFramesRead < numberOfFrames) {
        SINT numFramesToRead = numberOfFrames - numFramesRead;

        AudioBufferList fillBufList;
        fillBufList.mNumberBuffers = 1;
        fillBufList.mBuffers[0].mNumberChannels = getSignalInfo().getChannelCount();
        fillBufList.mBuffers[0].mDataByteSize = getSignalInfo().frames2samples(numFramesToRead) * sizeof(sampleBuffer[0]);
        fillBufList.mBuffers[0].mData = sampleBuffer + getSignalInfo().frames2samples(numFramesRead);

        UInt32 numFramesToReadInOut = numFramesToRead; // input/output parameter
        OSStatus err = ExtAudioFileRead(m_audioFile, &numFramesToReadInOut, &fillBufList);
        // TODO(uklotz): Should this be handled?
        Q_UNUSED(err);
        if (0 == numFramesToReadInOut) {
            // EOF
            break; // done
        }
        numFramesRead += numFramesToReadInOut;
    }
    return numFramesRead;
}

} // namespace mixxx
