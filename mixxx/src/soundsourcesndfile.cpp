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

#include <taglib/flacfile.h>
#include <taglib/aifffile.h>
#include <taglib/rifffile.h>
#include <taglib/wavfile.h>

#include "trackinfoobject.h"
#include "soundsourcesndfile.h"
#include <qstring.h>
#include <QtDebug>

/*
   Class for reading files using libsndfile
 */
SoundSourceSndFile::SoundSourceSndFile(QString qFilename) :
    Mixxx::SoundSource(qFilename)
{
    m_bOpened = false;
    info = new SF_INFO;
    info->format = 0;   // Must be set to 0 per the API for reading (non-RAW files)
    filelength = 0;
}

SoundSourceSndFile::~SoundSourceSndFile()
{
    if (m_bOpened) {
        sf_close(fh);
    }
    delete info;
}

QList<QString> SoundSourceSndFile::supportedFileExtensions()
{
    QList<QString> list;
    list.push_back("aiff");
    list.push_back("aif");
    list.push_back("wav");
    list.push_back("flac");
    return list;
}

int SoundSourceSndFile::open() {
#ifdef __WINDOWS__
    QByteArray qbaFilename = m_qFilename.toLocal8Bit();
#else
    QByteArray qbaFilename = m_qFilename.toUtf8();
#endif
    fh = sf_open( qbaFilename.data(), SFM_READ, info );

    if (fh == NULL) {   // sf_format_check is only for writes
        qWarning() << "libsndfile: Error opening file" << m_qFilename << sf_strerror(fh);
        return -1;
    }

    if (sf_error(fh)>0) {
        qWarning() << "libsndfile: Error opening file" << m_qFilename << sf_strerror(fh);
        return -1;
    }

    channels = info->channels;

    // This is the 'virtual' filelength. No matter how many channels the file
    // actually has, we pretend it has 2.
    filelength = 2*info->frames; // File length with two interleaved channels
    m_iSampleRate =  info->samplerate;
    m_bOpened = true;
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

            // readNo*2 is strictly less than available buffer space

            // rryan 2/2009
            // Mini-proof of the below:
            // size = 20, destination is a 20 element array 0-19
            // readNo = 10 (or less, but 10 in this case)
            // i = 10-1 = 9, so dest[9*2] and dest[9*2+1],
            // so the first iteration touches the very ends of destination
            // on the last iteration, dest[0] and dest[1] are assigned to dest[0]

            for(int i=(readNo-1); i>=0; i--) {
                dest[i*2]     = dest[i];
                dest[(i*2)+1] = dest[i];
            }

            // We doubled the readNo bytes we read into stereo.
            return readNo * 2;
        } else {
            // We do not support music with more than 2 channels.
            return 0;
        }
    }

    // The file has errors or is not open. Tell the truth and return 0.
    return 0;
}

int SoundSourceSndFile::parseHeader()
{
    QString location = getFilename();
    setType(location.section(".",-1).toLower());

    bool result;
    bool is_flac = location.endsWith("flac", Qt::CaseInsensitive);
    bool is_wav = location.endsWith("wav", Qt::CaseInsensitive);

#ifdef __WINDOWS__
    /* From Tobias: A Utf-8 string did not work on my Windows XP (German edition)
     * If you try this conversion, f.isValid() will return false in many cases
     * and processTaglibFile() will fail
     *
     * The method toLocal8Bit() returns the local 8-bit representation of the string as a QByteArray.
     * The returned byte array is undefined if the string contains characters not supported
     * by the local 8-bit encoding.
     */
    QByteArray qBAFilename = m_qFilename.toLocal8Bit();
#else
    QByteArray qBAFilename = m_qFilename.toUtf8();
#endif

    if (is_flac) {
        TagLib::FLAC::File f(qBAFilename.constData());
        result = processTaglibFile(f);
        TagLib::ID3v2::Tag* id3v2 = f.ID3v2Tag();
        TagLib::Ogg::XiphComment* xiph = f.xiphComment();
        if (id3v2) {
            processID3v2Tag(id3v2);
        }
        if (xiph) {
            processXiphComment(xiph);
        }
    } else if (is_wav) {
        TagLib::RIFF::WAV::File f(qBAFilename.constData());
        result = processTaglibFile(f);

        TagLib::ID3v2::Tag* id3v2 = f.tag();
        if (id3v2) {
            processID3v2Tag(id3v2);
        }

        if (getDuration() <= 0) {
            // we're using a taglib version which doesn't know how to do wav
            // durations, set it with info from sndfile -bkgood
            // XXX remove this when ubuntu ships with an sufficiently
            // intelligent version of taglib, should happen in 11.10

            // Have to open the file for info to be valid.
            if (!m_bOpened) {
                open();
            }

            if (info->samplerate > 0) {
                setDuration(info->frames / info->samplerate);
            } else {
                qDebug() << "WARNING: WAV file with invalid samplerate."
                         << "Can't get duration using libsndfile.";
            }
        }
    } else {
        // Try AIFF
        TagLib::RIFF::AIFF::File f(qBAFilename.constData());
        result = processTaglibFile(f);

        TagLib::ID3v2::Tag* id3v2 = f.tag();
        if (id3v2) {
            processID3v2Tag(id3v2);
        }
    }

    if (result)
        return OK;
    return ERR;
}

/*
   Return the length of the file in samples.
 */
inline long unsigned SoundSourceSndFile::length()
{
    return filelength;
}
