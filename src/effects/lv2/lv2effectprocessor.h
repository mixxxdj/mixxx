#ifndef LV2EFFECTPROCESSOR_H
#define LV2EFFECTPROCESSOR_H

#include "effects/effectprocessor.h"

class LV2EffectProcessor : public EffectProcessor {
    void initialize(const QSet<QString>& registeredGroups);
    virtual void process(const QString& group,
                         const CSAMPLE* pInput, CSAMPLE* pOutput,
                         const unsigned int numSamples,
                         const GroupFeatureState& groupFeatures);
};


#endif // LV2EFFECTPROCESSOR_H
