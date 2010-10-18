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
#include <taglib/textidentificationframe.h>


#include "soundsource.h"

// static
const bool SoundSource::s_bDebugMetadata = false;

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
    m_fRG = 0.0f;
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
float SoundSource::getRG()
{
	//qDebug() << "Getting Replay Gain value:" << m_fRG;
	return m_fRG;
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
void SoundSource::setRG(float replaygain)
{
	//qDebug() << "Setting RG value to " << replaygain;
	m_fRG = replaygain;
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
    if (s_bDebugMetadata)
        qDebug() << "Parsing" << getFilename();

    if (f.isValid()) {
        TagLib::Tag *tag = f.tag();
        if (tag) {
            QString title = TStringToQString(tag->title());
            setTitle(title);

            QString artist = TStringToQString(tag->artist());
            setArtist(artist);

            QString album = TStringToQString(tag->album());
            setAlbum(album);

            QString comment = TStringToQString(tag->comment());
            setComment(comment);

            QString genre = TStringToQString(tag->genre());
            setGenre(genre);

            int iYear = tag->year();
            QString year = "";
            if (iYear > 0) {
                year = QString("%1").arg(iYear);
                setYear(year);
            }

            int iTrack = tag->track();
            QString trackNumber = "";
            if (iTrack > 0) {
                trackNumber = QString("%1").arg(tag->track());
                setTrackNumber(trackNumber);
            }

            if (s_bDebugMetadata)
                qDebug() << "TagLib" << "title" << title << "artist" << artist << "album" << album << "comment" << comment << "genre" << genre << "year" << year << "trackNumber" << trackNumber;
        }

        TagLib::AudioProperties *properties = f.audioProperties();
        if (properties) {
            int lengthSeconds = properties->length();
            int bitrate = properties->bitrate();
            int sampleRate = properties->sampleRate();
            int channels = properties->channels();

            if (s_bDebugMetadata)
                qDebug() << "TagLib" << "length" << lengthSeconds << "bitrate" << bitrate << "sampleRate" << sampleRate << "channels" << channels;

            setDuration(lengthSeconds);
            setBitrate(bitrate);
            setSampleRate(sampleRate);
            setChannels(channels);
        }

        // If we didn't get any audio properties, this was a failure.
        return properties;
    }
    return false;
}

void SoundSource::parseRGString (QString sRG) {
	 QString RGstring = sRG.remove( " dB" );
	 float fRG = pow(10,(RGstring.toFloat())/20);
	 //I found some mp3s of mine with replaygain tag set to 0dB even if not normalized.
	 //This is because of Rapid Evolution 3, I suppose. I prefer to rescan them by setting value to 0 (i.e. rescan via analyserrg)
	 if(fRG==1.0f){
		 fRG= 0.0f;
	 }
	 setRG(fRG);
}

void SoundSource::processBpmString(QString tagName, QString sBpm) {
    if (s_bDebugMetadata)
        qDebug() << tagName << "BPM" << sBpm;
    if (sBpm.length() > 0) {
        float fBpm = str2bpm(sBpm);
        if (fBpm > 0)
            setBPM(fBpm);
    }
}

bool SoundSource::processID3v2Tag(TagLib::ID3v2::Tag* id3v2) {

    // Print every frame in the file.
    if (s_bDebugMetadata) {
        TagLib::ID3v2::FrameList::ConstIterator it = id3v2->frameList().begin();
        for(; it != id3v2->frameList().end(); it++) {
            qDebug() << "ID3V2" << (*it)->frameID().data() << "-"
                     << TStringToQString((*it)->toString());
        }
    }

    TagLib::ID3v2::FrameList bpmFrame = id3v2->frameListMap()["TBPM"];
    if (!bpmFrame.isEmpty()) {
        QString sBpm = TStringToQString(bpmFrame.front()->toString());
        processBpmString("ID3v2", sBpm);
    }

    TagLib::ID3v2::FrameList keyFrame = id3v2->frameListMap()["TKEY"];
    if (!keyFrame.isEmpty()) {
        QString sKey = TStringToQString(keyFrame.front()->toString());
        if (s_bDebugMetadata)
            qDebug() << "KEY" << sKey;
        // TODO(XXX) write key to SoundSource and copy that to the Track
    }
    // Foobar2000-style ID3v2.3.0 tags
    // TODO: Check if everything is ok.
         TagLib::ID3v2::FrameList frames = id3v2->frameListMap()["TXXX"];
         for ( TagLib::ID3v2::FrameList::Iterator it = frames.begin(); it != frames.end(); ++it ) {
             TagLib::ID3v2::UserTextIdentificationFrame* RGframe =
                 dynamic_cast<TagLib::ID3v2::UserTextIdentificationFrame*>( *it );
             if ( RGframe && RGframe->fieldList().size() >= 2 )
             {
                 QString desc = TStringToQString( RGframe->description() ).toLower();
                 if ( desc == "replaygain_album_gain" ){
                	 QString sRG = TStringToQString( RGframe->fieldList()[1]);
                 	 parseRGString(sRG);
                 }
                 if ( desc == "replaygain_track_gain" ){
                	 QString sRG = TStringToQString( RGframe->fieldList()[1]);
                	 parseRGString(sRG);
                 }
             }
         }




    return true;
}

bool SoundSource::processAPETag(TagLib::APE::Tag* ape) {
    if (s_bDebugMetadata) {
        for(TagLib::APE::ItemListMap::ConstIterator it = ape->itemListMap().begin();
            it != ape->itemListMap().end(); ++it) {
            qDebug() << "APE" << TStringToQString((*it).first) << "-" << TStringToQString((*it).second.toString());
        }
    }

    if (ape->itemListMap().contains("BPM")) {
        QString sBpm = TStringToQString(ape->itemListMap()["BPM"].toString());
        processBpmString("APE", sBpm);
    }

    if ( ape->itemListMap().contains("REPLAYGAIN_ALBUM_GAIN") ) {
    	QString sRG = TStringToQString(ape->itemListMap()["REPLAYGAIN_ALBUM_GAIN"].toString());
    	parseRGString(sRG);
    }

    //Prefer track gain over album gain.
    if ( ape->itemListMap().contains("REPLAYGAIN_TRACK_GAIN") ) {
       	QString sRG = TStringToQString(ape->itemListMap()["REPLAYGAIN_TRACK_GAIN"].toString());
       	qDebug()<<"APE value" << sRG;
       	parseRGString(sRG);
       }
    return true;
}

bool SoundSource::processXiphComment(TagLib::Ogg::XiphComment* xiph) {
    if (s_bDebugMetadata) {
        for (TagLib::Ogg::FieldListMap::ConstIterator it = xiph->fieldListMap().begin();
             it != xiph->fieldListMap().end(); ++it) {
            qDebug() << "XIPH" << TStringToQString((*it).first) << "-" << TStringToQString((*it).second.toString());
        }
    }

    // Some tags use "BPM" so check for that.
    if (xiph->fieldListMap().contains("BPM")) {
        TagLib::StringList bpmString = xiph->fieldListMap()["BPM"];
        QString sBpm = TStringToQString(bpmString.toString());
        processBpmString("XIPH-BPM", sBpm);
    }

    // Give preference to the "TEMPO" tag which seems to be more standard
    if (xiph->fieldListMap().contains("TEMPO")) {
        TagLib::StringList bpmString = xiph->fieldListMap()["TEMPO"];
        QString sBpm = TStringToQString(bpmString.toString());
        processBpmString("XIPH-TEMPO", sBpm);
    }


    if (xiph->fieldListMap().contains("REPLAYGAIN_ALBUM_GAIN")) {
            TagLib::StringList rgainString = xiph->fieldListMap()["REPLAYGAIN_ALBUM_GAIN"];
            QString sRG = TStringToQString(rgainString.toString());
            parseRGString(sRG);
        }

    if (xiph->fieldListMap().contains("REPLAYGAIN_TRACK_GAIN")) {
                TagLib::StringList rgainString = xiph->fieldListMap()["REPLAYGAIN_TRACK_GAIN"];
                QString sRG = TStringToQString(rgainString.toString());
                parseRGString(sRG);
            }


    return true;
}

bool SoundSource::processMP4Tag(TagLib::MP4::Tag* mp4) {
    if (s_bDebugMetadata) {
        for(TagLib::MP4::ItemListMap::ConstIterator it = mp4->itemListMap().begin();
            it != mp4->itemListMap().end(); ++it) {
            qDebug() << "MP4" << TStringToQString((*it).first) << "-" << TStringToQString((*it).second.toStringList().toString());
        }
    }

    // Get BPM
    if (mp4->itemListMap().contains("tmpo")) {
        QString sBpm = TStringToQString(mp4->itemListMap()["tmpo"].toStringList().toString());
        processBpmString("MP4", sBpm);
    }
    return true;
}
