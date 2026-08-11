#include "qml/qmleffectunitproxy.h"

#include "effects/effectchain.h"
#include "effects/presets/effectchainpreset.h"
#include "moc_qmleffectunitproxy.cpp"

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
        return;
    }
    m_pEffectUnit->loadChainPreset(m_pPresetManager->presetAtIndex(index));
}

void QmlEffectUnitProxy::updatePreset() {
    if (canUpdatePreset()) {
        m_pPresetManager->updatePreset(m_pEffectUnit);
        emit presetChanged();
    }
}

void QmlEffectUnitProxy::renamePreset() {
    if (canRenamePreset() && m_pPresetManager->renamePreset(m_pEffectUnit->presetName())) {
        emit presetChanged();
    }
}

void QmlEffectUnitProxy::savePresetAs() {
    m_pPresetManager->savePresetAndReload(m_pEffectUnit);
    emit presetChanged();
}

} // namespace qml
} // namespace mixxx
