/***************************************************************************
                          soundsourceoggvorbis.cpp  -  ogg vorbis decoder
                             -------------------
    copyright            : (C) 2003 by Svein Magne Bang
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

#include "soundsourceoggvorbis.h"
#include "soundsourcetaglib.h"
#include "sampleutil.h"

#include <taglib/vorbisfile.h>

#include <vorbis/codec.h>

#include <QtDebug>

#ifdef __WINDOWS__
#include <io.h>
#include <fcntl.h>
#elif __APPLE__
#include <CoreFoundation/CFByteOrder.h>
#elif __LINUX__
#include <endian.h>
#endif

inline int getByteOrder() {
#ifdef __LINUX__
    return __BYTE_ORDER == __LITTLE_ENDIAN ? 0 : 1;
#elif __APPLE__
    return CFByteOrderGetCurrent() == CFByteOrderLittleEndian ? 0 : 1;
#else
    // Assume little endian.
    // TODO(XXX) BSD.
    return 0;
#endif

}

/*
   Class for reading Ogg Vorbis
 */

SoundSourceOggVorbis::SoundSourceOggVorbis(QString qFilename)
: Mixxx::SoundSource(qFilename)
{
    filelength = 0;
    setType("ogg");
}

SoundSourceOggVorbis::~SoundSourceOggVorbis()
{
    if (filelength > 0) {
        ov_clear(&vf);
    }
}

Result SoundSourceOggVorbis::open() {
    const QByteArray qBAFilename(getFilename().toLocal8Bit());
#ifdef __WINDOWS__
    if(ov_fopen(qBAFilename.constData(), &vf) < 0) {
        qDebug() << "oggvorbis: Input does not appear to be an Ogg bitstream.";
        filelength = 0;
        return ERR;
    }
#else
    FILE *vorbisfile =  fopen(qBAFilename.constData(), "r");

    if (!vorbisfile) {
        qDebug() << "oggvorbis: cannot open" << getFilename();
        return ERR;
    }

    if(ov_open(vorbisfile, &vf, NULL, 0) < 0) {
        qDebug() << "oggvorbis: Input does not appear to be an Ogg bitstream.";
        filelength = 0;
        return ERR;
    }
#endif

    // lookup the ogg's channels and samplerate
    vorbis_info * vi = ov_info(&vf, -1);

    channels = vi->channels;
    setChannels(channels);
    setSampleRate(vi->rate);

    if (channels > 2) {
        qDebug() << "oggvorbis: No support for more than 2 channels!";
        ov_clear(&vf);
        filelength = 0;
        return ERR;
    }

    // ov_pcm_total returns the total number of frames in the ogg file. The
    // frame is the channel-independent measure of samples. The total samples in
    // the file is channels * ov_pcm_total. rryan 7/2009 I verified this by
    // hand. a 30 second long 48khz mono ogg and a 48khz stereo ogg both report
    // 1440000 for ov_pcm_total.
    ogg_int64_t ret = ov_pcm_total(&vf, -1);

    if (ret >= 0) {
        // We pretend that the file is stereo to the rest of the world.
        filelength = ret * 2;
    }
    else //error
    {
      if (ret == OV_EINVAL) {
          //The file is not seekable. Not sure if any action is needed.
          qDebug() << "oggvorbis: file is not seekable " << getFilename();
      }
    }

    return OK;
}

/*
   seek to <filepos>
 */

long SoundSourceOggVorbis::seek(long filepos)
{
    // In our speak, filepos is a sample in the file abstraction (i.e. it's
    // stereo no matter what). filepos/2 is the frame we want to seek to.
    if (filepos % 2 != 0) {
        qDebug() << "SoundSourceOggVorbis got non-even seek target.";
        filepos--;
    }

    if (ov_seekable(&vf)) {
        if (ov_pcm_seek(&vf, filepos/2) != 0) {
            // This is totally common (i.e. you're at EOF). Let's not leave this
            // qDebug on.

            // qDebug() << "ogg vorbis: Seek ERR on seekable.";
        }

        // Even if an error occured, return them the current position because
        // that's what we promised. (Double it because ov_pcm_tell returns
        // frames and we pretend to the world that everything is stereo)
        return ov_pcm_tell(&vf) * 2;
    } else {
        qDebug() << "ogg vorbis: Seek ERR at file " << getFilename();
        return 0;
    }
}


/*
   read <size> samples into <destination>, and return the number of
   samples actually read.
 */

unsigned SoundSourceOggVorbis::read(volatile unsigned long size, const SAMPLE * destination) {
    if (size % 2 != 0) {
        qDebug() << "SoundSourceOggVorbis got non-even size in read.";
        size--;
    }

    char *pRead  = (char*) destination;
    SAMPLE *dest   = (SAMPLE*) destination;



    // 'needed' is size of buffer in bytes. 'size' is size in SAMPLEs,
    // which is 2 bytes.  If the stream is mono, we read 'size' bytes,
    // so that half the buffer is full, then below we double each
    // sample on the left and right channel. If the stream is stereo,
    // then ov_read interleaves the samples into the full length of
    // the buffer.

    // ov_read speaks bytes, we speak words.  needed is the bytes to
    // read, not words to read.

    // size is the maximum space in words that we have in
    // destination. For stereo files, read the full buffer (size*2
    // bytes). For mono files, only read half the buffer (size bytes),
    // and we will double the buffer to be in stereo later.
    unsigned int needed = size * channels;

    unsigned int index=0,ret=0;

    // loop until requested number of samples has been retrieved
    while (needed > 0) {
        // read samples into buffer
        ret = ov_read(&vf, pRead+index, needed, getByteOrder(), 2, 1, &current_section);

        if (ret <= 0) {
            // An error or EOF occured, break out and return what we have sofar.
            break;
        }

        index  += ret;
        needed -= ret;
    }

    // As of here, index is the total bytes read. (index/2/channels) is the
    // total frames read.

    // convert into stereo if file is mono
    if (channels == 1) {
        SampleUtil::doubleMonoToDualMono(dest, index / 2);
        // Pretend we read twice as many bytes as we did, since we just repeated
        // each pair of bytes.
        index *= 2;
    }

    // index is the total bytes read, so the words read is index/2
    return index / 2;
}

/*
   Parse the the file to get metadata
 */
Result SoundSourceOggVorbis::parseHeader() {
    QByteArray qBAFilename = getFilename().toLocal8Bit();
    TagLib::Ogg::Vorbis::File f(qBAFilename.constData());

    if (!readFileHeader(this, f)) {
        return ERR;
    }

    TagLib::Ogg::XiphComment *xiph = f.tag();
    if (xiph) {
        readXiphComment(this, *xiph);
    } else {
        // fallback
        const TagLib::Tag *tag(f.tag());
        if (tag) {
            readTag(this, *tag);
        } else {
            return ERR;
        }
    }

    return OK;
}

QImage SoundSourceOggVorbis::parseCoverArt() {
    TagLib::Ogg::Vorbis::File f(getFilename().toLocal8Bit().constData());
    TagLib::Ogg::XiphComment *xiph = f.tag();
    if (xiph) {
        return Mixxx::getCoverInXiphComment(*xiph);
    } else {
        return QImage();
    }
}

/*
   Return the length of the file in samples.
 */

inline long unsigned SoundSourceOggVorbis::length()
{
    return filelength;
}

QList<QString> SoundSourceOggVorbis::supportedFileExtensions()
{
    QList<QString> list;
    list.push_back("ogg");
    return list;
}
