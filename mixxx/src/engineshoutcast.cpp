/***************************************************************************
                          engineshoutcast.cpp  -  class to shoutcast the mix
                             -------------------
    copyright            : (C) 2007 by John Sully
                           (C) 2007 by Albert Santoni
                           (C) 2007 by Wesley Stessens
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "engineshoutcast.h"
#include "controllogpotmeter.h"
#include "configobject.h"
#include "dlgprefshoutcast.h"

#include "encodervorbis.h"

#include <QDebug>

/***************************************************************************
 *									   *
 * Notice To Future Developpers (of EngineRecord):					   *
 * 	There is code here to write the file in a seperate thread	   *
 * 	however it is unstable and has been abondoned.  Its only use	   *
 * 	was to support low priority recording, however I don't think its   *
 * 	worth the trouble.						   *
 * 									   *
 ***************************************************************************/

/*
This is some really ugly stuff. I left a lot of EngineRecord to be too.
I'll clean it up once I get something that starts to look like it can work.

I also have to implement a shoutcast connect function and check whether
there's a connection or whether it is needed to reconnect.
Right now I'm not doing that.
My test Icecast2 server is configured with a 1000 timeout value instead.
(default is 10) I'll fix that right after I can get vorbis sound to play through it.
*/

EngineShoutcast::EngineShoutcast(ConfigObject<ConfigValue> *_config)
{
/*    curBuf1 = true;
    config = _config;
    recReady = new ControlObject(ConfigKey("[Master]", "Record"));*/
    //fOut = new WriteAudioFile(_config);
    m_pShout = 0;
    m_iShoutStatus = 0;
    
    //Allocate Buffers
/*    fill = new shoutBuffer;
    fill->size = DEFAULT_BUFSIZE;
    fill->data = new CSAMPLE[fill->size];
    fill->valid = 0;

    write = new shoutBuffer;
    write->size = DEFAULT_BUFSIZE;
    write->data = new CSAMPLE[fill->size];
    write->valid = 0;*/
    //QThread::start();

    // Initialize libshout
    shout_init();

// INIT STUFF
	    if(/*recReady->get() == RECORD_READY && pIn[i] > THRESHOLD_REC*/ true)
	    {
	        //If we are waiting for a track to start before recording
	        //and the audio is high enough (a track is playing)
	        //then we can set the record flag to TRUE
	        qDebug("Setting Record flag to: ON");
//	        recReady->queueFromThread(RECORD_ON);
	        
	        if (!(m_pShout = shout_new())) {
		        qDebug() << "Could not allocate shout_t";
		        return;
	        }
	        if (shout_set_host(m_pShout, "127.0.0.1") != SHOUTERR_SUCCESS) {
		        qDebug() << "Error setting hostname:" << shout_get_error(m_pShout);
		        return;
	        }

	        if (shout_set_protocol(m_pShout, SHOUT_PROTOCOL_HTTP) != SHOUTERR_SUCCESS) {
		        qDebug() << "Error setting protocol:" << shout_get_error(m_pShout);
		        return;
	        }

	        if (shout_set_port(m_pShout, 8000) != SHOUTERR_SUCCESS) {
		        qDebug() << "Error setting port:" << shout_get_error(m_pShout);
		        return;
	        }

	        if (shout_set_password(m_pShout, "send0r") != SHOUTERR_SUCCESS) {
		        qDebug() << "Error setting password:" << shout_get_error(m_pShout);
		        return;
	        }
	        if (shout_set_mount(m_pShout, "/example.ogg") != SHOUTERR_SUCCESS) {
		        qDebug() << "Error setting mount:" << shout_get_error(m_pShout);
		        return;
	        }

	        if (shout_set_user(m_pShout, "source") != SHOUTERR_SUCCESS) {
		        qDebug() << "Error setting user:" << shout_get_error(m_pShout);
		        return;
	        }

	        if (shout_set_format(m_pShout, SHOUT_FORMAT_OGG) != SHOUTERR_SUCCESS) {
		        qDebug() << "Error setting format:" << shout_get_error(m_pShout);
		        return;
	        }

	        if (shout_set_nonblocking(m_pShout, 1) != SHOUTERR_SUCCESS) {
	          qDebug() << "Error setting non-blocking mode:" << shout_get_error(m_pShout);
	          return;
	        }	        
//	        m_iShoutStatus = shout_open(shout);
	        m_iShoutStatus = shout_open(m_pShout);
	        if (m_iShoutStatus == SHOUTERR_SUCCESS)
	          m_iShoutStatus = SHOUTERR_CONNECTED;

/*
Kinda dangerous this loop.
We don't want an eternal loop if the connection stays busy.
But this is just for testing.
*/
	        while (m_iShoutStatus == SHOUTERR_BUSY) {
	          qDebug() << "Connection pending. Sleeping...";
	          sleep(1);
//	          m_iShoutStatus = shout_get_connected(shout);
		  m_iShoutStatus = shout_get_connected(m_pShout);
	        }
		if (m_iShoutStatus == SHOUTERR_CONNECTED) {
			qDebug() << "***********Connected to Shoutcast server...";
		}
	    }
    // Initialize ogg vorbis encoder
    encoder = new EncoderVorbis;
    connect(encoder, SIGNAL(pageReady(unsigned char*, unsigned char*, int, int)),
            this, SLOT(writePage(unsigned char*, unsigned char*, int, int)));
    if (encoder->init() < 0) {
        qDebug() << "**** Vorbis init failed";
    }
}

EngineShoutcast::~EngineShoutcast()
{
//    if (shout)
//	    shout_close(shout);
    if (m_pShout)
        shout_close(m_pShout);
    shout_shutdown();

/*    delete fill->data;
    delete fill;
    delete write->data;
    delete write;*/

    delete encoder;
    //QThread::terminate();
    //qDebug("rec thread terminated");
//    delete fOut;
}

void EngineShoutcast::writePage(unsigned char *header, unsigned char *body,
                                int headerLen, int bodyLen)
{
    qDebug() << "writePage() " << bodyLen;
    int ret;
    if (m_iShoutStatus == SHOUTERR_CONNECTED) {
        ret = shout_send(m_pShout, header, headerLen);
        if (ret != SHOUTERR_SUCCESS) {
            qDebug() << "DEBUG: Send error: " << shout_get_error(m_pShout);
            return;
        } else {
            qDebug() << "yea I kinda sent header";
        }
        ret = shout_send(m_pShout, body, bodyLen);
        if (ret != SHOUTERR_SUCCESS) {
            qDebug() << "DEBUG: Send error: " << shout_get_error(m_pShout);
            return;
        } else {
            qDebug() << "yea I kinda sent footer";
        }
        if (shout_queuelen(m_pShout) > 0)
            printf("DEBUG: queue length: %d\n", (int)shout_queuelen(m_pShout));
        shout_sync(m_pShout);
    } else {
        printf("Error connecting: %s\n", shout_get_error(m_pShout));
    }
}

void EngineShoutcast::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize)
{
//    long ret;
//    CSAMPLE *Out = (CSAMPLE*) pOut;
    if (iBufferSize > 0) encoder->encodeBuffer(pOut, iBufferSize);
}


/*void EngineShoutcast::run()
{
    //Method will record the buffer to file
    //fOut->write(buffer1, validBuf1);
    shoutBuffer *temp;
    while(1)
    {
	mutexFill.lock();
	waitCondFill.wait(&mutexFill);
	
	//swap buffers
	temp = fill;
	fill = write;
	write = temp;

	mutexFill.unlock();
	
	//write record buffer to file
//	fOut->write(write->data, write->valid);
	write->valid = 0;
    }
}*/

