#include <QtDebug>

#include "controleffectknob.h"
#include "effects/effectparameterslot.h"
#include "controlobject.h"
#include "controlpushbutton.h"
#include "controllers/softtakeover.h"

EffectParameterSlot::EffectParameterSlot(const unsigned int iRackNumber,
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
    m_pControlValue = new ControlEffectKnob(
        ConfigKey(m_group, itemPrefix));
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


    m_pSoftTakeover = new SoftTakeover();

    clear();
}

EffectParameterSlot::~EffectParameterSlot() {
    //qDebug() << debugString() << "destroyed";
    m_pEffectParameter = NULL;
    m_pEffect.clear();
    delete m_pControlLoaded;
    delete m_pControlLinkType;
    delete m_pControlValue;
    delete m_pControlType;
    delete m_pSoftTakeover;
}

QString EffectParameterSlot::name() const {
    if (m_pEffectParameter) {
        return m_pEffectParameter->name();
    }
    return QString();
}

QString EffectParameterSlot::description() const {
    if (m_pEffectParameter) {
        return m_pEffectParameter->description();
    }
    return tr("No effect loaded.");
}

void EffectParameterSlot::loadEffect(EffectPointer pEffect) {
    //qDebug() << debugString() << "loadEffect" << (pEffect ? pEffect->getManifest().name() : "(null)");
    clear();
    if (pEffect) {
        m_pEffect = pEffect;
        // Returns null if it doesn't have a parameter for that number
        m_pEffectParameter = pEffect->getParameter(m_iParameterNumber);

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

            m_pControlValue->setRange(dMinimum, dMaximum, false);
            EffectManifestParameter::ControlHint type = m_pEffectParameter->getControlHint();
            m_pControlValue->setType(type);
            m_pControlValue->setDefaultValue(dDefault);
            m_pControlValue->set(dValue);
            // TODO(rryan) expose this from EffectParameter
            m_pControlType->setAndConfirm(static_cast<double>(type));
            // Default loaded parameters to loaded and unlinked
            m_pControlLoaded->setAndConfirm(1.0);
            m_pControlLinkType->set(m_pEffectParameter->getLinkType());

            connect(m_pEffectParameter, SIGNAL(valueChanged(QVariant)),
                    this, SLOT(slotParameterValueChanged(QVariant)));
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

    m_pEffect.clear();
    m_pControlLoaded->setAndConfirm(0.0);
    m_pControlValue->set(0.0);
    m_pControlValue->setDefaultValue(0.0);
    m_pControlType->setAndConfirm(0.0);
    m_pControlLinkType->set(EffectManifestParameter::LINK_NONE);
    emit(updated());
}

void EffectParameterSlot::slotLoaded(double v) {
    Q_UNUSED(v);
    //qDebug() << debugString() << "slotLoaded" << v;
    qWarning() << "WARNING: loaded is a read-only control.";
}

void EffectParameterSlot::slotLinkType(double v) {
    //qDebug() << debugString() << "slotLinkType" << v;
    if (m_pEffectParameter) {
        // Intermediate cast to integer is needed for VC++.
        m_pEffectParameter->setLinkType(
            static_cast<EffectManifestParameter::LinkType>(int(v)));
    }
}

EffectManifestParameter::LinkType EffectParameterSlot::getLinkType() const{
    //qDebug() << debugString() << "slotLinkType" << v;
    if (m_pEffectParameter) {
        return m_pEffectParameter->getLinkType();
    }
    return EffectManifestParameter::LINK_NONE;
}

void EffectParameterSlot::slotValueChanged(double v) {
    //qDebug() << debugString() << "slotValueChanged" << v;
    if (m_pEffectParameter) {
        m_pEffectParameter->setValue(v);
    }
}

void EffectParameterSlot::slotValueType(double v) {
    Q_UNUSED(v);
    //qDebug() << debugString() << "slotValueType" << v;
    qWarning() << "WARNING: value_type is a read-only control.";
}


void EffectParameterSlot::slotParameterValueChanged(QVariant value) {
    //qDebug() << debugString() << "slotParameterValueChanged" << value.toDouble();
    m_pControlValue->set(value.toDouble());
}

void EffectParameterSlot::onChainParameterChanged(double parameter) {
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
                if (!m_pSoftTakeover->ignore(m_pControlValue, parameter * 127)) {
                    m_pControlValue->setParameterFrom(parameter, NULL);
                }
                break;
            case EffectManifestParameter::LINK_NONE:
            default:
                break;
        }
    }
}

double EffectParameterSlot::getValueParameter() const {
    return m_pControlValue->getParameter();
}
