#include "effects/native/echoeffect.h"

#include <QtDebug>

#include "util/sample.h"

#define INCREMENT_RING(index, increment, length) index = (index + increment) % length

constexpr int EchoGroupState::kMaxDelaySeconds;
constexpr int EchoGroupState::kChannelCount;
constexpr int EchoGroupState::kRampLength;

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

    EffectManifestParameter* delay = manifest.addParameter();
    delay->setId("delay_time");
    delay->setName(QObject::tr("Delay"));
    delay->setDescription(QObject::tr("Delay time (seconds)"));
    delay->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    delay->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    delay->setUnitsHint(EffectManifestParameter::UnitsHint::TIME);
    delay->setMinimum(0.1);
    delay->setDefault(1.0);
    delay->setMaximum(EchoGroupState::kMaxDelaySeconds);

    EffectManifestParameter* feedback = manifest.addParameter();
    feedback->setId("feedback_amount");
    feedback->setName(QObject::tr("Feedback"));
    feedback->setDescription(
            QObject::tr("Amount the echo fades each time it loops"));
    feedback->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    feedback->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    feedback->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    feedback->setMinimum(0.00);
    feedback->setDefault(0.5);
    feedback->setMaximum(1.0);

    EffectManifestParameter* pingpong = manifest.addParameter();
    pingpong->setId("pingpong_amount");
    pingpong->setName(QObject::tr("PingPong"));
    pingpong->setDescription(
            QObject::tr("As the ping-pong amount increases, increasing amounts "
                        "of the echoed signal is bounced between the left and "
                        "right speakers."));
    pingpong->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    pingpong->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    pingpong->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    pingpong->setMinimum(0.0);
    pingpong->setDefault(0.0);
    pingpong->setMaximum(1.0);

    EffectManifestParameter* send = manifest.addParameter();
    send->setId("send_amount");
    send->setName(QObject::tr("Send"));
    send->setDescription(
            QObject::tr("How much of the signal to send into the delay buffer"));
    send->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    send->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    send->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    send->setDefaultLinkType(EffectManifestParameter::LinkType::LINKED);
    send->setMinimum(0.0);
    send->setDefault(1.0);
    send->setMaximum(1.0);

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
}

void EchoEffect::processChannel(const ChannelHandle& handle, EchoGroupState* pGroupState,
                                const CSAMPLE* pInput,
                                CSAMPLE* pOutput, const unsigned int numSamples,
                                const unsigned int sampleRate,
                                const EffectProcessor::EnableState enableState,
                                const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);
    Q_UNUSED(groupFeatures);
    DEBUG_ASSERT(0 == (numSamples % EchoGroupState::kChannelCount));
    EchoGroupState& gs = *pGroupState;
    double delay_time = m_pDelayParameter->value();
    double send_amount = m_pSendParameter->value();
    double feedback_amount = m_pFeedbackParameter->value();
    double pingpong_frac = m_pPingPongParameter->value();

    int delay_samples = EchoGroupState::kChannelCount * delay_time * sampleRate;
    VERIFY_OR_DEBUG_ASSERT(delay_samples <= gs.delay_buf.size()) {
        delay_samples = gs.delay_buf.size();
    }

    if (delay_time < gs.prev_delay_time) {
        // If the delay time has shrunk, we may need to wrap the write position.
        gs.write_position = gs.write_position % delay_samples;
    } else if (delay_time > gs.prev_delay_time) {
        // If the delay time has grown, we need to zero out the new portion
        // of the buffer we are using.
        SampleUtil::applyGain(gs.delay_buf.data(gs.prev_delay_samples), 0,
                              gs.delay_buf.size() - gs.prev_delay_samples);
    }

    int read_position = gs.write_position;
    gs.prev_delay_time = delay_time;
    gs.prev_delay_samples = delay_samples;

    // Feedback the delay buffer and then add the new input.
    for (unsigned int i = 0; i < numSamples; i += EchoGroupState::kChannelCount) {
        // Ramp the beginning and end of the delay buffer to prevent clicks.
        double write_ramper = 1.0;
        if (gs.write_position < EchoGroupState::kRampLength) {
            write_ramper = static_cast<double>(gs.write_position) / EchoGroupState::kRampLength;
        } else if (gs.write_position > delay_samples - EchoGroupState::kRampLength) {
            write_ramper = static_cast<double>(delay_samples - gs.write_position)
                    / EchoGroupState::kRampLength;
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
        INCREMENT_RING(gs.write_position, EchoGroupState::kChannelCount, delay_samples);
    }

    // Pingpong the output.  If the pingpong value is zero, all of the
    // math below should result in a simple copy of delay buf to pOutput.
    for (unsigned int i = 0; i < numSamples; i += EchoGroupState::kChannelCount) {
        if (gs.ping_pong_left) {
            // Left sample plus a fraction of the right sample, normalized
            // by 1 + fraction.
            pOutput[i] = pInput[i] +
                    ((gs.delay_buf[read_position] +
                            gs.delay_buf[read_position + 1] * pingpong_frac) /
                    (1 + pingpong_frac)) / 2.0;
            // Right sample reduced by (1 - fraction)
            pOutput[i + 1] = pInput[i + 1] +
                    (gs.delay_buf[read_position + 1] * (1 - pingpong_frac)) / 2.0;
        } else {
            // Left sample reduced by (1 - fraction)
            pOutput[i] = pInput[i] +
                    (gs.delay_buf[read_position] * (1 - pingpong_frac)) / 2.0;
            // Right sample plus fraction of left sample, normalized by
            // 1 + fraction
            pOutput[i + 1] = pInput[i + 1] +
                    ((gs.delay_buf[read_position + 1] +
                            gs.delay_buf[read_position] * pingpong_frac) /
                    (1 + pingpong_frac)) / 2.0;
        }

        INCREMENT_RING(read_position, EchoGroupState::kChannelCount, delay_samples);
        // If the buffer has looped around, flip-flop the ping-pong.
        if (read_position == 0) {
            gs.ping_pong_left = !gs.ping_pong_left;
        }
    }

    if (enableState == EffectProcessor::DISABLING) {
        gs.delay_buf.clear();
    }
}
