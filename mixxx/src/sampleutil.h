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

    // Add each sample of pSrc, multiplied by the gain, to pDest
    static void addWithGain(CSAMPLE* pDest, const CSAMPLE* pSrc,
                            CSAMPLE gain, int iNumSamples);

    // Add to each sample of pDest, pSrc1 multiplied by gain1 plus pSrc2
    // multiplied by gain2
    static void add2WithGain(CSAMPLE* pDest,
                             const CSAMPLE* pSrc1, CSAMPLE gain1,
                             const CSAMPLE* pSrc2, CSAMPLE gain2,
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


    // Convert a buffer of SAMPLEs to a buffer of CSAMPLEs. Does not work
    // in-place! pDest and pSrc must not be aliased.
    static void convert(CSAMPLE* pDest, const SAMPLE* pSrc, int iNumSamples);

};


#endif /* SAMPLEUTIL_H */
