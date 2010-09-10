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

#include <QtDebug>

#include <taglib/tag.h>
#include <taglib/audioproperties.h>
#include <taglib/vorbisfile.h>
#include <taglib/id3v2frame.h>
#include <taglib/id3v2header.h>
#include <taglib/id3v1tag.h>
#include <taglib/tmap.h>
#include <taglib/tstringlist.h>


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

bool SoundSource::processTaglibFile(TagLib::File& f) {
    if (f.isValid()) {
        TagLib::Tag *tag = f.tag();
        if (tag) {
            QString title = TStringToQString(tag->title());
            QString artist = TStringToQString(tag->artist());
            QString album = TStringToQString(tag->album());
            QString comment = TStringToQString(tag->comment());
            QString genre = TStringToQString(tag->genre());
            QString year = QString("%1").arg(tag->year());
            QString trackNumber = QString("%1").arg(tag->track());
            qDebug() << "TagLib" << "title" << title << "artist" << artist << "album" << album << "comment" << comment << "genre" << genre << "year" << year << "trackNumber" << trackNumber;

            setTitle(title);
            setArtist(artist);
            setAlbum(album);
            setComment(comment);
            setGenre(genre);
            setYear(year);
            setTrackNumber(trackNumber);
        }

        TagLib::AudioProperties *properties = f.audioProperties();
        if (properties) {
            int length = properties->length();
            int bitrate = properties->bitrate();
            int sampleRate = properties->sampleRate();
            int channels = properties->channels();

            qDebug() << "TagLib" << "length" << length << "bitrate" << bitrate << "sampleRate" << sampleRate << "channels" << channels;

            setDuration(length);
            setBitrate(bitrate);
            setSampleRate(sampleRate);
            setChannels(channels);
        }

        // If we didn't get any audio properties, this was a failure.
        return properties;
    }
    return false;
}

bool SoundSource::processID3v2Tag(TagLib::ID3v2::Tag* id3v2) {

    // Print every frame in the file.
    TagLib::ID3v2::FrameList::ConstIterator it = id3v2->frameList().begin();
    for(; it != id3v2->frameList().end(); it++) {
        qDebug() << "ID3V2" << (*it)->frameID().data() << "-"
                 << TStringToQString((*it)->toString());
    }

    TagLib::ID3v2::FrameList bpmFrame = id3v2->frameListMap()["TBPM"];
    if (!bpmFrame.isEmpty()) {
        QString sBpm = TStringToQString(bpmFrame.front()->toString());
        qDebug() << "BPM" << sBpm;
        if (sBpm.length() > 0) {
            float fBpm = str2bpm(sBpm);
            if (fBpm > 0)
                setBPM(fBpm);
        }
    }

    TagLib::ID3v2::FrameList keyFrame = id3v2->frameListMap()["TKEY"];
    if (!keyFrame.isEmpty()) {
        QString sKey = TStringToQString(keyFrame.front()->toString());
        qDebug() << "KEY" << sKey;
        // TODO(XXX) write key to SoundSource and copy that to the Track
    }

    return true;
}

bool SoundSource::processAPETag(TagLib::APE::Tag* ape) {
    for(TagLib::APE::ItemListMap::ConstIterator it = ape->itemListMap().begin();
        it != ape->itemListMap().end(); ++it) {
        qDebug() << "APE" << TStringToQString((*it).first) << "-" << TStringToQString((*it).second.toString());
    }
    return true;
}

bool SoundSource::processXiphComment(TagLib::Ogg::XiphComment* xiph) {
    for (TagLib::Ogg::FieldListMap::ConstIterator it = xiph->fieldListMap().begin();
         it != xiph->fieldListMap().end(); ++it) {
        qDebug() << "XIPH" << TStringToQString((*it).first) << "-" << TStringToQString((*it).second.toString());
    }

    if (xiph->fieldListMap().contains("BPM")) {
        TagLib::StringList bpmString = xiph->fieldListMap()["BPM"];
        QString sBpm = TStringToQString(bpmString.toString());
        float bpm = str2bpm(sBpm);
        if(bpm > 0.0f) {
            setBPM(bpm);
        }
    }

    return true;
}

bool SoundSource::processMP4Tag(TagLib::MP4::Tag* mp4) {
    for(TagLib::MP4::ItemListMap::ConstIterator it = mp4->itemListMap().begin();
        it != mp4->itemListMap().end(); ++it) {
        qDebug() << "MP4" << TStringToQString((*it).first) << "-" << TStringToQString((*it).second.toStringList().toString());
    }
    return true;
}
