/***************************************************************************
                          soundsourceopus.cpp  -  Opus decoder
                             -------------------
    copyright            : (C) 2013 by Tuukka Pasanen
                           (C) 2003 by Svein Magne Bang (Based on Ogg Vorbis Code)
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
   Class for reading Xiph Opus
 */

SoundSourceOpus::SoundSourceOpus(QString qFilename)
    : Mixxx::SoundSource(qFilename) {
    m_lFilelength = 0;
}

SoundSourceOpus::~SoundSourceOpus() {
    if (m_lFilelength > 0) {
        op_free(m_ptrOpusFile);
    }
}

int SoundSourceOpus::open() {
    int error = 0;
    QByteArray qBAFilename = m_qFilename.toLocal8Bit();

    qDebug() << m_qFilename;

    m_ptrOpusFile = op_open_file(qBAFilename.data(), &error);
    if ( m_ptrOpusFile == NULL ) {
        qDebug() << "opus: Input does not appear to be an Opus bitstream.";
        m_lFilelength = 0;
        return ERR;
    }

    // opusfile lib all ways gives you 48000 samplerate and stereo 16 bit sample
    m_iChannels = 2;
    this->setBitrate((int)op_bitrate_instant(m_ptrOpusFile));
    this->setSampleRate(48000);
    this->setChannels(m_iChannels);


    if (m_iChannels > 2) {
        qDebug() << "opus: No support for more than 2 m_iChannels!";
        op_free(m_ptrOpusFile);
        m_lFilelength = 0;
        return ERR;
    }

    // op_pcm_total returns the total number of frames in the ogg file. The
    // frame is the channel-independent measure of samples. The total samples in
    // the file is m_iChannels * ov_pcm_total. rryan 7/2009 I verified this by
    // hand. a 30 second long 48khz mono ogg and a 48khz stereo ogg both report
    // 1440000 for op_pcm_total.
    int64_t ret = op_pcm_total(m_ptrOpusFile, -1) * 2;

    // qDebug() << m_qFilename << "chan:" << m_iChannels << "sample:" << m_iSampleRate << "LEN:" << ret;


    if (ret >= 0) {
        // We pretend that the file is stereo to the rest of the world.
        m_lFilelength = ret;
    } else { //error
        if (ret == OP_EINVAL) {
            //The file is not seekable. Not sure if any action is needed.
            qDebug() << "opus: file is not seekable " << m_qFilename;
        }
    }

    return OK;
}

/*
   seek to <filepos>
 */

long SoundSourceOpus::seek(long filepos) {
    // In our speak, filepos is a sample in the file abstraction (i.e. it's
    // stereo no matter what). filepos/2 is the frame we want to seek to.
    if (filepos % 2 != 0) {
        qDebug() << "SoundSourceOpus got non-even seek target.";
        filepos--;
    }

    if (op_seekable(m_ptrOpusFile)) {
        // I can't say why filepos have to divide by two
        // Have no idea.. probably seek point is mono..
        if (op_pcm_seek(m_ptrOpusFile, filepos / 2) != 0) {
            // This is totally common (i.e. you're at EOF). Let's not leave this
            // qDebug on.

            qDebug() << "opus: Seek ERR on seekable.";
        }

        // qDebug() << "Wanted:" << filepos << "GOT:" << op_pcm_tell(m_ptrOpusFile);


        //return op_pcm_tell(m_ptrOpusFile);
        // We are here allways!
        return filepos;
    } else {
        qDebug() << "opus: Seek ERR at file " << m_qFilename;
        return 0;
    }
    return filepos;
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

    // SAMPLE and opus_int16 are mostly same
    // So just make pointer and hope for the best
    opus_int16 *l_iDest = (opus_int16 *) destination;

    unsigned int l_iNeeded = size;
    unsigned int l_iReaded = 0;
    unsigned int l_iRet=0;

    // loop until requested number of samples has been retrieved
    while (l_iNeeded > 0) {
        // read samples into buffer
        //ret = op_read_stereo(m_ptrOpusFile, l_iPcm, sizeof(l_iPcm)/sizeof(*l_iPcm));
        l_iRet = op_read_stereo(m_ptrOpusFile, l_iDest, l_iNeeded);

        if (l_iRet <= 0) {
            // An error or EOF occured, break out and return what we have sofar.
            break;
        }

        l_iNeeded -= l_iRet * 2;
        l_iReaded += l_iRet * 2;
        l_iDest += l_iRet * 2;
    }

    return l_iReaded;
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

inline long unsigned SoundSourceOpus::length() {
    return m_lFilelength;
}

QList<QString> SoundSourceOpus::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("opus");
    return list;
}
