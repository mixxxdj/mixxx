#include <QtDebug>
#include <QUrl>

#include <signal.h>

// shout.h checks for WIN32 to see if we are on Windows.
#ifdef WIN64
#define WIN32
#endif
#include <shout/shout.h>
#ifdef WIN64
#undef WIN32
#endif

#include "engine/sidechain/enginebroadcast.h"

#include "broadcast/defs_broadcast.h"
#include "control/controlpushbutton.h"
#include "encoder/encoder.h"
#include "encoder/encoderbroadcastsettings.h"
#include "mixer/playerinfo.h"
#include "preferences/usersettings.h"
#include "recording/defs_recording.h"

#include "track/track.h"

static const int kConnectRetries = 30;
static const int kMaxNetworkCache = 491520;  // 10 s mp3 @ 192 kbit/s
// Shoutcast default receive buffer 1048576 and autodumpsourcetime 30 s
// http://wiki.shoutcast.com/wiki/SHOUTcast_DNAS_Server_2
static const int kMaxShoutFailures = 3;

EngineBroadcast::EngineBroadcast(UserSettingsPointer pConfig)
        : m_pTextCodec(nullptr),
          m_pMetaData(),
          m_pShout(nullptr),
          m_pShoutMetaData(nullptr),
          m_iMetaDataLife(0),
          m_iShoutStatus(0),
          m_iShoutFailures(0),
          m_settings(pConfig),
          m_pConfig(pConfig),
          m_encoder(nullptr),
          m_pMasterSamplerate(new ControlProxy("[Master]", "samplerate")),
          m_custom_metadata(false),
          m_firstCall(false),
          m_format_is_mp3(false),
          m_format_is_ov(false),
          m_protocol_is_icecast1(false),
          m_protocol_is_icecast2(false),
          m_protocol_is_shoutcast(false),
          m_ogg_dynamic_update(false),
          m_threadWaiting(false),
          m_pOutputFifo(nullptr),
          m_retryCount(0),
          m_reconnectFirstDelay(0.0),
          m_reconnectPeriod(5.0),
          m_noDelayFirstReconnect(true),
          m_limitReconnects(true),
          m_maximumRetries(10) {
    const bool persist = true;
    m_pBroadcastEnabled = new ControlPushButton(
            ConfigKey(BROADCAST_PREF_KEY,"enabled"), persist);
    m_pBroadcastEnabled->setButtonMode(ControlPushButton::TOGGLE);
    connect(m_pBroadcastEnabled, SIGNAL(valueChanged(double)),
            this, SLOT(slotEnableCO(double)));

    m_pStatusCO = new ControlObject(ConfigKey(BROADCAST_PREF_KEY, "status"));
    m_pStatusCO->setReadOnly();
    m_pStatusCO->forceSet(STATUSCO_UNCONNECTED);

    setState(NETWORKSTREAMWORKER_STATE_INIT);

    // Initialize libshout
    shout_init();

    if (!(m_pShout = shout_new())) {
        errorDialog(tr("Mixxx encountered a problem"),
                tr("Could not allocate shout_t"));
    }

    if (!(m_pShoutMetaData = shout_metadata_new())) {
        errorDialog(tr("Mixxx encountered a problem"),
                tr("Could not allocate shout_metadata_t"));
    }

    setFunctionCode(14);
    if (shout_set_nonblocking(m_pShout, 1) != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting non-blocking mode:"),
                shout_get_error(m_pShout));
    }
}

EngineBroadcast::~EngineBroadcast() {
    m_pBroadcastEnabled->set(0);
    m_readSema.release();

    // Wait maximum ~4 seconds. User will get annoyed but
    // if there is some network problems we let them settle
    wait(4000);

    // Signal user if thread doesn't die
    VERIFY_OR_DEBUG_ASSERT(!isRunning()) {
       qWarning() << "EngineBroadcast:~EngineBroadcast(): Thread didn't die.\
       Ignored but file a bug report if problems rise!";
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

bool EngineBroadcast::isConnected() {
    if (m_pShout) {
        m_iShoutStatus = shout_get_connected(m_pShout);
        if (m_iShoutStatus == SHOUTERR_CONNECTED)
            return true;
    }
    return false;
}

QByteArray EngineBroadcast::encodeString(const QString& string) {
    if (m_pTextCodec) {
        return m_pTextCodec->fromUnicode(string);
    }
    return string.toLatin1();
}

void EngineBroadcast::updateFromPreferences() {
    qDebug() << "EngineBroadcast: updating from preferences";
    NetworkStreamWorker::debugState();

    double dStatus = m_pStatusCO->get();
    if (dStatus == STATUSCO_CONNECTED ||
            dStatus == STATUSCO_CONNECTING) {
        qDebug() << "EngineBroadcast::updateFromPreferences status:"
                 << dStatus
                 << ". Can't edit preferences when playing";
        return;
    }

    setState(NETWORKSTREAMWORKER_STATE_BUSY);

    // Delete m_encoder if it has been initialized (with maybe) different bitrate.
    // delete m_encoder calls write() check if it will be exit early
    DEBUG_ASSERT(m_iShoutStatus != SHOUTERR_CONNECTED);
    m_encoder.reset();

    m_format_is_mp3 = false;
    m_format_is_ov = false;
    m_protocol_is_icecast1 = false;
    m_protocol_is_icecast2 = false;
    m_protocol_is_shoutcast = false;
    m_ogg_dynamic_update = false;

    // Convert a bunch of QStrings to QByteArrays so we can get regular C char*
    // strings to pass to libshout.

    QString codec = m_settings.getMetadataCharset();
    QByteArray baCodec = codec.toLatin1();
    m_pTextCodec = QTextCodec::codecForName(baCodec);
    if (!m_pTextCodec) {
        qDebug() << "Couldn't find broadcast metadata codec for codec:" << codec
                 << " defaulting to ISO-8859-1.";
    }

    // Indicates our metadata is in the provided charset.
    shout_metadata_add(m_pShoutMetaData, "charset",  baCodec.constData());

    QString serverType = m_settings.getServertype();

    QString host = m_settings.getHost();
    int start = host.indexOf(QLatin1String("//"));
    if (start == -1) {
        // the host part requires preceding //.
        // Without them, the path is treated relative and goes to the
        // path() section.
        host.prepend(QLatin1String("//"));
    }
    QUrl serverUrl = host;

    int port = m_settings.getPort();
    serverUrl.setPort(port);

    QString mountPoint = m_settings.getMountpoint();
    if (!mountPoint.isEmpty()) {
        if (!mountPoint.startsWith('/')) {
            mountPoint.prepend('/');
        }
        serverUrl.setPath(mountPoint);
    }

    QString login = m_settings.getLogin();
    if (!login.isEmpty()) {
        serverUrl.setUserName(login);
    }

    qDebug() << "Using server URL:" << serverUrl;

    QByteArray baPassword = m_settings.getPassword().toLatin1();
    QByteArray baFormat = m_settings.getFormat().toLatin1();
    int iBitrate = m_settings.getBitrate();

    // Encode metadata like stream name, website, desc, genre, title/author with
    // the chosen TextCodec.
    QByteArray baStreamName = encodeString(m_settings.getStreamName());
    QByteArray baStreamWebsite = encodeString(m_settings.getStreamWebsite());
    QByteArray baStreamDesc = encodeString(m_settings.getStreamDesc());
    QByteArray baStreamGenre = encodeString(m_settings.getStreamGenre());

    // Whether the stream is public.
    bool streamPublic = m_settings.getStreamPublic();

    // Dynamic Ogg metadata update
    m_ogg_dynamic_update = m_settings.getOggDynamicUpdate();

    m_custom_metadata = m_settings.getEnableMetadata();
    m_customTitle = m_settings.getCustomTitle();
    m_customArtist = m_settings.getCustomArtist();

    m_metadataFormat = m_settings.getMetadataFormat();

    bool enableReconnect = m_settings.getEnableReconnect();
    if (enableReconnect) {
        m_reconnectFirstDelay = m_settings.getReconnectFirstDelay();
        m_reconnectPeriod = m_settings.getReconnectPeriod();
        m_noDelayFirstReconnect = m_settings.getNoDelayFirstReconnect();
        m_limitReconnects = m_settings.getLimitReconnects();
        m_maximumRetries = m_settings.getMaximumRetries();
    } else {
        m_limitReconnects = true;
        m_maximumRetries = 0;
    }

    int format;
    int protocol;

    if (shout_set_host(m_pShout, serverUrl.host().toLatin1().constData())
            != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting hostname!"), shout_get_error(m_pShout));
        return;
    }

    if (shout_set_port(m_pShout,
            static_cast<unsigned short>(serverUrl.port(BROADCAST_DEFAULT_PORT)))
            != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting port!"), shout_get_error(m_pShout));
        return;
    }

    if (shout_set_password(m_pShout, baPassword.constData())
            != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting password!"), shout_get_error(m_pShout));
        return;
    }

    if (shout_set_mount(m_pShout, serverUrl.path().toLatin1().constData())
            != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting mount!"), shout_get_error(m_pShout));
        return;
    }

    if (shout_set_user(m_pShout, serverUrl.userName().toLatin1().constData())
            != SHOUTERR_SUCCESS) {
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

    m_format_is_mp3 = !qstrcmp(baFormat.constData(), BROADCAST_FORMAT_MP3);
    m_format_is_ov = !qstrcmp(baFormat.constData(), BROADCAST_FORMAT_OV);
    if (m_format_is_mp3) {
        format = SHOUT_FORMAT_MP3;
    } else if (m_format_is_ov) {
        format = SHOUT_FORMAT_OGG;
    } else {
        qWarning() << "Error: unknown format:" << baFormat.constData();
        return;
    }

    if (shout_set_format(m_pShout, format) != SHOUTERR_SUCCESS) {
        errorDialog("Error setting streaming format!", shout_get_error(m_pShout));
        return;
    }

    if (iBitrate < 0) {
        qWarning() << "Error: unknown bit rate:" << iBitrate;
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

    if (shout_set_audio_info(
            m_pShout, SHOUT_AI_BITRATE,
            QByteArray::number(iBitrate).constData()) != SHOUTERR_SUCCESS) {
        errorDialog(tr("Error setting bitrate"), shout_get_error(m_pShout));
        return;
    }

    m_protocol_is_icecast2 = serverType == BROADCAST_SERVER_ICECAST2;
    m_protocol_is_shoutcast = serverType == BROADCAST_SERVER_SHOUTCAST;
    m_protocol_is_icecast1 = serverType == BROADCAST_SERVER_ICECAST1;


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
    EncoderBroadcastSettings broadcastSettings(m_settings);
    if (m_format_is_mp3) {
        m_encoder = EncoderFactory::getFactory().getNewEncoder(
            EncoderFactory::getFactory().getFormatFor(ENCODING_MP3), m_pConfig, this);
        m_encoder->setEncoderSettings(broadcastSettings);
    } else if (m_format_is_ov) {
        m_encoder = EncoderFactory::getFactory().getNewEncoder(
            EncoderFactory::getFactory().getFormatFor(ENCODING_OGG), m_pConfig, this);
        m_encoder->setEncoderSettings(broadcastSettings);
    } else {
        qDebug() << "**** Unknown Encoder Format";
        setState(NETWORKSTREAMWORKER_STATE_ERROR);
        m_lastErrorStr = "Encoder format error";
        return;
    }

    QString errorMsg;
    if(m_encoder->initEncoder(iMasterSamplerate, errorMsg) < 0) {
        // e.g., if lame is not found
        // init m_encoder itself will display a message box
        qDebug() << "**** Encoder init failed";
        qWarning() << errorMsg;
        // delete m_encoder calls write() make sure it will be exit early
        DEBUG_ASSERT(m_iShoutStatus != SHOUTERR_CONNECTED);
        m_encoder.reset();
        setState(NETWORKSTREAMWORKER_STATE_ERROR);
        m_lastErrorStr = "Encoder error";
        return;
    }
    setState(NETWORKSTREAMWORKER_STATE_READY);
}

bool EngineBroadcast::serverConnect() {
    start(QThread::HighPriority);
    setState(NETWORKSTREAMWORKER_STATE_CONNECTING);
    return true;
}

bool EngineBroadcast::processConnect() {
    qDebug() << "EngineBroadcast::processConnect()";

    // Make sure that we call updateFromPreferences always
    updateFromPreferences();

    if (!m_encoder) {
        // updateFromPreferences failed
        m_pStatusCO->forceSet(STATUSCO_FAILURE);
        qDebug() << "EngineBroadcast::processConnect() returning false";
        return false;
    }

    m_pStatusCO->forceSet(STATUSCO_CONNECTING);
    m_iShoutFailures = 0;
    m_lastErrorStr.clear();
    // set to a high number to automatically update the metadata
    // on the first change
    m_iMetaDataLife = 31337;
    // clear metadata, to make sure the first track is not skipped
    // because it was sent via an previous connection (see metaDataHasChanged)
    if(m_pMetaData) {
        m_pMetaData.reset();
    }
    // If static metadata is available, we only need to send metadata one time
    m_firstCall = false;

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
            m_lastErrorStr = shout_get_error(m_pShout);
            qWarning() << "Streaming server made fatal error. Can't continue connecting:"
                       << m_lastErrorStr;
            break;
        }

        m_iShoutFailures++;
        m_lastErrorStr = shout_get_error(m_pShout);
        qDebug() << m_iShoutFailures << "/" << kMaxShoutFailures
                 << "Streaming server failed connect. Failures:"
                 << m_lastErrorStr;
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
                m_pBroadcastEnabled->toBool()) {
            setState(NETWORKSTREAMWORKER_STATE_WAITING);
            qDebug() << "Connection pending. Waiting...";
            NetworkStreamWorker::debugState();
            m_iShoutStatus = shout_get_connected(m_pShout);

            if (m_iShoutStatus != SHOUTERR_BUSY &&
                    m_iShoutStatus != SHOUTERR_SUCCESS &&
                    m_iShoutStatus != SHOUTERR_CONNECTED) {
                qWarning() << "Streaming server made error:" << m_iShoutStatus;
            }

            // If socket is busy then we wait half second
            if (m_iShoutStatus == SHOUTERR_BUSY) {
                m_enabledMutex.lock();
                m_waitEnabled.wait(&m_enabledMutex, 500);
                m_enabledMutex.unlock();
            }

            ++ timeout;
        }
        if (m_iShoutStatus == SHOUTERR_CONNECTED) {
            setState(NETWORKSTREAMWORKER_STATE_READY);
            qDebug() << "***********Connected to streaming server...";

            m_retryCount = 0;

            if (m_pOutputFifo->readAvailable()) {
                m_pOutputFifo->flushReadData(m_pOutputFifo->readAvailable());
            }
            m_threadWaiting = true;
            m_pStatusCO->forceSet(STATUSCO_CONNECTED);
            emit(broadcastConnected());
            qDebug() << "EngineBroadcast::processConnect() returning true";
            return true;
        } else if (m_iShoutStatus == SHOUTERR_SOCKET) {
            m_lastErrorStr = "Socket error";
            qDebug() << "EngineBroadcast::processConnect() socket error."
                     << "Is socket already in use?";
        } else if (m_pBroadcastEnabled->toBool()) {
            m_lastErrorStr = shout_get_error(m_pShout);
            qDebug() << "EngineBroadcast::processConnect() error:"
                     << m_iShoutStatus << m_lastErrorStr;
        }
    }

    // no connection, clean up
    shout_close(m_pShout);
    // delete m_encoder calls write() check if it will be exit early
    DEBUG_ASSERT(m_iShoutStatus != SHOUTERR_CONNECTED);
    m_encoder.reset();
    if (m_pBroadcastEnabled->toBool()) {
        m_pStatusCO->forceSet(STATUSCO_FAILURE);
    } else {
        m_pStatusCO->forceSet(STATUSCO_UNCONNECTED);
    }
    qDebug() << "EngineBroadcast::processConnect() returning false";
    return false;
}

bool EngineBroadcast::processDisconnect() {
    qDebug() << "EngineBroadcast::processDisconnect()";
    bool disconnected = false;
    if (isConnected()) {
        m_threadWaiting = false;
        // We are connected but broadcast is disabled. Disconnect.
        shout_close(m_pShout);
        m_iShoutStatus = SHOUTERR_UNCONNECTED;
        emit(broadcastDisconnected());
        disconnected = true;
    }
    // delete m_encoder calls write() check if it will be exit early
    DEBUG_ASSERT(m_iShoutStatus != SHOUTERR_CONNECTED);
    m_encoder.reset();
    return disconnected;

}

void EngineBroadcast::write(const unsigned char *header, const unsigned char *body,
                            int headerLen, int bodyLen) {
    setFunctionCode(7);
    if (!m_pShout || m_iShoutStatus != SHOUTERR_CONNECTED) {
        // This happens when the decoder calls flush() and the connection is
        // already down
        return;
    }

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
        NetworkStreamWorker::debugState();
        if (queuelen > kMaxNetworkCache) {
            m_lastErrorStr = tr("Network cache overflow");
            tryReconnect();
        }
    }
}
// These are not used for streaming, but the interface requires them
int EngineBroadcast::tell() {
    if (!m_pShout) {
        return -1;
    }
    return -1;
}
// These are not used for streaming, but the interface requires them
void EngineBroadcast::seek(int pos) {
    Q_UNUSED(pos)
    return;
}
// These are not used for streaming, but the interface requires them
int EngineBroadcast::filelen() {
    return 0;
}

bool EngineBroadcast::writeSingle(const unsigned char* data, size_t len) {
    // We are already synced by EngineNetworkstream
    setFunctionCode(8);
    int ret = shout_send_raw(m_pShout, data, len);
    if (ret == SHOUTERR_BUSY) {
        // in case of busy, frames are queued
        // try to flush queue after a short sleep
        qDebug() << "EngineBroadcast::writeSingle() SHOUTERR_BUSY, trying again";
        QThread::msleep(10); // wait 10 ms until "busy" is over. TODO() tweak for an optimum.
        // if this fails, the queue is transmitted after the next regular shout_send_raw()
        (void)shout_send_raw(m_pShout, nullptr, 0);
    } else if (ret < SHOUTERR_SUCCESS) {
        m_lastErrorStr = shout_get_error(m_pShout);
        qDebug() << "EngineBroadcast::writeSingle() error:"
                 << ret << m_lastErrorStr;
        NetworkStreamWorker::debugState();
        if (++m_iShoutFailures > kMaxShoutFailures) {
            tryReconnect();
        }
        return false;
    } else {
        m_iShoutFailures = 0;
    }
    return true;
}

void EngineBroadcast::process(const CSAMPLE* pBuffer, const int iBufferSize) {
    setFunctionCode(4);

    setState(NETWORKSTREAMWORKER_STATE_BUSY);
    // If we are here then the user wants to be connected (broadcast is enabled
    // in the preferences).

    // If we aren't connected, bail.
    if (m_iShoutStatus != SHOUTERR_CONNECTED)
        return;

    // If we are connected, encode the samples.
    if (iBufferSize > 0 && m_encoder) {
        setFunctionCode(6);
        m_encoder->encodeBuffer(pBuffer, iBufferSize);
        // the encoded frames are received by the write() callback.
    }

    // Check if track metadata has changed and if so, update.
    if (metaDataHasChanged()) {
        updateMetaData();
    }
    setState(NETWORKSTREAMWORKER_STATE_READY);
}

bool EngineBroadcast::metaDataHasChanged() {
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

void EngineBroadcast::updateMetaData() {
    setFunctionCode(5);
    if (!m_pShout || !m_pShoutMetaData)
        return;

    /**
     * If track has changed and static metadata is disabled
     * Send new metadata to broadcast!
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
        if (m_pMetaData != nullptr) {

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
                setFunctionCode(9);
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
                setFunctionCode(10);
                shout_metadata_add(m_pShoutMetaData, "song",  baSong.constData());
            }
            setFunctionCode(11);
            shout_set_metadata(m_pShout, m_pShoutMetaData);

        }
    } else {
        // Otherwise we might use static metadata
        // If we use static metadata, we only need to call the following line once
        if (m_custom_metadata && !m_firstCall) {

            // see comment above...
            if (!m_format_is_mp3 && m_protocol_is_icecast2) {
                setFunctionCode(12);
                shout_metadata_add(
                        m_pShoutMetaData,"artist",encodeString(m_customArtist).constData());

                shout_metadata_add(
                        m_pShoutMetaData,"title",encodeString(m_customTitle).constData());
            } else {
                QByteArray baCustomSong = encodeString(m_customArtist.isEmpty() ? m_customTitle : m_customArtist + " - " + m_customTitle);
                shout_metadata_add(m_pShoutMetaData, "song", baCustomSong.constData());
            }

            setFunctionCode(13);
            shout_set_metadata(m_pShout, m_pShoutMetaData);
            m_firstCall = true;
        }
    }
}

void EngineBroadcast::errorDialog(QString text, QString detailedError) {
    qWarning() << "Streaming error: " << detailedError;
    NetworkStreamWorker::debugState();
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

void EngineBroadcast::infoDialog(QString text, QString detailedInfo) {
    ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
    props->setType(DLG_INFO);
    props->setTitle(tr("Live broadcasting"));
    props->setText(text);
    props->setDetails(detailedInfo);
    props->setKey(text + detailedInfo);
    props->setDefaultButton(QMessageBox::Close);
    props->setModal(false);
    ErrorDialogHandler::instance()->requestErrorDialog(props);
    NetworkStreamWorker::debugState();
}

// Is called from the Mixxx engine thread
void EngineBroadcast::outputAvailable() {
    m_readSema.release();
}

// Is called from the Mixxx engine thread
void EngineBroadcast::setOutputFifo(FIFO<CSAMPLE>* pOutputFifo) {
    m_pOutputFifo = pOutputFifo;
}

void EngineBroadcast::run() {
    unsigned static id = 0;
    QThread::currentThread()->setObjectName(QString("EngineBroadcast %1").arg(++id));
    qDebug() << "EngineBroadcast::run: starting thread";
    NetworkStreamWorker::debugState();
#ifndef __WINDOWS__
    ignoreSigpipe();
#endif

    VERIFY_OR_DEBUG_ASSERT(m_pOutputFifo) {
        qDebug() << "EngineBroadcast::run: Broadcast FIFO handle is not available. Aborting";
        return;
    }

    setState(NETWORKSTREAMWORKER_STATE_BUSY);
    if (!processConnect()) {
        m_pBroadcastEnabled->set(0);
        errorDialog(tr("Can't connect to streaming server"),
                    m_lastErrorStr + "\n" +
                    tr("Please check your connection to the Internet and verify that your username and password are correct."));
        return;
    }

    // Signal user also that we are connected
    infoDialog(tr("Mixxx has successfully connected to the streaming server"), "");

    while(true) {
        setFunctionCode(1);
        incRunCount();
        m_readSema.acquire();
        // Check to see if Broadcast is enabled, and pass the samples off to be
        // broadcast if necessary.
        if (!m_pBroadcastEnabled->toBool()) {
            m_threadWaiting = false;
            if (processDisconnect()) {
                m_pStatusCO->forceSet(STATUSCO_UNCONNECTED);
                infoDialog(tr("Mixxx has successfully disconnected from the streaming server"), "");
            }
            setFunctionCode(2);
            return;
        }
        int readAvailable = m_pOutputFifo->readAvailable();
        if (readAvailable) {
            setFunctionCode(3);
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

bool EngineBroadcast::threadWaiting() {
    return m_threadWaiting;
}

#ifndef __WINDOWS__
void EngineBroadcast::ignoreSigpipe() {
    // If the remote connection is closed, shout_send_raw() can cause a
    // SIGPIPE. If it is unhandled then Mixxx will quit immediately.
#ifdef Q_OS_MAC
    // The per-thread approach using pthread_sigmask below does not seem to work
    // on macOS.
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    if (sigaction(SIGPIPE, &sa, NULL) != 0) {
        qDebug() << "EngineBroadcast::ignoreSigpipe() failed";
    }
#else
    // http://www.microhowto.info/howto/ignore_sigpipe_without_affecting_other_threads_in_a_process.html
    sigset_t sigpipe_mask;
    sigemptyset(&sigpipe_mask);
    sigaddset(&sigpipe_mask, SIGPIPE);
    sigset_t saved_mask;
    if (pthread_sigmask(SIG_BLOCK, &sigpipe_mask, &saved_mask) != 0) {
        qDebug() << "EngineBroadcast::ignoreSigpipe() failed";
    }
#endif
}
#endif


void EngineBroadcast::slotEnableCO(double v) {
    if (v > 1.0) {
        // Wrap around manually .
        // Wrapping around in WPushbutton does not work
        // since the status button has 4 states, but this CO is bool
        m_pBroadcastEnabled->set(0.0);
        v = 0.0;
    }
    if (v > 0.0) {
        serverConnect();
    } else {
        // return early from Timeouts
        m_waitEnabled.wakeAll();
    }
}

bool EngineBroadcast::waitForRetry() {
    if (m_limitReconnects &&
            m_retryCount >= m_maximumRetries) {
        return false;
    }
    ++m_retryCount;

    qDebug() << "waitForRetry()" << m_retryCount << "/" << m_maximumRetries;

    double delay;
    if (m_retryCount == 1) {
        delay = m_reconnectFirstDelay;
    } else {
        delay = m_reconnectPeriod;
    }

    if (delay > 0) {
        m_enabledMutex.lock();
        m_waitEnabled.wait(&m_enabledMutex, delay * 1000);
        m_enabledMutex.unlock();
        if (!m_pBroadcastEnabled->toBool()) {
            return false;
        }
    }
    return true;
}

void EngineBroadcast::tryReconnect() {
    QString originalErrorStr = m_lastErrorStr;
    m_pStatusCO->forceSet(STATUSCO_FAILURE);

    processDisconnect();
    while (waitForRetry()) {
        if (processConnect()) {
            break;
        }
    }

    if (m_pStatusCO->get() == STATUSCO_FAILURE) {
        m_pBroadcastEnabled->set(0);
        m_readSema.release();
        QString errorText;
        if (m_retryCount > 0) {
            errorText = tr("Lost connection to streaming server and %1 attempts to reconnect have failed.")
                    .arg(m_retryCount);
        } else {
            errorText = tr("Lost connection to streaming server.");
        }
        errorDialog(errorText,
                    originalErrorStr + "\n" +
                    m_lastErrorStr + "\n" +
                    tr("Please check your connection to the Internet."));
    }
}
