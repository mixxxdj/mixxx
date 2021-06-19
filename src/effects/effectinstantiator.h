#pragma once

#include <QSharedPointer>
#include "effects/effectmanifest.h"
#include "effects/effectprocessor.h"

#ifdef __LILV__
#include "effects/lv2/lv2effectprocessor.h"
#endif /* __LILV__ */

class EffectInstantiator {
  public:
    virtual ~EffectInstantiator() {}
    virtual EffectProcessor* instantiate(EngineEffect* pEngineEffect,
                                         EffectManifestPointer pManifest) = 0;
};
typedef QSharedPointer<EffectInstantiator> EffectInstantiatorPointer;

template <typename T>
class EffectProcessorInstantiator : public EffectInstantiator {
  public:
    EffectProcessor* instantiate(EngineEffect* pEngineEffect,
                                 EffectManifestPointer pManifest) {
        Q_UNUSED(pManifest);
        return new T(pEngineEffect);
    }
};

#ifdef __LILV__
class LV2EffectProcessorInstantiator : public EffectInstantiator {
  public:
    LV2EffectProcessorInstantiator(const LilvPlugin* plugin,
            const QList<int>& audioPortIndices,
            const QList<int>& controlPortIndices)
            : m_pPlugin(plugin),
              m_audioPortIndices(audioPortIndices),
              m_controlPortIndices(controlPortIndices) {
    }

    EffectProcessor* instantiate(EngineEffect* pEngineEffect,
                                 EffectManifestPointer pManifest) {
        return new LV2EffectProcessor(pEngineEffect, pManifest, m_pPlugin,
                                      m_audioPortIndices, m_controlPortIndices);
    }
  private:
    const LilvPlugin* m_pPlugin;
    const QList<int> m_audioPortIndices;
    const QList<int> m_controlPortIndices;

};
#endif /* __LILV__ */
