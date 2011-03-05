#include <QtDebug>
#include <QMutexLocker>

#include "effects/effectparameter.h"

EffectParameter::EffectParameter(QObject* pParent, EffectManifestParameterPointer pParameter)
        : QObject(),
          m_mutex(QMutex::Recursive),
          m_pParameter(pParameter) {
    Q_ASSERT(pParameter); // Should never happen.

    qDebug() << debugString() << "Constructing new EffectParameter from EffectManifestParameter:"
             << m_pParameter->id();
    switch (m_pParameter->valueHint()) {
        case EffectManifestParameter::VALUE_BOOLEAN:
            // Minimum and maximum are undefined for a boolean.
            m_minimum = QVariant();
            m_maximum = QVariant();
            if (m_pParameter->hasDefault() && m_pParameter->getDefault().canConvert<bool>()) {
                m_default = m_pParameter->getDefault();
            } else {
                // Default to false if no default is given.
                m_default = QVariant(false);
            }
            m_value = m_default;
            break;
        case EffectManifestParameter::VALUE_INTEGRAL:
            m_minimum = m_pParameter->hasMinimum() && m_pParameter->getMinimum().canConvert<int>() ?
                    m_pParameter->getMinimum() : QVariant(0);
            m_maximum = m_pParameter->hasMaximum() && m_pParameter->getMinimum().canConvert<int>() ?
                    m_pParameter->getMaximum() : QVariant(1);

            // Sanity check the maximum and minimum
            if (clampRanges()) {
                qDebug() << debugString() << "WARNING: Parameter maximum is less than the minimum.";
            }

            // If the parameter specifies a default, set that. Otherwise use the minimum
            // value.
            if (m_pParameter->hasDefault() && m_pParameter->getDefault().canConvert<int>()) {
                m_default = m_pParameter->getDefault();
                if (m_default.toInt() < m_minimum.toInt() || m_default.toInt() > m_maximum.toInt()) {
                    qDebug() << debugString() << "WARNING: Parameter default is outside of minimum/maximum range.";
                    m_default = m_minimum;
                }
            } else {
                m_default = m_minimum;
            }

            // Finally, set the value to the default.
            m_value = m_default;
            break;
        case EffectManifestParameter::VALUE_UNKNOWN: // Treat unknown like float
        case EffectManifestParameter::VALUE_FLOAT:
            m_minimum = m_pParameter->hasMinimum() && m_pParameter->getMinimum().canConvert<double>() ?
                    m_pParameter->getMinimum() : QVariant(0.0f);
            m_maximum = m_pParameter->hasMaximum() && m_pParameter->getMinimum().canConvert<double>() ?
                    m_pParameter->getMaximum() : QVariant(1.0f);
            // Sanity check the maximum and minimum
            if (m_minimum.toDouble() > m_maximum.toDouble()) {
                qDebug() << debugString() << "WARNING: Parameter maximum is less than the minimum.";
                m_maximum = m_minimum;
            }

            // If the parameter specifies a default, set that. Otherwise use the minimum
            // value.
            if (m_pParameter->hasDefault() && m_pParameter->getDefault().canConvert<double>()) {
                m_default = m_pParameter->getDefault();
                if (m_default.toDouble() < m_minimum.toDouble() || m_default.toDouble() > m_maximum.toDouble()) {
                    qDebug() << debugString() << "WARNING: Parameter default is outside of minimum/maximum range.";
                    m_default = m_minimum;
                }
            } else {
                m_default = m_minimum;
            }

            // Finally, set the value to the default.
            m_value = m_default;
            break;
        default:
            qDebug() << debugString() << "ERROR: Unhandled valueHint";
            break;
    }
}

EffectParameter::~EffectParameter() {
    qDebug() << debugString() << "destroyed";
}

const QString EffectParameter::name() const {
    QMutexLocker locker(&m_mutex);
    return m_pParameter->name();
}

const QString EffectParameter::description() const {
    QMutexLocker locker(&m_mutex);
    return m_pParameter->description();
}

// static
bool EffectParameter::clampValue(EffectManifestParameter::ValueHint valueHint, QVariant& value,
                                 const QVariant& minimum, const QVariant& maximum) {
    switch (valueHint) {
        case EffectManifestParameter::VALUE_BOOLEAN:
            break;
        case EffectManifestParameter::VALUE_INTEGRAL:
            if (value.toInt() < minimum.toInt()) {
                value = minimum;
                return true;
            } else if (value.toInt() > maximum.toInt()) {
                value = maximum;
                return true;
            }
            break;
        case EffectManifestParameter::VALUE_FLOAT:
        case EffectManifestParameter::VALUE_UNKNOWN:
            if (value.toDouble() < minimum.toDouble()) {
                value = minimum;
                return true;
            } else if (value.toDouble() > maximum.toDouble()) {
                value = maximum;
                return true;
            }
            break;
        default:
            qDebug() << "ERROR: Unhandled valueHint";
            break;
    }
    return false;
}

bool EffectParameter::clampValue() {
    return clampValue(m_pParameter->valueHint(), m_value, m_minimum, m_maximum);
}

bool EffectParameter::clampDefault() {
    return clampValue(m_pParameter->valueHint(), m_default, m_minimum, m_maximum);
}

bool EffectParameter::checkType(const QVariant& value) const {
    switch (m_pParameter->valueHint()) {
        case EffectManifestParameter::VALUE_BOOLEAN:
            return value.canConvert<bool>();
        case EffectManifestParameter::VALUE_INTEGRAL:
            return value.canConvert<int>();
        case EffectManifestParameter::VALUE_FLOAT:
        case EffectManifestParameter::VALUE_UNKNOWN:
            return value.canConvert<double>();
        default:
            qDebug() << debugString() << "ERROR: Unhandled valueHint";
            break;
    }
    return false;
}

bool EffectParameter::clampRanges() {
    switch (m_pParameter->valueHint()) {
        case EffectManifestParameter::VALUE_BOOLEAN:
            break;
        case EffectManifestParameter::VALUE_INTEGRAL:
            if (m_minimum.toInt() > m_maximum.toInt()) {
                m_maximum = m_minimum;
                return true;
            }
            break;
        case EffectManifestParameter::VALUE_FLOAT:
        case EffectManifestParameter::VALUE_UNKNOWN:
            if (m_minimum.toDouble() > m_maximum.toDouble()) {
                m_maximum = m_minimum;
                return true;
            }
            break;
        default:
            qDebug() << debugString() << "ERROR: Unhandled valueHint";
            break;
    }
    return false;
}

QVariant EffectParameter::getValue() const {
    QMutexLocker locker(&m_mutex);
    return m_value;
}

void EffectParameter::setValue(QVariant value) {
    QMutexLocker locker(&m_mutex);

    if (!checkType(value)) {
        qDebug() << debugString() << "WARNING: Value for minimum cannot be converted to suitable value, ignoring.";
        return;
    }

    switch (m_pParameter->valueHint()) {
        case EffectManifestParameter::VALUE_BOOLEAN:
            m_value = value.toBool();
            break;
        case EffectManifestParameter::VALUE_INTEGRAL:
            m_value = value.toInt();
            break;
        case EffectManifestParameter::VALUE_UNKNOWN: // treat unknown as float
        case EffectManifestParameter::VALUE_FLOAT:
            // TODO(XXX) Handle inf, -inf, and nan
            m_value = value.toDouble();
            break;
        default:
            qDebug() << debugString() << "ERROR: Unhandled valueHint";
            break;
    }

    if (clampValue()) {
        qDebug() << debugString() << "WARNING: Value was outside of limits, clamped.";
    }
}

QVariant EffectParameter::getDefault() const {
    QMutexLocker locker(&m_mutex);
    return m_default;
}

void EffectParameter::setDefault(QVariant dflt) {
    QMutexLocker locker(&m_mutex);
    if (!checkType(dflt)) {
        qDebug() << debugString() << "WARNING: Value for maximum cannot be converted to suitable value, ignoring.";
        return;
    }

    switch (m_pParameter->valueHint()) {
        case EffectManifestParameter::VALUE_BOOLEAN:
            m_default = dflt.toBool();
            break;
        case EffectManifestParameter::VALUE_INTEGRAL:
            m_default = dflt.toInt();
            break;
        case EffectManifestParameter::VALUE_UNKNOWN:
        case EffectManifestParameter::VALUE_FLOAT:
            m_default = dflt.toDouble();
            break;
        default:
            qDebug() << debugString() << "ERROR: Unhandled valueHint";
            break;
    }

    if (clampDefault()) {
        qDebug() << debugString() << "WARNING: Default parameter value was outside of range, clamped.";
    }
}

QVariant EffectParameter::getMinimum() const {
    QMutexLocker locker(&m_mutex);
    return m_minimum;
}

void EffectParameter::setMinimum(QVariant minimum) {
    QMutexLocker locker(&m_mutex);
    if (!checkType(minimum)) {
        qDebug() << debugString() << "WARNING: Value for minimum cannot be converted to suitable value, ignoring.";
        return;
    }

    switch (m_pParameter->valueHint()) {
        case EffectManifestParameter::VALUE_BOOLEAN:
            // Minimum doesn't apply to booleans
            break;
        case EffectManifestParameter::VALUE_INTEGRAL:
            m_minimum = minimum.toInt();

            if (m_pParameter->hasMinimum() && m_minimum.toInt() < m_pParameter->getMinimum().toInt()) {
                qDebug() << debugString() << "WARNING: Minimum value is less than plugin's absolute minimum, clamping.";
                m_minimum = m_pParameter->getMinimum();
            }

            if (m_minimum.toInt() > m_maximum.toInt()) {
                qDebug() << debugString() << "WARNING: New minimum was above maximum, clamped.";
                m_minimum = m_maximum;
            }

            // There's a degenerate case here where the maximum could be lower
            // than the manifest minimum. If that's the case, then the minimum
            // value is currently below the manifest minimum. Since similar
            // guards exist in the setMaximum call, this should not be able to
            // happen.
            if (m_pParameter->hasMinimum()) {
                Q_ASSERT(m_minimum.toInt() >= m_pParameter->getMinimum().toInt());
            }
            break;
        case EffectManifestParameter::VALUE_UNKNOWN:
        case EffectManifestParameter::VALUE_FLOAT:
            m_minimum = minimum.toDouble();

            if (m_pParameter->hasMinimum() && m_minimum.toDouble() < m_pParameter->getMinimum().toDouble()) {
                qDebug() << debugString() << "WARNING: Minimum value is less than plugin's absolute minimum, clamping.";
                m_minimum = m_pParameter->getMinimum();
            }

            if (m_minimum.toDouble() > m_maximum.toDouble()) {
                qDebug() << debugString() << "WARNING: New minimum was above maximum, clamped.";
                m_minimum = m_maximum;
            }

            // There's a degenerate case here where the maximum could be lower
            // than the manifest minimum. If that's the case, then the minimum
            // value is currently below the manifest minimum. Since similar
            // guards exist in the setMaximum call, this should not be able to
            // happen.
            if (m_pParameter->hasMinimum()) {
                Q_ASSERT(m_minimum.toDouble() >= m_pParameter->getMinimum().toDouble());
            }
            break;
        default:
            qDebug() << debugString() << "ERROR: Unhandled valueHint";
            break;
    }

    if (clampValue()) {
        qDebug() << debugString() << "WARNING: Value was outside of new minimum, clamped.";
    }

    if (clampDefault()) {
        qDebug() << debugString() << "WARNING: Default was outside of new minimum, clamped.";
    }
}

QVariant EffectParameter::getMaximum() const {
    QMutexLocker locker(&m_mutex);
    return m_maximum;
}

void EffectParameter::setMaximum(QVariant maximum) {
    QMutexLocker locker(&m_mutex);
    if (!checkType(maximum)) {
        qDebug() << debugString() << "WARNING: Value for maximum cannot be converted to suitable value, ignoring.";
        return;
    }

    switch (m_pParameter->valueHint()) {
        case EffectManifestParameter::VALUE_BOOLEAN:
            // Maximum doesn't apply to booleans
            break;
        case EffectManifestParameter::VALUE_INTEGRAL:
            m_maximum = maximum.toInt();

            if (m_pParameter->hasMaximum() && m_maximum.toInt() > m_pParameter->getMaximum().toInt()) {
                qDebug() << debugString() << "WARNING: Maximum value is less than plugin's absolute maximum, clamping.";
                m_maximum = m_pParameter->getMaximum();
            }

            if (m_maximum.toInt() < m_minimum.toInt()) {
                qDebug() << debugString() << "WARNING: New maximum was below the minimum, clamped.";
                m_maximum = m_minimum;
            }

            // There's a degenerate case here where the minimum could be larger
            // than the manifest maximum. If that's the case, then the maximum
            // value is currently above the manifest maximum. Since similar
            // guards exist in the setMinimum call, this should not be able to
            // happen.
            if (m_pParameter->hasMaximum()) {
                Q_ASSERT(m_maximum.toInt() <= m_pParameter->getMaximum().toInt());
            }
            break;
        case EffectManifestParameter::VALUE_UNKNOWN:
        case EffectManifestParameter::VALUE_FLOAT:
            m_maximum = maximum.toDouble();

            if (m_pParameter->hasMaximum() && m_maximum.toDouble() > m_pParameter->getMaximum().toDouble()) {
                qDebug() << debugString() << "WARNING: Maximum value is less than plugin's absolute maximum, clamping.";
                m_maximum = m_pParameter->getMaximum();
            }

            if (m_maximum.toDouble() < m_minimum.toDouble()) {
                qDebug() << debugString() << "WARNING: New maximum was below the minimum, clamped.";
                m_maximum = m_minimum;
            }

            // There's a degenerate case here where the minimum could be larger
            // than the manifest maximum. If that's the case, then the maximum
            // value is currently above the manifest maximum. Since similar
            // guards exist in the setMinimum call, this should not be able to
            // happen.
            if (m_pParameter->hasMaximum()) {
                Q_ASSERT(m_maximum.toDouble() <= m_pParameter->getMaximum().toDouble());
            }
            break;
        default:
            qDebug() << debugString() << "ERROR: Unhandled valueHint";
            break;
    }

    if (clampValue()) {
        qDebug() << debugString() << "WARNING: Value was outside of new maximum, clamped.";
    }

    if (clampDefault()) {
        qDebug() << debugString() << "WARNING: Default was outside of new maximum, clamped.";
    }
}

