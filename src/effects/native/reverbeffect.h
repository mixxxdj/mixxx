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

class ReverbEffect : public EffectProcessor {
  private:
    #define LP_INNER 0.96f
    #define LP_OUTER 0.983f
    #define RUN_WG(n, junct_a, junct_b) waveguide_nl_process_lin(m_pWaveguide[n], junct_a - m_pOut[n*2+1], junct_b - m_pOut[n*2], m_pOut+n*2, m_pOut+n*2+1)

  public:
    ReverbEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~ReverbEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void process(const QString& group,
                 const CSAMPLE* pInput, CSAMPLE* pOutput,
                 const unsigned int numSamples);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pTimeParameter;
    EngineEffectParameter* m_pDampingParameter;
    waveguide_nl** m_pWaveguide;
    CSAMPLE* m_pOut;

    struct GroupState {
        GroupState()
                : delayPos(0),
                  time(0) {
            SampleUtil::applyGain(delayBuffer, 0, MAX_BUFFER_LEN);
        }
        CSAMPLE delayBuffer[MAX_BUFFER_LEN];
        unsigned int delayPos;
        unsigned int time;
    };
    QMap<QString, GroupState> m_groupState;

    DISALLOW_COPY_AND_ASSIGN(ReverbEffect);
};

#endif /* REVERBEFFECT_H */
