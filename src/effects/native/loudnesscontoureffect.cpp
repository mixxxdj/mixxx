#include "effects/native/loudnesscontoureffect.h"
#include "util/math.h"

namespace {

// The defaults are tweaked to match the TEA6320 IC
static const int kStartupSamplerate = 44100;
static const double kLoPeakFreq = 20.0;
static const double kHiShelveFreq = 3000.0;
static const double kMaxLoGain = 30.0;
static const double kHiShelveGainDiv = 3.0;
static const double kLoPleakQ = 0.2;
static const double kHiShelveQ = 0.7;

} // anonymous namesspace


// static
QString LoudnessContourEffect::getId() {
    return "org.mixxx.effects.loudnesscontour";
}

// static
EffectManifest LoudnessContourEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    manifest.setName(QObject::tr("Loudness Contour"));
    manifest.setShortName(QObject::tr("Loudness"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr(
            "Amplifies low and high frequencies at low volumes to compensate for reduced sensitivity of the human ear."));
    manifest.setEffectRampsFromDry(true);
    manifest.setIsMixingEQ(true);

    EffectManifestParameter* loudness = manifest.addParameter();
    loudness->setId("loudness");
    loudness->setName(QObject::tr("Loudness"));
    loudness->setDescription(QObject::tr("Set the gain of the applied loudness contour"));
    loudness->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    loudness->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    loudness->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    loudness->setDefaultLinkType(EffectManifestParameter::LINK_LINKED);
    loudness->setNeutralPointOnScale(1);
    loudness->setDefault(-kMaxLoGain / 2);
    loudness->setMinimum(-kMaxLoGain);
    loudness->setMaximum(0);

    EffectManifestParameter* killLow = manifest.addParameter();
    killLow->setId("useMasterGain");
    killLow->setName(QObject::tr("Master Gain"));
    killLow->setDescription(QObject::tr("Follow master Gain"));
    killLow->setControlHint(EffectManifestParameter::CONTROL_TOGGLE_STEPPING);
    killLow->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    killLow->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    killLow->setDefault(0);
    killLow->setMinimum(0);
    killLow->setMaximum(1);

    return manifest;
}

LoudnessContourEffectGroupState::LoudnessContourEffectGroupState()
        : m_oldMasterGain(1.0),
          m_oldLoudness(0.0),
          m_oldGain(1.0),
          m_oldFilterGainDb(0),
          m_oldUseMaster(false),
          m_oldSampleRate(kStartupSamplerate) {

    m_pBuf = SampleUtil::alloc(MAX_BUFFER_LEN);

    // Initialize the filters with default parameters
    m_low = std::make_unique<EngineFilterBiquad1Peaking>(kStartupSamplerate , kLoPeakFreq , kHiShelveQ);
    m_high = std::make_unique<EngineFilterBiquad1HighShelving>(kStartupSamplerate , kHiShelveFreq ,kHiShelveQ);
}

LoudnessContourEffectGroupState::~LoudnessContourEffectGroupState() {
    SampleUtil::free(m_pBuf);
}

void LoudnessContourEffectGroupState::setFilters(int sampleRate, double gain) {
    m_low->setFrequencyCorners(
            sampleRate, kLoPeakFreq, kLoPleakQ, gain);
    m_high->setFrequencyCorners(
            sampleRate, kHiShelveFreq, kHiShelveQ, gain / kHiShelveGainDiv);

}

LoudnessContourEffect::LoudnessContourEffect(
                EngineEffect* pEffect, const EffectManifest& manifest)
        : m_pLoudness(pEffect->getParameterById("loudness")),
          m_pUseMasterGain(pEffect->getParameterById("useMasterGain")) {
    Q_UNUSED(manifest);
    m_pMasterGain = std::make_unique<ControlProxy>("[Master]", "gain");
}

LoudnessContourEffect::~LoudnessContourEffect() {
}

void LoudnessContourEffect::processChannel(
        const ChannelHandle& handle,
        LoudnessContourEffectGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const unsigned int numSamples,
        const unsigned int sampleRate,
        const EffectProcessor::EnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);
    Q_UNUSED(groupFeatures);

    double filterGainDb = pState->m_oldFilterGainDb;
    double gain = pState->m_oldGain;

    if (enableState != EffectProcessor::DISABLING) {

        bool useMaster = m_pUseMasterGain->toBool();
        double loudness = m_pLoudness->value();
        double masterGain = m_pMasterGain->get();

        filterGainDb = loudness;

        if (useMaster != pState->m_oldUseMaster ||
                masterGain != pState->m_oldMasterGain ||
                loudness != pState->m_oldLoudness ||
                sampleRate != pState->m_oldSampleRate) {

            pState->m_oldUseMaster = useMaster;
            pState->m_oldMasterGain =  masterGain;
            pState->m_oldLoudness = loudness;
            pState->m_oldSampleRate = sampleRate;

            if (useMaster) {
                masterGain = math_clamp(masterGain, 0.03, 1.0); // Limit at 0 .. -30 dB
                double masterGainDb = ratio2db(masterGain);
                filterGainDb = loudness * masterGainDb / kMaxLoGain;
                gain = 1; // No need for adjust gain because master gain follows
            }
            else {
                filterGainDb = -loudness;
                gain = db2ratio(-filterGainDb); // compensate filter boost to avoid clipping
            }
            pState->setFilters(sampleRate, filterGainDb);
        }
    }

    if (filterGainDb == 0) {
        pState->m_low->pauseFilter();
        pState->m_high->pauseFilter();
        SampleUtil::copy(pOutput, pInput, numSamples);
    } else {
        pState->m_low->process(pInput, pOutput, numSamples);
        pState->m_high->process(pOutput, pState->m_pBuf, numSamples);
        SampleUtil::copyWithRampingGain(pOutput, pState->m_pBuf, pState->m_oldGain, gain, numSamples);
    }

    pState->m_oldFilterGainDb = filterGainDb ;
    pState->m_oldGain = gain;
}
