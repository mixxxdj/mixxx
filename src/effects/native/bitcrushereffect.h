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

    EngineEffectParameter* m_pDepthParameter;
    EngineEffectParameter* m_pFrequencyParameter;

    struct ChannelState {
        ChannelState() : phasor(0) {
        }
        CSAMPLE phasor;
    };
    QMap<QString, ChannelState> m_groupState;

    DISALLOW_COPY_AND_ASSIGN(BitCrusherProcessor);
};


#endif /* BITCRUSHEREFFECT_H */
