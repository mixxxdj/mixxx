// sampleutil.h
// Created 10/5/2009 by RJ Ryan (rryan@mit.edu)

#ifndef SAMPLEUTIL_H
#define SAMPLEUTIL_H

#include "defs.h"

// A group of utilities for working with samples. Automatically use SSE/SSE2
// optimizations where possible.
class SampleUtil {
  public:
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
    static void add2WithGain(CSAMPLE* pDest,
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


};


#endif /* SAMPLEUTIL_H */
