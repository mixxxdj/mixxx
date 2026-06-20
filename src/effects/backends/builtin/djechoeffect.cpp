#include "effects/backends/builtin/djechoeffect.h"

#include <algorithm>
#include <cmath>

#include "effects/backends/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/math.h"
#include "util/rampingvalue.h"
#include "util/sample.h"

constexpr int DJEchoGroupState::kMaxDelaySeconds;

namespace {

void incrementRing(int* pIndex, int increment, int length) {
    *pIndex = (*pIndex + increment) % length;
}

void decrementRing(int* pIndex, int decrement, int length) {
    *pIndex = (*pIndex + length - decrement) % length;
}

} // anonymous namespace

// static
QString DJEchoEffect::getId() {
    return "org.mixxx.effects.djecho";
}

// static
EffectManifestPointer DJEchoEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());

    pManifest->setAddDryToWet(true);
    pManifest->setEffectRampsFromDry(true);

    pManifest->setId(getId());
    pManifest->setName(QObject::tr("DJ Echo"));
    pManifest->setShortName(QObject::tr("DJ Echo"));
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription(QObject::tr(
            "High-quality echo with beat sync, feedback filtering, "
            "and stereo ping-pong. Designed for professional DJ mixing."));

    EffectManifestParameterPointer delay = pManifest->addParameter();
    delay->setId("delay_time");
    delay->setName(QObject::tr("Time"));
    delay->setShortName(QObject::tr("Time"));
    delay->setDescription(QObject::tr(
            "Delay time\n"
            "1/8 - 2 beats if tempo is detected\n"
            "1/8 - 4 seconds if no tempo is detected"));
    delay->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    delay->setUnitsHint(EffectManifestParameter::UnitsHint::Beats);
    delay->setRange(0.0, 0.5, 4.0);

    EffectManifestParameterPointer feedback = pManifest->addParameter();
    feedback->setId("feedback_amount");
    feedback->setName(QObject::tr("Feedback"));
    feedback->setShortName(QObject::tr("Feedback"));
    feedback->setDescription(QObject::tr("Amount the echo fades each time it loops"));
    feedback->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    feedback->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    feedback->setRange(0.00, db2ratio(-6.0), 0.95);

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

    EffectManifestParameterPointer warmth = pManifest->addParameter();
    warmth->setId("warmth");
    warmth->setName(QObject::tr("Warmth"));
    warmth->setShortName(QObject::tr("Warmth"));
    warmth->setDescription(QObject::tr(
            "Low-pass filter cutoff for the feedback loop.\n"
            "Higher values = brighter echo, lower values = warmer/darker echo."));
    warmth->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    warmth->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    warmth->setRange(0.0, 0.7, 1.0);

    EffectManifestParameterPointer hihat = pManifest->addParameter();
    hihat->setId("hihat");
    hihat->setName(QObject::tr("High Cut"));
    hihat->setShortName(QObject::tr("HiCut"));
    hihat->setDescription(QObject::tr(
            "High-pass filter for the feedback loop.\n"
            "Removes low-end buildup that muddies the mix."));
    hihat->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    hihat->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    hihat->setRange(0.0, 0.3, 1.0);

    EffectManifestParameterPointer quantize = pManifest->addParameter();
    quantize->setId("quantize");
    quantize->setName(QObject::tr("Quantize"));
    quantize->setShortName(QObject::tr("Quantize"));
    quantize->setDescription(
            QObject::tr("When enabled, the delay time snaps to beat divisions."));
    quantize->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    quantize->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    quantize->setRange(0, 0, 1);

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

    EffectManifestParameterPointer send = pManifest->addParameter();
    send->setId("send_amount");
    send->setName(QObject::tr("Send"));
    send->setShortName(QObject::tr("Send"));
    send->setDescription(QObject::tr(
            "Amount of the input signal sent to the echo buffer.\n"
            "Higher values create stronger initial echo."));
    send->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    send->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    send->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    send->setRange(0.0, db2ratio(-3.0), 1.00);

    return pManifest;
}

void DJEchoEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pDelayParameter = parameters.value("delay_time");
    m_pSendParameter = parameters.value("send_amount");
    m_pFeedbackParameter = parameters.value("feedback_amount");
    m_pPingPongParameter = parameters.value("pingpong_amount");
    m_pQuantizeParameter = parameters.value("quantize");
    m_pTripletParameter = parameters.value("triplet");
    m_pHiHatParameter = parameters.value("hihat");
    m_pWarmthParameter = parameters.value("warmth");
}

void DJEchoEffect::processChannel(
        DJEchoGroupState* pGroupState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        [[maybe_unused]] const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    double period = m_pDelayParameter->value();
    const auto send_current = static_cast<CSAMPLE_GAIN>(m_pSendParameter->value());
    const auto feedback_current = static_cast<CSAMPLE_GAIN>(m_pFeedbackParameter->value());
    const auto pingpong_frac = static_cast<CSAMPLE_GAIN>(m_pPingPongParameter->value());

    // Warmth and HiCut filter parameters
    const auto warmth = static_cast<CSAMPLE_GAIN>(m_pWarmthParameter->value());
    const auto hihat = static_cast<CSAMPLE_GAIN>(m_pHiHatParameter->value());

    // Convert warmth/hihat to filter cutoff frequencies (normalized 0-1)
    // Warmth: 0.0 = dark (500Hz LP), 1.0 = bright (20000Hz LP)
    const CSAMPLE_GAIN lp_cutoff = static_cast<CSAMPLE_GAIN>(500.0 + warmth * 19500.0);
    // HiCut: 0.0 = no HPF, 1.0 = aggressive HPF at 500Hz
    const CSAMPLE_GAIN hp_cutoff = static_cast<CSAMPLE_GAIN>(hihat * 500.0);

    double delay_seconds;
    if (groupFeatures.beat_length.has_value()) {
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
        period = std::max(period, 1 / 8.0);
        delay_seconds = period;
    }
    delay_seconds = std::min(delay_seconds,
            static_cast<double>(DJEchoGroupState::kMaxDelaySeconds));
    VERIFY_OR_DEBUG_ASSERT(delay_seconds > 0) {
        delay_seconds = 1 / engineParameters.sampleRate();
    }

    int delay_frames = static_cast<int>(delay_seconds * engineParameters.sampleRate());
    int delay_samples = delay_frames * engineParameters.channelCount();
    VERIFY_OR_DEBUG_ASSERT(delay_samples <= pGroupState->delay_buf.size()) {
        delay_samples = pGroupState->delay_buf.size();
    }

    int prev_read_position = pGroupState->write_position;
    decrementRing(&prev_read_position,
            pGroupState->prev_delay_samples,
            pGroupState->delay_buf.size());
    int read_position = pGroupState->write_position;
    decrementRing(&read_position, delay_samples, pGroupState->delay_buf.size());

    RampingValue<CSAMPLE_GAIN> send(pGroupState->prev_send,
            send_current,
            engineParameters.framesPerBuffer());
    RampingValue<CSAMPLE_GAIN> feedback(pGroupState->prev_feedback,
            feedback_current,
            engineParameters.framesPerBuffer());

    int rampIndex = 0;
    const auto chCount = engineParameters.channelCount();
    const double sampleRate = engineParameters.sampleRate();

    // Pre-compute filter coefficients
    constexpr double k2Pi = 6.283185307179586476925286766559;
    const CSAMPLE_GAIN lp_alpha = static_cast<CSAMPLE_GAIN>(
            1.0 - std::exp(-k2Pi * lp_cutoff / sampleRate));
    const CSAMPLE_GAIN hp_alpha = static_cast<CSAMPLE_GAIN>(
            1.0 - std::exp(-k2Pi * hp_cutoff / sampleRate));

    for (SINT i = 0; i < engineParameters.samplesPerBuffer(); i += chCount) {
        CSAMPLE_GAIN send_ramped = send.getNth(rampIndex);
        CSAMPLE_GAIN feedback_ramped = feedback.getNth(rampIndex);
        ++rampIndex;

        // Read from delay buffer with linear interpolation
        CSAMPLE bufferedSampleLeft = pGroupState->delay_buf[read_position];
        CSAMPLE bufferedSampleRight = 0;
        if (chCount > 1) {
            bufferedSampleRight = pGroupState->delay_buf[read_position + 1];
        }

        if (read_position != prev_read_position) {
            const CSAMPLE_GAIN frac = static_cast<CSAMPLE_GAIN>(i) /
                    engineParameters.samplesPerBuffer();
            bufferedSampleLeft *= frac;
            bufferedSampleLeft += pGroupState->delay_buf[prev_read_position] * (1 - frac);
            if (chCount > 1) {
                bufferedSampleRight *= frac;
                bufferedSampleRight += pGroupState->delay_buf[prev_read_position + 1] * (1 - frac);
            }
            incrementRing(&prev_read_position, chCount, pGroupState->delay_buf.size());
            incrementRing(&read_position, chCount, pGroupState->delay_buf.size());
        }

        // Apply feedback filtering to the delayed signal
        CSAMPLE filteredLeft = bufferedSampleLeft;
        CSAMPLE filteredRight = bufferedSampleRight;

        // Low-pass filter for warmth (one-pole)
        pGroupState->feedback_lp_state[0] +=
                lp_alpha * (filteredLeft - pGroupState->feedback_lp_state[0]);
        filteredLeft = pGroupState->feedback_lp_state[0];

        if (chCount > 1) {
            pGroupState->feedback_lp_state[1] += lp_alpha *
                    (filteredRight - pGroupState->feedback_lp_state[1]);
            filteredRight = pGroupState->feedback_lp_state[1];
        }

        // High-pass filter to remove low-end buildup (one-pole)
        pGroupState->feedback_filter_state[0] += hp_alpha *
                (filteredLeft - pGroupState->feedback_filter_state[0]);
        filteredLeft -= pGroupState->feedback_filter_state[0];

        if (chCount > 1) {
            pGroupState->feedback_filter_state[1] += hp_alpha *
                    (filteredRight - pGroupState->feedback_filter_state[1]);
            filteredRight -= pGroupState->feedback_filter_state[1];
        }

        // Output is dry + wet
        pOutput[i] = pInput[i] + filteredLeft * send_ramped;
        if (chCount > 1) {
            pOutput[i + 1] = pInput[i + 1] + filteredRight * send_ramped;
        }

        // Write to delay buffer with feedback and optional ping-pong
        CSAMPLE writeLeft = pInput[i] + filteredLeft * feedback_ramped;
        CSAMPLE writeRight = 0;
        if (chCount > 1) {
            writeRight = pInput[i + 1] + filteredRight * feedback_ramped;
        }

        // Ping-pong: swap channels based on pingpong_frac
        if (chCount > 1 && pingpong_frac > 0) {
            CSAMPLE_GAIN straight = static_cast<CSAMPLE_GAIN>(1.0 - pingpong_frac);
            CSAMPLE_GAIN crossed = pingpong_frac;
            CSAMPLE origLeft = writeLeft;
            CSAMPLE origRight = writeRight;
            writeLeft = origLeft * straight + origRight * crossed;
            writeRight = origRight * straight + origLeft * crossed;
        }

        pGroupState->delay_buf[pGroupState->write_position] = writeLeft;
        if (chCount > 1) {
            pGroupState->delay_buf[pGroupState->write_position + 1] = writeRight;
        }
        incrementRing(&pGroupState->write_position, chCount, pGroupState->delay_buf.size());
    }

    pGroupState->prev_send = send_current;
    pGroupState->prev_feedback = feedback_current;
    pGroupState->prev_delay_samples = delay_samples;
}
