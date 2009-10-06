// sampleutil.cpp
// Created 10/5/2009 by RJ Ryan (rryan@mit.edu)

#include <xmmintrin.h>

#include <QtDebug>

#include "sampleutil.h"

bool SampleUtil::m_sOptimizationsOn = true;

// static
void SampleUtil::applyGain(CSAMPLE* pBuffer,
                           CSAMPLE gain, int iNumSamples) {
    if (gain == 1.0f)
        return;
    if (gain == 0.0f) {
        memset(pBuffer, 0, sizeof(pBuffer[0]) * iNumSamples);
        return;
    }

    if (m_sOptimizationsOn) {
        return sseApplyGain(pBuffer, gain, iNumSamples);
    }

    for (int i = 0; i < iNumSamples; ++i) {
        pBuffer[i] *= gain;
    }
}

// static
void SampleUtil::sseApplyGain(CSAMPLE* pBuffer,
                              CSAMPLE gain, int iNumSamples) {
#ifdef __SSE__
    __m128 vSamples;
    __m128 vGain = _mm_set1_ps(gain);
    while (iNumSamples >= 4) {
        vSamples = _mm_load_ps(pBuffer);
        vSamples = _mm_mul_ps(vSamples, vGain);
        _mm_store_ps(pBuffer, vSamples);

        iNumSamples -= 4;
        pBuffer += 4;
    }
    if (iNumSamples > 0) {
        qDebug() << "Not div by 4";
    }
    while (iNumSamples > 0) {
        *pBuffer = *pBuffer * gain;
        pBuffer++;
        iNumSamples--;
    }
#endif
}

// static
void SampleUtil::applyAlternatingGain(CSAMPLE* pBuffer,
                                      CSAMPLE gain1, CSAMPLE gain2,
                                      int iNumSamples) {
    Q_ASSERT(iNumSamples % 2 == 0);
    // This handles gain1 == 1.0 && gain2 == 1.0f as well.
    if (gain1 == gain2) {
        return applyGain(pBuffer, gain1, iNumSamples);
    }
    if (m_sOptimizationsOn)
        return sseApplyAlternatingGain(pBuffer, gain1, gain2, iNumSamples);

    for (int i = 0; i < iNumSamples/2; i += 2) {
        pBuffer[i] *= gain1;
        pBuffer[i+1] *= gain2;
    }
}

// static
void SampleUtil::sseApplyAlternatingGain(CSAMPLE* pBuffer,
                                         CSAMPLE gain1, CSAMPLE gain2,
                                         int iNumSamples) {
#ifdef __SSE__
    __m128 vSamples;
    __m128 vGain = _mm_set_ps(gain1, gain2, gain1, gain2);
    while (iNumSamples >= 4) {
        vSamples = _mm_load_ps(pBuffer);
        vSamples = _mm_mul_ps(vSamples, vGain);
        _mm_store_ps(pBuffer, vSamples);

        iNumSamples -= 4;
        pBuffer += 4;
    }
    if (iNumSamples > 0) {
        qDebug() << "Not div by 4";
    }
    while (iNumSamples > 0) {
        *pBuffer = *pBuffer * gain1;
        pBuffer++;
        iNumSamples--;
        *pBuffer = *pBuffer * gain2;
        pBuffer++;
        iNumSamples--;
    }
#endif
}

// static
void SampleUtil::addWithGain(CSAMPLE* pDest, const CSAMPLE* pSrc,
                             CSAMPLE gain, int iNumSamples) {
    if (gain == 0.0f)
        return;
    if (m_sOptimizationsOn)
        return sseAddWithGain(pDest, pSrc, gain, iNumSamples);

    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] += pSrc[i] * gain;
    }
}

// static
void SampleUtil::sseAddWithGain(CSAMPLE* pDest, const CSAMPLE* pSrc,
                             CSAMPLE gain, int iNumSamples) {
#ifdef __SSE__
    __m128 vSrcSamples;
    __m128 vDestSamples;
    __m128 vGain = _mm_set1_ps(gain);
    while (iNumSamples >= 4) {
        vSrcSamples = _mm_load_ps(pSrc);
        vSrcSamples = _mm_mul_ps(vSrcSamples, vGain);
        vDestSamples = _mm_load_ps(pDest);
        _mm_store_ps(pDest, _mm_add_ps(vDestSamples, vSrcSamples));
        iNumSamples -= 4;
        pDest += 4;
        pSrc += 4;
    }
    if (iNumSamples > 0) {
        qDebug() << "Not div by 4";
    }
    while (iNumSamples > 0) {
        *pDest = *pDest + *pSrc * gain;
        pDest++;
        pSrc++;
        iNumSamples--;
    }
#endif
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

    if (m_sOptimizationsOn)
        return sseAdd2WithGain(pDest, pSrc1, gain1, pSrc2, gain2, iNumSamples);

    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] += pSrc1[i] * gain1 + pSrc2[i] * gain2;
    }
}

// static
void SampleUtil::sseAdd2WithGain(CSAMPLE* pDest,
                                 const CSAMPLE* pSrc1, CSAMPLE gain1,
                                 const CSAMPLE* pSrc2, CSAMPLE gain2,
                                 int iNumSamples) {
#ifdef __SSE__
    __m128 vSrc1Samples;
    __m128 vSrc2Samples;
    __m128 vDestSamples;
    __m128 vGain1 = _mm_set1_ps(gain1);
    __m128 vGain2 = _mm_set1_ps(gain2);
    while (iNumSamples >= 4) {
        vSrc1Samples = _mm_load_ps(pSrc1);
        vSrc1Samples = _mm_mul_ps(vSrc1Samples, vGain1);
        vSrc2Samples = _mm_load_ps(pSrc2);
        vSrc2Samples = _mm_mul_ps(vSrc2Samples, vGain2);
        vDestSamples = _mm_load_ps(pDest);
        vDestSamples = _mm_add_ps(vDestSamples, vSrc1Samples);
        vDestSamples = _mm_add_ps(vDestSamples, vSrc2Samples);
        _mm_store_ps(pDest, vDestSamples);
        iNumSamples -= 4;
        pDest += 4;
        pSrc1 += 4;
        pSrc2 += 4;
    }
    if (iNumSamples > 0) {
        qDebug() << "Not div by 4";
    }
    while (iNumSamples > 0) {
        *pDest = *pDest + *pSrc1 * gain1 + *pSrc2 * gain2;
        pDest++;
        pSrc1++;
        pSrc2++;
        iNumSamples--;
    }
#endif
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

    if (m_sOptimizationsOn)
        return sseAdd3WithGain(pDest, pSrc1, gain1, pSrc2, gain2,
                               pSrc3, gain3, iNumSamples);

    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] += pSrc1[i] * gain1 + pSrc2[i] * gain2 + pSrc3[i] * gain3;
    }
}

// static
void SampleUtil::sseAdd3WithGain(CSAMPLE* pDest,
                                 const CSAMPLE* pSrc1, CSAMPLE gain1,
                                 const CSAMPLE* pSrc2, CSAMPLE gain2,
                                 const CSAMPLE* pSrc3, CSAMPLE gain3,
                                 int iNumSamples) {
#ifdef __SSE__
    __m128 vSrc1Samples;
    __m128 vSrc2Samples;
    __m128 vSrc3Samples;
    __m128 vDestSamples;
    __m128 vGain1 = _mm_set1_ps(gain1);
    __m128 vGain2 = _mm_set1_ps(gain2);
    __m128 vGain3 = _mm_set1_ps(gain3);
    while (iNumSamples >= 4) {
        vSrc1Samples = _mm_load_ps(pSrc1);
        vSrc1Samples = _mm_mul_ps(vSrc1Samples, vGain1);
        vSrc2Samples = _mm_load_ps(pSrc2);
        vSrc2Samples = _mm_mul_ps(vSrc2Samples, vGain2);
        vSrc3Samples = _mm_load_ps(pSrc3);
        vSrc3Samples = _mm_mul_ps(vSrc3Samples, vGain3);
        vDestSamples = _mm_load_ps(pDest);
        vDestSamples = _mm_add_ps(vDestSamples, vSrc1Samples);
        vDestSamples = _mm_add_ps(vDestSamples, vSrc2Samples);
        vDestSamples = _mm_add_ps(vDestSamples, vSrc3Samples);
        _mm_store_ps(pDest, vDestSamples);
        iNumSamples -= 4;
        pDest += 4;
        pSrc1 += 4;
        pSrc2 += 4;
        pSrc3 += 4;
    }
    if (iNumSamples > 0) {
        qDebug() << "Not div by 4";
    }
    while (iNumSamples > 0) {
        *pDest = *pDest + *pSrc1 * gain1 + *pSrc2 * gain2 + *pSrc3 * gain3;
        pDest++;
        pSrc1++;
        pSrc2++;
        pSrc3++;
        iNumSamples--;
    }
#endif
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
    }    if (m_sOptimizationsOn) {
        return sseCopyWithGain(pDest, pSrc, gain, iNumSamples);
    }

    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc[i] * gain;
    }

    // OR! need to test which fares better
    // memcpy(pDest, pSrc, sizeof(pDest[0]) * iNumSamples);
    // applyGain(pDest, gain);
}

// static
void SampleUtil::sseCopyWithGain(CSAMPLE* pDest, const CSAMPLE* pSrc,
                                 CSAMPLE gain, int iNumSamples) {
#ifdef __SSE__
    __m128 vSrcSamples;
    __m128 vGain = _mm_set1_ps(gain);
    while (iNumSamples >= 4) {
        vSrcSamples = _mm_load_ps(pSrc);
        vSrcSamples = _mm_mul_ps(vSrcSamples, vGain);
        _mm_store_ps(pDest, vSrcSamples);
        iNumSamples -= 4;
        pDest += 4;
        pSrc += 4;
    }
    if (iNumSamples > 0) {
        qDebug() << "Not div by 4";
    }
    while (iNumSamples > 0) {
        *pDest = *pSrc * gain;
        pDest++;
        pSrc++;
        iNumSamples--;
    }
#endif
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
    if (m_sOptimizationsOn) {
        return sseCopy2WithGain(pDest, pSrc1, gain1, pSrc2, gain2, iNumSamples);
    }

    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc1[i] * gain1 + pSrc2[i] * gain2;
    }
}

// static
void SampleUtil::sseCopy2WithGain(CSAMPLE* pDest,
                                  const CSAMPLE* pSrc1, CSAMPLE gain1,
                                  const CSAMPLE* pSrc2, CSAMPLE gain2,
                                  int iNumSamples) {
#ifdef __SSE__
    __m128 vSrc1Samples;
    __m128 vSrc2Samples;
    __m128 vGain1 = _mm_set1_ps(gain1);
    __m128 vGain2 = _mm_set1_ps(gain2);
    while (iNumSamples >= 4) {
        vSrc1Samples = _mm_load_ps(pSrc1);
        vSrc1Samples = _mm_mul_ps(vSrc1Samples, vGain1);
        vSrc2Samples = _mm_load_ps(pSrc2);
        vSrc2Samples = _mm_mul_ps(vSrc2Samples, vGain2);
        _mm_store_ps(pDest, _mm_add_ps(vSrc1Samples, vSrc2Samples));
        iNumSamples -= 4;
        pDest += 4;
        pSrc1 += 4;
        pSrc2 += 4;
    }
    if (iNumSamples > 0) {
        qDebug() << "Not div by 4";
    }
    while (iNumSamples > 0) {
        *pDest = *pSrc1 * gain1 + *pSrc2 * gain2;
        pDest++;
        pSrc1++;
        pSrc2++;
        iNumSamples--;
    }
#endif
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
    if (m_sOptimizationsOn) {
        return sseCopy3WithGain(pDest, pSrc1, gain1, pSrc2, gain2,
                                pSrc3, gain3, iNumSamples);
    }

    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc1[i] * gain1 + pSrc2[i] * gain2 + pSrc3[i] * gain3;
    }
}

// static
void SampleUtil::sseCopy3WithGain(CSAMPLE* pDest,
                                  const CSAMPLE* pSrc1, CSAMPLE gain1,
                                  const CSAMPLE* pSrc2, CSAMPLE gain2,
                                  const CSAMPLE* pSrc3, CSAMPLE gain3,
                                  int iNumSamples) {
#ifdef __SSE__
    __m128 vSrc1Samples;
    __m128 vSrc2Samples;
    __m128 vSrc3Samples;
    __m128 vGain1 = _mm_set1_ps(gain1);
    __m128 vGain2 = _mm_set1_ps(gain2);
    __m128 vGain3 = _mm_set1_ps(gain3);
    while (iNumSamples >= 4) {
        vSrc1Samples = _mm_load_ps(pSrc1);
        vSrc1Samples = _mm_mul_ps(vSrc1Samples, vGain1);
        vSrc2Samples = _mm_load_ps(pSrc2);
        vSrc2Samples = _mm_mul_ps(vSrc2Samples, vGain2);
        vSrc3Samples = _mm_load_ps(pSrc3);
        vSrc3Samples = _mm_mul_ps(vSrc3Samples, vGain3);

        vSrc1Samples = _mm_add_ps(vSrc1Samples, vSrc2Samples);
        vSrc1Samples = _mm_add_ps(vSrc1Samples, vSrc3Samples);
        _mm_store_ps(pDest, vSrc1Samples);
        iNumSamples -= 4;
        pDest += 4;
        pSrc1 += 4;
        pSrc2 += 4;
        pSrc3 += 4;
    }
    if (iNumSamples > 0) {
        qDebug() << "Not div by 4";
    }
    while (iNumSamples > 0) {
        *pDest = *pSrc1 * gain1 + *pSrc2 * gain2 + *pSrc3 * gain3;
        pDest++;
        pSrc1++;
        pSrc2++;
        pSrc3++;
        iNumSamples--;
    }
#endif
}

// static
void SampleUtil::convert(CSAMPLE* pDest, const SAMPLE* pSrc,
                         int iNumSamples) {
    if (m_sOptimizationsOn) {
        return sseConvert(pDest, pSrc, iNumSamples);
    }

    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc[i];
    }
}

// static
void SampleUtil::sseConvert(CSAMPLE* pDest, const SAMPLE* pSrc,
                            int iNumSamples) {
#ifdef __SSE__
    __m64 vSrcSamples;
    __m128 vDestSamples;
    while (iNumSamples >= 4) {
        vSrcSamples = *((__m64*)pSrc); // ????
        vDestSamples = _mm_cvtpi16_ps(vSrcSamples);
        _mm_store_ps(pDest, vDestSamples);
        iNumSamples -= 4;
        pDest += 4;
        pSrc += 4;
    }
    if (iNumSamples > 0) {
        qDebug() << "Not div by 4";
    }
    while (iNumSamples > 0) {
        *pDest = *pSrc;
        pDest++;
        pSrc++;
        iNumSamples--;
    }
#endif
}

// static
void SampleUtil::sumAbsPerChannel(CSAMPLE* pfAbsL, CSAMPLE* pfAbsR,
                                  const CSAMPLE* pBuffer, int iNumSamples) {
    Q_ASSERT(iNumSamples % 2 == 0);
    if (m_sOptimizationsOn) {
        return sseSumAbsPerChannel(pfAbsL, pfAbsR, pBuffer, iNumSamples);
    }

    CSAMPLE fAbsL = 0.0f;
    CSAMPLE fAbsR = 0.0f;

    for (int i = 0; i < iNumSamples/2; ++i) {
        fAbsL += fabs(pBuffer[i*2]);
        fAbsR += fabs(pBuffer[i*2+1]);
    }

    *pfAbsL = fAbsL;
    *pfAbsR = fAbsR;
}

void SampleUtil::sseSumAbsPerChannel(CSAMPLE* pfAbsL, CSAMPLE* pfAbsR,
                                     const CSAMPLE* pBuffer, int iNumSamples) {
#ifdef __SSE__
    CSAMPLE fAbsL = 0.0f;
    CSAMPLE fAbsR = 0.0f;

    __m128 vSrcSamples;
    __m128 vSum = _mm_setzero_ps();
    // This mask will clear the sign bit of a float if ANDed
    __m128 vSignMask = _mm_set1_ps(0x7fffffff);

    while (iNumSamples >= 4) {
        vSrcSamples = _mm_load_ps(pBuffer);
        vSrcSamples = _mm_and_ps(vSrcSamples, vSignMask);
        vSum = _mm_add_ps(vSum, vSrcSamples);
        iNumSamples -= 4;
        pBuffer += 4;
    }
    CSAMPLE result[4]; // TODO(XXX) alignment
    _mm_store_ps(result, vSum);
    fAbsL = result[0] + result[2];
    fAbsR = result[1] + result[3];
    if (iNumSamples > 0) {
        qDebug() << "Not div by 4";
    }
    while (iNumSamples >= 2) {
        fAbsL += fabs(*pBuffer++);
        fAbsR += fabs(*pBuffer++);
        iNumSamples -= 2;
    }

    *pfAbsL = fAbsL;
    *pfAbsR = fAbsR;
#endif
}

// static
bool SampleUtil::isOutsideRange(CSAMPLE fMax, CSAMPLE fMin,
                                const CSAMPLE* pBuffer, int iNumSamples) {
    if (m_sOptimizationsOn) {
        return sseIsOutsideRange(fMax, fMin, pBuffer, iNumSamples);
    }

    bool clamped = false;;
    for (int i = 0; i < iNumSamples; ++i) {
        CSAMPLE sample = pBuffer[i];
        if (sample > fMax) {
            clamped = true;
        } else if (sample < fMin) {
            clamped = true;
        }
    }
    return clamped;
}

// static
bool SampleUtil::sseIsOutsideRange(CSAMPLE fMax, CSAMPLE fMin,
                                   const CSAMPLE* pBuffer, int iNumSamples) {
    bool outside = false;
#ifdef __SSE__
    __m128 vSrcSamples;
    __m128 vClamped = _mm_setzero_ps();
    __m128 vMax = _mm_set1_ps(fMax);
    __m128 vMin = _mm_set1_ps(fMin);
    while (iNumSamples >= 4) {
        vSrcSamples = _mm_load_ps(pBuffer);
        vClamped = _mm_or_ps(vClamped, _mm_cmplt_ps(vSrcSamples, vMin));
        vClamped = _mm_or_ps(vClamped, _mm_cmpgt_ps(vSrcSamples, vMax));
        iNumSamples -= 4;
        pBuffer += 4;
    }
    CSAMPLE clamp[4]; // TODO(XXX) alignment
    _mm_store_ps(clamp, vClamped);
    if (clamp[0] != 0 || clamp[1] != 0 ||
        clamp[2] != 0 || clamp[3] != 0) {
        outside = true;
    }
    if (iNumSamples > 0) {
        qDebug() << "Not div by 4";
    }
    while (iNumSamples > 0) {
        CSAMPLE sample = *pBuffer;
        if (sample > fMax) {
            outside = true;
        } else if (sample < fMin) {
            outside = true;
        }
        pBuffer++;
        iNumSamples--;
    }
#endif
    return outside;
}

// static
bool SampleUtil::copyClampBuffer(CSAMPLE fMax, CSAMPLE fMin,
                                 CSAMPLE* pDest, const CSAMPLE* pSrc,
                                 int iNumSamples) {
    if (m_sOptimizationsOn) {
        return sseCopyClampBuffer(fMax, fMin, pDest, pSrc, iNumSamples);
    }

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
bool SampleUtil::sseCopyClampBuffer(CSAMPLE fMax, CSAMPLE fMin,
                                    CSAMPLE* pDest, const CSAMPLE* pSrc,
                                    int iNumSamples) {
    bool clamped = false;
#ifdef __SSE__
    __m128 vSrcSamples;
    __m128 vClamped = _mm_setzero_ps();
    __m128 vMax = _mm_set1_ps(fMax);
    __m128 vMin = _mm_set1_ps(fMin);
    while (iNumSamples >= 4) {
        vSrcSamples = _mm_load_ps(pSrc);
        vClamped = _mm_or_ps(vClamped, _mm_cmplt_ps(vSrcSamples, vMin));
        vClamped = _mm_or_ps(vClamped, _mm_cmpgt_ps(vSrcSamples, vMax));
        vSrcSamples = _mm_max_ps(vSrcSamples, vMin);
        vSrcSamples = _mm_min_ps(vSrcSamples, vMax);
        _mm_store_ps(pDest, vSrcSamples);
        iNumSamples -= 4;
        pDest += 4;
        pSrc += 4;
    }
    CSAMPLE clamp[4]; // TODO(XXX) alignment
    _mm_store_ps(clamp, vClamped);
    if (clamp[0] != 0 || clamp[1] != 0 ||
        clamp[2] != 0 || clamp[3] != 0) {
        clamped = true;
    }
    if (iNumSamples > 0) {
        qDebug() << "Not div by 4";
    }
    while (iNumSamples > 0) {
        CSAMPLE sample = *pSrc;
        if (sample > fMax) {
            sample = fMax;
            clamped = true;
        } else if (sample < fMin) {
            sample = fMax;
            clamped = true;
        }
        *pDest = sample;
        pDest++;
        pSrc++;
        iNumSamples--;
    }
#endif
    if (clamped)
        qDebug() << "clamped";
    return clamped;
}

// static
void SampleUtil::interleaveBuffer(CSAMPLE* pDest,
                                  const CSAMPLE* pSrc1, const CSAMPLE* pSrc2,
                                  int iNumSamples) {
    if (m_sOptimizationsOn) {
        return sseInterleaveBuffer(pDest, pSrc1, pSrc2, iNumSamples);
    }

    for (int i = 0; i < iNumSamples; ++i) {
        pDest[2*i] = pSrc1[i];
        pDest[2*i+1] = pSrc2[i];
    }
}

// static
void SampleUtil::sseInterleaveBuffer(CSAMPLE* pDest,
                                     const CSAMPLE* pSrc1, const CSAMPLE* pSrc2,
                                     int iNumSamples) {
#ifdef __SSE__
    __m128 vSrc1Samples;
    __m128 vSrc2Samples;
    __m128 vLow;
    __m128 vHigh;
    while (iNumSamples >= 4) {
        vSrc1Samples = _mm_load_ps(pSrc1);
        vSrc2Samples = _mm_load_ps(pSrc2);
        // vSrc1Samples is l1,l2,l3,l4
        // vSrc2Samples is r1,r2,r3,r4
        vLow = _mm_unpacklo_ps(vSrc1Samples, vSrc2Samples);
        // vLow is l1,r1,l2,r2
        vHigh = _mm_unpackhi_ps(vSrc1Samples, vSrc2Samples);
        // vHigh is l3,r3,l4,r4
        _mm_store_ps(pDest, vLow);
        _mm_store_ps(pDest+4, vHigh);
        iNumSamples -= 4;
        pSrc1 += 4;
        pSrc2 += 4;
        pDest += 8;
    }
    while (iNumSamples > 0) {
        *pDest++ = *pSrc1++;
        *pDest++ = *pSrc2++;
        iNumSamples--;
    }
#endif
}


// static
void SampleUtil::deinterleaveBuffer(CSAMPLE* pDest1, CSAMPLE* pDest2,
                                  const CSAMPLE* pSrc, int iNumSamples) {
    if (m_sOptimizationsOn) {
        return sseDeinterleaveBuffer(pDest1, pDest2, pSrc, iNumSamples);
    }

    for (int i = 0; i < iNumSamples; ++i) {
        pDest1[i] = pSrc[i*2];
        pDest2[i] = pSrc[i*2+1];
    }
}

// static
void SampleUtil::sseDeinterleaveBuffer(CSAMPLE* pDest1, CSAMPLE* pDest2,
                                       const CSAMPLE* pSrc, int iNumSamples) {
#ifdef __SSE__
    __m128 vSrc1Samples;
    __m128 vSrc2Samples;
    __m128 vDst1Samples;
    __m128 vDst2Samples;
    while (iNumSamples >= 4) {
        vSrc1Samples = _mm_load_ps(pSrc);
        vSrc2Samples = _mm_load_ps(pSrc+4);
        // vSrc1Samples is l1,r1,l2,r2
        // vSrc2Samples is l3,r3,l4,r4

        // First shuffle the middle elements of both.
        vSrc1Samples = _mm_shuffle_ps(vSrc1Samples, vSrc1Samples,
                                      _MM_SHUFFLE(0, 2, 1, 3));
        vSrc2Samples = _mm_shuffle_ps(vSrc2Samples, vSrc2Samples,
                                      _MM_SHUFFLE(0, 2, 1, 3));
        // vSrc1Samples is now l1,l2,r1,r2
        // vSrc2Samples is now l3,l4,r3,r4

        // Now move the low half of src2 into the high of src1 to make dst1. To
        // make dst2, move the high half of src1 into the low half of src2.
        vDst1Samples = _mm_movelh_ps(vSrc1Samples, vSrc2Samples);
        vDst2Samples = _mm_movehl_ps(vSrc2Samples, vSrc1Samples);
        // vDst1Samples is now l1,l2,l3,l4
        // vDst2Samples is now r1,r2,r3,r4

        _mm_store_ps(pDest1, vDst1Samples);
        _mm_store_ps(pDest2, vDst2Samples);
        iNumSamples -= 4;
        pSrc += 8;
        pDest1 += 4;
        pDest2 += 4;
    }
    while (iNumSamples > 0) {
        *pDest1++ = *pSrc++;
        *pDest2++ = *pSrc++;
        iNumSamples--;
    }
#endif
}

void SampleUtil::setOptimizations(bool opt) {
    qDebug() << "Opts" << opt;
    m_sOptimizationsOn = opt;
}
