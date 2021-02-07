#include "effects/effectparameterslotbase.h"

#include <QtDebug>

#include "control/controleffectknob.h"
#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "moc_effectparameterslotbase.cpp"

EffectParameterSlotBase::EffectParameterSlotBase(const QString& group,
        const unsigned int iParameterSlotNumber)
        : m_iParameterSlotNumber(iParameterSlotNumber),
          m_group(group),
          m_pEffectParameter(nullptr),
          m_pControlLoaded(nullptr),
          m_pControlType(nullptr),
          m_dChainParameter(0.0) {
}

EffectParameterSlotBase::~EffectParameterSlotBase() {
    m_pEffectParameter = nullptr;
    m_pEffect.clear();
    delete m_pControlLoaded;
    delete m_pControlType;
}

QString EffectParameterSlotBase::name() const {
    if (m_pEffectParameter) {
        return m_pEffectParameter->name();
    }
    return QString();
}

QString EffectParameterSlotBase::shortName() const {
    if (m_pEffectParameter) {
        return m_pEffectParameter->shortName();
    }
    return QString();
}

QString EffectParameterSlotBase::description() const {
    if (m_pEffectParameter) {
        return m_pEffectParameter->description();
    }
    return tr("No effect loaded.");
}

EffectManifestParameterPointer EffectParameterSlotBase::getManifest() {
    if (m_pEffectParameter) {
        return m_pEffectParameter->manifest();
    }
    return EffectManifestParameterPointer();
}
