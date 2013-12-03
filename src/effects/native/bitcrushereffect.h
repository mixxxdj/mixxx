#ifndef BITCRUSHEREFFECT_H
#define BITCRUSHEREFFECT_H

#include <QMap>

#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "effects/native/nativebackend.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "util.h"

class BitCrusherProcessor : public EffectProcessor {
  public:
    BitCrusherProcessor(const EffectManifest& manifest);
    virtual ~BitCrusherProcessor();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void initialize(EngineEffect* pEffect);

    // See effectprocessor.h
    void process(const QString& group,
                 const CSAMPLE* pInput, CSAMPLE *pOutput,
                 const unsigned int numSamples);

  private:
    QString debugString() const {
        return "BitCrusherProcessor";
    }

    EngineEffectParameter* m_pBitDepthParameter;
    EngineEffectParameter* m_pDownsampleParameter;

    struct ChannelState {
        // Default accumulator to 1 so we immediately pick an input value.
        ChannelState()
                : hold_l(0),
                  hold_r(0),
                  accumulator(1) {
        }
        CSAMPLE hold_l, hold_r;
        // Accumulated fractions of a samplerate period.
        CSAMPLE accumulator;
    };
    QMap<QString, ChannelState> m_groupState;

    DISALLOW_COPY_AND_ASSIGN(BitCrusherProcessor);
};


#endif /* BITCRUSHEREFFECT_H */
