#include "effects/effectparameterslotbase.h"

#include "control/controlobject.h"
#include "effects/effectparameter.h"
#include "moc_effectparameterslotbase.cpp"

EffectParameterSlotBase::EffectParameterSlotBase(const QString& group,
        const unsigned int iParameterSlotNumber,
        const EffectParameterType parameterType)
        : m_iParameterSlotNumber(iParameterSlotNumber),
          m_group(group),
          m_pEffectParameter(nullptr),
          m_pManifestParameter(nullptr),
          m_parameterType(parameterType),
          m_pControlLoaded(nullptr),
          m_pControlType(nullptr),
          m_dChainParameter(0.0) {
}

EffectParameterSlotBase::~EffectParameterSlotBase() {
    m_pEffectParameter = nullptr;
    m_pManifestParameter.clear();
    delete m_pControlLoaded;
    delete m_pControlType;
}

QString EffectParameterSlotBase::name() const {
    if (m_pManifestParameter) {
        return m_pManifestParameter->name();
    }
    return QString();
}

QString EffectParameterSlotBase::shortName() const {
    if (m_pManifestParameter) {
        return m_pManifestParameter->shortName();
    }
    return QString();
}

QString EffectParameterSlotBase::description() const {
    if (m_pManifestParameter) {
        return m_pManifestParameter->description();
    }
    return tr("No effect loaded.");
}

EffectParameterType EffectParameterSlotBase::parameterType() const {
    return m_parameterType;
}

EffectManifestParameterPointer EffectParameterSlotBase::getManifest() {
    if (m_pManifestParameter) {
        return m_pManifestParameter;
    }
    return EffectManifestParameterPointer();
}

void EffectParameterSlotBase::syncSofttakeover() {
}

void EffectParameterSlotBase::onEffectMetaParameterChanged(double parameter, bool force) {
    Q_UNUSED(parameter);
    Q_UNUSED(force);
}

void EffectParameterSlotBase::slotValueChanged(double v) {
    if (m_pEffectParameter) {
        m_pEffectParameter->setValue(v);
        emit valueChanged(v);
    }
}
