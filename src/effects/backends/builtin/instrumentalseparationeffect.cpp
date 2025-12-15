#include "effects/backends/builtin/instrumentalseparationeffect.h"

#include "effects/backends/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/filters/enginefilterbiquad1.h"
#include "util/math.h"
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
          m_oldHighBoost(0.5) {
    m_tempBuffer = mixxx::SampleBuffer(engineParameters.samplesPerBuffer());
    
    // Create filters for instrumental enhancement
    m_pBassEnhancer = new EngineFilterBiquad1Peaking(
            engineParameters.sampleRate(), kBassFreq, kBoostQ);
    m_pMidCut = new EngineFilterBiquad1Peaking(
            engineParameters.sampleRate(), kMidFreq, kBoostQ);
    m_pHighEnhancer = new EngineFilterBiquad1Peaking(
            engineParameters.sampleRate(), kHighFreq, kBoostQ);
    m_pVocalNotch1 = new EngineFilterBiquad1Band(
            engineParameters.sampleRate(), kVocalNotch1Freq, kNotchQ);
    m_pVocalNotch2 = new EngineFilterBiquad1Band(
            engineParameters.sampleRate(), kVocalNotch2Freq, kNotchQ);
}

InstrumentalSeparationGroupState::~InstrumentalSeparationGroupState() {
    delete m_pBassEnhancer;
    delete m_pMidCut;
    delete m_pHighEnhancer;
    delete m_pVocalNotch1;
    delete m_pVocalNotch2;
}

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
        
        // Boost bass frequencies
        double bassGain = bassBoost * 12.0; // Up to +12dB
        m_pBassEnhancer->setFrequencyCorners(
                sampleRate, kBassFreq, kBoostQ, bassGain);
        
        // Cut mid frequencies (vocal range) based on intensity
        double midCut = -(intensity * 15.0); // Up to -15dB
        m_pMidCut->setFrequencyCorners(
                sampleRate, kMidFreq, kBoostQ, midCut);
        
        // Boost high frequencies
        double highGain = highBoost * 10.0; // Up to +10dB
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

    const CSAMPLE intensity = m_pIntensity ? m_pIntensity->value() : 0.0;
    const CSAMPLE bassBoost = m_pBassBoost ? m_pBassBoost->value() : 0.5;
    const CSAMPLE highBoost = m_pHighBoost ? m_pHighBoost->value() : 0.5;

    if (enableState == EffectEnableState::Disabling) {
        // Ramp to dry signal
        SampleUtil::copy(pOutput, pInput, engineParameters.samplesPerBuffer());
        return;
    }

    if (intensity <= 0.0) {
        // Pass through when no intensity
        SampleUtil::copy(pOutput, pInput, engineParameters.samplesPerBuffer());
        return;
    }

    // Update filter parameters
    pState->setFilters(engineParameters.sampleRate(), intensity, bassBoost, highBoost);

    // Copy input to output first
    SampleUtil::copy(pOutput, pInput, engineParameters.samplesPerBuffer());

    // Enhance stereo width for instrumental elements (opposite of vocal)
    // Instrumentals typically have wider stereo spread
    for (SINT i = 0; i < engineParameters.samplesPerBuffer(); i += 2) {
        CSAMPLE left = pOutput[i];
        CSAMPLE right = pOutput[i + 1];
        
        // Extract mid and side components
        CSAMPLE mid = (left + right) * 0.5;
        CSAMPLE side = (left - right) * 0.5;
        
        // Enhance side component slightly (up to 50% wider)
        side *= (1.0 + intensity * 0.5);
        
        pOutput[i] = mid + side;
        pOutput[i + 1] = mid - side;
    }

    // Apply bass enhancement
    if (bassBoost > 0.0) {
        pState->m_pBassEnhancer->process(pOutput, pOutput, engineParameters.samplesPerBuffer());
    }
    
    // Apply mid-range cut (vocal suppression)
    pState->m_pMidCut->process(pOutput, pOutput, engineParameters.samplesPerBuffer());
    
    // Apply high frequency enhancement
    if (highBoost > 0.0) {
        pState->m_pHighEnhancer->process(pOutput, pOutput, engineParameters.samplesPerBuffer());
    }
    
    // Apply vocal notch filters with intensity scaling
    if (intensity > 0.3) {
        // Create notched version in temp buffer
        SampleUtil::copy(pState->m_tempBuffer.data(), pOutput, engineParameters.samplesPerBuffer());
        
        pState->m_pVocalNotch1->process(
                pState->m_tempBuffer.data(), 
                pState->m_tempBuffer.data(), 
                engineParameters.samplesPerBuffer());
        pState->m_pVocalNotch2->process(
                pState->m_tempBuffer.data(), 
                pState->m_tempBuffer.data(), 
                engineParameters.samplesPerBuffer());
        
        // Mix notched signal based on intensity
        double notchMix = (intensity - 0.3) / 0.7; // Scale from 0.3-1.0 to 0-1
        for (SINT i = 0; i < engineParameters.samplesPerBuffer(); ++i) {
            pOutput[i] = pOutput[i] * (1.0 - notchMix) + 
                         pState->m_tempBuffer[i] * notchMix;
        }
    }

    // Apply intensity as wet/dry mix
    const CSAMPLE wet = intensity;
    const CSAMPLE dry = 1.0 - intensity;
    
    for (SINT i = 0; i < engineParameters.samplesPerBuffer(); ++i) {
        pOutput[i] = (pOutput[i] * wet) + (pInput[i] * dry);
    }
}
