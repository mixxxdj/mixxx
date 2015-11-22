/***************************************************************************
                  engineshoutcast.cpp  -  class to live stream the mix
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

#include <QtDebug>

#include <signal.h>

// shout.h checks for WIN32 to see if we are on Windows.
#ifdef WIN64
#define WIN32
#endif
#include <shout/shout.h>
#ifdef WIN64
#undef WIN32
#endif

#include "engine/sidechain/engineshoutcast.h"
#include "configobject.h"
#include "playerinfo.h"
#include "encoder/encoder.h"
#include "encoder/encodermp3.h"
#include "encoder/encodervorbis.h"
#include "shoutcast/defs_shoutcast.h"
#include "trackinfoobject.h"
#include "util/sleep.h"
#include "controlpushbutton.h"

static const int kConnectRetries = 10;
static const int kMaxNetworkCache = 491520;  // 10 s mp3 @ 192 kbit/s
static const int kMaxShoutFailures = 3;


EngineShoutcast::EngineShoutcast(ConfigObject<ConfigValue>* _config)
        : m_pTextCodec(NULL),
          m_pMetaData(),
          m_pShout(NULL),
          m_pShoutMetaData(NULL),
          m_iMetaDataLife(0),
          m_iShoutStatus(0),
          m_iShoutFailures(0),
          m_pConfig(_config),
          m_encoder(NULL),
          m_pMasterSamplerate(new ControlObjectSlave("[Master]", "samplerate")),
          m_custom_metadata(false),
          m_firstCall(false),
          m_format_is_mp3(false),
          m_format_is_ov(false),
          m_protocol_is_icecast1(false),
          m_protocol_is_icecast2(false),
          m_protocol_is_shoutcast(false),
          m_ogg_dynamic_update(false),
          m_threadWaiting(false),
          m_pOutputFifo(NULL) {
    const bool persist = true;
    m_pShoutcastEnabled = new ControlPushButton(
            ConfigKey(SHOUTCAST_PREF_KEY,"enabled"), persist);
    m_pShoutcastEnabled->setButtonMode(ControlPushButton::TOGGLE);
    connect(m_pShoutcastEnabled, SIGNAL(valueChanged(double)),
            this, SLOT(slotEnableCO(double)));

    m_pStatusCO = new ControlObject(ConfigKey(SHOUTCAST_PREF_KEY, "status"));
    m_pStatusCO->connectValueChangeRequest(
            this, SLOT(slotStatusCO(double)));
    m_pStatusCO->setAndConfirm(STATUSCO_UNCONNECTED);

    setState(NETWORKSTREAMWORKER_STATE_INIT);

    // Initialize libshout
    shout_init();

    if (!(m_pShout = shout_new())) {
        errorDialog(tr("Mixxx encountered a problem"), tr("Could not allocate shout_t"));
    }

    if (!(m_pShoutMetaData = shout_metadata_new())) {
        errorDialog(tr("Mixxx encountered a problem"), tr("Could not allocate shout_metadata_t"));
    }

    if (shout_set_nonblocking(m_pShout, 1) != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting non-blocking mode:"), shout_get_error(m_pShout));
    }
}

EngineShoutcast::~EngineShoutcast() {
    m_pShoutcastEnabled->set(0);
    m_readSema.release();

    // Wait maximum ~4 seconds. User will get annoyed but
    // if there is some network problems we let them settle
    wait(4000);

    // Signal user if thread doesn't die
    DEBUG_ASSERT_AND_HANDLE(isRunning()) {
       qWarning() << "EngineShoutcast:~EngineShoutcast(): Thread didn't die.\
       Ignored but add this to bug report if problems rise!";
    }

    delete m_pStatusCO;
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

    if(getState() == NETWORKSTREAMWORKER_STATE_CONNECTED) {
        qDebug() << "EngineShoutcast::updateFromPreferences: Can't edit preferences when playing";
        return;
    }

    setState(NETWORKSTREAMWORKER_STATE_BUSY);

    m_format_is_mp3 = false;
    m_format_is_ov = false;
    m_protocol_is_icecast1 = false;
    m_protocol_is_icecast2 = false;
    m_protocol_is_shoutcast = false;
    m_ogg_dynamic_update = false;

    // Convert a bunch of QStrings to QByteArrays so we can get regular C char*
    // strings to pass to libshout.

    QString codec = m_pConfig->getValueString(
            ConfigKey(SHOUTCAST_PREF_KEY, "metadata_charset"));
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

    // Whether the stream is public.
    bool streamPublic = m_pConfig->getValueString(
            ConfigKey(SHOUTCAST_PREF_KEY, "stream_public")).toInt() > 0;

    // Dynamic Ogg metadata update
    m_ogg_dynamic_update = (bool)m_pConfig->getValueString(
            ConfigKey(SHOUTCAST_PREF_KEY,"ogg_dynamicupdate")).toInt();

    m_custom_metadata = (bool)m_pConfig->getValueString(
            ConfigKey(SHOUTCAST_PREF_KEY, "enable_metadata")).toInt();
    m_customTitle = m_pConfig->getValueString(
            ConfigKey(SHOUTCAST_PREF_KEY, "custom_title"));
    m_customArtist = m_pConfig->getValueString(
            ConfigKey(SHOUTCAST_PREF_KEY, "custom_artist"));

    m_metadataFormat = m_pConfig->getValueString(
            ConfigKey(SHOUTCAST_PREF_KEY, "metadata_format"));

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

    if (shout_set_public(m_pShout, streamPublic ? 1 : 0) != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting stream public!"), shout_get_error(m_pShout));
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
        errorDialog("Error setting streaming format!", shout_get_error(m_pShout));
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
    setState(NETWORKSTREAMWORKER_STATE_READY);
}

bool EngineShoutcast::serverConnect() {
    start(QThread::HighPriority);
    setState(NETWORKSTREAMWORKER_STATE_CONNECTING);
    return true;
}

bool EngineShoutcast::processConnect() {
    m_pStatusCO->setAndConfirm(STATUSCO_CONNECTING);
    m_iShoutFailures = 0;
    // set to a high number to automatically update the metadata
    // on the first change
    m_iMetaDataLife = 31337;
    // clear metadata, to make sure the first track is not skipped
    // because it was sent via an previous connection (see metaDataHasChanged)
    if(m_pMetaData) {
        m_pMetaData.clear();
    }
    //If static metadata is available, we only need to send metadata one time
    m_firstCall = false;

    // Make sure that we call updateFromPreferences always
    if (m_encoder == NULL) {
        updateFromPreferences();
    }

    while (m_iShoutFailures < kMaxShoutFailures) {
        shout_close(m_pShout);
        m_iShoutStatus = shout_open(m_pShout);
        if (m_iShoutStatus == SHOUTERR_SUCCESS) {
            m_iShoutStatus = SHOUTERR_CONNECTED;
            setState(NETWORKSTREAMWORKER_STATE_CONNECTED);
        }

        if ((m_iShoutStatus == SHOUTERR_BUSY) ||
            (m_iShoutStatus == SHOUTERR_CONNECTED) ||
            (m_iShoutStatus == SHOUTERR_SUCCESS))
        {
            break;
        }

        // SHOUTERR_INSANE self is corrupt or incorrect
        // SHOUTERR_UNSUPPORTED The protocol/format combination is unsupported
        // SHOUTERR_NOLOGIN The server refused login
        // SHOUTERR_MALLOC There wasn't enough memory to complete the operation
        if (m_iShoutStatus == SHOUTERR_INSANE ||
            m_iShoutStatus == SHOUTERR_UNSUPPORTED ||
            m_iShoutStatus == SHOUTERR_NOLOGIN ||
            m_iShoutStatus == SHOUTERR_MALLOC) {
            qDebug() << "Streaming server made fatal error. Can't continue connecting:" << shout_get_error(m_pShout);
            break;
        }

        m_iShoutFailures++;
        qDebug() << m_iShoutFailures << "/" << kMaxShoutFailures << "Streaming server failed connect. Failures:" << shout_get_error(m_pShout);
    }

    // If we don't have any fatal errors let's try to connect
    if ((m_iShoutStatus == SHOUTERR_BUSY ||
         m_iShoutStatus == SHOUTERR_CONNECTED ||
         m_iShoutStatus == SHOUTERR_SUCCESS) &&
         m_iShoutFailures < kMaxShoutFailures) {
        m_iShoutFailures = 0;
        int timeout = 0;
        while (m_iShoutStatus == SHOUTERR_BUSY &&
                timeout < kConnectRetries &&
                m_pShoutcastEnabled->toBool()) {
            setState(NETWORKSTREAMWORKER_STATE_WAITING);
            qDebug() << "Connection pending. Waiting...";
            m_iShoutStatus = shout_get_connected(m_pShout);

            if (m_iShoutStatus != SHOUTERR_BUSY &&
                m_iShoutStatus != SHOUTERR_SUCCESS &&
                m_iShoutStatus != SHOUTERR_CONNECTED) {
                qDebug() << "Streaming server made error:" << shout_get_error(m_pShout);
            }

            // If socket is busy then we wait half second
            if(m_iShoutStatus == SHOUTERR_BUSY)
            {
               QThread::msleep(500);
            }

            ++ timeout;
        }
        if (m_iShoutStatus == SHOUTERR_CONNECTED) {
            setState(NETWORKSTREAMWORKER_STATE_READY);
            qDebug() << "***********Connected to streaming server...";

            // Signal user also that we are connected
            infoDialog(tr("Mixxx has successfully connected to the streaming server"), "");

            if (m_pOutputFifo->readAvailable()) {
                m_pOutputFifo->flushReadData(m_pOutputFifo->readAvailable());
            }
            m_threadWaiting = true;
            m_pStatusCO->setAndConfirm(STATUSCO_CONNECTED);
            emit(shoutcastConnected());
            return true;
        } else if (m_iShoutStatus == SHOUTERR_SOCKET) {
            qDebug() << "EngineShoutcast::processConnect() socket error."
                     << "Is socket already in use?";
        } else if (m_pShoutcastEnabled->toBool()) {
            qDebug() << "EngineShoutcast::processConnect() error:"
                     << m_iShoutStatus << shout_get_error(m_pShout);
        }
    } else {
        // no connection

    }
    shout_close(m_pShout);
    if (m_encoder) {
        m_encoder->flush();
        delete m_encoder;
        m_encoder = NULL;
    }
    if (m_pShoutcastEnabled->toBool()) {
        m_pStatusCO->setAndConfirm(STATUSCO_FAILURE);
        m_pShoutcastEnabled->set(0);
    } else {
        m_pStatusCO->setAndConfirm(STATUSCO_UNCONNECTED);
    }
    return false;
}

void EngineShoutcast::processDisconnect() {
    qDebug() << "EngineShoutcast::processDisconnect()";
    if (isConnected()) {
        m_threadWaiting = false;
        // We are conneced but shoutcast is disabled. Disconnect.
        shout_close(m_pShout);
        infoDialog(tr("Mixxx has successfully disconnected from the streaming server"), "");
        m_iShoutStatus = SHOUTERR_UNCONNECTED;
        emit(shoutcastDisconnected());
    }

    if (m_encoder) {
        m_encoder->flush();
        delete m_encoder;
        m_encoder = NULL;
    }
}

void EngineShoutcast::write(unsigned char *header, unsigned char *body,
                            int headerLen, int bodyLen) {
    if (!m_pShout) {
        return;
    }

    if (m_iShoutStatus == SHOUTERR_CONNECTED) {
        // Send header if there is one
        if (headerLen > 0) {
            if(!writeSingle(header, headerLen)) {
                return;
            }
        }

        if(!writeSingle(body, bodyLen)) {
            return;
        }

        ssize_t queuelen = shout_queuelen(m_pShout);
        if (queuelen > 0) {
            qDebug() << "shout_queuelen" << queuelen;
            if (queuelen > kMaxNetworkCache) {
                m_pStatusCO->setAndConfirm(STATUSCO_FAILURE);
                processDisconnect();
                if (!processConnect()) {
                    errorDialog(tr("Lost connection to streaming server"),
                                tr("Please check your connection to the Internet and verify that your username and password are correct."));
                }
            }
        }
    }
}

bool EngineShoutcast::writeSingle(const unsigned char* data, size_t len) {
    // We are already synced by EngineNetworkstream
    int ret = shout_send_raw(m_pShout, data, len);
    if (ret < SHOUTERR_SUCCESS && ret != SHOUTERR_BUSY) {
        // in case of bussy, frames are queued and queue is checked below
        qDebug() << "EngineShoutcast::write() header error:"
                 << ret << shout_get_error(m_pShout);
        if (m_iShoutFailures > kMaxShoutFailures) {
            m_pStatusCO->setAndConfirm(STATUSCO_FAILURE);
            processDisconnect();
            if (!processConnect()) {
                errorDialog(tr("Lost connection to streaming server"),
                            tr("Please check your connection to the Internet and verify that your username and password are correct."));
            }
        } else{
            m_iShoutFailures++;
        }
        return false;
    } else {
        m_iShoutFailures = 0;
    }
    return true;
}

void EngineShoutcast::process(const CSAMPLE* pBuffer, const int iBufferSize) {

    setState(NETWORKSTREAMWORKER_STATE_BUSY);
    // If we are here then the user wants to be connected (shoutcast is enabled
    // in the preferences).

    // If we aren't connected, bail.
    if (m_iShoutStatus != SHOUTERR_CONNECTED)
        return;

    // If we are connected, encode the samples.
    if (iBufferSize > 0 && m_encoder) {
        m_encoder->encodeBuffer(pBuffer, iBufferSize);
        // the encoded frames are received by the write() callback.
    }

    // Check if track metadata has changed and if so, update.
    if (metaDataHasChanged()) {
        updateMetaData();
    }
    setState(NETWORKSTREAMWORKER_STATE_READY);
}

bool EngineShoutcast::metaDataHasChanged() {
    TrackPointer pTrack;

    // TODO(rryan): This is latency and buffer size dependent. Should be based
    // on time.
    if (m_iMetaDataLife < 16) {
        m_iMetaDataLife++;
        return false;
    }

    m_iMetaDataLife = 0;

    pTrack = PlayerInfo::instance().getCurrentPlayingTrack();
    if (!pTrack)
        return false;

    if (m_pMetaData) {
        if (!pTrack->getId().isValid() || !m_pMetaData->getId().isValid()) {
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


    // If we use either MP3 streaming or OGG streaming with dynamic update of
    // metadata being enabled, we want dynamic metadata changes
    if (!m_custom_metadata && (m_format_is_mp3 || m_ogg_dynamic_update)) {
        if (m_pMetaData != NULL) {

            QString artist = m_pMetaData->getArtist();
            QString title = m_pMetaData->getTitle();

            // shoutcast uses only "song" as field for "artist - title".
            // icecast2 supports separate fields for "artist" and "title",
            // which will get displayed accordingly if the streamingformat and
            // player supports it. ("song" is treated as an alias for "title")
            //
            // Note (EinWesen):
            // Currently that seems to be OGG only, although it is no problem
            // setting both fields for MP3, tested players do not show anything different.
            // Also I do not know about icecast1. To be safe, i stick to the
            // old way for those use cases.
            if (!m_format_is_mp3 && m_protocol_is_icecast2) {
                shout_metadata_add(m_pShoutMetaData, "artist",  encodeString(artist).constData());
                shout_metadata_add(m_pShoutMetaData, "title",  encodeString(title).constData());
            } else {
                // we are going to take the metadata format and replace all
                // the references to $title and $artist by doing a single
                // pass over the string
                int replaceIndex = 0;

                // Make a copy so we don't overwrite the references only
                // once per streaming session.
                QString metadataFinal = m_metadataFormat;
                do {
                    // find the next occurrence
                    replaceIndex = metadataFinal.indexOf(
                                      QRegExp("\\$artist|\\$title"),
                                      replaceIndex);

                    if (replaceIndex != -1) {
                        if (metadataFinal.indexOf(
                                          QRegExp("\\$artist"), replaceIndex)
                                          == replaceIndex) {
                            metadataFinal.replace(replaceIndex, 7, artist);
                            // skip to the end of the replacement
                            replaceIndex += artist.length();
                        } else {
                            metadataFinal.replace(replaceIndex, 6, title);
                            replaceIndex += title.length();
                        }
                    }
                } while (replaceIndex != -1);

                QByteArray baSong = encodeString(metadataFinal);
                shout_metadata_add(m_pShoutMetaData, "song",  baSong.constData());
            }
            shout_set_metadata(m_pShout, m_pShoutMetaData);

        }
    } else {
        // Otherwise we might use static metadata
        // If we use static metadata, we only need to call the following line once
        if (m_custom_metadata && !m_firstCall) {

            // see comment above...
            if (!m_format_is_mp3 && m_protocol_is_icecast2) {
                shout_metadata_add(
                        m_pShoutMetaData,"artist",encodeString(m_customArtist).constData());

                shout_metadata_add(
                        m_pShoutMetaData,"title",encodeString(m_customTitle).constData());
            } else {
                QByteArray baCustomSong = encodeString(m_customArtist.isEmpty() ? m_customTitle : m_customArtist + " - " + m_customTitle);
                shout_metadata_add(m_pShoutMetaData, "song", baCustomSong.constData());
            }

            shout_set_metadata(m_pShout, m_pShoutMetaData);
            m_firstCall = true;
        }
    }
}

void EngineShoutcast::errorDialog(QString text, QString detailedError) {
    qWarning() << "Streaming error: " << detailedError;
    ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
    props->setType(DLG_WARNING);
    props->setTitle(tr("Live broadcasting"));
    props->setText(text);
    props->setDetails(detailedError);
    props->setKey(detailedError);   // To prevent multiple windows for the same error
    props->setDefaultButton(QMessageBox::Close);
    props->setModal(false);
    ErrorDialogHandler::instance()->requestErrorDialog(props);
    setState(NETWORKSTREAMWORKER_STATE_ERROR);
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

// Is called from the Mixxx engine thread
void EngineShoutcast::outputAvailable() {
    m_readSema.release();
}

// Is called from the Mixxx engine thread
void EngineShoutcast::setOutputFifo(FIFO<CSAMPLE>* pOutputFifo) {
    m_pOutputFifo = pOutputFifo;
}

void EngineShoutcast::run() {
    unsigned static id = 0;
    QThread::currentThread()->setObjectName(QString("EngineShoutcast %1").arg(++id));
    qDebug() << "EngineShoutcast::run: starting thread";

#ifndef __WINDOWS__
    ignoreSigpipe();
#endif

    DEBUG_ASSERT_AND_HANDLE(m_pOutputFifo) {
        qDebug() << "EngineShoutcast::run: Shoutcast FIFO handle is not available. Aborting";
        return;
    }

    setState(NETWORKSTREAMWORKER_STATE_BUSY);
    if (!processConnect()) {
        errorDialog(tr("Can't connect to streaming server"),
                    tr("Please check your connection to the Internet and verify that your username and password are correct."));
        return;
    }

    while(true) {
        m_readSema.acquire();
        // Check to see if Shoutcast is enabled, and pass the samples off to be
        // broadcast if necessary.
        if (!m_pShoutcastEnabled->toBool()) {
            m_threadWaiting = false;
            m_pStatusCO->setAndConfirm(STATUSCO_UNCONNECTED);
            processDisconnect();
            return;
        }
        int readAvailable = m_pOutputFifo->readAvailable();
        if (readAvailable) {
            CSAMPLE* dataPtr1;
            ring_buffer_size_t size1;
            CSAMPLE* dataPtr2;
            ring_buffer_size_t size2;
            // We use size1 and size2, so we can ignore the return value
            (void)m_pOutputFifo->aquireReadRegions(readAvailable, &dataPtr1, &size1,
                    &dataPtr2, &size2);
            process(dataPtr1, size1);
            if (size2 > 0) {
                process(dataPtr2, size2);
            }
            m_pOutputFifo->releaseReadRegions(readAvailable);
        }
    }
}

bool EngineShoutcast::threadWaiting() {
    return m_threadWaiting;
}

#ifndef __WINDOWS__
void EngineShoutcast::ignoreSigpipe()
{
    // shout_send_raw() can cause SIGPIPE, which is passed to this theread
    // and which will finally crash Mixxx if it remains unhandled.
    // Each thread has its own signal mask, so it is safe to do this for the
    // shoutcast thread only
    // http://www.microhowto.info/howto/ignore_sigpipe_without_affecting_other_threads_in_a_process.html
    sigset_t sigpipe_mask;
    sigemptyset(&sigpipe_mask);
    sigaddset(&sigpipe_mask, SIGPIPE);
    sigset_t saved_mask;
    if (pthread_sigmask(SIG_BLOCK, &sigpipe_mask, &saved_mask) == -1) {
        qDebug() << "EngineShoutcast::ignoreSigpipe() failed";
    }
}
#endif

void EngineShoutcast::slotStatusCO(double v) {
    // Ignore external sets "status"
    Q_UNUSED(v);
    qWarning() << "WARNING:"
            << SHOUTCAST_PREF_KEY << "\"status\" is a read-only control, ignoring";
}

void EngineShoutcast::slotEnableCO(double v) {
    if (v > 1.0) {
        // Wrap around manually .
        // Wrapping around in WPushbutton does not work
        // since the status button has 4 states, but this CO is bool
        m_pShoutcastEnabled->set(0.0);
    }
    if (v > 0.0) {
        serverConnect();
    }
}
