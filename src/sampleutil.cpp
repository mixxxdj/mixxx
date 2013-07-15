// sampleutil.cpp
// Created 10/5/2009 by RJ Ryan (rryan@mit.edu)

#ifdef __WINDOWS__
#pragma intrinsic(fabs)sc
typedef __int64 int64_t;
typedef __int32 int32_t;
#endif

#include <QtDebug>

#include "sampleutil.h"

// static
CSAMPLE* SampleUtil::alloc(int size) {
    // TODO(XXX) align the array
    return new CSAMPLE[size];
}

void SampleUtil::free(CSAMPLE* pBuffer) {
    delete [] pBuffer;
}

// static
void SampleUtil::applyGain(CSAMPLE* pBuffer,
                           CSAMPLE gain, int iNumSamples) {
    if (gain == 1.0f)
        return;
    if (gain == 0.0f) {
        memset(pBuffer, 0, sizeof(pBuffer[0]) * iNumSamples);
        return;
    }

    for (int i = 0; i < iNumSamples; ++i) {
        pBuffer[i] *= gain;
    }
}

// static
void SampleUtil::applyRampingGain(CSAMPLE* pBuffer,
                                  CSAMPLE gain1, CSAMPLE gain2, int iNumSamples) {
    if (gain1 == 1.0f && gain2 == 1.0f)
        return;
    if (gain1 == 0.0f && gain2 == 0.0f) {
        memset(pBuffer, 0, sizeof(pBuffer[0]) * iNumSamples);
        return;
    }

    const CSAMPLE delta = 2.0 * (gain2 - gain1) / iNumSamples;
    CSAMPLE gain = gain1;
    for (int i = 0; i < iNumSamples; i += 2, gain += delta) {
        pBuffer[i] *= gain;
        pBuffer[i + 1] *= gain;
    }
}

// static
void SampleUtil::applyAlternatingGain(CSAMPLE* pBuffer,
                                      CSAMPLE gain1, CSAMPLE gain2,
                                      int iNumSamples) {
    // This handles gain1 == 1.0 && gain2 == 1.0f as well.
    if (gain1 == gain2) {
        return applyGain(pBuffer, gain1, iNumSamples);
    }

    for (int i = 0; i < iNumSamples; i += 2) {
        pBuffer[i] *= gain1;
        pBuffer[i+1] *= gain2;
    }
}

// static
void SampleUtil::addWithGain(CSAMPLE* pDest, const CSAMPLE* pSrc,
                             CSAMPLE gain, int iNumSamples) {
    if (gain == 0.0f)
        return;

    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] += pSrc[i] * gain;
    }
}

// static
void SampleUtil::add2WithGain(CSAMPLE* pDest,
                              const CSAMPLE* pSrc1, CSAMPLE gain1,
                              const CSAMPLE* pSrc2, CSAMPLE gain2,
                              int iNumSamples) {
    if (gain1 == 0.0f) {
        return addWithGain(pDest, pSrc2, gain2, iNumSamples);
    } else if (gain2 == 0.0f) {
        return addWithGain(pDest, pSrc1, gain1, iNumSamples);
    }

    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] += pSrc1[i] * gain1 + pSrc2[i] * gain2;
    }
}

// static
void SampleUtil::add3WithGain(CSAMPLE* pDest,
                              const CSAMPLE* pSrc1, CSAMPLE gain1,
                              const CSAMPLE* pSrc2, CSAMPLE gain2,
                              const CSAMPLE* pSrc3, CSAMPLE gain3,
                              int iNumSamples) {
    if (gain1 == 0.0f) {
        return add2WithGain(pDest, pSrc2, gain2, pSrc3, gain3, iNumSamples);
    } else if (gain2 == 0.0f) {
        return add2WithGain(pDest, pSrc1, gain1, pSrc3, gain3, iNumSamples);
    } else if (gain3 == 0.0f) {
        return add2WithGain(pDest, pSrc1, gain1, pSrc2, gain2, iNumSamples);
    }

    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] += pSrc1[i] * gain1 + pSrc2[i] * gain2 + pSrc3[i] * gain3;
    }
}

// static
void SampleUtil::copyWithGain(CSAMPLE* pDest, const CSAMPLE* pSrc,
                              CSAMPLE gain, int iNumSamples) {
    if (pDest == pSrc) {
        return applyGain(pDest, gain, iNumSamples);
    }
    if (gain == 1.0f) {
        memcpy(pDest, pSrc, sizeof(pDest[0]) * iNumSamples);
        return;
    }
    if (gain == 0.0f) {
        memset(pDest, 0, sizeof(pDest[0]) * iNumSamples);
        return;
    }

    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc[i] * gain;
    }

    // OR! need to test which fares better
    // memcpy(pDest, pSrc, sizeof(pDest[0]) * iNumSamples);
    // applyGain(pDest, gain);
}

// static
void SampleUtil::copyWithRampingGain(CSAMPLE* pDest, const CSAMPLE* pSrc,
                                     CSAMPLE gain1, CSAMPLE gain2, int iNumSamples) {
    if (pDest == pSrc) {
        return applyRampingGain(pDest, gain1, gain2, iNumSamples);
    }
    if (gain1 == 1.0f && gain2 == 1.0f) {
        memcpy(pDest, pSrc, sizeof(pDest[0]) * iNumSamples);
        return;
    }
    if (gain1 == 0.0f && gain2 == 0.0f) {
        memset(pDest, 0, sizeof(pDest[0]) * iNumSamples);
        return;
    }

    const CSAMPLE delta = 2.0 * (gain2 - gain1) / iNumSamples;
    CSAMPLE gain = gain1;
    for (int i = 0; i < iNumSamples; i += 2, gain += delta) {
        pDest[i] = pSrc[i] * gain;
        pDest[i + 1] = pSrc[i + 1] * gain;
    }

    // OR! need to test which fares better
    // memcpy(pDest, pSrc, sizeof(pDest[0]) * iNumSamples);
    // applyGain(pDest, gain);
}

// static
void SampleUtil::copy2WithGain(CSAMPLE* pDest,
                               const CSAMPLE* pSrc1, CSAMPLE gain1,
                               const CSAMPLE* pSrc2, CSAMPLE gain2,
                               int iNumSamples) {
    if (gain1 == 0.0f) {
        return copyWithGain(pDest, pSrc2, gain2, iNumSamples);
    }
    if (gain2 == 0.0f) {
        return copyWithGain(pDest, pSrc1, gain1, iNumSamples);
    }

    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc1[i] * gain1 + pSrc2[i] * gain2;
    }
}

// static
void SampleUtil::copy3WithGain(CSAMPLE* pDest,
                               const CSAMPLE* pSrc1, CSAMPLE gain1,
                               const CSAMPLE* pSrc2, CSAMPLE gain2,
                               const CSAMPLE* pSrc3, CSAMPLE gain3,
                               int iNumSamples) {
    if (gain1 == 0.0f) {
        return copy2WithGain(pDest, pSrc2, gain2, pSrc3, gain3, iNumSamples);
    }
    if (gain2 == 0.0f) {
        return copy2WithGain(pDest, pSrc1, gain1, pSrc3, gain3, iNumSamples);
    }
    if (gain3 == 0.0f) {
        return copy2WithGain(pDest, pSrc1, gain1, pSrc2, gain2, iNumSamples);
    }

    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc1[i] * gain1 + pSrc2[i] * gain2 + pSrc3[i] * gain3;
    }
}

// static
void SampleUtil::copy3WithRampingGain(CSAMPLE* pDest,
                               const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                               const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                               const CSAMPLE* pSrc3, CSAMPLE gain3in, CSAMPLE gain3out,
                               int iNumSamples) {
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    const CSAMPLE delta3 = 2.0 * (gain3out - gain3in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    CSAMPLE gain2 = gain2in;
    CSAMPLE gain3 = gain3in;
    for (int i = 0; i < iNumSamples; i += 2, gain1 += delta1, gain2 += delta2, gain3 += delta3) {
        pDest[i] = pSrc1[i] * gain1 + pSrc2[i] * gain2 + pSrc3[i] * gain3;
        pDest[i + 1] = pSrc1[i + 1] * gain1 + pSrc2[i + 1] * gain2 + pSrc3[i + 1] * gain3;
    }
}

// static
void SampleUtil::copy4WithGain(CSAMPLE* pDest,
                               const CSAMPLE* pSrc1, CSAMPLE gain1,
                               const CSAMPLE* pSrc2, CSAMPLE gain2,
                               const CSAMPLE* pSrc3, CSAMPLE gain3,
                               const CSAMPLE* pSrc4, CSAMPLE gain4,
                               int iNumSamples) {
    if (gain1 == 0.0f) {
        return copy3WithGain(pDest, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, iNumSamples);
    }
    if (gain2 == 0.0f) {
        return copy3WithGain(pDest, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, iNumSamples);
    }
    if (gain3 == 0.0f) {
        return copy3WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, iNumSamples);
    }
    if (gain4 == 0.0f) {
        return copy3WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, iNumSamples);
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc1[i] * gain1 + pSrc2[i] * gain2 + pSrc3[i] * gain3 + pSrc4[i] * gain4;
    }
}

// static
void SampleUtil::copy5WithGain(CSAMPLE* pDest,
                               const CSAMPLE* pSrc1, CSAMPLE gain1,
                               const CSAMPLE* pSrc2, CSAMPLE gain2,
                               const CSAMPLE* pSrc3, CSAMPLE gain3,
                               const CSAMPLE* pSrc4, CSAMPLE gain4,
                               const CSAMPLE* pSrc5, CSAMPLE gain5,
                               int iNumSamples) {
    if (gain1 == 0.0f) {
        return copy4WithGain(pDest, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, iNumSamples);
    }
    if (gain2 == 0.0f) {
        return copy4WithGain(pDest, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, iNumSamples);
    }
    if (gain3 == 0.0f) {
        return copy4WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, iNumSamples);
    }
    if (gain4 == 0.0f) {
        return copy4WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, iNumSamples);
    }
    if (gain5 == 0.0f) {
        return copy4WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, iNumSamples);
    }

    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc1[i] * gain1 + pSrc2[i] * gain2 + pSrc3[i] * gain3 + pSrc4[i] * gain4 + pSrc5[i] * gain5;
    }
}

// static
void SampleUtil::copy6WithGain(CSAMPLE* pDest,
                               const CSAMPLE* pSrc1, CSAMPLE gain1,
                               const CSAMPLE* pSrc2, CSAMPLE gain2,
                               const CSAMPLE* pSrc3, CSAMPLE gain3,
                               const CSAMPLE* pSrc4, CSAMPLE gain4,
                               const CSAMPLE* pSrc5, CSAMPLE gain5,
                               const CSAMPLE* pSrc6, CSAMPLE gain6,
                               int iNumSamples) {
    if (gain1 == 0.0f) {
        return copy5WithGain(pDest, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4,
                             pSrc5, gain5, pSrc6, gain6, iNumSamples);
    }
    if (gain2 == 0.0f) {
        return copy5WithGain(pDest, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4,
                             pSrc5, gain5, pSrc6, gain6, iNumSamples);
    }
    if (gain3 == 0.0f) {
        return copy5WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4,
                             pSrc5, gain5, pSrc6, gain6, iNumSamples);
    }
    if (gain4 == 0.0f) {
        return copy5WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3,
                             pSrc5, gain5, pSrc6, gain6, iNumSamples);
    }
    if (gain5 == 0.0f) {
        return copy5WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3,
                             pSrc4, gain4, pSrc6, gain6, iNumSamples);
    }
    if (gain6 == 0.0f) {
        return copy5WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3,
                             pSrc4, gain4, pSrc5, gain5, iNumSamples);
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc1[i] * gain1 + pSrc2[i] * gain2 + pSrc3[i] * gain3 +
                pSrc4[i] * gain4 + pSrc5[i] * gain5 + pSrc6[i] * gain6;
    }
}

// static
void SampleUtil::copy7WithGain(CSAMPLE* pDest,
                               const CSAMPLE* pSrc1, CSAMPLE gain1,
                               const CSAMPLE* pSrc2, CSAMPLE gain2,
                               const CSAMPLE* pSrc3, CSAMPLE gain3,
                               const CSAMPLE* pSrc4, CSAMPLE gain4,
                               const CSAMPLE* pSrc5, CSAMPLE gain5,
                               const CSAMPLE* pSrc6, CSAMPLE gain6,
                               const CSAMPLE* pSrc7, CSAMPLE gain7,
                               int iNumSamples) {
    if (gain1 == 0.0f) {
        return copy6WithGain(pDest, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4,
                             pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, iNumSamples);
    }
    if (gain2 == 0.0f) {
        return copy6WithGain(pDest, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4,
                             pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, iNumSamples);
    }
    if (gain3 == 0.0f) {
        return copy6WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4,
                             pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, iNumSamples);
    }
    if (gain4 == 0.0f) {
        return copy6WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3,
                             pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, iNumSamples);
    }
    if (gain5 == 0.0f) {
        return copy6WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3,
                             pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, iNumSamples);
    }
    if (gain6 == 0.0f) {
        return copy6WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3,
                             pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, iNumSamples);
    }
    if (gain7 == 0.0f) {
        return copy6WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3,
                             pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, iNumSamples);
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc1[i] * gain1 + pSrc2[i] * gain2 + pSrc3[i] * gain3 +
                pSrc4[i] * gain4 + pSrc5[i] * gain5 + pSrc6[i] * gain6 + pSrc7[i] * gain7;
    }
}

// static
void SampleUtil::convert(CSAMPLE* pDest, const SAMPLE* pSrc,
                         int iNumSamples) {
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc[i];
    }
}

// static
void SampleUtil::sumAbsPerChannel(CSAMPLE* pfAbsL, CSAMPLE* pfAbsR,
                                  const CSAMPLE* pBuffer, int iNumSamples) {
    CSAMPLE fAbsL = 0.0f;
    CSAMPLE fAbsR = 0.0f;

    for (int i = 0; i < iNumSamples; i += 2) {
        fAbsL += fabs(pBuffer[i]);
        fAbsR += fabs(pBuffer[i+1]);
    }

    *pfAbsL = fAbsL;
    *pfAbsR = fAbsR;
}

// static
bool SampleUtil::isOutsideRange(CSAMPLE fMax, CSAMPLE fMin,
                                const CSAMPLE* pBuffer, int iNumSamples) {
    for (int i = 0; i < iNumSamples; ++i) {
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
bool SampleUtil::copyClampBuffer(CSAMPLE fMax, CSAMPLE fMin,
                                 CSAMPLE* pDest, const CSAMPLE* pSrc,
                                 int iNumSamples) {
    bool clamped = false;
    if (pSrc == pDest) {
        for (int i = 0; i < iNumSamples; ++i) {
            CSAMPLE sample = pSrc[i];
            if (sample > fMax) {
                clamped = true;
                pDest[i] = fMax;
            } else if (sample < fMin) {
                clamped = true;
                pDest[i] = fMin;
            }
        }
    } else {
        for (int i = 0; i < iNumSamples; ++i) {
            CSAMPLE sample = pSrc[i];
            if (sample > fMax) {
                sample = fMax;
                clamped = true;
            } else if (sample < fMin) {
                sample = fMin;
                clamped = true;
            }
            pDest[i] = sample;
        }
    }
    return clamped;
}

// static
void SampleUtil::interleaveBuffer(CSAMPLE* pDest,
                                  const CSAMPLE* pSrc1, const CSAMPLE* pSrc2,
                                  int iNumSamples) {
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[2*i] = pSrc1[i];
        pDest[2*i+1] = pSrc2[i];
    }
}

// static
void SampleUtil::deinterleaveBuffer(CSAMPLE* pDest1, CSAMPLE* pDest2,
                                  const CSAMPLE* pSrc, int iNumSamples) {
    for (int i = 0; i < iNumSamples; ++i) {
        pDest1[i] = pSrc[i*2];
        pDest2[i] = pSrc[i*2+1];
    }
}

// static
void SampleUtil::linearCrossfadeBuffers(CSAMPLE* pDest,
                                        const CSAMPLE* pSrcFadeOut,
                                        const CSAMPLE* pSrcFadeIn,
                                        int iNumSamples) {
    double cross_mix = 0.0;
    double cross_inc = 2.0 / static_cast<double>(iNumSamples);
    for (int i = 0; i + 1 < iNumSamples; i += 2) {
        pDest[i] = pSrcFadeIn[i] * cross_mix
                   + pSrcFadeOut[i] * (1.0 - cross_mix);
        pDest[i + 1] = pSrcFadeIn[i + 1] * cross_mix
                       + pSrcFadeOut[i + 1] * (1.0 - cross_mix);
        cross_mix += cross_inc;
    }
}
