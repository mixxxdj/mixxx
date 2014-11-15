// sampleutil.h
// Created 10/5/2009 by RJ Ryan (rryan@mit.edu)

#ifndef SAMPLEUTIL_H
#define SAMPLEUTIL_H

#include "util/types.h"
#include "util/math.h"

// NOTE(uklotzde): We assume that the STL algorithms are thoroughly optimized!
#include <algorithm>

#include <cstring> // memset

// MSVC does this
// __declspec(align(16))
// while GCC does
// __attribute__((aligned(16)))
// IntelCC does
// _MM_ALIGN_16
// but I dont know how to test for ICC.

#if !_ALIGN_16
#define _ALIGN_16
#define _ALIGN_STACK
#elif (defined __GNUC__)
#define _ALIGN_16 __attribute__((aligned(16)))
#define _ALIGN_STACK __attribute__((force_align_arg_pointer, aligned(16)))
#elif (defined _MSC_VER)
#define _ALIGN_16 __declspec(align(16))
#define _ALIGN_STACK
#else
#error Please email mixxx-devel@lists.sourceforge.net and tell us what is the equivalent of __attribute__((aligned(16))) for your compiler.
#endif

#define assert_aligned(data) (Q_ASSERT(reinterpret_cast<int64_t>(data) % 16 == 0));

// A group of utilities for working with samples.
class SampleUtil {
public:
    static bool m_sOptimizationsOn;

    // Allocated a buffer of CSAMPLE's with length size. Ensures that the buffer
    // is 16-byte aligned for SSE enhancement.
    static CSAMPLE* alloc(unsigned int size);

    // Frees a 16-byte aligned buffer allocated by SampleUtil::alloc()
    static void free(CSAMPLE* pBuffer);

    // Multiply every sample in pBuffer by gain
    static void applyGain(CSAMPLE* pBuffer, CSAMPLE gain,
            unsigned int iNumSamples);

    // Sets every sample in pBuffer to zero
    inline
    static void clear(CSAMPLE* pBuffer, unsigned int iNumSamples) {
        // Special case: This works, because the binary representation
        // of 0.0f is 0!
        memset(pBuffer, 0, sizeof(*pBuffer) * iNumSamples);
        //fill(pBuffer, CSAMPLE_ZERO, iNumSamples);
    }

    // Sets every sample in pBuffer to value
    inline
    static void fill(CSAMPLE* pBuffer, CSAMPLE value, unsigned int iNumSamples) {
        std::fill(pBuffer, pBuffer + iNumSamples, value);
    }

    // Copies every sample from pSrc to pDest
    inline
    static void copy(CSAMPLE* pDest, const CSAMPLE* pSrc,
            unsigned int iNumSamples) {
        std::copy(pSrc, pSrc + iNumSamples, pDest);
    }

    // In-place widens the mono samples in pBuffer to stereo
    // samples.
    // (numFrames) samples will be read from pBuffer
    // (numFrames * 2) samples will be written to pBuffer
    static void widenMonoToStereo(SAMPLE* pBuffer, unsigned int numFrames);

    // In-place widens the mono samples in pBuffer to stereo
    // samples.
    // (numFrames) samples will be read from pBuffer
    // (numFrames * 2) samples will be written into pBuffer
    static void widenMonoToStereo(CSAMPLE* pBuffer, unsigned int numFrames);

    // Copies and widens the mono samples in pSrc to stereo
    // samples into pDest.
    // (numFrames) samples will be read from pSrc
    // (numFrames * 2) samples will be written into pDest
    static void copyWidenMonoToStereo(CSAMPLE* pDest, const CSAMPLE* pSrc,
            unsigned int numFrames);

    // In-place narrows interleaved multi-channel samples in
    // pBuffer with numChannels >= 2 to stereo samples.
    // Only samples from the first two channels are used.
    // pBuffer must contain (numFrames * numChannels) samples
    // (numFrames * 2) samples will be written into pBuffer
    static void narrowMultiToStereo(CSAMPLE* pBuffer, unsigned int numFrames,
            unsigned int numChannels);

    // Copies and narrows interleaved multi-channel sample data
    // in pSrc with numChannels >= 2 to stereo samples into pDest.
    // Only samples from the first two channels are used.
    // pSrc must contain (numFrames * numChannels) samples
    // (numFrames * 2) samples will be written into pDest
    static void copyNarrowMultiToStereo(CSAMPLE* pDest, const CSAMPLE* pSrc,
            unsigned int numFrames, unsigned int numChannels);

    // Apply a different gain to every other sample.
    static void applyAlternatingGain(CSAMPLE* pBuffer, CSAMPLE gain1,
            CSAMPLE gain2, int iNumSamples);

    // Multiply every sample in pBuffer ramping from gain1 to gain2.
    // We use ramping as often as possible to prevent soundwave discontinuities
    // which can cause audible clicks and pops.
    static void applyRampingGain(CSAMPLE* pBuffer, CSAMPLE old_gain,
            CSAMPLE new_gain, int iNumSamples);

    // Add each sample of pSrc, multiplied by the gain, to pDest
    static void addWithGain(CSAMPLE* pDest, const CSAMPLE* pSrc, CSAMPLE gain,
            int iNumSamples);

    // Add each sample of pSrc, multiplied by the gain, to pDest
    static void addWithRampingGain(CSAMPLE* pDest, const CSAMPLE* pSrc,
            CSAMPLE old_gain, CSAMPLE new_gain, int iNumSamples);

    // Add to each sample of pDest, pSrc1 multiplied by gain1 plus pSrc2
    // multiplied by gain2
    static void add2WithGain(CSAMPLE* pDest, const CSAMPLE* pSrc1,
            CSAMPLE gain1, const CSAMPLE* pSrc2, CSAMPLE gain2,
            int iNumSamples);

    // Add to each sample of pDest, pSrc1 multiplied by gain1 plus pSrc2
    // multiplied by gain2 plus pSrc3 multiplied by gain3
    static void add3WithGain(CSAMPLE* pDest, const CSAMPLE* pSrc1,
            CSAMPLE gain1, const CSAMPLE* pSrc2, CSAMPLE gain2,
            const CSAMPLE* pSrc3, CSAMPLE gain3, int iNumSamples);

    // Copy pSrc to pDest and multiply each sample by a factor of gain.
    static void copyWithGain(CSAMPLE* pDest, const CSAMPLE* pSrc, CSAMPLE gain,
            int iNumSamples);

    // Copy pSrc to pDest and ramp gain
    static void copyWithRampingGain(CSAMPLE* pDest, const CSAMPLE* pSrc,
            CSAMPLE old_gain, CSAMPLE new_gain, int iNumSamples);

    // Convert and normalize a buffer of SAMPLEs in the range [-SHRT_MAX, SHRT_MAX]
    // to a buffer of CSAMPLEs in the range [-1.0, 1.0].
    static void convertS16ToFloat32(CSAMPLE* pDest, const SAMPLE* pSrc,
            int iNumSamples);

    // Convert and normalize a buffer of CSAMPLEs in the range [CSAMPLE_MIN, CSAMPLE_MAX]
    // to a buffer of SAMPLEs in the range [-SHRT_MAX, SHRT_MAX].
    static void convertFloat32ToS16(SAMPLE* pDest, const CSAMPLE* pSrc,
            int iNumSamples);

    // For each pair of samples in pBuffer (l,r) -- stores the sum of the
    // absolute values of l in pfAbsL, and the sum of the absolute values of r
    // in pfAbsR.
    // returns true in case of clipping > +-1
    static bool sumAbsPerChannel(CSAMPLE* pfAbsL, CSAMPLE* pfAbsR,
            const CSAMPLE* pBuffer, int iNumSamples);

    // Returns true if the buffer contains any samples outside of the range
    // [fMin,fMax].
    static bool isOutsideRange(CSAMPLE fMax, CSAMPLE fMin,
            const CSAMPLE* pBuffer, int iNumSamples);

    // Copied every sample in pSrc to pDest, limiting the values in pDest to the
    // range [fMin, fMax]. If pDest and pSrc are aliases, will not copy -- will
    // only clamp. Returns true if any samples in pSrc were outside the range
    // [fMin, fMax].
    static void copyClampBuffer(CSAMPLE* pDest, const CSAMPLE* pSrc,
            int iNumSamples);

    // returns a SAMPLE that is between CSAMPLE_MIN and CSAMPLE_MAX
    inline static CSAMPLE clampSample(CSAMPLE in) {
        return math_clamp(in, CSAMPLE_MIN, CSAMPLE_MAX);
    }

    // Interleave the samples in pSrc1 and pSrc2 into pDest. iNumSamples must be
    // the number of samples in pSrc1 and pSrc2, and pDest must have at least
    // space for iNumSamples*2 samples. pDest must not be an alias of pSrc1 or
    // pSrc2.
    static void interleaveBuffer(CSAMPLE* pDest, const CSAMPLE* pSrc1,
            const CSAMPLE* pSrc2, int iNumSamples);

    // Deinterleave the samples in pSrc alternately into pDest1 and
    // pDest2. iNumSamples must be the number of samples in pDest1 and pDest2,
    // and pSrc must have at least iNumSamples*2 samples. Neither pDest1 or
    // pDest2 can be aliases of pSrc.
    static void deinterleaveBuffer(CSAMPLE* pDest1, CSAMPLE* pDest2,
            const CSAMPLE* pSrc, int iNumSamples);

    // Crossfade two buffers together and put the result in pDest.  All the
    // buffers must be the same length.  pDest may be an alias of the source
    // buffers.  It is preferable to use the copyWithRamping functions, but
    // sometimes this function is necessary.
    static void linearCrossfadeBuffers(CSAMPLE* pDest,
            const CSAMPLE* pSrcFadeOut, const CSAMPLE* pSrcFadeIn,
            int iNumSamples);

    // Mix a buffer down to mono, putting the result in both of the channels.
    // This uses a simple (L+R)/2 method, which assumes that the audio is
    // "mono-compatible", ie there are no major out-of-phase parts of the signal.
    static void mixStereoToMono(CSAMPLE* pDest, const CSAMPLE* pSrc,
            int iNumSamples);

    // Include auto-generated methods (e.g. copyXWithGain, copyXWithRampingGain,
    // etc.)
#include "sampleutil_autogen.h"
};

#endif /* SAMPLEUTIL_H */
