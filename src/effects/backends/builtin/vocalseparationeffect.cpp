#include "effects/backends/builtin/vocalseparationeffect.h"

#include "effects/backends/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/filters/enginefilterbiquad1.h"
#include "util/math.h"
#include "util/sample.h"

namespace {
// Vocal frequency range typically 80Hz - 12kHz, with emphasis on 300Hz - 3.5kHz
constexpr double kMinVocalFreq = 80.0;     // Hz
constexpr double kMaxVocalFreq = 12000.0;  // Hz
constexpr double kDefaultCenterFreq = 1000.0; // Hz - centered on vocal range
constexpr double kVocalQ = 1.5; // Q factor for vocal emphasis
} // anonymous namespace

// static
QString VocalSeparationEffect::getId() {
    return "org.mixxx.effects.vocalseparation";
}

// static
EffectManifestPointer VocalSeparationEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Vocal Separation"));
    pManifest->setShortName(QObject::tr("Vocal"));
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription(QObject::tr(
            "Isolates and emphasizes vocal frequencies in the mix. "
            "Uses multi-band filtering to enhance vocal presence while "
            "attenuating instrumental elements."));
    pManifest->setEffectRampsFromDry(true);
    pManifest->setMetaknobDefault(0.0);

    EffectManifestParameterPointer intensity = pManifest->addParameter();
    intensity->setId("intensity");
    intensity->setName(QObject::tr("Vocal Intensity"));
    intensity->setShortName(QObject::tr("Intensity"));
    intensity->setDescription(QObject::tr(
            "Controls the strength of vocal isolation. "
            "Higher values emphasize vocals more while reducing instrumentals."));
    intensity->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    intensity->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    intensity->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    intensity->setNeutralPointOnScale(0.0);
    intensity->setRange(0.0, 0.0, 1.0);

    EffectManifestParameterPointer centerFreq = pManifest->addParameter();
    centerFreq->setId("centerfreq");
    centerFreq->setName(QObject::tr("Vocal Center Frequency"));
    centerFreq->setShortName(QObject::tr("Center"));
    centerFreq->setDescription(QObject::tr(
            "Adjusts the center frequency of vocal isolation. "
            "Lower values for deeper voices, higher for brighter vocals."));
    centerFreq->setValueScaler(EffectManifestParameter::ValueScaler::Logarithmic);
    centerFreq->setUnitsHint(EffectManifestParameter::UnitsHint::Hertz);
    centerFreq->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    centerFreq->setNeutralPointOnScale(0.5);
    centerFreq->setRange(kMinVocalFreq, kDefaultCenterFreq, kMaxVocalFreq);

    EffectManifestParameterPointer stereoWidth = pManifest->addParameter();
    stereoWidth->setId("stereowidth");
    stereoWidth->setName(QObject::tr("Stereo Width"));
    stereoWidth->setShortName(QObject::tr("Width"));
    stereoWidth->setDescription(QObject::tr(
            "Adjusts stereo width. Center vocals are typically mono. "
            "Lower values narrow the stereo field to isolate center vocals."));
    stereoWidth->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    stereoWidth->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    stereoWidth->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    stereoWidth->setNeutralPointOnScale(0.5);
    stereoWidth->setRange(0.0, 0.5, 1.0);

    return pManifest;
}

VocalSeparationGroupState::VocalSeparationGroupState(
        const mixxx::EngineParameters& engineParameters)
        : EffectState(engineParameters),
          m_oldIntensity(0.0),
          m_oldCenterFreq(kDefaultCenterFreq) {
    m_tempBuffer = mixxx::SampleBuffer(engineParameters.samplesPerBuffer());
    
    // Create filters for vocal enhancement
    m_pVocalEnhancer1 = new EngineFilterBiquad1Peaking(
            engineParameters.sampleRate(), 800.0, kVocalQ);
    m_pVocalEnhancer2 = new EngineFilterBiquad1Peaking(
            engineParameters.sampleRate(), 2500.0, kVocalQ);
    m_pHighPass = new EngineFilterBiquad1High(
            engineParameters.sampleRate(), kMinVocalFreq, 0.707);
    m_pLowPass = new EngineFilterBiquad1Low(
            engineParameters.sampleRate(), kMaxVocalFreq, 0.707);
}

VocalSeparationGroupState::~VocalSeparationGroupState() {
    delete m_pVocalEnhancer1;
    delete m_pVocalEnhancer2;
    delete m_pHighPass;
    delete m_pLowPass;
}

void VocalSeparationEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pIntensity = parameters.value("intensity");
    m_pCenterFreq = parameters.value("centerfreq");
    m_pStereoWidth = parameters.value("stereowidth");
}

void VocalSeparationGroupState::setFilters(
        mixxx::audio::SampleRate sampleRate,
        double intensity,
        double centerFreq) {
    // Update filters only when parameters change significantly
    if (std::abs(intensity - m_oldIntensity) > 0.01 ||
        std::abs(centerFreq - m_oldCenterFreq) > 10.0) {
        
        m_pVocalEnhancer1->setFrequencyCorners(
                sampleRate, centerFreq * 0.8, kVocalQ, intensity * 6.0);
        m_pVocalEnhancer2->setFrequencyCorners(
                sampleRate, centerFreq * 2.5, kVocalQ, intensity * 4.0);
        m_pHighPass->setFrequencyCorners(
                sampleRate, kMinVocalFreq + (intensity * 120.0), 0.707);
        m_pLowPass->setFrequencyCorners(
                sampleRate, kMaxVocalFreq - (intensity * 3000.0), 0.707);
        
        m_oldIntensity = intensity;
        m_oldCenterFreq = centerFreq;
    }
}

void VocalSeparationEffect::processChannel(
        VocalSeparationGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(groupFeatures);

    const CSAMPLE intensity = m_pIntensity ? m_pIntensity->value() : 0.0;
    const CSAMPLE centerFreq = m_pCenterFreq ? m_pCenterFreq->value() : kDefaultCenterFreq;
    const CSAMPLE stereoWidth = m_pStereoWidth ? m_pStereoWidth->value() : 0.5;

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
    pState->setFilters(engineParameters.sampleRate(), intensity, centerFreq);

    // Copy input to output first
    SampleUtil::copy(pOutput, pInput, engineParameters.samplesPerBuffer());

    // Apply stereo width reduction for center vocal isolation
    if (stereoWidth < 1.0) {
        for (SINT i = 0; i < engineParameters.samplesPerBuffer(); i += 2) {
            CSAMPLE left = pOutput[i];
            CSAMPLE right = pOutput[i + 1];
            
            // Extract mid (mono/center) and side (stereo) components
            CSAMPLE mid = (left + right) * 0.5;
            CSAMPLE side = (left - right) * 0.5;
            
            // Reduce side component based on stereoWidth
            side *= stereoWidth * 2.0; // Scale to 0-2 range
            
            pOutput[i] = mid + side;
            pOutput[i + 1] = mid - side;
        }
    }

    // Apply high-pass filter
    pState->m_pHighPass->process(pOutput, pOutput, engineParameters.samplesPerBuffer());
    
    // Apply low-pass filter
    pState->m_pLowPass->process(pOutput, pOutput, engineParameters.samplesPerBuffer());
    
    // Apply vocal enhancement peaking filters
    pState->m_pVocalEnhancer1->process(pOutput, pOutput, engineParameters.samplesPerBuffer());
    pState->m_pVocalEnhancer2->process(pOutput, pOutput, engineParameters.samplesPerBuffer());

    // Apply intensity as wet/dry mix
    const CSAMPLE wet = intensity;
    const CSAMPLE dry = 1.0 - intensity;
    
    for (SINT i = 0; i < engineParameters.samplesPerBuffer(); ++i) {
        pOutput[i] = (pOutput[i] * wet) + (pInput[i] * dry);
    }
}
