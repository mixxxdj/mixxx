
#include "shoutcast/shoutcastmanager.h"
#include "shoutcast/defs_shoutcast.h"
#include "engine/sidechain/engineshoutcast.h"
#include "engine/sidechain/enginesidechain.h"
#include "engine/enginemaster.h"

ShoutcastManager::ShoutcastManager(ConfigObject<ConfigValue>* pConfig,
                                   EngineMaster* pEngine)
        : m_pConfig(pConfig) {
    EngineSideChain* pSidechain = pEngine->getSideChain();
    if (pSidechain) {
        pSidechain->addSideChainWorker(new EngineShoutcast(pConfig));
    }
}

ShoutcastManager::~ShoutcastManager() {
    // Disable shoutcast so when Mixxx starts again it will not connect.
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "enabled"), 0);
}

void ShoutcastManager::setEnabled(bool value) {
    m_pConfig->set(ConfigKey(SHOUTCAST_PREF_KEY, "enabled"),
                   ConfigValue(value));
}

bool ShoutcastManager::isEnabled() {
    return m_pConfig->getValueString(
        ConfigKey(SHOUTCAST_PREF_KEY, "enabled")).toInt() == 1;
}
