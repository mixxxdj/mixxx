#include "effects/backends/builtin/proreverbeffect.h"

#include <algorithm>
#include <cmath>

#include "effects/backends/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/sample.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {
// Prime number ratios for tap delays to avoid flutter echo
constexpr int kTapRatios[] = {1, 7, 13, 19, 31, 43, 61, 73};
constexpr int kNumTaps = 8;

// Damping filter coefficient (one-pole low-pass)
constexpr double kDampingMin = 0.1;
constexpr double kDampingMax = 0.95;

// Diffusion all-pass filter coefficient
constexpr double kDiffusionCoeff = 0.7;

// Mode presets: Hall, Room, Plate, Spring
struct ReverbPreset {
    double feedback;
    double size;
    double damping;
    const char* name;
};

constexpr ReverbPreset kPresets[] = {
        {0.85, 0.75, 0.5, "Hall"},
        {0.65, 0.45, 0.6, "Room"},
        {0.75, 0.55, 0.4, "Plate"},
        {0.55, 0.30, 0.7, "Spring"},
};
constexpr int kNumModes = 4;

} // anonymous namespace

QString ProReverbEffect::getId() {
    return QStringLiteral("org.mixxx.effects.proreverb");
}

EffectManifestPointer ProReverbEffect::getManifest() {
    auto pManifest = EffectManifestPointer::create();
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Pro Reverb"));
    pManifest->setAuthor("DJ Sugar");
    pManifest->setVersion("1.0");
    pManifest->setDescription(QObject::tr(
            "Professional multi-mode reverb with Hall, Room, Plate, and Spring modes. "
            "Comparable to Rekordbox and Serato reverb quality."));

    auto pSend = pManifest->addParameter();
    pSend->setId("send");
    pSend->setName(QObject::tr("Send"));
    pSend->setDescription(QObject::tr("Amount of signal sent to reverb"));
    pSend->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    pSend->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    pSend->setRange(0.0, 0.3, 1.0);

    auto pFeedback = pManifest->addParameter();
    pFeedback->setId("feedback");
    pFeedback->setName(QObject::tr("Feedback"));
    pFeedback->setDescription(QObject::tr("Reverb feedback amount (tail length)"));
    pFeedback->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    pFeedback->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    pFeedback->setRange(0.0, 0.6, 0.95);

    auto pSize = pManifest->addParameter();
    pSize->setId("size");
    pSize->setName(QObject::tr("Size"));
    pSize->setDescription(QObject::tr("Reverb room size"));
    pSize->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    pSize->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    pSize->setRange(0.1, 0.5, 1.0);

    auto pDamping = pManifest->addParameter();
    pDamping->setId("damping");
    pDamping->setName(QObject::tr("Damping"));
    pDamping->setDescription(QObject::tr("High frequency damping (darkness)"));
    pDamping->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    pDamping->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    pDamping->setRange(0.0, 0.5, 1.0);

    auto pMode = pManifest->addParameter();
    pMode->setId("mode");
    pMode->setName(QObject::tr("Mode"));
    pMode->setDescription(QObject::tr("Reverb mode: 0=Hall, 1=Room, 2=Plate, 3=Spring"));
    pMode->setValueScaler(EffectManifestParameter::ValueScaler::Integral);
    pMode->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    pMode->setRange(0.0, 0.0, 3.0);

    auto pDryWet = pManifest->addParameter();
    pDryWet->setId("drywet");
    pDryWet->setName(QObject::tr("Dry/Wet"));
    pDryWet->setDescription(QObject::tr("Dry/wet mix"));
    pDryWet->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    pDryWet->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    pDryWet->setRange(0.0, 0.3, 1.0);

    return pManifest;
}

void ProReverbEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pSendParameter = parameters.value("send");
    m_pFeedbackParameter = parameters.value("feedback");
    m_pSizeParameter = parameters.value("size");
    m_pDampingParameter = parameters.value("damping");
    m_pModeParameter = parameters.value("mode");
    m_pDryWetParameter = parameters.value("drywet");
}

void ProReverbEffect::updateTapDelays(
        ProReverbGroupState* pState,
        const mixxx::EngineParameters& engineParameters,
        int size_param) {
    const int sampleRate = engineParameters.sampleRate();
    const double baseDelay = size_param / 100.0 * sampleRate;

    for (int i = 0; i < kNumTaps; ++i) {
        double delay = baseDelay * kTapRatios[i] / kTapRatios[kNumTaps - 1];
        pState->tap_delays[i] = std::max(1, static_cast<int>(delay));
        pState->tap_gains[i] = static_cast<CSAMPLE>(
                1.0 / (1.0 + i * 0.3));
    }
}

void ProReverbEffect::processChannel(
        ProReverbGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        [[maybe_unused]] const EffectEnableState enableState,
        [[maybe_unused]] const GroupFeatureState& groupFeatures) {
    const SINT numSamples = engineParameters.samplesPerBuffer();

    const CSAMPLE_GAIN send = static_cast<CSAMPLE_GAIN>(m_pSendParameter->value());
    const CSAMPLE_GAIN feedback = static_cast<CSAMPLE_GAIN>(m_pFeedbackParameter->value());
    const CSAMPLE_GAIN size = static_cast<CSAMPLE_GAIN>(m_pSizeParameter->value());
    const CSAMPLE_GAIN damping = static_cast<CSAMPLE_GAIN>(m_pDampingParameter->value());
    const int mode = static_cast<int>(m_pModeParameter->value());
    const CSAMPLE_GAIN dryWet = static_cast<CSAMPLE_GAIN>(m_pDryWetParameter->value());

    const int safeMode = std::clamp(mode, 0, kNumModes - 1);

    const double presetFeedback = kPresets[safeMode].feedback;
    const double presetDamping = kPresets[safeMode].damping;
    const double effectiveFeedback = presetFeedback * feedback;
    const double effectiveDamping = presetDamping + (damping - 0.5) * 0.3;

    int size_param = static_cast<int>(size * 100);
    updateTapDelays(pState, engineParameters, size_param);

    const double damp_coeff = kDampingMin + effectiveDamping * (kDampingMax - kDampingMin);

    for (SINT i = 0; i < numSamples; ++i) {
        CSAMPLE inputSample = pInput[i];

        CSAMPLE delayedSum = 0.0f;
        for (int t = 0; t < kNumTaps; ++t) {
            int readPos = pState->write_position - pState->tap_delays[t];
            if (readPos < 0) {
                readPos += pState->delay_buf.size();
            }
            delayedSum += pState->delay_buf[readPos] * pState->tap_gains[t];
        }

        CSAMPLE dampedFeedback = delayedSum;
        pState->diff_state[0] = static_cast<CSAMPLE>(dampedFeedback * (1.0f - damp_coeff) +
                pState->diff_state[0] * damp_coeff);
        dampedFeedback = pState->diff_state[0];

        for (int d = 0; d < 4; ++d) {
            CSAMPLE in = dampedFeedback;
            CSAMPLE out = static_cast<CSAMPLE>(pState->diff_state[d + 1] - kDiffusionCoeff * in);
            pState->diff_state[d + 1] = static_cast<CSAMPLE>(in + kDiffusionCoeff * out);
            dampedFeedback = out;
        }

        CSAMPLE writeSample = static_cast<CSAMPLE>(
                inputSample + dampedFeedback * effectiveFeedback);
        pState->delay_buf[pState->write_position] = writeSample;
        pState->write_position = (pState->write_position + 1) % pState->delay_buf.size();

        CSAMPLE reverbOut = delayedSum * send;
        pOutput[i] = inputSample * (1.0f - dryWet) + reverbOut * dryWet;
    }

    pState->prev_send = send;
    pState->prev_feedback = feedback;
    pState->prev_size = size;
}
