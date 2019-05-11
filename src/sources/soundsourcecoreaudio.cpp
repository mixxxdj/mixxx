#include "sources/soundsourcecoreaudio.h"
#include "sources/mp3decoding.h"

#include "util/logger.h"
#include "util/math.h"

namespace mixxx {

namespace {

const Logger kLogger("SoundSourceCoreAudio");

// The maximum number of samples per MP3 frame
const SINT kMp3MaxFrameSize = 1152;

// NOTE(rryan): For every MP3 seek we jump back kStabilizationFrames frames from
// the seek position and read forward to allow the decoder to stabilize. The
// cover-test.mp3 file needs this otherwise SoundSourceProxyTest.seekForward
// fails. I can't find any good documentation on how to figure out the
// appropriate amount to pre-fetch from the ExtAudioFile API. Oddly, the "prime"
// information -- which AIUI is supposed to tell us this information -- is zero
// for this file. We use the same frame pre-fetch count from SoundSourceMp3.
const SINT kMp3StabilizationFrames =
        kMp3SeekFramePrefetchCount * kMp3MaxFrameSize;

static CSAMPLE kMp3StabilizationScratchBuffer[kMp3StabilizationFrames * 2]; // stereo

} // namespace

SoundSourceCoreAudio::SoundSourceCoreAudio(QUrl url)
        : SoundSource(url),
          LegacyAudioSourceAdapter(this, this),
          m_bFileIsMp3(false),
          m_headerFrames(0) {
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
        kLogger.debug() << "Error opening file " << fileName;
        return OpenResult::Failed;
    }

    // get the input file format
    UInt32 inputFormatSize = sizeof(m_inputFormat);
    err = ExtAudioFileGetProperty(m_audioFile,
            kExtAudioFileProperty_FileDataFormat,
            &inputFormatSize,
            &m_inputFormat);
    if (err != noErr) {
        kLogger.debug() << "Error getting file format (" << fileName << ")";
        return OpenResult::Aborted;
    }
    m_bFileIsMp3 = m_inputFormat.mFormatID == kAudioFormatMPEGLayer3;

    // create the output format
    const UInt32 numChannels =
            params.channelCount().valid() ? params.channelCount() : 2;
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
        kLogger.debug() << "Error setting file property";
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
        kLogger.debug() << "Error getting number of frames";
        return OpenResult::Failed;
    }

    //
    // WORKAROUND for bug in ExtFileAudio
    //

    AudioConverterRef acRef;
    UInt32 acrsize = sizeof(AudioConverterRef);
    err = ExtAudioFileGetProperty(m_audioFile,
            kExtAudioFileProperty_AudioConverter,
            &acrsize,
            &acRef);
    //_ThrowExceptionIfErr(@"kExtAudioFileProperty_AudioConverter", err);

    AudioConverterPrimeInfo primeInfo;
    UInt32 piSize = sizeof(AudioConverterPrimeInfo);
    memset(&primeInfo, 0, piSize);
    err = AudioConverterGetProperty(acRef, kAudioConverterPrimeInfo, &piSize, &primeInfo);
    if (err != kAudioConverterErr_PropertyNotSupported) { // Only if decompressing
        //_ThrowExceptionIfErr(@"kAudioConverterPrimeInfo", err);
        m_headerFrames = primeInfo.leadingFrames;
    } else {
        m_headerFrames = 0;
    }

    setChannelCount(m_outputFormat.NumberChannels());
    setSampleRate(m_inputFormat.mSampleRate);
    // NOTE(uklotzde): This is what I found when migrating
    // the code from SoundSource (sample-oriented) to the new
    // AudioSource (frame-oriented) API. It is not documented
    // when m_headerFrames > 0 and what the consequences are.
    initFrameIndexRangeOnce(IndexRange::forward(0 /*m_headerFrames*/, totalFrameCount));

    //Seek to first position, which forces us to skip over all the header frames.
    //This makes sure we're ready to just let the Analyzer rip and it'll
    //get the number of samples it expects (ie. no header frames).
    seekSampleFrame(frameIndexMin());

    return OpenResult::Succeeded;
}

void SoundSourceCoreAudio::close() {
    ExtAudioFileDispose(m_audioFile);
}

SINT SoundSourceCoreAudio::seekSampleFrame(SINT frameIndex) {
    DEBUG_ASSERT(isValidFrameIndex(frameIndex));

    // See comments above on kMp3StabilizationFrames.
    const SINT stabilization_frames = m_bFileIsMp3 ? math_min(
                                                             kMp3StabilizationFrames, SINT(frameIndex + m_headerFrames))
                                                   : 0;
    OSStatus err = ExtAudioFileSeek(
            m_audioFile, frameIndex + m_headerFrames - stabilization_frames);
    if (stabilization_frames > 0) {
        readSampleFrames(stabilization_frames,
                &kMp3StabilizationScratchBuffer[0]);
    }

    //_ThrowExceptionIfErr(@"ExtAudioFileSeek", err);
    //kLogger.debug() << "Seeking to" << frameIndex;
    if (err != noErr) {
        kLogger.debug() << "Error seeking to" << frameIndex; // << GetMacOSStatusErrorString(err) << GetMacOSStatusCommentString(err);
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
    if (sampleBuffer == nullptr) {
        SInt64 frameOffset = 0;
        const OSStatus osErr = ExtAudioFileTell(m_audioFile, &frameOffset);
        if (osErr == noErr) {
            const SINT frameIndexBefore = frameIndexMin() + frameOffset;
            const SINT frameIndexAfter = seekSampleFrame(frameIndexBefore + numberOfFrames);
            DEBUG_ASSERT(frameIndexBefore <= frameIndexAfter);
            return frameIndexAfter - frameIndexBefore;
        } else {
            kLogger.warning() << "Error to determine the current position for skipping sample frames" << osErr;
            return 0; // abort
        }
    }

    SINT numFramesRead = 0;
    while (numFramesRead < numberOfFrames) {
        SINT numFramesToRead = numberOfFrames - numFramesRead;

        AudioBufferList fillBufList;
        fillBufList.mNumberBuffers = 1;
        fillBufList.mBuffers[0].mNumberChannels = channelCount();
        fillBufList.mBuffers[0].mDataByteSize = frames2samples(numFramesToRead) * sizeof(sampleBuffer[0]);
        fillBufList.mBuffers[0].mData = sampleBuffer + frames2samples(numFramesRead);

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

QString SoundSourceProviderCoreAudio::getName() const {
    return "Apple Core Audio";
}

QStringList SoundSourceProviderCoreAudio::getSupportedFileExtensions() const {
    QStringList supportedFileExtensions;
    supportedFileExtensions.append("m4a");
    supportedFileExtensions.append("mp3");
    supportedFileExtensions.append("mp2");
    //Can add mp3, mp2, ac3, and others here if you want.
    //See:
    //  http://developer.apple.com/library/mac/documentation/MusicAudio/Reference/AudioFileConvertRef/Reference/reference.html#//apple_ref/doc/c_ref/AudioFileTypeID

    //XXX: ... but make sure you implement handling for any new format in ParseHeader!!!!!! -- asantoni
    return supportedFileExtensions;
}

} // namespace mixxx
