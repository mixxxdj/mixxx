#include "effects/backends/builtin/echoeffect.h"

#include "effects/backends/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/math.h"
#include "util/rampingvalue.h"
#include "util/sample.h"

constexpr int EchoGroupState::kMaxDelaySeconds;

namespace {

void incrementRing(int* pIndex, int increment, int length) {
    *pIndex = (*pIndex + increment) % length;
}

void decrementRing(int* pIndex, int decrement, int length) {
    *pIndex = (*pIndex + length - decrement) % length;
}

} // anonymous namespace

// static
QString EchoEffect::getId() {
    return "org.mixxx.effects.echo";
}

// static
EffectManifestPointer EchoEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());

    pManifest->setAddDryToWet(true);
    pManifest->setEffectRampsFromDry(true);

    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Echo"));
    pManifest->setShortName(QObject::tr("Echo"));
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription(QObject::tr(
            "Stores the input signal in a temporary buffer and outputs it after a short time"));

    EffectManifestParameterPointer delay = pManifest->addParameter();
    delay->setId("delay_time");
    delay->setName(QObject::tr("Time"));
    delay->setShortName(QObject::tr("Time"));
    delay->setDescription(QObject::tr(
            "Delay time\n"
            "1/8 - 2 beats if tempo is detected\n"
            "1/8 - 2 seconds if no tempo is detected"));
    delay->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    delay->setUnitsHint(EffectManifestParameter::UnitsHint::Beats);
    delay->setRange(0.0, 0.5, 2.0);

    EffectManifestParameterPointer feedback = pManifest->addParameter();
    feedback->setId("feedback_amount");
    feedback->setName(QObject::tr("Feedback"));
    feedback->setShortName(QObject::tr("Feedback"));
    feedback->setDescription(QObject::tr(
            "Amount the echo fades each time it loops"));
    feedback->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    feedback->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    feedback->setRange(0.00, db2ratio(-3.0), 1.00);

    EffectManifestParameterPointer pingpong = pManifest->addParameter();
    pingpong->setId("pingpong_amount");
    pingpong->setName(QObject::tr("Ping Pong"));
    pingpong->setShortName(QObject::tr("Ping Pong"));
    pingpong->setDescription(
            QObject::tr("How much the echoed sound bounces between the left "
                        "and right sides of the stereo field"));
    pingpong->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    pingpong->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    pingpong->setRange(0.0, 0.0, 1.0);

    EffectManifestParameterPointer send = pManifest->addParameter();
    send->setId("send_amount");
    send->setName(QObject::tr("Send"));
    send->setShortName(QObject::tr("Send"));
    send->setDescription(QObject::tr(
            "How much of the signal to send into the delay buffer"));
    send->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    send->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    send->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    send->setRange(0.0, db2ratio(-3.0), 1.0);

    EffectManifestParameterPointer quantize = pManifest->addParameter();
    quantize->setId("quantize");
    quantize->setName(QObject::tr("Quantize"));
    quantize->setShortName(QObject::tr("Quantize"));
    quantize->setDescription(QObject::tr(
            "Round the Time parameter to the nearest 1/4 beat."));
    quantize->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    quantize->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    quantize->setRange(0, 1, 1);

    EffectManifestParameterPointer triplet = pManifest->addParameter();
    triplet->setId("triplet");
    triplet->setName(QObject::tr("Triplets"));
    triplet->setShortName(QObject::tr("Triplets"));
    triplet->setDescription(
            QObject::tr("When the Quantize parameter is enabled, divide "
                        "rounded 1/4 beats of Time parameter by 3."));
    triplet->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    triplet->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    triplet->setRange(0, 0, 1);

    return pManifest;
}

void EchoEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pDelayParameter = parameters.value("delay_time");
    m_pSendParameter = parameters.value("send_amount");
    m_pFeedbackParameter = parameters.value("feedback_amount");
    m_pPingPongParameter = parameters.value("pingpong_amount");
    m_pQuantizeParameter = parameters.value("quantize");
    m_pTripletParameter = parameters.value("triplet");
}

void EchoEffect::processChannel(
        EchoGroupState* pGroupState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    // The minimum of the parameter is zero so the exact center of the knob is 1 beat.
    double period = m_pDelayParameter->value();
    const auto send_current = static_cast<CSAMPLE_GAIN>(m_pSendParameter->value());
    const auto feedback_current = static_cast<CSAMPLE_GAIN>(m_pFeedbackParameter->value());
    const auto pingpong_frac = static_cast<CSAMPLE_GAIN>(m_pPingPongParameter->value());

    double delay_seconds;
    if (groupFeatures.beat_length.has_value()) {
        // period is a number of beats
        if (m_pQuantizeParameter->toBool()) {
            period = std::max(roundToFraction(period, 4), 1 / 8.0);
            if (m_pTripletParameter->toBool()) {
                period /= 3.0;
            }
        } else if (period < 1 / 8.0) {
            period = 1 / 8.0;
        }
        delay_seconds = period * groupFeatures.beat_length->seconds;
    } else {
        // period is a number of seconds
        period = std::max(period, 1 / 8.0);
        delay_seconds = period;
    }
    VERIFY_OR_DEBUG_ASSERT(delay_seconds > 0) {
        delay_seconds = 1 / engineParameters.sampleRate();
    }

    int delay_samples = static_cast<int>(delay_seconds *
            engineParameters.channelCount() * engineParameters.sampleRate());
    VERIFY_OR_DEBUG_ASSERT(delay_samples <= pGroupState->delay_buf.size()) {
        delay_samples = pGroupState->delay_buf.size();
    }

    int prev_read_position = pGroupState->write_position;
    decrementRing(&prev_read_position,
            pGroupState->prev_delay_samples,
            pGroupState->delay_buf.size());
    int read_position = pGroupState->write_position;
    decrementRing(&read_position, delay_samples, pGroupState->delay_buf.size());

    RampingValue<CSAMPLE_GAIN> send(send_current,
            pGroupState->prev_send,
            engineParameters.framesPerBuffer());
    // Feedback the delay buffer and then add the new input.

    RampingValue<CSAMPLE_GAIN> feedback(feedback_current,
            pGroupState->prev_feedback,
            engineParameters.framesPerBuffer());

    int rampIndex = 0;
    //TODO: rewrite to remove assumption of stereo buffer
    for (SINT i = 0;
            i < engineParameters.samplesPerBuffer();
            i += engineParameters.channelCount()) {
        CSAMPLE_GAIN send_ramped = send.getNth(rampIndex);
        CSAMPLE_GAIN feedback_ramped = feedback.getNth(rampIndex);
        ++rampIndex;

        CSAMPLE bufferedSampleLeft = pGroupState->delay_buf[read_position];
        CSAMPLE bufferedSampleRight = pGroupState->delay_buf[read_position + 1];
        if (read_position != prev_read_position) {
            const CSAMPLE_GAIN frac = static_cast<CSAMPLE_GAIN>(i) /
                    engineParameters.samplesPerBuffer();
            bufferedSampleLeft *= frac;
            bufferedSampleRight *= frac;
            bufferedSampleLeft += pGroupState->delay_buf[prev_read_position] * (1 - frac);
            bufferedSampleRight += pGroupState->delay_buf[prev_read_position + 1] * (1 - frac);
            incrementRing(&prev_read_position,
                    engineParameters.channelCount(),
                    pGroupState->delay_buf.size());
        }
        incrementRing(&read_position,
                engineParameters.channelCount(),
                pGroupState->delay_buf.size());

        // Actual delays distort and saturate, so clamp the buffer here.
        pGroupState->delay_buf[pGroupState->write_position] = SampleUtil::clampSample(
                pInput[i] * send_ramped +
                bufferedSampleLeft * feedback_ramped);
        pGroupState->delay_buf[pGroupState->write_position + 1] = SampleUtil::clampSample(
                pInput[i + 1] * send_ramped +
                bufferedSampleRight * feedback_ramped);

        // Pingpong the output.  If the pingpong value is zero, all of the
        // math below should result in a simple copy of delay buf to pOutput.
        if (pGroupState->ping_pong < delay_samples / 2) {
            // Left sample plus a fraction of the right sample, normalized
            // by 1 + fraction.
            pOutput[i] =
                    (bufferedSampleLeft + bufferedSampleRight * pingpong_frac) /
                    (1 + pingpong_frac);
            // Right sample reduced by (1 - fraction)
            pOutput[i + 1] = bufferedSampleRight * (1 - pingpong_frac);
        } else {
            // Left sample reduced by (1 - fraction)
            pOutput[i] = bufferedSampleLeft * (1 - pingpong_frac);
            // Right sample plus fraction of left sample, normalized by
            // 1 + fraction
            pOutput[i + 1] =
                    (bufferedSampleRight + bufferedSampleLeft * pingpong_frac) /
                    (1 + pingpong_frac);
        }

        incrementRing(&pGroupState->write_position,
                engineParameters.channelCount(),
                pGroupState->delay_buf.size());

        ++(pGroupState->ping_pong);
        if (pGroupState->ping_pong >= delay_samples) {
            pGroupState->ping_pong = 0;
        }
    }

    // The ramping of the send parameter handles ramping when enabling, so
    // this effect must handle ramping to dry when disabling itself (instead
    // of being handled by EngineEffect::process).
    if (enableState == EffectEnableState::Disabling) {
        SampleUtil::applyRampingGain(pOutput, 1.0, 0.0, engineParameters.samplesPerBuffer());
        pGroupState->delay_buf.clear();
        pGroupState->prev_send = 0;
    } else {
        pGroupState->prev_send = send_current;
    }

    pGroupState->prev_feedback = feedback_current;
    pGroupState->prev_delay_samples = delay_samples;
}
