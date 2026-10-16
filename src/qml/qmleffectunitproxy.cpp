#include "qml/qmleffectunitproxy.h"

#include "effects/effectchain.h"
#include "effects/presets/effectchainpreset.h"
#include "moc_qmleffectunitproxy.cpp"
#include "util/assert.h"

namespace mixxx {
namespace qml {

QmlEffectUnitProxy::QmlEffectUnitProxy(std::shared_ptr<EffectsManager> pEffectsManager,
        int unitNumber,
        EffectChainPointer pEffectUnit,
        QObject* parent)
        : QObject(parent),
          m_pEffectsManager(std::move(pEffectsManager)),
          m_pPresetManager(m_pEffectsManager->getChainPresetManager()),
          m_unitNumber(unitNumber),
          m_pEffectUnit(std::move(pEffectUnit)) {
    VERIFY_OR_DEBUG_ASSERT(m_pEffectUnit) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pPresetManager) {
        return;
    }
    connect(m_pEffectUnit.get(),
            &EffectChain::chainPresetChanged,
            this,
            &QmlEffectUnitProxy::presetChanged);
    connect(m_pPresetManager.get(),
            &EffectChainPresetManager::effectChainPresetListUpdated,
            this,
            &QmlEffectUnitProxy::presetChanged);
    connect(m_pPresetManager.get(),
            &EffectChainPresetManager::effectChainPresetRenamed,
            this,
            &QmlEffectUnitProxy::presetChanged);
}

int QmlEffectUnitProxy::getUnitNumber() const {
    return m_unitNumber;
}

QString QmlEffectUnitProxy::getGroup() const {
    return m_pEffectUnit->getGroup();
}

QString QmlEffectUnitProxy::getPresetName() const {
    return m_pEffectUnit->presetName();
}

int QmlEffectUnitProxy::getPresetIndex() const {
    return m_pPresetManager->presetIndex(m_pEffectUnit->presetName());
}

EffectChainPresetPointer QmlEffectUnitProxy::currentPreset() const {
    return m_pPresetManager->getPreset(m_pEffectUnit->presetName());
}

bool QmlEffectUnitProxy::isPresetReadOnly() const {
    const auto pPreset = currentPreset();
    return !pPreset || pPreset->isReadOnly();
}

bool QmlEffectUnitProxy::canUpdatePreset() const {
    const auto pPreset = currentPreset();
    return pPreset && !pPreset->isReadOnly();
}

bool QmlEffectUnitProxy::canRenamePreset() const {
    return canUpdatePreset() && !m_pEffectUnit->presetName().isEmpty();
}

void QmlEffectUnitProxy::loadPreset(int index) {
    if (index < 0 || index >= m_pPresetManager->numPresets()) {
        qWarning() << "QmlEffectUnitProxy::loadPreset: index out of range" << index;
        return;
    }
    m_pEffectUnit->loadChainPreset(m_pPresetManager->presetAtIndex(index));
}

void QmlEffectUnitProxy::updatePreset() {
    if (!canUpdatePreset()) {
        qWarning() << "QmlEffectUnitProxy::updatePreset: preset can not be updated";
        return;
    }
    m_pPresetManager->updatePreset(m_pEffectUnit);
}

bool QmlEffectUnitProxy::renamePreset() {
    if (!canRenamePreset()) {
        qWarning() << "QmlEffectUnitProxy::renamePreset: preset can not be renamed";
        return false;
    }
    return m_pPresetManager->renamePreset(m_pEffectUnit->presetName());
}

void QmlEffectUnitProxy::savePresetAs() {
    m_pPresetManager->savePresetAndReload(m_pEffectUnit);
}

} // namespace qml
} // namespace mixxx
