#include "effects/builtin/filtereffect.h"
#include "util/math.h"

namespace {
const double kMinCorner = 13; // Hz
const double kMaxCorner = 22050; // Hz
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

    EffectManifestParameterPointer lpf = pManifest->addParameter();
    lpf->setId("lpf");
    lpf->setName(QObject::tr("Low Pass Filter Cutoff"));
    lpf->setShortName(QObject::tr("LPF"));
    lpf->setDescription(QObject::tr(
        "Corner frequency ratio of the low pass filter"));
    lpf->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    lpf->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    lpf->setUnitsHint(EffectManifestParameter::UnitsHint::HERTZ);
    lpf->setDefaultLinkType(EffectManifestParameter::LinkType::LINKED_LEFT);
    lpf->setNeutralPointOnScale(1);
    lpf->setDefault(kMaxCorner);
    lpf->setMinimum(kMinCorner);
    lpf->setMaximum(kMaxCorner);

    EffectManifestParameterPointer q = pManifest->addParameter();
    q->setId("q");
    q->setName(QObject::tr("Resonance"));
    q->setShortName(QObject::tr("Q"));
    q->setDescription(QObject::tr(
        "Resonance of the filters\n"
        "Default: flat top")); // What does this mean?
    q->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    q->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    q->setUnitsHint(EffectManifestParameter::UnitsHint::SAMPLERATE);
    q->setDefault(0.707106781); // 0.707106781 = Butterworth
    q->setMinimum(0.4);
    q->setMaximum(4.0);

    EffectManifestParameterPointer hpf = pManifest->addParameter();
    hpf->setId("hpf");
    hpf->setName(QObject::tr("High Pass Filter Cutoff"));
    hpf->setShortName(QObject::tr("HPF"));
    hpf->setDescription(QObject::tr(
        "Corner frequency ratio of the high pass filter"));
    hpf->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    hpf->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    hpf->setUnitsHint(EffectManifestParameter::UnitsHint::HERTZ);
    hpf->setDefaultLinkType(EffectManifestParameter::LinkType::LINKED_RIGHT);
    hpf->setNeutralPointOnScale(0.0);
    hpf->setDefault(kMinCorner);
    hpf->setMinimum(kMinCorner);
    hpf->setMaximum(kMaxCorner);

    return pManifest;
}

FilterGroupState::FilterGroupState(const mixxx::EngineParameters& bufferParameters)
        : EffectState(bufferParameters),
          m_loFreq(kMaxCorner / bufferParameters.sampleRate()),
          m_q(0.707106781),
          m_hiFreq(kMinCorner / bufferParameters.sampleRate()) {
    m_buffer = mixxx::SampleBuffer(bufferParameters.samplesPerBuffer());
    m_pLowFilter = new EngineFilterBiquad1Low(1, m_loFreq, m_q, true);
    m_pHighFilter = new EngineFilterBiquad1High(1, m_hiFreq, m_q, true);
}

FilterGroupState::~FilterGroupState() {
    delete m_pLowFilter;
    delete m_pHighFilter;
}

FilterEffect::FilterEffect(EngineEffect* pEffect)
        : m_pLPF(pEffect->getParameterById("lpf")),
          m_pQ(pEffect->getParameterById("q")),
          m_pHPF(pEffect->getParameterById("hpf")) {
}

FilterEffect::~FilterEffect() {
    //qDebug() << debugString() << "destroyed";
}

void FilterEffect::processChannel(const ChannelHandle& handle,
                                  FilterGroupState* pState,
                                  const CSAMPLE* pInput, CSAMPLE* pOutput,
                                  const mixxx::EngineParameters& bufferParameters,
                                  const EffectEnableState enableState,
                                  const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);
    Q_UNUSED(groupFeatures);

    double hpf;
    double lpf;
    double q = m_pQ->value();

    const double minCornerNormalized = kMinCorner / bufferParameters.sampleRate();
    const double maxCornerNormalized = kMaxCorner / bufferParameters.sampleRate();

    if (enableState == EffectEnableState::Disabling) {
        // Ramp to dry, when disabling, this will ramp from dry when enabling as well
        hpf = minCornerNormalized;
        lpf = maxCornerNormalized;
    } else {
        hpf = m_pHPF->value() / bufferParameters.sampleRate();
        lpf = m_pLPF->value() / bufferParameters.sampleRate();
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
        pState->m_pLowFilter->setFrequencyCorners(1, lpf, clampedQ);
        pState->m_pHighFilter->setFrequencyCorners(1, hpf, clampedQ);
    }

    const CSAMPLE* pLpfInput = pState->m_buffer.data();
    CSAMPLE* pHpfOutput = pState->m_buffer.data();
    if (lpf >= maxCornerNormalized && pState->m_loFreq >= maxCornerNormalized) {
        // Lpf disabled Hpf can write directly to output
        pHpfOutput = pOutput;
        pLpfInput = pHpfOutput;
    }

    if (hpf > minCornerNormalized) {
        // hpf enabled, fade-in is handled in the filter when starting from pause
        pState->m_pHighFilter->process(pInput, pHpfOutput, bufferParameters.samplesPerBuffer());
    } else if (pState->m_hiFreq > minCornerNormalized) {
            // hpf disabling
            pState->m_pHighFilter->processAndPauseFilter(pInput,
                    pHpfOutput, bufferParameters.samplesPerBuffer());
    } else {
        // paused LP uses input directly
        pLpfInput = pInput;
    }

    if (lpf < maxCornerNormalized) {
        // lpf enabled, fade-in is handled in the filter when starting from pause
        pState->m_pLowFilter->process(pLpfInput, pOutput, bufferParameters.samplesPerBuffer());
    } else if (pState->m_loFreq < maxCornerNormalized) {
        // hpf disabling
        pState->m_pLowFilter->processAndPauseFilter(pLpfInput,
                pOutput, bufferParameters.samplesPerBuffer());
    } else if (pLpfInput == pInput) {
        // Both disabled
        if (pOutput != pInput) {
            // We need to copy pInput pOutput
            SampleUtil::copy(pOutput, pInput, bufferParameters.samplesPerBuffer());
        }
    }

    pState->m_loFreq = lpf;
    pState->m_q = q;
    pState->m_hiFreq = hpf;
}
