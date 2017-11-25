#ifndef ECHOEFFECT_H
#define ECHOEFFECT_H

#include <QMap>

#include "effects/effectprocessor.h"
#include "engine/engine.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/samplebuffer.h"

struct EchoGroupState {
    // 3 seconds max. This supports the full range of 2 beats for tempos down to
    // 40 BPM.
    static constexpr int kMaxDelaySeconds = 3;
    static constexpr auto kChannelCount = mixxx::kEngineChannelCount;

    EchoGroupState()
            : delay_buf(mixxx::AudioSignal::SampleRate::max() * kMaxDelaySeconds *
                        kChannelCount) {
        delay_buf.clear();
        prev_send = 0.0f;
        prev_feedback= 0.0f;
        prev_delay_samples = 0;
        write_position = 0;
        ping_pong = 0;
    }

    mixxx::SampleBuffer delay_buf;
    CSAMPLE_GAIN prev_send;
    CSAMPLE_GAIN prev_feedback;
    int prev_delay_samples;
    int write_position;
    int ping_pong;
};

class EchoEffect : public PerChannelEffectProcessor<EchoGroupState> {
  public:
    EchoEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    ~EchoEffect() override;

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processChannel(const ChannelHandle& handle,
                        EchoGroupState* pState,
                        const CSAMPLE* pInput, CSAMPLE* pOutput,
                        const unsigned int numSamples,
                        const unsigned int sampleRate,
                        const EffectProcessor::EnableState enableState,
                        const GroupFeatureState& groupFeatures) override;

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pDelayParameter;
    EngineEffectParameter* m_pSendParameter;
    EngineEffectParameter* m_pFeedbackParameter;
    EngineEffectParameter* m_pPingPongParameter;
    EngineEffectParameter* m_pQuantizeParameter;
    EngineEffectParameter* m_pTripletParameter;

    DISALLOW_COPY_AND_ASSIGN(EchoEffect);
};

#endif /* ECHOEFFECT_H */
