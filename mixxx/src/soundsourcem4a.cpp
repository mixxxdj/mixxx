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

#include <neaacdec.h>

#ifdef __WINDOWS__
#include <io.h>
#include <fcntl.h>
#endif

#include <QtDebug>

#include "soundsourcem4a.h"

#include "trackinfoobject.h"

#include "m4a/mp4-mixxx.cpp"

SoundSourceM4A::SoundSourceM4A(QString qFileName)
  : SoundSource(qFileName) {

    // Initialize variables to invalid values in case loading fails.
    mp4file = MP4_INVALID_FILE_HANDLE;
    filelength = -1;
    channels = 0;
    memset(&ipd, 0, sizeof(ipd));

    // Copy QString to char[] buffer for mp4_open to read from later
    int bytes = qFileName.length() + 1;
    ipd.filename = new char[bytes];
    ipd.remote = false; // File is not an stream
    strncpy(ipd.filename, qFileName, bytes);
    ipd.filename[bytes-1] = '\0';

    int mp4_open_status = mp4_open(&ipd);
    if (mp4_open_status != 0) {
        // The file was loading and failing iratically because
        // ipd.remote was an in an uninitialized state, it needed to be
        // set to false.
        qDebug() << "SSM4A: failed to open MP4 file '"
                 << qFileName << "' w/ status:" << mp4_open_status << " FIXME.";
        return;
    }

    // mp4_open succeeded -> populate variables
    mp4_private* mp = (struct mp4_private*)ipd.private_ipd;
    Q_ASSERT(mp);
    mp4file = mp->mp4.handle;
    filelength = mp4_total_samples(&ipd);
    SRATE = mp->sample_rate;
    channels = mp->channels;

    qDebug() << "SSM4A: channels:" << channels
             << "filelength:" << filelength
             << "Sample Rate:" << SRATE;
}

SoundSourceM4A::~SoundSourceM4A() {
    if (ipd.filename) {
        delete ipd.filename;
        ipd.filename = NULL;
    }

    if (mp4file != MP4_INVALID_FILE_HANDLE) {
        mp4_close(&ipd);
        mp4file = MP4_INVALID_FILE_HANDLE;
    }
}

long SoundSourceM4A::seek(long filepos){
    // Abort if file did not load.
    if (filelength == -1)
        return 0;

    qDebug() << "SSM4A::seek()" << filepos;

    // qDebug() << "MP4SEEK: seek time:" << filepos / (channels * SRATE) ;

    int position = mp4_seek_sample(&ipd, filepos);
    //int position = mp4_seek(&ipd, filepos / (channels * SRATE));
    return position;
}

unsigned SoundSourceM4A::read(volatile unsigned long size, const SAMPLE* destination) {
    // Abort if file did not load.
    if (filelength == -1)
        return -1;

    //qDebug() << "SSM4A::read()" << size;

    // We want to read a total of "size" samples, and the mp4_read()
    // function wants to know how many bytes we want to decode. One
    // sample is 16-bits = 2 bytes here, so we multiply size by 2 to
    // get the number of bytes we want to decode.

    // rryan 2/2009 Can M4A files be non-stereo? If so, we need to
    // replicate logic here similar to in other areas.

    int total_bytes_to_decode = size * 2;
    int total_bytes_decoded = 0;
    int num_bytes_req = 4096;
    char* buffer = (char*)destination;
    do {
        if (total_bytes_decoded + num_bytes_req > total_bytes_to_decode)
            num_bytes_req = total_bytes_to_decode - total_bytes_decoded;

        // (char *)&destination[total_bytes_decoded/2],
        int numRead = mp4_read(&ipd,
                               buffer,
                               num_bytes_req);
        if(numRead <= 0) {
            qDebug() << "EOF";
            break;
        }
        buffer += numRead;
        total_bytes_decoded += numRead;
    } while (total_bytes_decoded < total_bytes_to_decode);

    // Tell us about it only if we end up decoding a different value
    // then what we expect.

    if (total_bytes_decoded % (size * 2))
        qDebug() << "MP4READ: total_bytes_decoded:"
                 << total_bytes_decoded
                 << "size:"
                 << size;

    //There are two bytes in a 16-bit sample, so divide by 2.
    return total_bytes_decoded / 2;
}

inline long unsigned SoundSourceM4A::length(){
     if (filelength == -1)
         return -1;
     return filelength;
     //return channels * mp4_duration(&ipd) * SRATE;
}

int SoundSourceM4A::ParseHeader( TrackInfoObject * Track){
    if (Track->getHeaderParsed())
        return OK;

    QString mp4FileName = Track->getLocation();
    MP4FileHandle mp4file = MP4Read(mp4FileName);

    if (mp4file == MP4_INVALID_FILE_HANDLE) {
        qDebug() << "mp4: " << mp4FileName
                 << "could not be opened using the MP4 decoder.";
        Track->setHeaderParsed(false);
        return ERR;
    }

    char* value = NULL;
    if (MP4GetMetadataName(mp4file, &value) && value != NULL) {
        Track->setTitle(value);
        MP4Free(value);
        value = NULL;
    }

    if (MP4GetMetadataArtist(mp4file, &value) && value != NULL) {
        Track->setArtist(value);
        MP4Free(value);
        value = NULL;
    }

    if (MP4GetMetadataComment(mp4file, &value) && value != NULL) {
        Track->setComment(value);
        MP4Free(value);
        value = NULL;
    }

    u_int16_t bpm = 0;
    if (MP4GetMetadataTempo(mp4file, &bpm)) {
        if(bpm > 0) {
            Track->setBpm(bpm);
            Track->setBpmConfirm(true);
        }
    }

    // We are only interested in first track for the initial dev iteration
    int track_id = MP4FindTrackId(mp4file, 0, MP4_AUDIO_TRACK_TYPE);

    if (track_id == MP4_INVALID_TRACK_ID) {
        qDebug() << "Could not find any audio tracks in " << mp4FileName;
    }

    // Get the track duration in time scale units of the mp4
    int duration_scale = MP4GetTrackDuration(mp4file, track_id);
    int duration_seconds = MP4ConvertFromTrackDuration(mp4file, track_id,
                                                       duration_scale,
                                                       MP4_SECS_TIME_SCALE);
    Track->setDuration(duration_seconds);

    int bits_per_second = MP4GetTrackBitRate(mp4file, track_id);
    Track->setBitrate(bits_per_second/1000);

    MP4Close(mp4file);
    Track->setHeaderParsed(true);
    Track->setType("m4a");
    // FIXME: hard-coded to 2 channels - real value is not available until
    // faacDecInit2 is called
    Track->setChannels(2);

    return OK;
}
