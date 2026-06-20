#include "effects/backends/builtin/pronoiseeffect.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>

#include "effects/backends/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {
struct FastRNG {
    uint32_t state = 12345;

    float nextFloat() {
        state = state * 1103515245 + 12345;
        return static_cast<float>(state) / 4294967296.0f;
    }
};

thread_local FastRNG g_rng;

constexpr double kSmoothCoeff = 0.01;

enum NoiseMode {
    WHITE = 0,
    PINK = 1,
    BANDPASS = 2,
    NUM_MODES = 3
};

} // anonymous namespace

CSAMPLE ProNoiseEffect::generateWhiteNoise() {
    return g_rng.nextFloat() * 2.0f - 1.0f;
}

CSAMPLE ProNoiseEffect::generatePinkNoise(ProNoiseGroupState* pState) {
    constexpr int numGenerators = 7;
    constexpr uint32_t updateMasks[numGenerators] = {
            0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40};

    pState->pink_index = (pState->pink_index + 1) & 0x7F;

    pState->pink_running_sum = 0.0f;
    for (int i = 0; i < numGenerators; ++i) {
        if ((pState->pink_index & updateMasks[i]) == 0) {
            pState->pink_state[i] = generateWhiteNoise();
        }
        pState->pink_running_sum += pState->pink_state[i];
    }

    pState->pink_running_sum += generateWhiteNoise() * 0.5f;

    return pState->pink_running_sum / static_cast<float>(numGenerators + 1);
}

QString ProNoiseEffect::getId() {
    return QStringLiteral("org.mixxx.effects.pronoise");
}

EffectManifestPointer ProNoiseEffect::getManifest() {
    auto pManifest = EffectManifestPointer::create();
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Pro Noise"));
    pManifest->setAuthor("DJ Sugar");
    pManifest->setVersion("1.0");
    pManifest->setDescription(QObject::tr(
            "Professional noise generator for DJ transitions. "
            "Supports white noise, pink noise, and bandpass-filtered noise. "
            "Comparable to Rekordbox's Noise effect."));

    auto pSend = pManifest->addParameter();
    pSend->setId("send");
    pSend->setName(QObject::tr("Send"));
    pSend->setDescription(QObject::tr("Noise volume"));
    pSend->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    pSend->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    pSend->setRange(0.0, 0.5, 1.0);

    auto pColor = pManifest->addParameter();
    pColor->setId("color");
    pColor->setName(QObject::tr("Color"));
    pColor->setDescription(QObject::tr("Noise color (0=white, 1=pink)"));
    pColor->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    pColor->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    pColor->setRange(0.0, 0.5, 1.0);

    auto pBandwidth = pManifest->addParameter();
    pBandwidth->setId("bandwidth");
    pBandwidth->setName(QObject::tr("Bandwidth"));
    pBandwidth->setDescription(QObject::tr("Filter bandwidth for bandpass mode"));
    pBandwidth->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    pBandwidth->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    pBandwidth->setRange(0.0, 0.5, 1.0);

    auto pMode = pManifest->addParameter();
    pMode->setId("mode");
    pMode->setName(QObject::tr("Mode"));
    pMode->setDescription(QObject::tr("Noise mode: 0=White, 1=Pink, 2=Bandpass"));
    pMode->setValueScaler(EffectManifestParameter::ValueScaler::Integral);
    pMode->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    pMode->setRange(0.0, 0.0, 2.0);

    return pManifest;
}

void ProNoiseEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pSendParameter = parameters.value("send");
    m_pColorParameter = parameters.value("color");
    m_pBandwidthParameter = parameters.value("bandwidth");
    m_pModeParameter = parameters.value("mode");
}

void ProNoiseEffect::processChannel(
        ProNoiseGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        [[maybe_unused]] const EffectEnableState enableState,
        [[maybe_unused]] const GroupFeatureState& groupFeatures) {
    const int sampleRate = engineParameters.sampleRate();
    const SINT numSamples = engineParameters.samplesPerBuffer();

    double send = m_pSendParameter->value();
    double color = m_pColorParameter->value();
    double bandwidth = m_pBandwidthParameter->value();
    int mode = static_cast<int>(m_pModeParameter->value());

    mode = std::clamp(mode, 0, NUM_MODES - 1);

    double smoothSend = pState->prev_send + kSmoothCoeff * (send - pState->prev_send);
    double smoothBandwidth = pState->prev_bandwidth +
            kSmoothCoeff * (bandwidth - pState->prev_bandwidth);

    pState->prev_send = static_cast<CSAMPLE>(smoothSend);
    pState->prev_bandwidth = static_cast<CSAMPLE>(smoothBandwidth);

    // Use color to blend between white (0) and pink (1) noise
    // For bandpass mode, color controls the center frequency offset
    double bp_center, bp_q;
    if (mode == BANDPASS) {
        bp_center = 500.0 + smoothBandwidth * 9500.0;
        bp_q = 0.5 + color * 9.5;
    } else {
        bp_center = 1000.0 + smoothBandwidth * 8000.0;
        bp_q = 1.0 + smoothBandwidth * 4.0;
    }
    double w0 = 2.0 * M_PI * bp_center / sampleRate;
    double cosw0 = std::cos(w0);
    double sinw0 = std::sin(w0);
    double alpha = sinw0 / (2.0 * bp_q);

    double b0 = alpha;
    double b1 = 0.0;
    double b2 = -alpha;
    double a0 = 1.0 + alpha;
    double a1 = -2.0 * cosw0;
    double a2 = 1.0 - alpha;

    b0 /= a0;
    b1 /= a0;
    b2 /= a0;
    a1 /= a0;
    a2 /= a0;

    for (SINT i = 0; i < numSamples; ++i) {
        CSAMPLE noise;

        switch (mode) {
        case WHITE:
            noise = generateWhiteNoise();
            break;
        case PINK:
            noise = generatePinkNoise(pState);
            break;
        case BANDPASS: {
            CSAMPLE white = generateWhiteNoise();
            CSAMPLE filtered = static_cast<CSAMPLE>(b0 * white +
                    b1 * pState->filter_x1 + b2 * pState->filter_x2 -
                    a1 * pState->filter_y1 - a2 * pState->filter_y2);
            pState->filter_x2 = pState->filter_x1;
            pState->filter_x1 = white;
            pState->filter_y2 = pState->filter_y1;
            pState->filter_y1 = filtered;
            noise = filtered;
            break;
        }
        default:
            noise = generateWhiteNoise();
            break;
        }

        // Blend white/pink based on color parameter (only for non-bandpass modes)
        if (mode != BANDPASS) {
            CSAMPLE whiteNoise = generateWhiteNoise();
            CSAMPLE pinkNoise = generatePinkNoise(pState);
            noise = static_cast<CSAMPLE>(whiteNoise * (1.0f - color) + pinkNoise * color);
        }

        pOutput[i] = static_cast<CSAMPLE>(pInput[i] + noise * smoothSend);
    }
}
