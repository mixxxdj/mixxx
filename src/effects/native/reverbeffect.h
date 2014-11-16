// Ported from CAPS Reverb.
// This effect is GPL code.

#ifndef REVERBEFFECT_H
#define REVERBEFFECT_H

#include <QMap>

#include "util.h"
#include "util/types.h"
#include "util/defs.h"
#include "effects/effectprocessor.h"
#include "effects/native/reverb/Reverb.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "sampleutil.h"

struct ReverbGroupState {
    ReverbGroupState() {
        // Default damping value.
        prev_bandwidth = 0.5;
        prev_damping = 0.5;
        reverb.init();
        reverb.activate();
        crossfade_buffer = SampleUtil::alloc(MAX_BUFFER_LEN);
    }

    ~ReverbGroupState() {
        SampleUtil::free(crossfade_buffer);
    }

    MixxxPlateX2 reverb;
    CSAMPLE* crossfade_buffer;
    double prev_bandwidth;
    double prev_damping;
};

class ReverbEffect : public GroupEffectProcessor<ReverbGroupState> {
  public:
    ReverbEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~ReverbEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processGroup(const QString& group,
                      ReverbGroupState* pState,
                      const CSAMPLE* pInput, CSAMPLE* pOutput,
                      const unsigned int numSamples,
                      const unsigned int sampleRate,
                      const EffectProcessor::EnableState enableState,
                      const GroupFeatureState& groupFeatures);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pBandWidthParameter;
    EngineEffectParameter* m_pDampingParameter;

    DISALLOW_COPY_AND_ASSIGN(ReverbEffect);
};

#endif /* REVERBEFFECT_H */
