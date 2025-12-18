#include "effects/backends/builtin/instrumentalseparationeffect.h"

#include "effects/backends/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/filters/enginefilterbiquad1.h"
#include "util/math.h"
#include "util/rampingvalue.h"
#include "util/sample.h"

namespace {
// Instrumental frequency emphasis - bass and highs, cut mids (vocals)
constexpr double kBassFreq = 80.0;      // Hz
constexpr double kMidFreq = 1200.0;     // Hz - vocal range to cut
constexpr double kHighFreq = 8000.0;    // Hz
constexpr double kVocalNotch1Freq = 800.0;   // Hz
constexpr double kVocalNotch2Freq = 2500.0;  // Hz
constexpr double kNotchQ = 2.0; // Narrow notch for vocal suppression
constexpr double kBoostQ = 1.0; // Wider boost for instruments
} // anonymous namespace

// static
QString InstrumentalSeparationEffect::getId() {
    return "org.mixxx.effects.instrumentalseparation";
}

// static
EffectManifestPointer InstrumentalSeparationEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Instrumental Separation"));
    pManifest->setShortName(QObject::tr("Instrumental"));
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription(QObject::tr(
            "Isolates instrumental elements by emphasizing bass and high frequencies "
            "while attenuating vocal-range mid frequencies. "
            "Ideal for creating instrumental versions or karaoke tracks."));
    pManifest->setEffectRampsFromDry(true);
    pManifest->setMetaknobDefault(0.0);

    EffectManifestParameterPointer intensity = pManifest->addParameter();
    intensity->setId("intensity");
    intensity->setName(QObject::tr("Separation Intensity"));
    intensity->setShortName(QObject::tr("Intensity"));
    intensity->setDescription(QObject::tr(
            "Controls the strength of instrumental isolation. "
            "Higher values reduce vocals more while enhancing instruments."));
    intensity->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    intensity->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    intensity->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    intensity->setNeutralPointOnScale(0.0);
    intensity->setRange(0.0, 0.0, 1.0);

    EffectManifestParameterPointer bassBoost = pManifest->addParameter();
    bassBoost->setId("bassboost");
    bassBoost->setName(QObject::tr("Bass Emphasis"));
    bassBoost->setShortName(QObject::tr("Bass"));
    bassBoost->setDescription(QObject::tr(
            "Enhances low-frequency instruments like bass guitar and kick drum."));
    bassBoost->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    bassBoost->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    bassBoost->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    bassBoost->setNeutralPointOnScale(0.5);
    bassBoost->setRange(0.0, 0.5, 1.0);

    EffectManifestParameterPointer highBoost = pManifest->addParameter();
    highBoost->setId("highboost");
    highBoost->setName(QObject::tr("High Emphasis"));
    highBoost->setShortName(QObject::tr("Highs"));
    highBoost->setDescription(QObject::tr(
            "Enhances high-frequency instruments like cymbals, hi-hats, and synths."));
    highBoost->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    highBoost->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    highBoost->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    highBoost->setNeutralPointOnScale(0.5);
    highBoost->setRange(0.0, 0.5, 1.0);

    return pManifest;
}

InstrumentalSeparationGroupState::InstrumentalSeparationGroupState(
        const mixxx::EngineParameters& engineParameters)
        : EffectState(engineParameters),
          m_oldIntensity(0.0),
          m_oldBassBoost(0.5),
          m_oldHighBoost(0.5),
          m_previousIntensity(0.0) {
    m_tempBuffer = mixxx::SampleBuffer(engineParameters.samplesPerBuffer());
    
    // Create filters for instrumental enhancement
    m_pBassEnhancer = std::make_unique<EngineFilterBiquad1Peaking>(
            engineParameters.sampleRate(), kBassFreq, kBoostQ);
    m_pMidCut = std::make_unique<EngineFilterBiquad1Peaking>(
            engineParameters.sampleRate(), kMidFreq, kBoostQ);
    m_pHighEnhancer = std::make_unique<EngineFilterBiquad1Peaking>(
            engineParameters.sampleRate(), kHighFreq, kBoostQ);
    m_pVocalNotch1 = std::make_unique<EngineFilterBiquad1Band>(
            engineParameters.sampleRate(), kVocalNotch1Freq, kNotchQ);
    m_pVocalNotch2 = std::make_unique<EngineFilterBiquad1Band>(
            engineParameters.sampleRate(), kVocalNotch2Freq, kNotchQ);
}

InstrumentalSeparationGroupState::~InstrumentalSeparationGroupState() = default;

void InstrumentalSeparationEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pIntensity = parameters.value("intensity");
    m_pBassBoost = parameters.value("bassboost");
    m_pHighBoost = parameters.value("highboost");
}

void InstrumentalSeparationGroupState::setFilters(
        mixxx::audio::SampleRate sampleRate,
        double intensity,
        double bassBoost,
        double highBoost) {
    // Update filters only when parameters change significantly
    if (std::abs(intensity - m_oldIntensity) > 0.01 ||
        std::abs(bassBoost - m_oldBassBoost) > 0.05 ||
        std::abs(highBoost - m_oldHighBoost) > 0.05) {
        
        // Scale all filter gains by intensity to avoid wet/dry mixing
        // At intensity=0, all filters have 0dB gain (transparent)
        // At intensity=1, filters have full effect
        
        // Boost bass frequencies
        double bassGain = intensity * bassBoost * 12.0; // Up to +12dB
        m_pBassEnhancer->setFrequencyCorners(
                sampleRate, kBassFreq, kBoostQ, bassGain);
        
        // Cut mid frequencies (vocal range) based on intensity
        double midCut = -(intensity * intensity * 15.0); // Up to -15dB, squared for smoother ramp
        m_pMidCut->setFrequencyCorners(
                sampleRate, kMidFreq, kBoostQ, midCut);
        
        // Boost high frequencies
        double highGain = intensity * highBoost * 10.0; // Up to +10dB
        m_pHighEnhancer->setFrequencyCorners(
                sampleRate, kHighFreq, kBoostQ, highGain);
        
        // Set vocal notch filters (inverse gain for notch)
        m_pVocalNotch1->setFrequencyCorners(
                sampleRate, kVocalNotch1Freq, kNotchQ);
        m_pVocalNotch2->setFrequencyCorners(
                sampleRate, kVocalNotch2Freq, kNotchQ);
        
        m_oldIntensity = intensity;
        m_oldBassBoost = bassBoost;
        m_oldHighBoost = highBoost;
    }
}

void InstrumentalSeparationEffect::processChannel(
        InstrumentalSeparationGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(groupFeatures);

    CSAMPLE intensity = m_pIntensity ? static_cast<CSAMPLE>(m_pIntensity->value()) : 0.0f;
    const CSAMPLE bassBoost = m_pBassBoost ? static_cast<CSAMPLE>(m_pBassBoost->value()) : 0.5f;
    const CSAMPLE highBoost = m_pHighBoost ? static_cast<CSAMPLE>(m_pHighBoost->value()) : 0.5f;

    if (enableState == EffectEnableState::Disabling) {
        // Ramp to dry signal by setting intensity to 0
        // This allows the wet/dry mix to crossfade smoothly
        intensity = 0.0f;
    }

    // Ramp intensity from previous value to avoid discontinuities
    RampingValue<CSAMPLE_GAIN> intensityRamped(
            intensity, pState->m_previousIntensity, engineParameters.framesPerBuffer());

    // Update filter parameters
    pState->setFilters(engineParameters.sampleRate(), intensity, bassBoost, highBoost);

    // Process signal through filters to maintain consistent group delay
    // even when intensity is 0. This prevents clicks from phase misalignment.
    SampleUtil::copy(pState->m_tempBuffer.data(), pInput, engineParameters.samplesPerBuffer());

    // Enhance stereo width for instrumental elements (opposite of vocal)
    // Instrumentals typically have wider stereo spread
    for (SINT i = 0; i < engineParameters.samplesPerBuffer(); i += 2) {
        CSAMPLE_GAIN intensity_ramped = intensityRamped.getNth(i / 2);
        
        CSAMPLE left = pState->m_tempBuffer[i];
        CSAMPLE right = pState->m_tempBuffer[i + 1];
        
        // Extract mid and side components
        CSAMPLE mid = (left + right) * 0.5f;
        CSAMPLE side = (left - right) * 0.5f;
        
        // Enhance side component slightly (up to 50% wider)
        side *= (1.0f + intensity_ramped * 0.5f);
        
        pState->m_tempBuffer[i] = mid + side;
        pState->m_tempBuffer[i + 1] = mid - side;
    }

    // Apply bass enhancement
    pState->m_pBassEnhancer->process(
            pState->m_tempBuffer.data(),
            pState->m_tempBuffer.data(),
            engineParameters.samplesPerBuffer());
    
    // Apply mid-range cut (vocal suppression)
    pState->m_pMidCut->process(
            pState->m_tempBuffer.data(),
            pState->m_tempBuffer.data(),
            engineParameters.samplesPerBuffer());
    
    // Apply high frequency enhancement
    pState->m_pHighEnhancer->process(
            pState->m_tempBuffer.data(),
            pState->m_tempBuffer.data(),
            engineParameters.samplesPerBuffer());
    
    // Apply vocal notch filters (always process to maintain group delay)
    pState->m_pVocalNotch1->process(
            pState->m_tempBuffer.data(),
            pState->m_tempBuffer.data(),
            engineParameters.samplesPerBuffer());
    pState->m_pVocalNotch2->process(
            pState->m_tempBuffer.data(),
            pState->m_tempBuffer.data(),
            engineParameters.samplesPerBuffer());

    // Output the processed signal
    // No wet/dry mixing here to avoid comb filtering from group delay
    // The effect rack's dry/wet knob handles the final mix
    SampleUtil::copy(pOutput, pState->m_tempBuffer.data(), engineParameters.samplesPerBuffer());
    
    // Update previous intensity for next callback
    if (enableState == EffectEnableState::Disabling) {
        pState->m_previousIntensity = 0.0;
    } else {
        pState->m_previousIntensity = intensity;
    }
}
