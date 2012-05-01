/***************************************************************************
                  engineshoutcast.cpp  -  class to shoutcast the mix
                             -------------------
    copyright            : (C) 2007 by Wesley Stessens
                           (C) 2007 by Albert Santoni
                         : (C) 2010 by Tobias Rafreider
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


#include "recording/encodervorbis.h"
#include "recording/encodermp3.h"

#include "playerinfo.h"
#include "trackinfoobject.h"
#ifdef __WINDOWS__
    #include <windows.h>
    //sleep on linux assumes seconds where as Sleep on Windows assumes milliseconds
    #define sleep(x) Sleep(x*1000)
#else
#include <unistd.h>
#endif

#define TIMEOUT 10

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
          m_encoder(NULL),
          m_pUpdateShoutcastFromPrefs(NULL),
          m_pMasterSamplerate(new ControlObjectThread(
              ControlObject::getControl(ConfigKey("[Master]", "samplerate")))),
          m_shoutMutex(QMutex::Recursive) {

    m_pShout = 0;
    m_iShoutStatus = 0;
    m_pShoutcastNeedUpdateFromPrefs = new ControlObject(ConfigKey("[Shoutcast]","update_from_prefs"));
    m_pUpdateShoutcastFromPrefs = new ControlObjectThreadMain(m_pShoutcastNeedUpdateFromPrefs);

    m_bQuit = false;

    m_firstCall = false;
    // Initialize libshout
    shout_init();

    if (!(m_pShout = shout_new())) {
        errorDialog(tr("Mixxx encountered a problem"), tr("Could not allocate shout_t"));
        return;
    }

    if (!(m_pShoutMetaData = shout_metadata_new())) {
        errorDialog(tr("Mixxx encountered a problem"), tr("Could not allocate shout_metadata_t"));
        return;
    }
    if (shout_set_nonblocking(m_pShout, 1) != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting non-blocking mode:"), shout_get_error(m_pShout));
        return;
    }
}

/*
 * Cleanup EngineShoutcast
 */
EngineShoutcast::~EngineShoutcast()
{
    QMutexLocker locker(&m_shoutMutex);

    if (m_encoder){
        m_encoder->flush();
        delete m_encoder;
    }

    delete m_pUpdateShoutcastFromPrefs;
    delete m_pShoutcastNeedUpdateFromPrefs;
    delete m_pMasterSamplerate;

    if (m_pShoutMetaData)
        shout_metadata_free(m_pShoutMetaData);
    if (m_pShout) {
        shout_close(m_pShout);
        shout_free(m_pShout);
    }
    shout_shutdown();
}

bool EngineShoutcast::serverDisconnect()
{
    QMutexLocker locker(&m_shoutMutex);
    if (m_encoder){
        m_encoder->flush();
        delete m_encoder;
        m_encoder = NULL;
    }

    if (m_pShout) {
        shout_close(m_pShout);
        return true;
    }
    return false; //if no connection has been established, nothing can be disconnected
}

bool EngineShoutcast::isConnected()
{
    QMutexLocker locker(&m_shoutMutex);
    if (m_pShout) {
        m_iShoutStatus = shout_get_connected(m_pShout);
        if (m_iShoutStatus == SHOUTERR_CONNECTED)
            return true;
    }
    return false;
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

    m_baFormat    = m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"format")).toLatin1();

    m_custom_metadata = (bool)m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"enable_metadata")).toInt();
    m_baCustom_title = m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"custom_title")).toLatin1();
    m_baCustom_artist = m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"custom_artist")).toLatin1();

    int format;
    int len;
    int protocol;


    if (shout_set_host(m_pShout, baHost.data()) != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting hostname!"), shout_get_error(m_pShout));
        return;
    }

    if (shout_set_protocol(m_pShout, SHOUT_PROTOCOL_HTTP) != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting protocol!"), shout_get_error(m_pShout));
        return;
    }

    if (shout_set_port(m_pShout, baPort.toUInt()) != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting port!"), shout_get_error(m_pShout));
        return;
    }

    if (shout_set_password(m_pShout, baPassword.data()) != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting password!"), shout_get_error(m_pShout));
        return;
    }
    if (shout_set_mount(m_pShout, baMountPoint.data()) != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting mount!"), shout_get_error(m_pShout));
        return;
    }

    if (shout_set_user(m_pShout, baLogin.data()) != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting username!"), shout_get_error(m_pShout));
        return;
    }
    if (shout_set_name(m_pShout, baStreamName.data()) != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting stream name!"), shout_get_error(m_pShout));
        return;
    }
    if (shout_set_description(m_pShout, baStreamDesc.data()) != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting stream description!"), shout_get_error(m_pShout));
        return;
    }
    if (shout_set_genre(m_pShout, baStreamGenre.data()) != SHOUTERR_SUCCESS) {
          errorDialog(tr("Error setting stream genre!"), shout_get_error(m_pShout));
        return;
    }
    if (shout_set_url(m_pShout, baStreamWebsite.data()) != SHOUTERR_SUCCESS) {
       errorDialog(tr("Error setting stream url!"), shout_get_error(m_pShout));
       return;
    }


    if ( !qstrcmp(m_baFormat.data(), "MP3")) {
        format = SHOUT_FORMAT_MP3;
    }
    else if ( !qstrcmp(m_baFormat.data(), "Ogg Vorbis")) {
        format = SHOUT_FORMAT_OGG;
    }
    else {
        qDebug() << "Error: unknown format:" << m_baFormat.data();
        return;
    }

    if (shout_set_format(m_pShout, format) != SHOUTERR_SUCCESS) {
        errorDialog("Error setting soutcast format!", shout_get_error(m_pShout));
        return;
    }

    if ((len = baBitrate.indexOf(' ')) != -1) {
        baBitrate.resize(len);
    }
    int iBitrate = baBitrate.toInt();

    int iMasterSamplerate = m_pMasterSamplerate->get();
    if (format == SHOUT_FORMAT_OGG && iMasterSamplerate == 96000) {
        errorDialog(tr("Broadcasting at 96kHz with Ogg Vorbis is not currently "
                    "supported. Please try a different sample-rate or switch "
                    "to a different encoding."),
                    tr("See https://bugs.launchpad.net/mixxx/+bug/686212 for more "
                    "information."));
        return;
    }

    if (shout_set_audio_info(m_pShout, SHOUT_AI_BITRATE, baBitrate.data()) != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting bitrate"), shout_get_error(m_pShout));
        return;
    }

    if ( ! qstricmp(baServerType.data(), "Icecast 2")) {
        protocol = SHOUT_PROTOCOL_HTTP;
    } else if ( ! qstricmp(baServerType.data(), "Shoutcast")) {
        protocol = SHOUT_PROTOCOL_ICY;
    } else if ( ! qstricmp(baServerType.data(), "Icecast 1")) {
        protocol = SHOUT_PROTOCOL_XAUDIOCAST;
    } else {
        errorDialog(tr("Error: unknown server protocol!"), shout_get_error(m_pShout));
        return;
    }

    if (( protocol == SHOUT_PROTOCOL_ICY ) && ( format != SHOUT_FORMAT_MP3)) {
        errorDialog(tr("Error: libshout only supports Shoutcast with MP3 format!"), shout_get_error(m_pShout));
        return;
    }

    if (shout_set_protocol(m_pShout, protocol) != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting protocol!"), shout_get_error(m_pShout));
        return;
    }

    // Initialize m_encoder
    if(m_encoder) {
        delete m_encoder;        //delete m_encoder if it has been initalized (with maybe) different bitrate
    }
    if ( ! qstrcmp(m_baFormat, "MP3")) {
        m_encoder = new EncoderMp3(this);

    }
    else if ( ! qstrcmp(m_baFormat, "Ogg Vorbis")) {
        m_encoder = new EncoderVorbis(this);
    }
    else {
        qDebug() << "**** Unknown Encoder Format";
        return;
    }
    if (m_encoder->initEncoder(iBitrate) < 0) {
        //e.g., if lame is not found
        //init m_encoder itself will display a message box
        qDebug() << "**** Encoder init failed";
        delete m_encoder;
        m_encoder = NULL;
    }

}

/*
 * Reset the Server state and Connect to the Server.
 *
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
    m_iMetaDataLife = 31337;
    //If static metadata is available, we only need to send metadata one time
    m_firstCall = false;

    /*Check if m_encoder is initalized
     * Encoder is initalized in updateFromPreferences which is called always before serverConnect()
     * If m_encoder is NULL, then we propably want to use MP3 streaming, however, lame could not be found
     * It does not make sense to connect
     */
     if(m_encoder == NULL){
        m_pConfig->set(ConfigKey("[Shoutcast]","enabled"),ConfigValue("0"));
        return false;
    }
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
        sleep(1);
    }
    if (m_iShoutFailures == iMaxTries) {
        if (m_pShout)
            shout_close(m_pShout);
        m_pConfig->set(ConfigKey("[Shoutcast]","enabled"),ConfigValue("0"));
    }
    if (m_bQuit) {
        if (m_pShout)
            shout_close(m_pShout);
        return false;
    }

    m_iShoutFailures = 0;
    int timeout = 0;
    while (m_iShoutStatus == SHOUTERR_BUSY && timeout < TIMEOUT) {
        qDebug() << "Connection pending. Sleeping...";
        sleep(1);
        m_iShoutStatus = shout_get_connected(m_pShout);
        ++ timeout;
    }
    if (m_iShoutStatus == SHOUTERR_CONNECTED) {
        qDebug() << "***********Connected to Shoutcast server...";
        return true;
    }
    //otherwise disable shoutcast in preferences
    m_pConfig->set(ConfigKey("[Shoutcast]","enabled"),ConfigValue("0"));
    if(m_pShout){
        shout_close(m_pShout);
        //errorDialog(tr("Mixxx could not connect to the server"), tr("Please check your connection to the Internet and verify that your username and password are correct."));
    }

    return false;
}

/*
 * Called by the encoder in method 'encodebuffer()' to flush the stream to the server.
 */
void EngineShoutcast::write(unsigned char *header, unsigned char *body,
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
                if ( m_iShoutFailures > 3 ){
                    if(!serverConnect())
                        errorDialog(tr("Lost connection to streaming server"), tr("Please check your connection to the Internet and verify that your username and password are correct."));
                 }
                 else{
                    m_iShoutFailures++;
                 }

                return;
            } else {
                //qDebug() << "yea I kinda sent header";
            }
        }

        ret = shout_send(m_pShout, body, bodyLen);
        if (ret != SHOUTERR_SUCCESS) {
            qDebug() << "DEBUG: Send error: " << shout_get_error(m_pShout);
            if ( m_iShoutFailures > 3 ){
                    if(!serverConnect())
                        errorDialog(tr("Lost connection to streaming server"), tr("Please check your connection to the Internet and verify that your username and password are correct."));
             }
             else{
                m_iShoutFailures++;
             }

            return;
        } else {
            //qDebug() << "yea I kinda sent footer";
        }
        if (shout_queuelen(m_pShout) > 0)
            printf("DEBUG: queue length: %d\n", (int)shout_queuelen(m_pShout));
    } else {
        qDebug() << "Error connecting to Shoutcast server:" << shout_get_error(m_pShout);
       // errorDialog(tr("Shoutcast aborted connect after 3 tries"), tr("Please check your connection to the Internet and verify that your username and password are correct."));
    }
}

/*
 * This is called by the Engine implementation for each sample.
 * Encode and send the stream, as well as check for metadata changes.
 */
void EngineShoutcast::process(const CSAMPLE *, const CSAMPLE *pOut, const int iBufferSize)
{
    QMutexLocker locker(&m_shoutMutex);
     //Check to see if Shoutcast is enabled, and pass the samples off to be broadcast if necessary.
     bool prefEnabled = (m_pConfig->getValueString(ConfigKey("[Shoutcast]","enabled")).toInt() == 1);

    if (prefEnabled) {
        if(!isConnected()){
            //Initialize the m_pShout structure with the info from Mixxx's m_shoutcast preferences.
            updateFromPreferences();

            if(serverConnect()){
                ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
                props->setType(DLG_INFO);
                props->setTitle(tr("Live broadcasting"));
                props->setText(tr("Mixxx has successfully connected to the shoutcast server"));
                ErrorDialogHandler::instance()->requestErrorDialog(props);
            }
            else{
                errorDialog(tr("Mixxx could not connect to streaming server"), tr("Please check your connection to the Internet and verify that your username and password are correct."));

            }
        }
        //send to shoutcast, if connection has been established
        if (m_iShoutStatus != SHOUTERR_CONNECTED)
            return;

        if (iBufferSize > 0 && m_encoder){
            m_encoder->encodeBuffer(pOut, iBufferSize); //encode and send to shoutcast
        }
        //Check if track has changed and submit its new metadata to shoutcast
        if (metaDataHasChanged())
            updateMetaData();

        if (m_pUpdateShoutcastFromPrefs->get() > 0.0f){
            /*
             * You cannot change bitrate, hostname, etc while connected to a stream
             */
            serverDisconnect();
            updateFromPreferences();
            serverConnect();
        }
     }
    //if shoutcast is disabled
    else{
        if(isConnected()){
            serverDisconnect();
            ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
            props->setType(DLG_INFO);
            props->setTitle(tr("Live broadcasting"));
            props->setText(tr("Mixxx has successfully disconnected to the shoutcast server"));

            ErrorDialogHandler::instance()->requestErrorDialog(props);
        }
    }
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
    TrackPointer pTrack;


    if ( m_iMetaDataLife < 16 ) {
        m_iMetaDataLife++;
        return false;
    }

    m_iMetaDataLife = 0;


    pTrack = PlayerInfo::Instance().getCurrentPlayingTrack();
    if ( !pTrack )
        return false;

    if ( m_pMetaData ) {
        if ((pTrack->getId() == -1) || (m_pMetaData->getId() == -1)) {
            if ((pTrack->getArtist() == m_pMetaData->getArtist()) &&
                (pTrack->getTitle() == m_pMetaData->getArtist())) {
                return false;
            }
        }
        else if (pTrack->getId() == m_pMetaData->getId()) {
            return false;
        }
    }

    m_pMetaData = pTrack;
    return true;
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
    /**
     * If track has changed and static metadata is disabled
     * Send new metadata to shoutcast!
     * This works only for MP3 streams properly as stated in comments, see shout.h
     * WARNING: Changing OGG metadata dynamically by using shout_set_metadata
     * will cause stream interruptions to listeners
     *
     * Also note: Do not try to include Vorbis comments in OGG packages and send them to stream.
     * This was done in EncoderVorbis previously and caused interruptions on track change as well
     * which sounds awful to listeners.
     * To conlcude: Only write OGG metadata one time, i.e., if static metadata is used.
      */


    //If we use MP3 streaming and want dynamic metadata changes
    if(!m_custom_metadata && !qstrcmp(m_baFormat, "MP3")){
        if (m_pMetaData != NULL) {
            // convert QStrings to char*s
            QByteArray baArtist = m_pMetaData->getArtist().toLatin1();
            QByteArray baTitle = m_pMetaData->getTitle().toLatin1();

            if (baArtist.isEmpty())
                baSong = baTitle;
            else
                baSong = baArtist + " - " + baTitle;

            /** Update metadata */
            shout_metadata_add(m_pShoutMetaData, "song",  baSong.data());
                shout_set_metadata(m_pShout, m_pShoutMetaData);
        }
    }
    //Otherwise we might use static metadata
    else{
        /** If we use static metadata, we only need to call the following line once **/
        if(m_custom_metadata && !m_firstCall){
            baSong = m_baCustom_artist + " - " + m_baCustom_title;
            /** Update metadata */
            shout_metadata_add(m_pShoutMetaData, "song",  baSong.data());
                shout_set_metadata(m_pShout, m_pShoutMetaData);
            m_firstCall = true;
        }
    }
}
/* -------- ------------------------------------------------------
Purpose: Common error dialog creation code for run-time exceptions
         Notify user when connected or disconnected and so on
Input:   Detailed error string
Output:  -
-------- ------------------------------------------------------ */
void EngineShoutcast::errorDialog(QString text, QString detailedError) {
    qWarning() << "Shoutcast error: " << detailedError;
    ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
    props->setType(DLG_WARNING);
    props->setTitle(tr("Live broadcasting"));
    props->setText(text);
    props->setDetails(detailedError);
    props->setKey(detailedError);   // To prevent multiple windows for the same error
    props->setDefaultButton(QMessageBox::Close);

    props->setModal(false);

    ErrorDialogHandler::instance()->requestErrorDialog(props);
}


