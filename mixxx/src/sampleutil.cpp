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
    if (gain1 == 1.0f && gain2 == 1.0f)
        return;
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
    if (m_sOptimizationsOn) {
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
void SampleUtil::convert(CSAMPLE* pDest, const SAMPLE* pSrc,
                         int iNumSamples) {
    if (m_sOptimizationsOn) {
        // Not done yet
        //return sseConvert(pDest, pSrc, iNumSamples);
    }

    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc[i];
    }
}

// static
void SampleUtil::sseConvert(CSAMPLE* pDest, const SAMPLE* pSrc,
                            int iNumSamples) {
    // TODO(XXX)
}

void SampleUtil::setOptimizations(bool opt) {
    qDebug() << "Opts" << opt;
    m_sOptimizationsOn = opt;
}
