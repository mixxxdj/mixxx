#include "effects/backends/builtin/proflangereffect.h"

#include <algorithm>
#include <cmath>

#include "effects/backends/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

QString ProFlangerEffect::getId() {
    return QStringLiteral("org.mixxx.effects.proflanger");
}

EffectManifestPointer ProFlangerEffect::getManifest() {
    auto pManifest = EffectManifestPointer::create();
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Pro Flanger"));
    pManifest->setAuthor("DJ Sugar");
    pManifest->setVersion("1.0");
    pManifest->setDescription(QObject::tr(
            "Professional DJ flanger with LFO-modulated delay. "
            "Creates the classic 'sweeping jet' effect. "
            "Comparable to Rekordbox's Flanger effect."));

    auto pRate = pManifest->addParameter();
    pRate->setId("rate");
    pRate->setName(QObject::tr("Rate"));
    pRate->setShortName(QObject::tr("Rate"));
    pRate->setDescription(QObject::tr(
            "Speed of the LFO modulation.\n"
            "Slow = gentle sweep, Fast = intense flutter."));
    pRate->setValueScaler(EffectManifestParameter::ValueScaler::Logarithmic);
    pRate->setUnitsHint(EffectManifestParameter::UnitsHint::Hertz);
    pRate->setRange(0.1, 1.0, 10.0);

    auto pDepth = pManifest->addParameter();
    pDepth->setId("depth");
    pDepth->setName(QObject::tr("Depth"));
    pDepth->setShortName(QObject::tr("Depth"));
    pDepth->setDescription(QObject::tr(
            "Amount of delay modulation.\n"
            "Higher = more pronounced sweeping effect."));
    pDepth->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    pDepth->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    pDepth->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    pDepth->setRange(0.0, 0.5, 1.0);

    auto pFeedback = pManifest->addParameter();
    pFeedback->setId("feedback");
    pFeedback->setName(QObject::tr("Feedback"));
    pFeedback->setShortName(QObject::tr("Fdbk"));
    pFeedback->setDescription(QObject::tr(
            "Amount of output fed back into input.\n"
            "Higher = more intense/resonant effect."));
    pFeedback->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    pFeedback->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    pFeedback->setRange(0.0, 0.3, 0.9);

    auto pStereo = pManifest->addParameter();
    pStereo->setId("stereo");
    pStereo->setName(QObject::tr("Stereo"));
    pStereo->setShortName(QObject::tr("Stereo"));
    pStereo->setDescription(QObject::tr(
            "Stereo mode: on = inverted LFO on right channel for wide stereo effect."));
    pStereo->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    pStereo->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    pStereo->setRange(0.0, 1.0, 1.0);

    return pManifest;
}

void ProFlangerEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pRateParameter = parameters.value("rate");
    m_pDepthParameter = parameters.value("depth");
    m_pFeedbackParameter = parameters.value("feedback");
    m_pStereoParameter = parameters.value("stereo");
}

void ProFlangerEffect::processChannel(
        FlangerGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        [[maybe_unused]] const EffectEnableState enableState,
        [[maybe_unused]] const GroupFeatureState& groupFeatures) {
    const SINT numSamples = engineParameters.samplesPerBuffer();
    const int chCount = engineParameters.channelCount();
    const float sampleRate = static_cast<float>(engineParameters.sampleRate());

    float rate = m_pRateParameter->value();
    float depth = m_pDepthParameter->value();
    float feedback = m_pFeedbackParameter->value();
    bool stereo = m_pStereoParameter->value() > 0.5f;

    // Smooth parameters
    float smoothRate = pState->prev_rate + 0.01f * (rate - pState->prev_rate);
    float smoothDepth = pState->prev_depth + 0.01f * (depth - pState->prev_depth);
    float smoothFeedback = pState->prev_feedback + 0.01f * (feedback - pState->prev_feedback);
    pState->prev_rate = smoothRate;
    pState->prev_depth = smoothDepth;
    pState->prev_feedback = smoothFeedback;

    // LFO phase increment
    float phaseInc = (2.0f * M_PI * smoothRate) / sampleRate;

    // Base delay in samples (0.5ms to 5ms range for flanger)
    float baseDelayMs = 1.0f;
    float modDepthMs = 4.0f * smoothDepth;
    float baseDelaySamples = baseDelayMs * sampleRate * 0.001f;

    for (SINT i = 0; i < numSamples; ++i) {
        int ch = i % chCount;

        // Write to delay buffer
        pState->delay_buf[pState->write_position] = pInput[i];

        // Compute LFO value (sine wave)
        float lfo;
        if (stereo && ch == 1) {
            // Inverted LFO for stereo channel
            lfo = -std::sin(pState->lfo_phase);
        } else {
            lfo = std::sin(pState->lfo_phase);
        }

        // Compute modulated delay
        float delaySamples = baseDelaySamples +
                modDepthMs * sampleRate * 0.001f * (lfo * 0.5f + 0.5f);

        // Read from delay buffer with linear interpolation
        float readPos = pState->write_position - delaySamples;
        while (readPos < 0) {
            readPos += kMaxDelaySamples;
        }

        int readPosInt = static_cast<int>(readPos);
        float frac = readPos - readPosInt;
        int readPos2 = (readPosInt + 1) % kMaxDelaySamples;

        CSAMPLE delayed = pState->delay_buf[readPosInt] * (1.0f - frac) +
                pState->delay_buf[readPos2] * frac;

        // Mix with feedback
        float output = pInput[i] + delayed * smoothFeedback;

        // Write back to delay buffer with feedback
        pState->delay_buf[pState->write_position] = pInput[i] + delayed * smoothFeedback;

        // Advance write position
        pState->write_position = (pState->write_position + 1) % kMaxDelaySamples;

        // Advance LFO phase
        pState->lfo_phase += phaseInc;
        while (pState->lfo_phase > 2.0f * M_PI) {
            pState->lfo_phase -= 2.0f * M_PI;
        }

        // Output with dry/wet mix controlled by depth
        pOutput[i] = pInput[i] * (1.0f - smoothDepth * 0.5f) + output * (smoothDepth * 0.5f);
    }
}
