#include <cstdlib>
#include <cstddef>

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

namespace {

#ifdef __AVX__
constexpr size_t kAlignment = 32;
#else
constexpr size_t kAlignment = 16;
#endif

// TODO() Check if uintptr_t is available on all our build targets and use that
// instead of size_t, we can remove the sizeof(size_t) check than
constexpr bool useAlignedAlloc() {
    // This will work on all targets and compilers.
    // It will return true bot 32 bit builds and false for 64 bit builds
    return alignof(max_align_t) < kAlignment &&
            sizeof(CSAMPLE*) == sizeof(size_t);
}

} // anonymous namespace

// static
CSAMPLE* SampleUtil::alloc(SINT size) {
    // To speed up vectorization we align our sample buffers to 16-byte (128
    // bit) boundaries on SSE builds and 32-byte (256 bit) on AVX builds so
    // that vectorized loops doesn't have to do a serial ramp-up before going
    // parallel.
    //
    // Pointers returned by malloc are aligned for the largest scalar type. On
    // most platforms the largest scalar type is long double (16 bytes).
    // However, on MSVC x86 long double is 8 bytes.
    // This can be tested via alignof(std::max_align_t)
    if (useAlignedAlloc()) {
#if defined(_MSC_VER)
        // On MSVC, we use _aligned_malloc to handle aligning pointers to 16-byte
        // boundaries.
        return static_cast<CSAMPLE*>(
                _aligned_malloc(sizeof(CSAMPLE) * size, kAlignment));
#elif defined(_GLIBCXX_HAVE_ALIGNED_ALLOC)
        std::size_t alloc_size = sizeof(CSAMPLE) * size;
        // The size (in bytes) must be an integral multiple of kAlignment
        std::size_t aligned_alloc_size = alloc_size;
        if (alloc_size % kAlignment != 0) {
            aligned_alloc_size += (kAlignment - alloc_size % kAlignment);
        }
        DEBUG_ASSERT(aligned_alloc_size % kAlignment == 0);
        return static_cast<CSAMPLE*>(std::aligned_alloc(kAlignment, aligned_alloc_size));
#else
        // On other platforms that might not support std::aligned_alloc
        // yet but where long double is 8 bytes this code allocates 16 additional
        // slack bytes so we can adjust the pointer we return to the caller to be
        // 16-byte aligned. We record a pointer to the true start of the buffer
        // in the slack space as well so that we can free it correctly.
        const size_t alignment = kAlignment;
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
        // Our platform already produces aligned pointers (or is an exotic target)
        return new CSAMPLE[size];
    }
}

void SampleUtil::free(CSAMPLE* pBuffer) {
    // See SampleUtil::alloc() for details
    if (useAlignedAlloc()) {
#if defined(_MSC_VER)
        _aligned_free(pBuffer);
#elif defined(_GLIBCXX_HAVE_ALIGNED_ALLOC)
        std::free(pBuffer);
#else
        // Pointer to the original memory is stored before pBuffer
        if (!pBuffer) {
            return;
        }
        std::free(*((void**)((void*)pBuffer) - 1));
#endif
    } else {
        delete[] pBuffer;
    }
}

// static
void SampleUtil::applyGain(CSAMPLE* pBuffer, CSAMPLE_GAIN gain,
        SINT numSamples) {
    if (gain == CSAMPLE_GAIN_ONE)
        return;
    if (gain == CSAMPLE_GAIN_ZERO) {
        clear(pBuffer, numSamples);
        return;
    }

    // note: LOOP VECTORIZED.
    for (SINT i = 0; i < numSamples; ++i) {
        pBuffer[i] *= gain;
    }
}

// static
void SampleUtil::applyRampingGain(CSAMPLE* pBuffer, CSAMPLE_GAIN old_gain,
        CSAMPLE_GAIN new_gain, SINT numSamples) {
    if (old_gain == CSAMPLE_GAIN_ONE && new_gain == CSAMPLE_GAIN_ONE) {
        return;
    }
    if (old_gain == CSAMPLE_GAIN_ZERO && new_gain == CSAMPLE_GAIN_ZERO) {
        clear(pBuffer, numSamples);
        return;
    }

    const CSAMPLE_GAIN gain_delta = (new_gain - old_gain)
            / CSAMPLE_GAIN(numSamples / 2);
    if (gain_delta != 0) {
        const CSAMPLE_GAIN start_gain = old_gain + gain_delta;
        // note: LOOP VECTORIZED.
        for (int i = 0; i < numSamples / 2; ++i) {
            const CSAMPLE_GAIN gain = start_gain + gain_delta * i;
            // a loop counter i += 2 prevents vectorizing.
            pBuffer[i * 2] *= gain;
            pBuffer[i * 2 + 1] *= gain;
        }
    } else {
        // note: LOOP VECTORIZED.
        for (int i = 0; i < numSamples; ++i) {
            pBuffer[i] *= old_gain;
        }
    }
}

// static
void SampleUtil::applyAlternatingGain(CSAMPLE* pBuffer, CSAMPLE gain1,
        CSAMPLE gain2, SINT numSamples) {
    // This handles gain1 == CSAMPLE_GAIN_ONE && gain2 == CSAMPLE_GAIN_ONE as well.
    if (gain1 == gain2) {
        return applyGain(pBuffer, gain1, numSamples);
    }

    // note: LOOP VECTORIZED.
    for (SINT i = 0; i < numSamples / 2; ++i) {
        pBuffer[i * 2] *= gain1;
        pBuffer[i * 2 + 1] *= gain2;
    }
}


void SampleUtil::applyRampingAlternatingGain(CSAMPLE* pBuffer,
        CSAMPLE gain1, CSAMPLE gain2,
        CSAMPLE gain1Old, CSAMPLE gain2Old, SINT numSamples) {
    if (gain1 == gain1Old && gain2 == gain2Old){
        applyAlternatingGain(pBuffer, gain1, gain2, numSamples);
        return;
    }

    const CSAMPLE_GAIN gain1Delta = (gain1 - gain1Old)
            / CSAMPLE_GAIN(numSamples / 2);
    if (gain1Delta != 0) {
        const CSAMPLE_GAIN start_gain = gain1Old + gain1Delta;
        // note: LOOP VECTORIZED.
        for (int i = 0; i < numSamples / 2; ++i) {
            const CSAMPLE_GAIN gain = start_gain + gain1Delta * i;
            pBuffer[i * 2] *= gain;
        }
    } else {
        // note: LOOP VECTORIZED.
        for (int i = 0; i < numSamples; ++i) {
            pBuffer[i * 2] *= gain1Old;
        }
    }

    const CSAMPLE_GAIN gain2Delta = (gain2 - gain2Old)
            / CSAMPLE_GAIN(numSamples / 2);
    if (gain2Delta != 0) {
        const CSAMPLE_GAIN start_gain = gain2Old + gain2Delta;
        // note: LOOP VECTORIZED.
        for (int i = 0; i < numSamples / 2; ++i) {
            const CSAMPLE_GAIN gain = start_gain + gain2Delta * i;
            pBuffer[i * 2 + 1] *= gain;
        }
    } else {
        // note: LOOP VECTORIZED.
        for (int i = 0; i < numSamples; ++i) {
            pBuffer[i * 2 + 1] *= gain2Old;
        }
    }
}

// static
void SampleUtil::add(CSAMPLE* M_RESTRICT pDest,
        const CSAMPLE* M_RESTRICT pSrc,
        SINT numSamples) {
    // note: LOOP VECTORIZED.
    for (SINT i = 0; i < numSamples; ++i) {
        pDest[i] += pSrc[i];
    }
}

// static
void SampleUtil::addWithGain(CSAMPLE* M_RESTRICT pDest,
        const CSAMPLE* M_RESTRICT pSrc,
        CSAMPLE_GAIN gain, SINT numSamples) {
    if (gain == CSAMPLE_GAIN_ZERO) {
        return;
    }

    // note: LOOP VECTORIZED.
    for (SINT i = 0; i < numSamples; ++i) {
        pDest[i] += pSrc[i] * gain;
    }
}

void SampleUtil::addWithRampingGain(CSAMPLE* M_RESTRICT pDest,
        const CSAMPLE* M_RESTRICT pSrc,
        CSAMPLE_GAIN old_gain, CSAMPLE_GAIN new_gain,
        SINT numSamples) {
    if (old_gain == CSAMPLE_GAIN_ZERO && new_gain == CSAMPLE_GAIN_ZERO) {
        return;
    }

    const CSAMPLE_GAIN gain_delta = (new_gain - old_gain)
            / CSAMPLE_GAIN(numSamples / 2);
    if (gain_delta != 0) {
        const CSAMPLE_GAIN start_gain = old_gain + gain_delta;
        // note: LOOP VECTORIZED.
        for (int i = 0; i < numSamples / 2; ++i) {
            const CSAMPLE_GAIN gain = start_gain + gain_delta * i;
            pDest[i * 2] += pSrc[i * 2] * gain;
            pDest[i * 2 + 1] += pSrc[i * 2 + 1] * gain;
        }
    } else {
        // note: LOOP VECTORIZED.
        for (int i = 0; i < numSamples; ++i) {
            pDest[i] += pSrc[i] * old_gain;
        }
    }
}

// static
void SampleUtil::add2WithGain(CSAMPLE* M_RESTRICT pDest,
        const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
        const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
        SINT numSamples) {
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        return addWithGain(pDest, pSrc2, gain2, numSamples);
    } else if (gain2 == CSAMPLE_GAIN_ZERO) {
        return addWithGain(pDest, pSrc1, gain1, numSamples);
    }

    // note: LOOP VECTORIZED.
    for (int i = 0; i < numSamples; ++i) {
        pDest[i] += pSrc1[i] * gain1 + pSrc2[i] * gain2;
    }
}

// static
void SampleUtil::add3WithGain(CSAMPLE* pDest,
        const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
        const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
        const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3,
        SINT numSamples) {
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        return add2WithGain(pDest, pSrc2, gain2, pSrc3, gain3, numSamples);
    } else if (gain2 == CSAMPLE_GAIN_ZERO) {
        return add2WithGain(pDest, pSrc1, gain1, pSrc3, gain3, numSamples);
    } else if (gain3 == CSAMPLE_GAIN_ZERO) {
        return add2WithGain(pDest, pSrc1, gain1, pSrc2, gain2, numSamples);
    }

    // note: LOOP VECTORIZED.
    for (SINT i = 0; i < numSamples; ++i) {
        pDest[i] += pSrc1[i] * gain1 + pSrc2[i] * gain2 + pSrc3[i] * gain3;
    }
}

// static
void SampleUtil::copyWithGain(CSAMPLE* M_RESTRICT pDest,
        const CSAMPLE* M_RESTRICT pSrc,
        CSAMPLE_GAIN gain, SINT numSamples) {
    if (gain == CSAMPLE_GAIN_ONE) {
        copy(pDest, pSrc, numSamples);
        return;
    }
    if (gain == CSAMPLE_GAIN_ZERO) {
        clear(pDest, numSamples);
        return;
    }

    // note: LOOP VECTORIZED.
    for (SINT i = 0; i < numSamples; ++i) {
        pDest[i] = pSrc[i] * gain;
    }

    // OR! need to test which fares better
    // copy(pDest, pSrc, iNumSamples);
    // applyGain(pDest, gain);
}

// static
void SampleUtil::copyWithRampingGain(CSAMPLE* M_RESTRICT pDest,
        const CSAMPLE* M_RESTRICT pSrc,
        CSAMPLE_GAIN old_gain,
        CSAMPLE_GAIN new_gain,
        SINT numSamples) {
    if (old_gain == CSAMPLE_GAIN_ONE && new_gain == CSAMPLE_GAIN_ONE) {
        copy(pDest, pSrc, numSamples);
        return;
    }
    if (old_gain == CSAMPLE_GAIN_ZERO && new_gain == CSAMPLE_GAIN_ZERO) {
        clear(pDest, numSamples);
        return;
    }

    const CSAMPLE_GAIN gain_delta = (new_gain - old_gain)
            / CSAMPLE_GAIN(numSamples / 2);
    if (gain_delta != 0) {
        const CSAMPLE_GAIN start_gain = old_gain + gain_delta;
        // note: LOOP VECTORIZED only with "int i"
        for (int i = 0; i < numSamples / 2; ++i) {
            const CSAMPLE_GAIN gain = start_gain + gain_delta * i;
            pDest[i * 2] = pSrc[i * 2] * gain;
            pDest[i * 2 + 1] = pSrc[i * 2 + 1] * gain;
        }
    } else {
        // note: LOOP VECTORIZED.
        for (SINT i = 0; i < numSamples; ++i) {
            pDest[i] = pSrc[i] * old_gain;
        }
    }

    // OR! need to test which fares better
    // copy(pDest, pSrc, iNumSamples);
    // applyRampingGain(pDest, gain);
}

// static
void SampleUtil::convertS16ToFloat32(CSAMPLE* M_RESTRICT pDest,
        const SAMPLE* M_RESTRICT pSrc, SINT numSamples) {
    // SAMPLE_MIN = -32768 is a valid low sample, whereas SAMPLE_MAX = 32767
    // is the highest valid sample. Note that this means that although some
    // sample values convert to -1.0, none will convert to +1.0.
    DEBUG_ASSERT(-SAMPLE_MIN >= SAMPLE_MAX);
    const CSAMPLE kConversionFactor = -SAMPLE_MIN;
    // note: LOOP VECTORIZED.
    for (SINT i = 0; i < numSamples; ++i) {
        pDest[i] = CSAMPLE(pSrc[i]) / kConversionFactor;
    }
}

//static
void SampleUtil::convertFloat32ToS16(SAMPLE* pDest, const CSAMPLE* pSrc,
        SINT numSamples) {
    DEBUG_ASSERT(-SAMPLE_MIN >= SAMPLE_MAX);
    const CSAMPLE kConversionFactor = -SAMPLE_MIN;
    // note: LOOP VECTORIZED only with "int i"
    for (int i = 0; i < numSamples; ++i) {
        pDest[i] = SAMPLE(pSrc[i] * kConversionFactor);
    }
}

// static
SampleUtil::CLIP_STATUS SampleUtil::sumAbsPerChannel(CSAMPLE* pfAbsL,
        CSAMPLE* pfAbsR, const CSAMPLE* pBuffer, SINT numSamples) {
    CSAMPLE fAbsL = CSAMPLE_ZERO;
    CSAMPLE fAbsR = CSAMPLE_ZERO;
    CSAMPLE clippedL = 0;
    CSAMPLE clippedR = 0;

    // note: LOOP VECTORIZED.
    for (SINT i = 0; i < numSamples / 2; ++i) {
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
void SampleUtil::copyClampBuffer(CSAMPLE* M_RESTRICT pDest,
        const CSAMPLE* M_RESTRICT pSrc, SINT iNumSamples) {
    // note: LOOP VECTORIZED.
    for (SINT i = 0; i < iNumSamples; ++i) {
        pDest[i] = clampSample(pSrc[i]);
    }
}

// static
void SampleUtil::interleaveBuffer(CSAMPLE* M_RESTRICT pDest,
        const CSAMPLE* M_RESTRICT pSrc1,
        const CSAMPLE* M_RESTRICT pSrc2,
        SINT numFrames) {
    // note: LOOP VECTORIZED.
    for (SINT i = 0; i < numFrames; ++i) {
        pDest[2 * i] = pSrc1[i];
        pDest[2 * i + 1] = pSrc2[i];
    }
}

// static
void SampleUtil::deinterleaveBuffer(CSAMPLE* M_RESTRICT pDest1,
        CSAMPLE* M_RESTRICT pDest2,
        const CSAMPLE* M_RESTRICT pSrc,
        SINT numFrames) {
    // note: LOOP VECTORIZED.
    for (SINT i = 0; i < numFrames; ++i) {
        pDest1[i] = pSrc[i * 2];
        pDest2[i] = pSrc[i * 2 + 1];
    }
}

// static
void SampleUtil::linearCrossfadeBuffersOut(
        CSAMPLE* pDestSrcFadeOut,
        const CSAMPLE* pSrcFadeIn,
        SINT numSamples) {
    // M_RESTRICT unoptimizes the function for some reason.
    const CSAMPLE_GAIN cross_inc = CSAMPLE_GAIN_ONE
            / CSAMPLE_GAIN(numSamples / 2);
    // note: LOOP VECTORIZED. only with "int i"
    for (int i = 0; i < numSamples / 2; ++i) {
        const CSAMPLE_GAIN cross_mix = cross_inc * i;
        pDestSrcFadeOut[i * 2] *= (CSAMPLE_GAIN_ONE - cross_mix);
        pDestSrcFadeOut[i * 2] += pSrcFadeIn[i * 2] * cross_mix;
        pDestSrcFadeOut[i * 2 + 1] *= (CSAMPLE_GAIN_ONE - cross_mix);
        pDestSrcFadeOut[i * 2 + 1] += pSrcFadeIn[i * 2 + 1] * cross_mix;
    }
}

// static
void SampleUtil::linearCrossfadeBuffersIn(
        CSAMPLE* pDestSrcFadeIn,
        const CSAMPLE* pSrcFadeOut,
        SINT numSamples) {
    // M_RESTRICT unoptimizes the function for some reason.
    const CSAMPLE_GAIN cross_inc = CSAMPLE_GAIN_ONE / CSAMPLE_GAIN(numSamples / 2);
    // note: LOOP VECTORIZED. only with "int i"
    for (int i = 0; i < numSamples / 2; ++i) {
        const CSAMPLE_GAIN cross_mix = cross_inc * i;
        pDestSrcFadeIn[i * 2] *= cross_mix;
        pDestSrcFadeIn[i * 2] += pSrcFadeOut[i * 2] * (CSAMPLE_GAIN_ONE - cross_mix);
        pDestSrcFadeIn[i * 2 + 1] *= cross_mix;
        pDestSrcFadeIn[i * 2 + 1] += pSrcFadeOut[i * 2 + 1] * (CSAMPLE_GAIN_ONE - cross_mix);
    }
}

// static
void SampleUtil::mixStereoToMono(CSAMPLE* pDest, const CSAMPLE* pSrc,
        SINT numSamples) {
    const CSAMPLE_GAIN mixScale = CSAMPLE_GAIN_ONE
            / (CSAMPLE_GAIN_ONE + CSAMPLE_GAIN_ONE);
    // note: LOOP VECTORIZED
    for (SINT i = 0; i < numSamples / 2; ++i) {
        pDest[i * 2] = (pSrc[i * 2] + pSrc[i * 2 + 1]) * mixScale;
        pDest[i * 2 + 1] = pDest[i * 2];
    }
}

// static
void SampleUtil::doubleMonoToDualMono(CSAMPLE* pBuffer, SINT numFrames) {
    // backward loop
    SINT i = numFrames;
    // Unvectorizable Loop
    while (0 < i--) {
        const CSAMPLE s = pBuffer[i];
        pBuffer[i * 2] = s;
        pBuffer[i * 2 + 1] = s;
    }
}

// static
void SampleUtil::copyMonoToDualMono(CSAMPLE* M_RESTRICT pDest,
        const CSAMPLE* M_RESTRICT pSrc, SINT numFrames) {
    // forward loop
    // note: LOOP VECTORIZED
    for (SINT i = 0; i < numFrames; ++i) {
        const CSAMPLE s = pSrc[i];
        pDest[i * 2] = s;
        pDest[i * 2 + 1] = s;
    }
}

// static
void SampleUtil::addMonoToStereo(CSAMPLE* M_RESTRICT pDest,
        const CSAMPLE* M_RESTRICT pSrc, SINT numFrames) {
    // forward loop
    // note: LOOP VECTORIZED
    for (SINT i = 0; i < numFrames; ++i) {
        const CSAMPLE s = pSrc[i];
        pDest[i * 2] += s;
        pDest[i * 2 + 1] += s;
    }
}

// static
void SampleUtil::stripMultiToStereo(
        CSAMPLE* pBuffer,
        SINT numFrames,
        int numChannels) {
    DEBUG_ASSERT(numChannels > 2);
    // forward loop
    for (SINT i = 0; i < numFrames; ++i) {
        pBuffer[i * 2] = pBuffer[i * numChannels];
        pBuffer[i * 2 + 1] = pBuffer[i * numChannels + 1];
    }
}

// static
void SampleUtil::copyMultiToStereo(
        CSAMPLE* M_RESTRICT pDest,
        const CSAMPLE* M_RESTRICT pSrc,
        SINT numFrames,
        int numChannels) {
    DEBUG_ASSERT(numChannels > 2);
    // forward loop
    for (SINT i = 0; i < numFrames; ++i) {
        pDest[i * 2] = pSrc[i * numChannels];
        pDest[i * 2 + 1] = pSrc[i * numChannels + 1];
    }
}


// static
void SampleUtil::reverse(CSAMPLE* pBuffer, SINT numSamples) {
    for (SINT j = 0; j < numSamples / 4; ++j) {
        const SINT endpos = (numSamples - 1) - j * 2 ;
        CSAMPLE temp1 = pBuffer[j * 2];
        CSAMPLE temp2 = pBuffer[j * 2 + 1];
        pBuffer[j * 2] = pBuffer[endpos - 1];
        pBuffer[j * 2 + 1] = pBuffer[endpos];
        pBuffer[endpos - 1] = temp1;
        pBuffer[endpos] = temp2;
    }
}

// static
void SampleUtil::copyReverse(CSAMPLE* M_RESTRICT pDest,
        const CSAMPLE* M_RESTRICT pSrc, SINT numSamples) {
    for (SINT j = 0; j < numSamples / 2; ++j) {
        const int endpos = (numSamples - 1) - j * 2;
        pDest[j * 2] = pSrc[endpos - 1];
        pDest[j * 2 + 1] = pSrc[endpos];
    }
}
