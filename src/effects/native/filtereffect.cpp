#include "effects/native/filtereffect.h"
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
EffectManifest FilterEffect::getManifest() {
    EffectManifest manifest;
    manifest.setId(getId());
    //: Long name
    manifest.setName(QObject::tr("Filter"));
    //: Short name
    manifest.setShortName(QObject::tr("Filter"));
    manifest.setAuthor("The Mixxx Team");
    manifest.setVersion("1.0");
    manifest.setDescription(QObject::tr("The filter changes the tone of the "
                                        "music by allowing only high or low "
                                        "frequencies to pass through."));
    manifest.setEffectRampsFromDry(true);

    EffectManifestParameter* lpf = manifest.addParameter();
    lpf->setId("lpf");
    lpf->setName(QObject::tr("LPF"));
    lpf->setDescription(QObject::tr("Corner frequency ratio of the low pass filter"));
    lpf->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    lpf->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    lpf->setUnitsHint(EffectManifestParameter::UnitsHint::HERTZ);
    lpf->setDefaultLinkType(EffectManifestParameter::LinkType::LINKED_LEFT);
    lpf->setNeutralPointOnScale(1);
    lpf->setDefault(kMaxCorner);
    lpf->setMinimum(kMinCorner);
    lpf->setMaximum(kMaxCorner);

    EffectManifestParameter* q = manifest.addParameter();
    q->setId("q");
    q->setName(QObject::tr("Resonance"));
    q->setShortName(QObject::tr("Q"));
    q->setDescription(QObject::tr("Resonance of the filters, default = Flat top"));
    q->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    q->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    q->setUnitsHint(EffectManifestParameter::UnitsHint::SAMPLERATE);
    q->setDefault(0.707106781); // 0.707106781 = Butterworth
    q->setMinimum(0.4);
    q->setMaximum(4.0);

    EffectManifestParameter* hpf = manifest.addParameter();
    hpf->setId("hpf");
    hpf->setName(QObject::tr("HPF"));
    hpf->setDescription(QObject::tr("Corner frequency ratio of the high pass filter"));
    hpf->setControlHint(EffectManifestParameter::ControlHint::KNOB_LOGARITHMIC);
    hpf->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    hpf->setUnitsHint(EffectManifestParameter::UnitsHint::HERTZ);
    hpf->setDefaultLinkType(EffectManifestParameter::LinkType::LINKED_RIGHT);
    hpf->setNeutralPointOnScale(0.0);
    hpf->setDefault(kMinCorner);
    hpf->setMinimum(kMinCorner);
    hpf->setMaximum(kMaxCorner);

    return manifest;
}

FilterGroupState::FilterGroupState()
        : m_loFreq(kMaxCorner / 44100),
          m_q(0.707106781),
          m_hiFreq(kMinCorner / 44100) {
    m_pBuf = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pLowFilter = new EngineFilterBiquad1Low(1, m_loFreq, m_q, true);
    m_pHighFilter = new EngineFilterBiquad1High(1, m_hiFreq, m_q, true);
}

FilterGroupState::~FilterGroupState() {
    SampleUtil::free(m_pBuf);
    delete m_pLowFilter;
    delete m_pHighFilter;
}

FilterEffect::FilterEffect(EngineEffect* pEffect,
                           const EffectManifest& manifest)
        : m_pLPF(pEffect->getParameterById("lpf")),
          m_pQ(pEffect->getParameterById("q")),
          m_pHPF(pEffect->getParameterById("hpf")) {
    Q_UNUSED(manifest);
}

FilterEffect::~FilterEffect() {
    //qDebug() << debugString() << "destroyed";
}

void FilterEffect::processChannel(const ChannelHandle& handle,
                                  FilterGroupState* pState,
                                  const CSAMPLE* pInput, CSAMPLE* pOutput,
                                  const unsigned int numSamples,
                                  const unsigned int sampleRate,
                                  const EffectProcessor::EnableState enableState,
                                  const GroupFeatureState& groupFeatures) {
    Q_UNUSED(handle);
    Q_UNUSED(groupFeatures);
    Q_UNUSED(sampleRate);

    double hpf;
    double lpf;
    double q = m_pQ->value();

    const double minCornerNormalized = kMinCorner / sampleRate;
    const double maxCornerNormalized = kMaxCorner / sampleRate;

    if (enableState == EffectProcessor::DISABLING) {
        // Ramp to dry, when disabling, this will ramp from dry when enabling as well
        hpf = minCornerNormalized;
        lpf = maxCornerNormalized;
    } else {
        hpf = m_pHPF->value() / sampleRate;
        lpf = m_pLPF->value() / sampleRate;
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

    const CSAMPLE* pLpfInput = pState->m_pBuf;
    CSAMPLE* pHpfOutput = pState->m_pBuf;
    if (lpf >= maxCornerNormalized && pState->m_loFreq >= maxCornerNormalized) {
        // Lpf disabled Hpf can write directly to output
        pHpfOutput = pOutput;
        pLpfInput = pHpfOutput;
    }

    if (hpf > minCornerNormalized) {
        // hpf enabled, fade-in is handled in the filter when starting from pause
        pState->m_pHighFilter->process(pInput, pHpfOutput, numSamples);
    } else if (pState->m_hiFreq > minCornerNormalized) {
            // hpf disabling
            pState->m_pHighFilter->processAndPauseFilter(pInput,
                    pHpfOutput, numSamples);
    } else {
        // paused LP uses input directly
        pLpfInput = pInput;
    }

    if (lpf < maxCornerNormalized) {
        // lpf enabled, fade-in is handled in the filter when starting from pause
        pState->m_pLowFilter->process(pLpfInput, pOutput, numSamples);
    } else if (pState->m_loFreq < maxCornerNormalized) {
        // hpf disabling
        pState->m_pLowFilter->processAndPauseFilter(pLpfInput,
                pOutput, numSamples);
    } else if (pLpfInput == pInput) {
        // Both disabled
        if (pOutput != pInput) {
            // We need to copy pInput pOutput
            SampleUtil::copy(pOutput, pInput, numSamples);
        }
    }

    pState->m_loFreq = lpf;
    pState->m_q = q;
    pState->m_hiFreq = hpf;
}
