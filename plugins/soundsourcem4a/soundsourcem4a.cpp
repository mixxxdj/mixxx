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

#include <QtDebug>
#include "soundsourcem4a.h"
#include "m4a/mp4-mixxx.cpp"

namespace Mixxx {

SoundSourceM4A::SoundSourceM4A(QString qFileName)
  : SoundSource(qFileName) {

    // Initialize variables to invalid values in case loading fails.
    mp4file = MP4_INVALID_FILE_HANDLE;
    filelength = 0;
    memset(&ipd, 0, sizeof(ipd));
}

SoundSourceM4A::~SoundSourceM4A() {
    if (ipd.filename) {
        delete [] ipd.filename;
        ipd.filename = NULL;
    }

    if (mp4file != MP4_INVALID_FILE_HANDLE) {
        mp4_close(&ipd);
        mp4file = MP4_INVALID_FILE_HANDLE;
    }
}

int SoundSourceM4A::open()
{
    //Initialize the FAAD2 decoder...
    initializeDecoder();

    //qDebug() << "SSM4A: channels:" << m_iChannels
    //         << "filelength:" << filelength
    //         << "Sample Rate:" << m_iSampleRate;
    return OK;
}

int SoundSourceM4A::initializeDecoder()
{
    // Copy QString to char[] buffer for mp4_open to read from later
    QByteArray qbaFileName;
#ifdef Q_OS_WIN32
    // fopen() doesn't do utf8 on windows
    qbaFileName = m_qFilename.toLocal8Bit();
#else
    qbaFileName = m_qFilename.toUtf8();
#endif
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
                 << m_qFilename << " with status:" << mp4_open_status;
        return ERR;
    }

    // mp4_open succeeded -> populate variables
    mp4_private* mp = (struct mp4_private*)ipd.private_ipd;
    Q_ASSERT(mp);
    mp4file = mp->mp4.handle;
    filelength = mp4_total_samples(&ipd);
    m_iSampleRate = mp->sample_rate;
    m_iChannels = mp->channels;

    return OK;
}

long SoundSourceM4A::seek(long filepos){
    // Abort if file did not load.
    if (filelength == 0)
        return 0;

    //qDebug() << "SSM4A::seek()" << filepos;

    // qDebug() << "MP4SEEK: seek time:" << filepos / (m_iChannels * m_iSampleRate) ;

    int position = mp4_seek_sample(&ipd, filepos);
    //int position = mp4_seek(&ipd, filepos / (m_iChannels * m_iSampleRate));
    return position;
}

unsigned SoundSourceM4A::read(volatile unsigned long size, const SAMPLE* destination) {
    // Abort if file did not load.
    if (filelength == 0)
        return 0;

    //qDebug() << "SSM4A::read()" << size;

    // We want to read a total of "size" samples, and the mp4_read()
    // function wants to know how many bytes we want to decode. One
    // sample is 16-bits = 2 bytes here, so we multiply size by channels to
    // get the number of bytes we want to decode.

    int total_bytes_to_decode = size * m_iChannels;
    int total_bytes_decoded = 0;
    int num_bytes_req = 4096;
    char* buffer = (char*)destination;
    SAMPLE * as_buffer = (SAMPLE*) destination; //pointer for mono->stereo filling.
    do {
        if (total_bytes_decoded + num_bytes_req > total_bytes_to_decode)
            num_bytes_req = total_bytes_to_decode - total_bytes_decoded;

        // (char *)&destination[total_bytes_decoded/2],
        int numRead = mp4_read(&ipd,
                               buffer,
                               num_bytes_req);
        if(numRead <= 0) {
            //qDebug() << "SSM4A::read: EOF";
            break;
        }
        buffer += numRead;
        total_bytes_decoded += numRead;
    } while (total_bytes_decoded < total_bytes_to_decode);

    // At this point *destination should be filled. If mono : double all samples
    // (L => R)
    if (m_iChannels == 1) {
        for (int i = total_bytes_decoded/2-1; i >= 0; --i) {
            // as_buffer[i] is an audio sample (s16)
            //scroll through , copying L->R & expanding buffer
            as_buffer[i*2+1] = as_buffer[i];
            as_buffer[i*2] = as_buffer[i];
        }
    }

    // Tell us about it only if we end up decoding a different value
    // then what we expect.

    if (total_bytes_decoded % (size * 2)) {
        qDebug() << "SSM4A::read : total_bytes_decoded:"
                 << total_bytes_decoded
                 << "size:"
                 << size;
    }

    //There are two bytes in a 16-bit sample, so divide by 2.
    return total_bytes_decoded / 2;
}

inline long unsigned SoundSourceM4A::length(){
    return filelength;
    //return m_iChannels * mp4_duration(&ipd) * m_iSampleRate;
}

int SoundSourceM4A::parseHeader(){
    setType("m4a");
    
#ifdef __WINDOWS__
    // fopen() doesn't do utf8 on windows
    TagLib::MP4::File f(getFilename().toLocal8Bit().constData());
#else
    TagLib::MP4::File f(getFilename().toUtf8().constData());
#endif

    bool result = processTaglibFile(f);
    TagLib::MP4::Tag* tag = f.tag();

    if (tag) {
        processMP4Tag(tag);
    }

    if (result)
        return OK;
    return ERR;
}

QList<QString> SoundSourceM4A::supportedFileExtensions()
{
    QList<QString> list;
    list.push_back("m4a");
    list.push_back("mp4");
    return list;
}

}
