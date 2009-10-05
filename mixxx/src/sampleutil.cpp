// sampleutil.cpp
// Created 10/5/2009 by RJ Ryan (rryan@mit.edu)

#include <xmmintrin.h>

#include <QtDebug>

#include "sampleutil.h"


// static
void SampleUtil::applyGain(CSAMPLE* pBuffer,
                           CSAMPLE gain, int iNumSamples) {
    if (gain == 1.0f)
        return;
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
#else
    for (int i = 0; i < iNumSamples; ++i) {
        pBuffer[i] *= gain;
    }
#endif
}

// static
void SampleUtil::addWithGain(CSAMPLE* pDest, const CSAMPLE* pSrc,
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
#else
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] += pSrc[i] * gain;
    }
#endif
}

// static
void SampleUtil::add2WithGain(CSAMPLE* pDest,
                              const CSAMPLE* pSrc1, CSAMPLE gain1,
                              const CSAMPLE* pSrc2, CSAMPLE gain2,
                              int iNumSamples) {
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] += pSrc1[i] * gain1 + pSrc2[i] * gain2;
    }
}

// static
void SampleUtil::copyWithGain(CSAMPLE* pDest, const CSAMPLE* pSrc,
                              CSAMPLE gain, int iNumSamples) {
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc[i] * gain;
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
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc1[i] * gain1 + pSrc2[i] * gain2;
    }
}

// static
void SampleUtil::convert(CSAMPLE* pDest, const SAMPLE* pSrc,
                         int iNumSamples) {
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc[i];
    }
}
