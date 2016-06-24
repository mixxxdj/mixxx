#ifndef MIXXX_UTIL_SAMPLE_H
#define MIXXX_UTIL_SAMPLE_H

#include <algorithm>
#include <cstring> // memset

#include <QFlags>

#include "util/types.h"
#include "util/platform.h"

// A group of utilities for working with samples.
class SampleUtil {
  public:
    // If more audio channels are added in the future, this can be used
    // as bitflags, e.g CLIPPING_CH3 = 4
    enum CLIP_FLAG {
        NO_CLIPPING = 0,
        CLIPPING_LEFT = 1,
        CLIPPING_RIGHT = 2,
    };
    Q_DECLARE_FLAGS(CLIP_STATUS, CLIP_FLAG);

    // Allocated a buffer of CSAMPLE's with length size. Ensures that the buffer
    // is 16-byte aligned for SSE enhancement.
    static CSAMPLE* alloc(int size);

    // Frees a 16-byte aligned buffer allocated by SampleUtil::alloc()
    static void free(CSAMPLE* pBuffer);

    // Sets every sample in pBuffer to zero
    inline
    static void clear(CSAMPLE* pBuffer, int iNumSamples) {
        // Special case: This works, because the binary representation
        // of 0.0f is 0!
        memset(pBuffer, 0, sizeof(*pBuffer) * iNumSamples);
        //fill(pBuffer, CSAMPLE_ZERO, iNumSamples);
    }

    // Sets every sample in pBuffer to value
    inline
    static void fill(CSAMPLE* pBuffer, CSAMPLE value,
            int iNumSamples) {
        std::fill(pBuffer, pBuffer + iNumSamples, value);
    }

    // Copies every sample from pSrc to pDest
    inline
    static void copy(CSAMPLE* M_RESTRICT pDest, const CSAMPLE* M_RESTRICT pSrc,
            int iNumSamples) {
        // Benchmark results on 32 bit SSE2 Atom Cpu (Linux)
        // memcpy 7263 ns
        // std::copy 9289 ns
        // SampleUtil::copy 6565 ns
        //
        // Benchmark results from a 64 bit i5 Cpu (Linux)
        // memcpy 518 ns
        // std::copy 664 ns
        // SampleUtil::copy 661 ns
        //
        // memcpy() calls __memcpy_sse2() on 64 bit build only
        // (not available on Debian 32 bit builds)
        // However the Debian 32 bit memcpy() uses a SSE version of
        // memcpy() when called directly from Mixxx source but this
        // requires some checks that can be omitted when inlining the
        // following vectorized loop. Btw.: memcpy() calls from the Qt
        // library are not using SSE istructions.
#ifdef __SSE__
        if (sizeof(void*) == 4) { // 32 bit
            // note: LOOP VECTORIZED.
            for (int i = 0; i < iNumSamples; ++i) { // 571 ns
                pDest[i] = pSrc[i];
            }
        } else
#endif
        {
            memcpy(pDest, pSrc, iNumSamples * sizeof(CSAMPLE));
        }
    }

    // Limits a CSAMPLE value to the valid range [-CSAMPLE_PEAK, CSAMPLE_PEAK]
    inline static CSAMPLE clampSample(CSAMPLE in) {
        return CSAMPLE_clamp(in);
    }

    // Limits a CSAMPLE_GAIN value to the valid range [CSAMPLE_GAIN_MIN, CSAMPLE_GAIN_MAX]
    inline static CSAMPLE clampGain(CSAMPLE_GAIN in) {
        return CSAMPLE_GAIN_clamp(in);
    }

    // Multiply every sample in pBuffer by gain
    static void applyGain(CSAMPLE* pBuffer, CSAMPLE gain,
            int iNumSamples);

    // Copy pSrc to pDest and multiply each sample by a factor of gain.
    // For optimum performance use the in-place function applyGain()
    // if pDest == pSrc!
    static void copyWithGain(CSAMPLE* pDest, const CSAMPLE* pSrc,
            CSAMPLE_GAIN gain, int iNumSamples);

    // Apply a different gain to every other sample.
    static void applyAlternatingGain(CSAMPLE* pBuffer, CSAMPLE_GAIN gain1,
            CSAMPLE_GAIN gain2, int iNumSamples);

    // Multiply every sample in pBuffer ramping from gain1 to gain2.
    // We use ramping as often as possible to prevent soundwave discontinuities
    // which can cause audible clicks and pops.
    static void applyRampingGain(CSAMPLE* pBuffer, CSAMPLE_GAIN old_gain,
            CSAMPLE_GAIN new_gain, int iNumSamples);

    // Copy pSrc to pDest and ramp gain
    // For optimum performance use the in-place function applyRampingGain()
    // if pDest == pSrc!
    static void copyWithRampingGain(CSAMPLE* pDest, const CSAMPLE* pSrc,
            CSAMPLE_GAIN old_gain, CSAMPLE_GAIN new_gain,
            int iNumSamples);

    // Add each sample of pSrc, multiplied by the gain, to pDest
    static void addWithGain(CSAMPLE* pDest, const CSAMPLE* pSrc,
            CSAMPLE_GAIN gain, int iNumSamples);

    // Add each sample of pSrc, multiplied by the gain, to pDest
    static void addWithRampingGain(CSAMPLE* pDest, const CSAMPLE* pSrc,
            CSAMPLE_GAIN old_gain, CSAMPLE_GAIN new_gain,
            int iNumSamples);

    // Add to each sample of pDest, pSrc1 multiplied by gain1 plus pSrc2
    // multiplied by gain2
    static void add2WithGain(CSAMPLE* pDest, const CSAMPLE* pSrc1,
            CSAMPLE_GAIN gain1, const CSAMPLE* pSrc2, CSAMPLE_GAIN gain2,
            int iNumSamples);

    // Add to each sample of pDest, pSrc1 multiplied by gain1 plus pSrc2
    // multiplied by gain2 plus pSrc3 multiplied by gain3
    static void add3WithGain(CSAMPLE* pDest, const CSAMPLE* pSrc1,
            CSAMPLE_GAIN gain1, const CSAMPLE* pSrc2, CSAMPLE_GAIN gain2,
            const CSAMPLE* pSrc3, CSAMPLE_GAIN gain3, int iNumSamples);

    // Convert and normalize a buffer of SAMPLEs in the range [-SAMPLE_MAX, SAMPLE_MAX]
    // to a buffer of CSAMPLEs in the range [-1.0, 1.0].
    static void convertS16ToFloat32(CSAMPLE* pDest, const SAMPLE* pSrc,
            int iNumSamples);

    // Convert and normalize a buffer of CSAMPLEs in the range [-1.0, 1.0]
    // to a buffer of SAMPLEs in the range [-SAMPLE_MAX, SAMPLE_MAX].
    static void convertFloat32ToS16(SAMPLE* pDest, const CSAMPLE* pSrc,
            unsigned int iNumSamples);

    // For each pair of samples in pBuffer (l,r) -- stores the sum of the
    // absolute values of l in pfAbsL, and the sum of the absolute values of r
    // in pfAbsR.
    // The return value tells whether there is clipping in pBuffer or not.
    static CLIP_STATUS sumAbsPerChannel(CSAMPLE* pfAbsL, CSAMPLE* pfAbsR,
            const CSAMPLE* pBuffer, int iNumSamples);

    // Copies every sample in pSrc to pDest, limiting the values in pDest
    // to the valid range of CSAMPLE. If pDest and pSrc are aliases, will
    // not copy will only clamp. Returns true if any samples in pSrc were
    // outside the valid range of CSAMPLE.
    static void copyClampBuffer(CSAMPLE* pDest, const CSAMPLE* pSrc,
            int iNumSamples);

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

    // In-place doubles the mono samples in pBuffer to dual mono samples.
    // (numFrames) samples will be read from pBuffer
    // (numFrames * 2) samples will be written into pBuffer
    static void doubleMonoToDualMono(CSAMPLE* pBuffer, int numFrames);

    // Copies and doubles the mono samples in pSrc to dual mono samples
    // into pDest.
    // (numFrames) samples will be read from pSrc
    // (numFrames * 2) samples will be written into pDest
    static void copyMonoToDualMono(CSAMPLE* pDest, const CSAMPLE* pSrc,
            int numFrames);

    // In-place strips interleaved multi-channel samples in pBuffer with
    // numChannels >= 2 down to stereo samples. Only samples from the first
    // two channels will be read and written. Samples from all other
    // channels are discarded.
    // pBuffer must contain (numFrames * numChannels) samples
    // (numFrames * 2) samples will be written into pBuffer
    static void stripMultiToStereo(CSAMPLE* pBuffer, int numFrames,
            int numChannels);

    // Copies and strips interleaved multi-channel sample data in pSrc with
    // numChannels >= 2 down to stereo samples into pDest. Only samples from
    // the first two channels will be read and written. Samples from all other
    // channels will be ignored.
    // pSrc must contain (numFrames * numChannels) samples
    // (numFrames * 2) samples will be written into pDest
    static void copyMultiToStereo(CSAMPLE* pDest, const CSAMPLE* pSrc,
            int numFrames, int numChannels);

    // reverses stereo sample in place
    static void reverse(CSAMPLE* pBuffer, int iNumSamples);

    // copy pSrc to pDest and reverses stereo sample order (backward)
    static void copyReverse(CSAMPLE* M_RESTRICT pDest,
            const CSAMPLE* M_RESTRICT pSrc, int iNumSamples);


    // Include auto-generated methods (e.g. copyXWithGain, copyXWithRampingGain,
    // etc.)
#include "util/sample_autogen.h"
};

Q_DECLARE_OPERATORS_FOR_FLAGS(SampleUtil::CLIP_STATUS);

#endif /* MIXXX_UTIL_SAMPLE_H */
