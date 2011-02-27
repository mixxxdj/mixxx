
#include "effects/effectsmanager.h"

EffectsManager::EffectsManager(QObject* pParent)
        : QObject(pParent) {
}

EffectsManager::~EffectsManager() {
}

void EffectsManager::addEffectsBackend(EffectsBackend* pBackend) {
    Q_ASSERT(pBackend);
    m_backends.append(pBackend);
}
