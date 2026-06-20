#include "effects/backends/builtin/profiltereffect.h"

#include <algorithm>
#include <cmath>

#include "effects/backends/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {
enum FilterMode {
    LPF = 0,
    HPF = 1,
    BPF = 2,
    NOTCH = 3,
    PEAK = 4,
    NUM_MODES = 5
};

constexpr double kSmoothCoeff = 0.05;

} // anonymous namespace

void ProFilterEffect::computeBiquadCoefficients(
        double cutoff,
        double resonance,
        double gain,
        double* b0,
        double* b1,
        double* b2,
        double* a1,
        double* a2,
        int mode,
        double sampleRate) {
    double fc = std::clamp(cutoff, 20.0, 20000.0) / sampleRate;
    double Q = std::clamp(resonance, 0.1, 20.0);
    double A = std::pow(10.0, gain / 40.0);

    double w0 = 2.0 * M_PI * fc;
    double cosw0 = std::cos(w0);
    double sinw0 = std::sin(w0);
    double alpha = sinw0 / (2.0 * Q);

    double a0;

    switch (mode) {
    case LPF:
        *b0 = (1.0 - cosw0) / 2.0;
        *b1 = 1.0 - cosw0;
        *b2 = (1.0 - cosw0) / 2.0;
        a0 = 1.0 + alpha;
        *a1 = -2.0 * cosw0;
        *a2 = 1.0 - alpha;
        break;
    case HPF:
        *b0 = (1.0 + cosw0) / 2.0;
        *b1 = -(1.0 + cosw0);
        *b2 = (1.0 + cosw0) / 2.0;
        a0 = 1.0 + alpha;
        *a1 = -2.0 * cosw0;
        *a2 = 1.0 - alpha;
        break;
    case BPF:
        *b0 = alpha;
        *b1 = 0.0;
        *b2 = -alpha;
        a0 = 1.0 + alpha;
        *a1 = -2.0 * cosw0;
        *a2 = 1.0 - alpha;
        break;
    case NOTCH:
        *b0 = 1.0;
        *b1 = -2.0 * cosw0;
        *b2 = 1.0;
        a0 = 1.0 + alpha;
        *a1 = -2.0 * cosw0;
        *a2 = 1.0 - alpha;
        break;
    case PEAK:
        *b0 = 1.0 + alpha * A;
        *b1 = -2.0 * cosw0;
        *b2 = 1.0 - alpha * A;
        a0 = 1.0 + alpha / A;
        *a1 = -2.0 * cosw0;
        *a2 = 1.0 - alpha / A;
        break;
    default:
        *b0 = 1.0;
        *b1 = 0.0;
        *b2 = 0.0;
        a0 = 1.0;
        *a1 = 0.0;
        *a2 = 0.0;
        break;
    }

    *b0 /= a0;
    *b1 /= a0;
    *b2 /= a0;
    *a1 /= a0;
    *a2 /= a0;
}

QString ProFilterEffect::getId() {
    return QStringLiteral("org.mixxx.effects.profilter");
}

EffectManifestPointer ProFilterEffect::getManifest() {
    auto pManifest = EffectManifestPointer::create();
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Pro Filter"));
    pManifest->setAuthor("DJ Sugar");
    pManifest->setVersion("1.0");
    pManifest->setDescription(QObject::tr(
            "Professional multi-mode filter with LPF, HPF, BPF, Notch, and Peaking modes. "
            "High resonance for dramatic sweeps. Comparable to Rekordbox/Serato filters."));

    auto pCutoff = pManifest->addParameter();
    pCutoff->setId("cutoff");
    pCutoff->setName(QObject::tr("Cutoff"));
    pCutoff->setDescription(QObject::tr("Filter cutoff frequency"));
    pCutoff->setValueScaler(EffectManifestParameter::ValueScaler::Logarithmic);
    pCutoff->setUnitsHint(EffectManifestParameter::UnitsHint::Hertz);
    pCutoff->setDefaultLinkType(EffectManifestParameter::LinkType::Linked);
    pCutoff->setRange(0.0, 0.5, 1.0);

    auto pResonance = pManifest->addParameter();
    pResonance->setId("resonance");
    pResonance->setName(QObject::tr("Resonance"));
    pResonance->setDescription(QObject::tr("Filter resonance (Q factor)"));
    pResonance->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    pResonance->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    pResonance->setRange(0.0, 0.3, 1.0);

    auto pMode = pManifest->addParameter();
    pMode->setId("mode");
    pMode->setName(QObject::tr("Mode"));
    pMode->setDescription(QObject::tr("Filter mode: 0=LPF, 1=HPF, 2=BPF, 3=Notch, 4=Peak"));
    pMode->setValueScaler(EffectManifestParameter::ValueScaler::Integral);
    pMode->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    pMode->setRange(0.0, 0.0, 4.0);

    auto pGain = pManifest->addParameter();
    pGain->setId("gain");
    pGain->setName(QObject::tr("Gain"));
    pGain->setDescription(QObject::tr("Gain (for peaking mode, in dB)"));
    pGain->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    pGain->setUnitsHint(EffectManifestParameter::UnitsHint::Decibel);
    pGain->setRange(-1.0, 0.0, 1.0);

    return pManifest;
}

void ProFilterEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pCutoffParameter = parameters.value("cutoff");
    m_pResonanceParameter = parameters.value("resonance");
    m_pModeParameter = parameters.value("mode");
    m_pGainParameter = parameters.value("gain");
}

void ProFilterEffect::processChannel(
        ProFilterGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        [[maybe_unused]] const EffectEnableState enableState,
        [[maybe_unused]] const GroupFeatureState& groupFeatures) {
    const int sampleRate = engineParameters.sampleRate();
    const SINT numSamples = engineParameters.samplesPerBuffer();
    const int chCount = engineParameters.channelCount();

    double cutoff_raw = m_pCutoffParameter->value();
    double resonance_raw = m_pResonanceParameter->value();
    int mode = static_cast<int>(m_pModeParameter->value());
    double gain_raw = m_pGainParameter->value();

    mode = std::clamp(mode, 0, NUM_MODES - 1);

    double cutoff = 20.0 * std::pow(1000.0, cutoff_raw);
    double resonance = 0.1 + resonance_raw * 19.9;
    double gain = gain_raw * 12.0;

    double smoothCutoff = pState->prev_cutoff + kSmoothCoeff * (cutoff - pState->prev_cutoff);
    double smoothResonance = pState->prev_resonance +
            kSmoothCoeff * (resonance - pState->prev_resonance);
    double smoothGain = pState->prev_gain + kSmoothCoeff * (gain - pState->prev_gain);

    pState->prev_cutoff = static_cast<CSAMPLE>(smoothCutoff);
    pState->prev_resonance = static_cast<CSAMPLE>(smoothResonance);
    pState->prev_gain = static_cast<CSAMPLE>(smoothGain);

    double b0, b1, b2, a1, a2;
    computeBiquadCoefficients(
            smoothCutoff, smoothResonance, smoothGain, &b0, &b1, &b2, &a1, &a2, mode, sampleRate);

    for (SINT i = 0; i < numSamples; ++i) {
        int ch = i % chCount;

        double x0 = static_cast<double>(pInput[i]);
        double y0 = b0 * x0 + b1 * pState->x1[ch] + b2 * pState->x2[ch] -
                a1 * pState->y1[ch] - a2 * pState->y2[ch];

        pState->x2[ch] = pState->x1[ch];
        pState->x1[ch] = static_cast<CSAMPLE>(x0);
        pState->y2[ch] = pState->y1[ch];
        pState->y1[ch] = static_cast<CSAMPLE>(y0);

        pOutput[i] = static_cast<CSAMPLE>(y0);
    }
}
