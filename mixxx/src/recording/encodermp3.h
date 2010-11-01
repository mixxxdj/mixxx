/****************************************************************************
                   encodermp3.h  - mp3 encoder for mixxx
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

#ifndef ENCODERMP3_H
#define ENCODERMP3_H

#include <QObject>
#include "defs.h"
#include "configobject.h"
#include "encoder.h"

///#include <lame/lame.h> // may be elsewhere on other distros besides Ubuntu
#include <QLibrary>
/*
 * We load the library explicitly to avoid legal issues
 * On Linux make sure the file libmp3lame.so is in /usr/lib.
 * If not use your package manager to install.
 * You might need to create a hard-link with ln -l if your distro names the file other than libmp3lame.so
 */

class EngineAbstractRecord;
class TrackInfoObject;

class EncoderMp3 : public Encoder {
    Q_OBJECT
  public:
    EncoderMp3(EngineAbstractRecord *engine=0);
    virtual ~EncoderMp3();
    int initEncoder(int bitrate);
    /** encodes audio and calls engine->write to send it a file or over a network **/
    void encodeBuffer(const CSAMPLE *samples, const int size);
    /** only used for audio recording. Adds metadata to the encoded auio, i.e., the ID3 tag **/
    void updateMetaData(char* artist, char* title, char* album);
    /** called at the end when encoding is finished **/
    void flush();

  private:
    void initStream();
    int bufferOutGrow(int size);
    int bufferInGrow(int size);

   
    //For lame
    struct lame_global_struct;
    typedef struct lame_global_struct lame_global_flags;
    typedef lame_global_flags *lame_t;
    lame_global_flags *m_lameFlags;

    /* MPEG modes */
    typedef enum MPEG_mode_e {
        STEREO = 0,
        JOINT_STEREO,
        DUAL_CHANNEL,   /* LAME doesn't supports this! */
        MONO,
        NOT_SET,
        MAX_INDICATOR   /* Don't use this! It's used for sanity checks. */
    } MPEG_mode;

    //Function pointer for lame
    typedef lame_global_flags* (*lame_init__)(void);
    typedef int (*lame_set_num_channels__)(lame_global_flags *, int);
    typedef int (*lame_set_in_samplerate__)(lame_global_flags *, int);
    typedef int (*lame_set_out_samplerate__)(lame_global_flags *, int);
    typedef int (*lame_set_brate__)(lame_global_flags *, int);
    typedef int (*lame_set_mode__)(lame_global_flags *, MPEG_mode);
    typedef int (*lame_set_quality__)(lame_global_flags *, int);
    typedef int (*lame_set_bWriteVbrTag__)(lame_global_flags *, int);
    typedef int (*lame_init_params__)(lame_global_flags *);
    typedef int (*lame_close__)(lame_global_flags *);
    typedef int (*lame_encode_flush__)(
        lame_global_flags *  gfp,				/* global context handle                 */
        unsigned char*       mp3buf,			/* pointer to encoded MP3 stream         */
        int                  size);				/* number of valid octets in this stream */
    typedef int (*lame_encode_buffer_float__)(
        lame_global_flags*  gfp,				/* global context handle         */
        const float     	buffer_l [],		/* PCM data for left channel     */
        const float     	buffer_r [],		/* PCM data for right channel    */
        const int           nsamples,			/* number of samples per channel */
        unsigned char*      mp3buf,				/* pointer to encoded MP3 stream */
        const int           mp3buf_size );

    lame_init__ 						lame_init;
    lame_set_num_channels__ 			lame_set_num_channels;
    lame_set_in_samplerate__ 			lame_set_in_samplerate;
    lame_set_out_samplerate__			lame_set_out_samplerate;
    lame_set_brate__					lame_set_brate;
    lame_set_mode__						lame_set_mode;
    lame_set_quality__					lame_set_quality;
    lame_set_bWriteVbrTag__				lame_set_bWriteVbrTag;
    lame_init_params__					lame_init_params;
    lame_close__						lame_close;
    lame_encode_flush__					lame_encode_flush;
    lame_encode_buffer_float__			lame_encode_buffer_float;

    // Function pointers for ID3 Tags
    typedef void (*id3tag_init__)(lame_global_flags *);
    typedef void (*id3tag_set_title__)(lame_global_flags *, const char* title);
    typedef void (*id3tag_set_artist__)(lame_global_flags *, const char* artist);
    typedef void (*id3tag_set_album__)(lame_global_flags *, const char* album);

    id3tag_init__						id3tag_init;
    id3tag_set_title__					id3tag_set_title;
    id3tag_set_artist__					id3tag_set_artist;
    id3tag_set_album__					id3tag_set_album;

    char *m_metaDataTitle;
    char *m_metaDataArtist;
    char *m_metaDataAlbum;

    unsigned char *m_bufferOut;
    int m_bufferOutSize;
    float *m_bufferIn[2];
    int m_bufferInSize;

    EngineAbstractRecord *m_pEngine;
    TrackPointer m_pMetaData;
    QLibrary* m_library;
    QFile m_mp3file;
    ControlObjectThread* m_samplerate;
};

#endif
