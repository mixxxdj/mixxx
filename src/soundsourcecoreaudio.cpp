/**
 * \file soundsource.cpp
 * \author Albert Santoni <alberts at mixxx dot org>
 * \date Dec 12, 2010
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "soundsourcecoreaudio.h"

#include "trackmetadatataglib.h"

#include <taglib/mpegfile.h>
#include <taglib/mp4file.h>

#include <QtDebug>

namespace
{
    Mixxx::AudioSource::size_type kChannelCount = 2;
}

QList<QString> SoundSourceCoreAudio::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("m4a");
    list.push_back("mp3");
    list.push_back("mp2");
    //Can add mp3, mp2, ac3, and others here if you want.
    //See:
    //  http://developer.apple.com/library/mac/documentation/MusicAudio/Reference/AudioFileConvertRef/Reference/reference.html#//apple_ref/doc/c_ref/AudioFileTypeID

    //XXX: ... but make sure you implement handling for any new format in ParseHeader!!!!!! -- asantoni
    return list;
}

SoundSourceCoreAudio::SoundSourceCoreAudio(QString filename)
        : Super(filename)
        , m_headerFrames(0) {
}

SoundSourceCoreAudio::~SoundSourceCoreAudio() {
    ExtAudioFileDispose(m_audioFile);
}

// soundsource overrides
Result SoundSourceCoreAudio::open() {
    //Open the audio file.
    OSStatus err;

    /** This code blocks works with OS X 10.5+ only. DO NOT DELETE IT for now. */
    CFStringRef urlStr = CFStringCreateWithCharacters(
        0, reinterpret_cast<const UniChar *>(getFilename().unicode()), getFilename().size());
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
        qDebug() << "SSCA: Error opening file " << getFilename();
        return ERR;
    }

    // get the input file format
    UInt32 inputFormatSize = sizeof(m_inputFormat);
    err = ExtAudioFileGetProperty(m_audioFile, kExtAudioFileProperty_FileDataFormat, &inputFormatSize, &m_inputFormat);
    if (err != noErr) {
        qDebug() << "SSCA: Error getting file format (" << getFilename() << ")";
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

Mixxx::AudioSource::diff_type SoundSourceCoreAudio::seekFrame(diff_type frameIndex) {
    OSStatus err = ExtAudioFileSeek(m_audioFile, frameIndex + m_headerFrames);
    //_ThrowExceptionIfErr(@"ExtAudioFileSeek", err);
    //qDebug() << "SSCA: Seeking to" << frameIndex;
    if (err != noErr) {
        qDebug() << "SSCA: Error seeking to" << frameIndex << " (file " << getFilename() << ")";// << GetMacOSStatusErrorString(err) << GetMacOSStatusCommentString(err);
    }
    return frameIndex;
}

Mixxx::AudioSource::size_type SoundSourceCoreAudio::readFrameSamplesInterleaved(size_type frameCount,
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
            // EOF reached
            break;
        }
        numFramesRead += numFramesToReadInOut;
    }
    return numFramesRead;
}

Result SoundSourceCoreAudio::parseMetadata(Mixxx::TrackMetadata* pMetadata) {
    if (getType() == "m4a") {
        TagLib::MP4::File f(getFilename().toLocal8Bit().constData());
        if (!readAudioProperties(pMetadata, f)) {
            return ERR;
        }
        TagLib::MP4::Tag *mp4(f.tag());
        if (mp4) {
            readMP4Tag(pMetadata, *mp4);
        } else {
            // fallback
            const TagLib::Tag *tag(f.tag());
            if (tag) {
                readTag(pMetadata, *tag);
            } else {
                return ERR;
            }
        }
    } else if (getType() == "mp3") {
        TagLib::MPEG::File f(getFilename().toLocal8Bit().constData());
        if (!readAudioProperties(pMetadata, f)) {
            return ERR;
        }
        TagLib::ID3v2::Tag* id3v2 = f.ID3v2Tag();
        if (id3v2) {
            readID3v2Tag(pMetadata, *id3v2);
        } else {
            TagLib::APE::Tag *ape = f.APETag();
            if (ape) {
                readAPETag(pMetadata, *ape);
            } else {
                // fallback
                const TagLib::Tag *tag(f.tag());
                if (tag) {
                    readTag(pMetadata, *tag);
                } else {
                    return ERR;
                }
            }
        }
    } else if (getType() == "mp2") {
        //TODO: MP2 metadata. Does anyone use mp2 files anymore?
        //      Feels like 1995 again...
        return ERR;
    }

    return OK;
}

QImage SoundSourceCoreAudio::parseCoverArt() {
    QImage coverArt;
    if (getType() == "m4a") {
        TagLib::MP4::File f(getFilename().toLocal8Bit().constData());
        TagLib::MP4::Tag *mp4(f.tag());
        if (mp4) {
            return Mixxx::readMP4TagCover(*mp4);
        } else {
            return QImage();
        }
    } else if (getType() == "mp3") {
        TagLib::MPEG::File f(getFilename().toLocal8Bit().constData());
        TagLib::ID3v2::Tag* id3v2 = f.ID3v2Tag();
        if (id3v2) {
            coverArt = Mixxx::readID3v2TagCover(*id3v2);
        }
        if (coverArt.isNull()) {
            TagLib::APE::Tag *ape = f.APETag();
            if (ape) {
                coverArt = Mixxx::readAPETagCover(*ape);
            }
        }
        return coverArt;
    }
    return coverArt;
}
