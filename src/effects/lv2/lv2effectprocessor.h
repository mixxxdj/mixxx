#ifndef LV2EFFECTPROCESSOR_H
#define LV2EFFECTPROCESSOR_H

#include "effects/effectprocessor.h"
#include "effects/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"
#include <lilv-0/lilv/lilv.h>

class LV2EffectProcessor : public EffectProcessor {
  public:
    LV2EffectProcessor(EngineEffect* pEngineEffect,
                       const EffectManifest& manifest,
                       const LilvPlugin* plugin,
                       QList<int> audioPortIndices,
                       QList<int> controlPortIndices);

    void initialize(const QSet<QString>& registeredGroups);
    virtual void process(const QString& group,
                         const CSAMPLE* pInput, CSAMPLE* pOutput,
                         const unsigned int numSamples,
			 const unsigned int sampleRate,
                         const GroupFeatureState& groupFeatures);
  private:
    QList<EngineEffectParameter*> m_parameters;
    float* inputL;
    float* inputR;
    float* outputL;
    float* outputR;
    float* params;
    LilvInstance* handle;

    int m_sampleRate;
};


#endif // LV2EFFECTPROCESSOR_H
