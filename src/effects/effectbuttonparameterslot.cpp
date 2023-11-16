#include "effects/effectbuttonparameterslot.h"

#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "effects/effectparameter.h"
#include "moc_effectbuttonparameterslot.cpp"

EffectButtonParameterSlot::EffectButtonParameterSlot(const QString& group,
        const unsigned int iParameterSlotNumber)
        : EffectParameterSlotBase(group, iParameterSlotNumber, EffectParameterType::Button) {
    QString itemPrefix = formatItemPrefix(iParameterSlotNumber);
    m_pControlLoaded = new ControlObject(
            ConfigKey(m_group, itemPrefix + QString("_loaded")));
    m_pControlValue = new ControlPushButton(
            ConfigKey(m_group, itemPrefix));
    m_pControlValue->setButtonMode(ControlPushButton::POWERWINDOW);
    m_pControlType = new ControlObject(
            ConfigKey(m_group, itemPrefix + QString("_type")));

    connect(m_pControlValue,
            &ControlObject::valueChanged,
            this,
            &EffectButtonParameterSlot::slotValueChanged);

    // Read-only controls.
    m_pControlType->setReadOnly();
    m_pControlLoaded->setReadOnly();

    clear();
}

EffectButtonParameterSlot::~EffectButtonParameterSlot() {
    // m_pControlLoaded and m_pControlType are deleted by ~EffectParameterSlotBase
    delete m_pControlValue;
}

void EffectButtonParameterSlot::loadParameter(EffectParameterPointer pEffectParameter) {
    if (m_pEffectParameter) {
        clear();
    }

    VERIFY_OR_DEBUG_ASSERT(pEffectParameter->manifest()->parameterType() ==
            EffectParameterType::Button) {
        return;
    }

    m_pEffectParameter = pEffectParameter;

    if (m_pEffectParameter) {
        m_pManifestParameter = m_pEffectParameter->manifest();
        m_pControlValue->set(m_pEffectParameter->getValue());
        m_pControlValue->setDefaultValue(m_pManifestParameter->getDefault());
        EffectManifestParameter::ValueScaler type = m_pManifestParameter->valueScaler();
        m_pControlType->forceSet(static_cast<double>(type));
        m_pControlLoaded->forceSet(1.0);
    }

    emit updated();
}

void EffectButtonParameterSlot::clear() {
    //qDebug() << debugString() << "clear";
    if (m_pEffectParameter) {
        m_pEffectParameter = nullptr;
        m_pManifestParameter.clear();
    }

    m_pControlLoaded->forceSet(0.0);
    m_pControlValue->set(0.0);
    m_pControlValue->setDefaultValue(0.0);
    m_pControlType->forceSet(0.0);
    emit updated();
}

void EffectButtonParameterSlot::setParameter(double value) {
    m_pControlValue->setParameterFrom(value, this);
}
