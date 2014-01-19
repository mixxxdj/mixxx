// Ported from SWH Plate Reverb 1423.
// This effect is GPL code.

#ifndef REVERBEFFECT_H
#define REVERBEFFECT_H

#include <QMap>

#include "defs.h"
#include "util.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "effects/effectprocessor.h"
#include "effects/native/waveguide_nl.h"
#include "sampleutil.h"

#define LP_INNER 0.96f
#define LP_OUTER 0.983f
const unsigned int kOutBufSize = 32;

struct ReverbGroupState {
    ReverbGroupState() {
        waveguide = (waveguide_nl**)malloc(8 * sizeof(waveguide_nl *));
        waveguide[0] = waveguide_nl_new(2389, LP_INNER, 0.04f, 0.0f);
        waveguide[1] = waveguide_nl_new(4742, LP_INNER, 0.17f, 0.0f);
        waveguide[2] = waveguide_nl_new(4623, LP_INNER, 0.52f, 0.0f);
        waveguide[3] = waveguide_nl_new(2142, LP_INNER, 0.48f, 0.0f);
        waveguide[4] = waveguide_nl_new(5597, LP_OUTER, 0.32f, 0.0f);
        waveguide[5] = waveguide_nl_new(3692, LP_OUTER, 0.89f, 0.0f);
        waveguide[6] = waveguide_nl_new(5611, LP_OUTER, 0.28f, 0.0f);
        waveguide[7] = waveguide_nl_new(3703, LP_OUTER, 0.29f, 0.0f);

        out = SampleUtil::alloc(kOutBufSize);
    }
    ~ReverbGroupState() {
        for (int i = 0; i < 8; i++) {
            waveguide_nl_reset(waveguide[i]);
        }
        SampleUtil::free(out);
    }
    waveguide_nl** waveguide;
    CSAMPLE* out;
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
                      const unsigned int numSamples);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pTimeParameter;
    EngineEffectParameter* m_pDampingParameter;

    DISALLOW_COPY_AND_ASSIGN(ReverbEffect);
};

#endif /* REVERBEFFECT_H */
