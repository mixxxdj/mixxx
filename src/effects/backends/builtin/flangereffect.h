#pragma once

#include <QMap>

#include "effects/backends/effectprocessor.h"
#include "util/class.h"
#include "util/rampingvalue.h"
#include "util/sample.h"
#include "util/types.h"

namespace {
constexpr double kMaxDelayMs = 13.0;
constexpr double kMinDelayMs = 0.22;
constexpr double kCenterDelayMs = (kMaxDelayMs - kMinDelayMs) / 2 + kMinDelayMs;
constexpr double kMaxLfoWidthMs = kMaxDelayMs - kMinDelayMs;
// using + 1.0 instead of ceil() for Mac OS
constexpr SINT kBufferLenth = static_cast<SINT>(kMaxDelayMs + 1.0) * 96; // for 96 kHz
constexpr double kMinLfoBeats = 1 / 4.0;
constexpr double kMaxLfoBeats = 32.0;
} // anonymous namespace

struct FlangerGroupState : public EffectState {
    FlangerGroupState(const mixxx::EngineParameters& engineParameters)
            : EffectState(engineParameters),
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
    ~FlangerGroupState() override = default;

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
    FlangerEffect() = default;
    ~FlangerEffect() override = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            FlangerGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameterPointer m_pSpeedParameter;
    EngineEffectParameterPointer m_pWidthParameter;
    EngineEffectParameterPointer m_pManualParameter;
    EngineEffectParameterPointer m_pRegenParameter;
    EngineEffectParameterPointer m_pMixParameter;
    EngineEffectParameterPointer m_pTripletParameter;

    DISALLOW_COPY_AND_ASSIGN(FlangerEffect);
};
