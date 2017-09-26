#include "effects/native/echoeffect.h"

#include <QtDebug>

#include "util/sample.h"
#include "util/math.h"

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
    delay->setName(QObject::tr("Time"));
    delay->setDescription(QObject::tr("Delay time\n"
        "1/8 - 2 beats if tempo is detected (decks and samplers) \n"
        "1/8 - 2 seconds if no tempo is detected (mic & aux inputs, master mix)"));
    delay->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    delay->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    delay->setUnitsHint(EffectManifestParameter::UnitsHint::BEATS);
    delay->setMinimum(0.0);
    delay->setDefault(0.5);
    delay->setMaximum(2.0);

    EffectManifestParameter* feedback = manifest.addParameter();
    feedback->setId("feedback_amount");
    feedback->setName(QObject::tr("Feedback"));
    feedback->setDescription(
            QObject::tr("Amount the echo fades each time it loops"));
    feedback->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    feedback->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    feedback->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    feedback->setMinimum(0.00);
    feedback->setDefault(0.75);
    feedback->setMaximum(1.00);

    EffectManifestParameter* pingpong = manifest.addParameter();
    pingpong->setId("pingpong_amount");
    pingpong->setName(QObject::tr("Ping Pong"));
    pingpong->setDescription(
            QObject::tr("As the ping pong amount increases, increasing amounts "
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

    EffectManifestParameter* quantize = manifest.addParameter();
    quantize->setId("quantize");
    quantize->setName("Quantize");
    quantize->setShortName("Quantize");
    quantize->setDescription("Round the Time parameter to the nearest 1/4 beat.");
    quantize->setControlHint(EffectManifestParameter::ControlHint::TOGGLE_STEPPING);
    quantize->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    quantize->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    quantize->setDefault(1);
    quantize->setMinimum(0);
    quantize->setMaximum(1);

    EffectManifestParameter* triplet = manifest.addParameter();
    triplet->setId("triplet");
    triplet->setName("Triplets");
    triplet->setDescription("When the Quantize parameter is enabled, divide rounded 1/4 beats of Time parameter by 3.");
    triplet->setControlHint(EffectManifestParameter::ControlHint::TOGGLE_STEPPING);
    triplet->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    triplet->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    triplet->setDefault(0);
    triplet->setMinimum(0);
    triplet->setMaximum(1);

    return manifest;
}

EchoEffect::EchoEffect(EngineEffect* pEffect, const EffectManifest& manifest)
        : m_pDelayParameter(pEffect->getParameterById("delay_time")),
          m_pSendParameter(pEffect->getParameterById("send_amount")),
          m_pFeedbackParameter(pEffect->getParameterById("feedback_amount")),
          m_pPingPongParameter(pEffect->getParameterById("pingpong_amount")),
          m_pQuantizeParameter(pEffect->getParameterById("quantize")),
          m_pTripletParameter(pEffect->getParameterById("triplet")) {
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

    DEBUG_ASSERT(0 == (numSamples % EchoGroupState::kChannelCount));
    EchoGroupState& gs = *pGroupState;
    // The minimum of the parameter is zero so the exact center of the knob is 1 beat.
    double period = m_pDelayParameter->value();
    double send_amount = m_pSendParameter->value();
    double feedback_amount = m_pFeedbackParameter->value();
    double pingpong_frac = m_pPingPongParameter->value();

    int delay_samples;
    if (groupFeatures.has_beat_length_sec) {
        // period is a number of beats
        if (m_pQuantizeParameter->toBool()) {
            period = std::max(roundToFraction(period, 4), 1/8.0);
            if (m_pTripletParameter->toBool()) {
                period /= 3.0;
            }
        } else if (period < 1/8.0) {
            period = 1/8.0;
        }
        delay_samples = period * groupFeatures.beat_length_sec
                * sampleRate * EchoGroupState::kChannelCount;
    } else {
        // period is a number of seconds
        period = std::max(period, 1/8.0);
        delay_samples = period * sampleRate * EchoGroupState::kChannelCount;
    }
    VERIFY_OR_DEBUG_ASSERT(delay_samples > 0) {
        delay_samples = 1;
    }
    VERIFY_OR_DEBUG_ASSERT(delay_samples <= gs.delay_buf.size()) {
        delay_samples = gs.delay_buf.size();
    }

    if (period < gs.prev_period) {
        // If the delay time has shrunk, we may need to wrap the write position.
        gs.write_position = gs.write_position % delay_samples;
    } else if (period > gs.prev_period) {
        // If the delay time has grown, we need to zero out the new portion
        // of the buffer we are using.
        SampleUtil::applyGain(gs.delay_buf.data(gs.prev_delay_samples), 0,
                              gs.delay_buf.size() - gs.prev_delay_samples);
    }

    int read_position = gs.write_position;

    // Feedback the delay buffer and then add the new input.
    const CSAMPLE_GAIN send_delta = (send_amount - gs.prev_send) /
            (numSamples / EchoGroupState::kChannelCount);
    const CSAMPLE_GAIN send_start = send_amount + send_delta;
    for (unsigned int i = 0; i < numSamples; i += EchoGroupState::kChannelCount) {
        CSAMPLE_GAIN send_ramped = send_start;
        if (send_delta > 0.0) {
            send_ramped += send_delta * i / EchoGroupState::kChannelCount;
        }
        gs.delay_buf[gs.write_position] *= feedback_amount;
        gs.delay_buf[gs.write_position + 1] *= feedback_amount;
        gs.delay_buf[gs.write_position] += pInput[i] * send_ramped;
        gs.delay_buf[gs.write_position + 1] += pInput[i + 1] * send_ramped;
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

    gs.prev_period = period;
    gs.prev_send = send_amount;
    gs.prev_delay_samples = delay_samples;
}
