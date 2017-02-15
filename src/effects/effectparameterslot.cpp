#include <QtDebug>

#include "control/controleffectknob.h"
#include "effects/effectparameterslot.h"
#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "controllers/softtakeover.h"

EffectParameterSlot::EffectParameterSlot(const QString& group, const unsigned int iParameterSlotNumber)
        : EffectParameterSlotBase(group, iParameterSlotNumber) {
    QString itemPrefix = formatItemPrefix(iParameterSlotNumber);
    m_pControlLoaded = new ControlObject(
            ConfigKey(m_group, itemPrefix + QString("_loaded")));
    m_pControlLinkType = new ControlPushButton(
            ConfigKey(m_group, itemPrefix + QString("_link_type")));
    m_pControlLinkType->setButtonMode(ControlPushButton::TOGGLE);
    m_pControlLinkType->setStates(EffectManifestParameter::NUM_LINK_TYPES);
    m_pControlLinkInverse = new ControlPushButton(
            ConfigKey(m_group, itemPrefix + QString("_link_inverse")));
    m_pControlLinkInverse->setButtonMode(ControlPushButton::TOGGLE);
    m_pControlValue = new ControlEffectKnob(
            ConfigKey(m_group, itemPrefix));
    m_pControlType = new ControlObject(
            ConfigKey(m_group, itemPrefix + QString("_type")));

    m_pControlLinkType->connectValueChangeRequest(
            this, SLOT(slotLinkTypeChanging(double)));
    connect(m_pControlLinkInverse, SIGNAL(valueChanged(double)),
            this, SLOT(slotLinkInverseChanged(double)));
    connect(m_pControlValue, SIGNAL(valueChanged(double)),
            this, SLOT(slotValueChanged(double)));

    // Read-only controls.
    m_pControlType->setReadOnly();
    m_pControlLoaded->setReadOnly();

    m_pSoftTakeover = new SoftTakeover();

    clear();
}

EffectParameterSlot::~EffectParameterSlot() {
    //qDebug() << debugString() << "destroyed";
    delete m_pControlValue;
    delete m_pSoftTakeover;
    delete m_pControlLinkType;
    delete m_pControlLinkInverse;
}

void EffectParameterSlot::loadEffect(EffectPointer pEffect) {
    //qDebug() << debugString() << "loadEffect" << (pEffect ? pEffect->getManifest().name() : "(null)");
    clear();
    if (pEffect) {
        // Returns null if it doesn't have a parameter for that number
        m_pEffectParameter = pEffect->getKnobParameterForSlot(m_iParameterSlotNumber);

        if (m_pEffectParameter) {
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

            //qDebug() << debugString()
            //         << QString("Val: %1 Min: %2 MinLimit: %3 Max: %4 MaxLimit: %5 Default: %6")
            //         .arg(dValue).arg(dMinimum).arg(dMinimumLimit).arg(dMaximum).arg(dMaximumLimit).arg(dDefault);

            EffectManifestParameter::ControlHint type = m_pEffectParameter->getControlHint();
            m_pControlValue->setBehaviour(type, dMinimum, dMaximum);
            m_pControlValue->setDefaultValue(dDefault);
            m_pControlValue->set(dValue);
            // TODO(rryan) expose this from EffectParameter
            m_pControlType->forceSet(static_cast<double>(type));
            // Default loaded parameters to loaded and unlinked
            m_pControlLoaded->forceSet(1.0);

            m_pControlLinkType->set(m_pEffectParameter->getDefaultLinkType());
            m_pControlLinkInverse->set(
                static_cast<double>(m_pEffectParameter->getDefaultLinkInversion()));

            connect(m_pEffectParameter, SIGNAL(valueChanged(double)),
                    this, SLOT(slotParameterValueChanged(double)));
        }
    }
    emit(updated());
}

void EffectParameterSlot::clear() {
    //qDebug() << debugString() << "clear";
    if (m_pEffectParameter) {
        m_pEffectParameter->disconnect(this);
        m_pEffectParameter = NULL;
    }

    m_pControlLoaded->forceSet(0.0);
    m_pControlValue->set(0.0);
    m_pControlValue->setDefaultValue(0.0);
    m_pControlType->forceSet(0.0);
    m_pControlLinkType->setAndConfirm(EffectManifestParameter::LINK_NONE);
    m_pSoftTakeover->setThreshold(SoftTakeover::kDefaultTakeoverThreshold);
    m_pControlLinkInverse->set(0.0);
    emit(updated());
}

void EffectParameterSlot::slotParameterValueChanged(double value) {
    //qDebug() << debugString() << "slotParameterValueChanged" << value.toDouble();
    m_pControlValue->set(value);
}

void EffectParameterSlot::slotLinkTypeChanging(double v) {
    m_pSoftTakeover->ignoreNext();
    if (v > EffectManifestParameter::LINK_LINKED) {
        double neutral = m_pEffectParameter->getNeutralPointOnScale();
        if (neutral > 0.0 && neutral < 1.0) {
            // Knob is already a split knob, meaning it has a positive and
            // negative effect if it's twisted above the neutral point or
            // below the neutral point.
            // Toggle back to 0
            v = EffectManifestParameter::LINK_NONE;
        }
    }
    if (static_cast<int>(v) == EffectManifestParameter::LINK_LINKED_LEFT ||
            static_cast<int>(v) == EffectManifestParameter::LINK_LINKED_RIGHT) {
        m_pSoftTakeover->setThreshold(
                SoftTakeover::kDefaultTakeoverThreshold * 2.0);
    } else {
        m_pSoftTakeover->setThreshold(SoftTakeover::kDefaultTakeoverThreshold);
    }
    m_pControlLinkType->setAndConfirm(v);
}

void EffectParameterSlot::slotLinkInverseChanged(double v) {
    Q_UNUSED(v);
    m_pSoftTakeover->ignoreNext();
}

void EffectParameterSlot::onEffectMetaParameterChanged(double parameter, bool force) {
    m_dChainParameter = parameter;
    if (m_pEffectParameter != NULL) {
        // Intermediate cast to integer is needed for VC++.
        EffectManifestParameter::LinkType type =
                static_cast<EffectManifestParameter::LinkType>(
                        static_cast<int>(m_pControlLinkType->get()));

        bool inverse = m_pControlLinkInverse->toBool();

        switch (type) {
            case EffectManifestParameter::LINK_LINKED:
                if (parameter < 0.0 || parameter > 1.0) {
                    return;
                }
                {
                    double neutral = m_pEffectParameter->getNeutralPointOnScale();
                    if (neutral > 0.0 && neutral < 1.0) {
                        if (inverse) {
                            // the neutral position must stick where it is
                            neutral = 1.0 - neutral;
                        }
                        // Knob is already a split knob
                        // Match to center position of meta knob
                        if (parameter <= 0.5) {
                            parameter /= 0.5;
                            parameter *= neutral;
                        } else {
                            parameter -= 0.5;
                            parameter /= 0.5;
                            parameter *= 1 - neutral;
                            parameter += neutral;
                        }
                    }
                }
                break;
            case EffectManifestParameter::LINK_LINKED_LEFT:
                if (parameter >= 0.5 && parameter <= 1.0) {
                    parameter = 1;
                } else if (parameter >= 0.0 && parameter <= 0.5) {
                    parameter *= 2;
                } else {
                    return;
                }
                break;
            case EffectManifestParameter::LINK_LINKED_RIGHT:
                if (parameter >= 0.5 && parameter <= 1.0) {
                    parameter -= 0.5;
                    parameter *= 2;
                } else if (parameter >= 0.0 && parameter < 0.5) {
                    parameter = 0.0;
                } else {
                    return;
                }
                break;
            case EffectManifestParameter::LINK_LINKED_LEFT_RIGHT:
                if (parameter >= 0.5 && parameter <= 1.0) {
                    parameter -= 0.5;
                    parameter *= 2;
                } else if (parameter >= 0.0 && parameter < 0.5) {
                    parameter *= 2;
                    parameter = 1.0 - parameter;
                } else {
                    return;
                }
                break;
            case EffectManifestParameter::LINK_NONE:
            default:
                return;
        }

        if (inverse) {
            parameter = 1.0 - parameter;
        }

        //qDebug() << "onEffectMetaParameterChanged" << debugString() << parameter << "force?" << force;
        if (force) {
            m_pControlValue->setParameterFrom(parameter, NULL);
            // This ensures that softtakover is in sync for following updates
            m_pSoftTakeover->ignore(m_pControlValue, parameter);
        } else if (!m_pSoftTakeover->ignore(m_pControlValue, parameter)) {
            m_pControlValue->setParameterFrom(parameter, NULL);
        }
    }
}

void EffectParameterSlot::syncSofttakeover() {
    double parameter = m_pControlValue->getParameter();
    m_pSoftTakeover->ignore(m_pControlValue, parameter);
}

double EffectParameterSlot::getValueParameter() const {
    return m_pControlValue->getParameter();
}

void EffectParameterSlot::slotValueChanged(double v) {
    if (m_pEffectParameter) {
        m_pEffectParameter->setValue(v);
    }
}
