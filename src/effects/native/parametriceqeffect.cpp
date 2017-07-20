#include "effects/native/parametriceqeffect.h"
#include "util/math.h"

namespace {
    int kBandCount = 2;
}

// static
QString ParametricEQEffect::getId() {
    return "org.mixxx.effects.parametriceq";
}

// static
EffectManifest ParametricEQEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Parametric Equalizer"));
    manifest.setShortName(QObject::tr("Param EQ"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr(
        "An 2-band parametric equalizer based on biquad filters"));
    manifest.setEffectRampsFromDry(true);
    manifest.setIsMasterEQ(true);

    EffectManifestParameter* lfmGain = manifest.addParameter();
    lfmGain->setId("lmfGain");
    lfmGain->setName(QString("LMF Gain"));
    lfmGain->setDescription(QObject::tr("Gain for Low Mid Filter"));
    lfmGain->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    lfmGain->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    lfmGain->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    lfmGain->setNeutralPointOnScale(0.5);
    lfmGain->setDefault(0);
    lfmGain->setMinimum(-18);
    lfmGain->setMaximum(18);

    EffectManifestParameter* lfmQ = manifest.addParameter();
    lfmQ->setId("lmfQ");
    lfmQ->setName(QString("LMF Q"));
    lfmQ->setDescription(QObject::tr("Q for Low Mid Filter"));
    lfmQ->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    lfmQ->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    lfmQ->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    lfmQ->setNeutralPointOnScale(0.5);
    lfmQ->setDefault(1.75);
    lfmQ->setMinimum(0.5);
    lfmQ->setMaximum(3.0);

    EffectManifestParameter* lfmCenter = manifest.addParameter();
    lfmCenter->setId("lmfCenter");
    lfmCenter->setName(QString("LMF Center"));
    lfmCenter->setDescription(QObject::tr("Center frequency for Low Mid Filter"));
    lfmCenter->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    lfmCenter->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    lfmCenter->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    lfmCenter->setNeutralPointOnScale(0.5);
    lfmCenter->setDefault(550);
    lfmCenter->setMinimum(100);
    lfmCenter->setMaximum(5000);

    EffectManifestParameter* hfmGain = manifest.addParameter();
    hfmGain->setId("hmfGain");
    hfmGain->setName(QString("HMF Gain"));
    hfmGain->setDescription(QObject::tr("Gain for High Mid Filter"));
    hfmGain->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    hfmGain->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    hfmGain->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    hfmGain->setNeutralPointOnScale(0.5);
    hfmGain->setDefault(0);
    hfmGain->setMinimum(-18);
    hfmGain->setMaximum(18);

    EffectManifestParameter* hfmQ = manifest.addParameter();
    hfmQ->setId("hmfQ");
    hfmQ->setName(QString("HMF Q"));
    hfmQ->setDescription(QObject::tr("Q for High Mid Filter"));
    hfmQ->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    hfmQ->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    hfmQ->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    hfmQ->setNeutralPointOnScale(0.5);
    hfmQ->setDefault(1.75);
    hfmQ->setMinimum(0.5);
    hfmQ->setMaximum(3.0);

    EffectManifestParameter* hfmCenter = manifest.addParameter();
    hfmCenter->setId("hmfCenter");
    hfmCenter->setName(QString("LMF Center"));
    hfmCenter->setDescription(QObject::tr("Center frequency for Low Mid Filter"));
    hfmCenter->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    hfmCenter->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    hfmCenter->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    hfmCenter->setNeutralPointOnScale(0.5);
    hfmCenter->setDefault(2600);
    hfmCenter->setMinimum(1500);
    hfmCenter->setMaximum(14000);

    return manifest;
}

ParametricEQEffectGroupState::ParametricEQEffectGroupState() {
    for (int i = 0; i < kBandCount; i++) {
        m_oldGain.append(1.0);
        m_oldQ.append(1.75);
    }

    m_oldCenter.append(550);
    m_oldCenter.append(2600);

    // Initialize the filters with default parameters
    for (int i = 0; i < kBandCount; i++) {
        m_bands.push_back(std::make_unique<EngineFilterBiquad1Peaking>(
                44100, m_oldCenter[i], m_oldQ[i]));
    }
}

void ParametricEQEffectGroupState::setFilters(int sampleRate) {
    for (int i = 0; i < kBandCount; i++) {
        m_bands[i]->setFrequencyCorners(
                sampleRate, m_oldCenter[i], m_oldQ[i], m_oldGain[i]);
    }
}

ParametricEQEffect::ParametricEQEffect(EngineEffect* pEffect,
                                 const EffectManifest& manifest)
        : m_oldSampleRate(44100) {
    Q_UNUSED(manifest);
    m_pPotGain.append(pEffect->getParameterById("lmfGain"));
    m_pPotQ.append(pEffect->getParameterById("lmfQ"));
    m_pPotCenter.append(pEffect->getParameterById("lmfCenter"));
    m_pPotGain.append(pEffect->getParameterById("hmfGain"));
    m_pPotQ.append(pEffect->getParameterById("hmfQ"));
    m_pPotCenter.append(pEffect->getParameterById("hmfCenter"));
}

ParametricEQEffect::~ParametricEQEffect() {
}

void ParametricEQEffect::processChannel(const ChannelHandle& handle,
                                     ParametricEQEffectGroupState* pState,
                                     const CSAMPLE* pInput, CSAMPLE* pOutput,
                                     const unsigned int numSamples,
                                     const unsigned int sampleRate,
                                     const EffectProcessor::EnableState enableState,
                                     const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);
    Q_UNUSED(groupFeatures);

    // If the sample rate has changed, initialize the filters using the new
    // sample rate
    if (m_oldSampleRate != sampleRate) {
        m_oldSampleRate = sampleRate;
        pState->setFilters(sampleRate);
    }

    CSAMPLE_GAIN fGain[2];
    CSAMPLE_GAIN fQ[2];
    CSAMPLE_GAIN fCenter[2];

    if (enableState == EffectProcessor::DISABLING) {
         // Ramp to dry, when disabling, this will ramp from dry when enabling as well
        for (int i = 0; i < kBandCount; i++) {
            fGain[i] = 1.0;
        }
    } else {
        for (int i = 0; i < kBandCount; i++) {
            fGain[i] = m_pPotGain[i]->value();
        }
    }

    for (int i = 0; i < kBandCount; i++) {
        fGain[i] = m_pPotGain[i]->value();
        fQ[i] = m_pPotQ[i]->value();
        fCenter[i] = m_pPotCenter[i]->value();
    }

    for (int i = 0; i < kBandCount; i++) {
        if (fGain[i] != pState->m_oldGain[i] ||
                fQ[i] != pState->m_oldQ[i] ||
                fCenter[i] != pState->m_oldCenter[i]) {
            pState->m_bands[i]->setFrequencyCorners(
                    sampleRate, fCenter[i], fQ[i], fGain[i]);
        }
    }

    if (fGain[0]) {
        pState->m_bands[0]->process(pInput, pOutput, numSamples);
        if (fGain[1]) {
            pState->m_bands[1]->process(pOutput, pOutput, numSamples);
        } else {
            pState->m_bands[1]->pauseFilter();
        }
    } else {
        pState->m_bands[0]->pauseFilter();
        if (fGain[1]) {
            pState->m_bands[1]->process(pInput, pOutput, numSamples);
        } else {
            pState->m_bands[1]->pauseFilter();
            SampleUtil::copy(pOutput, pInput, numSamples);
        }
    }

    for (int i = 0; i < kBandCount; i++) {
        pState->m_oldGain[i] = fGain[i];
        pState->m_oldQ[i] = fQ[i];
        pState->m_oldCenter[i] = fCenter[i];
    }

    if (enableState == EffectProcessor::DISABLING) {
        for (int i = 0; i < kBandCount; i++) {
            pState->m_bands[i]->pauseFilter();
        }
    }
}
