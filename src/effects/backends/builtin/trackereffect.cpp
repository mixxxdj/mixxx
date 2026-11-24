#include "effects/backends/builtin/trackereffect.h"

#include <algorithm>

#include "effects/backends/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/sample.h"

namespace {

inline SINT getMaskFromSize(SINT len) {
    SINT n = 2;
    while (n <= len) {
        n <<= 1;
    }
    return ((n >> 1) - 1);
}

} // anonymous namespace

// static
QString TrackerEffect::getId() {
    return "org.mixxx.effects.trackerdsp";
}

// static
EffectManifestPointer TrackerEffect::getManifest() {
    EffectManifestPointer pManifest(new EffectManifest());
    pManifest->setId(getId());
    pManifest->setName(QObject::tr("Tracker DSP"));
    pManifest->setAuthor("The Mixxx Team, libmodplug contributors");
    pManifest->setVersion("1.0");
    pManifest->setDescription(QObject::tr(
            "Libmodplug-style DSP effects for tracker modules: reverb, "
            "megabass, Pro-Logic surround, and noise reduction. "
            "Apply via the effect rack to shape tracker module sound "
            "during mixing."));

    // Reverb
    EffectManifestParameterPointer reverb = pManifest->addParameter();
    reverb->setId("reverb");
    reverb->setName(QObject::tr("Reverb"));
    reverb->setShortName(QObject::tr("Reverb"));
    reverb->setDescription(QObject::tr("Enable libmodplug-style reverb"));
    reverb->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    reverb->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    reverb->setRange(0, 0, 1);

    EffectManifestParameterPointer reverbDepth = pManifest->addParameter();
    reverbDepth->setId("reverb_depth");
    reverbDepth->setName(QObject::tr("Reverb Depth"));
    reverbDepth->setShortName(QObject::tr("Rvb Depth"));
    reverbDepth->setDescription(QObject::tr("Reverb intensity (0-100)"));
    reverbDepth->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    reverbDepth->setUnitsHint(EffectManifestParameter::UnitsHint::Percentage);
    reverbDepth->setRange(0, 50, 100);

    EffectManifestParameterPointer reverbDelay = pManifest->addParameter();
    reverbDelay->setId("reverb_delay");
    reverbDelay->setName(QObject::tr("Reverb Delay"));
    reverbDelay->setShortName(QObject::tr("Rvb Delay"));
    reverbDelay->setDescription(QObject::tr("Reverb delay in milliseconds (40-200)"));
    reverbDelay->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    reverbDelay->setUnitsHint(EffectManifestParameter::UnitsHint::Millisecond);
    reverbDelay->setRange(40, 100, 200);

    // Megabass
    EffectManifestParameterPointer megabass = pManifest->addParameter();
    megabass->setId("megabass");
    megabass->setName(QObject::tr("Megabass"));
    megabass->setShortName(QObject::tr("MegaBass"));
    megabass->setDescription(QObject::tr("Enable bass expansion"));
    megabass->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    megabass->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    megabass->setRange(0, 0, 1);

    EffectManifestParameterPointer bassDepth = pManifest->addParameter();
    bassDepth->setId("bass_depth");
    bassDepth->setName(QObject::tr("Bass Depth"));
    bassDepth->setShortName(QObject::tr("Bass Depth"));
    bassDepth->setDescription(QObject::tr("Bass expansion intensity (0-100)"));
    bassDepth->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    bassDepth->setUnitsHint(EffectManifestParameter::UnitsHint::Percentage);
    bassDepth->setRange(0, 50, 100);

    // Surround
    EffectManifestParameterPointer surround = pManifest->addParameter();
    surround->setId("surround");
    surround->setName(QObject::tr("Surround"));
    surround->setShortName(QObject::tr("Surround"));
    surround->setDescription(QObject::tr("Enable Pro-Logic surround"));
    surround->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    surround->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    surround->setRange(0, 0, 1);

    EffectManifestParameterPointer surroundDepth = pManifest->addParameter();
    surroundDepth->setId("surround_depth");
    surroundDepth->setName(QObject::tr("Surround Depth"));
    surroundDepth->setShortName(QObject::tr("Sur Depth"));
    surroundDepth->setDescription(QObject::tr("Surround intensity (0-100)"));
    surroundDepth->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    surroundDepth->setUnitsHint(EffectManifestParameter::UnitsHint::Percentage);
    surroundDepth->setRange(0, 50, 100);

    EffectManifestParameterPointer surroundDelay = pManifest->addParameter();
    surroundDelay->setId("surround_delay");
    surroundDelay->setName(QObject::tr("Surround Delay"));
    surroundDelay->setShortName(QObject::tr("Sur Delay"));
    surroundDelay->setDescription(QObject::tr("Surround delay in milliseconds (5-40)"));
    surroundDelay->setValueScaler(EffectManifestParameter::ValueScaler::Linear);
    surroundDelay->setUnitsHint(EffectManifestParameter::UnitsHint::Millisecond);
    surroundDelay->setRange(5, 20, 40);

    // Noise reduction
    EffectManifestParameterPointer noiseReduction = pManifest->addParameter();
    noiseReduction->setId("noise_reduction");
    noiseReduction->setName(QObject::tr("Noise Reduction"));
    noiseReduction->setShortName(QObject::tr("NR"));
    noiseReduction->setDescription(QObject::tr("Enable simple low-pass noise reduction"));
    noiseReduction->setValueScaler(EffectManifestParameter::ValueScaler::Toggle);
    noiseReduction->setUnitsHint(EffectManifestParameter::UnitsHint::Unknown);
    noiseReduction->setRange(0, 0, 1);

    return pManifest;
}

void TrackerEffect::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_pReverbParameter = parameters.value("reverb");
    m_pReverbDepthParameter = parameters.value("reverb_depth");
    m_pReverbDelayParameter = parameters.value("reverb_delay");
    m_pMegabassParameter = parameters.value("megabass");
    m_pBassDepthParameter = parameters.value("bass_depth");
    m_pSurroundParameter = parameters.value("surround");
    m_pSurroundDepthParameter = parameters.value("surround_depth");
    m_pSurroundDelayParameter = parameters.value("surround_delay");
    m_pNoiseReductionParameter = parameters.value("noise_reduction");
}

void TrackerEffectGroupState::reset() {
    m_nXBassSum = m_nXBassBufferPos = m_nXBassDlyPos = 0;
    m_nLeftNR = m_nRightNR = 0;
    m_nSurroundPos = m_nSurroundSize = 0;
    m_nDolbyLoFltPos = m_nDolbyLoFltSum = m_nDolbyLoDlyPos = 0;
    m_nDolbyHiFltPos = m_nDolbyHiFltSum = 0;
    m_nReverbBufferPos = m_nReverbBufferPos2 = m_nReverbBufferPos3 =
            m_nReverbBufferPos4 = 0;
    m_nReverbLoFltSum = m_nReverbLoFltPos = m_nReverbLoDlyPos = 0;
    m_gRvbLPSum = m_gRvbLPPos = 0;
    m_nReverbSize = 0;
    m_nXBassMask = 0;
    std::fill(m_xBassBuffer.begin(), m_xBassBuffer.end(), 0);
    std::fill(m_xBassDelay.begin(), m_xBassDelay.end(), 0);
    std::fill(m_dolbyLoFilterBuffer.begin(), m_dolbyLoFilterBuffer.end(), 0);
    std::fill(m_dolbyHiFilterBuffer.begin(), m_dolbyHiFilterBuffer.end(), 0);
    std::fill(m_dolbyLoFilterDelay.begin(), m_dolbyLoFilterDelay.end(), 0);
    std::fill(m_surroundBuffer.begin(), m_surroundBuffer.end(), 0);
    std::fill(m_reverbLoFilterBuffer.begin(), m_reverbLoFilterBuffer.end(), 0);
    std::fill(m_reverbLoFilterDelay.begin(), m_reverbLoFilterDelay.end(), 0);
    std::fill(m_reverbBuffer.begin(), m_reverbBuffer.end(), 0);
    std::fill(m_reverbBuffer2.begin(), m_reverbBuffer2.end(), 0);
    std::fill(m_reverbBuffer3.begin(), m_reverbBuffer3.end(), 0);
    std::fill(m_reverbBuffer4.begin(), m_reverbBuffer4.end(), 0);
    std::fill(m_gRvbLowPass.begin(), m_gRvbLowPass.end(), 0);
}

void TrackerEffect::configureReverb(
        TrackerEffectGroupState* pState, int depth, int delayMs) {
    SINT nReverbDelay = delayMs;
    if (nReverbDelay == 0) {
        nReverbDelay = 100;
    }
    pState->m_nReverbDelay = nReverbDelay;
    pState->m_nReverbDepth = depth / 10;

    SINT nrs = (pState->m_sampleRate * nReverbDelay) / 1000;
    SINT nfa = pState->m_nReverbDepth + 1;
    if (nrs > 8192) {
        nrs = 8192;
    }

    if (nrs != pState->m_nReverbSize || nfa != pState->m_nFilterAttn) {
        pState->m_nFilterAttn = nfa;
        pState->m_nReverbSize = nrs;
        pState->m_nReverbBufferPos = pState->m_nReverbBufferPos2 =
                pState->m_nReverbBufferPos3 = pState->m_nReverbBufferPos4 = 0;
        pState->m_nReverbLoFltSum = pState->m_nReverbLoFltPos =
                pState->m_nReverbLoDlyPos = 0;
        pState->m_gRvbLPSum = pState->m_gRvbLPPos = 0;

        pState->m_nReverbSize2 = (nrs * 13) / 17;
        if (pState->m_nReverbSize2 > 6128) {
            pState->m_nReverbSize2 = 6128;
        }
        pState->m_nReverbSize3 = (nrs * 7) / 13;
        if (pState->m_nReverbSize3 > 4432) {
            pState->m_nReverbSize3 = 4432;
        }
        pState->m_nReverbSize4 = (nrs * 7) / 19;
        if (pState->m_nReverbSize4 > 2964) {
            pState->m_nReverbSize4 = 2964;
        }

        std::fill(pState->m_reverbLoFilterBuffer.begin(),
                pState->m_reverbLoFilterBuffer.end(),
                0);
        std::fill(pState->m_reverbLoFilterDelay.begin(),
                pState->m_reverbLoFilterDelay.end(),
                0);
        std::fill(pState->m_reverbBuffer.begin(),
                pState->m_reverbBuffer.end(),
                0);
        std::fill(pState->m_reverbBuffer2.begin(),
                pState->m_reverbBuffer2.end(),
                0);
        std::fill(pState->m_reverbBuffer3.begin(),
                pState->m_reverbBuffer3.end(),
                0);
        std::fill(pState->m_reverbBuffer4.begin(),
                pState->m_reverbBuffer4.end(),
                0);
        std::fill(pState->m_gRvbLowPass.begin(),
                pState->m_gRvbLowPass.end(),
                0);
    }
}

void TrackerEffect::configureSurround(
        TrackerEffectGroupState* pState, int depth, int delayMs) {
    SINT nProLogicDelay = delayMs;
    if (nProLogicDelay == 0) {
        nProLogicDelay = 20;
    }
    pState->m_nProLogicDelay = nProLogicDelay;
    pState->m_nProLogicDepth = (depth * 12) / 100 + 4;

    pState->m_nSurroundPos = pState->m_nSurroundSize = 0;
    pState->m_nDolbyLoFltPos = pState->m_nDolbyLoFltSum =
            pState->m_nDolbyLoDlyPos = 0;
    pState->m_nDolbyHiFltPos = pState->m_nDolbyHiFltSum = 0;

    std::fill(pState->m_dolbyLoFilterBuffer.begin(),
            pState->m_dolbyLoFilterBuffer.end(),
            0);
    std::fill(pState->m_dolbyHiFilterBuffer.begin(),
            pState->m_dolbyHiFilterBuffer.end(),
            0);
    std::fill(pState->m_dolbyLoFilterDelay.begin(),
            pState->m_dolbyLoFilterDelay.end(),
            0);
    std::fill(pState->m_surroundBuffer.begin(),
            pState->m_surroundBuffer.end(),
            0);

    pState->m_nSurroundSize = (pState->m_sampleRate * nProLogicDelay) / 1000;
    if (pState->m_nSurroundSize > 8192) {
        pState->m_nSurroundSize = 8192;
    }

    if (pState->m_nProLogicDepth < 8) {
        pState->m_nDolbyDepth = (32 >> pState->m_nProLogicDepth) + 32;
    } else {
        pState->m_nDolbyDepth = (pState->m_nProLogicDepth < 16)
                ? (8 + (pState->m_nProLogicDepth - 8) * 7)
                : 64;
    }
    pState->m_nDolbyDepth >>= 2;
}

void TrackerEffect::configureMegabass(
        TrackerEffectGroupState* pState, int depth) {
    pState->m_nXBassRange = 14;
    pState->m_nXBassDepth = (depth * 6) / 100 + 2;
    if (pState->m_nXBassDepth > 8) {
        pState->m_nXBassDepth = 8;
    }
    if (pState->m_nXBassDepth < 2) {
        pState->m_nXBassDepth = 2;
    }

    SINT nXBassSamples = (pState->m_sampleRate * pState->m_nXBassRange) / 10000;
    if (nXBassSamples > 128) {
        nXBassSamples = 128;
    }
    SINT mask = getMaskFromSize(nXBassSamples);
    if (mask != pState->m_nXBassMask) {
        pState->m_nXBassMask = mask;
        pState->m_nXBassSum = pState->m_nXBassBufferPos =
                pState->m_nXBassDlyPos = 0;
        std::fill(pState->m_xBassBuffer.begin(),
                pState->m_xBassBuffer.end(),
                0);
        std::fill(pState->m_xBassDelay.begin(),
                pState->m_xBassDelay.end(),
                0);
    }
}

void TrackerEffect::processChannel(
        TrackerEffectGroupState* pState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(groupFeatures);

    const bool reverbOn = m_pReverbParameter->value() > 0;
    const bool megabassOn = m_pMegabassParameter->value() > 0;
    const bool surroundOn = m_pSurroundParameter->value() > 0;
    const bool noiseReductionOn = m_pNoiseReductionParameter->value() > 0;

    if (pState->m_sampleRate != engineParameters.sampleRate()) {
        pState->setEngineParameters(engineParameters);
    }

    if (enableState == EffectEnableState::Enabling) {
        pState->reset();
    }

    if (reverbOn) {
        configureReverb(pState,
                static_cast<int>(m_pReverbDepthParameter->value()),
                static_cast<int>(m_pReverbDelayParameter->value()));
    }
    if (surroundOn) {
        configureSurround(pState,
                static_cast<int>(m_pSurroundDepthParameter->value()),
                static_cast<int>(m_pSurroundDelayParameter->value()));
    }
    if (megabassOn) {
        configureMegabass(pState,
                static_cast<int>(m_pBassDepthParameter->value()));
    }

    const SINT frameCount = engineParameters.framesPerBuffer();
    const SINT sampleCount = frameCount * 2;

    // copy input to output first (so dry signal is preserved if no effect)
    SampleUtil::copy(pOutput, pInput, sampleCount);

    if (!reverbOn && !megabassOn && !surroundOn && !noiseReductionOn) {
        return;
    }

    // convert float to int for DSP processing (libmodplug uses 32-bit ints)
    SINT* intBuffer = pState->m_intBuffer.data();
    for (SINT i = 0; i < sampleCount; ++i) {
        intBuffer[i] = static_cast<SINT>(pOutput[i] * 32768.0f);
    }

    // apply effects in order (matching libmodplug order)
    if (reverbOn && pState->m_nReverbSize > 0) {
        processReverb(pState, intBuffer, frameCount);
    }
    if (surroundOn && pState->m_nSurroundSize > 0) {
        processSurround(pState, intBuffer, frameCount);
    }
    if (megabassOn && pState->m_nXBassMask > 0) {
        processMegabass(pState, intBuffer, frameCount);
    }
    if (noiseReductionOn) {
        processNoiseReduction(pState, intBuffer, frameCount);
    }

    // convert back to float
    for (SINT i = 0; i < sampleCount; ++i) {
        pOutput[i] = static_cast<CSAMPLE>(intBuffer[i]) / 32768.0f;
    }
}

void TrackerEffect::processReverb(
        TrackerEffectGroupState* pState, SINT* pr, SINT frameCount) {
    for (SINT i = 0; i < frameCount; ++i) {
        SINT echo = pState->m_reverbBuffer[pState->m_nReverbBufferPos] +
                pState->m_reverbBuffer2[pState->m_nReverbBufferPos2] +
                pState->m_reverbBuffer3[pState->m_nReverbBufferPos3] +
                pState->m_reverbBuffer4[pState->m_nReverbBufferPos4];

        SINT echodly = pState->m_reverbLoFilterDelay[pState->m_nReverbLoDlyPos];
        pState->m_reverbLoFilterDelay[pState->m_nReverbLoDlyPos] = echo >> 1;
        pState->m_nReverbLoDlyPos = (pState->m_nReverbLoDlyPos + 1) & 0x1F;

        SINT n = pState->m_nReverbLoFltPos;
        pState->m_nReverbLoFltSum -= pState->m_reverbLoFilterBuffer[n];
        SINT tmp = echo / 128;
        pState->m_reverbLoFilterBuffer[n] = tmp;
        pState->m_nReverbLoFltSum += tmp;
        echodly -= pState->m_nReverbLoFltSum;
        pState->m_nReverbLoFltPos = (n + 1) & 0x3F;

        SINT v = (pr[0] + pr[1]) >> pState->m_nFilterAttn;
        pr[0] += echodly;
        pr[1] += echodly;
        v += echodly >> 2;
        pState->m_reverbBuffer3[pState->m_nReverbBufferPos3] = v;
        pState->m_reverbBuffer4[pState->m_nReverbBufferPos4] = v;
        v += echodly >> 4;
        v >>= 1;

        pState->m_gRvbLPSum -= pState->m_gRvbLowPass[pState->m_gRvbLPPos];
        pState->m_gRvbLPSum += v;
        pState->m_gRvbLowPass[pState->m_gRvbLPPos] = v;
        pState->m_gRvbLPPos = (pState->m_gRvbLPPos + 1) & 7;

        SINT vlp = pState->m_gRvbLPSum >> 2;
        pState->m_reverbBuffer[pState->m_nReverbBufferPos] = vlp;
        pState->m_reverbBuffer2[pState->m_nReverbBufferPos2] = vlp;

        if (++pState->m_nReverbBufferPos >= pState->m_nReverbSize) {
            pState->m_nReverbBufferPos = 0;
        }
        if (++pState->m_nReverbBufferPos2 >= pState->m_nReverbSize2) {
            pState->m_nReverbBufferPos2 = 0;
        }
        if (++pState->m_nReverbBufferPos3 >= pState->m_nReverbSize3) {
            pState->m_nReverbBufferPos3 = 0;
        }
        if (++pState->m_nReverbBufferPos4 >= pState->m_nReverbSize4) {
            pState->m_nReverbBufferPos4 = 0;
        }

        pr += 2;
    }
}

void TrackerEffect::processSurround(
        TrackerEffectGroupState* pState, SINT* pr, SINT frameCount) {
    SINT n = pState->m_nDolbyLoFltPos;

    for (SINT i = 0; i < frameCount; ++i) {
        SINT v = (pr[0] + pr[1] + 1) >> 1;
        v *= pState->m_nDolbyDepth;
        v >>= 8;

        pState->m_nDolbyHiFltSum -= pState->m_dolbyHiFilterBuffer[pState->m_nDolbyHiFltPos];
        pState->m_dolbyHiFilterBuffer[pState->m_nDolbyHiFltPos] = v;
        pState->m_nDolbyHiFltSum += v;
        v = pState->m_nDolbyHiFltSum;
        pState->m_nDolbyHiFltPos = (pState->m_nDolbyHiFltPos + 1) & 31;

        SINT secho = pState->m_surroundBuffer[pState->m_nSurroundPos];
        pState->m_surroundBuffer[pState->m_nSurroundPos] = v;

        v = pState->m_dolbyLoFilterDelay[pState->m_nDolbyLoDlyPos];
        pState->m_dolbyLoFilterDelay[pState->m_nDolbyLoDlyPos] = secho;
        pState->m_nDolbyLoDlyPos = (pState->m_nDolbyLoDlyPos + 1) & 0x1F;

        pState->m_nDolbyLoFltSum -= pState->m_dolbyLoFilterBuffer[n];
        SINT tmp = secho / 64;
        pState->m_dolbyLoFilterBuffer[n] = tmp;
        pState->m_nDolbyLoFltSum += tmp;
        v -= pState->m_nDolbyLoFltSum;
        n = (n + 1) & 0x3F;

        pr[0] += v;
        pr[1] -= v;

        pState->m_nSurroundPos = (pState->m_nSurroundPos + 1) % pState->m_nSurroundSize;
        pr += 2;
    }

    pState->m_nDolbyLoFltPos = n;
}

void TrackerEffect::processMegabass(
        TrackerEffectGroupState* pState, SINT* px, SINT frameCount) {
    SINT xba = pState->m_nXBassDepth + 1;
    SINT xbamask = (1 << xba) - 1;
    SINT n = pState->m_nXBassBufferPos;

    for (SINT i = 0; i < frameCount; ++i) {
        pState->m_nXBassSum -= pState->m_xBassBuffer[n];
        SINT tmp0 = px[0] + px[1];
        SINT tmp = (tmp0 + ((tmp0 >> 31) & xbamask)) >> xba;
        pState->m_xBassBuffer[n] = tmp;
        pState->m_nXBassSum += tmp;

        SINT v = pState->m_xBassDelay[pState->m_nXBassDlyPos];
        pState->m_xBassDelay[pState->m_nXBassDlyPos] = px[0];
        px[0] = v + pState->m_nXBassSum;

        v = pState->m_xBassDelay[pState->m_nXBassDlyPos + 1];
        pState->m_xBassDelay[pState->m_nXBassDlyPos + 1] = px[1];
        px[1] = v + pState->m_nXBassSum;

        pState->m_nXBassDlyPos = (pState->m_nXBassDlyPos + 2) & pState->m_nXBassMask;
        px += 2;
        n = (n + 1) & pState->m_nXBassMask;
    }

    pState->m_nXBassBufferPos = n;
}

void TrackerEffect::processNoiseReduction(
        TrackerEffectGroupState* pState, SINT* pnr, SINT frameCount) {
    SINT n1 = pState->m_nLeftNR;
    SINT n2 = pState->m_nRightNR;

    for (SINT i = 0; i < frameCount; ++i) {
        SINT vnr = pnr[0] >> 1;
        pnr[0] = vnr + n1;
        n1 = vnr;

        vnr = pnr[1] >> 1;
        pnr[1] = vnr + n2;
        n2 = vnr;

        pnr += 2;
    }

    pState->m_nLeftNR = n1;
    pState->m_nRightNR = n2;
}
