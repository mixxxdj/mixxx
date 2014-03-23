#include <QtDebug>

#include "defs.h"
#include "controllinpotmeter.h"
#include "effects/effectparameterslot.h"
#include "controlobject.h"
#include "controlpushbutton.h"

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
          m_pEffectParameter(NULL) {
    QString itemPrefix = QString("parameter%1").arg(QString::number(iParameterNumber+1));
    m_pControlLoaded = new ControlObject(
        ConfigKey(m_group, itemPrefix + QString("_loaded")));
    m_pControlLinkType = new ControlPushButton(
        ConfigKey(m_group, itemPrefix + QString("_link_type")));
    m_pControlLinkType->setButtonMode(ControlPushButton::TOGGLE);
    m_pControlLinkType->setStates(EffectManifestParameter::NUM_LINK_TYPES);
    m_pControlValue = new ControlLinPotmeter(
        ConfigKey(m_group, itemPrefix));
    m_pControlValueType = new ControlObject(
        ConfigKey(m_group, itemPrefix + QString("_value_type")));
    m_pControlValueDefault = new ControlObject(
        ConfigKey(m_group, itemPrefix + QString("_value_default")));
    m_pControlValueMaximum = new ControlObject(
        ConfigKey(m_group, itemPrefix + QString("_value_max")));
    m_pControlValueMaximumLimit = new ControlObject(
        ConfigKey(m_group, itemPrefix + QString("_value_max_limit")));
    m_pControlValueMinimum = new ControlObject(
        ConfigKey(m_group, itemPrefix + QString("_value_min")));
    m_pControlValueMinimumLimit = new ControlObject(
        ConfigKey(m_group, itemPrefix + QString("_value_min_limit")));

    connect(m_pControlLinkType, SIGNAL(valueChanged(double)),
            this, SLOT(slotLinkType(double)));
    connect(m_pControlValue, SIGNAL(valueChanged(double)),
            this, SLOT(slotValueChanged(double)));
    connect(m_pControlValueMaximum, SIGNAL(valueChanged(double)),
            this, SLOT(slotValueMaximum(double)));
    connect(m_pControlValueMinimum, SIGNAL(valueChanged(double)),
            this, SLOT(slotValueMinimum(double)));

    // Read-only controls.
    m_pControlValueType->connectValueChangeRequest(
        this, SLOT(slotValueType(double)), Qt::AutoConnection);
    m_pControlValueDefault->connectValueChangeRequest(
        this, SLOT(slotValueDefault(double)), Qt::AutoConnection);
    m_pControlLoaded->connectValueChangeRequest(
        this, SLOT(slotLoaded(double)), Qt::AutoConnection);
    m_pControlValueMinimumLimit->connectValueChangeRequest(
        this, SLOT(slotValueMaximumLimit(double)), Qt::AutoConnection);
    m_pControlValueMaximumLimit->connectValueChangeRequest(
        this, SLOT(slotValueMinimumLimit(double)), Qt::AutoConnection);

    clear();
}

EffectParameterSlot::~EffectParameterSlot() {
    //qDebug() << debugString() << "destroyed";
    m_pEffectParameter = NULL;
    m_pEffect.clear();
    delete m_pControlLoaded;
    delete m_pControlLinkType;
    delete m_pControlValue;
    delete m_pControlValueType;
    delete m_pControlValueDefault;
    delete m_pControlValueMaximum;
    delete m_pControlValueMaximumLimit;
    delete m_pControlValueMinimum;
    delete m_pControlValueMinimumLimit;
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
            qDebug() << debugString() << "Loading effect parameter" << m_pEffectParameter->name();
            double dValue = m_pEffectParameter->getValue().toDouble();
            double dMinimum = m_pEffectParameter->getMinimum().toDouble();
            double dMinimumLimit = dMinimum; // TODO(rryan) expose limit from EffectParameter
            double dMaximum = m_pEffectParameter->getMaximum().toDouble();
            double dMaximumLimit = dMaximum; // TODO(rryan) expose limit from EffectParameter
            double dDefault = m_pEffectParameter->getDefault().toDouble();

            if (dValue > dMaximum || dValue < dMinimum ||
                dMinimum < dMinimumLimit || dMaximum > dMaximumLimit ||
                dDefault > dMaximum || dDefault < dMinimum) {
                qDebug() << debugString() << "WARNING: EffectParameter does not satisfy basic sanity checks.";
            }

            qDebug() << debugString()
                    << QString("Val: %1 Min: %2 MinLimit: %3 Max: %4 MaxLimit: %5 Default: %6")
                    .arg(dValue).arg(dMinimum).arg(dMinimumLimit).arg(dMaximum).arg(dMaximumLimit).arg(dDefault);

            m_pControlValue->set(dValue);
            m_pControlValue->setDefaultValue(dDefault);
            m_pControlValue->setRange(dMinimum, dMaximum, false);
            m_pControlValueMinimum->set(dMinimum);
            m_pControlValueMinimumLimit->setAndConfirm(dMinimumLimit);
            m_pControlValueMaximum->set(dMaximum);
            m_pControlValueMaximumLimit->setAndConfirm(dMaximumLimit);
            // TODO(rryan) expose this from EffectParameter
            m_pControlValueType->setAndConfirm(0);
            m_pControlValueDefault->setAndConfirm(dDefault);
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
    m_pControlValueType->setAndConfirm(0.0);
    m_pControlValueDefault->setAndConfirm(0.0);
    m_pControlValueMaximum->set(0.0);
    m_pControlValueMaximumLimit->setAndConfirm(0.0);
    m_pControlValueMinimum->set(0.0);
    m_pControlValueMinimumLimit->setAndConfirm(0.0);
    m_pControlLinkType->set(EffectManifestParameter::LINK_NONE);
    emit(updated());
}

void EffectParameterSlot::slotLoaded(double v) {
    qDebug() << debugString() << "slotLoaded" << v;
    qDebug() << "WARNING: loaded is a read-only control.";
}

void EffectParameterSlot::slotLinkType(double v) {
    qDebug() << debugString() << "slotLinkType" << v;
    if (m_pEffectParameter) {
        m_pEffectParameter->setLinkType(
            static_cast<EffectManifestParameter::LinkType>(v));
    }
}

void EffectParameterSlot::slotValueChanged(double v) {
    qDebug() << debugString() << "slotValueChanged" << v;

    double dMin = m_pControlValueMinimum->get();
    double dMax = m_pControlValueMaximum->get();
    if (v < dMin || v > dMax) {
        qDebug() << debugString() << "value out of limits";
        v = math_clamp(v, dMin, dMax);
        m_pControlValue->set(v);
    }

    if (m_pEffectParameter) {
        m_pEffectParameter->setValue(v);
    }
}

void EffectParameterSlot::slotValueType(double v) {
    qDebug() << debugString() << "slotValueType" << v;
    qDebug() << "WARNING: value_type is a read-only control.";
}

void EffectParameterSlot::slotValueDefault(double v) {
    qDebug() << debugString() << "slotValueDefault" << v;
    qDebug() << "WARNING: value_default is a read-only control.";
}

void EffectParameterSlot::slotValueMaximum(double v) {
    qDebug() << debugString() << "slotValueMaximum" << v;
    double dMaxLimit = m_pControlValueMaximumLimit->get();
    if (v > dMaxLimit) {
        qDebug() << "WARNING: Maximum parameter value is out of limits.";
        v = dMaxLimit;
        m_pControlValueMaximum->set(v);
    }
    if (m_pEffectParameter) {
        m_pEffectParameter->setMaximum(v);
    }
}

void EffectParameterSlot::slotValueMaximumLimit(double v) {
    qDebug() << debugString() << "slotValueMaximumLimit" << v;
    qDebug() << "WARNING: value_max_limit is a read-only control.";
}

void EffectParameterSlot::slotValueMinimum(double v) {
    qDebug() << debugString() << "slotValueMinimum" << v;
    double dMinLimit = m_pControlValueMinimumLimit->get();
    if (v < dMinLimit) {
        qDebug() << "WARNING: Minimum parameter value is out of limits.";
        v = dMinLimit;
        m_pControlValueMinimum->set(v);
    }

    if (m_pEffectParameter) {
        m_pEffectParameter->setMinimum(v);
    }
}

void EffectParameterSlot::slotValueMinimumLimit(double v) {
    qDebug() << debugString() << "slotValueMinimumLimit" << v;
    qDebug() << "WARNING: value_min_limit is a read-only control.";
}

void EffectParameterSlot::slotParameterValueChanged(QVariant value) {
    m_pControlValue->set(value.toDouble());
}
