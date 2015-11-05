/****************************************************************************
                   encodermp3.cpp  - mp3 encoder for mixxx
                             -------------------
    copyright            : (C) 2007 by Wesley Stessens
                           (C) 2009 by Phillip Whelan (rewritten for mp3)
                           (C) 2010 by Tobias Rafreider (fixes for shoutcast, dynamic loading of lame_enc.dll, etc)
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
#include <QObject>
#include <limits.h>

#include "encoder/encodermp3.h"
#include "encoder/encodercallback.h"
#include "errordialoghandler.h"

EncoderMp3::EncoderMp3(EncoderCallback* pCallback)
  : m_lameFlags(NULL),
    m_metaDataTitle(NULL),
    m_metaDataArtist(NULL),
    m_metaDataAlbum(NULL),
    m_bufferOut(NULL),
    m_bufferOutSize(0),
    /*
     * @ Author: Tobias Rafreider
     * Nobody has initialized the field before my code review.  At runtime the
     * Integer field was inialized by a large random value such that the
     * following pointer fields were never initialized in the methods
     * 'bufferOutGrow()' and 'bufferInGrow()' --> Valgrind shows invalid writes
     * :-)
     *
     * m_bufferOut = (unsigned char *)realloc(m_bufferOut, size);
     * m_bufferIn[0] = (float *)realloc(m_bufferIn[0], size * sizeof(float));
     * m_bufferIn[1] = (float *)realloc(m_bufferIn[1], size * sizeof(float));
     *
     * This has solved many segfaults when using and even closing shoutcast
     * along with LAME.  This bug was detected by using Valgrind memory analyser
     *
     */
    m_bufferInSize(0),
    m_pCallback(pCallback),
    m_library(NULL) {
    m_bufferIn[0] = NULL;
    m_bufferIn[1] = NULL;

    //These are the function pointers for lame
    lame_init =  0;
    lame_set_num_channels = 0;
    lame_set_in_samplerate =  0;
    lame_set_out_samplerate = 0;
    lame_close = 0;
    lame_set_brate = 0;
    lame_set_mode = 0;
    lame_set_quality = 0;
    lame_set_bWriteVbrTag = 0;
    lame_encode_buffer_float = 0;
    lame_init_params = 0;
    lame_encode_flush = 0;

    id3tag_init= 0;
    id3tag_set_title = 0;
    id3tag_set_artist = 0;
    id3tag_set_album = 0;

    /*
     * Load shared library
     */
    QStringList libnames;
    QString libname = "";
#ifdef __LINUX__
    libnames << "mp3lame";
#elif __WINDOWS__
    libnames << "lame_enc.dll";
    libnames << "libmp3lame.dll";
#elif __APPLE__
    libnames << "/usr/local/lib/libmp3lame.dylib";
    //Using MacPorts (former DarwinPorts) results in ...
    libnames << "/opt/local/lib/libmp3lame.dylib";
#endif

    foreach (QString libname, libnames) {
        m_library = new QLibrary(libname, 0);
        if (m_library->load()) {
            qDebug() << "Successfully loaded encoder library " << libname;
            break;
        } else {
            qWarning() << "Failed to load " << libname << ", " << m_library->errorString();
        }
        delete m_library;
        m_library = NULL;
    }

    if (!m_library || !m_library->isLoaded()) {
        ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
        props->setType(DLG_WARNING);
        props->setTitle(QObject::tr("Encoder"));
        QString missingCodec = QObject::tr("<html>Mixxx cannot record or stream in MP3 without the MP3 encoder &quot;lame&quot;. Due to licensing issues, we cannot include this with Mixxx. To record or stream in MP3, you must download <b>libmp3lame</b> and install it on your system. <p>See <a href='http://mixxx.org/wiki/doku.php/internet_broadcasting#%1'>Mixxx Wiki</a> for more information. </html>");

#ifdef __LINUX__
        missingCodec = missingCodec.arg("linux");
#elif __WINDOWS__
        missingCodec = missingCodec.arg("windows");
#elif __APPLE__
        missingCodec = missingCodec.arg("mac_osx");
#endif
        props->setText(missingCodec);
        props->setKey(missingCodec);
        ErrorDialogHandler::instance()->requestErrorDialog(props);
        return;
    }

    typedef const char* (*get_lame_version__)(void);
    get_lame_version__ get_lame_version = (get_lame_version__)m_library->resolve("get_lame_version");


    //initalize function pointers
    lame_init                   = (lame_init__)m_library->resolve("lame_init");
    lame_set_num_channels       = (lame_set_num_channels__)m_library->resolve("lame_set_num_channels");
    lame_set_in_samplerate      = (lame_set_in_samplerate__)m_library->resolve("lame_set_in_samplerate");
    lame_set_out_samplerate     = (lame_set_out_samplerate__)m_library->resolve("lame_set_out_samplerate");
    lame_close                  = (lame_close__)m_library->resolve("lame_close");
    lame_set_brate              = (lame_set_brate__)m_library->resolve("lame_set_brate");
    lame_set_mode               = (lame_set_mode__)m_library->resolve("lame_set_mode");
    lame_set_quality            = (lame_set_quality__)m_library->resolve("lame_set_quality");
    lame_set_bWriteVbrTag       = (lame_set_bWriteVbrTag__)m_library->resolve("lame_set_bWriteVbrTag");
    lame_encode_buffer_float    = (lame_encode_buffer_float__)m_library->resolve("lame_encode_buffer_float");
    lame_init_params            = (lame_init_params__)m_library->resolve("lame_init_params");
    lame_encode_flush           = (lame_encode_flush__)m_library->resolve("lame_encode_flush");

    id3tag_init                 = (id3tag_init__)m_library->resolve("id3tag_init");
    id3tag_set_title            = (id3tag_set_title__)m_library->resolve("id3tag_set_title");
    id3tag_set_artist           = (id3tag_set_artist__)m_library->resolve("id3tag_set_artist");
    id3tag_set_album            = (id3tag_set_album__)m_library->resolve("id3tag_set_album");


      /*
     * Check if all function pointers are not NULL
     * Otherwise, the lame_enc.dll, libmp3lame.so or libmp3lame.mylib do not comply with the official header lame.h
     * Indicates a modified lame version
     *
     * Should not happend on Linux, but many lame binaries for Windows are modified.
     */
    if(!lame_init ||
       !lame_set_num_channels ||
       !lame_set_in_samplerate ||
       !lame_set_out_samplerate ||
       !lame_close ||
       !lame_set_brate ||
       !lame_set_mode ||
       !lame_set_quality ||
       !lame_set_bWriteVbrTag ||
       !lame_encode_buffer_float ||
       !lame_init_params ||
       !lame_encode_flush ||
       !get_lame_version ||
       !id3tag_init ||
       !id3tag_set_title ||
       !id3tag_set_artist ||
       !id3tag_set_album) {
        m_library->unload();
        m_library = NULL;
        //print qDebugs to detect which function pointers are null
        qDebug() << "lame_init: " << lame_init;
        qDebug() << "lame_set_num_channels: " << lame_set_num_channels;
        qDebug() << "lame_set_in_samplerate: " << lame_set_in_samplerate;
        qDebug() << "lame_set_out_samplerate: " << lame_set_out_samplerate;
        qDebug() << "lame_close: " << lame_close;
        qDebug() << "lame_set_brate " << lame_set_brate;
        qDebug() << "lame_set_mode: " << lame_set_mode;
        qDebug() << "lame_set_quality: " << lame_set_quality;
        qDebug() << "lame_set_bWriteVbrTag: " << lame_set_bWriteVbrTag;
        qDebug() << "lame_encode_buffer_float: " << lame_encode_buffer_float;
        qDebug() << "lame_init_params: " << lame_init_params;
        qDebug() << "lame_encode_flush: " << lame_encode_flush;
        qDebug() << "get_lame_version: " << get_lame_version;
        qDebug() << "id3tag_init: " << id3tag_init;
        qDebug() << "id3tag_set_title : " << id3tag_set_title ;
        qDebug() << "id3tag_set_artist: " << id3tag_set_artist;
        qDebug() << "id3tag_set_album  " << id3tag_set_album ;

        ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
        props->setType(DLG_WARNING);
        props->setTitle(QObject::tr("Encoder"));
        QString key = QObject::tr("<html>Mixxx has detected that you use a modified version of libmp3lame. See <a href='http://mixxx.org/wiki/doku.php/internet_broadcasting'>Mixxx Wiki</a> for more information.</html>");
        props->setText(key);
        props->setKey(key);
        ErrorDialogHandler::instance()->requestErrorDialog(props);
        return;
    }
    qDebug() << "Loaded libmp3lame version " << get_lame_version();
}

// Destructor
EncoderMp3::~EncoderMp3() {
    if (m_library != NULL && m_library->isLoaded()) {
        flush();
        lame_close(m_lameFlags);
        m_library->unload(); //unload dll, so, ...
        qDebug() << "Unloaded libmp3lame ";
        m_library = NULL;
    }
    //free requested buffers
    if (m_bufferIn[0] != NULL)
        delete m_bufferIn[0];
    if (m_bufferIn[1] != NULL)
        delete m_bufferIn[1];
    if (m_bufferOut != NULL)
        delete m_bufferOut;

    lame_init =  0;
    lame_set_num_channels = 0;
    lame_set_in_samplerate =  0;
    lame_set_out_samplerate = 0;
    lame_close = 0;
    lame_set_brate = 0;
    lame_set_mode = 0;
    lame_set_quality = 0;
    lame_set_bWriteVbrTag = 0;
    lame_encode_buffer_float = 0;
    lame_init_params = 0;
    lame_encode_flush = 0;

    id3tag_init= 0;
    id3tag_set_title = 0;
    id3tag_set_artist = 0;
    id3tag_set_album = 0;
}

/*
 * Grow the outBuffer if needed.
 */

int EncoderMp3::bufferOutGrow(int size) {
    if (m_bufferOutSize >= size)
        return 0;

    m_bufferOut = (unsigned char *)realloc(m_bufferOut, size);
    if (m_bufferOut == NULL)
        return -1;

    m_bufferOutSize = size;
    return 0;
}

/*
 * Grow the inBuffer(s) if needed.
 */

int EncoderMp3::bufferInGrow(int size) {
    if (m_bufferInSize >= size)
        return 0;

    m_bufferIn[0] = (float *)realloc(m_bufferIn[0], size * sizeof(float));
    m_bufferIn[1] = (float *)realloc(m_bufferIn[1], size * sizeof(float));
    if ((m_bufferIn[0] == NULL) || (m_bufferIn[1] == NULL))
        return -1;

    m_bufferInSize = size;
    return 0;
}

//Using this method requires to call method 'write()' or 'sendPackages()'
//depending on which context you use the class (shoutcast or recording to HDD)
void EncoderMp3::flush() {
    if (m_library == NULL || !m_library->isLoaded())
        return;
    int rc = 0;
    /**Flush also writes ID3 tags **/
    rc = lame_encode_flush(m_lameFlags, m_bufferOut, m_bufferOutSize);
    if (rc < 0) {
        return;
    }
    //end encoded audio to shoutcast or file
    m_pCallback->write(NULL, m_bufferOut, 0, rc);
}

void EncoderMp3::encodeBuffer(const CSAMPLE *samples, const int size) {
    if (m_library == NULL || !m_library->isLoaded())
        return;
    int outsize = 0;
    int rc = 0;

    outsize = (int)((1.25 * size + 7200) + 1);
    bufferOutGrow(outsize);

    bufferInGrow(size);

    // Deinterleave samples. We use normalized floats in the engine [-1.0, 1.0]
    // but LAME expects samples in the range [SHRT_MIN, SHRT_MAX].
    for (int i = 0; i < size/2; ++i) {
        m_bufferIn[0][i] = samples[i*2] * SHRT_MAX;
        m_bufferIn[1][i] = samples[i*2+1] * SHRT_MAX;
    }

    rc = lame_encode_buffer_float(m_lameFlags, m_bufferIn[0], m_bufferIn[1],
                                  size/2, m_bufferOut, m_bufferOutSize);
    if (rc < 0) {
        return;
    }
    //write encoded audio to shoutcast stream or file
    m_pCallback->write(NULL, m_bufferOut, 0, rc);
}

void EncoderMp3::initStream() {
    m_bufferOutSize = (int)((1.25 * 20000 + 7200) + 1);
    m_bufferOut = (unsigned char *)malloc(m_bufferOutSize);

    m_bufferIn[0] = (float *)malloc(m_bufferOutSize * sizeof(float));
    m_bufferIn[1] = (float *)malloc(m_bufferOutSize * sizeof(float));
    return;
}

int EncoderMp3::initEncoder(int bitrate, int samplerate) {
    if (m_library == NULL || !m_library->isLoaded())
        return -1;

    unsigned long samplerate_in = samplerate;
    unsigned long samplerate_out =
            (samplerate_in > 48000 ? 48000 : samplerate_in);

    m_lameFlags = lame_init();

    if (m_lameFlags == NULL) {
        qDebug() << "Unable to initialize MP3";
        return -1;
    }

    lame_set_num_channels(m_lameFlags, 2);
    lame_set_in_samplerate(m_lameFlags, samplerate_in);
    lame_set_out_samplerate(m_lameFlags, samplerate_out);
    lame_set_brate(m_lameFlags, bitrate);
    lame_set_mode(m_lameFlags, STEREO);
    lame_set_quality(m_lameFlags, 2);
    lame_set_bWriteVbrTag(m_lameFlags, 0);

    //ID3 Tag if fiels are not NULL
    id3tag_init(m_lameFlags);
    if (m_metaDataTitle)
        id3tag_set_title(m_lameFlags, m_metaDataTitle);
    if (m_metaDataArtist)
        id3tag_set_artist(m_lameFlags, m_metaDataArtist);
    if (m_metaDataAlbum)
        id3tag_set_album(m_lameFlags,m_metaDataAlbum);


    if ((lame_init_params(m_lameFlags)) < 0) {
        qDebug() << "Unable to initialize MP3 parameters";
        return -1;
    }

    initStream();

    return 0;
}

void EncoderMp3::updateMetaData(char* artist, char* title, char* album) {
    m_metaDataTitle = title;
    m_metaDataArtist = artist;
    m_metaDataAlbum = album;
}
