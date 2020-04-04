#pragma once

#include <QVariant>
#include <QString>
#include <QtDebug>
#include <QSharedPointer>

#include "util/assert.h"
#include "effects/defs.h"

class EffectManifestParameter;
typedef QSharedPointer<EffectManifestParameter> EffectManifestParameterPointer;

class EffectManifestParameter {
  public:
    enum class ParameterType {
        KNOB,
        BUTTON,

        NUM_TYPES
    };

    enum class ValueScaler {
        UNKNOWN = 0,
        LINEAR,
        LINEAR_INVERSE,
        LOGARITHMIC,
        LOGARITHMIC_INVERSE,
        INTEGRAL,   // A step rotary, steps given by m_steps
                    // are arranged with equal distance on scale
        TOGGLE      // For button and enum controls, not accessible
                    // from many controllers, no linking to meta knob
    };

    enum class SemanticHint {
        UNKNOWN = 0,
        SAMPLES,
        NOTE,
    };

    enum class UnitsHint {
        UNKNOWN = 0,
        TIME,
        HERTZ,
        SAMPLERATE, // fraction of the samplerate
        BEATS, // multiples of a beat
    };

    enum class LinkType {
        NONE = 0,  // Not controlled by the meta knob
        LINKED,  // Controlled by the meta knob as it is
        LINKED_LEFT,  // Controlled by the left side of the meta knob
        LINKED_RIGHT, // Controlled by the right side of the meta knob
        LINKED_LEFT_RIGHT, // Controlled by both sides of the meta knob
        NUM_LINK_TYPES
    };

    static QString LinkTypeToString (LinkType type) {
        if (type == LinkType::LINKED) {
            return "LINKED";
        } else if (type == LinkType::LINKED_LEFT) {
            return "LINKED_LEFT";
        } else if (type == LinkType::LINKED_RIGHT) {
            return "LINKED_RIGHT";
        } else if (type == LinkType::LINKED_LEFT_RIGHT) {
            return "LINKED_LEFT_RIGHT";
        } else {
            return "NONE";
        }
    }

    static LinkType LinkTypeFromString (const QString& string) {
        if (string == "LINKED") {
            return LinkType::LINKED;
        } else if (string == "LINKED_LEFT") {
            return LinkType::LINKED_LEFT;
        } else if (string == "LINKED_RIGHT") {
            return LinkType::LINKED_RIGHT;
        } else if (string == "LINKED_LEFT_RIGHT") {
            return LinkType::LINKED_LEFT_RIGHT;
        } else {
            return LinkType::NONE;
        }
    }

    enum class LinkInversion {
        NOT_INVERTED = 0,
        INVERTED = 1
    };

    EffectManifestParameter()
            : m_valueScaler(ValueScaler::UNKNOWN),
              m_semanticHint(SemanticHint::UNKNOWN),
              m_unitsHint(UnitsHint::UNKNOWN),
              m_defaultLinkType(LinkType::NONE),
              m_defaultLinkInversion(LinkInversion::NOT_INVERTED),
              m_neutralPointOnScale(0.0),
              m_default(0),
              m_minimum(0),
              m_maximum(1.0),
              m_showInParametertSlot(true) {
    }

    ~EffectManifestParameter() {
        //qDebug() << debugString() << "destroyed";
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

    ////////////////////////////////////////////////////////////////////////////////
    // Usage hints
    ////////////////////////////////////////////////////////////////////////////////

    const ParameterType& parameterType() const {
        return m_parameterType;
    }

    void setParameterType(const ParameterType parameterType) {
        m_parameterType = parameterType;
    }

    ValueScaler valueScaler() const {
        return m_valueScaler;
    }
    void setValueScaler(ValueScaler valueScaler) {
        m_valueScaler = valueScaler;
        if (valueScaler == ValueScaler::TOGGLE) {
            setParameterType(ParameterType::BUTTON);
        } else {
            setParameterType(ParameterType::KNOB);
        }
    }

    SemanticHint semanticHint() const {
        return m_semanticHint;
    }
    void setSemanticHint(SemanticHint semanticHint) {
        m_semanticHint = semanticHint;
    }

    UnitsHint unitsHint() const {
        return m_unitsHint;
    }
    void setUnitsHint(UnitsHint unitsHint) {
        m_unitsHint = unitsHint;
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


    // Neutral Point On Scale is the parameter in the range 0 .. 1 on the knob that
    // is adopted as neutral when controlled by the meta knob.
    // This is allows to link the meta knob in a way that two effects are
    // cranked in simultaneous, or in case of a split filter like meta knob,
    // both effects are neutral at meta knob center.
    // A EQ Gain has usually a neutral point of 0.5 (0 dB) while a delay knob
    // has a neutral point of 0.0 (no delay)
    // A EQ Gain knob cannot be used on a split meta knob.
    double neutralPointOnScale() const {
        return m_neutralPointOnScale;
    }
    void setNeutralPointOnScale(double neutralPoint) {
        m_neutralPointOnScale = neutralPoint;
    }


    // These store the mapping between the parameter slot and
    // the effective parameter which is loaded onto the slot.
    // This is required because we have only 8 parameter slots, but
    // LV2 or VST effects can have more then 8.
    bool showInParameterSlot() const {
        return m_showInParametertSlot;
    }
    void setShowInParameterSlot(double show) {
        m_showInParametertSlot = show;
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

    // Minimum, default, and maximum are set together in one function so their
    // validity only needs to be checked once.
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
    const QList<QPair<QString, double> >& getSteps() const {
        return m_steps;
    }

  private:
    QString debugString() const {
        return QString("EffectManifestParameter(%1)").arg(m_id);
    }

    QString m_id;
    QString m_name;
    QString m_shortName;
    QString m_description;

    ParameterType m_parameterType;
    ValueScaler m_valueScaler;
    SemanticHint m_semanticHint;
    UnitsHint m_unitsHint;
    LinkType m_defaultLinkType;
    LinkInversion m_defaultLinkInversion;
    double m_neutralPointOnScale;

    double m_default;
    double m_minimum;
    double m_maximum;

    // Used to describe steps of
    // CONTROL_INTEGRAL and CONTROL_TOGGLE
    // effect parameters
    // Each pair has the following form:
    // name - value
    QList<QPair<QString, double> > m_steps;

    bool m_showInParametertSlot;
};

inline uint qHash(const EffectManifestParameter::ParameterType& parameterType) {
    return static_cast<uint>(parameterType);
}

QDebug operator<<(QDebug dbg, const EffectManifestParameter& parameter);
