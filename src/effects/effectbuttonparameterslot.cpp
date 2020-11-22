#include <QtDebug>

#include "control/controleffectknob.h"
#include "effects/effectbuttonparameterslot.h"
#include "effects/effectxmlelements.h"
#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "util/math.h"
#include "util/xml.h"

EffectButtonParameterSlot::EffectButtonParameterSlot(const QString& group,
                                                     const unsigned int iParameterSlotNumber)
        : EffectParameterSlotBase(group,
                                  iParameterSlotNumber) {
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
    //qDebug() << debugString() << "destroyed";
    // m_pControlLoaded and m_pControlType are deleted by ~EffectParameterSlotBase
    delete m_pControlValue;
}

void EffectButtonParameterSlot::loadEffect(EffectPointer pEffect) {
    //qDebug() << debugString() << "loadEffect" << (pEffect ? pEffect->getManifest().name() : "(null)");
    if (m_pEffectParameter) {
        clear();
    }

    if (pEffect) {
        m_pEffect = pEffect;
        // Returns null if it doesn't have a parameter for that number
        m_pEffectParameter = pEffect->getButtonParameterForSlot(m_iParameterSlotNumber);

        if (m_pEffectParameter) {
            // Set the number of states
            int numStates = math_max(m_pEffectParameter->manifest()->getSteps().size(), 2);
            m_pControlValue->setStates(numStates);
            //qDebug() << debugString() << "Loading effect parameter" << m_pEffectParameter->name();
            double dValue = m_pEffectParameter->getValue();
            double dMinimum = m_pEffectParameter->getMinimum();
            double dMinimumLimit = dMinimum; // TODO(rryan) expose limit from EffectParameter
            double dMaximum = m_pEffectParameter->getMaximum();
            double dMaximumLimit = dMaximum; // TODO(rryan) expose limit from EffectParameter
            double dDefault = m_pEffectParameter->getDefault();

            if (dValue > dMaximum || dValue < dMinimum ||
                dMinimum < dMinimumLimit || dMaximum > dMaximumLimit ||
                dDefault > dMaximum || dDefault < dMinimum) {
                qWarning() << debugString() << "WARNING: EffectParameter does not satisfy basic sanity checks.";
            }

            // qDebug() << debugString()
            //         << QString("Val: %1 Min: %2 MinLimit: %3 Max: %4 MaxLimit: %5 Default: %6")
            //         .arg(dValue).arg(dMinimum).arg(dMinimumLimit).arg(dMaximum).arg(dMaximumLimit).arg(dDefault);

            m_pControlValue->set(dValue);
            m_pControlValue->setDefaultValue(dDefault);
            EffectManifestParameter::ControlHint type = m_pEffectParameter->getControlHint();
            // TODO(rryan) expose this from EffectParameter
            m_pControlType->forceSet(static_cast<double>(type));
            // Default loaded parameters to loaded and unlinked
            m_pControlLoaded->forceSet(1.0);

            connect(m_pEffectParameter,
                    &EffectParameter::valueChanged,
                    this,
                    &EffectButtonParameterSlot::slotParameterValueChanged);
        }
    }
    emit updated();
}

void EffectButtonParameterSlot::clear() {
    //qDebug() << debugString() << "clear";
    if (m_pEffectParameter) {
        m_pEffectParameter->disconnect(this);
        m_pEffectParameter = NULL;
    }

    m_pEffect.clear();
    m_pControlLoaded->forceSet(0.0);
    m_pControlValue->set(0.0);
    m_pControlValue->setDefaultValue(0.0);
    m_pControlType->forceSet(0.0);
    emit updated();
}

void EffectButtonParameterSlot::slotParameterValueChanged(double value) {
    //qDebug() << debugString() << "slotParameterValueChanged" << value.toDouble();
    m_pControlValue->set(value);
}

void EffectButtonParameterSlot::slotValueChanged(double v) {
    if (m_pEffectParameter) {
        m_pEffectParameter->setValue(v);
    }
}

QDomElement EffectButtonParameterSlot::toXml(QDomDocument* doc) const {
    QDomElement parameterElement;
    if (m_pEffectParameter != nullptr) {
        parameterElement = doc->createElement(EffectXml::Parameter);
        XmlParse::addElement(*doc, parameterElement,
                             EffectXml::ParameterValue,
                             QString::number(m_pControlValue->get()));
    }

    return parameterElement;
}

void EffectButtonParameterSlot::loadParameterSlotFromXml(const QDomElement&
                                                  parameterElement) {
    if (m_pEffectParameter == nullptr) {
        return;
    }
    if (!parameterElement.hasChildNodes()) {
        m_pControlValue->reset();
    } else {
        bool conversionWorked = false;
        double value = XmlParse::selectNodeDouble(parameterElement,
                                                  EffectXml::ParameterValue,
                                                  &conversionWorked);
        if (conversionWorked) {
            // Need to use setParameterFrom(..., nullptr) here to
            // trigger valueChanged() signal emission and execute slotValueChanged()
            m_pControlValue->setParameterFrom(value, nullptr);
        }
        // If the conversion failed, the default value is kept.
    }
}
