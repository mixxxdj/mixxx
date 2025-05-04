#pragma once

#include <QSharedPointer>
#include <QString>
#include <QVariant>
#include <QtDebug>

#include "effects/defs.h"
#include "util/assert.h"
#include "util/compatibility/qhash.h"

class EffectManifestParameter;
typedef QSharedPointer<EffectManifestParameter> EffectManifestParameterPointer;

class EffectManifestParameter {
  public:
    enum class ParameterType : int {
        Knob,
        Button,
        NumTypes,
    };

    enum class ValueScaler : int {
        Unknown = 0,
        Linear,
        LinearInverse,
        Logarithmic,
        LogarithmicInverse,
        /// A step rotary with integer steps arranged with equal distance on scale
        Integral,
        /// A step rotary, steps given by m_steps are arranged with equal
        /// distance on scale
        // Stepped,
        /// For button and enum controls, not accessible from many controllers,
        /// no linking to meta knob
        Toggle,
    };

    enum class UnitsHint : int {
        Unknown = 0,
        /// Multiples of a Beat
        Beats,
        Beat,
        Bar,
        BPM,
        Cent,
        Centimetre,
        Coefficient,
        Decibel,
        Degree,
        Frame,
        Hertz,
        Inch,
        KiloHertz,
        Kilometer,
        Meter,
        MegaHertz,
        Midinote,
        Mile,
        Minute,
        Millmeter,
        Millisecond,
        Octave,
        Percentage,
        /// Fraction of the Sample Rate
        SampleRate,
        Seconds,
        Semitone12tet,
        Time, // units?
    };

    const QHash<QString, UnitsHint> lv2UnitToUnitsHintHash{
            // Add custom LV2 units here (with correct case) and also
            // in unitsHintStringHash()
            {QString("bar"), UnitsHint::Bar},
            {QString("beat"), UnitsHint::Beat},
            {QString("bpm"), UnitsHint::BPM},
            {QString("cent"), UnitsHint::Cent},
            {QString("cm"), UnitsHint::Centimetre},
            {QString("coef"), UnitsHint::Coefficient},
            {QString("db"), UnitsHint::Decibel},
            {QString("degree"), UnitsHint::Degree},
            {QString("frame"), UnitsHint::Frame},
            {QString("hz"), UnitsHint::Hertz},
            {QString("inch"), UnitsHint::Inch},
            {QString("khz"), UnitsHint::KiloHertz},
            {QString("km"), UnitsHint::Kilometer},
            {QString("m"), UnitsHint::Meter},
            {QString("mhz"), UnitsHint::MegaHertz},
            {QString("midiNote"), UnitsHint::Midinote},
            {QString("mile"), UnitsHint::Mile},
            {QString("min"), UnitsHint::Minute},
            {QString("mm"), UnitsHint::Millmeter},
            {QString("ms"), UnitsHint::Millisecond},
            {QString("oct"), UnitsHint::Octave},
            {QString("pc"), UnitsHint::Percentage},
            {QString("s"), UnitsHint::Seconds},
            {QString("semitone12TET"), UnitsHint::Semitone12tet}};

    UnitsHint lv2UnitToUnitsHint(const QString& lv2) {
        QHash<QString, UnitsHint>::const_iterator uHintIt =
                lv2UnitToUnitsHintHash.find(lv2);
        if (uHintIt == lv2UnitToUnitsHintHash.constEnd()) {
            return UnitsHint::Unknown;
        }
        return uHintIt.value();
    }

    const QHash<UnitsHint, QString> unitsHintStringHash{
            {UnitsHint::BPM, QString("BPM")},
            {UnitsHint::Cent, QString("ct")},
            {UnitsHint::Centimetre, QString("cm")},
            {UnitsHint::Decibel, QString("dB")},
            {UnitsHint::Degree, QString("°")},
            {UnitsHint::Frame, QString("f")},
            {UnitsHint::Hertz, QString("Hz")},
            {UnitsHint::Inch, QString("in")},
            {UnitsHint::KiloHertz, QString("kHz")},
            {UnitsHint::Kilometer, QString("km")},
            {UnitsHint::Meter, QString("m")},
            {UnitsHint::MegaHertz, QString("MHz")},
            {UnitsHint::Mile, QString("mi")},
            {UnitsHint::Minute, QString("min")},
            {UnitsHint::Millmeter, QString("mm")},
            {UnitsHint::Millisecond, QString("ms")},
            {UnitsHint::Octave, QString("oct")},
            {UnitsHint::Percentage, QString("%")},
            {UnitsHint::Seconds, QString("s")},
            {UnitsHint::Semitone12tet, QString("semi")},
    };

    // Custom units we do not want to use in effect widgets
    QSet<QString> customUnitsBlacklist = {
            "(coef)",
            "G",
            "samp",
            "st"};

    bool ignoreCustomUnit(const QString& unit) {
        return customUnitsBlacklist.contains(unit);
    }

    enum class LinkType : int {
        /// Not controlled by the meta knob
        None = 0,
        /// Controlled by the meta knob as it is
        Linked,
        /// Controlled by the left side of the meta knob
        LinkedLeft,
        /// Controlled by the right side of the meta knob
        LinkedRight,
        /// Controlled by both sides of the meta knob
        LinkedLeftRight,
        NumLinkTypes,
    };

    static QString LinkTypeToString(LinkType type) {
        switch (type) {
        case LinkType::Linked:
            return QLatin1String("LINKED");
        case LinkType::LinkedLeft:
            return QLatin1String("LINKED_LEFT");
        case LinkType::LinkedRight:
            return QLatin1String("LINKED_RIGHT");
        case LinkType::LinkedLeftRight:
            return QLatin1String("LINKED_LEFT_RIGHT");
        default:
            return QLatin1String("NONE");
        }
    }

    static LinkType LinkTypeFromString(const QString& string) {
        if (string == QLatin1String("LINKED")) {
            return LinkType::Linked;
        } else if (string == QLatin1String("LINKED_LEFT")) {
            return LinkType::LinkedLeft;
        } else if (string == QLatin1String("LINKED_RIGHT")) {
            return LinkType::LinkedRight;
        } else if (string == QLatin1String("LINKED_LEFT_RIGHT")) {
            return LinkType::LinkedLeftRight;
        } else {
            return LinkType::None;
        }
    }

    enum class LinkInversion : int {
        NotInverted = 0,
        Inverted = 1
    };

    EffectManifestParameter()
            : m_valueScaler(ValueScaler::Unknown),
              m_unitsHint(UnitsHint::Unknown),
              m_defaultLinkType(LinkType::None),
              m_defaultLinkInversion(LinkInversion::NotInverted),
              m_neutralPointOnScale(0.0),
              m_default(0),
              m_minimum(0),
              m_maximum(1.0) {
    }

    ~EffectManifestParameter() {
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Parameter Information
    ////////////////////////////////////////////////////////////////////////////////

    const QString& id() const {
        return m_id;
    }
    void setId(const QString& id) {
        m_id = id;
    }

    const QString& name() const {
        return m_name;
    }
    void setName(const QString& name) {
        m_name = name;
    }

    const QString& shortName() const {
        return m_shortName;
    }
    void setShortName(const QString& shortName) {
        m_shortName = shortName;
    }

    const QString& description() const {
        return m_description;
    }
    void setDescription(const QString& description) {
        m_description = description;
    }

    int index() const {
        return m_iIndex;
    }
    void setIndex(int index) {
        m_iIndex = index;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Usage hints
    ////////////////////////////////////////////////////////////////////////////////

    const ParameterType& parameterType() const {
        return m_parameterType;
    }

    ValueScaler valueScaler() const {
        return m_valueScaler;
    }
    void setValueScaler(ValueScaler valueScaler) {
        m_valueScaler = valueScaler;
        if (valueScaler == ValueScaler::Toggle) {
            setParameterType(ParameterType::Button);
        } else {
            setParameterType(ParameterType::Knob);
        }
    }

    UnitsHint unitsHint() const {
        return m_unitsHint;
    }
    void setUnitsHint(UnitsHint unitsHint) {
        m_unitsHint = unitsHint;
        m_unitString = EffectManifestParameter::unitsHintStringHash.value(unitsHint);
    }
    const QString unitString() const {
        return m_unitString;
    }

    LinkType defaultLinkType() const {
        return m_defaultLinkType;
    }
    void setDefaultLinkType(const LinkType linkType) {
        m_defaultLinkType = linkType;
    }

    LinkInversion defaultLinkInversion() const {
        return m_defaultLinkInversion;
    }
    void setDefaultLinkInversion(const LinkInversion linkInversion) {
        m_defaultLinkInversion = linkInversion;
    }

    /// Neutral Point On Scale is the parameter in the range 0 .. 1 on the knob that
    /// is adopted as neutral when controlled by the meta knob.
    /// This is allows to link the meta knob in a way that two effects are
    /// cranked in simultaneous, or in case of a split filter like meta knob,
    /// both effects are neutral at meta knob center.
    /// A EQ Gain has usually a neutral point of 0.5 (0 dB) while a delay knob
    /// has a neutral point of 0.0 (no delay)
    /// A EQ Gain knob cannot be used on a split meta knob.
    double neutralPointOnScale() const {
        return m_neutralPointOnScale;
    }
    void setNeutralPointOnScale(double neutralPoint) {
        m_neutralPointOnScale = neutralPoint;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Value Settings
    ////////////////////////////////////////////////////////////////////////////////

    const double& getDefault() const {
        return m_default;
    }

    const double& getMinimum() const {
        return m_minimum;
    }

    const double& getMaximum() const {
        return m_maximum;
    }

    /// Minimum, default, and maximum are set together in one function so their
    /// validity only needs to be checked once.
    void setRange(const double& minimum, const double& defaultValue, const double& maximum) {
        VERIFY_OR_DEBUG_ASSERT(minimum <= defaultValue && defaultValue <= maximum) {
            qWarning() << "EffectManifestParameter" << m_name
                       << "tried to set invalid parameter range:"
                       << "minimum" << minimum
                       << "default" << defaultValue
                       << "maximum" << maximum;
            return;
        }
        m_minimum = minimum;
        m_default = defaultValue;
        m_maximum = maximum;
    }

    void appendStep(const QPair<QString, double>& step) {
        m_steps.append(step);
    }
    const QList<QPair<QString, double>>& getSteps() const {
        return m_steps;
    }

    /// Converts a raw parameter value into a readable string with units (e.g., "0.25" → "250 ms").
    /// Handles formatting for integral/toggle vs. continuous values.
    /// Used by UI components like knobs and labels to show user-friendly tooltips.
    QString valueToString(double value) const {
        // Clamp value between allowed min and max
        double clamped = std::clamp(value, m_minimum, m_maximum);

        QString unit = m_unitString.isEmpty() ? QString() : QStringLiteral(" ") + m_unitString;

        // Format based on scaler type
        if (m_valueScaler == ValueScaler::Toggle || m_valueScaler == ValueScaler::Integral) {
            return QString::number(static_cast<int>(clamped)) + unit;
        } else {
            return QString::number(clamped, 'f', 2) + unit;
        }
    }

  private:
    void setParameterType(const ParameterType parameterType) {
        m_parameterType = parameterType;
    }

    QString debugString() const {
        return QString("EffectManifestParameter(%1)").arg(m_id);
    }

    QString m_id;
    QString m_name;
    QString m_shortName;
    QString m_description;
    int m_iIndex;

    ParameterType m_parameterType;
    ValueScaler m_valueScaler;
    UnitsHint m_unitsHint;
    QString m_unitString;
    LinkType m_defaultLinkType;
    LinkInversion m_defaultLinkInversion;
    double m_neutralPointOnScale;

    double m_default;
    double m_minimum;
    double m_maximum;

    /// Used to describe steps of
    /// CONTROL_INTEGRAL and CONTROL_TOGGLE
    /// effect parameters
    /// Each pair has the following form:
    /// name - value
    QList<QPair<QString, double>> m_steps;
};

QDebug operator<<(QDebug dbg, const EffectManifestParameter& parameter);

typedef EffectManifestParameter::ParameterType EffectParameterType;

inline qhash_seed_t qHash(const EffectParameterType& parameterType) {
    return static_cast<qhash_seed_t>(parameterType);
}

typedef EffectManifestParameter::UnitsHint UnitsHint;

inline qhash_seed_t qHash(const UnitsHint& uhint) {
    return static_cast<qhash_seed_t>(uhint);
}
