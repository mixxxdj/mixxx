#include <QtDebug>
#include <QMutexLocker>

#include "defs.h"
#include "controlpotmeter.h"
#include "effects/effectparameterslot.h"

EffectParameterSlot::EffectParameterSlot(QObject* pParent, const unsigned int iChainNumber,
                                         const unsigned int iSlotNumber,
                                         const unsigned int iParameterNumber)
        : QObject(),
          m_mutex(QMutex::Recursive),
          m_iChainNumber(iChainNumber),
          m_iSlotNumber(iSlotNumber),
          m_iParameterNumber(iParameterNumber),
          m_group(formatGroupString(iChainNumber, iSlotNumber, iParameterNumber)) {
    m_pControlEnabled = new ControlObject(ConfigKey(m_group, QString("enabled")));
    m_pControlLinked = new ControlObject(ConfigKey(m_group, QString("linked")));
    m_pControlValue = new ControlObject(ConfigKey(m_group, QString("value")));
    m_pControlValueNormalized = new ControlPotmeter(ConfigKey(m_group, QString("value_normalized")), 0.0, 1.0);
    m_pControlValueType = new ControlObject(ConfigKey(m_group, QString("value_type")));
    m_pControlValueDefault = new ControlObject(ConfigKey(m_group, QString("value_default")));
    m_pControlValueMaximum = new ControlObject(ConfigKey(m_group, QString("value_max")));
    m_pControlValueMaximumLimit = new ControlObject(ConfigKey(m_group, QString("value_max_limit")));
    m_pControlValueMinimum = new ControlObject(ConfigKey(m_group, QString("value_min")));
    m_pControlValueMinimumLimit = new ControlObject(ConfigKey(m_group, QString("value_min_limit")));

    connect(m_pControlEnabled, SIGNAL(valueChanged(double)),
            this, SLOT(slotEnabled(double)),
            Qt::DirectConnection);
    connect(m_pControlLinked, SIGNAL(valueChanged(double)),
            this, SLOT(slotEnabled(double)),
            Qt::DirectConnection);
    connect(m_pControlValue, SIGNAL(valueChanged(double)),
            this, SLOT(slotValue(double)),
            Qt::DirectConnection);
    connect(m_pControlValueNormalized, SIGNAL(valueChanged(double)),
            this, SLOT(slotValueNormalized(double)),
            Qt::DirectConnection);
    connect(m_pControlValueType, SIGNAL(valueChanged(double)),
            this, SLOT(slotValueType(double)),
            Qt::DirectConnection);
    connect(m_pControlValueDefault, SIGNAL(valueChanged(double)),
            this, SLOT(slotValueDefault(double)),
            Qt::DirectConnection);
    connect(m_pControlValueMaximum, SIGNAL(valueChanged(double)),
            this, SLOT(slotValueMaximum(double)),
            Qt::DirectConnection);
    connect(m_pControlValueMaximumLimit, SIGNAL(valueChanged(double)),
            this, SLOT(slotValueMaximumLimit(double)),
            Qt::DirectConnection);
    connect(m_pControlValueMinimum, SIGNAL(valueChanged(double)),
            this, SLOT(slotValueMinimum(double)),
            Qt::DirectConnection);
    connect(m_pControlValueMinimumLimit, SIGNAL(valueChanged(double)),
            this, SLOT(slotValueMinimumLimit(double)),
            Qt::DirectConnection);

    clear();
}

EffectParameterSlot::~EffectParameterSlot() {
    qDebug() << debugString() << "destroyed";
    m_pEffectParameter.clear();
    delete m_pControlEnabled;
    delete m_pControlLinked;
    delete m_pControlValue;
    delete m_pControlValueNormalized;
    delete m_pControlValueType;
    delete m_pControlValueDefault;
    delete m_pControlValueMaximum;
    delete m_pControlValueMaximumLimit;
    delete m_pControlValueMinimum;
    delete m_pControlValueMinimumLimit;
}

void EffectParameterSlot::loadEffect(EffectPointer pEffect) {
    qDebug() << debugString() << "loadEffect" << (pEffect ? pEffect->getManifest()->name() : "(null)");
    QMutexLocker locker(&m_mutex);
    if (pEffect) {
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
            double dNormalized = (dValue - dMinimum) / (dMaximum - dMinimum);

            qDebug() << debugString() << QString("Val: %1 Min: %2 MinLimit: %3 Max: %4 MaxLimit: %5 Default: %6 Norm: %7").arg(dValue).arg(dMinimum).arg(dMinimumLimit).arg(dMaximum).arg(dMaximumLimit).arg(dDefault).arg(dNormalized);

            m_pControlValue->set(dValue);
            // Convert to stupid GUI widget convention
            m_pControlValueNormalized->set(dNormalized);
            m_pControlValueMinimum->set(dMinimum);
            m_pControlValueMinimumLimit->set(dMinimumLimit);
            m_pControlValueMaximum->set(dMaximum);
            m_pControlValueMaximumLimit->set(dMaximumLimit);
            m_pControlValueType->set(0); // TODO(rryan) expose this from EffectParameter
            m_pControlValueDefault->set(dDefault);
            // Default loaded parameters to enabled and unlinked
            m_pControlEnabled->set(1.0f);
            m_pControlLinked->set(0.0f);
        }
    } else {
        clear();
    }
}

void EffectParameterSlot::clear() {
    qDebug() << debugString() << "clear";
    m_pEffectParameter.clear();
    m_pControlEnabled->set(0.0f);
    m_pControlValue->set(0.0f);
    m_pControlValueNormalized->set(0.0f);
    m_pControlValueType->set(0.0f);
    m_pControlValueDefault->set(0.0f);
    m_pControlValueMaximum->set(0.0f);
    m_pControlValueMaximumLimit->set(0.0f);
    m_pControlValueMinimum->set(0.0f);
    m_pControlValueMinimumLimit->set(0.0f);
}

void EffectParameterSlot::slotEnabled(double v) {
    qDebug() << debugString() << "slotEnabled" << v;
    QMutexLocker locker(&m_mutex);
    qDebug() << "WARNING: Somebody has set a read-only control. Stability may be compromised.";
    // TODO(rryan) add protection
}

void EffectParameterSlot::slotLinked(double v) {
    qDebug() << debugString() << "slotLinked" << v;
    QMutexLocker locker(&m_mutex);
}

void EffectParameterSlot::slotValue(double v) {
    qDebug() << debugString() << "slotValue" << v;
    QMutexLocker locker(&m_mutex);

    double dMin = m_pControlValueMinimum->get();
    double dMax = m_pControlValueMaximum->get();
    if (v < dMin || v > dMax) {
        qDebug() << debugString() << "value out of limits";
        v = math_clamp(v, dMin, dMax);
        m_pControlValue->set(v);
    }
    double dNormalized = (dMax - dMin > 0) ? (v - dMin) / (dMax - dMin) : 0.0f;
    m_pControlValueNormalized->set(dNormalized);

    if (m_pEffectParameter) {
        m_pEffectParameter->setValue(v);
    }
}

void EffectParameterSlot::slotValueNormalized(double v) {
    qDebug() << debugString() << "slotValueNormalized" << v;
    QMutexLocker locker(&m_mutex);

    // Clamp to [0.0, 1.0]
    if (v < 0.0f || v > 1.0f) {
        qDebug() << debugString() << "value out of limits";
        v = math_clamp(v, 0.0f, 1.0f);
        // Convert back to stupid control system format
        m_pControlValueNormalized->set(v);
    }

    // Now set the raw value to match the interpolated equivalent.
    double dMin = m_pControlValueMinimum->get();
    double dMax = m_pControlValueMaximum->get();
    // TODO(rryan) implement curve types, just linear for now.
    double dRaw = dMin + v * (dMax - dMin);
    qDebug() << debugString() << "Normalized set of" << v << "produces raw value of" << dRaw;
    m_pControlValue->set(dRaw);

    if (m_pEffectParameter) {
        m_pEffectParameter->setValue(dRaw);
    }
}

void EffectParameterSlot::slotValueType(double v) {
    qDebug() << debugString() << "slotValueType" << v;
    QMutexLocker locker(&m_mutex);
    qDebug() << debugString() << "WARNING: Somebody has set a read-only control. Stability may be compromised.";
}

void EffectParameterSlot::slotValueDefault(double v) {
    qDebug() << debugString() << "slotValueDefault" << v;
    QMutexLocker locker(&m_mutex);
    qDebug() << debugString() << "WARNING: Somebody has set a read-only control. Stability may be compromised.";
}

void EffectParameterSlot::slotValueMaximum(double v) {
    qDebug() << debugString() << "slotValueMaximum" << v;
    QMutexLocker locker(&m_mutex);
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
    QMutexLocker locker(&m_mutex);
    qDebug() << "WARNING: Somebody has set a read-only control. Stability may be compromised.";
}

void EffectParameterSlot::slotValueMinimum(double v) {
    qDebug() << debugString() << "slotValueMinimum" << v;
    QMutexLocker locker(&m_mutex);
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
    QMutexLocker locker(&m_mutex);
    qDebug() << debugString() << "WARNING: Somebody has set a read-only control. Stability may be compromised.";
}
