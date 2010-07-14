/***************************************************************************
                          soundsource.cpp  -  description
                             -------------------
    begin                : Wed Feb 20 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                :
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "soundsource.h"

/*
   SoundSource is an Uber-class for the reading and decoding of audio-files.
   Each class must have the following member functions:
   initializer with a filename
   seek()
   read()
   length()
   In addition there must be a static member:
   int ParseHeader(TrackInfoObject *Track)
   which is used for parsing header information, like trackname,length etc. The
   return type is int: 0 for OK, -1 for an error.
 */
SoundSource::SoundSource(QString qFilename)
{
    m_qFilename = qFilename;
    m_iSampleRate = 0;
    m_fBPM = 0.0f;
    m_iDuration = 0;
    m_iBitrate = 0;
    m_iChannels = 0;
}

SoundSource::~SoundSource()
{
}


QList<long> *SoundSource::getCuePoints()
{
    return 0;
}

QString SoundSource::getFilename()
{
    return m_qFilename;
}

float SoundSource::str2bpm( QString sBpm ) {
  float bpm = sBpm.toFloat();
  if(bpm < 60) bpm = 0;
  while( bpm > 300 ) bpm = bpm / 10.;
  return bpm;
}

QString SoundSource::getArtist()
{
    return m_sArtist;
}
QString SoundSource::getTitle()
{
    return m_sTitle;
}
QString SoundSource::getAlbum()
{
    return m_sAlbum;
}
QString SoundSource::getType()
{
    return m_sType;
}
QString SoundSource::getComment()
{
    return m_sComment;
}
QString SoundSource::getYear()
{
    return m_sYear;
}
QString SoundSource::getGenre()
{
    return m_sGenre;
}
QString SoundSource::getTrackNumber()
{
    return m_sTrackNumber;
}
float SoundSource::getBPM()
{
    return m_fBPM;
}
int SoundSource::getDuration()
{
    return m_iDuration;
}
int SoundSource::getBitrate()
{
    return m_iBitrate;
}
unsigned int SoundSource::getSampleRate()
{
    return m_iSampleRate;
}
int SoundSource::getChannels()
{
    return m_iChannels;
}

void SoundSource::setArtist(QString artist)
{
    m_sArtist = artist;
}
void SoundSource::setTitle(QString title)
{
    m_sTitle = title;
}
void SoundSource::setAlbum(QString album)
{
    m_sAlbum = album;
}
void SoundSource::setComment(QString comment)
{
    m_sComment = comment;
}
void SoundSource::setType(QString type)
{
    m_sType = type;
}
void SoundSource::setYear(QString year)
{
    m_sYear = year;
}
void SoundSource::setGenre(QString genre)
{
    m_sGenre = genre;
}
void SoundSource::setTrackNumber(QString trackNumber)
{
    m_sTrackNumber = trackNumber;
}
void SoundSource::setBPM(float bpm)
{
    m_fBPM = bpm;
}
void SoundSource::setDuration(int duration)
{
    m_iDuration = duration;
}
void SoundSource::setBitrate(int bitrate)
{
    m_iBitrate = bitrate;
}
void SoundSource::setSampleRate(unsigned int samplerate)
{
    m_iSampleRate = samplerate;
}
void SoundSource::setChannels(int channels)
{
    m_iChannels = channels;
}
