/****************************************************************************
                   encodermp3.cpp  - mp3 encoder for mixxx
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

#include <stdlib.h> // needed for random num     
#include <time.h> // needed for random num     
#include <string.h> // needed for memcpy
#include <QDebug>

#include "encodermp3.h"
#include "../engine/engineabstractrecord.h"
#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "playerinfo.h"
#include "trackinfoobject.h"
#include "defs_recording.h"
#include "errordialoghandler.h"

// Constructor
EncoderMp3::EncoderMp3(ConfigObject<ConfigValue> *_config, EngineAbstractRecord *engine)
{
    pEngine = engine;
    metaDataTitle = metaDataArtist = "";
    m_pMetaData = NULL;
    m_bufferIn[0] = NULL;
    m_bufferIn[1] = NULL;
    m_bufferOut = NULL;
    m_bufferOutSize = 0;
    m_lameFlags = NULL;
    m_pConfig = _config;
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
	lame_encode_flush_nogap = 0;
		
	/*
	 * @ Author: Tobias Rafreider
	 * Nobody has initialized the field before my code review.
     * At runtime the Integer field was inialized by a large random value
     * such that the following pointer fields were never initialized in the
	 * methods 'bufferOutGrow()' and 'bufferInGrow()' --> Valgrind shows invalid writes :-)
	 * 
     * m_bufferOut = (unsigned char *)realloc(m_bufferOut, size);
	 * m_bufferIn[0] = (float *)realloc(m_bufferIn[0], size * sizeof(float));
     * m_bufferIn[1] = (float *)realloc(m_bufferIn[1], size * sizeof(float));
	 *
     * This has solved many segfaults when using and even closing shoutcast along with LAME. 
	 * This bug was detected by using Valgrind memory analyser 
     *
    */
	m_bufferInSize = 0; 

	/*
	 * Load shared library
	 */
	QString libname = "";
#ifdef __LINUX__
	libname = "/usr/lib/libmp3lame.so";
#elif __WINDOWS__
	libname = "lame_enc.dll";
#elif __APPLE__
	libname ="lame_enc.dll"; //TODO
#endif

	m_library = new QLibrary(libname);

	if(!m_library->load()){
		ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
		props->setType(DLG_WARNING);
		props->setTitle("Encoder");
		QString key = "";
	#ifdef __LINUX__
		key = "<html>Mixxx is unable to load or find the MP3 encoder lame. <p>Please install libmp3lame (also known as lame) and check if /usr/lib/libmp3lame.so exists on your system </html>";
		props->setText(key);
	#elif __WINDOWS__
		key = "<html>Mixxx is unable to load or find the MP3 encoder lame. <p>Please put lame_enc.dll in the directory you have installed Mixxx </html>";
		props->setText(key);
	#elif __APPLE__
		key = "<html>Mixxx is unable to load or find the MP3 encoder lame. <p>Please put lame_enc.dll in the directory you have installed Mixxx </html>";
		props->setText(key);
#endif;
	    props->setKey(key);
		ErrorDialogHandler::instance()->requestErrorDialog(props);
		return;
	}
	
	typedef const char* (*get_lame_version__)(void);
	get_lame_version__ get_lame_version = (get_lame_version__)m_library->resolve("get_lame_version");
	
	
	//initalize function pointers
	lame_init =  (lame_init__)m_library->resolve("lame_init");
	//qDebug() << "lame_init  " << &lame_init ;
	lame_set_num_channels =(lame_set_num_channels__)m_library->resolve("lame_set_num_channels");
	//qDebug() << "lame_set_num_channels  " << &lame_set_num_channels ;
	lame_set_in_samplerate = (lame_set_in_samplerate__)m_library->resolve("lame_set_in_samplerate");
	//qDebug() << "lame_set_in_samplerate  " << &lame_set_in_samplerate ;
	lame_set_out_samplerate = (lame_set_out_samplerate__)m_library->resolve("lame_set_out_samplerate");
	//qDebug() << "lame_set_out_samplerate  " << &lame_set_out_samplerate ;
	lame_close = (lame_close__)m_library->resolve("lame_close");
	//qDebug() << "lame_close" << &lame_close;
	lame_set_brate = (lame_set_brate__)m_library->resolve("lame_set_brate");
	//qDebug() << "lame_set_brate" << &lame_set_brate;
	lame_set_mode = (lame_set_mode__)m_library->resolve("lame_set_mode");
	//qDebug() << "lame_set_mode" << &lame_set_mode;
	lame_set_quality = (lame_set_quality__)m_library->resolve("lame_set_quality");
	//qDebug() << "lame_set_quality" << &lame_set_quality;
	lame_set_bWriteVbrTag = (lame_set_bWriteVbrTag__)m_library->resolve("lame_set_bWriteVbrTag");
	//qDebug() << "lame_set_bWriteVbrTag " << &lame_set_bWriteVbrTag ;
	lame_encode_buffer_float = (lame_encode_buffer_float__)m_library->resolve("lame_encode_buffer_float");
	//qDebug() << "lame_encode_buffer_float " << &lame_encode_buffer_float ;
	lame_init_params = (lame_init_params__)m_library->resolve("lame_init_params");
	//qDebug() << "lame_init_params " << &lame_init_params;
	lame_encode_flush_nogap = (lame_encode_flush_nogap__)m_library->resolve("lame_encode_flush_nogap");
	//qDebug() << "lame_encode_flush_nogap " << &lame_encode_flush_nogap;

	//Check if all function pointer are not NULL

	if(	!lame_init ||
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
		!lame_encode_flush_nogap ||
		!get_lame_version
	)
	{
		qDebug() << "Error loading liblame: Function pointer may be NULL";
		m_library->unload();
		m_library = NULL;
		return;
	}	
		
	qDebug() << "Loaded liblame version " << get_lame_version();	
}

// Destructor
EncoderMp3::~EncoderMp3()
{
	if(m_library != NULL && m_library->isLoaded()){
    	flush();
    	lame_close(m_lameFlags);
		m_library->unload(); //unload dll, so, ...
		m_library = NULL;
	}
	//free requested buffers
	if(m_bufferIn[0] != NULL) delete m_bufferIn[0];
    if(m_bufferIn[1] != NULL) delete m_bufferIn[1];
	if(m_bufferOut != NULL) delete m_bufferOut;
	
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
	lame_encode_flush_nogap = 0;
	//close mp3 file, if open	
	closeFile();
	
}

/*
 * Grow the outBuffer if needed.
 */

int EncoderMp3::bufferOutGrow(int size)
{
    if ( m_bufferOutSize >= size )
        return 0;
    
    m_bufferOut = (unsigned char *)realloc(m_bufferOut, size);
    if ( m_bufferOut == NULL )
        return -1;
    
    m_bufferOutSize = size;
    return 0;
}

/*
 * Grow the inBuffer(s) if needed.
 */

int EncoderMp3::bufferInGrow(int size)
{
	
    if ( m_bufferInSize >= size )
        return 0;
    
    m_bufferIn[0] = (float *)realloc(m_bufferIn[0], size * sizeof(float));
    m_bufferIn[1] = (float *)realloc(m_bufferIn[1], size * sizeof(float));
    if ((m_bufferIn[0] == NULL) || (m_bufferIn[1] == NULL))
        return -1;
    
    m_bufferInSize = size;
    return 0;
}
//Using this method requires to call method 'write()' or 'sendPackages()' depending on which context you use the class (shoutcast or recording to HDD)
void EncoderMp3::flush()
{
	if(m_library == NULL || !m_library->isLoaded())
		return;
    rc = 0;
 
    rc = lame_encode_flush_nogap(m_lameFlags, m_bufferOut, m_bufferOutSize);
}

void EncoderMp3::encodeBuffer(const CSAMPLE *samples, const int size)
{
	if(m_library == NULL || !m_library->isLoaded())
		return;
    int outsize = 0;
    rc = 0;
    int i = 0;
    
    outsize = (int)((1.25 * size + 7200) + 1);
    bufferOutGrow(outsize);
    
    bufferInGrow(size);
    
    // Deinterleave samples
    for (i = 0; i < size/2; ++i)
    {
        m_bufferIn[0][i] = samples[i*2];
        m_bufferIn[1][i] = samples[i*2+1];
    }
    
    rc = lame_encode_buffer_float(m_lameFlags, m_bufferIn[0], m_bufferIn[1], 
            size/2, m_bufferOut, m_bufferOutSize);
    if ( rc < 0 )
        return;
 
}
void EncoderMp3::sendPackages(){
	if(m_library == NULL || !m_library->isLoaded())
		return;
	pEngine->writePage(NULL, m_bufferOut, 0, rc);
}

void EncoderMp3::initStream()
{
    m_bufferOutSize = (int)((1.25 * 20000 + 7200) + 1);
    m_bufferOut = (unsigned char *)malloc(m_bufferOutSize);
    
    m_bufferIn[0] = (float *)malloc(m_bufferOutSize * sizeof(float));
    m_bufferIn[1] = (float *)malloc(m_bufferOutSize * sizeof(float));
    return;
}

int EncoderMp3::initEncoder(int bitrate)
{
	if(m_library == NULL || !m_library->isLoaded())
		return -1;
    unsigned long samplerate = m_pConfig->getValueString(ConfigKey("[Soundcard]","Samplerate")).toULong();
    
    m_lameFlags = lame_init();
	
    if ( m_lameFlags == NULL ) {
        qDebug() << "Unable to initialize MP3";
        return -1;
    }
    
    lame_set_num_channels(m_lameFlags, 2);
    lame_set_in_samplerate(m_lameFlags, samplerate);
    lame_set_out_samplerate(m_lameFlags, samplerate);
    lame_set_brate(m_lameFlags, bitrate);
    lame_set_mode(m_lameFlags, STEREO);
    lame_set_quality(m_lameFlags, 2);
    lame_set_bWriteVbrTag(m_lameFlags, 0);
   
    if (( lame_init_params(m_lameFlags)) < 0) {
        qDebug() << "Unable to initialize MP3 parameters";
        return -1;
    }
    
    initStream();
    
    return 0;
}
//Creates a new MP3 file
void EncoderMp3::openFile(){
    QByteArray baPath = m_pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY,"Path")).toAscii();
	mp3file.setFileName(baPath);

	if (!mp3file.open(QIODevice::WriteOnly | QIODevice::Text)){
        ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
		props->setType(DLG_WARNING);
		props->setTitle(tr("Recording"));
		props->setText(tr("Could not create mp3 file for recording"));
		ErrorDialogHandler::instance()->requestErrorDialog(props);
	}


}
void EncoderMp3::closeFile(){
	if(mp3file.handle() != -1)
		mp3file.close();
}
void EncoderMp3::writeFile(){
	if(m_library == NULL || !m_library->isLoaded())
		return;	
	//file must be open
	if(mp3file.handle() != -1){
		mp3file.write((const char*)m_bufferOut, rc);
	}
}

