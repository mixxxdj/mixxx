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

#include "broadcast/defs_broadcast.h"
#include "control/controlpushbutton.h"
#include "preferences/usersettings.h"

#include "engine/sidechain/enginebroadcast.h"

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

    QList<BroadcastProfilePtr> profiles = m_settings->profiles();
    for(BroadcastProfilePtr profile : profiles) {
    	addConnection(profile);
    }

    // Connect add/remove/renamed profiles signals.
    // Passing the raw pointer from QSharedPointer to connect() is fine, since
    // connect is trusted that it won't delete the pointer
    connect(m_settings.data(), SIGNAL(profileAdded(BroadcastProfilePtr)),
            this, SLOT(slotProfileAdded(BroadcastProfilePtr)));
    connect(m_settings.data(), SIGNAL(profileRemoved(BroadcastProfilePtr)),
            this, SLOT(slotProfileRemoved(BroadcastProfilePtr)));
    connect(m_settings.data(), SIGNAL(profileRenamed(QString, BroadcastProfilePtr)),
            this, SLOT(slotProfileRenamed(QString, BroadcastProfilePtr)));
    connect(m_settings.data(), SIGNAL(profilesChanged()),
            this, SLOT(slotProfilesChanged()));

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

    ShoutOutputPtr output(new ShoutOutput(profile, m_pConfig));
    m_connections.insert(profileName, output);

    qDebug() << "EngineBroadcast::addConnection: created connection for profile" << profileName;
    return true;
}

bool EngineBroadcast::removeConnection(BroadcastProfilePtr profile) {
    if(!profile)
        return false;

    ShoutOutputPtr output = m_connections.take(profile->getProfileName());
    if(output) {
        output->serverDisconnect();
        qDebug() << "EngineBroadcast::addConnection: removed connection for profile" << profile->getProfileName();
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
    QThread::currentThread()->setObjectName(QString("EngineBroadcast"));
    qDebug() << "EngineBroadcast::run: Starting thread";

#ifndef __WINDOWS__
    ignoreSigpipe();
#endif

    VERIFY_OR_DEBUG_ASSERT(m_pOutputFifo) {
        qDebug() << "EngineBroadcast::run: Broadcast FIFO handle is not available. Aborting";
        return;
    }

    setState(NETWORKSTREAMWORKER_STATE_READING);

    while(true) {
        setFunctionCode(1);
        incRunCount();
        m_readSema.acquire();

        // Stop the thread if broadcasting is turned off
        if (!m_pBroadcastEnabled->toBool()) {
        	qDebug() << "EngineBroadcast::run: Broadcasting disabled. Terminating thread";
            m_threadWaiting = false;
            setFunctionCode(2);
            break;
        }

        int readAvailable = m_pOutputFifo->readAvailable();
        if (readAvailable) {
        	qDebug() << "EngineBroadcast::run: Read available.";
            setFunctionCode(3);

            CSAMPLE* dataPtr1;
            ring_buffer_size_t size1;
            CSAMPLE* dataPtr2;
            ring_buffer_size_t size2;

            // We use size1 and size2, so we can ignore the return value
            (void)m_pOutputFifo->aquireReadRegions(readAvailable, &dataPtr1, &size1,
                    &dataPtr2, &size2);

            // Push frames to the streaming connections.
            process(dataPtr1, size1);
            if (size2 > 0) {
                process(dataPtr2, size2);
            }

            m_pOutputFifo->releaseReadRegions(readAvailable);
        }
    }

    qDebug() << "EngineBroadcast::run: Stopping thread";
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
        start(QThread::HighPriority);
        setState(NETWORKSTREAMWORKER_STATE_CONNECTING);
    }
}

void EngineBroadcast::slotProfileAdded(BroadcastProfilePtr profile) {
    addConnection(profile);
}

void EngineBroadcast::slotProfileRemoved(BroadcastProfilePtr profile) {
    removeConnection(profile);
}

void EngineBroadcast::slotProfileRenamed(QString oldName, BroadcastProfilePtr profile) {
    ShoutOutputPtr oldItem = m_connections.take(oldName);
    if(oldItem) {
        // Profile in ShoutOutput is a reference, which is supposed
        // to have already been updated
        QString newName = profile->getProfileName();
        m_connections.insert(newName, oldItem);
    }
}

void EngineBroadcast::slotProfilesChanged() {
	if(m_pBroadcastEnabled->toBool()) {
		for(ShoutOutputPtr c : m_connections.values()) {
			if(c) c->applySettings();
		}
	}
}
