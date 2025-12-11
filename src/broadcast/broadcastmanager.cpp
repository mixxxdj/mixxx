#include "broadcast/broadcastmanager.h"

#include <shoutidjc/shout.h>

#include "broadcast/defs_broadcast.h"
#include "control/controlpushbutton.h"
#include "engine/sidechain/enginenetworkstream.h"
#include "control/controlproxy.h"
#include "moc_broadcastmanager.cpp"
#include "preferences/settingsmanager.h"
#include "soundio/soundmanager.h"
#include "util/logger.h"


namespace {
const mixxx::Logger kLogger("BroadcastManager");
} // namespace

BroadcastManager::BroadcastManager(SettingsManager* pSettingsManager,
                                   SoundManager* pSoundManager)
        : m_pConfig(pSettingsManager->settings()),
          m_pBroadcastSettings(pSettingsManager->broadcastSettings()),
          m_pNetworkStream(pSoundManager->getNetworkStream()) {
    const bool persist = true;
    m_pBroadcastEnabled = new ControlPushButton(
            ConfigKey(BROADCAST_PREF_KEY,"enabled"), persist);
    m_pBroadcastEnabled->setButtonMode(mixxx::control::ButtonMode::Toggle);
    connect(m_pBroadcastEnabled,
            &ControlPushButton::valueChanged,
            this,
            &BroadcastManager::slotControlEnabled);
       
    // NEW: set up recording control proxies
    m_pRecordingStatus = new ControlProxy("[Recording]", "status", this);
    m_pRecordingToggle = new ControlProxy("[Recording]", "toggle_recording", this);
    m_autoStartedRecording = false;

    m_pStatusCO = new ControlObject(ConfigKey(BROADCAST_PREF_KEY, "status"));
    m_pStatusCO->setReadOnly();
    m_pStatusCO->forceSet(STATUSCO_UNCONNECTED);

    // Initialize libshout
    shout_init();

    // Initialize connections list from the current state of BroadcastSettings
    const QList<BroadcastProfilePtr> profiles = m_pBroadcastSettings->profiles();
    for (const BroadcastProfilePtr& profile : profiles) {
        addConnection(profile);
    }

    // Connect add/remove profiles signals.
    // Passing the raw pointer from QSharedPointer to connect() is fine, since
    // connect is trusted that it won't delete the pointer
    connect(m_pBroadcastSettings.data(),
            &BroadcastSettings::profileAdded,
            this,
            &BroadcastManager::slotProfileAdded);
    connect(m_pBroadcastSettings.data(),
            &BroadcastSettings::profileRemoved,
            this,
            &BroadcastManager::slotProfileRemoved);
    connect(m_pBroadcastSettings.data(),
            &BroadcastSettings::profilesChanged,
            this,
            &BroadcastManager::slotProfilesChanged);
}

BroadcastManager::~BroadcastManager() {
    // Disable broadcast so when Mixxx starts again it will not connect.
    m_pBroadcastEnabled->set(0);

    delete m_pStatusCO;
    delete m_pBroadcastEnabled;

    // NEW: delete recording proxies
    delete m_pRecordingStatus;
    delete m_pRecordingToggle;

    shout_shutdown();
}

void BroadcastManager::setEnabled(bool value) {
    m_pBroadcastEnabled->set(value);
    // TODO(Palakis): apparently, calling set on a ControlObject and not through
    // a ControlProxy doesn't trigger the valueChanged signal here.
    // This is a quick fix, but there has to be a better way to do this, rather
    // than calling the associated slot directly.
    slotControlEnabled(value);
}

bool BroadcastManager::isEnabled() {
    return m_pBroadcastEnabled->toBool();
}

void BroadcastManager::slotControlEnabled(double v) {
    if (v > 1.0) {
        // Wrap around manually .
        // Wrapping around in WPushbutton does not work
        // since the status button has 4 states, but this CO is bool
        v = 0.0;
    }

    if (v > 0.0) {
        bool atLeastOneEnabled = false;
        QList<BroadcastProfilePtr> profiles = m_pBroadcastSettings->profiles();
        for (const BroadcastProfilePtr& profile : std::as_const(profiles)) {
            if (profile->getEnabled()) {
                atLeastOneEnabled = true;
                break;
            }
        }

        if (!atLeastOneEnabled) {
            m_pBroadcastEnabled->set(false);
            emit broadcastEnabled(0.0);
            QMessageBox::warning(nullptr, tr("Action failed"),
                                tr("Please enable at least one connection to use Live Broadcasting."));
            return;
        }

        slotProfilesChanged();
        // NEW: auto-start recording when going live, if enabled in preferences
        const bool autoRecordOnLive =
                m_pConfig->getValue(
                        ConfigKey(BROADCAST_PREF_KEY, "auto_record_on_live"))
                        .toBool();
        const bool recordingActive = m_pRecordingStatus->toBool();
        
        if (autoRecordOnLive && !recordingActive) {
            // Simulate pressing the Record button
            m_pRecordingToggle->set(1.0);
            m_autoStartedRecording = true;
        }

    } else {
        // Turning broadcasting OFF
        m_pBroadcastEnabled->set(false);
        m_pStatusCO->forceSet(STATUSCO_UNCONNECTED);

        // NEW: only stop recording if this session was auto-started
        if (m_autoStartedRecording) {
            const bool recordingActive = m_pRecordingStatus->toBool();
            if (recordingActive) {
                m_pRecordingToggle->set(1.0);
            }
            m_autoStartedRecording = false;
        }
        
        const QList<BroadcastProfilePtr> profiles = m_pBroadcastSettings->profiles();
        for (BroadcastProfilePtr profile : profiles) {
            if (profile->connectionStatus() == BroadcastProfile::STATUS_FAILURE) {
                profile->setConnectionStatus(BroadcastProfile::STATUS_UNCONNECTED);
            }
        }
    }

    emit broadcastEnabled(v > 0.0);
}

void BroadcastManager::slotProfileAdded(BroadcastProfilePtr profile) {
    addConnection(profile);
}

void BroadcastManager::slotProfileRemoved(BroadcastProfilePtr profile) {
    removeConnection(profile);
}

void BroadcastManager::slotProfilesChanged() {
    const QVector<NetworkOutputStreamWorkerPtr> workers = m_pNetworkStream->outputWorkers();
    for (const NetworkOutputStreamWorkerPtr& pWorker : workers) {
        ShoutConnectionPtr connection = qSharedPointerCast<ShoutConnection>(pWorker);
        if (connection) {
            BroadcastProfilePtr profile = connection->profile();
            if (profile->connectionStatus() == BroadcastProfile::STATUS_FAILURE
                    && !profile->getEnabled()) {
                profile->setConnectionStatus(BroadcastProfile::STATUS_UNCONNECTED);
            }
            connection->applySettings();
        }
    }
}

bool BroadcastManager::addConnection(BroadcastProfilePtr profile) {
    if (!profile) {
        return false;
    }

    if (findConnectionForProfile(profile).isNull() == false) {
        return false;
    }

    ShoutConnectionPtr connection(new ShoutConnection(profile, m_pConfig));
    m_pNetworkStream->addOutputWorker(connection);

    connect(profile.data(),
            &BroadcastProfile::connectionStatusChanged,
            this,
            &BroadcastManager::slotConnectionStatusChanged);

    kLogger.debug() << "addConnection: created connection for profile"
                    << profile->getProfileName();
    return true;
}

bool BroadcastManager::removeConnection(BroadcastProfilePtr profile) {
    if (!profile) {
        return false;
    }

    ShoutConnectionPtr connection = findConnectionForProfile(profile);
    if (connection) {
        disconnect(profile.data(),
                &BroadcastProfile::connectionStatusChanged,
                this,
                &BroadcastManager::slotConnectionStatusChanged);

        // Disabling the profile tells ShoutOutput's thread to disconnect
        connection->profile()->setEnabled(false);
        m_pNetworkStream->removeOutputWorker(connection);

        kLogger.debug() << "removeConnection: removed connection for profile"
                        << profile->getProfileName();
        return true;
    }

    return false;
}

ShoutConnectionPtr BroadcastManager::findConnectionForProfile(BroadcastProfilePtr profile) {
    const QVector<NetworkOutputStreamWorkerPtr> workers = m_pNetworkStream->outputWorkers();
    for (const NetworkOutputStreamWorkerPtr& pWorker : workers) {
        ShoutConnectionPtr connection = qSharedPointerCast<ShoutConnection>(pWorker);
        if (connection.isNull()) {
            continue;
        }

        if (connection->profile() == profile) {
            return connection;
        }
    }

    return ShoutConnectionPtr();
}

void BroadcastManager::slotConnectionStatusChanged(int newState) {
    Q_UNUSED(newState);
    int enabledCount = 0, connectingCount = 0,
        connectedCount = 0, failedCount = 0;

    // Collect status info
    const QList<BroadcastProfilePtr> profiles = m_pBroadcastSettings->profiles();
    for (BroadcastProfilePtr profile : profiles) {
        if (!profile->getEnabled()) {
            continue;
        }
        enabledCount++;

        int status = profile->connectionStatus();
        if (status == BroadcastProfile::STATUS_FAILURE) {
            failedCount++;
        }
        else if (status == BroadcastProfile::STATUS_CONNECTING) {
            connectingCount++;
        }
        else if (status == BroadcastProfile::STATUS_CONNECTED) {
            connectedCount++;
        }
    }

    // Changed global status indicator depending on global connections status
    if (enabledCount < 1) {
        // Disable Live Broadcasting if all connections are disabled manually.
        // Calling setEnabled will also update the status CO to UNCONNECTED
        setEnabled(false);
    }
    else if (failedCount >= enabledCount) {
        m_pStatusCO->forceSet(STATUSCO_FAILURE);
    }
    else if (failedCount > 0 && failedCount < enabledCount) {
        m_pStatusCO->forceSet(STATUSCO_WARNING);
    }
    else if (connectingCount > 0) {
        m_pStatusCO->forceSet(STATUSCO_CONNECTING);
    }
    else if (connectedCount > 0) {
        m_pStatusCO->forceSet(STATUSCO_CONNECTED);
    }
    else {
        m_pStatusCO->forceSet(STATUSCO_UNCONNECTED);
    }
}
