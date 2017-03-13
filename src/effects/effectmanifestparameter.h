#ifndef EFFECTMANIFESTPARAMETER_H
#define EFFECTMANIFESTPARAMETER_H

#include <QVariant>
#include <QString>
#include <QtDebug>

class EffectManifestParameter {
  public:
    enum ControlHint {
        CONTROL_UNKNOWN = 0,
        CONTROL_KNOB_LINEAR,
        CONTROL_KNOB_LOGARITHMIC,
        CONTROL_KNOB_STEPPING,   // A step rotary, steps given by m_steps
                                 // are arranged with equal distance on scale
        CONTROL_TOGGLE_STEPPING  // For button and enum controls, not accessible
                                 // form many controllers, no linking to super knob
    };

    enum SemanticHint {
        SEMANTIC_UNKNOWN = 0,
        SEMANTIC_SAMPLES,
        SEMANTIC_NOTE,
    };

    enum UnitsHint {
        UNITS_UNKNOWN = 0,
        UNITS_TIME,
        UNITS_HERTZ,
        UNITS_SAMPLERATE, // fraction of the samplerate
        UNITS_BEATS, // multiples of a beat
    };

    enum LinkType {
        LINK_NONE = 0,  // Not controlled by the super knob
        LINK_LINKED,  // Controlled by the super knob as it is
        LINK_LINKED_LEFT,  // Controlled by the left side of the super knob
        LINK_LINKED_RIGHT, // Controlled by the right side of the super knob
        LINK_LINKED_LEFT_RIGHT, // Controlled by both sides of the super knob
        NUM_LINK_TYPES
    };

    EffectManifestParameter()
            : m_controlHint(CONTROL_UNKNOWN),
              m_semanticHint(SEMANTIC_UNKNOWN),
              m_unitsHint(UNITS_UNKNOWN),
              m_defaultLinkType(LINK_NONE),
              m_neutralPointOnScale(0.0),
              m_default(0),
              m_minimum(0),
              m_maximum(1.0),
              m_showInParametertSlot(true) {
    }

    virtual ~EffectManifestParameter() {
        //qDebug() << debugString() << "destroyed";
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Parameter Information
    ////////////////////////////////////////////////////////////////////////////////

    virtual const QString& id() const {
        return m_id;
    }
    virtual void setId(const QString& id) {
        m_id = id;
    }

    virtual const QString& name() const {
        return m_name;
    }
    virtual void setName(const QString& name) {
        m_name = name;
    }

    virtual const QString& description() const {
        return m_description;
    }
    virtual void setDescription(const QString& description) {
        m_description = description;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Usage hints
    ////////////////////////////////////////////////////////////////////////////////

    virtual ControlHint controlHint() const {
        return m_controlHint;
    }
    virtual void setControlHint(ControlHint controlHint) {
        m_controlHint = controlHint;
    }

    virtual SemanticHint semanticHint() const {
        return m_semanticHint;
    }
    virtual void setSemanticHint(SemanticHint semanticHint) {
        m_semanticHint = semanticHint;
    }

    virtual UnitsHint unitsHint() const {
        return m_unitsHint;
    }
    virtual void setUnitsHint(UnitsHint unitsHint) {
        m_unitsHint = unitsHint;
    }

    virtual LinkType defaultLinkType() const {
        return m_defaultLinkType;
    }
    virtual void setDefaultLinkType(LinkType linkType) {
        m_defaultLinkType = linkType;
    }

    // Neutral Point On Scale is the parameter in the range 0 .. 1 on the knob that
    // is adopted as neutral when controlled by the super knob.
    // This is allows to link the super knob in a way that two effects are
    // cranked in simultaneous, or in case of a split filter like super knob,
    // both effects are neutral at super knob center.
    // A EQ Gain has usually a neutral point of 0.5 (0 dB) while a delay knob
    // has a neutral point of 0.0 (no delay)
    // A EQ Gain knob cannot be used on a split super knob.
    virtual double neutralPointOnScale() const {
        return m_neutralPointOnScale;
    }
    virtual void setNeutralPointOnScale(double neutralPoint) {
        m_neutralPointOnScale = neutralPoint;
    }


    // These store the mapping between the parameter slot and
    // the effective parameter which is loaded onto the slot.
    // This is required because we have only 8 parameter slots, but
    // LV2 or VST effects can have more then 8.
    virtual bool showInParameterSlot() const {
        return m_showInParametertSlot;
    }
    virtual void setShowInParameterSlot(double show) {
        m_showInParametertSlot = show;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Value Settings
    ////////////////////////////////////////////////////////////////////////////////

    virtual const double& getDefault() const {
        return m_default;
    }
    virtual void setDefault(const double& defaultValue) {
        m_default = defaultValue;
    }

    virtual const double& getMinimum() const {
        return m_minimum;
    }
    virtual void setMinimum(const double& minimum) {
        m_minimum = minimum;
    }

    virtual const double& getMaximum() const {
        return m_maximum;
    }
    virtual void setMaximum(const double& maximum) {
        m_maximum = maximum;
    }

    virtual void appendStep(const QPair<QString, double>& step) {
        m_steps.append(step);
    }
    virtual const QList<QPair<QString, double> >& getSteps() const {
        return m_steps;
    }

  private:
    QString debugString() const {
        return QString("EffectManifestParameter(%1)").arg(m_id);
    }

    QString m_id;
    QString m_name;
    QString m_description;

    ControlHint m_controlHint;
    SemanticHint m_semanticHint;
    UnitsHint m_unitsHint;
    LinkType m_defaultLinkType;
    double m_neutralPointOnScale;

    double m_default;
    double m_minimum;
    double m_maximum;

    // Used to describe steps of
    // CONTROL_KNOB_STEPPING and CONTROL_TOGGLE_STEPPING
    // effect parameters
    // Each pair has the following form:
    // name - value
    QList<QPair<QString, double> > m_steps;

    bool m_showInParametertSlot;
};

QDebug operator<<(QDebug dbg, const EffectManifestParameter& parameter);

#endif /* EFFECTMANIFESTPARAMETER_H */
