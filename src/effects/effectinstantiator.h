#ifndef EFFECTINSTANTIATOR_H
#define EFFECTINSTANTIATOR_H

#include <QSharedPointer>

#include "effects/effectmanifest.h"
#include "effects/effectprocessor.h"

class EngineEffect;

class EffectInstantiator {
  public:
    virtual ~EffectInstantiator() {}
    virtual EffectProcessor* instantiate(EngineEffect* pEngineEffect,
                                         const EffectManifest& manifest) = 0;
};
typedef QSharedPointer<EffectInstantiator> EffectInstantiatorPointer;

template <typename T>
class EffectProcessorInstantiator : public EffectInstantiator {
  public:
    EffectProcessor* instantiate(EngineEffect* pEngineEffect,
                                 const EffectManifest& manifest) {
        return new T(pEngineEffect, manifest);
    }
};

#endif /* EFFECTINSTANTIATOR_H */
