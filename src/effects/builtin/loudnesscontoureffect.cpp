#include "effects/builtin/loudnesscontoureffect.h"
#include "util/math.h"

namespace {

// The defaults are tweaked to match the TEA6320 IC
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
EffectManifestPointer LoudnessContourEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Loudness Contour"));
    pManifest->setShortName(QObject::tr("Loudness"));
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription(QObject::tr(
        "Amplifies low and high frequencies at low volumes to compensate for reduced sensitivity of the human ear."));
    pManifest->setEffectRampsFromDry(true);
    pManifest->setMetaknobDefault(-kMaxLoGain / 2);

    EffectManifestParameterPointer loudness = pManifest->addParameter();
    loudness->setId("loudness");
    loudness->setName(QObject::tr("Loudness"));
    loudness->setShortName(QObject::tr("Loudness"));
    loudness->setDescription(QObject::tr(
        "Set the gain of the applied loudness contour"));
    loudness->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    loudness->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    loudness->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    loudness->setDefaultLinkType(EffectManifestParameter::LinkType::LINKED);
    loudness->setNeutralPointOnScale(1);
    loudness->setDefault(-kMaxLoGain / 2);
    loudness->setMinimum(-kMaxLoGain);
    loudness->setMaximum(0);

    EffectManifestParameterPointer useGain = pManifest->addParameter();
    useGain->setId("useGain");
    useGain->setName(QObject::tr("Use Gain"));
    useGain->setShortName(QObject::tr("Use Gain"));
    useGain->setDescription(QObject::tr(
        "Follow Gain Knob"));
    useGain->setControlHint(EffectManifestParameter::ControlHint::TOGGLE_STEPPING);
    useGain->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    useGain->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    useGain->setDefault(0);
    useGain->setMinimum(0);
    useGain->setMaximum(1);

    return pManifest;
}

LoudnessContourEffectGroupState::LoudnessContourEffectGroupState(
        const mixxx::EngineParameters& bufferParameters)
        : EffectState(bufferParameters),
          m_oldGainKnob(1.0),
          m_oldLoudness(0.0),
          m_oldGain(1.0f),
          m_oldFilterGainDb(0),
          m_oldUseGain(false),
          m_oldSampleRate(bufferParameters.sampleRate()) {
    m_pBuf = SampleUtil::alloc(bufferParameters.samplesPerBuffer());

    // Initialize the filters with default parameters
    m_low = std::make_unique<EngineFilterBiquad1Peaking>(
            bufferParameters.sampleRate() , kLoPeakFreq , kHiShelveQ);
    m_high = std::make_unique<EngineFilterBiquad1HighShelving>(
            bufferParameters.sampleRate() , kHiShelveFreq ,kHiShelveQ);
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
                EngineEffect* pEffect)
        : m_pLoudness(pEffect->getParameterById("loudness")),
          m_pUseGain(pEffect->getParameterById("useGain")) {
}

LoudnessContourEffect::~LoudnessContourEffect() {
}

void LoudnessContourEffect::processChannel(
        const ChannelHandle& handle,
        LoudnessContourEffectGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& bufferParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);
    Q_UNUSED(groupFeatures);

    double filterGainDb = pState->m_oldFilterGainDb;
    auto gain = static_cast<CSAMPLE_GAIN>(pState->m_oldGain);

    if (enableState != EffectEnableState::Disabling) {

        bool useGain = m_pUseGain->toBool() && groupFeatures.has_gain;
        double loudness = m_pLoudness->value();
        double gainKnob = groupFeatures.gain;

        filterGainDb = loudness;

        if (useGain != pState->m_oldUseGain ||
                gainKnob != pState->m_oldGainKnob ||
                loudness != pState->m_oldLoudness ||
                bufferParameters.sampleRate() != pState->m_oldSampleRate) {

            pState->m_oldUseGain = useGain;
            pState->m_oldGainKnob =  gainKnob;
            pState->m_oldLoudness = loudness;
            pState->m_oldSampleRate = bufferParameters.sampleRate();

            if (useGain) {
                gainKnob = math_clamp(gainKnob, 0.03, 1.0); // Limit at 0 .. -30 dB
                double gainKnobDb = ratio2db(gainKnob);
                filterGainDb = loudness * gainKnobDb / kMaxLoGain;
                gain = 1; // No need for adjust gain because master gain follows
            }
            else {
                filterGainDb = -loudness;
                // compensate filter boost to avoid clipping
                gain = static_cast<CSAMPLE_GAIN>(db2ratio(-filterGainDb));
            }
            pState->setFilters(bufferParameters.sampleRate(), filterGainDb);
        }
    }

    if (filterGainDb == 0) {
        pState->m_low->pauseFilter();
        pState->m_high->pauseFilter();
        SampleUtil::copy(pOutput, pInput, bufferParameters.samplesPerBuffer());
    } else {
        pState->m_low->process(pInput, pOutput, bufferParameters.samplesPerBuffer());
        pState->m_high->process(pOutput, pState->m_pBuf, bufferParameters.samplesPerBuffer());
        SampleUtil::copyWithRampingGain(
                pOutput, pState->m_pBuf, pState->m_oldGain, gain,
                bufferParameters.samplesPerBuffer());
    }

    pState->m_oldFilterGainDb = filterGainDb ;
    pState->m_oldGain = gain;
}
