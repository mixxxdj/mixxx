#include <cstdlib>

#include "util/sample.h"
#include "util/math.h"

#ifdef __WINDOWS__
#include <QtGlobal>
typedef qint64 int64_t;
typedef qint32 int32_t;
#endif

// LOOP VECTORIZED below marks the loops that are processed with the 128 bit SSE
// registers as tested with gcc 4.6 and the -ftree-vectorizer-verbose=2 flag on
// an Intel i5 CPU. When changing, be careful to not disturb the vectorization.
// https://gcc.gnu.org/projects/tree-ssa/vectorization.html
// This also utilizes AVX registers when compiled for a recent 64-bit CPU
// using scons optimize=native.

// TODO() Check if uintptr_t is available on all our build targets and use that
// instead of size_t, we can remove the sizeof(size_t) check than
static inline bool useAlignedAlloc() {
    // This will work on all targets and compilers.
    // It will return true on MSVC 32 bit builds and false for
    // Linux 32 and 64 bit builds
    return (sizeof(long double) == 8 && sizeof(CSAMPLE*) <= 8 &&
            sizeof(CSAMPLE*) == sizeof(size_t));
}

// static
CSAMPLE* SampleUtil::alloc(int size) {
    // To speed up vectorization we align our sample buffers to 16-byte (128
    // bit) boundaries so that vectorized loops doesn't have to do a serial
    // ramp-up before going parallel.
    //
    // Pointers returned by malloc are aligned for the largest scalar type. On
    // most platforms the largest scalar type is long double (16 bytes).
    // However, on MSVC x86 long double is 8 bytes.
    //
    // On MSVC, we use _aligned_malloc to handle aligning pointers to 16-byte
    // boundaries. On other platforms where long double is 8 bytes this code
    // allocates 16 additional slack bytes so we can adjust the pointer we
    // return to the caller to be 16-byte aligned. We record a pointer to the
    // true start of the buffer in the slack space as well so that we can free
    // it correctly.
    // TODO(XXX): Replace with C++11 aligned_alloc.
    // TODO(XXX): consider 32 byte alignement to optimize for AVX builds
    if (useAlignedAlloc()) {
#ifdef _MSC_VER
        return static_cast<CSAMPLE*>(_aligned_malloc(sizeof(CSAMPLE)*size, 16));
#else
        // This block will be only used on non-Windows platforms that don't
        // produce 16-byte aligned pointers via malloc. We allocate 16 bytes of
        // slack space so that we can align the pointer we return to the caller.
        const size_t alignment = 16;
        const size_t unaligned_size = sizeof(CSAMPLE[size]) + alignment;
        void* pUnaligned = std::malloc(unaligned_size);
        if (pUnaligned == NULL) {
            return NULL;
        }
        // Shift
        void* pAligned = (void*)(((size_t)pUnaligned & ~(alignment - 1)) + alignment);
        // Store pointer to the original buffer in the slack space before the
        // shifted pointer.
        *((void**)(pAligned) - 1) = pUnaligned;
        return static_cast<CSAMPLE*>(pAligned);
#endif
    } else {
        // Our platform already produces 16-byte aligned pointers (or is an exotic target)
        // We should be explicit about what we want from the system.
        // TODO(XXX): Use posix_memalign, memalign, or aligned_alloc.
        return new CSAMPLE[size];
    }
}

void SampleUtil::free(CSAMPLE* pBuffer) {
    // See SampleUtil::alloc() for details
    if (useAlignedAlloc()) {
        if (pBuffer == NULL) {
            return;
        }
#ifdef _MSC_VER
        _aligned_free(pBuffer);
#else
        // Pointer to the original memory is stored before pBuffer
        std::free(*((void**)((void*)pBuffer) - 1));
#endif
    } else {
        delete[] pBuffer;
    }
}

// static
void SampleUtil::applyGain(CSAMPLE* pBuffer, CSAMPLE_GAIN gain,
        int iNumSamples) {
    if (gain == CSAMPLE_GAIN_ONE)
        return;
    if (gain == CSAMPLE_GAIN_ZERO) {
        clear(pBuffer, iNumSamples);
        return;
    }

    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples; ++i) {
        pBuffer[i] *= gain;
    }
}

// static
void SampleUtil::applyRampingGain(CSAMPLE* pBuffer, CSAMPLE_GAIN old_gain,
        CSAMPLE_GAIN new_gain, int iNumSamples) {
    if (old_gain == CSAMPLE_GAIN_ONE && new_gain == CSAMPLE_GAIN_ONE) {
        return;
    }
    if (old_gain == CSAMPLE_GAIN_ZERO && new_gain == CSAMPLE_GAIN_ZERO) {
        clear(pBuffer, iNumSamples);
        return;
    }

    const CSAMPLE_GAIN gain_delta = (new_gain - old_gain)
            / CSAMPLE_GAIN(iNumSamples / 2);
    if (gain_delta) {
        const CSAMPLE_GAIN start_gain = old_gain + gain_delta;
        // note: LOOP VECTORIZED.
        for (int i = 0; i < iNumSamples / 2; ++i) {
            const CSAMPLE_GAIN gain = start_gain + gain_delta * i;
            // a loop counter i += 2 prevents vectorizing.
            pBuffer[i * 2] *= gain;
            pBuffer[i * 2 + 1] *= gain;
        }
    } else {
        // note: LOOP VECTORIZED.
        for (int i = 0; i < iNumSamples; ++i) {
            pBuffer[i] *= old_gain;
        }
    }
}

// static
void SampleUtil::applyAlternatingGain(CSAMPLE* pBuffer, CSAMPLE gain1,
        CSAMPLE gain2, int iNumSamples) {
    // This handles gain1 == CSAMPLE_GAIN_ONE && gain2 == CSAMPLE_GAIN_ONE as well.
    if (gain1 == gain2) {
        return applyGain(pBuffer, gain1, iNumSamples);
    }

    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        pBuffer[i * 2] *= gain1;
        pBuffer[i * 2 + 1] *= gain2;
    }
}

// static
void SampleUtil::addWithGain(CSAMPLE* M_RESTRICT pDest, const CSAMPLE* M_RESTRICT pSrc,
        CSAMPLE_GAIN gain, int iNumSamples) {
    if (gain == CSAMPLE_GAIN_ZERO) {
        return;
    }

    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] += pSrc[i] * gain;
    }
}

void SampleUtil::addWithRampingGain(CSAMPLE* M_RESTRICT pDest, const CSAMPLE* M_RESTRICT pSrc,
        CSAMPLE_GAIN old_gain, CSAMPLE_GAIN new_gain,
        int iNumSamples) {
    if (old_gain == CSAMPLE_GAIN_ZERO && new_gain == CSAMPLE_GAIN_ZERO) {
        return;
    }

    const CSAMPLE_GAIN gain_delta = (new_gain - old_gain)
            / CSAMPLE_GAIN(iNumSamples / 2);
    if (gain_delta) {
        const CSAMPLE_GAIN start_gain = old_gain + gain_delta;
        // note: LOOP VECTORIZED.
        for (int i = 0; i < iNumSamples / 2; ++i) {
            const CSAMPLE_GAIN gain = start_gain + gain_delta * i;
            pDest[i * 2] += pSrc[i * 2] * gain;
            pDest[i * 2 + 1] += pSrc[i * 2 + 1] * gain;
        }
    } else {
        // note: LOOP VECTORIZED.
        for (int i = 0; i < iNumSamples; ++i) {
            pDest[i] += pSrc[i] * old_gain;
        }
    }
}

// static
void SampleUtil::add2WithGain(CSAMPLE* M_RESTRICT pDest, const CSAMPLE* M_RESTRICT pSrc1,
        CSAMPLE_GAIN gain1, const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
        int iNumSamples) {
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        return addWithGain(pDest, pSrc2, gain2, iNumSamples);
    } else if (gain2 == CSAMPLE_GAIN_ZERO) {
        return addWithGain(pDest, pSrc1, gain1, iNumSamples);
    }

    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] += pSrc1[i] * gain1 + pSrc2[i] * gain2;
    }
}

// static
void SampleUtil::add3WithGain(CSAMPLE* pDest, const CSAMPLE* M_RESTRICT pSrc1,
        CSAMPLE_GAIN gain1, const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
        const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3, int iNumSamples) {
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        return add2WithGain(pDest, pSrc2, gain2, pSrc3, gain3, iNumSamples);
    } else if (gain2 == CSAMPLE_GAIN_ZERO) {
        return add2WithGain(pDest, pSrc1, gain1, pSrc3, gain3, iNumSamples);
    } else if (gain3 == CSAMPLE_GAIN_ZERO) {
        return add2WithGain(pDest, pSrc1, gain1, pSrc2, gain2, iNumSamples);
    }

    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] += pSrc1[i] * gain1 + pSrc2[i] * gain2 + pSrc3[i] * gain3;
    }
}

// static
void SampleUtil::copyWithGain(CSAMPLE* M_RESTRICT pDest, const CSAMPLE* M_RESTRICT pSrc,
        CSAMPLE_GAIN gain, int iNumSamples) {
    if (gain == CSAMPLE_GAIN_ONE) {
        copy(pDest, pSrc, iNumSamples);
        return;
    }
    if (gain == CSAMPLE_GAIN_ZERO) {
        clear(pDest, iNumSamples);
        return;
    }

    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc[i] * gain;
    }

    // OR! need to test which fares better
    // copy(pDest, pSrc, iNumSamples);
    // applyGain(pDest, gain);
}

// static
void SampleUtil::copyWithRampingGain(CSAMPLE* M_RESTRICT pDest, const CSAMPLE* M_RESTRICT pSrc,
        CSAMPLE_GAIN old_gain, CSAMPLE_GAIN new_gain, int iNumSamples) {
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
    if (gain_delta) {
        const CSAMPLE_GAIN start_gain = old_gain + gain_delta;
        // note: LOOP VECTORIZED.
        for (int i = 0; i < iNumSamples / 2; ++i) {
            const CSAMPLE_GAIN gain = start_gain + gain_delta * i;
            pDest[i * 2] = pSrc[i * 2] * gain;
            pDest[i * 2 + 1] = pSrc[i * 2 + 1] * gain;
        }
    } else {
        // note: LOOP VECTORIZED.
        for (int i = 0; i < iNumSamples; ++i) {
            pDest[i] = pSrc[i] * old_gain;
        }
    }

    // OR! need to test which fares better
    // copy(pDest, pSrc, iNumSamples);
    // applyRampingGain(pDest, gain);
}

// static
void SampleUtil::convertS16ToFloat32(CSAMPLE* M_RESTRICT pDest, const SAMPLE* M_RESTRICT pSrc,
        int iNumSamples) {
    // SAMPLE_MIN = -32768 is a valid low sample, whereas SAMPLE_MAX = 32767
    // is the highest valid sample. Note that this means that although some
    // sample values convert to -1.0, none will convert to +1.0.
    DEBUG_ASSERT(-SAMPLE_MIN >= SAMPLE_MAX);
    const CSAMPLE kConversionFactor = -SAMPLE_MIN;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = CSAMPLE(pSrc[i]) / kConversionFactor;
    }
}

//static
void SampleUtil::convertFloat32ToS16(SAMPLE* pDest, const CSAMPLE* pSrc,
        unsigned int iNumSamples) {
    DEBUG_ASSERT(-SAMPLE_MIN >= SAMPLE_MAX);
    const CSAMPLE kConversionFactor = -SAMPLE_MIN;
    for (unsigned int i = 0; i < iNumSamples; ++i) {
        pDest[i] = SAMPLE(pSrc[i] * kConversionFactor);
    }
}

// static
SampleUtil::CLIP_STATUS SampleUtil::sumAbsPerChannel(CSAMPLE* pfAbsL, CSAMPLE* pfAbsR,
        const CSAMPLE* pBuffer, int iNumSamples) {
    CSAMPLE fAbsL = CSAMPLE_ZERO;
    CSAMPLE fAbsR = CSAMPLE_ZERO;
    CSAMPLE clippedL = 0;
    CSAMPLE clippedR = 0;

    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        CSAMPLE absl = fabs(pBuffer[i * 2]);
        fAbsL += absl;
        clippedL += absl > CSAMPLE_PEAK ? 1 : 0;
        CSAMPLE absr = fabs(pBuffer[i * 2 + 1]);
        fAbsR += absr;
        // Replacing the code with a bool clipped will prevent vetorizing
        clippedR += absr > CSAMPLE_PEAK ? 1 : 0;
    }

    *pfAbsL = fAbsL;
    *pfAbsR = fAbsR;
    SampleUtil::CLIP_STATUS clipping = SampleUtil::NO_CLIPPING;
    if (clippedL > 0) {
        clipping |= SampleUtil::CLIPPING_LEFT;
    }
    if (clippedR > 0) {
        clipping |= SampleUtil::CLIPPING_RIGHT;
    }
    return clipping;
}

// static
void SampleUtil::copyClampBuffer(CSAMPLE* M_RESTRICT pDest, const CSAMPLE* M_RESTRICT pSrc,
        int iNumSamples) {
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = clampSample(pSrc[i]);
    }
}

// static
void SampleUtil::interleaveBuffer(CSAMPLE* M_RESTRICT pDest, const CSAMPLE* M_RESTRICT pSrc1,
        const CSAMPLE* M_RESTRICT pSrc2, int iNumSamples) {
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[2 * i] = pSrc1[i];
        pDest[2 * i + 1] = pSrc2[i];
    }
}

// static
void SampleUtil::deinterleaveBuffer(CSAMPLE* pDest1, CSAMPLE* pDest2,
        const CSAMPLE* pSrc, int iNumSamples) {
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples; ++i) {
        pDest1[i] = pSrc[i * 2];
        pDest2[i] = pSrc[i * 2 + 1];
    }
}

// static
void SampleUtil::linearCrossfadeBuffers(CSAMPLE* pDest,
        const CSAMPLE* pSrcFadeOut, const CSAMPLE* pSrcFadeIn,
        int iNumSamples) {
    const CSAMPLE_GAIN cross_inc = CSAMPLE_GAIN_ONE
            / CSAMPLE_GAIN(iNumSamples / 2);
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN cross_mix = cross_inc * i;
        pDest[i * 2] = pSrcFadeIn[i * 2] * cross_mix
                + pSrcFadeOut[i * 2] * (CSAMPLE_GAIN_ONE - cross_mix);
        pDest[i * 2 + 1] = pSrcFadeIn[i * 2 + 1] * cross_mix
                + pSrcFadeOut[i * 2 + 1] * (CSAMPLE_GAIN_ONE - cross_mix);

    }
}

// static
void SampleUtil::mixStereoToMono(CSAMPLE* pDest, const CSAMPLE* pSrc,
        int iNumSamples) {
    const CSAMPLE_GAIN mixScale = CSAMPLE_GAIN_ONE
            / (CSAMPLE_GAIN_ONE + CSAMPLE_GAIN_ONE);
    // note: LOOP VECTORIZED
    for (int i = 0; i < iNumSamples / 2; ++i) {
        pDest[i * 2] = (pSrc[i * 2] + pSrc[i * 2 + 1]) * mixScale;
        pDest[i * 2 + 1] = pDest[i * 2];
    }
}

// static
void SampleUtil::doubleMonoToDualMono(CSAMPLE* pBuffer, int numFrames) {
    // backward loop
    int i = numFrames;
    // Unvectorizable Loop
    while (0 < i--) {
        const CSAMPLE s = pBuffer[i];
        pBuffer[i * 2] = s;
        pBuffer[i * 2 + 1] = s;
    }
}

// static
void SampleUtil::copyMonoToDualMono(CSAMPLE* M_RESTRICT pDest, const CSAMPLE* M_RESTRICT pSrc,
        int numFrames) {
    // forward loop
    // note: LOOP VECTORIZED
    for (int i = 0; i < numFrames; ++i) {
        const CSAMPLE s = pSrc[i];
        pDest[i * 2] = s;
        pDest[i * 2 + 1] = s;
    }
}

// static
void SampleUtil::stripMultiToStereo(CSAMPLE* pBuffer, int numFrames,
        int numChannels) {
    // forward loop
    for (int i = 0; i < numFrames; ++i) {
        pBuffer[i * 2] = pBuffer[i * numChannels];
        pBuffer[i * 2 + 1] = pBuffer[i * numChannels + 1];
    }
}

// static
void SampleUtil::copyMultiToStereo(CSAMPLE* M_RESTRICT pDest, const CSAMPLE* M_RESTRICT pSrc,
        int numFrames, int numChannels) {
    // forward loop
    for (int i = 0; i < numFrames; ++i) {
        pDest[i * 2] = pSrc[i * numChannels];
        pDest[i * 2 + 1] = pSrc[i * numChannels + 1];
    }
}


// static
void SampleUtil::reverse(CSAMPLE* pBuffer, int iNumSamples) {
    for (int j = 0; j < iNumSamples / 4; ++j) {
        const int endpos = (iNumSamples - 1) - j * 2 ;
        CSAMPLE temp1 = pBuffer[j * 2];
        CSAMPLE temp2 = pBuffer[j * 2 + 1];
        pBuffer[j * 2] = pBuffer[endpos - 1];
        pBuffer[j * 2 + 1] = pBuffer[endpos];
        pBuffer[endpos - 1] = temp1;
        pBuffer[endpos] = temp2;
    }
}

// static
void SampleUtil::copyReverse(CSAMPLE* M_RESTRICT pDest, const CSAMPLE* M_RESTRICT pSrc,
        int iNumSamples) {
    for (int j = 0; j < iNumSamples / 2; ++j) {
        const int endpos = (iNumSamples - 1) - j * 2;
        pDest[j * 2] = pSrc[endpos - 1];
        pDest[j * 2 + 1] = pSrc[endpos];
    }
}
