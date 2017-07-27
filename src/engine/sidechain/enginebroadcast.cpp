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

namespace {
const int kNetworkLatencyFrames = 8192; // 185 ms @ 44100 Hz
// Related chunk sizes:
// Mp3 frames = 1152 samples
// Ogg frames = 64 to 8192 samples.
// In Mixxx 1.11 we transmit every decoder-frames at once,
// Which results in case of ogg in a dynamic latency from 0.14 ms to to 185 ms
// Now we have switched to a fixed latency of 8192 frames (stereo samples) =
// which is 185 @ 44100 ms and twice the maximum of the max mixxx audio buffer
const int kBufferFrames = kNetworkLatencyFrames * 4; // 743 ms @ 44100 Hz
// normally * 2 is sufficient.
// We allow to buffer two extra chunks for a CPU overload case, when
// the broadcast thread is not scheduled in time.
}

EngineBroadcast::EngineBroadcast(UserSettingsPointer pConfig,
                                 BroadcastSettingsPointer pBroadcastSettings,
                                 QSharedPointer<EngineNetworkStream> pNetworkStream)
        : m_settings(pBroadcastSettings),
          m_pConfig(pConfig),
          m_pNetworkStream(pNetworkStream),
          m_threadWaiting(false),
          m_pOutputFifo(nullptr) {
    const bool persist = true;

    m_pBroadcastEnabled = new ControlPushButton(
            ConfigKey(BROADCAST_PREF_KEY,"enabled"), persist);
    m_pBroadcastEnabled->setButtonMode(ControlPushButton::TOGGLE);
    connect(m_pBroadcastEnabled, SIGNAL(valueChanged(double)),
            this, SLOT(slotEnableCO(double)));

    m_pStatusCO = new ControlObject(ConfigKey(BROADCAST_PREF_KEY, "status"));
    m_pStatusCO->setReadOnly();
    m_pStatusCO->forceSet(STATUSCO_UNCONNECTED);

    // Initialize connections list from the current state of BroadcastSettings
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
    delete m_pStatusCO;

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

    int fifoSize = m_pNetworkStream->getNumOutputChannels() * kBufferFrames;

    ShoutOutputPtr output(new ShoutOutput(profile, m_pConfig, fifoSize));
    m_connections.insert(profileName, output);

    qDebug() << "EngineBroadcast::addConnection: created connection for profile" << profileName;
    return true;
}

bool EngineBroadcast::removeConnection(BroadcastProfilePtr profile) {
    if(!profile)
        return false;

    ShoutOutputPtr output = m_connections.take(profile->getProfileName());
    if(output) {
        // Disabling the profile tells ShoutOutput's thread to disconnect
        output->profile()->setEnabled(false);

        qDebug() << "EngineBroadcast::removeConnection: removed connection for profile" << profile->getProfileName();
        return true;
    }

    return false;
}

void EngineBroadcast::process(const CSAMPLE* pBuffer, const int iBufferSize) {
    setFunctionCode(4);

    QList<ShoutOutputPtr> connections = m_connections.values();
    for(ShoutOutputPtr c : connections) {
        if(!c)
            continue;

        if(!c->threadWaiting())
            continue;

        FIFO<CSAMPLE>* cFifo = c->getOutputFifo();
        if(cFifo) {
            int available = cFifo->writeAvailable();

            int copyCount = math_min(available, iBufferSize);
            if(copyCount > 0) {
                cFifo->write(pBuffer, copyCount);
            }

            int interval = copyCount;
            int outputChannels = m_pNetworkStream->getNumOutputChannels();
            // Same formula as in EngineNetworkStream::writingDone:
            // Check for desired kNetworkLatencyFrames + 1/2 interval to
            // avoid big jitter due to interferences with sync code
            if(cFifo->readAvailable() + interval / 2
                    >= (outputChannels * kNetworkLatencyFrames)) {
                c->outputAvailable();
            }
        }
    }
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
    QThread::currentThread()->setObjectName("EngineBroadcast");
    qDebug() << "EngineBroadcast::run: Starting thread";

    VERIFY_OR_DEBUG_ASSERT(m_pOutputFifo) {
        qDebug() << "EngineBroadcast::run: Broadcast FIFO handle is not available. Aborting";
        return;
    }

    if(m_pOutputFifo->readAvailable()) {
        m_pOutputFifo->flushReadData(m_pOutputFifo->readAvailable());
    }
    m_threadWaiting = true; // no frames received without this
    m_pStatusCO->forceSet(STATUSCO_CONNECTING);

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

        // TODO(Palakis): have this value governed by the connections' statuses

        m_pStatusCO->forceSet(STATUSCO_CONNECTED);

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

    m_threadWaiting = false;
    m_pStatusCO->forceSet(STATUSCO_UNCONNECTED);

    qDebug() << "EngineBroadcast::run: Thread stopped";
}

bool EngineBroadcast::threadWaiting() {
    return m_threadWaiting;
}

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
        slotProfilesChanged();
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
