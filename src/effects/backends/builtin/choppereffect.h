#pragma once

#include <QMap>

#include "effects/backends/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/engine.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/samplebuffer.h"

class ChopperState : public EffectState {
  public:
    // 3 seconds max. This supports the full range of 2 beats for tempos down to
    // 40 BPM.
    static constexpr int kMaxDelaySeconds = 3;

    ChopperState(const mixxx::EngineParameters engineParameters)
            : EffectState(engineParameters) {
        audioParametersChanged(engineParameters);
        clear();
        mixPrev = 0;
        startPos = 0;
        selectedLoop = loop.data();
    }

    void audioParametersChanged(const mixxx::EngineParameters engineParameters) {
        loop = mixxx::SampleBuffer(kMaxDelaySeconds *
                engineParameters.sampleRate() *
                engineParameters.channelCount());
        reverseLoop = mixxx::SampleBuffer(kMaxDelaySeconds *
                engineParameters.sampleRate() *
                engineParameters.channelCount());
    };

    void clear() {
        writeSamplePos = 0;
        readSamplePos = 0;
        isRecording = true;
    };

    mixxx::SampleBuffer loop;
    mixxx::SampleBuffer reverseLoop;
    CSAMPLE* selectedLoop;
    int readSamplePos;
    int writeSamplePos;
    int startPos;
    int endPos;
    bool isRecording;
    double mixPrev;
};

class ChopperEffect : public EffectProcessorImpl<ChopperState> {
  public:
    ChopperEffect(){};

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            ChopperState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

  private:
    EngineEffectParameterPointer m_pLengthParameter;
    EngineEffectParameterPointer m_pMixParameter;
    EngineEffectParameterPointer m_pTripletParameter;
    EngineEffectParameterPointer m_pQuantizeParameter;
    EngineEffectParameterPointer m_pReverseParameter;
    DISALLOW_COPY_AND_ASSIGN(ChopperEffect);
};
