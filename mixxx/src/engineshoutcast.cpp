/***************************************************************************
                  engineshoutcast.cpp  -  class to shoutcast the mix
                             -------------------
    copyright            : (C) 2007 by Wesley Stessens
                           (C) 2007 by Albert Santoni
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
//#include "controllogpotmeter.h"
#include "configobject.h"
#include "dlgprefshoutcast.h"

#include "encodervorbis.h"
#include "playerinfo.h"
#include "trackinfoobject.h"

#include <QDebug>
#include <stdio.h> // currently used for writing to stdout

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
//    writeFn = &EngineShoutcast::writePage;

    m_pShout = 0;
    m_iShoutStatus = 0;

    // Initialize libshout
    shout_init();

// INIT STUFF
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

//    serverConnect();

    // Initialize ogg vorbis encoder
    encoder = new EncoderVorbis(_config, this);
    if (encoder->initEncoder() < 0) {
        qDebug() << "**** Vorbis init failed";
    }
}

EngineShoutcast::~EngineShoutcast()
{
    if (m_pShout)
        shout_close(m_pShout);
    shout_shutdown();

    delete encoder;
}

void EngineShoutcast::serverConnect()
{
qDebug("in serverConnect();");
    if (m_pShout)
        shout_close(m_pShout);
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
        m_iShoutStatus = shout_get_connected(m_pShout);
    }
    if (m_iShoutStatus == SHOUTERR_CONNECTED) {
        qDebug() << "***********Connected to Shoutcast server...";
    }
}

void EngineShoutcast::writePage(unsigned char *header, unsigned char *body,
                                int headerLen, int bodyLen)
{
fwrite(header,1,headerLen,stdout);
fwrite(body,1,bodyLen,stdout);
    qDebug() << "writePage() will write " << bodyLen << " data";
    int ret;
//    usleep(100000);
/*    qDebug() << "getconnected" << shout_get_connected(m_pShout);
    m_iShoutStatus = shout_get_connected(m_pShout);
    if (m_iShoutStatus != SHOUTERR_CONNECTED) {
        serverConnect();
    }*/
    if (m_iShoutStatus == SHOUTERR_CONNECTED) {
////////        shout_sync(m_pShout);
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
    } else {
        printf("Error connecting: %s\n", shout_get_error(m_pShout));
    }
}

/*void EngineShoutcast::wrapper2writePage(void *pObj, unsigned char *header, unsigned char *body,
                                        int headerLen, int bodyLen)
{
    EngineShoutcast* mySelf = (EngineShoutcast*)pObj;
    pObj->writePage(header, body, headerLen, bodyLen);
}*/

void EngineShoutcast::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize)
{
//    encoder->encodeBuffer((void*) &objA, EngineShoutcast::wrapper2writePage, pOut, iBufferSize);
    if (iBufferSize > 0) encoder->encodeBuffer(pOut, iBufferSize);
}
