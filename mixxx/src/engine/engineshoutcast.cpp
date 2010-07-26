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

#ifdef __SHOUTCAST_VORBIS__
#include "encodervorbis.h"
#endif // __SHOUTCAST_VORBIS__
#ifdef __SHOUTCAST_LAME__
#include "encodermp3.h"
#endif // __SHOUTCAST_LAME__

#include "playerinfo.h"
#include "trackinfoobject.h"

#include <QDebug>
#include <QMutexLocker>
#include <stdio.h> // currently used for writing to stdout


/*
 * Initialize EngineShoutcast
 */
EngineShoutcast::EngineShoutcast(ConfigObject<ConfigValue> *_config)
        : m_pMetaData(),
          m_pShout(NULL),
          m_pShoutMetaData(NULL),
          m_pConfig(_config),
          recReady(NULL),
          encoder(NULL),
          m_pUpdateShoutcastFromPrefs(NULL),
          m_pCrossfader(NULL),
          m_pVolume1(NULL),
          m_pVolume2(NULL),
          m_shoutMutex(QMutex::Recursive) {

    m_pShout = 0;
    m_iShoutStatus = 0;
    m_pUpdateShoutcastFromPrefs = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(SHOUTCAST_PREF_KEY, "update_from_prefs")));
    m_bQuit = false;

    m_pCrossfader = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Master]","crossfader")));
    m_pVolume1 = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel1]","volume")));
    m_pVolume2 = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Channel2]","volume")));

    QByteArray baBitrate = m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"bitrate")).toLatin1();
    QByteArray baFormat = m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"format")).toLatin1();
    int len;

    // Initialize libshout
    shout_init();

    if (!(m_pShout = shout_new())) {
        qDebug() << "Could not allocate shout_t";
        return;
    }

    if (!(m_pShoutMetaData = shout_metadata_new())) {
        qDebug() << "Cound not allocate shout_metadata_t";
        return;
    }

    //Initialize the m_pShout structure with the info from Mixxx's shoutcast preferences.
    updateFromPreferences();

    if (shout_set_nonblocking(m_pShout, 1) != SHOUTERR_SUCCESS) {
        qDebug() << "Error setting non-blocking mode:" << shout_get_error(m_pShout);
        return;
    }

    qDebug("********START SERVERCONNECT*******");
    if ( !serverConnect())
        return;
    qDebug("********SERVERCONNECTED********");


    if (( len = baBitrate.indexOf(' ')) != -1) {
        baBitrate.resize(len);
    }

    // Initialize encoder
    if ( ! qstrcmp(baFormat, "MP3")) {
#ifdef __SHOUTCAST_LAME__
        encoder = new EncoderMp3(m_pConfig, this);
#else
        qDebug() << "*** Missing MP3 Encoder Support";
        return;
#endif // __SHOUTCAST_LAME__
    }
    else if ( ! qstrcmp(baFormat, "Ogg Vorbis")) {
#ifdef __SHOUTCAST_VORBIS__
        encoder = new EncoderVorbis(m_pConfig, this);
#else
        qDebug() << "*** Missing OGG Vorbis Encoder Support";
        return;
#endif // __SHOUTCAST_VORBIS__
    }
    else {
        qDebug() << "**** Unknown Encoder Format";
        return;
    }


    if (encoder->initEncoder(baBitrate.toInt()) < 0) {
        qDebug() << "**** Vorbis init failed";
    }
}

/*
 * Cleanup EngineShoutcast
 */
EngineShoutcast::~EngineShoutcast()
{
    QMutexLocker locker(&m_shoutMutex);
    delete encoder;
    delete m_pUpdateShoutcastFromPrefs;
    delete m_pCrossfader;
    delete m_pVolume1;
    delete m_pVolume2;

    if (m_pShoutMetaData)
        shout_metadata_free(m_pShoutMetaData);
    if (m_pShout) {
        shout_close(m_pShout);
        shout_free(m_pShout);
    }
    shout_shutdown();
}

/*
 * Update EngineShoutcast values from the preferences.
 */
void EngineShoutcast::updateFromPreferences()
{
    QMutexLocker locker(&m_shoutMutex);
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

    int format;
    int len;
    int protocol;


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


    if ( !qstrcmp(baFormat.data(), "MP3")) {
        format = SHOUT_FORMAT_MP3;
    }
    else if ( !qstrcmp(baFormat.data(), "Ogg Vorbis")) {
        format = SHOUT_FORMAT_OGG;
    }
    else {
        qDebug() << "Error: unknown format:" << baFormat.data();
        return;
    }

    if (shout_set_format(m_pShout, format) != SHOUTERR_SUCCESS) {
        qDebug() << "Error setting format:" << shout_get_error(m_pShout);
        return;
    }


    if ((len = baBitrate.indexOf(' ')) != -1) {
        baBitrate.resize(len);
    }

    if (shout_set_audio_info(m_pShout, SHOUT_AI_BITRATE, baBitrate.data()) != SHOUTERR_SUCCESS) {
        qDebug() << "Error setting bitrate:" << shout_get_error(m_pShout);
        return;
    }

    if ( ! qstricmp(baServerType.data(), "Icecast 2")) {
        protocol = SHOUT_PROTOCOL_HTTP;
    } else if ( ! qstricmp(baServerType.data(), "Shoutcast")) {
        protocol = SHOUT_PROTOCOL_ICY;
    } else if ( ! qstricmp(baServerType.data(), "Icecast 1")) {
        protocol = SHOUT_PROTOCOL_XAUDIOCAST;
    } else {
        qDebug() << "Error: unknown server protocol:" << baServerType.data();
        return;
    }

    if (( protocol == SHOUT_PROTOCOL_ICY ) && ( format != SHOUT_FORMAT_MP3)) {
        qDebug() << "Error: libshout only supports Shoutcast With MP3 format";
    }

    if ( shout_set_protocol(m_pShout, protocol) != SHOUTERR_SUCCESS) {
        qDebug() << "Error setting protocol: " << shout_get_error(m_pShout);
        return;
    }

}

/*
 * Reset the Server state and Connect to the Server.
 */
bool EngineShoutcast::serverConnect()
{
    QMutexLocker locker(&m_shoutMutex);
    // set to busy in case another thread calls one of the other
    // EngineShoutcast calls
    m_iShoutStatus = SHOUTERR_BUSY;
    // reset the number of failures to zero
    m_iShoutFailures = 0;
    // set to a high number to automatically update the metadata
    // on the first change
    m_pMetaDataLife = 31337;

    const int iMaxTries = 3;
    while (!m_bQuit && m_iShoutFailures < iMaxTries) {
        if (m_pShout)
            shout_close(m_pShout);

        m_iShoutStatus = shout_open(m_pShout);
        if (m_iShoutStatus == SHOUTERR_SUCCESS)
            m_iShoutStatus = SHOUTERR_CONNECTED;

        if ((m_iShoutStatus == SHOUTERR_BUSY) ||
            (m_iShoutStatus == SHOUTERR_CONNECTED) ||
            (m_iShoutStatus == SHOUTERR_SUCCESS))
            break;

        m_iShoutFailures++;
        qDebug() << "Shoutcast failed connect. Failures:" << m_iShoutFailures;
        sleep(2);
    }
    if (m_iShoutFailures == iMaxTries) {
        qDebug() << "Shoutcast aborted connect after" << iMaxTries << "tries.";
        if (m_pShout)
            shout_close(m_pShout);
    }
    if (m_bQuit) {
        if (m_pShout)
            shout_close(m_pShout);
        return false;
    }

    m_iShoutFailures = 0;

    while (m_iShoutStatus == SHOUTERR_BUSY) {
        qDebug() << "Connection pending. Sleeping...";
        sleep(1);
        m_iShoutStatus = shout_get_connected(m_pShout);
    }
    if (m_iShoutStatus == SHOUTERR_CONNECTED) {
        qDebug() << "***********Connected to Shoutcast server...";
        return true;
    }

    return false;
}

/*
 * Called by the Engine implementation to flush the stream to the server.
 */
void EngineShoutcast::writePage(unsigned char *header, unsigned char *body,
                                int headerLen, int bodyLen)
{
    QMutexLocker locker(&m_shoutMutex);
    int ret;

    if (!m_pShout)
        return;

    if (m_iShoutStatus == SHOUTERR_CONNECTED) {
        // Send header if there is one
        if ( headerLen > 0 ) {
            ret = shout_send(m_pShout, header, headerLen);
            if (ret != SHOUTERR_SUCCESS) {
                qDebug() << "DEBUG: Send error: " << shout_get_error(m_pShout);
                if ( m_iShoutFailures > 3 )
                    serverConnect();
                else
                    m_iShoutFailures++;

                return;
            } else {
                //qDebug() << "yea I kinda sent header";
            }
        }

        ret = shout_send(m_pShout, body, bodyLen);
        if (ret != SHOUTERR_SUCCESS) {
            qDebug() << "DEBUG: Send error: " << shout_get_error(m_pShout);
            if ( m_iShoutFailures > 3 )
                    serverConnect();
                else
                    m_iShoutFailures++;

            return;
        } else {
            //qDebug() << "yea I kinda sent footer";
        }
        if (shout_queuelen(m_pShout) > 0)
            printf("DEBUG: queue length: %d\n", (int)shout_queuelen(m_pShout));
    } else {
        qDebug() << "Error connecting to Shoutcast server:" << shout_get_error(m_pShout);
    }
}

/*
 * This is called by the Engine implementation for each sample.
 * Encode and send the stream, as well as check for metadata changes.
 */
void EngineShoutcast::process(const CSAMPLE *, const CSAMPLE *pOut, const int iBufferSize)
{
    QMutexLocker locker(&m_shoutMutex);

    if (m_iShoutStatus != SHOUTERR_CONNECTED)
        return;

    if (iBufferSize > 0 && encoder)
        encoder->encodeBuffer(pOut, iBufferSize);

    if (metaDataHasChanged())
        updateMetaData();
}

/*
 * Algorithm which simply flips the lowest and/or second lowest bits,
 * bits 1 and 2, to represent which track is active and returns the result.
 */
int EngineShoutcast::getActiveTracks()
{
    int tracks = 0;


    if (ControlObject::getControl(ConfigKey("[Channel1]","play"))->get()==1.) tracks |= 1;
    if (ControlObject::getControl(ConfigKey("[Channel2]","play"))->get()==1.) tracks |= 2;

    if (tracks ==  0)
        return 0;

    // Detect the dominant track by checking the crossfader and volume levels
    if ((tracks & 1) && (tracks & 2)) {

        if ((m_pVolume1->get() == 0) && (m_pVolume2->get() == 0))
            return 0;

        if (m_pVolume2->get() == 0) {
            tracks = 1;
        }
        else if ( m_pVolume1->get() == 0) {
            tracks = 2;
        }
        // allow a bit of leeway with the crossfader
        else if ((m_pCrossfader->get() < 0.05) && (m_pCrossfader->get() > -0.05)) {

            if (m_pVolume1->get() > m_pVolume2->get()) {
                tracks = 1;
            }
            else if (m_pVolume1->get() < m_pVolume2->get()) {
                tracks = 2;
            }

        }
        else if ( m_pCrossfader->get() < -0.05 ) {
            tracks = 1;
        }
        else if ( m_pCrossfader->get() > 0.05 ) {
            tracks = 2;
        }

    }

    return tracks;
}


/*
 * Check if the metadata has changed since the previous check.
 * We also check when was the last check performed to avoid using
 * too much CPU and as well to avoid changing the metadata during
 * scratches.
 */
bool EngineShoutcast::metaDataHasChanged()
{
    QMutexLocker locker(&m_shoutMutex);
    int tracks;
    TrackPointer newMetaData;
    bool changed = false;


    if ( m_pMetaDataLife < 32 ) {
        m_pMetaDataLife++;
        return false;
    }

    m_pMetaDataLife = 0;


    tracks = getActiveTracks();


    switch (tracks)
    {
    case 0:
        // no tracks are playing
        // we should set the metadata to nothing
        break;
    case 1:
        // track 1 is active

        newMetaData = PlayerInfo::Instance().getTrackInfo(1);
        if (newMetaData != m_pMetaData)
        {
            m_pMetaData = newMetaData;
            changed = true;
        }
        break;
    case 2:
        // track 2 is active
        newMetaData = PlayerInfo::Instance().getTrackInfo(2);
        if (newMetaData != m_pMetaData)
        {
            m_pMetaData = newMetaData;
            changed = true;
        }
        break;
    case 3:
        // both tracks are active, just stick with it for now
        break;
    }


    return changed;
}

/*
 * Update shoutcast metadata.
 * This does not work for OGG/Vorbis and Icecast, since the actual
 * OGG/Vorbis stream contains the metadata.
 */
void EngineShoutcast::updateMetaData()
{
    QMutexLocker locker(&m_shoutMutex);
    if (!m_pShout || !m_pShoutMetaData)
        return;

    QByteArray baSong = "";
    if (m_pMetaData != NULL) {
        // convert QStrings to char*s
        QByteArray baArtist = m_pMetaData->getArtist().toLatin1();
        QByteArray baTitle = m_pMetaData->getTitle().toLatin1();
        baSong = baArtist + " - " + baTitle;
    }

    shout_metadata_add(m_pShoutMetaData, "song",  baSong.data());
    shout_set_metadata(m_pShout, m_pShoutMetaData);
}
