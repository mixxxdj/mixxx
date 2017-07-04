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

EngineBroadcast::EngineBroadcast(UserSettingsPointer pConfig,
                                 BroadcastSettingsPointer pBroadcastSettings)
        : m_settings(pBroadcastSettings),
          m_pConfig(pConfig),
          m_threadWaiting(false),
          m_pOutputFifo(nullptr) {
    const bool persist = true;
    m_pBroadcastEnabled = new ControlPushButton(
            ConfigKey(BROADCAST_PREF_KEY,"enabled"), persist);
    m_pBroadcastEnabled->setButtonMode(ControlPushButton::TOGGLE);
    connect(m_pBroadcastEnabled, SIGNAL(valueChanged(double)),
            this, SLOT(slotEnableCO(double)));

    setState(NETWORKSTREAMWORKER_STATE_INIT);

    // Initialize libshout
    shout_init();
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

    shout_shutdown();
}

bool EngineBroadcast::addConnection(BroadcastProfilePtr profile) {
    if(!profile)
        return false;

    QString profileName = profile->getProfileName();

    if(m_connections.contains(profileName))
        return false;

    ShoutOutputPtr output(new ShoutOutput(profile));
    m_connections.insert(profileName, output);
    return true;
}

bool EngineBroadcast::removeConnection(BroadcastProfilePtr profile) {
    if(!profile)
        return false;

    ShoutOutputPtr output = m_connections.take(profile->getProfileName());
    if(output) {
        output->serverDisconnect();
        return true;
    }

    return false;
}

void EngineBroadcast::process(const CSAMPLE* pBuffer, const int iBufferSize) {
    setFunctionCode(4);

    setState(NETWORKSTREAMWORKER_STATE_BUSY);

    for(ShoutOutputPtr output : m_connections) {
        if(!output)
            continue;

        if(output->isConnected())
            output->process(pBuffer, iBufferSize);

    }

    setState(NETWORKSTREAMWORKER_STATE_READY);
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

    // TODO(Palakis): use signals
    for(ShoutOutputPtr output : m_connections) {
        output->serverConnect();
    }

    while(true) {
        setFunctionCode(1);
        incRunCount();
        m_readSema.acquire();

        // Check to see if Broadcast is enabled, and pass the samples off to be
        // broadcast if necessary.
        if (!m_pBroadcastEnabled->toBool()) {
            m_threadWaiting = false;

            // TODO(Palakis): use signals
            for(ShoutOutputPtr output : m_connections) {
                output->serverDisconnect();
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
        //serverConnect();
        // TODO(Palakis): connect instances here
    } else {
        // return early from Timeouts
        m_waitEnabled.wakeAll();
    }
}

