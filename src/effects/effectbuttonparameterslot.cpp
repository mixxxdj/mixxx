#include <QtDebug>

#include "controleffectknob.h"
#include "effects/effectbuttonparameterslot.h"
#include "controlobject.h"
#include "controlpushbutton.h"

EffectButtonParameterSlot::EffectButtonParameterSlot(const unsigned int iRackNumber,
                                         const unsigned int iChainNumber,
                                         const unsigned int iSlotNumber,
                                         const unsigned int iParameterNumber)
        : m_iRackNumber(iRackNumber),
          m_iChainNumber(iChainNumber),
          m_iSlotNumber(iSlotNumber),
          m_iParameterNumber(iParameterNumber),
          m_group(formatGroupString(m_iRackNumber, m_iChainNumber,
                                    m_iSlotNumber)),
          m_pEffectParameter(NULL),
          m_dChainParameter(0.0) {
    QString itemPrefix = formatItemPrefix(iParameterNumber);
    m_pControlLoaded = new ControlObject(
        ConfigKey(m_group, itemPrefix + QString("_loaded")));
    m_pControlLinkType = new ControlPushButton(
        ConfigKey(m_group, itemPrefix + QString("_link_type")));
    m_pControlLinkType->setButtonMode(ControlPushButton::TOGGLE);
    m_pControlLinkType->setStates(EffectManifestParameter::NUM_LINK_TYPES);
    m_pControlValue = new ControlPushButton(
        ConfigKey(m_group, itemPrefix));
    m_pControlValue->setButtonMode(ControlPushButton::POWERWINDOW);
    m_pControlType = new ControlObject(
        ConfigKey(m_group, itemPrefix + QString("_type")));

    connect(m_pControlLinkType, SIGNAL(valueChanged(double)),
            this, SLOT(slotLinkType(double)));
    connect(m_pControlValue, SIGNAL(valueChanged(double)),
            this, SLOT(slotValueChanged(double)));

    // Read-only controls.
    m_pControlType->connectValueChangeRequest(
        this, SLOT(slotValueType(double)), Qt::AutoConnection);
    m_pControlLoaded->connectValueChangeRequest(
        this, SLOT(slotLoaded(double)), Qt::AutoConnection);

    clear();
}

EffectButtonParameterSlot::~EffectButtonParameterSlot() {
    //qDebug() << debugString() << "destroyed";
    m_pEffectParameter = NULL;
    m_pEffect.clear();
    delete m_pControlLoaded;
    delete m_pControlLinkType;
    delete m_pControlValue;
    delete m_pControlType;
}

QString EffectButtonParameterSlot::name() const {
    if (m_pEffectParameter) {
        return m_pEffectParameter->name();
    }
    return QString();
}

QString EffectButtonParameterSlot::description() const {
    if (m_pEffectParameter) {
        return m_pEffectParameter->description();
    }
    return tr("No effect loaded.");
}

void EffectButtonParameterSlot::loadEffect(EffectPointer pEffect) {
    //qDebug() << debugString() << "loadEffect" << (pEffect ? pEffect->getManifest().name() : "(null)");
    clear();
    if (pEffect) {
        m_pEffect = pEffect;
        // Returns null if it doesn't have a parameter for that number
        m_pEffectParameter = pEffect->getButtonParameter(m_iParameterNumber);

        if (m_pEffectParameter) {
            //qDebug() << debugString() << "Loading effect parameter" << m_pEffectParameter->name();
            double dValue = m_pEffectParameter->getValue().toDouble();
            double dMinimum = m_pEffectParameter->getMinimum().toDouble();
            double dMinimumLimit = dMinimum; // TODO(rryan) expose limit from EffectParameter
            double dMaximum = m_pEffectParameter->getMaximum().toDouble();
            double dMaximumLimit = dMaximum; // TODO(rryan) expose limit from EffectParameter
            double dDefault = m_pEffectParameter->getDefault().toDouble();

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
            m_pControlType->setAndConfirm(static_cast<double>(type));
            // Default loaded parameters to loaded and unlinked
            m_pControlLoaded->setAndConfirm(1.0);
            m_pControlLinkType->set(m_pEffectParameter->getLinkType());

            connect(m_pEffectParameter, SIGNAL(valueChanged(QVariant)),
                    this, SLOT(slotParameterValueChanged(QVariant)));
        }

        // Update the newly loaded parameter to match the current chain
        // superknob if it is linked.
        onChainParameterChanged(m_dChainParameter);
    }
    emit(updated());
}

void EffectButtonParameterSlot::clear() {
    //qDebug() << debugString() << "clear";
    if (m_pEffectParameter) {
        m_pEffectParameter->disconnect(this);
        m_pEffectParameter = NULL;
    }

    m_pEffect.clear();
    m_pControlLoaded->setAndConfirm(0.0);
    m_pControlValue->set(0.0);
    m_pControlValue->setDefaultValue(0.0);
    m_pControlType->setAndConfirm(0.0);
    m_pControlLinkType->set(EffectManifestParameter::LINK_NONE);
    emit(updated());
}

void EffectButtonParameterSlot::slotLoaded(double v) {
    Q_UNUSED(v);
    //qDebug() << debugString() << "slotLoaded" << v;
    qWarning() << "WARNING: loaded is a read-only control.";
}

void EffectButtonParameterSlot::slotLinkType(double v) {
    //qDebug() << debugString() << "slotLinkType" << v;
    if (m_pEffectParameter) {
        // Intermediate cast to integer is needed for VC++.
        m_pEffectParameter->setLinkType(
            static_cast<EffectManifestParameter::LinkType>(int(v)));
    }
}

void EffectButtonParameterSlot::slotValueChanged(double v) {
    //qDebug() << debugString() << "slotValueChanged" << v;
    if (m_pEffectParameter) {
        m_pEffectParameter->setValue(v, 11);
    }
}

void EffectButtonParameterSlot::slotValueType(double v) {
    Q_UNUSED(v);
    //qDebug() << debugString() << "slotValueType" << v;
    qWarning() << "WARNING: value_type is a read-only control.";
}


void EffectButtonParameterSlot::slotParameterValueChanged(QVariant value) {
    //qDebug() << debugString() << "slotParameterValueChanged" << value.toDouble();
    m_pControlValue->set(value.toDouble());
}

void EffectButtonParameterSlot::onChainParameterChanged(double parameter) {
    m_dChainParameter = parameter;
    if (m_pEffectParameter != NULL) {
        switch (m_pEffectParameter->getLinkType()) {
            case EffectManifestParameter::LINK_INVERSE:
                parameter = 1.0 - parameter;
                // Intentional fall-through.
            case EffectManifestParameter::LINK_LINKED:
                if (parameter < 0.0 || parameter > 1.0) {
                    return;
                }
                m_pControlValue->setParameterFrom(parameter, NULL);
                break;
            case EffectManifestParameter::LINK_NONE:
            default:
                break;
        }
    }
}
