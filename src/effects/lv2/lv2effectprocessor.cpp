#include "effects/lv2/lv2effectprocessor.h"

void LV2EffectProcessor::initialize(const QSet<QString>& registeredGroups) {
    Q_UNUSED(registeredGroups);

}

void LV2EffectProcessor::process(const QString& group,
                         const CSAMPLE* pInput, CSAMPLE* pOutput,
                         const unsigned int numSamples,
                         const GroupFeatureState& groupFeatures) {
    Q_UNUSED(groupFeatures);
    for (unsigned int i = 0; i < numSamples; i++) {
        pOutput[i] = 0;
    }
}
