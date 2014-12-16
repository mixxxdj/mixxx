/***************************************************************************
                          soundsourcesndfile.cpp  -  description
                             -------------------
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

#include "soundsourcesndfile.h"
#include "soundsourcetaglib.h"

#include "sampleutil.h"
#include "util/math.h"

#include <taglib/flacfile.h>
#include <taglib/aifffile.h>
#include <taglib/rifffile.h>
#include <taglib/wavfile.h>

#include <QString>
#include <QtDebug>


/*
   Class for reading files using libsndfile
 */
SoundSourceSndFile::SoundSourceSndFile(QString qFilename)
    : Mixxx::SoundSource(qFilename),
      fh(NULL),
      channels(0),
      filelength(0) {
    // Must be set to 0 per the API for reading (non-RAW files)
    memset(&info, 0, sizeof(info));
}

SoundSourceSndFile::~SoundSourceSndFile() {
    sf_close(fh);
}

QList<QString> SoundSourceSndFile::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("aiff");
    list.push_back("aif");
    list.push_back("wav");
    list.push_back("flac");
    return list;
}

Result SoundSourceSndFile::open() {
#ifdef __WINDOWS__
    // Pointer valid until string changed
    LPCWSTR lpcwFilename = (LPCWSTR)getFilename().utf16();
    fh = sf_wchar_open(lpcwFilename, SFM_READ, &info);
#else
    const QByteArray qbaFilename(getFilename().toLocal8Bit());
    fh = sf_open(qbaFilename.constData(), SFM_READ, &info);
#endif

    if (fh == NULL) {   // sf_format_check is only for writes
        qWarning() << "libsndfile: Error opening file" << getFilename() << sf_strerror(fh);
        return ERR;
    }

    if (sf_error(fh)>0) {
        qWarning() << "libsndfile: Error opening file" << getFilename() << sf_strerror(fh);
        return ERR;
    }

    channels = info.channels;
    setChannels(channels);
    setSampleRate(info.samplerate);
    // This is the 'virtual' filelength. No matter how many channels the file
    // actually has, we pretend it has 2.
    filelength = info.frames * 2; // File length with two interleaved channels

    return OK;
}

long SoundSourceSndFile::seek(long filepos)
{
    unsigned long filepos2 = (unsigned long)filepos;
    if (filelength>0)
    {
        filepos2 = math_min(filepos2,filelength);
        sf_seek(fh, (sf_count_t)filepos2/2, SEEK_SET);
        //Note that we don't error check sf_seek because it reports
        //benign errors under normal usage (ie. we sometimes seek past the end
        //of a song, and it will stop us.)
        return filepos2;
    }
    return 0;
}

/*
   read <size> samples into <destination>, and return the number of
   samples actually read. A sample is a single float representing a
   sample on one channel of the audio. In the case of a monaural file
   then size/2 samples are read from the mono file, and they are
   doubled into stereo.
 */
unsigned SoundSourceSndFile::read(unsigned long size, const SAMPLE * destination)
{
    SAMPLE * dest = (SAMPLE *)destination;
    if (filelength > 0)
    {
        if (channels==2)
        {
            unsigned long no = sf_read_short(fh, dest, size);

            // rryan 2/2009 This code used to lie and say we read
            // 'size' samples no matter what. I left this array
            // zeroing code here in case the Reader doesn't check
            // against this.
            for (unsigned long i=no; i<size; ++i)
                dest[i] = 0;

            return no;
        }
        else if(channels==1)
        {
            // We are not dealing with a stereo file. Read fewer
            // samples than requested and double them because we
            // pretend to every reader that all files are in stereo.
            int readNo = sf_read_short(fh, dest, size/2);

            // dest has enough capacity for (readNo * 2) samples
            SampleUtil::doubleMonoToDualMono(dest, readNo);

            // We doubled the readNo bytes we read into stereo.
            return readNo * 2;
        } else {
            // We do not support music with more than 2 channels.
            return 0;
        }
    }

    // The file has errors or is not open. Tell the truth and return 0.
    qDebug() << "The file has errors or is not open: " << getFilename();
    return 0;
}

Result SoundSourceSndFile::parseHeader()
{
    QString location = getFilename();
    setType(location.section(".",-1).toLower());

    bool is_flac = location.endsWith("flac", Qt::CaseInsensitive);
    bool is_wav = location.endsWith("wav", Qt::CaseInsensitive);
    QByteArray qBAFilename = getFilename().toLocal8Bit();

    if (is_flac) {
        TagLib::FLAC::File f(qBAFilename.constData());
        if (!readFileHeader(this, f)) {
            return ERR;
        }
        TagLib::Ogg::XiphComment* xiph = f.xiphComment();
        if (xiph) {
            readXiphComment(this, *xiph);
        }
        else {
            TagLib::ID3v2::Tag *id3v2(f.ID3v2Tag());
            if (id3v2) {
                readID3v2Tag(this, *id3v2);
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
    } else if (is_wav) {
        TagLib::RIFF::WAV::File f(qBAFilename.constData());
        if (!readFileHeader(this, f)) {
            return ERR;
        }
        TagLib::ID3v2::Tag *id3v2(f.ID3v2Tag());
        if (id3v2) {
            readID3v2Tag(this, *id3v2);
        } else {
            // fallback
            const TagLib::Tag *tag(f.tag());
            if (tag) {
                readTag(this, *tag);
            } else {
                return ERR;
            }
        }

        if (getDuration() <= 0) {
            // we're using a taglib version which doesn't know how to do wav
            // durations, set it with info from sndfile -bkgood
            // XXX remove this when ubuntu ships with an sufficiently
            // intelligent version of taglib, should happen in 11.10

            // Have to open the file for info to be valid.
            if (fh == NULL) {
                open();
            }

            if (info.samplerate > 0) {
                setDuration(info.frames / info.samplerate);
            } else {
                qDebug() << "WARNING: WAV file with invalid samplerate."
                         << "Can't get duration using libsndfile.";
            }
        }
    } else {
        // Try AIFF
        TagLib::RIFF::AIFF::File f(qBAFilename.constData());
        if (!readFileHeader(this, f)) {
            return ERR;
        }
        TagLib::ID3v2::Tag *id3v2(f.tag());
        if (id3v2) {
            readID3v2Tag(this, *id3v2);
        } else {
            return ERR;
        }
    }

    return OK;
}

QImage SoundSourceSndFile::parseCoverArt() {
    QImage coverArt;
    QString location = getFilename();
    setType(location.section(".",-1).toLower());
    const QByteArray qBAFilename(getFilename().toLocal8Bit());

    if (getType() == "flac") {
        TagLib::FLAC::File f(qBAFilename.constData());
        TagLib::ID3v2::Tag* id3v2 = f.ID3v2Tag();
        if (id3v2) {
            coverArt = Mixxx::getCoverInID3v2Tag(*id3v2);
        }
        if (coverArt.isNull()) {
            TagLib::Ogg::XiphComment *xiph = f.xiphComment();
            if (xiph) {
                coverArt = Mixxx::getCoverInXiphComment(*xiph);
            }
        }
        if (coverArt.isNull()) {
            TagLib::List<TagLib::FLAC::Picture*> covers = f.pictureList();
            if (!covers.isEmpty()) {
                std::list<TagLib::FLAC::Picture*>::iterator it = covers.begin();
                TagLib::FLAC::Picture* cover = *it;
                coverArt = QImage::fromData(
                    QByteArray(cover->data().data(), cover->data().size()));
            }
        }
    } else if (getType() == "wav") {
        TagLib::RIFF::WAV::File f(qBAFilename.constData());
        TagLib::ID3v2::Tag* id3v2 = f.tag();
        if (id3v2) {
            coverArt = Mixxx::getCoverInID3v2Tag(*id3v2);
        }
    } else {
        // Try AIFF
        TagLib::RIFF::AIFF::File f(qBAFilename.constData());
        TagLib::ID3v2::Tag* id3v2 = f.tag();
        if (id3v2) {
            coverArt = Mixxx::getCoverInID3v2Tag(*id3v2);
        }
    }
    return coverArt;
}

/*
   Return the length of the file in samples.
 */
inline long unsigned SoundSourceSndFile::length()
{
    return filelength;
}
