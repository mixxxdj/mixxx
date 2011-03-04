#ifndef EFFECT_H
#define EFFECT_H

#include <QSharedPointer>

#include "effects/effectmanifest.h"
#include "effects/effectparameter.h"

class EffectsBackend;

class Effect;
typedef QSharedPointer<Effect> EffectPointer;

class Effect : public QObject {
    Q_OBJECT
  public:
    Effect(EffectsBackend* pBackend, EffectManifest& pManifest);
    virtual ~Effect();

    virtual const EffectManifest& getManifest() const;

    unsigned int numParameters() const;
    EffectParameterPointer getParameter(unsigned int parameterNumber);

  private:
    EffectsBackend* m_pEffectsBackend;
    EffectManifest& m_effectManifest;
    QList<EffectParameterPointer> m_parameters;
};

#endif /* EFFECT_H */
