#include "effects/backends/builtin/procompressoreffect.h"

#include <algorithm>
#include <cmath>

#include "effects/backends/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

namespace {

float db2ratio(float db) {
    return std::pow(10.0f, db / 20.0f);
}

float ratio2db(float ratio) {
    return 20.0f * std::log10(ratio);
}

} // namespace

QString ProCompressorEffect::getId() {
    return QStringLiteral("org.mixxx.effects.procompressor");
}

EffectManifestPointer ProCompressorEffect::getManifest() {
    auto pManifest = EffectManifestPointer::create();
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Pro Compressor"));
    pManifest->setAuthor("DJ Sugar");
    pManifest->setVersion("1.0");
    pManifest->setDescription(QObject::tr(
            "Professional DJ compressor for smooth mixing. "
            "Reduces dynamic range by compressing signals above a threshold. "
            "Comparable to Rekordbox's Compressor effect."));

    auto pThreshold = pManifest->addParameter();
    pThreshold->setId("threshold");
    pThreshold->setName(QObject::tr("Threshold"));
    pThreshold->setShortName(QObject::tr("Thresh"));
    pThreshold->setDescription(
            QObject::tr("Level above which compression begins.\n"
                        "Lower values = more compression."));
    pThreshold->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    pThreshold->setUnitsHint(EffectManifestParameter::UnitsHint::Decibel);
    pThreshold->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    pThreshold->setRange(-30.0, -12.0, 0.0);

    auto pRatio = pManifest->addParameter();
    pRatio->setId("ratio");
    pRatio->setName(QObject::tr("Ratio"));
    pRatio->setShortName(QObject::tr("Ratio"));
    pRatio->setDescription(
            QObject::tr("Amount of compression applied.\n"
                        "1:1 = no compression, 20:1 = extreme limiting."));
    pRatio->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    pRatio->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    pRatio->setRange(1.0, 4.0, 20.0);

    auto pAttack = pManifest->addParameter();
    pAttack->setId("attack");
    pAttack->setName(QObject::tr("Attack"));
    pAttack->setShortName(QObject::tr("Attack"));
    pAttack->setDescription(
            QObject::tr("How quickly the compressor engages.\n"
                        "Fast = punchy, Slow = smooth."));
    pAttack->setValueScaler(EffectManifestParameter::ValueScaler::Logarithmic);
    pAttack->setUnitsHint(EffectManifestParameter::UnitsHint::Millisecond);
    pAttack->setRange(1.0, 10.0, 100.0);

    auto pRelease = pManifest->addParameter();
    pRelease->setId("release");
    pRelease->setName(QObject::tr("Release"));
    pRelease->setShortName(QObject::tr("Release"));
    pRelease->setDescription(
            QObject::tr("How quickly the compressor releases.\n"
                        "Fast = pumping, Slow = smooth."));
    pRelease->setValueScaler(EffectManifestParameter::ValueScaler::Logarithmic);
    pRelease->setUnitsHint(EffectManifestParameter::UnitsHint::Millisecond);
    pRelease->setRange(10.0, 100.0, 1000.0);

    auto pMakeup = pManifest->addParameter();
    pMakeup->setId("makeup");
    pMakeup->setName(QObject::tr("Makeup"));
    pMakeup->setShortName(QObject::tr("Makeup"));
    pMakeup->setDescription(QObject::tr(
            "Gain added after compression to compensate for level reduction."));
    pMakeup->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    pMakeup->setUnitsHint(EffectManifestParameter::UnitsHint::Decibel);
    pMakeup->setRange(0.0, 6.0, 24.0);

    auto pDryWet = pManifest->addParameter();
    pDryWet->setId("drywet");
    pDryWet->setName(QObject::tr("Dry/Wet"));
    pDryWet->setShortName(QObject::tr("D/W"));
    pDryWet->setDescription(QObject::tr(
            "Mix between dry (uncompressed) and wet (compressed) signal."));
    pDryWet->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    pDryWet->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    pDryWet->setRange(0.0, 1.0, 1.0);

    return pManifest;
}

void ProCompressorEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pThresholdParameter = parameters.value("threshold");
    m_pRatioParameter = parameters.value("ratio");
    m_pAttackParameter = parameters.value("attack");
    m_pReleaseParameter = parameters.value("release");
    m_pMakeupParameter = parameters.value("makeup");
    m_pDryWetParameter = parameters.value("drywet");
}

void ProCompressorEffect::processChannel(ProCompressorGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        [[maybe_unused]] const EffectEnableState enableState,
        [[maybe_unused]] const GroupFeatureState& groupFeatures) {
    const SINT numSamples = engineParameters.samplesPerBuffer();
    const float sampleRate = static_cast<float>(engineParameters.sampleRate());

    float threshold = static_cast<float>(m_pThresholdParameter->value());
    float ratio = static_cast<float>(m_pRatioParameter->value());
    float attack = static_cast<float>(m_pAttackParameter->value());
    float release = static_cast<float>(m_pReleaseParameter->value());
    float makeup = static_cast<float>(m_pMakeupParameter->value());
    float dryWet = static_cast<float>(m_pDryWetParameter->value());

    // Attack/release coefficients
    float attackCoeff = std::exp(-1.0f / (attack * 0.001f * sampleRate));
    float releaseCoeff = std::exp(-1.0f / (release * 0.001f * sampleRate));

    // Makeup gain linear
    float makeupLin = db2ratio(makeup);

    // Smooth parameter changes
    float smoothThreshold = pState->prev_threshold +
            0.001f * (threshold - pState->prev_threshold);
    float smoothRatio =
            pState->prev_ratio + 0.001f * (ratio - pState->prev_ratio);
    pState->prev_threshold = smoothThreshold;
    pState->prev_ratio = smoothRatio;

    float thresholdSmoothLin = db2ratio(smoothThreshold);

    for (SINT i = 0; i < numSamples; ++i) {
        // Compute input level (peak detection)
        float inputAbs = std::abs(pInput[i]);

        // Envelope follower
        float targetEnv = inputAbs;
        if (targetEnv > pState->envelope) {
            pState->envelope = attackCoeff * pState->envelope +
                    (1.0f - attackCoeff) * targetEnv;
        } else {
            pState->envelope = releaseCoeff * pState->envelope +
                    (1.0f - releaseCoeff) * targetEnv;
        }

        // Gain computer
        float gain = 1.0f;
        if (pState->envelope > thresholdSmoothLin) {
            float envDB = ratio2db(pState->envelope);
            float compressedDB =
                    smoothThreshold + (envDB - smoothThreshold) / smoothRatio;
            gain = db2ratio(compressedDB - envDB);
        }

        // Smooth gain
        pState->prev_gain =
                pState->prev_gain + 0.0001f * (gain - pState->prev_gain);

        // Apply compression with makeup gain and dry/wet mix
        float wet = pInput[i] * pState->prev_gain * makeupLin;
        float dry = pInput[i];
        pOutput[i] = dry * (1.0f - dryWet) + wet * dryWet;
    }
}
