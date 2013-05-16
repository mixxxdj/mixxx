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

#include <QDebug>

#include <signal.h>

#ifdef __WINDOWS__
#include <windows.h>
//sleep on linux assumes seconds where as Sleep on Windows assumes milliseconds
#define sleep(x) Sleep(x*1000)
#else
#include <unistd.h>
#endif

#include "engine/sidechain/engineshoutcast.h"

#include "configobject.h"
#include "playerinfo.h"
#include "encoder/encoder.h"
#include "encoder/encodermp3.h"
#include "encoder/encodervorbis.h"
#include "shoutcast/defs_shoutcast.h"
#include "trackinfoobject.h"

#define TIMEOUT 10

EngineShoutcast::EngineShoutcast(ConfigObject<ConfigValue> *_config)
        : m_pTextCodec(NULL),
          m_pMetaData(),
          m_pShout(NULL),
          m_pShoutMetaData(NULL),
          m_iMetaDataLife(0),
          m_iShoutStatus(0),
          m_pConfig(_config),
          m_encoder(NULL),
          m_pShoutcastNeedUpdateFromPrefs(NULL),
          m_pUpdateShoutcastFromPrefs(NULL),
          m_pMasterSamplerate(new ControlObjectThread(
              ControlObject::getControl(ConfigKey("[Master]", "samplerate")))),
          m_pShoutcastStatus(new ControlObjectThread(
              new ControlObject(ConfigKey(SHOUTCAST_PREF_KEY, "status")))),
          m_bQuit(false),
          m_custom_metadata(false),
          m_firstCall(false),
          m_format_is_mp3(false),
          m_format_is_ov(false),
          m_protocol_is_icecast1(false),
          m_protocol_is_icecast2(false),
          m_protocol_is_shoutcast(false),
          m_ogg_dynamic_update(false) {

#ifndef __WINDOWS__
    // Ignore SIGPIPE signals that we get when the remote streaming server
    // disconnects.
    signal(SIGPIPE, SIG_IGN);
#endif

    m_pShoutcastStatus->slotSet(SHOUTCAST_DISCONNECTED);
    m_pShoutcastNeedUpdateFromPrefs = new ControlObject(
        ConfigKey(SHOUTCAST_PREF_KEY,"update_from_prefs"));
    m_pUpdateShoutcastFromPrefs = new ControlObjectThread(
        m_pShoutcastNeedUpdateFromPrefs);

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

EngineShoutcast::~EngineShoutcast() {
    if (m_encoder) {
        m_encoder->flush();
        delete m_encoder;
    }

    delete m_pUpdateShoutcastFromPrefs;
    delete m_pShoutcastNeedUpdateFromPrefs;
    delete m_pShoutcastStatus;
    delete m_pMasterSamplerate;

    if (m_pShoutMetaData) {
        shout_metadata_free(m_pShoutMetaData);
    }
    if (m_pShout) {
        shout_close(m_pShout);
        shout_free(m_pShout);
    }
    shout_shutdown();
}

bool EngineShoutcast::serverDisconnect() {
    if (m_encoder){
        m_encoder->flush();
        delete m_encoder;
        m_encoder = NULL;
    }

    m_pShoutcastStatus->slotSet(SHOUTCAST_DISCONNECTED);

    if (m_pShout) {
        shout_close(m_pShout);
        return true;
    }
    return false; //if no connection has been established, nothing can be disconnected
}

bool EngineShoutcast::isConnected() {
    if (m_pShout) {
        m_iShoutStatus = shout_get_connected(m_pShout);
        if (m_iShoutStatus == SHOUTERR_CONNECTED)
            return true;
    }
    return false;
}

QByteArray EngineShoutcast::encodeString(const QString& string) {
    if (m_pTextCodec) {
        return m_pTextCodec->fromUnicode(string);
    }
    return string.toLatin1();
}

void EngineShoutcast::updateFromPreferences() {
    qDebug() << "EngineShoutcast: updating from preferences";

    m_pUpdateShoutcastFromPrefs->slotSet(0.0f);

    m_format_is_mp3 = false;
    m_format_is_ov = false;
    m_protocol_is_icecast1 = false;
    m_protocol_is_icecast2 = false;
    m_protocol_is_shoutcast = false;
    m_ogg_dynamic_update = false;

    // Convert a bunch of QStrings to QByteArrays so we can get regular C char*
    // strings to pass to libshout.

    QString codec = m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY, "metadata_charset"));
    QByteArray baCodec = codec.toLatin1();
    m_pTextCodec = QTextCodec::codecForName(baCodec);
    if (!m_pTextCodec) {
        qDebug() << "Couldn't find shoutcast metadata codec for codec:" << codec
                 << " defaulting to ISO-8859-1.";
    }

    // Indicates our metadata is in the provided charset.
    shout_metadata_add(m_pShoutMetaData, "charset",  baCodec.constData());

    // Host, server type, port, mountpoint, login, password should be latin1.
    QByteArray baHost = m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY, "host")).toLatin1();
    QByteArray baServerType = m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY, "servertype")).toLatin1();
    QByteArray baPort = m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY, "port")).toLatin1();
    QByteArray baMountPoint = m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY, "mountpoint")).toLatin1();
    QByteArray baLogin = m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY, "login")).toLatin1();
    QByteArray baPassword = m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY, "password")).toLatin1();
    QByteArray baFormat = m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY, "format")).toLatin1();
    QByteArray baBitrate = m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY, "bitrate")).toLatin1();

    // Encode metadata like stream name, website, desc, genre, title/author with
    // the chosen TextCodec.
    QByteArray baStreamName = encodeString(m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY, "stream_name")));
    QByteArray baStreamWebsite = encodeString(m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY, "stream_website")));
    QByteArray baStreamDesc = encodeString(m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY, "stream_desc")));
    QByteArray baStreamGenre = encodeString(m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY, "stream_genre")));
    QByteArray baStreamPublic = encodeString(m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY, "stream_public")));

    // Dynamic Ogg metadata update
    m_ogg_dynamic_update = (bool)m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"ogg_dynamicupdate")).toInt();

    m_custom_metadata = (bool)m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY, "enable_metadata")).toInt();
    QString title = m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY, "custom_title"));
    QString artist = m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY, "custom_artist"));
    m_baCustomSong = encodeString(artist.isEmpty() ? title : artist + " - " + title);

    int format;
    int protocol;

    if (shout_set_host(m_pShout, baHost.constData()) != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting hostname!"), shout_get_error(m_pShout));
        return;
    }

    // WTF? Why SHOUT_PROTOCOL_HTTP and not.. the chosen protocol?
    if (shout_set_protocol(m_pShout, SHOUT_PROTOCOL_HTTP) != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting protocol!"), shout_get_error(m_pShout));
        return;
    }

    if (shout_set_port(m_pShout, baPort.toUInt()) != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting port!"), shout_get_error(m_pShout));
        return;
    }

    if (shout_set_password(m_pShout, baPassword.constData()) != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting password!"), shout_get_error(m_pShout));
        return;
    }

    if (shout_set_mount(m_pShout, baMountPoint.constData()) != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting mount!"), shout_get_error(m_pShout));
        return;
    }


    if (shout_set_user(m_pShout, baLogin.constData()) != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting username!"), shout_get_error(m_pShout));
        return;
    }

    if (shout_set_name(m_pShout, baStreamName.constData()) != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting stream name!"), shout_get_error(m_pShout));
        return;
    }

    if (shout_set_description(m_pShout, baStreamDesc.constData()) != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting stream description!"), shout_get_error(m_pShout));
        return;
    }

    if (shout_set_genre(m_pShout, baStreamGenre.constData()) != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting stream genre!"), shout_get_error(m_pShout));
        return;
    }

    if (shout_set_url(m_pShout, baStreamWebsite.constData()) != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting stream url!"), shout_get_error(m_pShout));
        return;
    }

    m_format_is_mp3 = !qstrcmp(baFormat.constData(), SHOUTCAST_FORMAT_MP3);
    m_format_is_ov = !qstrcmp(baFormat.constData(), SHOUTCAST_FORMAT_OV);
    if (m_format_is_mp3) {
        format = SHOUT_FORMAT_MP3;
    } else if (m_format_is_ov) {
        format = SHOUT_FORMAT_OGG;
    } else {
        qDebug() << "Error: unknown format:" << baFormat.constData();
        return;
    }

    if (shout_set_format(m_pShout, format) != SHOUTERR_SUCCESS) {
        errorDialog("Error setting soutcast format!", shout_get_error(m_pShout));
        return;
    }

    bool bitrate_is_int = false;
    int iBitrate = baBitrate.toInt(&bitrate_is_int);

    if (!bitrate_is_int) {
        qDebug() << "Error: unknown bitrate:" << baBitrate.constData();
    }

    int iMasterSamplerate = m_pMasterSamplerate->get();
    if (m_format_is_ov && iMasterSamplerate == 96000) {
        errorDialog(tr("Broadcasting at 96kHz with Ogg Vorbis is not currently "
                       "supported. Please try a different sample-rate or switch "
                       "to a different encoding."),
                    tr("See https://bugs.launchpad.net/mixxx/+bug/686212 for more "
                       "information."));
        return;
    }

    if (shout_set_audio_info(m_pShout, SHOUT_AI_BITRATE, baBitrate.constData()) != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting bitrate"), shout_get_error(m_pShout));
        return;
    }

    m_protocol_is_icecast2 = !qstricmp(baServerType.constData(), SHOUTCAST_SERVER_ICECAST2);
    m_protocol_is_shoutcast = !qstricmp(baServerType.constData(), SHOUTCAST_SERVER_SHOUTCAST);
    m_protocol_is_icecast1 = !qstricmp(baServerType.constData(), SHOUTCAST_SERVER_ICECAST1);


    if (m_protocol_is_icecast2) {
        protocol = SHOUT_PROTOCOL_HTTP;
    } else if (m_protocol_is_shoutcast) {
        protocol = SHOUT_PROTOCOL_ICY;
    } else if (m_protocol_is_icecast1) {
        protocol = SHOUT_PROTOCOL_XAUDIOCAST;
    } else {
        errorDialog(tr("Error: unknown server protocol!"), shout_get_error(m_pShout));
        return;
    }

    if (m_protocol_is_shoutcast && !m_format_is_mp3) {
        errorDialog(tr("Error: libshout only supports Shoutcast with MP3 format!"),
                    shout_get_error(m_pShout));
        return;
    }

    if (shout_set_protocol(m_pShout, protocol) != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting protocol!"), shout_get_error(m_pShout));
        return;
    }

    // Initialize m_encoder
    if (m_encoder) {
        // delete m_encoder if it has been initalized (with maybe) different bitrate
        delete m_encoder;
        m_encoder = NULL;
    }

    if (m_format_is_mp3) {
        m_encoder = new EncoderMp3(this);
    } else if (m_format_is_ov) {
        m_encoder = new EncoderVorbis(this);
    } else {
        qDebug() << "**** Unknown Encoder Format";
        return;
    }

    if (m_encoder->initEncoder(iBitrate, iMasterSamplerate) < 0) {
        //e.g., if lame is not found
        //init m_encoder itself will display a message box
        qDebug() << "**** Encoder init failed";
        delete m_encoder;
        m_encoder = NULL;
    }
}

bool EngineShoutcast::serverConnect() {
    // set to busy in case another thread calls one of the other
    // EngineShoutcast calls
    m_iShoutStatus = SHOUTERR_BUSY;
    m_pShoutcastStatus->slotSet(SHOUTCAST_CONNECTING);
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
        m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY,"enabled"),ConfigValue("0"));
        m_pShoutcastStatus->slotSet(SHOUTCAST_DISCONNECTED);
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
        m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY,"enabled"),ConfigValue("0"));
        m_pShoutcastStatus->slotSet(SHOUTCAST_DISCONNECTED);
        return false;
    }
    if (m_bQuit) {
        if (m_pShout)
            shout_close(m_pShout);
        m_pShoutcastStatus->slotSet(SHOUTCAST_DISCONNECTED);
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
        m_pShoutcastStatus->slotSet(SHOUTCAST_CONNECTED);
        return true;
    }
    //otherwise disable shoutcast in preferences
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY,"enabled"),ConfigValue("0"));
    if(m_pShout){
        shout_close(m_pShout);
        //errorDialog(tr("Mixxx could not connect to the server"), tr("Please check your connection to the Internet and verify that your username and password are correct."));
    }
    m_pShoutcastStatus->slotSet(SHOUTCAST_DISCONNECTED);
    return false;
}

void EngineShoutcast::write(unsigned char *header, unsigned char *body,
                            int headerLen, int bodyLen) {
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
        if (shout_queuelen(m_pShout) > 0) {
            qDebug() << "DEBUG: queue length:" << (int)shout_queuelen(m_pShout);
        }
    } else {
        qDebug() << "Error connecting to Shoutcast server:" << shout_get_error(m_pShout);
        // errorDialog(tr("Shoutcast aborted connect after 3 tries"), tr("Please check your connection to the Internet and verify that your username and password are correct."));
    }
}

void EngineShoutcast::process(const CSAMPLE* pBuffer, const int iBufferSize) {
    //Check to see if Shoutcast is enabled, and pass the samples off to be broadcast if necessary.
    bool prefEnabled = (m_pConfig->getValueString(ConfigKey(SHOUTCAST_PREF_KEY,"enabled")).toInt() == 1);

    if (!prefEnabled) {
        if (isConnected()) {
            // We are conneced but shoutcast is disabled. Disconnect.
            serverDisconnect();
            infoDialog(tr("Mixxx has successfully disconnected to the shoutcast server"), "");
        }
        return;
    }

    // If we are here then the user wants to be connected (shoutcast is enabled
    // in the preferences).

    bool connected = isConnected();

    // If we aren't connected or the user has changed their preferences,
    // disconnect, update from prefs, and reconnect.
    if (!connected || m_pUpdateShoutcastFromPrefs->get() > 0.0f) {
        if (connected) {
            serverDisconnect();
        }

        // Initialize/update the encoder and libshout setup.
        updateFromPreferences();

        if (serverConnect()) {
            infoDialog(tr("Mixxx has successfully connected to the shoutcast server"), "");
        } else {
            errorDialog(tr("Mixxx could not connect to streaming server"),
                        tr("Please check your connection to the Internet and verify that your username and password are correct."));
        }
    }

    // If we aren't connected, bail.
    if (m_iShoutStatus != SHOUTERR_CONNECTED)
        return;

    // If we are connected, encode the samples.
    if (iBufferSize > 0 && m_encoder){
        m_encoder->encodeBuffer(pBuffer, iBufferSize);
    }

    // Check if track metadata has changed and if so, update.
    if (metaDataHasChanged()) {
        updateMetaData();
    }
}

bool EngineShoutcast::metaDataHasChanged() {
    TrackPointer pTrack;

    if (m_iMetaDataLife < 16) {
        m_iMetaDataLife++;
        return false;
    }

    m_iMetaDataLife = 0;

    pTrack = PlayerInfo::Instance().getCurrentPlayingTrack();
    if (!pTrack)
        return false;

    if (m_pMetaData) {
        if ((pTrack->getId() == -1) || (m_pMetaData->getId() == -1)) {
            if ((pTrack->getArtist() == m_pMetaData->getArtist()) &&
                (pTrack->getTitle() == m_pMetaData->getArtist())) {
                return false;
            }
        } else if (pTrack->getId() == m_pMetaData->getId()) {
            return false;
        }
    }
    m_pMetaData = pTrack;
    return true;
}

void EngineShoutcast::updateMetaData() {
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


    //If we use either MP3 streaming or OGG streaming with dynamic update of metadata being enabled,
    //we want dynamic metadata changes
    if (!m_custom_metadata && (m_format_is_mp3 || m_ogg_dynamic_update)) {
        if (m_pMetaData != NULL) {
            QString artist = m_pMetaData->getArtist();
            QString title = m_pMetaData->getTitle();
            QByteArray baSong = encodeString(artist.isEmpty() ? title : artist + " - " + title);
            shout_metadata_add(m_pShoutMetaData, "song",  baSong.constData());
            shout_set_metadata(m_pShout, m_pShoutMetaData);
        }
    } else {
        //Otherwise we might use static metadata
        /** If we use static metadata, we only need to call the following line once **/
        if (m_custom_metadata && !m_firstCall) {
            shout_metadata_add(m_pShoutMetaData, "song",  m_baCustomSong.constData());
            shout_set_metadata(m_pShout, m_pShoutMetaData);
            m_firstCall = true;
        }
    }
}

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

void EngineShoutcast::infoDialog(QString text, QString detailedInfo) {
    ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
    props->setType(DLG_INFO);
    props->setTitle(tr("Live broadcasting"));
    props->setText(text);
    props->setDetails(detailedInfo);
    props->setKey(text + detailedInfo);
    props->setDefaultButton(QMessageBox::Close);
    props->setModal(false);
    ErrorDialogHandler::instance()->requestErrorDialog(props);
}
