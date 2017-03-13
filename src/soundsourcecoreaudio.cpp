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

#include <QtDebug>
#include <taglib/mpegfile.h>
#include <taglib/mp4file.h>

#include "soundsourcecoreaudio.h"
#include "soundsourcetaglib.h"
#include "util/math.h"

SoundSourceCoreAudio::SoundSourceCoreAudio(QString filename)
        : SoundSource(filename),
          m_samples(0),
          m_headerFrames(0) {
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
    CAStreamBasicDescription inputFormat;
    UInt32 size = sizeof(inputFormat);
    m_inputFormat = inputFormat;
    err = ExtAudioFileGetProperty(m_audioFile, kExtAudioFileProperty_FileDataFormat, &size, &inputFormat);
    if (err != noErr) {
        qDebug() << "SSCA: Error getting file format (" << getFilename() << ")";
        return ERR;
    }

    //Debugging:
    //printf ("Source File format: "); inputFormat.Print();
    //printf ("Dest File format: "); outputFormat.Print();

    // create the output format
    m_outputFormat = CAStreamBasicDescription(
        inputFormat.mSampleRate, 2,
        CAStreamBasicDescription::kPCMFormatInt16, true);

    // set the client format
    err = ExtAudioFileSetProperty(m_audioFile, kExtAudioFileProperty_ClientDataFormat,
                                  sizeof(m_outputFormat), &m_outputFormat);
    if (err != noErr) {
        qDebug() << "SSCA: Error setting file property";
        return ERR;
    }

    setChannels(m_outputFormat.NumberChannels());

    //get the total length in frames of the audio file - copypasta: http://discussions.apple.com/thread.jspa?threadID=2364583&tstart=47
    UInt32        dataSize;
    SInt64        totalFrameCount;
    dataSize    = sizeof(totalFrameCount); //XXX: This looks sketchy to me - Albert
    err            = ExtAudioFileGetProperty(m_audioFile, kExtAudioFileProperty_FileLengthFrames, &dataSize, &totalFrameCount);
    if (err != noErr) {
        qDebug() << "SSCA: Error getting number of frames";
        return ERR;
    }

    //
    // WORKAROUND for bug in ExtFileAudio
    //

    AudioConverterRef acRef;
    UInt32 acrsize=sizeof(AudioConverterRef);
    err = ExtAudioFileGetProperty(m_audioFile, kExtAudioFileProperty_AudioConverter, &acrsize, &acRef);
    //_ThrowExceptionIfErr(@"kExtAudioFileProperty_AudioConverter", err);

    AudioConverterPrimeInfo primeInfo;
    UInt32 piSize=sizeof(AudioConverterPrimeInfo);
    memset(&primeInfo, 0, piSize);
    err = AudioConverterGetProperty(acRef, kAudioConverterPrimeInfo, &piSize, &primeInfo);
    if (err != kAudioConverterErr_PropertyNotSupported) { // Only if decompressing
        //_ThrowExceptionIfErr(@"kAudioConverterPrimeInfo", err);
        m_headerFrames=primeInfo.leadingFrames;
    }

    m_samples = (totalFrameCount/* - m_headerFrames*/) * getChannels();
    setDuration(m_samples / (inputFormat.mSampleRate * getChannels()));
    setSampleRate(inputFormat.mSampleRate);
    qDebug() << m_samples << totalFrameCount << getChannels();

    //Seek to position 0, which forces us to skip over all the header frames.
    //This makes sure we're ready to just let the Analyser rip and it'll
    //get the number of samples it expects (ie. no header frames).
    seek(0);

    return OK;
}

long SoundSourceCoreAudio::seek(long filepos) {
    // important division here, filepos is in audio samples (i.e. shorts)
    // but libflac expects a number in time samples. I _think_ this should
    // be hard-coded at two because *2 is the assumption the caller makes
    // -- bkgood
    OSStatus err = noErr;
    SInt64 segmentStart = filepos / 2;

    err = ExtAudioFileSeek(m_audioFile, (SInt64)segmentStart+m_headerFrames);
    //_ThrowExceptionIfErr(@"ExtAudioFileSeek", err);
    //qDebug() << "SSCA: Seeking to" << segmentStart;

    //err = ExtAudioFileSeek(m_audioFile, filepos / 2);
    if (err != noErr) {
        qDebug() << "SSCA: Error seeking to" << filepos << " (file " << getFilename() << ")";// << GetMacOSStatusErrorString(err) << GetMacOSStatusCommentString(err);
    }
    return filepos;
}

unsigned int SoundSourceCoreAudio::read(unsigned long size, const SAMPLE *destination) {
    //if (!m_decoder) return 0;
    OSStatus err;
    SAMPLE *destBuffer(const_cast<SAMPLE*>(destination));
    UInt32 numFrames = 0;//(size / 2); /// m_outputFormat.mBytesPerFrame);
    unsigned int totalFramesToRead = size/2;
    unsigned int numFramesRead = 0;
    unsigned int numFramesToRead = totalFramesToRead;

    while (numFramesRead < totalFramesToRead) { //FIXME: Hardcoded 2
        numFramesToRead = totalFramesToRead - numFramesRead;

        AudioBufferList fillBufList;
        fillBufList.mNumberBuffers = 1; //Decode a single track?
        fillBufList.mBuffers[0].mNumberChannels = m_outputFormat.mChannelsPerFrame;
        fillBufList.mBuffers[0].mDataByteSize = math_min<unsigned int>(1024, numFramesToRead*4);//numFramesToRead*sizeof(*destBuffer); // 2 = num bytes per SAMPLE
        fillBufList.mBuffers[0].mData = (void*)(&destBuffer[numFramesRead*2]);

        // client format is always linear PCM - so here we determine how many frames of lpcm
        // we can read/write given our buffer size
        numFrames = numFramesToRead; //This silly variable acts as both a parameter and return value.
        err = ExtAudioFileRead (m_audioFile, &numFrames, &fillBufList);
        //The actual number of frames read also comes back in numFrames.
        //(It's both a parameter to a function and a return value. wat apple?)
        //XThrowIfError (err, "ExtAudioFileRead");
        if (!numFrames) {
            // this is our termination condition
            break;
        }
        numFramesRead += numFrames;
    }
    return numFramesRead*2;
}

inline unsigned long SoundSourceCoreAudio::length() {
    return m_samples;
}

Result SoundSourceCoreAudio::parseHeader() {
    if (getFilename().endsWith(".m4a")) {
        setType("m4a");
        TagLib::MP4::File f(getFilename().toLocal8Bit().constData());
        if (!readFileHeader(this, f)) {
            return ERR;
        }
        TagLib::MP4::Tag *mp4(f.tag());
        if (mp4) {
            readMP4Tag(this, *mp4);
        } else {
            // fallback
            const TagLib::Tag *tag(f.tag());
            if (tag) {
                readTag(this, *tag);
            } else {
                return ERR;
            }
        }
    } else if (getFilename().endsWith(".mp3")) {
        setType("mp3");
        TagLib::MPEG::File f(getFilename().toLocal8Bit().constData());
        if (!readFileHeader(this, f)) {
            return ERR;
        }
        TagLib::ID3v2::Tag* id3v2 = f.ID3v2Tag();
        if (id3v2) {
            readID3v2Tag(this, *id3v2);
        } else {
            TagLib::APE::Tag *ape = f.APETag();
            if (ape) {
                readAPETag(this, *ape);
            } else {
                // fallback
                const TagLib::Tag *tag(f.tag());
                if (tag) {
                    readTag(this, *tag);
                } else {
                    return ERR;
                }
            }
        }
    } else if (getFilename().endsWith(".mp2")) {
        setType("mp2");
        //TODO: MP2 metadata. Does anyone use mp2 files anymore?
        //      Feels like 1995 again...
        return ERR;
    }

    return OK;
}

QImage SoundSourceCoreAudio::parseCoverArt() {
    QImage coverArt;
    if (getFilename().endsWith(".m4a")) {
        setType("m4a");
        TagLib::MP4::File f(getFilename().toLocal8Bit().constData());
        TagLib::MP4::Tag *mp4(f.tag());
        if (mp4) {
            return Mixxx::getCoverInMP4Tag(*mp4);
        } else {
            return QImage();
        }
    } else if (getFilename().endsWith(".mp3")) {
        setType("mp3");
        TagLib::MPEG::File f(getFilename().toLocal8Bit().constData());
        TagLib::ID3v2::Tag* id3v2 = f.ID3v2Tag();
        if (id3v2) {
            coverArt = Mixxx::getCoverInID3v2Tag(*id3v2);
        }
        if (coverArt.isNull()) {
            TagLib::APE::Tag *ape = f.APETag();
            if (ape) {
                coverArt = Mixxx::getCoverInAPETag(*ape);
            }
        }
        return coverArt;
    }
    return coverArt;
}

// static
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
