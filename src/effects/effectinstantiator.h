#ifndef EFFECTINSTANTIATOR_H
#define EFFECTINSTANTIATOR_H

#include <QSharedPointer>

#include "effects/effectmanifest.h"
#include "effects/effectprocessor.h"
#include "effects/lv2/lv2effectprocessor.h"

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

class LV2EffectProcessorInstantiator : public EffectInstantiator {
  public:
    EffectProcessor* instantiate(EngineEffect* pEngineEffect,
                                 const EffectManifest& manifest) {
        Q_UNUSED(pEngineEffect);
        Q_UNUSED(manifest);
        return new LV2EffectProcessor();
    }
};

#endif /* EFFECTINSTANTIATOR_H */
