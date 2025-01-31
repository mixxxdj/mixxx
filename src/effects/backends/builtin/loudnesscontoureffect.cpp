#include "effects/backends/builtin/loudnesscontoureffect.h"

#include "effects/backends/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/filters/enginefilterbiquad1.h"
#include "util/math.h"

namespace {

// The defaults are tweaked to match the TEA6320 IC
static constexpr double kLoPeakFreq = 20.0;
static constexpr double kHiShelveFreq = 3000.0;
static constexpr double kMaxLoGain = 30.0;
static constexpr double kHiShelveGainDiv = 3.0;
static constexpr double kLoPleakQ = 0.2;
static constexpr double kHiShelveQ = 0.7;

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
            "Amplifies low and high frequencies at low volumes to compensate "
            "for reduced sensitivity of the human ear."));
    pManifest->setEffectRampsFromDry(true);
    pManifest->setMetaknobDefault(1.0);

    EffectManifestParameterPointer loudness = pManifest->addParameter();
    loudness->setId("loudness");
    loudness->setName(QObject::tr("Loudness"));
    loudness->setShortName(QObject::tr("Loudness"));
    loudness->setDescription(QObject::tr(
            "Set the gain of the applied loudness contour"));
    loudness->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    loudness->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    loudness->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    loudness->setNeutralPointOnScale(1);
    loudness->setRange(-kMaxLoGain, -kMaxLoGain / 2, 0);

    EffectManifestParameterPointer useGain = pManifest->addParameter();
    useGain->setId("useGain");
    useGain->setName(QObject::tr("Use Gain"));
    useGain->setShortName(QObject::tr("Use Gain"));
    useGain->setDescription(QObject::tr(
            "Follow Gain Knob"));
    useGain->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    useGain->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    useGain->setRange(0, 0, 1);

    return pManifest;
}

LoudnessContourEffectGroupState::LoudnessContourEffectGroupState(
        const mixxx::EngineParameters& engineParameters)
        : EffectState(engineParameters),
          m_oldGainKnob(1.0),
          m_oldLoudness(0.0),
          m_oldGain(1.0f),
          m_oldFilterGainDb(0),
          m_oldUseGain(false),
          m_oldSampleRate(engineParameters.sampleRate()) {
    m_pBuf = SampleUtil::alloc(engineParameters.samplesPerBuffer());

    // Initialize the filters with default parameters
    m_low = std::make_unique<EngineFilterBiquad1Peaking>(
            engineParameters.sampleRate(), kLoPeakFreq, kHiShelveQ);
    m_high = std::make_unique<EngineFilterBiquad1HighShelving>(
            engineParameters.sampleRate(), kHiShelveFreq, kHiShelveQ);
}

LoudnessContourEffectGroupState::~LoudnessContourEffectGroupState() {
    SampleUtil::free(m_pBuf);
}

void LoudnessContourEffectGroupState::setFilters(mixxx::audio::SampleRate sampleRate, double gain) {
    m_low->setFrequencyCorners(
            sampleRate, kLoPeakFreq, kLoPleakQ, gain);
    m_high->setFrequencyCorners(
            sampleRate, kHiShelveFreq, kHiShelveQ, gain / kHiShelveGainDiv);
}

void LoudnessContourEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pLoudness = parameters.value("loudness");
    m_pUseGain = parameters.value("useGain");
}

void LoudnessContourEffect::processChannel(
        LoudnessContourEffectGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    double filterGainDb = pState->m_oldFilterGainDb;
    auto gain = static_cast<CSAMPLE_GAIN>(pState->m_oldGain);

    if (enableState != EffectEnableState::Disabling) {
        bool useGain = m_pUseGain->toBool() && groupFeatures.gain.has_value();
        double loudness = m_pLoudness->value();
        double gainKnob = groupFeatures.gain.value_or(1.0);

        filterGainDb = loudness;

        if (useGain != pState->m_oldUseGain ||
                gainKnob != pState->m_oldGainKnob ||
                loudness != pState->m_oldLoudness ||
                engineParameters.sampleRate() != pState->m_oldSampleRate) {
            pState->m_oldUseGain = useGain;
            pState->m_oldGainKnob = gainKnob;
            pState->m_oldLoudness = loudness;
            pState->m_oldSampleRate = engineParameters.sampleRate();

            if (useGain) {
                gainKnob = math_clamp(gainKnob, 0.03, 1.0); // Limit at 0 .. -30 dB
                double gainKnobDb = ratio2db(gainKnob);
                filterGainDb = loudness * gainKnobDb / kMaxLoGain;
                gain = 1; // No need for adjust gain because main gain follows
            } else {
                filterGainDb = -loudness;
                // compensate filter boost to avoid clipping
                gain = static_cast<CSAMPLE_GAIN>(db2ratio(-filterGainDb));
            }
            pState->setFilters(engineParameters.sampleRate(), filterGainDb);
        }
    }

    if (filterGainDb == 0) {
        pState->m_low->pauseFilter();
        pState->m_high->pauseFilter();
        if (pOutput != pInput) {
            SampleUtil::copy(pOutput, pInput, engineParameters.samplesPerBuffer());
        }
    } else {
        pState->m_low->process(pInput, pOutput, engineParameters.samplesPerBuffer());
        pState->m_high->process(pOutput, pState->m_pBuf, engineParameters.samplesPerBuffer());
        SampleUtil::copyWithRampingGain(pOutput,
                pState->m_pBuf,
                pState->m_oldGain,
                gain,
                engineParameters.samplesPerBuffer());
    }

    pState->m_oldFilterGainDb = filterGainDb;
    pState->m_oldGain = gain;
}
