/***************************************************************************
                          soundsourceopus.cpp  -  Opus decoder
                             -------------------
    copyright            : (C) 2013 by Tuukka Pasanen
                           (C) 2003 by Svein Magne Bang
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

#include <taglib/opusfile.h>
#include <vorbis/codec.h>

#include "trackinfoobject.h"
#include "soundsourceopus.h"
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

SoundSourceOpus::SoundSourceOpus(QString qFilename)
: Mixxx::SoundSource(qFilename)
{
    filelength = 0;
}

SoundSourceOpus::~SoundSourceOpus()
{
    if (filelength > 0){
        op_free(vf);
    }
}

int SoundSourceOpus::open()
{
    int error = 0;
    QByteArray qBAFilename = m_qFilename.toLocal8Bit();
    
    qDebug() << m_qFilename;
    
    vf = op_open_file(qBAFilename.data(), &error);
    if( vf == NULL ) {
        qDebug() << "opus: Input does not appear to be an Opus bitstream.";
        filelength = 0;
        return ERR;
    }
    
    // lookup the ogg's channels and samplerate
    channels = op_channel_count(vf, -1);
    //m_iSampleRate = op_bitrate (vf, -1) ;
    // opusfile lib all ways gives you 48000 samplerate
    m_iSampleRate = 48000;

    qDebug() << m_qFilename << "chan:" << channels << "sample:" << m_iSampleRate;

    if(channels > 2){
        qDebug() << "opus: No support for more than 2 channels!";
        op_free(vf);
        filelength = 0;
        return ERR;
    }

    // ov_pcm_total returns the total number of frames in the ogg file. The
    // frame is the channel-independent measure of samples. The total samples in
    // the file is channels * ov_pcm_total. rryan 7/2009 I verified this by
    // hand. a 30 second long 48khz mono ogg and a 48khz stereo ogg both report
    // 1440000 for ov_pcm_total.
    ogg_int64_t ret = op_pcm_total (vf, -1);

    if (ret >= 0) {
        // We pretend that the file is stereo to the rest of the world.
        filelength = ret * 2;
    }
    else //error
    {
      if (ret == OV_EINVAL) {
          //The file is not seekable. Not sure if any action is needed.
          qDebug() << "opus: file is not seekable " << m_qFilename;
      }
    }

    return OK;
}

/*
   seek to <filepos>
 */

long SoundSourceOpus::seek(long filepos)
{
    // In our speak, filepos is a sample in the file abstraction (i.e. it's
    // stereo no matter what). filepos/2 is the frame we want to seek to.
    if (filepos % 2 != 0) {
        qDebug() << "SoundSourceOpus got non-even seek target.";
        filepos--;
    }

    if (op_seekable(vf)) {
        if (op_pcm_seek(vf, filepos/2) != 0) {
            // This is totally common (i.e. you're at EOF). Let's not leave this
            // qDebug on.

            // qDebug() << "ogg vorbis: Seek ERR on seekable.";
        }

        // Even if an error occured, return them the current position because
        // that's what we promised. (Double it because ov_pcm_tell returns
        // frames and we pretend to the world that everything is stereo)
        return op_pcm_tell(vf) * 2;
    } else {
        qDebug() << "ogg vorbis: Seek ERR at file " << m_qFilename;
        return 0;
    }
}


/*
   read <size> samples into <destination>, and return the number of
   samples actually read.
 */

unsigned SoundSourceOpus::read(volatile unsigned long size, const SAMPLE * destination) {
    if (size % 2 != 0) {
        qDebug() << "SoundSourceOpus got non-even size in read.";
        size--;
    }

    opus_int16 *pRead  = (opus_int16*) destination;
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
    unsigned int needed = size;

    unsigned int index=0,ret=0;

    // loop until requested number of samples has been retrieved
    while (needed > 0) {
        // read samples into buffer
        ret = op_read_stereo(vf, pRead+index, needed);
        qDebug() << "2# Levea pensseliseta! need" << needed << "! read" << ret << "/cur" << index;

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
        // rryan 2/2009
        // Mini-proof of the below:
        // size = 20, destination is a 20 element array 0-19
        // readNo = 10 (or less, but 10 in this case)
        // i = 10-1 = 9, so dest[9*2] and dest[9*2+1],
        // so the first iteration touches the very ends of destination
        // on the last iteration, dest[0] and dest[1] are assigned to dest[0]
        for(int i=(index/2-1); i>=0; i--) {
            dest[i*2]     = dest[i];
            dest[(i*2)+1] = dest[i];
        }

        // Pretend we read twice as many bytes as we did, since we just repeated
        // each pair of bytes.
        index *= 2;
    }

    // index is the total bytes read, so the words read is index/2
    return size;
}

/*
   Parse the the file to get metadata
 */
int SoundSourceOpus::parseHeader() {
    setType("opus");

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
    TagLib::Ogg::Opus::File f(qBAFilename.constData());

    // Takes care of all the default metadata
    bool result = processTaglibFile(f);


    TagLib::Ogg::XiphComment *tag = f.tag();

    if (tag) {
        processXiphComment(tag);
    }

    return result ? OK : ERR;
}

/*
   Return the length of the file in samples.
 */

inline long unsigned SoundSourceOpus::length()
{
    return filelength;
}

QList<QString> SoundSourceOpus::supportedFileExtensions()
{
    QList<QString> list;
    list.push_back("opus");
    return list;
}
