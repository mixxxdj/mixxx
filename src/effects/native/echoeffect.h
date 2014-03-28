#ifndef ECHOEFFECT_H
#define ECHOEFFECT_H

#include <QMap>

#include "defs.h"
#include "util.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/enginefilterbutterworth8.h"
#include "effects/effectprocessor.h"
#include "sampleutil.h"

struct EchoGroupState {
    EchoGroupState() {
        delay_buf = SampleUtil::alloc(MAX_BUFFER_LEN);
        // TODO(owilliams): use the actual samplerate.
        feedback_lowpass =
                new EngineFilterButterworth8Low(44100, 10000);
        SampleUtil::applyGain(delay_buf, 0, MAX_BUFFER_LEN);
        prev_delay_time = 0.0;
        prev_delay_samples = 0;
        write_position = 0;
        ping_pong_left = true;
    }
    ~EchoGroupState() {
        SampleUtil::free(delay_buf);
        delete feedback_lowpass;
    }
    CSAMPLE* delay_buf;
    EngineFilterButterworth8Low* feedback_lowpass;
    double prev_delay_time;
    int prev_delay_samples;
    int write_position;
    bool ping_pong_left;
};

class EchoEffect : public GroupEffectProcessor<EchoGroupState> {
  public:
    EchoEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~EchoEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processGroup(const QString& group,
                      EchoGroupState* pState,
                      const CSAMPLE* pInput, CSAMPLE* pOutput,
                      const unsigned int numSamples);

  private:
    int getDelaySamples(double delay_time) const;

    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pDelayParameter;
    EngineEffectParameter* m_pFeedbackParameter;
    EngineEffectParameter* m_pPingPongParameter;

    DISALLOW_COPY_AND_ASSIGN(EchoEffect);
};

#endif /* ECHOEFFECT_H */
