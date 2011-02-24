#ifndef EFFECT_H
#define EFFECT_H

#include <QSharedPointer>

#include "effects/effectmanifest.h"

class Effect;
typedef QSharedPointer<Effect> EffectPointer;

class Effect : public QObject {
    Q_OBJECT
  public:
    Effect(EffectsBackend* pBackend, EffectManifest& pManifest);
    virtual ~Effect();

    virtual const EffectManifest& getManifest() const {
        return m_pEffectManifest;
    }

    virtual EffectsBackend* getBackend() const {
        return m_pEffectsBackend;
    }

  private:
    EffectsBackend* m_pEffectsBackend;
    EffectManifest& m_pEffectManifest;
};

#endif /* EFFECT_H */
