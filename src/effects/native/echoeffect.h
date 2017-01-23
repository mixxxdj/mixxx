#ifndef ECHOEFFECT_H
#define ECHOEFFECT_H

#include <QMap>

#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/audiosignal.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/samplebuffer.h"
#include "util/types.h"

struct EchoGroupState {
    // 2 seconds max.
    static constexpr int kMaxDelaySeconds = 2;
    // TODO(XXX): When we move from stereo to multi-channel this needs updating.
    static constexpr int kChannelCount = mixxx::AudioSignal::kChannelCountStereo;
    // Ramp length in samples when we are at the start of an echo.
    // TODO(XXX): make this samplerate independent
    static constexpr int kRampLength = 500;

    EchoGroupState()
            : delay_buf(mixxx::AudioSignal::kSamplingRateMax * kMaxDelaySeconds *
                        kChannelCount) {
        delay_buf.clear();
        prev_delay_time = 0.0;
        prev_delay_samples = 0;
        write_position = 0;
        ping_pong_left = true;
    }

    SampleBuffer delay_buf;
    double prev_delay_time;
    int prev_delay_samples;
    int write_position;
    bool ping_pong_left;
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

    DISALLOW_COPY_AND_ASSIGN(EchoEffect);
};

#endif /* ECHOEFFECT_H */
