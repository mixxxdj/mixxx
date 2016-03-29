
#include "shoutcast/shoutcastmanager.h"
#include "shoutcast/defs_shoutcast.h"
#include "engine/sidechain/enginesidechain.h"
#include "engine/sidechain/enginenetworkstream.h"
#include "engine/enginemaster.h"
#include "soundio/soundmanager.h"

ShoutcastManager::ShoutcastManager(UserSettingsPointer pConfig,
                                   SoundManager* pSoundManager)
        : m_pConfig(pConfig) {
    QSharedPointer<EngineNetworkStream> pNetworkStream =
            pSoundManager->getNetworkStream();
    if (!pNetworkStream.isNull()) {
        m_pShoutcast = QSharedPointer<EngineShoutcast>(
                new EngineShoutcast(pConfig));
        pNetworkStream->addWorker(m_pShoutcast);
    }
    m_pShoutcastEnabled = new ControlObjectSlave(
            SHOUTCAST_PREF_KEY, "enabled", this);
    m_pShoutcastEnabled->connectValueChanged(SLOT(slotControlEnabled(double)));
}

ShoutcastManager::~ShoutcastManager() {
    // Disable shoutcast so when Mixxx starts again it will not connect.
    m_pShoutcastEnabled->set(0);
}

void ShoutcastManager::setEnabled(bool value) {
    m_pShoutcastEnabled->set(value);
}

bool ShoutcastManager::isEnabled() {
    return m_pShoutcastEnabled->toBool();
}

void ShoutcastManager::slotControlEnabled(double v) {
    emit(shoutcastEnabled(v > 0.0));
}
