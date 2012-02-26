#include "audiotagger.h"

#include <QtDebug>
#include <taglib/tag.h>
#include <taglib/id3v2frame.h>

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
#include <taglib/textidentificationframe.h>

AudioTagger::AudioTagger (QString file)
{
    m_artist = "";
    m_title = "";
    m_genre = "";
    m_composer = "";
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


void AudioTagger::setComposer (QString composer )
{
    m_composer = composer;
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
bool AudioTagger::save ()
{
    TagLib::File* file = NULL;

    if(m_file.endsWith(".mp3", Qt::CaseInsensitive)){
        file =  new TagLib::MPEG::File(m_file.toUtf8().constData());
        //process special ID3 fields, APEv2 fiels, etc

        //If the mp3 has no ID3v2 tag, we create a new one and add the TBPM and TKEY frame
        addID3v2Tag( ((TagLib::MPEG::File*) file)->ID3v2Tag(true)  );
        //If the mp3 has an APE tag, we update
        addAPETag( ((TagLib::MPEG::File*) file)->APETag(false)  );

    }
    if(m_file.endsWith(".m4a", Qt::CaseInsensitive)){
        file =  new TagLib::MP4::File(m_file.toUtf8().constData());
        //process special ID3 fields, APEv2 fiels, etc
        processMP4Tag(((TagLib::MP4::File*) file)->tag());

    }
    if(m_file.endsWith(".ogg", Qt::CaseInsensitive)){
        file =  new TagLib::Ogg::Vorbis::File(m_file.toUtf8().constData());
        //process special ID3 fields, APEv2 fiels, etc
        addXiphComment( ((TagLib::Ogg::Vorbis::File*) file)->tag()   );

    }
    if(m_file.endsWith(".wav", Qt::CaseInsensitive)){
        file =  new TagLib::RIFF::WAV::File(m_file.toUtf8().constData());
        //If the flac has no ID3v2 tag, we create a new one and add the TBPM and TKEY frame
        addID3v2Tag( ((TagLib::RIFF::WAV::File*) file)->tag()  );

    }
    if(m_file.endsWith(".flac", Qt::CaseInsensitive)){
        file =  new TagLib::FLAC::File(m_file.toUtf8().constData());

        //If the flac has no ID3v2 tag, we create a new one and add the TBPM and TKEY frame
        addID3v2Tag( ((TagLib::FLAC::File*) file)->ID3v2Tag(true)  );
        //If the flac has no APE tag, we create a new one and add the TBPM and TKEY frame
        addXiphComment( ((TagLib::FLAC::File*) file)->xiphComment (true)   );

    }
    if(m_file.endsWith(".aif", Qt::CaseInsensitive) || m_file.endsWith(".aiff", Qt::CaseInsensitive)){
        file =  new TagLib::RIFF::AIFF::File(m_file.toUtf8().constData());
        //If the flac has no ID3v2 tag, we create a new one and add the TBPM and TKEY frame
        addID3v2Tag( ((TagLib::RIFF::AIFF::File*) file)->tag()  );

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
        int success = file->save();
        if (success) {
            qDebug() << "Successfully updated metadata of track " << m_file;
        } else {
             qDebug() << "Could not update metadata of track " << m_file;
         }
        //delete file and return
        delete file;
        return success;
    }
    else{
        return false;
    }


}
void AudioTagger::addID3v2Tag(TagLib::ID3v2::Tag* id3v2)
{
    if(!id3v2) return;

    TagLib::ID3v2::FrameList bpmFrame = id3v2->frameListMap()["TBPM"];
    if (!bpmFrame.isEmpty())
    {
        bpmFrame.front()->setText(m_bpm.toStdString());

    }
    else
    {
        /*
         * add new frame TextIdentificationFrame which is responsible for TKEY and TBPM
         * see http://developer.kde.org/~wheeler/taglib/api/classTagLib_1_1ID3v2_1_1TextIdentificationFrame.html
         */

        TagLib::ID3v2::TextIdentificationFrame* newFrame = new TagLib::ID3v2::TextIdentificationFrame("TBPM", TagLib::String::Latin1);

        newFrame->setText(m_bpm.toStdString());
        id3v2->addFrame(newFrame);

    }

    TagLib::ID3v2::FrameList keyFrame = id3v2->frameListMap()["TKEY"];
    if (!keyFrame.isEmpty())
    {
        keyFrame.front()->setText(m_key.toStdString());

    }
    else
    {
        //add new frame
        TagLib::ID3v2::TextIdentificationFrame* newFrame = new TagLib::ID3v2::TextIdentificationFrame("TKEY", TagLib::String::Latin1);

        newFrame->setText(m_key.toStdString());
        id3v2->addFrame(newFrame);

    }

    TagLib::ID3v2::FrameList composerFrame = id3v2->frameListMap()["TCOM"];
    if (!composerFrame.isEmpty())
    {
        composerFrame.front()->setText(m_composer.toStdString());
    }
    else
    {
        //add new frame
        TagLib::ID3v2::TextIdentificationFrame* newFrame =
                new TagLib::ID3v2::TextIdentificationFrame(
                    "TCOM", TagLib::String::Latin1);
        newFrame->setText(m_composer.toStdString());
        id3v2->addFrame(newFrame);
    }
}
void AudioTagger::addAPETag(TagLib::APE::Tag* ape)
{
    if(!ape) return;
    /*
     * Adds to the item specified by key the data value.
     * If replace is true, then all of the other values on the same key will be removed first.
     */
    ape->addValue("BPM",m_bpm.toStdString(), true);
    ape->addValue("BPM",m_bpm.toStdString(), true);
    ape->addValue("Composer",m_composer.toStdString(), true);

}
void AudioTagger::addXiphComment(TagLib::Ogg::XiphComment* xiph)
{
    if(!xiph) return;

    // Some tools use "BPM" so check for that.

    /* Taglib does not support the update of Vorbis comments.
     * thus, we have to reomve the old comment and add the new one
     */
    xiph->removeField("BPM");
    xiph->addField("BPM", m_bpm.toStdString());

    xiph->removeField("TEMPO");
    xiph->addField("TEMPO", m_bpm.toStdString());

    xiph->removeField("INITIALKEY");
    xiph->addField("INITIALKEY", m_key.toStdString());
    xiph->removeField("KEY");
    xiph->addField("KEY", m_key.toStdString());

    xiph->removeField("COMPOSER");
    xiph->addField("COMPOSER", m_key.toStdString());
}
void AudioTagger::processMP4Tag(TagLib::MP4::Tag* mp4)
{
    //TODO

}


