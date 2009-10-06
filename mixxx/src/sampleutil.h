// sampleutil.h
// Created 10/5/2009 by RJ Ryan (rryan@mit.edu)

#ifndef SAMPLEUTIL_H
#define SAMPLEUTIL_H

#include "defs.h"

// A group of utilities for working with samples. Automatically use SSE/SSE2
// optimizations where possible.
class SampleUtil {
  public:

    // Allocated a buffer of CSAMPLE's with length size. Ensures that the buffer
    // is 16-byte aligned for use for use with SampleUtil SSE enhanced
    // functions.
    static CSAMPLE* alloc(int size);

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

    // Copy to pDest, each sample of pSrc1 multiplied by gain1 plus pSrc2
    // multiplied by gain2
    static void copy2WithGain(CSAMPLE* pDest,
                              const CSAMPLE* pSrc1, CSAMPLE gain1,
                              const CSAMPLE* pSrc2, CSAMPLE gain2,
                              int iNumSamples);

    // Copy to pDest, each sample of pSrc1 multiplied by gain1 plus pSrc2
    // multiplied by gain2 plus pSrc3 multiplied by gain3
    static void copy3WithGain(CSAMPLE* pDest,
                              const CSAMPLE* pSrc1, CSAMPLE gain1,
                              const CSAMPLE* pSrc2, CSAMPLE gain2,
                              const CSAMPLE* pSrc3, CSAMPLE gain3,
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

    static void setOptimizations(bool opt);

  private:
    static bool m_sOptimizationsOn;
    static void sseApplyGain(CSAMPLE* pBuffer, CSAMPLE gain, int iNumSamples);
    static void sseApplyAlternatingGain(CSAMPLE* pBuffer,
                                        CSAMPLE gain1, CSAMPLE gain2,
                                        int iNumSamples);
    static void sseAddWithGain(CSAMPLE* pDest, const CSAMPLE* pSrc,
                               CSAMPLE gain, int iNumSamples);
    static void sseAdd2WithGain(CSAMPLE* pDest,
                                const CSAMPLE* pSrc1, CSAMPLE gain1,
                                const CSAMPLE* pSrc2, CSAMPLE gain2,
                                int iNumSamples);
    static void sseAdd3WithGain(CSAMPLE* pDest,
                                const CSAMPLE* pSrc1, CSAMPLE gain1,
                                const CSAMPLE* pSrc2, CSAMPLE gain2,
                                const CSAMPLE* pSrc3, CSAMPLE gain3,
                                int iNumSamples);

    static void sseCopyWithGain(CSAMPLE* pDest, const CSAMPLE* pSrc,
                                CSAMPLE gain, int iNumSamples);
    static void sseCopy2WithGain(CSAMPLE* pDest,
                                 const CSAMPLE* pSrc1, CSAMPLE gain1,
                                 const CSAMPLE* pSrc2, CSAMPLE gain2,
                                 int iNumSamples);
    static void sseCopy3WithGain(CSAMPLE* pDest,
                                 const CSAMPLE* pSrc1, CSAMPLE gain1,
                                 const CSAMPLE* pSrc2, CSAMPLE gain2,
                                 const CSAMPLE* pSrc3, CSAMPLE gain3,
                                 int iNumSamples);
    static void sseConvert(CSAMPLE* pDest, const SAMPLE* pSrc, int iNumSamples);
    static void sseSumAbsPerChannel(CSAMPLE* pfAbsL, CSAMPLE* pfAbsR,
                                    const CSAMPLE* pBuffer, int iNumSamples);
    static bool sseIsOutsideRange(CSAMPLE fMax, CSAMPLE fMin,
                                  const CSAMPLE* pBuffer, int iNumSamples);
    static bool sseCopyClampBuffer(CSAMPLE fMax, CSAMPLE fMin,
                                   CSAMPLE* pDest, const CSAMPLE* pSrc,
                                   int iNumSamples);



};


#endif /* SAMPLEUTIL_H */
