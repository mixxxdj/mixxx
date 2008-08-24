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

#include "trackinfoobject.h"
#include "soundsourcem4a.h"
#include <QtDebug>

#ifdef __WIN__
#include <io.h>
#include <fcntl.h>
#endif

SoundSourceM4A::SoundSourceM4A(QString qFileName) : SoundSource(qFileName)
{
   QString mp4FileName;
   mp4FileName = qFileName;
   trackId = MP4_INVALID_TRACK_ID;
   sampleId = 0;
   mp4file = MP4Read(mp4FileName);
   if (mp4file == MP4_INVALID_FILE_HANDLE) {
     qDebug() << "mp4: " << mp4FileName << "could not be opened using the MP4 decoder.";
     filelength = 0;
     return;
   }
   trackId = MP4FindTrackId(mp4file, 0); // We are only interested in first track for the initial dev iteration
   if (trackId == MP4_INVALID_TRACK_ID) {
	   qDebug() << "trackId is invalid.";
	   return;
   }
   channels = 2; //FIXME: hard coded M4A to 2 channels

   filelength = MP4GetTrackNumberOfSamples(mp4file, trackId);
   SRATE = (filelength * 1024 * 1000) / MP4ConvertFromTrackDuration(mp4file, trackId, MP4GetTrackDuration(mp4file, trackId), MP4_MSECS_TIME_SCALE);
   SRATE = 44100; // FIXME: Hard-coded SRATE, above formula for SRATE overflows an unsigned long... :(
   qDebug() << "SRATE:"<< SRATE;
   qDebug() << "filelength:" << filelength;
}

SoundSourceM4A::~SoundSourceM4A(){
   if (mp4file != MP4_INVALID_FILE_HANDLE) {
      MP4Close(mp4file);
   }
}

long SoundSourceM4A::seek(long filepos){
	Q_ASSERT(filepos % 2 == 0);
	sampleId = 1 + (2 * filepos / READCHUNKSIZE); // sampleId is 1 indexed
    qDebug() << "seek sampleId:"<< sampleId << "filepos:"<< filepos;
}

unsigned SoundSourceM4A::read(volatile unsigned long size, const SAMPLE* destination){
	Q_ASSERT(size % 2 == 0);

    MP4Timestamp sampleTime;
    MP4Duration sampleDuration, sampleRenderingOffset;
    bool isSyncSample;

//    uint32_t sample_size = MP4GetTrackMaxSampleSize(mp4file, trackId);
//    uint8_t *sample = (uint8_t *)malloc(sample_size);
    unsigned int this_size = MP4GetTrackMaxSampleSize(mp4file, trackId); // size;

    uint8_t *sample = (uint8_t*) destination;

	bool ret = MP4ReadSample(mp4file,
			trackId,
			sampleId,
			&sample,
			&this_size,
			&sampleTime,
			&sampleDuration,
			&sampleRenderingOffset,
			&isSyncSample);
    qDebug() << "read Track:"<< trackId << "Sample:" << sampleId << "Length:" << this_size << "success:"<<ret;
	if (ret == false) {
		qDebug() << "read: Sample read error\n";
		return -1;
	}
    return this_size;
}

inline long unsigned SoundSourceM4A::length(){
   return filelength;
}

int SoundSourceM4A::ParseHeader( TrackInfoObject * Track){
    QString mp4FileName = Track->getLocation();

    char *value;
    MP4FileHandle mp4file = MP4Read(mp4FileName);
    if (mp4file == MP4_INVALID_FILE_HANDLE) {
     qDebug() << "mp4: " << mp4FileName << "could not be opened using the MP4 decoder.";
     return ERR;
   }

   if (MP4GetMetadataName(mp4file, &value) && value != NULL) {
     Track->setTitle(value);
     free(value);
   }
   if (MP4GetMetadataArtist(mp4file, &value) && value != NULL) {
     Track->setArtist(value);
     free(value);
   }
   if (MP4GetMetadataComment(mp4file, &value) && value != NULL) {
     Track->setComment(value);
     free(value);
   }
   u_int16_t bpm = 0;
   if (MP4GetMetadataTempo(mp4file, &bpm)) {
       if(bpm > 0) {
           Track->setBpm(bpm);
           Track->setBpmConfirm(true);
       }
   }
   Track->setHeaderParsed(true);
   Track->setType("m4a");

   int trackId = MP4FindTrackId(mp4file, 0); // We are only interested in first track for the initial dev iteration
   Track->setDuration(MP4ConvertFromTrackDuration(mp4file, trackId, MP4GetTrackDuration(mp4file, trackId), MP4_SECS_TIME_SCALE));
   Track->setBitrate(MP4GetTrackBitRate(mp4file, trackId)/1000);

   Track->setChannels(2); // FIXME: hard-coded to 2 channels

   MP4Close(mp4file);
   return OK;
}
