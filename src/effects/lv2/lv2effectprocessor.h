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
    ~LV2EffectProcessor();

    void initialize(const QSet<ChannelHandleAndGroup>& registeredChannels);
    virtual void process(const ChannelHandle& handle,
                         const CSAMPLE* pInput, CSAMPLE* pOutput,
                         const unsigned int numSamples,
                         const unsigned int sampleRate,
						 const enum EnableState enableState,
                         const GroupFeatureState& groupFeatures);
  private:
    QList<EngineEffectParameter*> m_parameters;
    float* m_inputL;
    float* m_inputR;
    float* m_outputL;
    float* m_outputR;
    float* m_params;
    LilvInstance* m_handle;

    int m_sampleRate;
};


#endif // LV2EFFECTPROCESSOR_H
