
#include "shoutcast/shoutcastmanager.h"
#include "shoutcast/defs_shoutcast.h"
#include "engine/sidechain/enginesidechain.h"
#include "engine/sidechain/enginenetworkstream.h"
#include "engine/enginemaster.h"
#include "soundmanager.h"

ShoutcastManager::ShoutcastManager(ConfigObject<ConfigValue>* pConfig,
                                   SoundManager* pSoundManager)
        : m_pConfig(pConfig) {
    QSharedPointer<EngineNetworkStream> pNetworkStream =
            pSoundManager->getNetworkStream();
    if (!pNetworkStream.isNull()) {
        m_pShoutcast = QSharedPointer<EngineShoutcast>(
                new EngineShoutcast(pConfig));
        pNetworkStream->addWorker(m_pShoutcast);
    }
}

ShoutcastManager::~ShoutcastManager() {
    // Disable shoutcast so when Mixxx starts again it will not connect.
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "enabled"), 0);
}

void ShoutcastManager::setEnabled(bool value) {
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "enabled"),
                   ConfigValue(value));

    /**
     *  Should this be started somewhere else?
     */
    if (value == true) {
        m_pShoutcast->serverConnect();
    } else {
        m_pShoutcast->serverDisconnect();
    }
}

bool ShoutcastManager::isEnabled() {
    return m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY, "enabled")).toInt() == 1;
}
