#include "effects/backends/builtin/filtereffect.h"

#include "effects/backends/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/filters/enginefilterbiquad1.h"
#include "util/math.h"

namespace {
constexpr double kMinCorner = 13;    // Hz
constexpr double kMaxCorner = 22050; // Hz
} // anonymous namespace

// static
QString FilterEffect::getId() {
    return "org.mixxx.effects.filter";
}

// static
EffectManifestPointer FilterEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Filter"));
    pManifest->setShortName(QObject::tr("Filter"));
    pManifest->setAuthor("The Mixxx Team");
    pManifest->setVersion("1.0");
    pManifest->setDescription(QObject::tr(
            "Allows only high or low frequencies to play."));
    pManifest->setEffectRampsFromDry(true);
    pManifest->setMetaknobDefault(0.5);

    EffectManifestParameterPointer lpf = pManifest->addParameter();
    lpf->setId("lpf");
    lpf->setName(QObject::tr("Low Pass Filter Cutoff"));
    lpf->setShortName(QObject::tr("LPF"));
    lpf->setDescription(QObject::tr(
            "Corner frequency ratio of the low pass filter"));
    lpf->setValueScaler(EffectManifestParameter::ValueScaler::Logarithmic);
    lpf->setUnitsHint(EffectManifestParameter::UnitsHint::Hertz);
    lpf->setDefaultLinkType(EffectManifestParameter::LinkType::LinkedLeft);
    lpf->setNeutralPointOnScale(1);
    lpf->setRange(kMinCorner, kMaxCorner, kMaxCorner);

    EffectManifestParameterPointer q = pManifest->addParameter();
    q->setId("q");
    q->setName(QObject::tr("Resonance"));
    q->setShortName(QObject::tr("Q"));
    q->setDescription(QObject::tr(
            "Resonance of the filters\n"
            "Default: flat top")); // What does this mean?
    q->setValueScaler(EffectManifestParameter::ValueScaler::Logarithmic);
    q->setUnitsHint(EffectManifestParameter::UnitsHint::SampleRate);
    q->setRange(0.4, 0.707106781, 4.0); // 0.707106781 = Butterworth

    EffectManifestParameterPointer hpf = pManifest->addParameter();
    hpf->setId("hpf");
    hpf->setName(QObject::tr("High Pass Filter Cutoff"));
    hpf->setShortName(QObject::tr("HPF"));
    hpf->setDescription(QObject::tr(
            "Corner frequency ratio of the high pass filter"));
    hpf->setValueScaler(EffectManifestParameter::ValueScaler::Logarithmic);
    hpf->setUnitsHint(EffectManifestParameter::UnitsHint::Hertz);
    hpf->setDefaultLinkType(EffectManifestParameter::LinkType::LinkedRight);
    hpf->setNeutralPointOnScale(0.0);
    hpf->setRange(kMinCorner, kMinCorner, kMaxCorner);

    return pManifest;
}

FilterGroupState::FilterGroupState(const mixxx::EngineParameters& engineParameters)
        : EffectState(engineParameters),
          m_loFreq(kMaxCorner),
          m_q(0.707106781),
          m_hiFreq(kMinCorner) {
    m_buffer = mixxx::SampleBuffer(engineParameters.samplesPerBuffer());
    m_pLowFilter = new EngineFilterBiquad1Low(engineParameters.sampleRate(), m_loFreq, m_q, true);
    m_pHighFilter = new EngineFilterBiquad1High(engineParameters.sampleRate(), m_hiFreq, m_q, true);
}

FilterGroupState::~FilterGroupState() {
    delete m_pLowFilter;
    delete m_pHighFilter;
}

void FilterEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pLPF = parameters.value("lpf");
    m_pQ = parameters.value("q");
    m_pHPF = parameters.value("hpf");
}

void FilterEffect::processChannel(
        FilterGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(groupFeatures);

    double hpf;
    double lpf;
    double q = m_pQ->value();

    if (enableState == EffectEnableState::Disabling) {
        // Ramp to dry, when disabling, this will ramp from dry when enabling as well
        hpf = kMinCorner;
        lpf = kMaxCorner;
    } else {
        hpf = m_pHPF->value();
        lpf = m_pLPF->value();
    }

    if ((pState->m_loFreq != lpf) ||
            (pState->m_q != q) ||
            (pState->m_hiFreq != hpf)) {
        // limit Q to ~4 in case of overlap
        // Determined empirically at 1000 Hz
        double ratio = hpf / lpf;
        double clampedQ = q;
        if (ratio < 1.414 && ratio >= 1) {
            ratio -= 1;
            double qmax = 2 + ratio * ratio * ratio * 29;
            clampedQ = math_min(clampedQ, qmax);
        } else if (ratio < 1 && ratio >= 0.7) {
            clampedQ = math_min(clampedQ, 2.0);
        } else if (ratio < 0.7 && ratio > 0.1) {
            ratio -= 0.1;
            double qmax = 4 - 2 / 0.6 * ratio;
            clampedQ = math_min(clampedQ, qmax);
        }
        pState->m_pLowFilter->setFrequencyCorners(engineParameters.sampleRate(), lpf, clampedQ);
        pState->m_pHighFilter->setFrequencyCorners(engineParameters.sampleRate(), hpf, clampedQ);
    }

    const CSAMPLE* pLpfInput = pState->m_buffer.data();
    CSAMPLE* pHpfOutput = pState->m_buffer.data();
    if (lpf >= kMaxCorner && pState->m_loFreq >= kMaxCorner) {
        // Lpf disabled Hpf can write directly to output
        pHpfOutput = pOutput;
        pLpfInput = pHpfOutput;
    }

    if (hpf > kMinCorner) {
        // hpf enabled, fade-in is handled in the filter when starting from pause
        pState->m_pHighFilter->process(pInput, pHpfOutput, engineParameters.samplesPerBuffer());
    } else if (pState->m_hiFreq > kMinCorner) {
        // hpf disabling
        pState->m_pHighFilter->processAndPauseFilter(pInput,
                pHpfOutput,
                engineParameters.samplesPerBuffer());
    } else {
        // paused LP uses input directly
        pLpfInput = pInput;
    }

    if (lpf < kMaxCorner) {
        // lpf enabled, fade-in is handled in the filter when starting from pause
        pState->m_pLowFilter->process(pLpfInput, pOutput, engineParameters.samplesPerBuffer());
    } else if (pState->m_loFreq < kMaxCorner) {
        // hpf disabling
        pState->m_pLowFilter->processAndPauseFilter(pLpfInput,
                pOutput,
                engineParameters.samplesPerBuffer());
    } else if (pLpfInput == pInput) {
        // Both disabled
        if (pOutput != pInput) {
            // We need to copy pInput pOutput
            SampleUtil::copy(pOutput, pInput, engineParameters.samplesPerBuffer());
        }
    }

    pState->m_loFreq = lpf;
    pState->m_q = q;
    pState->m_hiFreq = hpf;
}
