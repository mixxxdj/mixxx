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
    m_pShout = 0;
    m_iShoutStatus = 0;
    m_pConfig = _config;
    m_pUpdateShoutcastFromPrefs = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(SHOUTCAST_PREF_KEY, "update_from_prefs")));
    
    // Initialize libshout
    shout_init();

// INIT STUFF
    if (!(m_pShout = shout_new())) {
        qDebug() << "Could not allocate shout_t";
        return;
    }
    
    //Initialize the m_pShout structure with the info from Mixxx's shoutcast preferences.
    updateFromPreferences();

    if (shout_set_nonblocking(m_pShout, 1) != SHOUTERR_SUCCESS) {
        qDebug() << "Error setting non-blocking mode:" << shout_get_error(m_pShout);
        return;
    }
    
qDebug("********START SERVERCONNECT*******");
    serverConnect();

    // Initialize ogg vorbis encoder
    encoder = new EncoderVorbis(m_pConfig, this);
    if (encoder->initEncoder() < 0) {
        qDebug() << "**** Vorbis init failed";
    }
}

EngineShoutcast::~EngineShoutcast()
{
    delete encoder;
    delete m_pUpdateShoutcastFromPrefs;
    
    if (m_pShout)
        shout_close(m_pShout);
    shout_shutdown();
}

void EngineShoutcast::updateFromPreferences()
{
    qDebug() << "EngineShoutcast: updating from preferences";
    
    m_pUpdateShoutcastFromPrefs->slotSet(0.0f);
    
    //Convert a bunch of QStrings to QByteArrays so we can get regular C char* strings to pass to libshout.
    QByteArray baHost       = m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"host")).toLatin1();
    QByteArray baServerType = m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"servertype")).toLatin1();
    QByteArray baPort       = m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"port")).toLatin1();
    QByteArray baMountPoint = m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"mountpoint")).toLatin1();
    QByteArray baLogin      = m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"login")).toLatin1();
    QByteArray baPassword   = m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"password")).toLatin1();
    QByteArray baStreamName = m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"stream_name")).toLatin1();
    QByteArray baStreamWebsite = m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"stream_website")).toLatin1();
    QByteArray baStreamDesc = m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"stream_desc")).toLatin1();
    QByteArray baStreamGenre = m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"stream_genre")).toLatin1();
    QByteArray baStreamPublic = m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"stream_public")).toLatin1();
    QByteArray baBitrate    = m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"bitrate")).toLatin1();
    QByteArray baFormat    = m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"format")).toLatin1();
    
    if (shout_set_host(m_pShout, baHost.data()) != SHOUTERR_SUCCESS) {
        qDebug() << "Error setting hostname:" << shout_get_error(m_pShout);
        return;
    }

    if (shout_set_protocol(m_pShout, SHOUT_PROTOCOL_HTTP) != SHOUTERR_SUCCESS) {
        qDebug() << "Error setting protocol:" << shout_get_error(m_pShout);
        return;
    }

    if (shout_set_port(m_pShout, baPort.toUInt()) != SHOUTERR_SUCCESS) {
        qDebug() << "Error setting port:" << shout_get_error(m_pShout);
        return;
    }

    if (shout_set_password(m_pShout, baPassword.data()) != SHOUTERR_SUCCESS) {
        qDebug() << "Error setting password:" << shout_get_error(m_pShout);
        return;
    }
    if (shout_set_mount(m_pShout, baMountPoint.data()) != SHOUTERR_SUCCESS) {
        qDebug() << "Error setting mount:" << shout_get_error(m_pShout);
        return;
    }

    if (shout_set_user(m_pShout, baLogin.data()) != SHOUTERR_SUCCESS) {
        qDebug() << "Error setting user:" << shout_get_error(m_pShout);
        return;
    }
    //FIXME: Set the shoutcast format according to the prefs (baFormat.data())
    if (shout_set_format(m_pShout, SHOUT_FORMAT_OGG) != SHOUTERR_SUCCESS) {
        qDebug() << "Error setting format:" << shout_get_error(m_pShout);
        return;
    }


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
//fwrite(header,1,headerLen,stdout);
//fwrite(body,1,bodyLen,stdout);
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
        qDebug() << "Error connecting to Shoutcast server:" << shout_get_error(m_pShout);
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
