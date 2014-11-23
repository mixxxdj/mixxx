#ifndef ECHOEFFECT_H
#define ECHOEFFECT_H

#include <QMap>

#include "util.h"
#include "util/defs.h"
#include "util/types.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "effects/effectprocessor.h"
#include "sampleutil.h"

struct EchoGroupState {
    EchoGroupState() {
        delay_buf = SampleUtil::alloc(MAX_BUFFER_LEN);
        SampleUtil::applyGain(delay_buf, 0, MAX_BUFFER_LEN);
        prev_delay_time = 0.0;
        prev_delay_samples = 0;
        write_position = 0;
        ping_pong_left = true;
    }
    ~EchoGroupState() {
        SampleUtil::free(delay_buf);
    }
    CSAMPLE* delay_buf;
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
                      const unsigned int numSamples,
                      const unsigned int sampleRate,
                      const EffectProcessor::EnableState enableState,
                      const GroupFeatureState& groupFeatures);

  private:
    int getDelaySamples(double delay_time, const unsigned int sampleRate) const;

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
