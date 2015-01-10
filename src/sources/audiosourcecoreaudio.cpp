#include "sources/audiosourcecoreaudio.h"

#include "util/math.h"

#include <QtDebug>

namespace Mixxx {

namespace
{
AudioSource::size_type kChannelCount = 2;
}

AudioSourceCoreAudio::AudioSourceCoreAudio()
        : m_headerFrames(0) {
}

AudioSourceCoreAudio::~AudioSourceCoreAudio() {
    preDestroy();
}

AudioSourcePointer AudioSourceCoreAudio::open(QString fileName) {
    AudioSourceCoreAudio* pAudioSourceCoreAudio(new AudioSourceCoreAudio);
    AudioSourcePointer pAudioSource(pAudioSourceCoreAudio); // take ownership
    if (OK == pAudioSourceCoreAudio->postConstruct(fileName)) {
        // success
        return pAudioSource;
    } else {
        // failure
        return AudioSourcePointer();
    }
}

// soundsource overrides
Result AudioSourceCoreAudio::postConstruct(QString fileName) {
    //Open the audio file.
    OSStatus err;

    /** This code blocks works with OS X 10.5+ only. DO NOT DELETE IT for now. */
    CFStringRef urlStr = CFStringCreateWithCharacters(
        0, reinterpret_cast<const UniChar *>(fileName.unicode()), fileName.size());
    CFURLRef urlRef = CFURLCreateWithFileSystemPath(NULL, urlStr, kCFURLPOSIXPathStyle, false);
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
        qDebug() << "SSCA: Error opening file " << fileName;
        return ERR;
    }

    // get the input file format
    UInt32 inputFormatSize = sizeof(m_inputFormat);
    err = ExtAudioFileGetProperty(m_audioFile, kExtAudioFileProperty_FileDataFormat, &inputFormatSize, &m_inputFormat);
    if (err != noErr) {
        qDebug() << "SSCA: Error getting file format (" << fileName << ")";
        return ERR;
    }

    // create the output format
    m_outputFormat = CAStreamBasicDescription(
        m_inputFormat.mSampleRate, kChannelCount,
        CAStreamBasicDescription::kPCMFormatFloat32, true);

    // set the client format
    err = ExtAudioFileSetProperty(m_audioFile, kExtAudioFileProperty_ClientDataFormat,
                                  sizeof(m_outputFormat), &m_outputFormat);
    if (err != noErr) {
        qDebug() << "SSCA: Error setting file property";
        return ERR;
    }

    //get the total length in frames of the audio file - copypasta: http://discussions.apple.com/thread.jspa?threadID=2364583&tstart=47
    SInt64 totalFrameCount;
    UInt32 totalFrameCountSize = sizeof(totalFrameCount);
    err = ExtAudioFileGetProperty(m_audioFile, kExtAudioFileProperty_FileLengthFrames, &totalFrameCountSize, &totalFrameCount);
    if (err != noErr) {
        qDebug() << "SSCA: Error getting number of frames";
        return ERR;
    }

    //
    // WORKAROUND for bug in ExtFileAudio
    //

    AudioConverterRef acRef;
    UInt32 acrsize = sizeof(AudioConverterRef);
    err = ExtAudioFileGetProperty(m_audioFile, kExtAudioFileProperty_AudioConverter, &acrsize, &acRef);
    //_ThrowExceptionIfErr(@"kExtAudioFileProperty_AudioConverter", err);

    AudioConverterPrimeInfo primeInfo;
    UInt32 piSize = sizeof(AudioConverterPrimeInfo);
    memset(&primeInfo, 0, piSize);
    err = AudioConverterGetProperty(acRef, kAudioConverterPrimeInfo, &piSize, &primeInfo);
    if (err != kAudioConverterErr_PropertyNotSupported) { // Only if decompressing
        //_ThrowExceptionIfErr(@"kAudioConverterPrimeInfo", err);
        m_headerFrames = primeInfo.leadingFrames;
    }

    setChannelCount(m_outputFormat.NumberChannels());
    setFrameRate(m_inputFormat.mSampleRate);
    setFrameCount(totalFrameCount/* - m_headerFrames*/);

    //Seek to position 0, which forces us to skip over all the header frames.
    //This makes sure we're ready to just let the Analyser rip and it'll
    //get the number of samples it expects (ie. no header frames).
    seekFrame(0);

    return OK;
}

void AudioSourceCoreAudio::preDestroy() {
    ExtAudioFileDispose(m_audioFile);
}

AudioSource::diff_type AudioSourceCoreAudio::seekFrame(diff_type frameIndex) {
    OSStatus err = ExtAudioFileSeek(m_audioFile, frameIndex + m_headerFrames);
    //_ThrowExceptionIfErr(@"ExtAudioFileSeek", err);
    //qDebug() << "SSCA: Seeking to" << frameIndex;
    if (err != noErr) {
        qDebug() << "SSCA: Error seeking to" << frameIndex;// << GetMacOSStatusErrorString(err) << GetMacOSStatusCommentString(err);
    }
    return frameIndex;
}

AudioSource::size_type AudioSourceCoreAudio::readFrameSamplesInterleaved(size_type frameCount,
        sample_type* sampleBuffer) {
    //if (!m_decoder) return 0;
    size_type numFramesRead = 0;

    while (numFramesRead < frameCount) {
        size_type numFramesToRead = frameCount - numFramesRead;

        AudioBufferList fillBufList;
        fillBufList.mNumberBuffers = 1;
        fillBufList.mBuffers[0].mNumberChannels = getChannelCount();
        fillBufList.mBuffers[0].mDataByteSize = frames2samples(numFramesToRead) * sizeof(sampleBuffer[0]);
        fillBufList.mBuffers[0].mData = sampleBuffer + frames2samples(numFramesRead);

        UInt32 numFramesToReadInOut = numFramesToRead; // input/output parameter
        OSStatus err = ExtAudioFileRead(m_audioFile, &numFramesToReadInOut, &fillBufList);
        if (0 == numFramesToReadInOut) {
            // EOF
            break; // done
        }
        numFramesRead += numFramesToReadInOut;
    }
    return numFramesRead;
}

} // namespace Mixxx
