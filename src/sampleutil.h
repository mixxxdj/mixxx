// sampleutil.h
// Created 10/5/2009 by RJ Ryan (rryan@mit.edu)

#ifndef SAMPLEUTIL_H
#define SAMPLEUTIL_H

#include "defs.h"

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

// A group of utilities for working with samples. Automatically use SSE/SSE2
// optimizations where possible.
class SampleUtil {
  public:
    static bool m_sOptimizationsOn;

    // Allocated a buffer of CSAMPLE's with length size. Ensures that the buffer
    // is 16-byte aligned for use for use with SampleUtil SSE enhanced
    // functions.
    static CSAMPLE* alloc(int size);

    // Frees a 16-byte aligned buffer allocated by SampleUtil::alloc()
    static void free(CSAMPLE* pBuffer);

    // Multiply every sample in pBuffer by gain
    static void applyGain(CSAMPLE* pBuffer, CSAMPLE gain, int iNumSamples);

    // Apply a different gain to every other sample.
    static void applyAlternatingGain(CSAMPLE* pBuffer,
                                     CSAMPLE gain1, CSAMPLE gain2,
                                     int iNumSamples);

    // Add each sample of pSrc, multiplied by the gain, to pDest
    static void addWithGain(CSAMPLE* pDest, const CSAMPLE* pSrc,
                            CSAMPLE gain, int iNumSamples);

    // Add to each sample of pDest, pSrc1 multiplied by gain1 plus pSrc2
    // multiplied by gain2
    static void add2WithGain(CSAMPLE* pDest,
                             const CSAMPLE* pSrc1, CSAMPLE gain1,
                             const CSAMPLE* pSrc2, CSAMPLE gain2,
                             int iNumSamples);

    // Add to each sample of pDest, pSrc1 multiplied by gain1 plus pSrc2
    // multiplied by gain2 plus pSrc3 multiplied by gain3
    static void add3WithGain(CSAMPLE* pDest,
                             const CSAMPLE* pSrc1, CSAMPLE gain1,
                             const CSAMPLE* pSrc2, CSAMPLE gain2,
                             const CSAMPLE* pSrc3, CSAMPLE gain3,
                             int iNumSamples);

    // Copy pSrc to pDest and multiply each sample by a factor of gain.
    static void copyWithGain(CSAMPLE* pDest, const CSAMPLE* pSrc,
                             CSAMPLE gain, int iNumSamples);

    // Copies the sum of each channel, multiplied by its gain into pDest
    static void copy2WithGain(CSAMPLE* pDest,
                              const CSAMPLE* pSrc1, CSAMPLE gain1,
                              const CSAMPLE* pSrc2, CSAMPLE gain2,
                              int iNumSamples);

    // Copies the sum of each channel, multiplied by its gain into pDest
    static void copy3WithGain(CSAMPLE* pDest,
                              const CSAMPLE* pSrc1, CSAMPLE gain1,
                              const CSAMPLE* pSrc2, CSAMPLE gain2,
                              const CSAMPLE* pSrc3, CSAMPLE gain3,
                              int iNumSamples);

    // Copies the sum of each channel, multiplied by its gain into pDest
    static void copy4WithGain(CSAMPLE* pDest,
                              const CSAMPLE* pSrc1, CSAMPLE gain1,
                              const CSAMPLE* pSrc2, CSAMPLE gain2,
                              const CSAMPLE* pSrc3, CSAMPLE gain3,
                              const CSAMPLE* pSrc4, CSAMPLE gain4,
                              int iNumSamples);

    // Copies the sum of each channel, multiplied by its gain into pDest
    static void copy5WithGain(CSAMPLE* pDest,
                              const CSAMPLE* pSrc1, CSAMPLE gain1,
                              const CSAMPLE* pSrc2, CSAMPLE gain2,
                              const CSAMPLE* pSrc3, CSAMPLE gain3,
                              const CSAMPLE* pSrc4, CSAMPLE gain4,
                              const CSAMPLE* pSrc5, CSAMPLE gain5,
                              int iNumSamples);

    // Copies the sum of each channel, multiplied by its gain into pDest
    static void copy6WithGain(CSAMPLE* pDest,
                              const CSAMPLE* pSrc1, CSAMPLE gain1,
                              const CSAMPLE* pSrc2, CSAMPLE gain2,
                              const CSAMPLE* pSrc3, CSAMPLE gain3,
                              const CSAMPLE* pSrc4, CSAMPLE gain4,
                              const CSAMPLE* pSrc5, CSAMPLE gain5,
                              const CSAMPLE* pSrc6, CSAMPLE gain6,
                              int iNumSamples);

    // Copies the sum of each channel, multiplied by its gain into pDest
    static void copy7WithGain(CSAMPLE* pDest,
                              const CSAMPLE* pSrc1, CSAMPLE gain1,
                              const CSAMPLE* pSrc2, CSAMPLE gain2,
                              const CSAMPLE* pSrc3, CSAMPLE gain3,
                              const CSAMPLE* pSrc4, CSAMPLE gain4,
                              const CSAMPLE* pSrc5, CSAMPLE gain5,
                              const CSAMPLE* pSrc6, CSAMPLE gain6,
                              const CSAMPLE* pSrc7, CSAMPLE gain7,
                              int iNumSamples);

    // Convert a buffer of SAMPLEs to a buffer of CSAMPLEs. Does not work
    // in-place! pDest and pSrc must not be aliased.
    static void convert(CSAMPLE* pDest, const SAMPLE* pSrc, int iNumSamples);

    // For each pair of samples in pBuffer (l,r) -- stores the sum of the
    // absolute values of l in pfAbsL, and the sum of the absolute values of r
    // in pfAbsR.
    static void sumAbsPerChannel(CSAMPLE* pfAbsL, CSAMPLE* pfAbsR,
                                 const CSAMPLE* pBuffer, int iNumSamples);

    // Returns true if the buffer contains any samples outside of the range
    // [fMin,fMax].
    static bool isOutsideRange(CSAMPLE fMax, CSAMPLE fMin,
                               const CSAMPLE* pBuffer, int iNumSamples);

    // Copied every sample in pSrc to pDest, limiting the values in pDest to the
    // range [fMin, fMax]. If pDest and pSrc are aliases, will not copy -- will
    // only clamp. Returns true if any samples in pSrc were outside the range
    // [fMin, fMax].
    static bool copyClampBuffer(CSAMPLE fMax, CSAMPLE fMin,
                                CSAMPLE* pDest, const CSAMPLE* pSrc,
                                int iNumSamples);

    // Interleave the samples in pSrc1 and pSrc2 into pDest. iNumSamples must be
    // the number of samples in pSrc1 and pSrc2, and pDest must have at least
    // space for iNumSamples*2 samples. pDest must not be an alias of pSrc1 or
    // pSrc2.
    static void interleaveBuffer(CSAMPLE* pDest,
                                 const CSAMPLE* pSrc1, const CSAMPLE* pSrc2,
                                 int iNumSamples);

    // Deinterleave the samples in pSrc alternately into pDest1 and
    // pDest2. iNumSamples must be the number of samples in pDest1 and pDest2,
    // and pSrc must have at least iNumSamples*2 samples. Neither pDest1 or
    // pDest2 can be aliases of pSrc.
    static void deinterleaveBuffer(CSAMPLE* pDest1, CSAMPLE* pDest2,
                                   const CSAMPLE* pSrc, int iNumSamples);

    static void setOptimizations(bool opt);

  private:
    static void sseApplyGain(CSAMPLE* pBuffer,
                             CSAMPLE gain, int iNumSamples) _ALIGN_STACK;
    static void sseApplyAlternatingGain(CSAMPLE* pBuffer,
                                        CSAMPLE gain1, CSAMPLE gain2,
                                        int iNumSamples) _ALIGN_STACK;
    static void sseAddWithGain(CSAMPLE* pDest, const CSAMPLE* pSrc,
                               CSAMPLE gain, int iNumSamples) _ALIGN_STACK;
    static void sseAdd2WithGain(CSAMPLE* pDest,
                                const CSAMPLE* pSrc1, CSAMPLE gain1,
                                const CSAMPLE* pSrc2, CSAMPLE gain2,
                                int iNumSamples) _ALIGN_STACK;
    static void sseAdd3WithGain(CSAMPLE* pDest,
                                const CSAMPLE* pSrc1, CSAMPLE gain1,
                                const CSAMPLE* pSrc2, CSAMPLE gain2,
                                const CSAMPLE* pSrc3, CSAMPLE gain3,
                                int iNumSamples) _ALIGN_STACK;

    static void sseCopyWithGain(CSAMPLE* pDest, const CSAMPLE* pSrc,
                                CSAMPLE gain, int iNumSamples) _ALIGN_STACK;
    static void sseCopy2WithGain(CSAMPLE* pDest,
                                 const CSAMPLE* pSrc1, CSAMPLE gain1,
                                 const CSAMPLE* pSrc2, CSAMPLE gain2,
                                 int iNumSamples) _ALIGN_STACK;
    static void sseCopy3WithGain(CSAMPLE* pDest,
                                 const CSAMPLE* pSrc1, CSAMPLE gain1,
                                 const CSAMPLE* pSrc2, CSAMPLE gain2,
                                 const CSAMPLE* pSrc3, CSAMPLE gain3,
                                 int iNumSamples) _ALIGN_STACK;
    static void sseConvert(CSAMPLE* pDest,
                           const SAMPLE* pSrc, int iNumSamples) _ALIGN_STACK;
    static void sseSumAbsPerChannel(CSAMPLE* pfAbsL, CSAMPLE* pfAbsR,
                                    const CSAMPLE* pBuffer,
                                    int iNumSamples) _ALIGN_STACK;
    static bool sseIsOutsideRange(CSAMPLE fMax, CSAMPLE fMin,
                                  const CSAMPLE* pBuffer,
                                  int iNumSamples) _ALIGN_STACK;
    static bool sseCopyClampBuffer(CSAMPLE fMax, CSAMPLE fMin,
                                   CSAMPLE* pDest, const CSAMPLE* pSrc,
                                   int iNumSamples) _ALIGN_STACK;
    static void sseInterleaveBuffer(CSAMPLE* pDest,
                                    const CSAMPLE* pSrc1, const CSAMPLE* pSrc2,
                                    int iNumSamples) _ALIGN_STACK;
    static void sseDeinterleaveBuffer(CSAMPLE* pDest1, CSAMPLE* pDest2,
                                      const CSAMPLE* pSrc,
                                      int iNumSamples) _ALIGN_STACK;



};


#endif /* SAMPLEUTIL_H */
