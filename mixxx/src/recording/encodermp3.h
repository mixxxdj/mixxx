/****************************************************************************
                   encodermp3.h  - mp3 encoder for mixxx
                             -------------------
    copyright            : (C) 2007 by Wesley Stessens
                           (C) 2009 by Phillip Whelan (rewritten for mp3)
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
 * We load the library exlicitly to avoid legal issues
 * On Linux make sure the file libmp3lame.so is in /usr/lib.
 * If not use your packet manager to install.
 * You might need to create a hard-link with ln -l if your distro names the file other than libmp3lame.so
 */

class EngineAbstractRecord;
class TrackInfoObject;

class EncoderMp3 : public Encoder {

public:
    EncoderMp3(ConfigObject<ConfigValue> *_config, EngineAbstractRecord *engine=0);
    ~EncoderMp3();
    int initEncoder(int bitrate);
    void encodeBuffer(const CSAMPLE *samples, const int size);
	void updateMetaData(char* artist, char* title){};
	void flush();
	//Call this method in conjunction with shoutcast streaming
	void sendPackages();
	
	/*
	 * If we record to hard disk drive
	 */

	//creates or opens an audio file
	void openFile();
	//writes encoded audio to file
	void writeFile();
	//closes the audio file    
	void closeFile();
private:
    
    void initStream();
    int bufferOutGrow(int size);
    int bufferInGrow(int size);
	int rc; //size of encoded samples

    ConfigObject<ConfigValue> *m_pConfig; /* provides ConfigKey access */
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
	typedef int (*lame_encode_flush_nogap__)(lame_global_flags *  gfp,    /* global context handle                 */
        unsigned char*       mp3buf, /* pointer to encoded MP3 stream         */
        int                  size);  /* number of valid octets in this stream */
	typedef int (*lame_encode_buffer_float__)(lame_global_flags*  gfp,        /* global context handle         */
        const float     buffer_l [],       /* PCM data for left channel     */
        const float     buffer_r [],       /* PCM data for right channel    */
        const int           nsamples,      /* number of samples per channel */
        unsigned char*      mp3buf,        /* pointer to encoded MP3 stream */
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
	lame_encode_flush_nogap__			lame_encode_flush_nogap;
	lame_encode_buffer_float__			lame_encode_buffer_float;



    unsigned char *m_bufferOut;
    int m_bufferOutSize;
    float *m_bufferIn[2];
    int m_bufferInSize;
    
    EngineAbstractRecord *pEngine;
    TrackInfoObject *m_pMetaData;
    char *metaDataTitle;
    char *metaDataArtist;
	QLibrary* m_library;
	QFile mp3file;
};

#endif
