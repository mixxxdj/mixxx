#include <QtDebug>

#include "effects/native/echoeffect.h"

#include "sampleutil.h"

#define INCREMENT_RING(index, increment, length) index = (index + increment) % length
#define RAMP_LENGTH 500

// static
QString EchoEffect::getId() {
    return "org.mixxx.effects.echo";
}

// static
EffectManifest EchoEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Echo"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr("Simple Echo with pingpong"));

    EffectManifestParameter* time = manifest.addParameter();
    time->setId("send_amount");
    time->setName(QObject::tr("Send"));
    time->setDescription(
            QObject::tr("How much of the signal to send into the delay buffer"));
    time->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    time->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    time->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    time->setMinimum(0.0);
    time->setDefault(1.0);
    time->setMaximum(1.0);

    time = manifest.addParameter();
    time->setId("delay_time");
    time->setName(QObject::tr("Delay"));
    time->setDescription(QObject::tr("Delay time (seconds)"));
    time->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    time->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    time->setUnitsHint(EffectManifestParameter::UNITS_TIME);
    time->setDefaultLinkType(EffectManifestParameter::LINK_LINKED);
    time->setMinimum(0.1);
    time->setDefault(0.25);
    time->setMaximum(2.0);

    time = manifest.addParameter();
    time->setId("feedback_amount");
    time->setName(QObject::tr("Feedback"));
    time->setDescription(
            QObject::tr("Amount the echo fades each time it loops"));
    time->setControlHint(EffectManifestParameter::CONTROL_KNOB_LOGARITHMIC);
    time->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    time->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    time->setMinimum(0.00);
    time->setDefault(0.40);
    time->setMaximum(1.0);

    time = manifest.addParameter();
    time->setId("pingpong_amount");
    time->setName(QObject::tr("PingPong"));
    time->setDescription(
            QObject::tr("As the ping-pong amount increases, increasing amounts "
                        "of the echoed signal is bounced between the left and "
                        "right speakers."));
    time->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    time->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    time->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    time->setMinimum(0.0);
    time->setDefault(0.0);
    time->setMaximum(1.0);

    return manifest;
}

EchoEffect::EchoEffect(EngineEffect* pEffect, const EffectManifest& manifest)
        : m_pDelayParameter(pEffect->getParameterById("delay_time")),
          m_pSendParameter(pEffect->getParameterById("send_amount")),
          m_pFeedbackParameter(pEffect->getParameterById("feedback_amount")),
          m_pPingPongParameter(pEffect->getParameterById("pingpong_amount")) {
    Q_UNUSED(manifest);
}

EchoEffect::~EchoEffect() {
    //qDebug() << debugString() << "destroyed";
}

int EchoEffect::getDelaySamples(double delay_time, const unsigned int sampleRate) const {
    int delay_samples = delay_time * sampleRate;
    if (delay_samples % 2 == 1) {
        --delay_samples;
    }
    if (delay_samples > static_cast<int>(MAX_BUFFER_LEN)) {
        qWarning() << "Delay buffer requested is larger than max buffer!";
        delay_samples = static_cast<int>(MAX_BUFFER_LEN);
    }
    return delay_samples;
}

void EchoEffect::processGroup(const QString& group, EchoGroupState* pGroupState,
                              const CSAMPLE* pInput,
                              CSAMPLE* pOutput, const unsigned int numSamples,
                              const unsigned int sampleRate,
                              const EffectProcessor::EnableState enableState,
                              const GroupFeatureState& groupFeatures) {
    Q_UNUSED(group);
    Q_UNUSED(enableState);
    Q_UNUSED(groupFeatures);
    EchoGroupState& gs = *pGroupState;
    double delay_time = m_pDelayParameter->value();
    double send_amount = m_pSendParameter->value();
    double feedback_amount = m_pFeedbackParameter->value();
    double pingpong_frac = m_pPingPongParameter->value();

    // TODO(owilliams): get actual sample rate from somewhere.

    int delay_samples = gs.prev_delay_samples;

    if (delay_time < gs.prev_delay_time) {
        // If the delay time has shrunk, we may need to wrap the write position.
        delay_samples = getDelaySamples(delay_time, sampleRate);
        gs.write_position = gs.write_position % delay_samples;
    } else if (delay_time > gs.prev_delay_time) {
        // If the delay time has grown, we need to zero out the new portion
        // of the buffer we are using.
        SampleUtil::applyGain(gs.delay_buf + gs.prev_delay_samples, 0,
                              MAX_BUFFER_LEN - gs.prev_delay_samples);
        delay_samples = getDelaySamples(delay_time, sampleRate);
    }

    int read_position = gs.write_position;
    gs.prev_delay_time = delay_time;
    gs.prev_delay_samples = delay_samples;

    // Feedback the delay buffer and then add the new input.
    for (unsigned int i = 0; i < numSamples; i += 2) {
        // Ramp the beginning and end of the delay buffer to prevent clicks.
        double write_ramper = 1.0;
        if (gs.write_position < RAMP_LENGTH) {
            write_ramper = static_cast<double>(gs.write_position) / RAMP_LENGTH;
        } else if (gs.write_position > delay_samples - RAMP_LENGTH) {
            write_ramper = static_cast<double>(delay_samples - gs.write_position)
                    / RAMP_LENGTH;
        }
        gs.delay_buf[gs.write_position] *= feedback_amount;
        gs.delay_buf[gs.write_position + 1] *= feedback_amount;
        gs.delay_buf[gs.write_position] += pInput[i] * send_amount * write_ramper;
        gs.delay_buf[gs.write_position + 1] += pInput[i + 1] * send_amount * write_ramper;
        // Actual delays distort and saturate, so clamp the buffer here.
        gs.delay_buf[gs.write_position] =
                SampleUtil::clampSample(gs.delay_buf[gs.write_position]);
        gs.delay_buf[gs.write_position + 1] =
                SampleUtil::clampSample(gs.delay_buf[gs.write_position + 1]);
        INCREMENT_RING(gs.write_position, 2, delay_samples);
    }

    // Pingpong the output.  If the pingpong value is zero, all of the
    // math below should result in a simple copy of delay buf to pOutput.
    for (unsigned int i = 0; i + 1 < numSamples; i += 2) {
        if (gs.ping_pong_left) {
            // Left sample plus a fraction of the right sample, normalized
            // by 1 + fraction.
            pOutput[i] = (pInput[i] +
                    (gs.delay_buf[read_position] +
                            gs.delay_buf[read_position + 1] * pingpong_frac) /
                    (1 + pingpong_frac)) / 2.0;
            // Right sample reduced by (1 - fraction)
            pOutput[i + 1] = (pInput[i + 1] +
                    gs.delay_buf[read_position + 1] * (1 - pingpong_frac)) / 2.0;
        } else {
            // Left sample reduced by (1 - fraction)
            pOutput[i] = (pInput[i] +
                    gs.delay_buf[read_position] * (1 - pingpong_frac)) / 2.0;
            // Right sample plus fraction of left sample, normalized by
            // 1 + fraction
            pOutput[i + 1] = (pInput[i + 1] +
                    (gs.delay_buf[read_position + 1] +
                            gs.delay_buf[read_position] * pingpong_frac) /
                    (1 + pingpong_frac)) / 2.0;
        }

        INCREMENT_RING(read_position, 2, delay_samples);
        // If the buffer has looped around, flip-flop the ping-pong.
        if (read_position == 0) {
            gs.ping_pong_left = !gs.ping_pong_left;
        }
    }
}
