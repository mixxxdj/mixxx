#include <QtDebug>

#include "effects/native/echoeffect.h"

#include "sampleutil.h"

#define INCREMENT_RING(index, increment, length) index = (index + increment) % length

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
    manifest.setDescription(QObject::tr("Simple Echo.  Applies "
            "feedback and runs a simple low-pass filter to reduce high "
            "frequencies"));

    EffectManifestParameter* time = manifest.addParameter();
    time->setId("delay_time");
    time->setName(QObject::tr("Delay"));
    time->setDescription(QObject::tr("Delay time (seconds)"));
    time->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    time->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    time->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    time->setUnitsHint(EffectManifestParameter::UNITS_TIME);
    time->setLinkHint(EffectManifestParameter::LINK_LINKED);
    time->setMinimum(0.1);
    time->setDefault(0.25);
    time->setMaximum(2.0);

    time = manifest.addParameter();
    time->setId("feedback_amount");
    time->setName(QObject::tr("Feedback"));
    time->setDescription(
            QObject::tr("Amount the echo fades each time it loops"));
    time->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    time->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    time->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    time->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    time->setMinimum(0.00);
    time->setDefault(0.40);
    // Allow > 1.0 feedback for DANGEROUS TESTING-ONLY feedback!
    time->setMaximum(1.1);

    time = manifest.addParameter();
    time->setId("pingpong_amount");
    time->setName(QObject::tr("PingPong"));
    time->setDescription(
            QObject::tr("As the ping-pong amount increases, increasing amounts "
                        "of the echoed signal is bounced between the left and "
                        "right speakers."));
    time->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    time->setValueHint(EffectManifestParameter::VALUE_FLOAT);
    time->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    time->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    time->setMinimum(0.0);
    time->setDefault(0.0);
    time->setMaximum(1.0);

    return manifest;
}

EchoEffect::EchoEffect(EngineEffect* pEffect, const EffectManifest& manifest)
        : m_pDelayParameter(pEffect->getParameterById("delay_time")),
          m_pFeedbackParameter(pEffect->getParameterById("feedback_amount")),
          m_pPingPongParameter(pEffect->getParameterById("pingpong_amount")) {
    Q_UNUSED(manifest);
}

EchoEffect::~EchoEffect() {
    //qDebug() << debugString() << "destroyed";
}

int EchoEffect::getDelaySamples(double delay_time) const {
    // TODO(owilliams): Use real samplerate.
    int delay_samples = delay_time * 44100;
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
                              const GroupFeatureState& groupFeatures) {
    Q_UNUSED(group);
    Q_UNUSED(groupFeatures);
    EchoGroupState& gs = *pGroupState;
    double delay_time =
            m_pDelayParameter ? m_pDelayParameter->value().toDouble() : 1.0f;
    double feedback_amount =
            m_pFeedbackParameter ? m_pFeedbackParameter->value().toDouble() : 0.25f;
    double pingpong_frac =
            m_pPingPongParameter ? m_pPingPongParameter->value().toDouble()
                                 : 0.25f;

    // TODO(owilliams): get actual sample rate from somewhere.

    int delay_samples = gs.prev_delay_samples;

    if (delay_time < gs.prev_delay_time) {
        // If the delay time has shrunk, we may need to wrap the write position.
        delay_samples = getDelaySamples(delay_time);
        gs.write_position = gs.write_position % delay_samples;
    } else if (delay_time > gs.prev_delay_time) {
        // If the delay time has grown, we need to zero out the new portion
        // of the buffer we are using.
        SampleUtil::applyGain(
                gs.delay_buf + gs.prev_delay_samples,
                0,
                MAX_BUFFER_LEN - gs.prev_delay_samples);
        delay_samples = getDelaySamples(delay_time);
    }

    int read_position = gs.write_position;
    gs.prev_delay_time = delay_time;
    gs.prev_delay_samples = delay_samples;

    // Lowpass the delay buffer to deaden it a bit.
    gs.feedback_lowpass->process(
            gs.delay_buf, gs.delay_buf, numSamples);

    // Feedback the delay buffer and then add the new input.
    for (unsigned int i = 0; i < numSamples; ++i) {
        gs.delay_buf[gs.write_position] *= feedback_amount;
        gs.delay_buf[gs.write_position] += pInput[i];
        INCREMENT_RING(gs.write_position, 1, delay_samples);
    }

    // TODO(owilliams): delay buffer clipping goes here.

    // Pingpong the output.  If the pingpong value is zero, all of the
    // math below should result in a simple copy of delay buf to pOutput.
    for (unsigned int i = 0; i + 1 < numSamples; i += 2) {
        if (gs.ping_pong_left) {
            // Left sample plus a fraction of the right sample, normalized
            // by 1 + fraction.
            pOutput[i] =
                    (gs.delay_buf[read_position] +
                            gs.delay_buf[read_position + 1] * pingpong_frac) /
                    (1 + pingpong_frac);
            // Right sample reduced by (1 - fraction)
            pOutput[i + 1] = gs.delay_buf[read_position + 1] * (1 - pingpong_frac);
        } else {
            // Left sample reduced by (1 - fraction)
            pOutput[i] = gs.delay_buf[read_position] * (1 - pingpong_frac);
            // Right sample plus fraction of left sample, normalized by
            // 1 + fraction
            pOutput[i + 1] =
                    (gs.delay_buf[read_position + 1] +
                            gs.delay_buf[read_position] * pingpong_frac) /
                    (1 + pingpong_frac);
        }

        INCREMENT_RING(read_position, 2, delay_samples);
        // If the buffer has looped around, flip-flop the ping-pong.
        if (read_position == 0) {
            gs.ping_pong_left = !gs.ping_pong_left;
        }
    }
}
