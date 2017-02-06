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

} // anonymous namespace


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

    EffectManifestParameter* loudness = manifest.addParameter();
    loudness->setId("loudness");
    loudness->setName(QObject::tr("Loudness"));
    loudness->setDescription(
            QObject::tr("Set the gain of the applied loudness contour"));
    loudness->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    loudness->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    loudness->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    loudness->setDefaultLinkType(EffectManifestParameter::LinkType::LINKED);
    loudness->setNeutralPointOnScale(1);
    loudness->setDefault(-kMaxLoGain / 2);
    loudness->setMinimum(-kMaxLoGain);
    loudness->setMaximum(0);

    EffectManifestParameter* useGain = manifest.addParameter();
    useGain->setId("useGain");
    useGain->setName(QObject::tr("Use Gain"));
    useGain->setDescription(QObject::tr("Follow Gain Knob"));
    useGain->setControlHint(EffectManifestParameter::ControlHint::TOGGLE_STEPPING);
    useGain->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    useGain->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    useGain->setDefault(0);
    useGain->setMinimum(0);
    useGain->setMaximum(1);

    return manifest;
}

LoudnessContourEffectGroupState::LoudnessContourEffectGroupState()
        : m_oldGainKnob(1.0),
          m_oldLoudness(0.0),
          m_oldGain(1.0),
          m_oldFilterGainDb(0),
          m_oldUseGain(false),
          m_oldSampleRate(kStartupSamplerate) {

    m_pBuf = SampleUtil::alloc(MAX_BUFFER_LEN);

    // Initialize the filters with default parameters
    m_low = std::make_unique<EngineFilterBiquad1Peaking>(
            kStartupSamplerate , kLoPeakFreq , kHiShelveQ);
    m_high = std::make_unique<EngineFilterBiquad1HighShelving>(
            kStartupSamplerate , kHiShelveFreq ,kHiShelveQ);
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
          m_pUseGain(pEffect->getParameterById("useGain")) {
    Q_UNUSED(manifest);
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

        bool useGain = m_pUseGain->toBool() && groupFeatures.has_gain;
        double loudness = m_pLoudness->value();
        double gainKnob = groupFeatures.gain;

        filterGainDb = loudness;

        if (useGain != pState->m_oldUseGain ||
                gainKnob != pState->m_oldGainKnob ||
                loudness != pState->m_oldLoudness ||
                sampleRate != pState->m_oldSampleRate) {

            pState->m_oldUseGain = useGain;
            pState->m_oldGainKnob =  gainKnob;
            pState->m_oldLoudness = loudness;
            pState->m_oldSampleRate = sampleRate;

            if (useGain) {
                gainKnob = math_clamp(gainKnob, 0.03, 1.0); // Limit at 0 .. -30 dB
                double gainKnobDb = ratio2db(gainKnob);
                filterGainDb = loudness * gainKnobDb / kMaxLoGain;
                gain = 1; // No need for adjust gain because master gain follows
            }
            else {
                filterGainDb = -loudness;
                // compensate filter boost to avoid clipping
                gain = db2ratio(-filterGainDb);
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
        SampleUtil::copyWithRampingGain(
                pOutput, pState->m_pBuf, pState->m_oldGain, gain, numSamples);
    }

    pState->m_oldFilterGainDb = filterGainDb ;
    pState->m_oldGain = gain;
}
