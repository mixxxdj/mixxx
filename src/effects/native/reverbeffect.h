// Ported from SWH Plate Reverb 1423.
// This effect is GPL code.

#ifndef REVERBEFFECT_H
#define REVERBEFFECT_H

#include <QMap>

#include "defs.h"
#include "util.h"
#include "effects/effectprocessor.h"
#include "effects/native/reverb/Reverb.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "sampleutil.h"

struct ReverbGroupState {
    ReverbGroupState() {
        // Default damping value.
        prev_damping = 0.5;
        reverb.init();
        reverb.activate();
    }

    ~ReverbGroupState() { }

    MixxxPlateX2 reverb;
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
                      const unsigned int numSamples);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pBandWidthParameter;
    EngineEffectParameter* m_pDampingParameter;

    DISALLOW_COPY_AND_ASSIGN(ReverbEffect);
};

#endif /* REVERBEFFECT_H */
