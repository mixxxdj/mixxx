/***************************************************************************
                          soundsourcem4a.cpp  -  mp4/m4a decoder
                             -------------------
    copyright            : (C) 2008 by Garth Dahlstrom
    email                : ironstorm@users.sf.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "soundsourcem4a.h"
#include "soundsourcetaglib.h"
#include "sampleutil.h"

#include <taglib/mp4file.h>

#include <neaacdec.h>

#ifdef __MP4V2__
    #include <mp4v2/mp4v2.h>
#else
    #include <mp4.h>
#endif

#ifdef __WINDOWS__
#include <io.h>
#include <fcntl.h>
#endif

#include "m4a/mp4-mixxx.cpp"

namespace Mixxx {

SoundSourceM4A::SoundSourceM4A(QString qFileName)
    : SoundSource(qFileName)
    , trackId(0) {
    setType("m4a");
    mp4_init(&ipd);
}

SoundSourceM4A::~SoundSourceM4A() {
    closeThis();
}

Result SoundSourceM4A::open()
{
    //Initialize the FAAD2 decoder...
    return initializeDecoder();
}

void SoundSourceM4A::closeThis() {
    delete[] ipd.filename;
    mp4_close(&ipd);
}

void SoundSourceM4A::close() {
    closeThis();
    Super::close();
}

Result SoundSourceM4A::initializeDecoder()
{
    // Copy QString to char[] buffer for mp4_open to read from later
    const QByteArray qbaFileName(getFilename().toLocal8Bit());
    int bytes = qbaFileName.length() + 1;
    ipd.filename = new char[bytes];
    strncpy(ipd.filename, qbaFileName.constData(), bytes);
    ipd.filename[bytes-1] = '\0';
    ipd.remote = false; // File is not an stream
    // The file was loading and failing erratically because
    // ipd.remote was an in an uninitialized state, it needed to be
    // set to false.

    int mp4_open_status = mp4_open(&ipd);
    if (mp4_open_status != 0) {
        qWarning() << "SSM4A::initializeDecoder failed"
                 << getFilename() << " with status:" << mp4_open_status;
        return ERR;
    }

    // mp4_open succeeded -> populate variables
    int channels = mp4_channels(&ipd);
    if (2 < channels) {
        qWarning() << "SSM4A::initializeDecoder failed"
                 << getFilename() << "unsupported number of channels" << channels;
        return ERR;
    }

    setChannelCount(channels);
    setSampleRate(mp4_sample_rate(&ipd));
    setFrameCount(mp4_total_frames(&ipd));
    setDuration(mp4_duration(&ipd));

    qDebug() << "#channels" << getChannelCount()
            << "samples rate" << getSampleRate()
            << "#frames" << getFrameCount()
            << "duration" << getDuration();

    return OK;
}

AudioSource::diff_type SoundSourceM4A::seekFrame(diff_type frameIndex) {
    return mp4_seek_frame(&ipd, frameIndex);
}

AudioSource::size_type SoundSourceM4A::readFrameSamplesInterleaved(size_type frameCount, sample_type* sampleBuffer) {
    // We want to read a total of "frameCount" frames, and the mp4_read()
    // function wants to know how many bytes we want to decode. One
    // sample is 32-bits = 4 bytes here. One block contains 1024
    // frames with samples for each channel.
    const size_type bytes_per_block = kFramesPerBlock * getChannelCount() * sizeof(sample_type);
    const size_type total_samples_to_decode = frames2samples(frameCount);
    const size_type total_bytes_to_decode = total_samples_to_decode * sizeof(sample_type);
    size_type total_bytes_decoded = 0;
    char* byteBuffer = reinterpret_cast<char*>(sampleBuffer);
    do {
        const size_type num_bytes_req = math_min(bytes_per_block, total_bytes_to_decode - total_bytes_decoded);
        const int numRead = mp4_read(&ipd, byteBuffer, num_bytes_req);
        if (numRead <= 0) {
            //qDebug() << "SSM4A::read: EOF";
            break;
        }
        byteBuffer += numRead;
        total_bytes_decoded += numRead;
    } while (total_bytes_decoded < total_bytes_to_decode);
    const size_type total_samples_decoded = total_bytes_decoded / sizeof(sample_type);
    return samples2frames(total_samples_decoded);
}

Result SoundSourceM4A::parseHeader(){
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

    return OK;
}

QImage SoundSourceM4A::parseCoverArt() {
    TagLib::MP4::File f(getFilename().toLocal8Bit().constData());
    TagLib::MP4::Tag *mp4(f.tag());
    if (mp4) {
        return getCoverInMP4Tag(*mp4);
    } else {
        return QImage();
    }
}

QList<QString> SoundSourceM4A::supportedFileExtensions()
{
    QList<QString> list;
    list.push_back("m4a");
    list.push_back("mp4");
    return list;
}

}
