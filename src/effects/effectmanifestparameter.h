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
        CONTROL_TOGGLE
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
        LINK_NONE = 0,
        LINK_LINKED,
        LINK_INVERSE,
        NUM_LINK_TYPES
    };

    EffectManifestParameter()
            : m_controlHint(CONTROL_UNKNOWN),
              m_semanticHint(SEMANTIC_UNKNOWN),
              m_unitsHint(UNITS_UNKNOWN),
              m_linkHint(LINK_NONE),
              m_default(0),
              m_minimum(0),
              m_maximum(1.0) {
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

    virtual LinkType linkHint() const {
        return m_linkHint;
    }
    virtual void setLinkHint(LinkType linkHint) {
        m_linkHint = linkHint;
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
    LinkType m_linkHint;

    double m_default;
    double m_minimum;
    double m_maximum;
};

QDebug operator<<(QDebug dbg, const EffectManifestParameter& parameter);

#endif /* EFFECTMANIFESTPARAMETER_H */
