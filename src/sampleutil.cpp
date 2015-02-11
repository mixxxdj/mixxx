// sampleutil.cpp
// Created 10/5/2009 by RJ Ryan (rryan@mit.edu)

#include "sampleutil.h"
#include "util/math.h"

#ifdef __WINDOWS__
#include <QtGlobal>
typedef qint64 int64_t;
typedef qint32 int32_t;
#endif

// the note: LOOP VECTORIZED below marks
// the loops that are processed with the 128 bit SSE registers
// it was tested with gcc 4.6 with the -ftree-vectorizer-verbose=2 flag
// on an Intel i5 CPU
// When changing, be carefull to not prevent the vectorizing
// https://gcc.gnu.org/projects/tree-ssa/vectorization.html

// TODO() Check if uintptr_t is availabe on all our build targets and use that
// instead of size_t, we can remove the sizeof(size_t) check than 
static inline bool useAlignedAlloc() {
    // This will work on all targets and compilers. 
    // It will return true on MSVC 32 bit builds and false for
    // Linux 32 and 64 bit builds
    return (sizeof(long double) == 8 && sizeof(CSAMPLE*) <= 8 &&
            sizeof(CSAMPLE*) == sizeof(size_t));
}

// static
CSAMPLE* SampleUtil::alloc(unsigned int size) {
    // For optimal use of SSE registers, it is required to align
    // the sample buffers to 16 Byte (128 bit) boundary

    // Pointers returned by malloc are aligned for the largest scalar type,
    // which is long double with usually 16 byte.
    // An exception is MSVC X86 where long double is mapped to double.

    // In case of sizeof(long double) = 8
    // this code over allocates the requested buffer to be able to
    // shift the returned pointer to a 16 byte alignment
    // In the memory before, a pointer to the original
    // malloced area is stored used to free the memory
    // This code can be replaced by C11 <stdlib.h> aligned_alloc()
    // or MSVC ::_aligned_malloc(size, alignment) and  ::_aligned_free(ptr);

    if (useAlignedAlloc()) {
        // We need to shift the alignment to 16
        const size_t alignment = 16;
        const size_t unaligned_size = sizeof(CSAMPLE[size]) + alignment;
        void* pUnaligned = std::malloc(unaligned_size);
        if (pUnaligned == NULL) {
            return NULL;
        }
        // Shift
        void* pAlligned = (void*)(((size_t)pUnaligned & ~(alignment - 1)) + alignment);
        // Store pointer to original relative to the shifted pointer
        *((void**)(pAlligned) - 1) = pUnaligned;
        return (CSAMPLE*)pAlligned;
    } else {
        // We are either correct aligned or on an exotic architecture
        return new CSAMPLE[size];
    }
}

void SampleUtil::free(CSAMPLE* pBuffer) {
    // See SampleUtil::alloc() for details
    if (useAlignedAlloc()) {
        if (pBuffer == NULL) {
            return;
        }
        // Pointer to the original memory is stored before pBuffer
        std::free(*((void**)((void*)pBuffer) - 1));
    } else {
        // We are either correct aligned or on an exotic architecture
        delete[] pBuffer;
    }
}

// static
void SampleUtil::applyGain(CSAMPLE* pBuffer, CSAMPLE_GAIN gain,
        unsigned int iNumSamples) {
    if (gain == CSAMPLE_GAIN_ONE)
        return;
    if (gain == CSAMPLE_GAIN_ZERO) {
        clear(pBuffer, iNumSamples);
        return;
    }

    // note: LOOP VECTORIZED.
    for (unsigned int i = 0; i < iNumSamples; ++i) {
        pBuffer[i] *= gain;
    }
}

// static
void SampleUtil::applyRampingGain(CSAMPLE* pBuffer, CSAMPLE_GAIN old_gain,
        CSAMPLE_GAIN new_gain, unsigned int iNumSamples) {
    if (old_gain == CSAMPLE_GAIN_ONE && new_gain == CSAMPLE_GAIN_ONE) {
        return;
    }
    if (old_gain == CSAMPLE_GAIN_ZERO && new_gain == CSAMPLE_GAIN_ZERO) {
        clear(pBuffer, iNumSamples);
        return;
    }

    const CSAMPLE_GAIN gain_delta = (new_gain - old_gain)
            / CSAMPLE_GAIN(iNumSamples / 2);
    CSAMPLE_GAIN gain = old_gain;
    if (gain_delta) {
        for (unsigned int i = 0; i < iNumSamples; i += 2) {
            gain += gain_delta;
            pBuffer[i] *= gain;
            pBuffer[i + 1] *= gain;
        }
    } else {
        for (unsigned int i = 0; i < iNumSamples; ++i) {
            pBuffer[i] *= gain;
        }
    }
}

// static
void SampleUtil::applyAlternatingGain(CSAMPLE* pBuffer, CSAMPLE gain1,
        CSAMPLE gain2, unsigned int iNumSamples) {
    // This handles gain1 == CSAMPLE_GAIN_ONE && gain2 == CSAMPLE_GAIN_ONE as well.
    if (gain1 == gain2) {
        return applyGain(pBuffer, gain1, iNumSamples);
    }

    for (unsigned int i = 0; i < iNumSamples; i += 2) {
        pBuffer[i] *= gain1;
        pBuffer[i + 1] *= gain2;
    }
}

// static
void SampleUtil::addWithGain(CSAMPLE* _RESTRICT pDest, const CSAMPLE* _RESTRICT pSrc,
        CSAMPLE_GAIN gain, unsigned int iNumSamples) {
    if (gain == CSAMPLE_GAIN_ZERO) {
        return;
    }

    // note: LOOP VECTORIZED.
    for (int i = 0; i < (int)iNumSamples; ++i) {
        pDest[i] += pSrc[i] * gain;
    }
}

void SampleUtil::addWithRampingGain(CSAMPLE* _RESTRICT pDest, const CSAMPLE* _RESTRICT pSrc,
        CSAMPLE_GAIN old_gain, CSAMPLE_GAIN new_gain,
        unsigned int iNumSamples) {
    if (old_gain == CSAMPLE_GAIN_ZERO && new_gain == CSAMPLE_GAIN_ZERO) {
        return;
    }

    const CSAMPLE_GAIN gain_delta = (new_gain - old_gain)
            / CSAMPLE_GAIN(iNumSamples / 2);
    CSAMPLE_GAIN gain = old_gain;
    if (gain_delta) {
        for (unsigned int i = 0; i < iNumSamples; i += 2) {
            gain += gain_delta;
            pDest[i] += pSrc[i] * gain;
            pDest[i + 1] += pSrc[i + 1] * gain;
        }
    } else {
        // note: LOOP VECTORIZED.
        for (unsigned int i = 0; i < iNumSamples; ++i) {
            pDest[i] += pSrc[i] * gain;
        }
    }
}

// static
void SampleUtil::add2WithGain(CSAMPLE* _RESTRICT pDest, const CSAMPLE* _RESTRICT pSrc1,
        CSAMPLE_GAIN gain1, const CSAMPLE* _RESTRICT pSrc2, CSAMPLE_GAIN gain2,
        unsigned int iNumSamples) {
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        return addWithGain(pDest, pSrc2, gain2, iNumSamples);
    } else if (gain2 == CSAMPLE_GAIN_ZERO) {
        return addWithGain(pDest, pSrc1, gain1, iNumSamples);
    }

    // note: LOOP VECTORIZED.
    for (unsigned int i = 0; i < iNumSamples; ++i) {
        pDest[i] += pSrc1[i] * gain1 + pSrc2[i] * gain2;
    }
}

// static
void SampleUtil::add3WithGain(CSAMPLE* pDest, const CSAMPLE* _RESTRICT pSrc1,
        CSAMPLE_GAIN gain1, const CSAMPLE* _RESTRICT pSrc2, CSAMPLE_GAIN gain2,
        const CSAMPLE* _RESTRICT pSrc3, CSAMPLE_GAIN gain3, unsigned int iNumSamples) {
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        return add2WithGain(pDest, pSrc2, gain2, pSrc3, gain3, iNumSamples);
    } else if (gain2 == CSAMPLE_GAIN_ZERO) {
        return add2WithGain(pDest, pSrc1, gain1, pSrc3, gain3, iNumSamples);
    } else if (gain3 == CSAMPLE_GAIN_ZERO) {
        return add2WithGain(pDest, pSrc1, gain1, pSrc2, gain2, iNumSamples);
    }

    // note: LOOP VECTORIZED.
    for (unsigned int i = 0; i < iNumSamples; ++i) {
        pDest[i] += pSrc1[i] * gain1 + pSrc2[i] * gain2 + pSrc3[i] * gain3;
    }
}

// static
void SampleUtil::copyWithGain(CSAMPLE* _RESTRICT pDest, const CSAMPLE* _RESTRICT pSrc,
        CSAMPLE_GAIN gain, unsigned int iNumSamples) {
    if (gain == CSAMPLE_GAIN_ONE) {
        copy(pDest, pSrc, iNumSamples);
        return;
    }
    if (gain == CSAMPLE_GAIN_ZERO) {
        clear(pDest, iNumSamples);
        return;
    }

    // note: LOOP VECTORIZED.
    for (unsigned int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc[i] * gain;
    }

    // OR! need to test which fares better
    // copy(pDest, pSrc, iNumSamples);
    // applyGain(pDest, gain);
}

// static
void SampleUtil::copyWithRampingGain(CSAMPLE* _RESTRICT pDest, const CSAMPLE* _RESTRICT pSrc,
        CSAMPLE_GAIN old_gain, CSAMPLE_GAIN new_gain,
        unsigned int iNumSamples) {
    if (old_gain == CSAMPLE_GAIN_ONE && new_gain == CSAMPLE_GAIN_ONE) {
        copy(pDest, pSrc, iNumSamples);
        return;
    }
    if (old_gain == CSAMPLE_GAIN_ZERO && new_gain == CSAMPLE_GAIN_ZERO) {
        clear(pDest, iNumSamples);
        return;
    }

    const CSAMPLE_GAIN gain_delta = (new_gain - old_gain)
            / CSAMPLE_GAIN(iNumSamples / 2);
    CSAMPLE_GAIN gain = old_gain;
    if (gain_delta) {
        for (unsigned int i = 0; i < iNumSamples; i += 2) {
            gain += gain_delta;
            pDest[i] = pSrc[i] * gain;
            pDest[i + 1] = pSrc[i + 1] * gain;
        }
    } else {
        // note: LOOP VECTORIZED.
        for (unsigned int i = 0; i < iNumSamples; ++i) {
            pDest[i] = pSrc[i] * gain;
        }
    }

    // OR! need to test which fares better
    // copy(pDest, pSrc, iNumSamples);
    // applyRampingGain(pDest, gain);
}

// static
void SampleUtil::convertS16ToFloat32(CSAMPLE* _RESTRICT pDest, const SAMPLE* _RESTRICT pSrc,
        unsigned int iNumSamples) {
    // -32768 is a valid low sample, whereas 32767 is the highest valid sample.
    // Note that this means that although some sample values convert to -1.0,
    // none will convert to +1.0.
    const CSAMPLE kConversionFactor = 0x8000;
    // note: LOOP VECTORIZED.
    for (unsigned int i = 0; i < iNumSamples; ++i) {
        pDest[i] = CSAMPLE(pSrc[i]) / kConversionFactor;
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
void SampleUtil::copyClampBuffer(CSAMPLE* _RESTRICT pDest, const _RESTRICT CSAMPLE* pSrc,
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
    const CSAMPLE_GAIN cross_inc = CSAMPLE_GAIN_ONE
            / CSAMPLE_GAIN(iNumSamples / 2);
    CSAMPLE_GAIN cross_mix = CSAMPLE_GAIN_ZERO;
    for (unsigned int i = 0; i + 1 < iNumSamples; i += 2) {
        pDest[i] = pSrcFadeIn[i] * cross_mix
                + pSrcFadeOut[i] * (CSAMPLE_GAIN_ONE - cross_mix);
        pDest[i + 1] = pSrcFadeIn[i + 1] * cross_mix
                + pSrcFadeOut[i + 1] * (CSAMPLE_GAIN_ONE - cross_mix);
        cross_mix += cross_inc;
    }
}

// static
void SampleUtil::mixStereoToMono(CSAMPLE* pDest, const CSAMPLE* pSrc,
        unsigned int iNumSamples) {
    const CSAMPLE_GAIN mixScale = CSAMPLE_GAIN_ONE
            / (CSAMPLE_GAIN_ONE + CSAMPLE_GAIN_ONE);
    for (unsigned int i = 0; i + 1 < iNumSamples; i += 2) {
        pDest[i] = (pSrc[i] + pSrc[i + 1]) * mixScale;
        pDest[i + 1] = pDest[i];
    }
}

// static
void SampleUtil::doubleMonoToDualMono(SAMPLE* pBuffer, unsigned int numFrames) {
    // backward loop
    unsigned int i = numFrames;
    while (0 < i--) {
        pBuffer[i * 2] = pBuffer[i];
        pBuffer[i * 2 + 1] = pBuffer[i];
    }
}

// static
void SampleUtil::doubleMonoToDualMono(CSAMPLE* pBuffer, unsigned int numFrames) {
    // backward loop
    unsigned int i = numFrames;
    while (0 < i--) {
        pBuffer[i * 2] = pBuffer[i];
        pBuffer[i * 2 + 1] = pBuffer[i];
    }
}

// static
void SampleUtil::copyMonoToDualMono(CSAMPLE* _RESTRICT pDest, const CSAMPLE* _RESTRICT pSrc,
        unsigned int numFrames) {
    // forward loop
    for (unsigned int i = 0; i < numFrames; ++i) {
        pDest[i * 2] = pSrc[i];
        pDest[i * 2 + 1] = pSrc[i];
    }
}

// static
void SampleUtil::stripMultiToStereo(CSAMPLE* pBuffer, unsigned int numFrames,
        unsigned int numChannels) {
    // forward loop
    for (unsigned int i = 0; i < numFrames; ++i) {
        pBuffer[i * 2] = pBuffer[i * numChannels];
        pBuffer[i * 2 + 1] = pBuffer[i * numChannels + 1];
    }
}

// static
void SampleUtil::copyMultiToStereo(CSAMPLE* _RESTRICT pDest, const CSAMPLE* _RESTRICT pSrc,
        unsigned int numFrames, unsigned int numChannels) {
    // forward loop
    for (unsigned int i = 0; i < numFrames; ++i) {
        pDest[i * 2] = pSrc[i * numChannels];
        pDest[i * 2 + 1] = pSrc[i * numChannels + 1];
    }
}

