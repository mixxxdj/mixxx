#include <QtDebug>

#include "effects/effectparameter.h"

EffectParameter::EffectParameter(QObject* pParent, const EffectManifestParameter& parameter)
        : QObject(pParent),
          m_parameter(parameter) {

    switch (m_parameter.valueHint()) {
        case EffectManifestParameter::VALUE_BOOLEAN:
            // Minimum and maximum are undefined for a boolean.
            m_minimum = QVariant();
            m_maximum = QVariant();
            if (m_parameter.hasDefault() && m_parameter.getDefault().canConvert<bool>()) {
                m_default = m_parameter.getDefault();
            } else {
                // Default to false if no default is given.
                m_default = QVariant(false);
            }
            m_value = m_default;
            break;
        case EffectManifestParameter::VALUE_INTEGRAL:
            m_minimum = m_parameter.hasMinimum() && m_parameter.getMinimum().canConvert<int>() ?
                    m_parameter.getMinimum() : QVariant(0);
            m_maximum = m_parameter.hasMaximum() && m_parameter.getMinimum().canConvert<int>() ?
                    m_parameter.getMaximum() : QVariant(1);

            // Sanity check the maximum and minimum
            if (clampRanges()) {
                qDebug() << "WARNING: Parameter maximum is less than the minimum.";
            }

            // If the parameter specifies a default, set that. Otherwise use the minimum
            // value.
            if (m_parameter.hasDefault() && m_parameter.getDefault().canConvert<int>()) {
                m_default = m_parameter.getDefault();
                if (m_default.toInt() < m_minimum.toInt() || m_default.toInt() > m_maximum.toInt()) {
                    qDebug() << "WARNING: Parameter default is outside of minimum/maximum range.";
                    m_default = m_minimum;
                }
            } else {
                m_default = m_minimum;
            }

            // Finally, set the value to the default.
            m_value = m_default;
            break;
        case EffectManifestParameter::VALUE_UNKNOWN:
            // We're boned! Fall through to the float case.
        case EffectManifestParameter::VALUE_FLOAT:
            m_minimum = m_parameter.hasMinimum() && m_parameter.getMinimum().canConvert<double>() ?
                    m_parameter.getMinimum() : QVariant(0.0f);
            m_maximum = m_parameter.hasMaximum() && m_parameter.getMinimum().canConvert<double>() ?
                    m_parameter.getMaximum() : QVariant(1.0f);
            // Sanity check the maximum and minimum
            if (m_minimum.toDouble() > m_maximum.toDouble()) {
                qDebug() << "WARNING: Parameter maximum is less than the minimum.";
                m_maximum = m_minimum;
            }

            // If the parameter specifies a default, set that. Otherwise use the minimum
            // value.
            if (m_parameter.hasDefault() && m_parameter.getDefault().canConvert<double>()) {
                m_default = m_parameter.getDefault();
                if (m_default.toDouble() < m_minimum.toDouble() || m_default.toDouble() > m_maximum.toDouble()) {
                    qDebug() << "WARNING: Parameter default is outside of minimum/maximum range.";
                    m_default = m_minimum;
                }
            } else {
                m_default = m_minimum;
            }

            // Finally, set the value to the default.
            m_value = m_default;
            break;
        default:
            qDebug() << "ERROR: Unhandled valueHint";
            break;
    }
}

EffectParameter::~EffectParameter() {

}

const QString EffectParameter::name() const {
    return m_parameter.name();
}

const QString EffectParameter::description() const {
    return m_parameter.description();
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
    return clampValue(m_parameter.valueHint(), m_value, m_minimum, m_maximum);
}

bool EffectParameter::clampDefault() {
    return clampValue(m_parameter.valueHint(), m_default, m_minimum, m_maximum);
}

bool EffectParameter::checkType(const QVariant& value) const {
    switch (m_parameter.valueHint()) {
        case EffectManifestParameter::VALUE_BOOLEAN:
            return value.canConvert<bool>();
        case EffectManifestParameter::VALUE_INTEGRAL:
            return value.canConvert<int>();
        case EffectManifestParameter::VALUE_FLOAT:
        case EffectManifestParameter::VALUE_UNKNOWN:
            return value.canConvert<double>();
        default:
            qDebug() << "ERROR: Unhandled valueHint";
            break;
    }
    return false;
}

bool EffectParameter::clampRanges() {
    switch (m_parameter.valueHint()) {
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
            qDebug() << "ERROR: Unhandled valueHint";
            break;
    }
    return false;
}

QVariant EffectParameter::getValue() const {
    return m_value;
}

void EffectParameter::setValue(QVariant value) {
    if (!checkType(value)) {
        qDebug() << "WARNING: Value for minimum cannot be converted to suitable value, ignoring.";
        return;
    }

    switch (m_parameter.valueHint()) {
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
            qDebug() << "ERROR: Unhandled valueHint";
            break;
    }

    if (clampValue()) {
        qDebug() << "WARNING: Value was outside of limits, clamped.";
    }
}

QVariant EffectParameter::getDefault() const {
    return m_default;
}

void EffectParameter::setDefault(QVariant dflt) {
    if (!checkType(dflt)) {
        qDebug() << "WARNING: Value for maximum cannot be converted to suitable value, ignoring.";
        return;
    }

    switch (m_parameter.valueHint()) {
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
            qDebug() << "ERROR: Unhandled valueHint";
            break;
    }

    if (clampDefault()) {
        qDebug() << "WARNING: Default parameter value was outside of range, clamped.";
    }
}

QVariant EffectParameter::getMinimum() const {
    return m_minimum;
}

void EffectParameter::setMinimum(QVariant minimum) {
    if (!checkType(minimum)) {
        qDebug() << "WARNING: Value for minimum cannot be converted to suitable value, ignoring.";
        return;
    }

    switch (m_parameter.valueHint()) {
        case EffectManifestParameter::VALUE_BOOLEAN:
            // Minimum doesn't apply to booleans
            break;
        case EffectManifestParameter::VALUE_INTEGRAL:
            m_minimum = minimum.toInt();

            if (m_parameter.hasMinimum() && m_minimum.toInt() < m_parameter.getMinimum().toInt()) {
                qDebug() << "WARNING: Minimum value is less than plugin's absolute minimum, clamping.";
                m_minimum = m_parameter.getMinimum();
            }

            if (m_minimum.toInt() > m_maximum.toInt()) {
                qDebug() << "WARNING: New minimum was above maximum, clamped.";
                m_minimum = m_maximum;
            }

            // There's a degenerate case here where the maximum could be lower
            // than the manifest minimum. If that's the case, then the minimum
            // value is currently below the manifest minimum. Since similar
            // guards exist in the setMaximum call, this should not be able to
            // happen.
            if (m_parameter.hasMinimum()) {
                Q_ASSERT(m_minimum.toInt() >= m_parameter.getMinimum().toInt());
            }
            break;
        case EffectManifestParameter::VALUE_UNKNOWN:
        case EffectManifestParameter::VALUE_FLOAT:
            m_minimum = minimum.toDouble();

            if (m_parameter.hasMinimum() && m_minimum.toDouble() < m_parameter.getMinimum().toDouble()) {
                qDebug() << "WARNING: Minimum value is less than plugin's absolute minimum, clamping.";
                m_minimum = m_parameter.getMinimum();
            }

            if (m_minimum.toDouble() > m_maximum.toDouble()) {
                qDebug() << "WARNING: New minimum was above maximum, clamped.";
                m_minimum = m_maximum;
            }

            // There's a degenerate case here where the maximum could be lower
            // than the manifest minimum. If that's the case, then the minimum
            // value is currently below the manifest minimum. Since similar
            // guards exist in the setMaximum call, this should not be able to
            // happen.
            if (m_parameter.hasMinimum()) {
                Q_ASSERT(m_minimum.toDouble() >= m_parameter.getMinimum().toDouble());
            }
            break;
        default:
            qDebug() << "ERROR: Unhandled valueHint";
            break;
    }

    if (clampValue()) {
        qDebug() << "WARNING: Value was outside of new minimum, clamped.";
    }

    if (clampDefault()) {
        qDebug() << "WARNING: Default was outside of new minimum, clamped.";
    }
}

QVariant EffectParameter::getMaximum() const {
    return m_default;
}

void EffectParameter::setMaximum(QVariant maximum) {
    if (!checkType(maximum)) {
        qDebug() << "WARNING: Value for maximum cannot be converted to suitable value, ignoring.";
        return;
    }

    switch (m_parameter.valueHint()) {
        case EffectManifestParameter::VALUE_BOOLEAN:
            // Maximum doesn't apply to booleans
            break;
        case EffectManifestParameter::VALUE_INTEGRAL:
            m_maximum = maximum.toInt();

            if (m_parameter.hasMaximum() && m_maximum.toInt() > m_parameter.getMaximum().toInt()) {
                qDebug() << "WARNING: Maximum value is less than plugin's absolute maximum, clamping.";
                m_maximum = m_parameter.getMaximum();
            }

            if (m_maximum.toInt() < m_minimum.toInt()) {
                qDebug() << "WARNING: New maximum was below the minimum, clamped.";
                m_maximum = m_minimum;
            }

            // There's a degenerate case here where the minimum could be larger
            // than the manifest maximum. If that's the case, then the maximum
            // value is currently above the manifest maximum. Since similar
            // guards exist in the setMinimum call, this should not be able to
            // happen.
            if (m_parameter.hasMaximum()) {
                Q_ASSERT(m_maximum.toInt() <= m_parameter.getMaximum().toInt());
            }
            break;
        case EffectManifestParameter::VALUE_UNKNOWN:
        case EffectManifestParameter::VALUE_FLOAT:
            m_maximum = maximum.toDouble();

            if (m_parameter.hasMaximum() && m_maximum.toDouble() > m_parameter.getMaximum().toDouble()) {
                qDebug() << "WARNING: Maximum value is less than plugin's absolute maximum, clamping.";
                m_maximum = m_parameter.getMaximum();
            }

            if (m_maximum.toDouble() < m_minimum.toDouble()) {
                qDebug() << "WARNING: New maximum was below the minimum, clamped.";
                m_maximum = m_minimum;
            }

            // There's a degenerate case here where the minimum could be larger
            // than the manifest maximum. If that's the case, then the maximum
            // value is currently above the manifest maximum. Since similar
            // guards exist in the setMinimum call, this should not be able to
            // happen.
            if (m_parameter.hasMaximum()) {
                Q_ASSERT(m_maximum.toDouble() <= m_parameter.getMaximum().toDouble());
            }
            break;
        default:
            qDebug() << "ERROR: Unhandled valueHint";
            break;
    }

    if (clampValue()) {
        qDebug() << "WARNING: Value was outside of new maximum, clamped.";
    }

    if (clampDefault()) {
        qDebug() << "WARNING: Default was outside of new maximum, clamped.";
    }
}

