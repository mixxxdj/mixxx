#include "effects/backends/builtin/vintagedelayeffect.h"

#include <algorithm>
#include <cmath>

#include "effects/backends/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"

QString VintageDelayEffect::getId() {
    return QStringLiteral("org.mixxx.effects.vintagedelay");
}

EffectManifestPointer VintageDelayEffect::getManifest() {
    auto pManifest = EffectManifestPointer::create();
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Vintage Delay"));
    pManifest->setAuthor("DJ Sugar");
    pManifest->setVersion("1.0");
    pManifest->setDescription(
            QObject::tr("Tape-style delay with warm, dark character. "
                        "Low-pass filtering in the feedback loop simulates "
                        "analog tape echo. "
                        "Comparable to classic Roland RE-201 Space Echo."));

    auto pTime = pManifest->addParameter();
    pTime->setId("time");
    pTime->setName(QObject::tr("Time"));
    pTime->setShortName(QObject::tr("Time"));
    pTime->setDescription(QObject::tr("Delay time in milliseconds."));
    pTime->setValueScaler(EffectManifestParameter::ValueScaler::Logarithmic);
    pTime->setUnitsHint(EffectManifestParameter::UnitsHint::Millisecond);
    pTime->setRange(50.0, 400.0, 2000.0);

    auto pFeedback = pManifest->addParameter();
    pFeedback->setId("feedback");
    pFeedback->setName(QObject::tr("Feedback"));
    pFeedback->setShortName(QObject::tr("Fdbk"));
    pFeedback->setDescription(
            QObject::tr("Amount of output fed back into input.\n"
                        "Higher = more repeats."));
    pFeedback->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    pFeedback->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    pFeedback->setRange(0.0, 0.4, 0.9);

    auto pTone = pManifest->addParameter();
    pTone->setId("tone");
    pTone->setName(QObject::tr("Tone"));
    pTone->setShortName(QObject::tr("Tone"));
    pTone->setDescription(
            QObject::tr("Brightness of the delay repeats.\n"
                        "0 = dark/warm, 1 = bright/clear."));
    pTone->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    pTone->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    pTone->setRange(0.0, 0.5, 1.0);

    auto pMix = pManifest->addParameter();
    pMix->setId("mix");
    pMix->setName(QObject::tr("Mix"));
    pMix->setShortName(QObject::tr("Mix"));
    pMix->setDescription(
            QObject::tr("Dry/wet mix.\n"
                        "0 = dry only, 1 = wet only."));
    pMix->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    pMix->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    pMix->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    pMix->setRange(0.0, 0.3, 1.0);

    return pManifest;
}

void VintageDelayEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pTimeParameter = parameters.value("time");
    m_pFeedbackParameter = parameters.value("feedback");
    m_pToneParameter = parameters.value("tone");
    m_pMixParameter = parameters.value("mix");
}

void VintageDelayEffect::processChannel(VintageDelayGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        [[maybe_unused]] const EffectEnableState enableState,
        [[maybe_unused]] const GroupFeatureState& groupFeatures) {
    const SINT numSamples = engineParameters.samplesPerBuffer();
    const int chCount = engineParameters.channelCount();
    const int sampleRate = engineParameters.sampleRate();

    float time = m_pTimeParameter->value();
    float feedback = m_pFeedbackParameter->value();
    float tone = m_pToneParameter->value();
    float mix = m_pMixParameter->value();

    // Compute delay in samples
    int delaySamples = static_cast<int>(time * sampleRate * 0.001f);

    // Smooth parameters
    float smoothFeedback =
            pState->prev_feedback + 0.001f * (feedback - pState->prev_feedback);
    float smoothTone = pState->prev_tone + 0.001f * (tone - pState->prev_tone);
    float smoothMix = pState->prev_mix + 0.001f * (mix - pState->prev_mix);
    pState->prev_feedback = smoothFeedback;
    pState->prev_tone = smoothTone;
    pState->prev_mix = smoothMix;

    // Recompute alpha based on smooth tone
    float smoothCutoff = 800.0f + smoothTone * 12000.0f;
    float smoothRc = 1.0f / (2.0f * 3.14159265f * smoothCutoff);
    float smoothAlpha = dt / (smoothRc + dt);

    for (SINT i = 0; i < numSamples; ++i) {
        int ch = i % chCount;

        // Read from delay buffer with linear interpolation
        float readPos =
                static_cast<float>(pState->write_position) - delaySamples;
        while (readPos < 0.0f) {
            readPos += pState->delay_buf.size();
        }

        int readPosInt = static_cast<int>(readPos);
        float frac = readPos - readPosInt;
        int readPos2 = (readPosInt + 1) % pState->delay_buf.size();

        CSAMPLE delayed = pState->delay_buf[readPosInt] * (1.0f - frac) +
                pState->delay_buf[readPos2] * frac;

        // Apply low-pass filter (tape character)
        pState->lp_state[ch] = pState->lp_state[ch] +
                smoothAlpha * (delayed - pState->lp_state[ch]);
        CSAMPLE filtered = pState->lp_state[ch];

        // Write to delay buffer with feedback
        pState->delay_buf[pState->write_position] =
                pInput[i] + filtered * smoothFeedback;

        // Advance write position
        pState->write_position =
                (pState->write_position + 1) % pState->delay_buf.size();

        // Output with dry/wet mix
        pOutput[i] = pInput[i] * (1.0f - smoothMix) + filtered * smoothMix;
    }
}
