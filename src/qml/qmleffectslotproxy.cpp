#include "qml/qmleffectslotproxy.h"

#include <QObject>
#include <QQmlEngine>

#include "effects/effectchain.h"
#include "effects/effectparameter.h"
#include "effects/effectslot.h"
#include "effects/presets/effectpresetmanager.h"
#include "moc_qmleffectslotproxy.cpp"

namespace mixxx {
namespace qml {

QmlEffectSlotProxy::QmlEffectSlotProxy(
        std::shared_ptr<EffectsManager> pEffectsManager,
        int unitNumber,
        EffectChainPointer pChainSlot,
        EffectSlotPointer pEffectSlot,
        QObject* parent)
        : QObject(parent),
          m_pEffectsManager(pEffectsManager),
          m_unitNumber(unitNumber),
          m_pChainSlot(pChainSlot),
          m_pEffectSlot(pEffectSlot),
          m_pParametersModel(new QmlEffectSlotParametersModel(m_pEffectSlot, this)) {
    DEBUG_ASSERT(m_pChainSlot);
    DEBUG_ASSERT(m_pEffectSlot);
    connect(m_pEffectSlot.get(),
            &EffectSlot::effectChanged,
            this,
            &QmlEffectSlotProxy::effectIdChanged);
}

int QmlEffectSlotProxy::getChainSlotNumber() const {
    return m_unitNumber;
}

QString QmlEffectSlotProxy::getChainSlotGroup() const {
    return m_pChainSlot->getGroup();
}

int QmlEffectSlotProxy::getNumber() const {
    return m_pEffectSlot->getEffectSlotNumber();
}

QString QmlEffectSlotProxy::getGroup() const {
    return m_pEffectSlot->getGroup();
}

QString QmlEffectSlotProxy::getEffectId() const {
    return m_pEffectSlot->id();
}

bool QmlEffectSlotProxy::isLoaded() const {
    return m_pEffectSlot->isLoaded();
}

QString QmlEffectSlotProxy::getEffectName() const {
    const auto pManifest = m_pEffectSlot->getManifest();
    return pManifest ? pManifest->displayName() : kNoEffectString;
}

QString QmlEffectSlotProxy::getEffectDescription() const {
    const auto pManifest = m_pEffectSlot->getManifest();
    return pManifest ? pManifest->description() : QString();
}

void QmlEffectSlotProxy::setEffectId(const QString& effectId) {
    const EffectManifestPointer pManifest =
            m_pEffectsManager->getBackendManager()->getManifestFromUniqueId(
                    effectId);
    m_pEffectSlot->loadEffectWithDefaults(pManifest);
}

QmlEffectSlotParametersModel* QmlEffectSlotProxy::getParametersModel() const {
    return m_pParametersModel;
}

void QmlEffectSlotProxy::setParameterVisible(const QString& parameterId, bool visible) {
    const auto parameterMaps = {
            m_pEffectSlot->getLoadedParameters(), m_pEffectSlot->getHiddenParameters()};
    for (const auto& parameterMap : parameterMaps) {
        for (const auto& parameters : parameterMap) {
            for (const auto& pParameter : parameters) {
                if (pParameter->manifest()->id() != parameterId) {
                    continue;
                }
                const bool isVisible =
                        m_pEffectSlot->getLoadedParameters()
                                .value(pParameter->manifest()->parameterType())
                                .contains(pParameter);
                if (visible != isVisible) {
                    if (visible) {
                        m_pEffectSlot->showParameter(pParameter);
                    } else {
                        m_pEffectSlot->hideParameter(pParameter);
                    }
                }
                return;
            }
        }
    }
}

void QmlEffectSlotProxy::saveDefaultSnapshot() {
    if (m_pEffectSlot->isLoaded()) {
        m_pEffectsManager->getEffectPresetManager()->saveDefaultForEffect(m_pEffectSlot);
    }
}

} // namespace qml
} // namespace mixxx
