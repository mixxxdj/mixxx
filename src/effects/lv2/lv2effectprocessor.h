#ifndef LV2EFFECTPROCESSOR_H
#define LV2EFFECTPROCESSOR_H

#include "effects/effectprocessor.h"
#include "effects/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"

class LV2EffectProcessor : public EffectProcessor {
  public:
    LV2EffectProcessor(EngineEffect* pEngineEffect,
                       const EffectManifest& manifest);

    void initialize(const QSet<QString>& registeredGroups);
    virtual void process(const QString& group,
                         const CSAMPLE* pInput, CSAMPLE* pOutput,
                         const unsigned int numSamples,
			 const unsigned int sampleRate,
                         const GroupFeatureState& groupFeatures);
  private:
    QList<EngineEffectParameter*> m_parameters;
};


#endif // LV2EFFECTPROCESSOR_H
