// sampleutil.cpp
// Created 10/5/2009 by RJ Ryan (rryan@mit.edu)

#include "sampleutil.h"
#include "util/math.h"

#include <QtDebug>

#include <functional>
#ifdef __WINDOWS__
#include <QtGlobal>
typedef qint64 int64_t;
typedef qint32 int32_t;
#endif

// static
CSAMPLE* SampleUtil::alloc(unsigned int size) {
    // TODO(XXX) align the array
    return new CSAMPLE[size];
}

void SampleUtil::free(CSAMPLE* pBuffer) {
    delete[] pBuffer;
}

// static
void SampleUtil::widenMonoToStereo(SAMPLE* pBuffer, unsigned int numFrames) {
    // backward loop
    unsigned int sampleOffset = numFrames;
    while (0 < sampleOffset--) {
        pBuffer[sampleOffset * 2] = pBuffer[sampleOffset];
        pBuffer[sampleOffset * 2 + 1] = pBuffer[sampleOffset];
    }
}

// static
void SampleUtil::widenMonoToStereo(CSAMPLE* pBuffer, unsigned int numFrames) {
    // backward loop
    unsigned int sampleOffset = numFrames;
    while (0 < sampleOffset--) {
        pBuffer[sampleOffset * 2] = pBuffer[sampleOffset];
        pBuffer[sampleOffset * 2 + 1] = pBuffer[sampleOffset];
    }
}

// static
void SampleUtil::copyWidenMonoToStereo(CSAMPLE* pDest, const CSAMPLE* pSrc,
        unsigned int numFrames) {
    // forward loop
    for (unsigned int i = 0; i < numFrames; ++i) {
        pDest[i * 2] = pSrc[i];
        pDest[i * 2 + 1] = pSrc[i];
    }
}

// static
void SampleUtil::narrowMultiToStereo(CSAMPLE* pBuffer, unsigned int numFrames,
        unsigned int numChannels) {
    // the copying implementation can be reused here
    copyNarrowMultiToStereo(pBuffer, pBuffer, numFrames, numChannels);
}

// static
void SampleUtil::copyNarrowMultiToStereo(CSAMPLE* pDest, const CSAMPLE* pSrc,
        unsigned int numFrames, unsigned int numChannels) {
    // forward loop
    for (unsigned int i = 0; i < numFrames; ++i) {
        pDest[i * 2] = pSrc[i * numChannels];
        pDest[i * 2 + 1] = pSrc[i * numChannels + 1];
    }
}

// static
void SampleUtil::applyGain(CSAMPLE* pBuffer, CSAMPLE gain,
        unsigned int iNumSamples) {
    if (gain == CSAMPLE_PEAK)
        return;
    if (gain == CSAMPLE_ZERO) {
        clear(pBuffer, iNumSamples);
        return;
    }

    std::transform(pBuffer, pBuffer + iNumSamples, pBuffer,
            std::bind2nd(std::multiplies<CSAMPLE>(), gain));
}

// static
void SampleUtil::applyRampingGain(CSAMPLE* pBuffer, CSAMPLE old_gain,
        CSAMPLE new_gain, unsigned int iNumSamples) {
    if (old_gain == CSAMPLE_PEAK && new_gain == CSAMPLE_PEAK) {
        return;
    }
    if (old_gain == CSAMPLE_ZERO && new_gain == CSAMPLE_ZERO) {
        clear(pBuffer, iNumSamples);
        return;
    }

    const CSAMPLE gain_delta = (CSAMPLE_PEAK + CSAMPLE_PEAK) * (new_gain - old_gain)
            / iNumSamples;
    CSAMPLE gain = old_gain;
    for (unsigned int i = 0; i < iNumSamples; i += 2, gain += gain_delta) {
        pBuffer[i] *= gain;
        pBuffer[i + 1] *= gain;
    }
}

// static
void SampleUtil::applyAlternatingGain(CSAMPLE* pBuffer, CSAMPLE gain1,
        CSAMPLE gain2, unsigned int iNumSamples) {
    // This handles gain1 == CSAMPLE_PEAK && gain2 == CSAMPLE_PEAK as well.
    if (gain1 == gain2) {
        return applyGain(pBuffer, gain1, iNumSamples);
    }

    for (unsigned int i = 0; i < iNumSamples; i += 2) {
        pBuffer[i] *= gain1;
        pBuffer[i + 1] *= gain2;
    }
}

// static
void SampleUtil::addWithGain(CSAMPLE* pDest, const CSAMPLE* pSrc, CSAMPLE gain,
        unsigned int iNumSamples) {
    if (gain == CSAMPLE_ZERO) {
        return;
    }

    for (unsigned int i = 0; i < iNumSamples; ++i) {
        pDest[i] += pSrc[i] * gain;
    }
}

void SampleUtil::addWithRampingGain(CSAMPLE* pDest, const CSAMPLE* pSrc,
        CSAMPLE old_gain, CSAMPLE new_gain, unsigned int iNumSamples) {
    if (old_gain == CSAMPLE_ZERO && new_gain == CSAMPLE_ZERO) {
        return;
    }

    const CSAMPLE gain_delta = (CSAMPLE_PEAK + CSAMPLE_PEAK) * (new_gain - old_gain)
            / iNumSamples;
    CSAMPLE gain = old_gain;
    for (unsigned int i = 0; i < iNumSamples; i += 2, gain += gain_delta) {
        pDest[i] += pSrc[i] * gain;
        pDest[i + 1] += pSrc[i + 1] * gain;
    }
}

// static
void SampleUtil::add2WithGain(CSAMPLE* pDest, const CSAMPLE* pSrc1,
        CSAMPLE gain1, const CSAMPLE* pSrc2, CSAMPLE gain2, unsigned int iNumSamples) {
    if (gain1 == CSAMPLE_ZERO) {
        return addWithGain(pDest, pSrc2, gain2, iNumSamples);
    } else if (gain2 == CSAMPLE_ZERO) {
        return addWithGain(pDest, pSrc1, gain1, iNumSamples);
    }

    for (unsigned int i = 0; i < iNumSamples; ++i) {
        pDest[i] += pSrc1[i] * gain1 + pSrc2[i] * gain2;
    }
}

// static
void SampleUtil::add3WithGain(CSAMPLE* pDest, const CSAMPLE* pSrc1,
        CSAMPLE gain1, const CSAMPLE* pSrc2, CSAMPLE gain2,
        const CSAMPLE* pSrc3, CSAMPLE gain3, unsigned int iNumSamples) {
    if (gain1 == CSAMPLE_ZERO) {
        return add2WithGain(pDest, pSrc2, gain2, pSrc3, gain3, iNumSamples);
    } else if (gain2 == CSAMPLE_ZERO) {
        return add2WithGain(pDest, pSrc1, gain1, pSrc3, gain3, iNumSamples);
    } else if (gain3 == CSAMPLE_ZERO) {
        return add2WithGain(pDest, pSrc1, gain1, pSrc2, gain2, iNumSamples);
    }

    for (unsigned int i = 0; i < iNumSamples; ++i) {
        pDest[i] += pSrc1[i] * gain1 + pSrc2[i] * gain2 + pSrc3[i] * gain3;
    }
}

// static
void SampleUtil::copyWithGain(CSAMPLE* pDest, const CSAMPLE* pSrc, CSAMPLE gain,
        unsigned int iNumSamples) {
    if (pDest == pSrc) {
        return applyGain(pDest, gain, iNumSamples);
    }
    if (gain == CSAMPLE_PEAK) {
        copy(pDest, pSrc, iNumSamples);
        return;
    }
    if (gain == CSAMPLE_ZERO) {
        clear(pDest, iNumSamples);
        return;
    }

    for (unsigned int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc[i] * gain;
    }

    // OR! need to test which fares better
    // copy(pDest, pSrc, iNumSamples);
    // applyGain(pDest, gain);
}

// static
void SampleUtil::copyWithRampingGain(CSAMPLE* pDest, const CSAMPLE* pSrc,
        CSAMPLE old_gain, CSAMPLE new_gain, unsigned int iNumSamples) {
    if (pDest == pSrc) {
        return applyRampingGain(pDest, old_gain, new_gain, iNumSamples);
    }
    if (old_gain == CSAMPLE_PEAK && new_gain == CSAMPLE_PEAK) {
        copy(pDest, pSrc, iNumSamples);
        return;
    }
    if (old_gain == CSAMPLE_ZERO && new_gain == CSAMPLE_ZERO) {
        clear(pDest, iNumSamples);
        return;
    }

    const CSAMPLE delta = (CSAMPLE_PEAK + CSAMPLE_PEAK) * (new_gain - old_gain)
            / iNumSamples;
    CSAMPLE gain = old_gain;
    for (unsigned int i = 0; i < iNumSamples; i += 2, gain += delta) {
        pDest[i] = pSrc[i] * gain;
        pDest[i + 1] = pSrc[i + 1] * gain;
    }

    // OR! need to test which fares better
    // copy(pDest, pSrc, iNumSamples);
    // applyRampingGain(pDest, gain);
}

// static
void SampleUtil::convertS16ToFloat32(CSAMPLE* pDest, const SAMPLE* pSrc,
        unsigned int iNumSamples) {
    for (unsigned int i = 0; i < iNumSamples; ++i) {
        pDest[i] = CSAMPLE(pSrc[i]) / CSAMPLE(SHRT_MAX);
    }
}

// static
void SampleUtil::convertFloat32ToS16(SAMPLE* pDest, const CSAMPLE* pSrc,
        unsigned int iNumSamples) {
    for (unsigned int i = 0; i < iNumSamples; ++i) {
        pDest[i] = SAMPLE(pSrc[i] * CSAMPLE(SHRT_MAX));
    }
}

// static
bool SampleUtil::sumAbsPerChannel(CSAMPLE* pfAbsL, CSAMPLE* pfAbsR,
        const CSAMPLE* pBuffer, unsigned int iNumSamples) {
    CSAMPLE fAbsL = CSAMPLE_ZERO;
    CSAMPLE fAbsR = CSAMPLE_ZERO;
    bool clipped = false;

    for (unsigned int i = 0; i < iNumSamples; i += 2) {
        CSAMPLE absl = fabs(pBuffer[i]);
        if (absl > CSAMPLE_PEAK) {
            clipped = true;
        }
        fAbsL += absl;

        CSAMPLE absr = fabs(pBuffer[i + 1]);
        if (absr > CSAMPLE_PEAK) {
            clipped = true;
        }
        fAbsR += absr;
    }

    *pfAbsL = fAbsL;
    *pfAbsR = fAbsR;
    return clipped;
}

// static
bool SampleUtil::isOutsideRange(CSAMPLE fMax, CSAMPLE fMin,
        const CSAMPLE* pBuffer, unsigned int iNumSamples) {
    for (unsigned int i = 0; i < iNumSamples; ++i) {
        CSAMPLE sample = pBuffer[i];
        if (sample > fMax) {
            return true;
        } else if (sample < fMin) {
            return true;
        }
    }
    return false;
}

// static
void SampleUtil::copyClampBuffer(CSAMPLE* pDest, const CSAMPLE* pSrc,
        unsigned int iNumSamples) {
    for (unsigned int i = 0; i < iNumSamples; ++i) {
        pDest[i] = clampSample(pSrc[i]);
    }
}

// static
void SampleUtil::interleaveBuffer(CSAMPLE* pDest, const CSAMPLE* pSrc1,
        const CSAMPLE* pSrc2, unsigned int iNumSamples) {
    for (unsigned int i = 0; i < iNumSamples; ++i) {
        pDest[i * 2] = pSrc1[i];
        pDest[i * 2 + 1] = pSrc2[i];
    }
}

// static
void SampleUtil::deinterleaveBuffer(CSAMPLE* pDest1, CSAMPLE* pDest2,
        const CSAMPLE* pSrc, unsigned int iNumSamples) {
    for (unsigned int i = 0; i < iNumSamples; ++i) {
        pDest1[i] = pSrc[i * 2];
        pDest2[i] = pSrc[i * 2 + 1];
    }
}

// static
void SampleUtil::linearCrossfadeBuffers(CSAMPLE* pDest,
        const CSAMPLE* pSrcFadeOut, const CSAMPLE* pSrcFadeIn,
        unsigned int iNumSamples) {
    CSAMPLE cross_mix = CSAMPLE_ZERO;
    CSAMPLE cross_inc = (CSAMPLE_PEAK + CSAMPLE_PEAK)
            / static_cast<double>(iNumSamples);
    for (unsigned int i = 0; i + 1 < iNumSamples; i += 2) {
        pDest[i] = pSrcFadeIn[i] * cross_mix
                + pSrcFadeOut[i] * (CSAMPLE_PEAK - cross_mix);
        pDest[i + 1] = pSrcFadeIn[i + 1] * cross_mix
                + pSrcFadeOut[i + 1] * (CSAMPLE_PEAK - cross_mix);
        cross_mix += cross_inc;
    }
}

// static
void SampleUtil::mixStereoToMono(CSAMPLE* pDest, const CSAMPLE* pSrc,
        unsigned int iNumSamples) {
    const CSAMPLE mixScale = CSAMPLE_PEAK / (CSAMPLE_PEAK + CSAMPLE_PEAK);
    for (unsigned int i = 0; i + 1 < iNumSamples; i += 2) {
        pDest[i] = (pSrc[i] + pSrc[i + 1]) * mixScale;
        pDest[i + 1] = pDest[i];
    }
}

