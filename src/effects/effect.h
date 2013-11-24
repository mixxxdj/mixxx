#ifndef EFFECT_H
#define EFFECT_H

#include <QSharedPointer>

#include "defs.h"
#include "util.h"
#include "effects/effectmanifest.h"
#include "effects/effectparameter.h"

class EffectProcessor;
class EngineEffect;

class Effect;
typedef QSharedPointer<Effect> EffectPointer;

// The Effect class is the main-thread representation of an instantiation of an
// effect. This class is NOT thread safe and must only be used by the main
// thread. The getEngineEffect() method can be used to get a pointer to the
// Engine-thread representation of the effect.
class Effect : public QObject {
    Q_OBJECT
  public:
    Effect(QObject* pParent, const EffectManifest& manifest, EffectProcessor* pProcessor);
    virtual ~Effect();

    const EffectManifest& getManifest() const;

    unsigned int numParameters() const;
    EffectParameter* getParameter(unsigned int parameterNumber);
    EffectParameter* getParameterById(const QString& id) const;
    EngineEffect* getEngineEffect();
    void setEngineParameterById(const QString& id, const QVariant& value);

  private:
    QString debugString() const {
        return QString("Effect(%1)").arg(m_manifest.name());
    }

    EffectManifest m_manifest;
    EngineEffect* m_pEngineEffect;
    QList<EffectParameter*> m_parameters;
    QMap<QString, EffectParameter*> m_parametersById;

    DISALLOW_COPY_AND_ASSIGN(Effect);
};

#endif /* EFFECT_H */
