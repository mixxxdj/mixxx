#ifndef EFFECTINSTANTIATOR_H
#define EFFECTINSTANTIATOR_H

#include <QSharedPointer>

#include "effects/effectmanifest.h"
#include "effects/effectprocessor.h"
#include "effects/lv2/lv2effectprocessor.h"
#include <lilv-0/lilv/lilv.h>

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
    LV2EffectProcessorInstantiator(const LilvPlugin* plugin,
                                   QList<int> audioPortIndices,
                                   QList<int> controlPortIndices)
            : m_pPlugin(plugin),
              audioPortIndices(audioPortIndices),
              controlPortIndices(controlPortIndices) { }

    EffectProcessor* instantiate(EngineEffect* pEngineEffect,
                                 const EffectManifest& manifest) {
        return new LV2EffectProcessor(pEngineEffect, manifest, m_pPlugin,
                                      audioPortIndices, controlPortIndices);
    }
  private:
    const LilvPlugin* m_pPlugin;
    QList<int> audioPortIndices;
    QList<int> controlPortIndices;

};

#endif /* EFFECTINSTANTIATOR_H */
