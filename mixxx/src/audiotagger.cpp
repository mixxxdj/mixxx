#include "audiotagger.h"

#include <QtDebug>
#include <taglib/tag.h>

#include <taglib/id3v2frame.h>
#include <taglib/id3v2header.h>
#include <taglib/id3v1tag.h>
#include <taglib/tmap.h>
#include <taglib/tstringlist.h>

#include <taglib/vorbisfile.h>
#include <taglib/wavpackfile.h>
#include <taglib/mpegfile.h>
#include <taglib/mp4file.h>
#include <taglib/oggfile.h>
#include <taglib/flacfile.h>
#include <taglib/aifffile.h>
#include <taglib/rifffile.h>
#include <taglib/wavfile.h>

AudioTagger::AudioTagger (QString file) 
{
    m_artist = "";
    m_title = "";
    m_genre = "";
    m_album = "";
    m_year = "";
    m_comment = "";
    m_key = "";
    m_bpm = "";
    m_tracknumber = "";

    m_file = file;
}

AudioTagger::~AudioTagger ( ) 
{ 

}


void AudioTagger::setArtist (QString artist )
{
    m_artist = artist;
}



void AudioTagger::setTitle (QString title )
{
    m_title = title;
}



void AudioTagger::setAlbum (QString album )
{
    m_album = album;
}



void AudioTagger::setGenre (QString genre )
{
    m_genre = genre;
}



void AudioTagger::setYear (QString year )
{
    m_year = year;
}



void AudioTagger::setComment (QString comment )
{
    m_comment = comment;
}

void AudioTagger::setKey (QString key )
{
m_key = key;
}

void AudioTagger::setBpm (QString bpm )
{
    m_bpm = bpm;
}

void AudioTagger::setTracknumber (QString tracknumber )
{
    m_tracknumber = tracknumber;
}
void AudioTagger::save ()
{
    TagLib::File* file = NULL;
    
    if(m_file.endsWith(".mp3", Qt::CaseInsensitive)){
        file =  new TagLib::MPEG::File(m_file.toUtf8().constData());
        //process special ID3 fields, APEv2 fiels, etc
   
    }
    if(m_file.endsWith(".mp4", Qt::CaseInsensitive)){
        file =  new TagLib::MP4::File(m_file.toUtf8().constData());
        //process special ID3 fields, APEv2 fiels, etc
   
    }
    if(m_file.endsWith(".ogg", Qt::CaseInsensitive)){
        file =  new TagLib::Ogg::Vorbis::File(m_file.toUtf8().constData());
        //process special ID3 fields, APEv2 fiels, etc
   
    }
    if(m_file.endsWith(".wav", Qt::CaseInsensitive)){
        file =  new TagLib::RIFF::WAV::File(m_file.toUtf8().constData());
        //process special ID3 fields, APEv2 fiels, etc
   
    }
    if(m_file.endsWith(".flac", Qt::CaseInsensitive)){
        file =  new TagLib::FLAC::File(m_file.toUtf8().constData());
        //process special ID3 fields, APEv2 fiels, etc
   
    }
    if(m_file.endsWith(".aif", Qt::CaseInsensitive) || m_file.endsWith(".aiff", Qt::CaseInsensitive)){
        file =  new TagLib::RIFF::AIFF::File(m_file.toUtf8().constData());
        //process special ID3 fields, APEv2 fiels, etc
   
    }
    
    //process standard tags
    if(file){
        TagLib::Tag *tag = file->tag();
        if (tag) 
        {
            tag->setArtist(m_artist.toStdString());
            tag->setTitle(m_title.toStdString());
            tag->setAlbum(m_album.toStdString());
            tag->setGenre(m_genre.toStdString());
            tag->setComment(m_comment.toStdString());
            uint year =  m_year.toUInt();
            if(year >  0)         
                tag->setYear(year);
            uint tracknumber = m_tracknumber.toUInt();
            if(tracknumber > 0)
                tag->setTrack(tracknumber);
           
        }
        //write audio tags to file
        if(file->save())
            qDebug() << "Successfully updated metadata of track " << m_file;
        else
             qDebug() << "Could not update metadata of track " << m_file;
        delete file;
    }

   
}
void AudioTagger::addID3v2Tag(TagLib::ID3v2::Tag* id3v2)
{

}
void AudioTagger::addAPETag(TagLib::APE::Tag* ape)
{

}
void AudioTagger::addXiphComment(TagLib::Ogg::XiphComment* xiph)
{

}
void AudioTagger::processMP4Tag(TagLib::MP4::Tag* mp4)
{

}


