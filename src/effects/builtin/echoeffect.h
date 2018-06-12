#ifndef ECHOEFFECT_H
#define ECHOEFFECT_H

#include <QMap>

#include "effects/effectprocessor.h"
#include "engine/engine.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/rampingvalue.h"
#include "util/sample.h"
#include "util/samplebuffer.h"

class EchoGroupState : public EffectState {
  public:
    // 3 seconds max. This supports the full range of 2 beats for tempos down to
    // 40 BPM.
    static constexpr int kMaxDelaySeconds = 3;

    EchoGroupState(const mixxx::EngineParameters bufferParameters)
           : EffectState(bufferParameters),
             send(bufferParameters.framesPerBuffer()),
             feedback(bufferParameters.framesPerBuffer()) {
       audioParametersChanged(bufferParameters);
       clear();
    }

    void audioParametersChanged(const mixxx::EngineParameters bufferParameters) {
        delay_buf = mixxx::SampleBuffer(kMaxDelaySeconds
                * bufferParameters.sampleRate() * bufferParameters.channelCount());
        send = RampingValue<CSAMPLE_GAIN>(bufferParameters.framesPerBuffer());
        feedback = RampingValue<CSAMPLE_GAIN>(bufferParameters.framesPerBuffer());
    };

    void clear() {
        delay_buf.clear();
        send.setCurrentCallbackValue(0);
        feedback.setCurrentCallbackValue(0);
        prev_delay_samples = 0;
        write_position = 0;
        ping_pong = 0;
    };

    mixxx::SampleBuffer delay_buf;
    RampingValue<CSAMPLE_GAIN> send;
    RampingValue<CSAMPLE_GAIN> feedback;
    int prev_delay_samples;
    int write_position;
    int ping_pong;
};

class EchoEffect : public EffectProcessorImpl<EchoGroupState> {
  public:
    EchoEffect(EngineEffect* pEffect);

    static QString getId();
    static EffectManifestPointer getManifest();

    void processChannel(const ChannelHandle& handle,
                        EchoGroupState* pState,
                        const CSAMPLE* pInput, CSAMPLE* pOutput,
                        const mixxx::EngineParameters& bufferParameters,
                        const EffectEnableState enableState,
                        const GroupFeatureState& groupFeatures);

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
