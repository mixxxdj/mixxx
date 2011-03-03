#include <QtDebug>
#include <QMutexLocker>

#include "defs.h"
#include "effects/effectparameterslot.h"

EffectParameterSlot::EffectParameterSlot(QObject* pParent, QString group, unsigned int parameterNumber)
        : QObject(pParent),
          m_mutex(QMutex::Recursive),
          m_group(group),
          m_iParameterNumber(parameterNumber) {
    QString basename = QString("parameter%1").arg(parameterNumber);

    m_pControlEnabled = new ControlObject(ConfigKey(group, QString("%1_enabled").arg(basename)));
    m_pControlValue = new ControlObject(ConfigKey(group, QString("%1_value").arg(basename)));
    m_pControlValueNormalized = new ControlObject(ConfigKey(group, QString("%1_value_normalized").arg(basename)));
    m_pControlValueType = new ControlObject(ConfigKey(group, QString("%1_value_type").arg(basename)));
    m_pControlValueDefault = new ControlObject(ConfigKey(group, QString("%1_value_default").arg(basename)));
    m_pControlValueMaximum = new ControlObject(ConfigKey(group, QString("%1_value_max").arg(basename)));
    m_pControlValueMaximumLimit = new ControlObject(ConfigKey(group, QString("%1_value_max_limit").arg(basename)));
    m_pControlValueMinimum = new ControlObject(ConfigKey(group, QString("%1_value_min").arg(basename)));
    m_pControlValueMinimumLimit = new ControlObject(ConfigKey(group, QString("%1_value_min_limit").arg(basename)));

    connect(m_pControlEnabled, SIGNAL(valueChanged(double)),
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
    delete m_pControlEnabled;
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
    qDebug() << debugString() << "loadEffect" << (pEffect ? pEffect->getManifest().name() : "(null)");
    QMutexLocker locker(&m_mutex);
    if (pEffect) {
        m_pEffect = pEffect;
        // TODO(XXX) setup control values
    } else {
        clear();
    }
}

void EffectParameterSlot::clear() {
    qDebug() << debugString() << "clear";
    QMutexLocker locker(&m_mutex);
    m_pEffect.clear();
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
    m_pControlValueNormalized->set(0.0f);
}

void EffectParameterSlot::slotValueNormalized(double v) {
    qDebug() << debugString() << "slotValueNormalized" << v;
    QMutexLocker locker(&m_mutex);
    // Clamp to [0.0, 1.0]
    if (v < 0.0f || v > 1.0f) {
        v = math_clamp(v, 0.0f, 1.0f);
        m_pControlValueNormalized->set(v);
    }

    // Now set the raw value to match the interpolated equivalent.
    double dMin = m_pControlValueMinimum->get();
    double dMax = m_pControlValueMaximum->get();
    // TODO(rryan) implement curve types, just linear for now.
    double dRaw = dMin + v * (dMax - dMin);
    m_pControlValue->set(dRaw);
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
        m_pControlValueMaximum->set(dMaxLimit);
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
        m_pControlValueMinimum->set(dMinLimit);
    }
}

void EffectParameterSlot::slotValueMinimumLimit(double v) {
    qDebug() << debugString() << "slotValueMinimumLimit" << v;
    QMutexLocker locker(&m_mutex);
    qDebug() << debugString() << "WARNING: Somebody has set a read-only control. Stability may be compromised.";
}

