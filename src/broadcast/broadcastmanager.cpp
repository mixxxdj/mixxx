// shout.h checks for WIN32 to see if we are on Windows.
#ifdef WIN64
#define WIN32
#endif
#include <shout/shout.h>
#ifdef WIN64
#undef WIN32
#endif

#include "broadcast/defs_broadcast.h"
#include "engine/enginemaster.h"
#include "engine/sidechain/enginenetworkstream.h"
#include "engine/sidechain/enginesidechain.h"
#include "soundio/soundmanager.h"

#include "broadcast/broadcastmanager.h"

BroadcastManager::BroadcastManager(SettingsManager* pSettingsManager,
                                   SoundManager* pSoundManager)
        : m_pConfig(pSettingsManager->settings()),
          m_pBroadcastSettings(pSettingsManager->broadcastSettings()),
          m_pNetworkStream(pSoundManager->getNetworkStream()) {
    const bool persist = true;
    m_pBroadcastEnabled = new ControlPushButton(
            ConfigKey(BROADCAST_PREF_KEY,"enabled"), persist);
    m_pBroadcastEnabled->setButtonMode(ControlPushButton::TOGGLE);
    connect(m_pBroadcastEnabled, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlEnabled(double)));

    m_pStatusCO = new ControlObject(ConfigKey(BROADCAST_PREF_KEY, "status"));
    m_pStatusCO->setReadOnly();
    m_pStatusCO->forceSet(STATUSCO_UNCONNECTED);

    // Initialize libshout
    shout_init();

    // Initialize connections list from the current state of BroadcastSettings
    QList<BroadcastProfilePtr> profiles = m_pBroadcastSettings->profiles();
    for(BroadcastProfilePtr profile : profiles) {
        addConnection(profile);
    }

    // Connect add/remove profiles signals.
    // Passing the raw pointer from QSharedPointer to connect() is fine, since
    // connect is trusted that it won't delete the pointer
    connect(m_pBroadcastSettings.data(), SIGNAL(profileAdded(BroadcastProfilePtr)),
            this, SLOT(slotProfileAdded(BroadcastProfilePtr)));
    connect(m_pBroadcastSettings.data(), SIGNAL(profileRemoved(BroadcastProfilePtr)),
            this, SLOT(slotProfileRemoved(BroadcastProfilePtr)));
    connect(m_pBroadcastSettings.data(), SIGNAL(profilesChanged()),
            this, SLOT(slotProfilesChanged()));
}

BroadcastManager::~BroadcastManager() {
    // Disable broadcast so when Mixxx starts again it will not connect.
    m_pBroadcastEnabled->set(0);

    delete m_pStatusCO;
    delete m_pBroadcastEnabled;

    shout_shutdown();
}

void BroadcastManager::setEnabled(bool value) {
    m_pBroadcastEnabled->set(value);
}

bool BroadcastManager::isEnabled() {
    return m_pBroadcastEnabled->toBool();
}

void BroadcastManager::slotControlEnabled(double v) {
    if (v > 1.0) {
        // Wrap around manually .
        // Wrapping around in WPushbutton does not work
        // since the status button has 4 states, but this CO is bool
        m_pBroadcastEnabled->set(0.0);
        v = 0.0;
    }

    if (v > 0.0) {
        slotProfilesChanged();
        m_pStatusCO->forceSet(STATUSCO_CONNECTED);
    } else {
        m_pStatusCO->forceSet(STATUSCO_UNCONNECTED);
    }

    emit(broadcastEnabled(v > 0.0));
}

void BroadcastManager::slotProfileAdded(BroadcastProfilePtr profile) {
    addConnection(profile);
}

void BroadcastManager::slotProfileRemoved(BroadcastProfilePtr profile) {
    removeConnection(profile);
}

void BroadcastManager::slotProfilesChanged() {
    if(m_pBroadcastEnabled->toBool()) {
        QVector<NetworkStreamWorkerPtr> workers = m_pNetworkStream->workers();
        for(NetworkStreamWorkerPtr pWorker : workers) {
            ShoutConnectionPtr connection = qSharedPointerCast<ShoutConnection>(pWorker);
            if(connection) {
                connection->applySettings();
            }
        }
    }
}

bool BroadcastManager::addConnection(BroadcastProfilePtr profile) {
    if(!profile)
        return false;

    if(findConnectionForProfile(profile).isNull() == false) {
        return false;
    }

    ShoutConnectionPtr connection(new ShoutConnection(profile, m_pConfig));
    m_pNetworkStream->addWorker(connection);

    qDebug() << "BroadcastManager::addConnection: created connection for profile" << profile->getProfileName();
    return true;
}

bool BroadcastManager::removeConnection(BroadcastProfilePtr profile) {
    if(!profile)
        return false;

    ShoutConnectionPtr connection = findConnectionForProfile(profile);
    if(connection) {
        // Disabling the profile tells ShoutOutput's thread to disconnect
        connection->profile()->setEnabled(false);
        m_pNetworkStream->removeWorker(connection);

        qDebug() << "BroadcastManager::removeConnection: removed connection for profile" << profile->getProfileName();
        return true;
    }

    return false;
}

ShoutConnectionPtr BroadcastManager::findConnectionForProfile(BroadcastProfilePtr profile) {
    QVector<NetworkStreamWorkerPtr> workers = m_pNetworkStream->workers();
    for(NetworkStreamWorkerPtr pWorker : workers) {
        ShoutConnectionPtr connection = qSharedPointerCast<ShoutConnection>(pWorker);
        if(connection.isNull())
            continue;

        if(connection->profile() == profile) {
            return connection;
        }
    }

    return ShoutConnectionPtr();
}
