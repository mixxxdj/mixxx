#ifndef FLANGEREFFECT_H
#define FLANGEREFFECT_H

#include <QMap>

#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/types.h"
#include "util/rampingvalue.h"

namespace {
constexpr double kMaxDelayMs = 13.0;
constexpr double kMinDelayMs = 0.22;
constexpr double kCenterDelayMs = (kMaxDelayMs - kMinDelayMs) / 2 + kMinDelayMs;
constexpr double kMaxLfoWidthMs = kMaxDelayMs - kMinDelayMs;
// using + 1.0 instead of ceil() for Mac OS
constexpr SINT kBufferLenth = static_cast<SINT>(kMaxDelayMs + 1.0)  * 96; // for 96 kHz
constexpr double kMinLfoBeats = 1/4.0;
constexpr double kMaxLfoBeats = 32.0;
} // anonymous namespace

struct FlangerGroupState : public EffectState {
    FlangerGroupState(const mixxx::EngineParameters& bufferParameters)
            : EffectState(bufferParameters),
              delayPos(0),
              lfoFrames(0),
              previousPeriodFrames(-1),
              prev_regen(0),
              prev_mix(0),
              prev_width(0),
              prev_manual(static_cast<CSAMPLE_GAIN>(kCenterDelayMs)) {
        SampleUtil::clear(delayLeft, kBufferLenth);
        SampleUtil::clear(delayRight, kBufferLenth);
    }
    CSAMPLE delayLeft[kBufferLenth];
    CSAMPLE delayRight[kBufferLenth];
    unsigned int delayPos;
    unsigned int lfoFrames;
    double previousPeriodFrames;
    CSAMPLE_GAIN prev_regen;
    CSAMPLE_GAIN prev_mix;
    CSAMPLE_GAIN prev_width;
    CSAMPLE_GAIN prev_manual;
};

class FlangerEffect : public EffectProcessorImpl<FlangerGroupState> {
  public:
    FlangerEffect(EngineEffect* pEffect);
    virtual ~FlangerEffect();

    static QString getId();
    static EffectManifestPointer getManifest();

    // See effectprocessor.h
    void processChannel(const ChannelHandle& handle,
                        FlangerGroupState* pState,
                        const CSAMPLE* pInput, CSAMPLE* pOutput,
                        const mixxx::EngineParameters& bufferParameters,
                        const EffectEnableState enableState,
                        const GroupFeatureState& groupFeatures);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pSpeedParameter;
    EngineEffectParameter* m_pWidthParameter;
    EngineEffectParameter* m_pManualParameter;
    EngineEffectParameter* m_pRegenParameter;
    EngineEffectParameter* m_pMixParameter;
    EngineEffectParameter* m_pTripletParameter;

    DISALLOW_COPY_AND_ASSIGN(FlangerEffect);
};

#endif /* FLANGEREFFECT_H */
