#ifndef SAMPLEUTILAUTOGEN_H
#define SAMPLEUTILAUTOGEN_H
////////////////////////////////////////////////////////
// THIS FILE IS AUTO-GENERATED. DO NOT EDIT DIRECTLY! //
// SEE scripts/generate_sample_functions.py           //
////////////////////////////////////////////////////////
static inline void copy1WithGain(CSAMPLE* pDest,
                                 const CSAMPLE* pSrc0, CSAMPLE gain0,
                                 int iNumSamples) {
    copyWithGain(pDest, pSrc0, gain0, iNumSamples);
    return;
}
static inline void copy1WithRampingGain(CSAMPLE* pDest,
                                        const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                        int iNumSamples) {
    copyWithRampingGain(pDest, pSrc0, gain0in, gain0out, iNumSamples);
    return;
}
static inline void copy2WithGain(CSAMPLE* pDest,
                                 const CSAMPLE* pSrc0, CSAMPLE gain0,
                                 const CSAMPLE* pSrc1, CSAMPLE gain1,
                                 int iNumSamples) {
    if (gain0 == 0.0) {
        copy1WithGain(pDest, pSrc1, gain1, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy1WithGain(pDest, pSrc0, gain0, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1;
    }
}
static inline void copy2WithRampingGain(CSAMPLE* pDest,
                                        const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                        const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                        int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy1WithRampingGain(pDest, pSrc1, gain1in, gain1out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy1WithRampingGain(pDest, pSrc0, gain0in, gain0out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1;
    }
}
static inline void copy3WithGain(CSAMPLE* pDest,
                                 const CSAMPLE* pSrc0, CSAMPLE gain0,
                                 const CSAMPLE* pSrc1, CSAMPLE gain1,
                                 const CSAMPLE* pSrc2, CSAMPLE gain2,
                                 int iNumSamples) {
    if (gain0 == 0.0) {
        copy2WithGain(pDest, pSrc1, gain1, pSrc2, gain2, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy2WithGain(pDest, pSrc0, gain0, pSrc2, gain2, iNumSamples);
        return;
    }
    if (gain2 == 0.0) {
        copy2WithGain(pDest, pSrc0, gain0, pSrc1, gain1, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2;
    }
}
static inline void copy3WithRampingGain(CSAMPLE* pDest,
                                        const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                        const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                        const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                                        int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy2WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy2WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, iNumSamples);
        return;
    }
    if (gain2in == 0.0 && gain2out == 0.0) {
        copy2WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    CSAMPLE gain2 = gain2in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1, gain2 += delta2) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1 +
                       pSrc2[i + 1] * gain2;
    }
}
static inline void copy4WithGain(CSAMPLE* pDest,
                                 const CSAMPLE* pSrc0, CSAMPLE gain0,
                                 const CSAMPLE* pSrc1, CSAMPLE gain1,
                                 const CSAMPLE* pSrc2, CSAMPLE gain2,
                                 const CSAMPLE* pSrc3, CSAMPLE gain3,
                                 int iNumSamples) {
    if (gain0 == 0.0) {
        copy3WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy3WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, iNumSamples);
        return;
    }
    if (gain2 == 0.0) {
        copy3WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, iNumSamples);
        return;
    }
    if (gain3 == 0.0) {
        copy3WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3;
    }
}
static inline void copy4WithRampingGain(CSAMPLE* pDest,
                                        const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                        const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                        const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                                        const CSAMPLE* pSrc3, CSAMPLE gain3in, CSAMPLE gain3out,
                                        int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy3WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy3WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, iNumSamples);
        return;
    }
    if (gain2in == 0.0 && gain2out == 0.0) {
        copy3WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, iNumSamples);
        return;
    }
    if (gain3in == 0.0 && gain3out == 0.0) {
        copy3WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    CSAMPLE gain2 = gain2in;
    const CSAMPLE delta3 = 2.0 * (gain3out - gain3in) / iNumSamples;
    CSAMPLE gain3 = gain3in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1, gain2 += delta2, gain3 += delta3) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1 +
                       pSrc2[i + 1] * gain2 +
                       pSrc3[i + 1] * gain3;
    }
}
static inline void copy5WithGain(CSAMPLE* pDest,
                                 const CSAMPLE* pSrc0, CSAMPLE gain0,
                                 const CSAMPLE* pSrc1, CSAMPLE gain1,
                                 const CSAMPLE* pSrc2, CSAMPLE gain2,
                                 const CSAMPLE* pSrc3, CSAMPLE gain3,
                                 const CSAMPLE* pSrc4, CSAMPLE gain4,
                                 int iNumSamples) {
    if (gain0 == 0.0) {
        copy4WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy4WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, iNumSamples);
        return;
    }
    if (gain2 == 0.0) {
        copy4WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, iNumSamples);
        return;
    }
    if (gain3 == 0.0) {
        copy4WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, iNumSamples);
        return;
    }
    if (gain4 == 0.0) {
        copy4WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4;
    }
}
static inline void copy5WithRampingGain(CSAMPLE* pDest,
                                        const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                        const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                        const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                                        const CSAMPLE* pSrc3, CSAMPLE gain3in, CSAMPLE gain3out,
                                        const CSAMPLE* pSrc4, CSAMPLE gain4in, CSAMPLE gain4out,
                                        int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy4WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy4WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, iNumSamples);
        return;
    }
    if (gain2in == 0.0 && gain2out == 0.0) {
        copy4WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, iNumSamples);
        return;
    }
    if (gain3in == 0.0 && gain3out == 0.0) {
        copy4WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, iNumSamples);
        return;
    }
    if (gain4in == 0.0 && gain4out == 0.0) {
        copy4WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    CSAMPLE gain2 = gain2in;
    const CSAMPLE delta3 = 2.0 * (gain3out - gain3in) / iNumSamples;
    CSAMPLE gain3 = gain3in;
    const CSAMPLE delta4 = 2.0 * (gain4out - gain4in) / iNumSamples;
    CSAMPLE gain4 = gain4in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1, gain2 += delta2, gain3 += delta3, gain4 += delta4) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1 +
                       pSrc2[i + 1] * gain2 +
                       pSrc3[i + 1] * gain3 +
                       pSrc4[i + 1] * gain4;
    }
}
static inline void copy6WithGain(CSAMPLE* pDest,
                                 const CSAMPLE* pSrc0, CSAMPLE gain0,
                                 const CSAMPLE* pSrc1, CSAMPLE gain1,
                                 const CSAMPLE* pSrc2, CSAMPLE gain2,
                                 const CSAMPLE* pSrc3, CSAMPLE gain3,
                                 const CSAMPLE* pSrc4, CSAMPLE gain4,
                                 const CSAMPLE* pSrc5, CSAMPLE gain5,
                                 int iNumSamples) {
    if (gain0 == 0.0) {
        copy5WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy5WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, iNumSamples);
        return;
    }
    if (gain2 == 0.0) {
        copy5WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, iNumSamples);
        return;
    }
    if (gain3 == 0.0) {
        copy5WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, iNumSamples);
        return;
    }
    if (gain4 == 0.0) {
        copy5WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, iNumSamples);
        return;
    }
    if (gain5 == 0.0) {
        copy5WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5;
    }
}
static inline void copy6WithRampingGain(CSAMPLE* pDest,
                                        const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                        const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                        const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                                        const CSAMPLE* pSrc3, CSAMPLE gain3in, CSAMPLE gain3out,
                                        const CSAMPLE* pSrc4, CSAMPLE gain4in, CSAMPLE gain4out,
                                        const CSAMPLE* pSrc5, CSAMPLE gain5in, CSAMPLE gain5out,
                                        int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy5WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy5WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, iNumSamples);
        return;
    }
    if (gain2in == 0.0 && gain2out == 0.0) {
        copy5WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, iNumSamples);
        return;
    }
    if (gain3in == 0.0 && gain3out == 0.0) {
        copy5WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, iNumSamples);
        return;
    }
    if (gain4in == 0.0 && gain4out == 0.0) {
        copy5WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, iNumSamples);
        return;
    }
    if (gain5in == 0.0 && gain5out == 0.0) {
        copy5WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    CSAMPLE gain2 = gain2in;
    const CSAMPLE delta3 = 2.0 * (gain3out - gain3in) / iNumSamples;
    CSAMPLE gain3 = gain3in;
    const CSAMPLE delta4 = 2.0 * (gain4out - gain4in) / iNumSamples;
    CSAMPLE gain4 = gain4in;
    const CSAMPLE delta5 = 2.0 * (gain5out - gain5in) / iNumSamples;
    CSAMPLE gain5 = gain5in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1, gain2 += delta2, gain3 += delta3, gain4 += delta4, gain5 += delta5) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1 +
                       pSrc2[i + 1] * gain2 +
                       pSrc3[i + 1] * gain3 +
                       pSrc4[i + 1] * gain4 +
                       pSrc5[i + 1] * gain5;
    }
}
static inline void copy7WithGain(CSAMPLE* pDest,
                                 const CSAMPLE* pSrc0, CSAMPLE gain0,
                                 const CSAMPLE* pSrc1, CSAMPLE gain1,
                                 const CSAMPLE* pSrc2, CSAMPLE gain2,
                                 const CSAMPLE* pSrc3, CSAMPLE gain3,
                                 const CSAMPLE* pSrc4, CSAMPLE gain4,
                                 const CSAMPLE* pSrc5, CSAMPLE gain5,
                                 const CSAMPLE* pSrc6, CSAMPLE gain6,
                                 int iNumSamples) {
    if (gain0 == 0.0) {
        copy6WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy6WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, iNumSamples);
        return;
    }
    if (gain2 == 0.0) {
        copy6WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, iNumSamples);
        return;
    }
    if (gain3 == 0.0) {
        copy6WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, iNumSamples);
        return;
    }
    if (gain4 == 0.0) {
        copy6WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, iNumSamples);
        return;
    }
    if (gain5 == 0.0) {
        copy6WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, iNumSamples);
        return;
    }
    if (gain6 == 0.0) {
        copy6WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6;
    }
}
static inline void copy7WithRampingGain(CSAMPLE* pDest,
                                        const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                        const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                        const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                                        const CSAMPLE* pSrc3, CSAMPLE gain3in, CSAMPLE gain3out,
                                        const CSAMPLE* pSrc4, CSAMPLE gain4in, CSAMPLE gain4out,
                                        const CSAMPLE* pSrc5, CSAMPLE gain5in, CSAMPLE gain5out,
                                        const CSAMPLE* pSrc6, CSAMPLE gain6in, CSAMPLE gain6out,
                                        int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy6WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy6WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, iNumSamples);
        return;
    }
    if (gain2in == 0.0 && gain2out == 0.0) {
        copy6WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, iNumSamples);
        return;
    }
    if (gain3in == 0.0 && gain3out == 0.0) {
        copy6WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, iNumSamples);
        return;
    }
    if (gain4in == 0.0 && gain4out == 0.0) {
        copy6WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, iNumSamples);
        return;
    }
    if (gain5in == 0.0 && gain5out == 0.0) {
        copy6WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, iNumSamples);
        return;
    }
    if (gain6in == 0.0 && gain6out == 0.0) {
        copy6WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    CSAMPLE gain2 = gain2in;
    const CSAMPLE delta3 = 2.0 * (gain3out - gain3in) / iNumSamples;
    CSAMPLE gain3 = gain3in;
    const CSAMPLE delta4 = 2.0 * (gain4out - gain4in) / iNumSamples;
    CSAMPLE gain4 = gain4in;
    const CSAMPLE delta5 = 2.0 * (gain5out - gain5in) / iNumSamples;
    CSAMPLE gain5 = gain5in;
    const CSAMPLE delta6 = 2.0 * (gain6out - gain6in) / iNumSamples;
    CSAMPLE gain6 = gain6in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1, gain2 += delta2, gain3 += delta3, gain4 += delta4, gain5 += delta5, gain6 += delta6) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1 +
                       pSrc2[i + 1] * gain2 +
                       pSrc3[i + 1] * gain3 +
                       pSrc4[i + 1] * gain4 +
                       pSrc5[i + 1] * gain5 +
                       pSrc6[i + 1] * gain6;
    }
}
static inline void copy8WithGain(CSAMPLE* pDest,
                                 const CSAMPLE* pSrc0, CSAMPLE gain0,
                                 const CSAMPLE* pSrc1, CSAMPLE gain1,
                                 const CSAMPLE* pSrc2, CSAMPLE gain2,
                                 const CSAMPLE* pSrc3, CSAMPLE gain3,
                                 const CSAMPLE* pSrc4, CSAMPLE gain4,
                                 const CSAMPLE* pSrc5, CSAMPLE gain5,
                                 const CSAMPLE* pSrc6, CSAMPLE gain6,
                                 const CSAMPLE* pSrc7, CSAMPLE gain7,
                                 int iNumSamples) {
    if (gain0 == 0.0) {
        copy7WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy7WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, iNumSamples);
        return;
    }
    if (gain2 == 0.0) {
        copy7WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, iNumSamples);
        return;
    }
    if (gain3 == 0.0) {
        copy7WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, iNumSamples);
        return;
    }
    if (gain4 == 0.0) {
        copy7WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, iNumSamples);
        return;
    }
    if (gain5 == 0.0) {
        copy7WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, iNumSamples);
        return;
    }
    if (gain6 == 0.0) {
        copy7WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, iNumSamples);
        return;
    }
    if (gain7 == 0.0) {
        copy7WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7;
    }
}
static inline void copy8WithRampingGain(CSAMPLE* pDest,
                                        const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                        const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                        const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                                        const CSAMPLE* pSrc3, CSAMPLE gain3in, CSAMPLE gain3out,
                                        const CSAMPLE* pSrc4, CSAMPLE gain4in, CSAMPLE gain4out,
                                        const CSAMPLE* pSrc5, CSAMPLE gain5in, CSAMPLE gain5out,
                                        const CSAMPLE* pSrc6, CSAMPLE gain6in, CSAMPLE gain6out,
                                        const CSAMPLE* pSrc7, CSAMPLE gain7in, CSAMPLE gain7out,
                                        int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy7WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy7WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, iNumSamples);
        return;
    }
    if (gain2in == 0.0 && gain2out == 0.0) {
        copy7WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, iNumSamples);
        return;
    }
    if (gain3in == 0.0 && gain3out == 0.0) {
        copy7WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, iNumSamples);
        return;
    }
    if (gain4in == 0.0 && gain4out == 0.0) {
        copy7WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, iNumSamples);
        return;
    }
    if (gain5in == 0.0 && gain5out == 0.0) {
        copy7WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, iNumSamples);
        return;
    }
    if (gain6in == 0.0 && gain6out == 0.0) {
        copy7WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, iNumSamples);
        return;
    }
    if (gain7in == 0.0 && gain7out == 0.0) {
        copy7WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    CSAMPLE gain2 = gain2in;
    const CSAMPLE delta3 = 2.0 * (gain3out - gain3in) / iNumSamples;
    CSAMPLE gain3 = gain3in;
    const CSAMPLE delta4 = 2.0 * (gain4out - gain4in) / iNumSamples;
    CSAMPLE gain4 = gain4in;
    const CSAMPLE delta5 = 2.0 * (gain5out - gain5in) / iNumSamples;
    CSAMPLE gain5 = gain5in;
    const CSAMPLE delta6 = 2.0 * (gain6out - gain6in) / iNumSamples;
    CSAMPLE gain6 = gain6in;
    const CSAMPLE delta7 = 2.0 * (gain7out - gain7in) / iNumSamples;
    CSAMPLE gain7 = gain7in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1, gain2 += delta2, gain3 += delta3, gain4 += delta4, gain5 += delta5, gain6 += delta6, gain7 += delta7) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1 +
                       pSrc2[i + 1] * gain2 +
                       pSrc3[i + 1] * gain3 +
                       pSrc4[i + 1] * gain4 +
                       pSrc5[i + 1] * gain5 +
                       pSrc6[i + 1] * gain6 +
                       pSrc7[i + 1] * gain7;
    }
}
static inline void copy9WithGain(CSAMPLE* pDest,
                                 const CSAMPLE* pSrc0, CSAMPLE gain0,
                                 const CSAMPLE* pSrc1, CSAMPLE gain1,
                                 const CSAMPLE* pSrc2, CSAMPLE gain2,
                                 const CSAMPLE* pSrc3, CSAMPLE gain3,
                                 const CSAMPLE* pSrc4, CSAMPLE gain4,
                                 const CSAMPLE* pSrc5, CSAMPLE gain5,
                                 const CSAMPLE* pSrc6, CSAMPLE gain6,
                                 const CSAMPLE* pSrc7, CSAMPLE gain7,
                                 const CSAMPLE* pSrc8, CSAMPLE gain8,
                                 int iNumSamples) {
    if (gain0 == 0.0) {
        copy8WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy8WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, iNumSamples);
        return;
    }
    if (gain2 == 0.0) {
        copy8WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, iNumSamples);
        return;
    }
    if (gain3 == 0.0) {
        copy8WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, iNumSamples);
        return;
    }
    if (gain4 == 0.0) {
        copy8WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, iNumSamples);
        return;
    }
    if (gain5 == 0.0) {
        copy8WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, iNumSamples);
        return;
    }
    if (gain6 == 0.0) {
        copy8WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, iNumSamples);
        return;
    }
    if (gain7 == 0.0) {
        copy8WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, iNumSamples);
        return;
    }
    if (gain8 == 0.0) {
        copy8WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8;
    }
}
static inline void copy9WithRampingGain(CSAMPLE* pDest,
                                        const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                        const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                        const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                                        const CSAMPLE* pSrc3, CSAMPLE gain3in, CSAMPLE gain3out,
                                        const CSAMPLE* pSrc4, CSAMPLE gain4in, CSAMPLE gain4out,
                                        const CSAMPLE* pSrc5, CSAMPLE gain5in, CSAMPLE gain5out,
                                        const CSAMPLE* pSrc6, CSAMPLE gain6in, CSAMPLE gain6out,
                                        const CSAMPLE* pSrc7, CSAMPLE gain7in, CSAMPLE gain7out,
                                        const CSAMPLE* pSrc8, CSAMPLE gain8in, CSAMPLE gain8out,
                                        int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy8WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy8WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, iNumSamples);
        return;
    }
    if (gain2in == 0.0 && gain2out == 0.0) {
        copy8WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, iNumSamples);
        return;
    }
    if (gain3in == 0.0 && gain3out == 0.0) {
        copy8WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, iNumSamples);
        return;
    }
    if (gain4in == 0.0 && gain4out == 0.0) {
        copy8WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, iNumSamples);
        return;
    }
    if (gain5in == 0.0 && gain5out == 0.0) {
        copy8WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, iNumSamples);
        return;
    }
    if (gain6in == 0.0 && gain6out == 0.0) {
        copy8WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, iNumSamples);
        return;
    }
    if (gain7in == 0.0 && gain7out == 0.0) {
        copy8WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, iNumSamples);
        return;
    }
    if (gain8in == 0.0 && gain8out == 0.0) {
        copy8WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    CSAMPLE gain2 = gain2in;
    const CSAMPLE delta3 = 2.0 * (gain3out - gain3in) / iNumSamples;
    CSAMPLE gain3 = gain3in;
    const CSAMPLE delta4 = 2.0 * (gain4out - gain4in) / iNumSamples;
    CSAMPLE gain4 = gain4in;
    const CSAMPLE delta5 = 2.0 * (gain5out - gain5in) / iNumSamples;
    CSAMPLE gain5 = gain5in;
    const CSAMPLE delta6 = 2.0 * (gain6out - gain6in) / iNumSamples;
    CSAMPLE gain6 = gain6in;
    const CSAMPLE delta7 = 2.0 * (gain7out - gain7in) / iNumSamples;
    CSAMPLE gain7 = gain7in;
    const CSAMPLE delta8 = 2.0 * (gain8out - gain8in) / iNumSamples;
    CSAMPLE gain8 = gain8in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1, gain2 += delta2, gain3 += delta3, gain4 += delta4, gain5 += delta5, gain6 += delta6, gain7 += delta7, gain8 += delta8) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1 +
                       pSrc2[i + 1] * gain2 +
                       pSrc3[i + 1] * gain3 +
                       pSrc4[i + 1] * gain4 +
                       pSrc5[i + 1] * gain5 +
                       pSrc6[i + 1] * gain6 +
                       pSrc7[i + 1] * gain7 +
                       pSrc8[i + 1] * gain8;
    }
}
static inline void copy10WithGain(CSAMPLE* pDest,
                                  const CSAMPLE* pSrc0, CSAMPLE gain0,
                                  const CSAMPLE* pSrc1, CSAMPLE gain1,
                                  const CSAMPLE* pSrc2, CSAMPLE gain2,
                                  const CSAMPLE* pSrc3, CSAMPLE gain3,
                                  const CSAMPLE* pSrc4, CSAMPLE gain4,
                                  const CSAMPLE* pSrc5, CSAMPLE gain5,
                                  const CSAMPLE* pSrc6, CSAMPLE gain6,
                                  const CSAMPLE* pSrc7, CSAMPLE gain7,
                                  const CSAMPLE* pSrc8, CSAMPLE gain8,
                                  const CSAMPLE* pSrc9, CSAMPLE gain9,
                                  int iNumSamples) {
    if (gain0 == 0.0) {
        copy9WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy9WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, iNumSamples);
        return;
    }
    if (gain2 == 0.0) {
        copy9WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, iNumSamples);
        return;
    }
    if (gain3 == 0.0) {
        copy9WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, iNumSamples);
        return;
    }
    if (gain4 == 0.0) {
        copy9WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, iNumSamples);
        return;
    }
    if (gain5 == 0.0) {
        copy9WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, iNumSamples);
        return;
    }
    if (gain6 == 0.0) {
        copy9WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, iNumSamples);
        return;
    }
    if (gain7 == 0.0) {
        copy9WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, iNumSamples);
        return;
    }
    if (gain8 == 0.0) {
        copy9WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, iNumSamples);
        return;
    }
    if (gain9 == 0.0) {
        copy9WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9;
    }
}
static inline void copy10WithRampingGain(CSAMPLE* pDest,
                                         const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                         const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                         const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                                         const CSAMPLE* pSrc3, CSAMPLE gain3in, CSAMPLE gain3out,
                                         const CSAMPLE* pSrc4, CSAMPLE gain4in, CSAMPLE gain4out,
                                         const CSAMPLE* pSrc5, CSAMPLE gain5in, CSAMPLE gain5out,
                                         const CSAMPLE* pSrc6, CSAMPLE gain6in, CSAMPLE gain6out,
                                         const CSAMPLE* pSrc7, CSAMPLE gain7in, CSAMPLE gain7out,
                                         const CSAMPLE* pSrc8, CSAMPLE gain8in, CSAMPLE gain8out,
                                         const CSAMPLE* pSrc9, CSAMPLE gain9in, CSAMPLE gain9out,
                                         int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy9WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy9WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, iNumSamples);
        return;
    }
    if (gain2in == 0.0 && gain2out == 0.0) {
        copy9WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, iNumSamples);
        return;
    }
    if (gain3in == 0.0 && gain3out == 0.0) {
        copy9WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, iNumSamples);
        return;
    }
    if (gain4in == 0.0 && gain4out == 0.0) {
        copy9WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, iNumSamples);
        return;
    }
    if (gain5in == 0.0 && gain5out == 0.0) {
        copy9WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, iNumSamples);
        return;
    }
    if (gain6in == 0.0 && gain6out == 0.0) {
        copy9WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, iNumSamples);
        return;
    }
    if (gain7in == 0.0 && gain7out == 0.0) {
        copy9WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, iNumSamples);
        return;
    }
    if (gain8in == 0.0 && gain8out == 0.0) {
        copy9WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, iNumSamples);
        return;
    }
    if (gain9in == 0.0 && gain9out == 0.0) {
        copy9WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    CSAMPLE gain2 = gain2in;
    const CSAMPLE delta3 = 2.0 * (gain3out - gain3in) / iNumSamples;
    CSAMPLE gain3 = gain3in;
    const CSAMPLE delta4 = 2.0 * (gain4out - gain4in) / iNumSamples;
    CSAMPLE gain4 = gain4in;
    const CSAMPLE delta5 = 2.0 * (gain5out - gain5in) / iNumSamples;
    CSAMPLE gain5 = gain5in;
    const CSAMPLE delta6 = 2.0 * (gain6out - gain6in) / iNumSamples;
    CSAMPLE gain6 = gain6in;
    const CSAMPLE delta7 = 2.0 * (gain7out - gain7in) / iNumSamples;
    CSAMPLE gain7 = gain7in;
    const CSAMPLE delta8 = 2.0 * (gain8out - gain8in) / iNumSamples;
    CSAMPLE gain8 = gain8in;
    const CSAMPLE delta9 = 2.0 * (gain9out - gain9in) / iNumSamples;
    CSAMPLE gain9 = gain9in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1, gain2 += delta2, gain3 += delta3, gain4 += delta4, gain5 += delta5, gain6 += delta6, gain7 += delta7, gain8 += delta8, gain9 += delta9) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1 +
                       pSrc2[i + 1] * gain2 +
                       pSrc3[i + 1] * gain3 +
                       pSrc4[i + 1] * gain4 +
                       pSrc5[i + 1] * gain5 +
                       pSrc6[i + 1] * gain6 +
                       pSrc7[i + 1] * gain7 +
                       pSrc8[i + 1] * gain8 +
                       pSrc9[i + 1] * gain9;
    }
}
static inline void copy11WithGain(CSAMPLE* pDest,
                                  const CSAMPLE* pSrc0, CSAMPLE gain0,
                                  const CSAMPLE* pSrc1, CSAMPLE gain1,
                                  const CSAMPLE* pSrc2, CSAMPLE gain2,
                                  const CSAMPLE* pSrc3, CSAMPLE gain3,
                                  const CSAMPLE* pSrc4, CSAMPLE gain4,
                                  const CSAMPLE* pSrc5, CSAMPLE gain5,
                                  const CSAMPLE* pSrc6, CSAMPLE gain6,
                                  const CSAMPLE* pSrc7, CSAMPLE gain7,
                                  const CSAMPLE* pSrc8, CSAMPLE gain8,
                                  const CSAMPLE* pSrc9, CSAMPLE gain9,
                                  const CSAMPLE* pSrc10, CSAMPLE gain10,
                                  int iNumSamples) {
    if (gain0 == 0.0) {
        copy10WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy10WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, iNumSamples);
        return;
    }
    if (gain2 == 0.0) {
        copy10WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, iNumSamples);
        return;
    }
    if (gain3 == 0.0) {
        copy10WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, iNumSamples);
        return;
    }
    if (gain4 == 0.0) {
        copy10WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, iNumSamples);
        return;
    }
    if (gain5 == 0.0) {
        copy10WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, iNumSamples);
        return;
    }
    if (gain6 == 0.0) {
        copy10WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, iNumSamples);
        return;
    }
    if (gain7 == 0.0) {
        copy10WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, iNumSamples);
        return;
    }
    if (gain8 == 0.0) {
        copy10WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, iNumSamples);
        return;
    }
    if (gain9 == 0.0) {
        copy10WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, iNumSamples);
        return;
    }
    if (gain10 == 0.0) {
        copy10WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10;
    }
}
static inline void copy11WithRampingGain(CSAMPLE* pDest,
                                         const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                         const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                         const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                                         const CSAMPLE* pSrc3, CSAMPLE gain3in, CSAMPLE gain3out,
                                         const CSAMPLE* pSrc4, CSAMPLE gain4in, CSAMPLE gain4out,
                                         const CSAMPLE* pSrc5, CSAMPLE gain5in, CSAMPLE gain5out,
                                         const CSAMPLE* pSrc6, CSAMPLE gain6in, CSAMPLE gain6out,
                                         const CSAMPLE* pSrc7, CSAMPLE gain7in, CSAMPLE gain7out,
                                         const CSAMPLE* pSrc8, CSAMPLE gain8in, CSAMPLE gain8out,
                                         const CSAMPLE* pSrc9, CSAMPLE gain9in, CSAMPLE gain9out,
                                         const CSAMPLE* pSrc10, CSAMPLE gain10in, CSAMPLE gain10out,
                                         int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy10WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy10WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, iNumSamples);
        return;
    }
    if (gain2in == 0.0 && gain2out == 0.0) {
        copy10WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, iNumSamples);
        return;
    }
    if (gain3in == 0.0 && gain3out == 0.0) {
        copy10WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, iNumSamples);
        return;
    }
    if (gain4in == 0.0 && gain4out == 0.0) {
        copy10WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, iNumSamples);
        return;
    }
    if (gain5in == 0.0 && gain5out == 0.0) {
        copy10WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, iNumSamples);
        return;
    }
    if (gain6in == 0.0 && gain6out == 0.0) {
        copy10WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, iNumSamples);
        return;
    }
    if (gain7in == 0.0 && gain7out == 0.0) {
        copy10WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, iNumSamples);
        return;
    }
    if (gain8in == 0.0 && gain8out == 0.0) {
        copy10WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, iNumSamples);
        return;
    }
    if (gain9in == 0.0 && gain9out == 0.0) {
        copy10WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, iNumSamples);
        return;
    }
    if (gain10in == 0.0 && gain10out == 0.0) {
        copy10WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    CSAMPLE gain2 = gain2in;
    const CSAMPLE delta3 = 2.0 * (gain3out - gain3in) / iNumSamples;
    CSAMPLE gain3 = gain3in;
    const CSAMPLE delta4 = 2.0 * (gain4out - gain4in) / iNumSamples;
    CSAMPLE gain4 = gain4in;
    const CSAMPLE delta5 = 2.0 * (gain5out - gain5in) / iNumSamples;
    CSAMPLE gain5 = gain5in;
    const CSAMPLE delta6 = 2.0 * (gain6out - gain6in) / iNumSamples;
    CSAMPLE gain6 = gain6in;
    const CSAMPLE delta7 = 2.0 * (gain7out - gain7in) / iNumSamples;
    CSAMPLE gain7 = gain7in;
    const CSAMPLE delta8 = 2.0 * (gain8out - gain8in) / iNumSamples;
    CSAMPLE gain8 = gain8in;
    const CSAMPLE delta9 = 2.0 * (gain9out - gain9in) / iNumSamples;
    CSAMPLE gain9 = gain9in;
    const CSAMPLE delta10 = 2.0 * (gain10out - gain10in) / iNumSamples;
    CSAMPLE gain10 = gain10in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1, gain2 += delta2, gain3 += delta3, gain4 += delta4, gain5 += delta5, gain6 += delta6, gain7 += delta7, gain8 += delta8, gain9 += delta9, gain10 += delta10) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1 +
                       pSrc2[i + 1] * gain2 +
                       pSrc3[i + 1] * gain3 +
                       pSrc4[i + 1] * gain4 +
                       pSrc5[i + 1] * gain5 +
                       pSrc6[i + 1] * gain6 +
                       pSrc7[i + 1] * gain7 +
                       pSrc8[i + 1] * gain8 +
                       pSrc9[i + 1] * gain9 +
                       pSrc10[i + 1] * gain10;
    }
}
static inline void copy12WithGain(CSAMPLE* pDest,
                                  const CSAMPLE* pSrc0, CSAMPLE gain0,
                                  const CSAMPLE* pSrc1, CSAMPLE gain1,
                                  const CSAMPLE* pSrc2, CSAMPLE gain2,
                                  const CSAMPLE* pSrc3, CSAMPLE gain3,
                                  const CSAMPLE* pSrc4, CSAMPLE gain4,
                                  const CSAMPLE* pSrc5, CSAMPLE gain5,
                                  const CSAMPLE* pSrc6, CSAMPLE gain6,
                                  const CSAMPLE* pSrc7, CSAMPLE gain7,
                                  const CSAMPLE* pSrc8, CSAMPLE gain8,
                                  const CSAMPLE* pSrc9, CSAMPLE gain9,
                                  const CSAMPLE* pSrc10, CSAMPLE gain10,
                                  const CSAMPLE* pSrc11, CSAMPLE gain11,
                                  int iNumSamples) {
    if (gain0 == 0.0) {
        copy11WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy11WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, iNumSamples);
        return;
    }
    if (gain2 == 0.0) {
        copy11WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, iNumSamples);
        return;
    }
    if (gain3 == 0.0) {
        copy11WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, iNumSamples);
        return;
    }
    if (gain4 == 0.0) {
        copy11WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, iNumSamples);
        return;
    }
    if (gain5 == 0.0) {
        copy11WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, iNumSamples);
        return;
    }
    if (gain6 == 0.0) {
        copy11WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, iNumSamples);
        return;
    }
    if (gain7 == 0.0) {
        copy11WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, iNumSamples);
        return;
    }
    if (gain8 == 0.0) {
        copy11WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, iNumSamples);
        return;
    }
    if (gain9 == 0.0) {
        copy11WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, iNumSamples);
        return;
    }
    if (gain10 == 0.0) {
        copy11WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, iNumSamples);
        return;
    }
    if (gain11 == 0.0) {
        copy11WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11;
    }
}
static inline void copy12WithRampingGain(CSAMPLE* pDest,
                                         const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                         const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                         const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                                         const CSAMPLE* pSrc3, CSAMPLE gain3in, CSAMPLE gain3out,
                                         const CSAMPLE* pSrc4, CSAMPLE gain4in, CSAMPLE gain4out,
                                         const CSAMPLE* pSrc5, CSAMPLE gain5in, CSAMPLE gain5out,
                                         const CSAMPLE* pSrc6, CSAMPLE gain6in, CSAMPLE gain6out,
                                         const CSAMPLE* pSrc7, CSAMPLE gain7in, CSAMPLE gain7out,
                                         const CSAMPLE* pSrc8, CSAMPLE gain8in, CSAMPLE gain8out,
                                         const CSAMPLE* pSrc9, CSAMPLE gain9in, CSAMPLE gain9out,
                                         const CSAMPLE* pSrc10, CSAMPLE gain10in, CSAMPLE gain10out,
                                         const CSAMPLE* pSrc11, CSAMPLE gain11in, CSAMPLE gain11out,
                                         int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy11WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy11WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, iNumSamples);
        return;
    }
    if (gain2in == 0.0 && gain2out == 0.0) {
        copy11WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, iNumSamples);
        return;
    }
    if (gain3in == 0.0 && gain3out == 0.0) {
        copy11WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, iNumSamples);
        return;
    }
    if (gain4in == 0.0 && gain4out == 0.0) {
        copy11WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, iNumSamples);
        return;
    }
    if (gain5in == 0.0 && gain5out == 0.0) {
        copy11WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, iNumSamples);
        return;
    }
    if (gain6in == 0.0 && gain6out == 0.0) {
        copy11WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, iNumSamples);
        return;
    }
    if (gain7in == 0.0 && gain7out == 0.0) {
        copy11WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, iNumSamples);
        return;
    }
    if (gain8in == 0.0 && gain8out == 0.0) {
        copy11WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, iNumSamples);
        return;
    }
    if (gain9in == 0.0 && gain9out == 0.0) {
        copy11WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, iNumSamples);
        return;
    }
    if (gain10in == 0.0 && gain10out == 0.0) {
        copy11WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, iNumSamples);
        return;
    }
    if (gain11in == 0.0 && gain11out == 0.0) {
        copy11WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    CSAMPLE gain2 = gain2in;
    const CSAMPLE delta3 = 2.0 * (gain3out - gain3in) / iNumSamples;
    CSAMPLE gain3 = gain3in;
    const CSAMPLE delta4 = 2.0 * (gain4out - gain4in) / iNumSamples;
    CSAMPLE gain4 = gain4in;
    const CSAMPLE delta5 = 2.0 * (gain5out - gain5in) / iNumSamples;
    CSAMPLE gain5 = gain5in;
    const CSAMPLE delta6 = 2.0 * (gain6out - gain6in) / iNumSamples;
    CSAMPLE gain6 = gain6in;
    const CSAMPLE delta7 = 2.0 * (gain7out - gain7in) / iNumSamples;
    CSAMPLE gain7 = gain7in;
    const CSAMPLE delta8 = 2.0 * (gain8out - gain8in) / iNumSamples;
    CSAMPLE gain8 = gain8in;
    const CSAMPLE delta9 = 2.0 * (gain9out - gain9in) / iNumSamples;
    CSAMPLE gain9 = gain9in;
    const CSAMPLE delta10 = 2.0 * (gain10out - gain10in) / iNumSamples;
    CSAMPLE gain10 = gain10in;
    const CSAMPLE delta11 = 2.0 * (gain11out - gain11in) / iNumSamples;
    CSAMPLE gain11 = gain11in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1, gain2 += delta2, gain3 += delta3, gain4 += delta4, gain5 += delta5, gain6 += delta6, gain7 += delta7, gain8 += delta8, gain9 += delta9, gain10 += delta10, gain11 += delta11) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1 +
                       pSrc2[i + 1] * gain2 +
                       pSrc3[i + 1] * gain3 +
                       pSrc4[i + 1] * gain4 +
                       pSrc5[i + 1] * gain5 +
                       pSrc6[i + 1] * gain6 +
                       pSrc7[i + 1] * gain7 +
                       pSrc8[i + 1] * gain8 +
                       pSrc9[i + 1] * gain9 +
                       pSrc10[i + 1] * gain10 +
                       pSrc11[i + 1] * gain11;
    }
}
static inline void copy13WithGain(CSAMPLE* pDest,
                                  const CSAMPLE* pSrc0, CSAMPLE gain0,
                                  const CSAMPLE* pSrc1, CSAMPLE gain1,
                                  const CSAMPLE* pSrc2, CSAMPLE gain2,
                                  const CSAMPLE* pSrc3, CSAMPLE gain3,
                                  const CSAMPLE* pSrc4, CSAMPLE gain4,
                                  const CSAMPLE* pSrc5, CSAMPLE gain5,
                                  const CSAMPLE* pSrc6, CSAMPLE gain6,
                                  const CSAMPLE* pSrc7, CSAMPLE gain7,
                                  const CSAMPLE* pSrc8, CSAMPLE gain8,
                                  const CSAMPLE* pSrc9, CSAMPLE gain9,
                                  const CSAMPLE* pSrc10, CSAMPLE gain10,
                                  const CSAMPLE* pSrc11, CSAMPLE gain11,
                                  const CSAMPLE* pSrc12, CSAMPLE gain12,
                                  int iNumSamples) {
    if (gain0 == 0.0) {
        copy12WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy12WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, iNumSamples);
        return;
    }
    if (gain2 == 0.0) {
        copy12WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, iNumSamples);
        return;
    }
    if (gain3 == 0.0) {
        copy12WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, iNumSamples);
        return;
    }
    if (gain4 == 0.0) {
        copy12WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, iNumSamples);
        return;
    }
    if (gain5 == 0.0) {
        copy12WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, iNumSamples);
        return;
    }
    if (gain6 == 0.0) {
        copy12WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, iNumSamples);
        return;
    }
    if (gain7 == 0.0) {
        copy12WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, iNumSamples);
        return;
    }
    if (gain8 == 0.0) {
        copy12WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, iNumSamples);
        return;
    }
    if (gain9 == 0.0) {
        copy12WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, iNumSamples);
        return;
    }
    if (gain10 == 0.0) {
        copy12WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, iNumSamples);
        return;
    }
    if (gain11 == 0.0) {
        copy12WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, iNumSamples);
        return;
    }
    if (gain12 == 0.0) {
        copy12WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12;
    }
}
static inline void copy13WithRampingGain(CSAMPLE* pDest,
                                         const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                         const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                         const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                                         const CSAMPLE* pSrc3, CSAMPLE gain3in, CSAMPLE gain3out,
                                         const CSAMPLE* pSrc4, CSAMPLE gain4in, CSAMPLE gain4out,
                                         const CSAMPLE* pSrc5, CSAMPLE gain5in, CSAMPLE gain5out,
                                         const CSAMPLE* pSrc6, CSAMPLE gain6in, CSAMPLE gain6out,
                                         const CSAMPLE* pSrc7, CSAMPLE gain7in, CSAMPLE gain7out,
                                         const CSAMPLE* pSrc8, CSAMPLE gain8in, CSAMPLE gain8out,
                                         const CSAMPLE* pSrc9, CSAMPLE gain9in, CSAMPLE gain9out,
                                         const CSAMPLE* pSrc10, CSAMPLE gain10in, CSAMPLE gain10out,
                                         const CSAMPLE* pSrc11, CSAMPLE gain11in, CSAMPLE gain11out,
                                         const CSAMPLE* pSrc12, CSAMPLE gain12in, CSAMPLE gain12out,
                                         int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy12WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy12WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, iNumSamples);
        return;
    }
    if (gain2in == 0.0 && gain2out == 0.0) {
        copy12WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, iNumSamples);
        return;
    }
    if (gain3in == 0.0 && gain3out == 0.0) {
        copy12WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, iNumSamples);
        return;
    }
    if (gain4in == 0.0 && gain4out == 0.0) {
        copy12WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, iNumSamples);
        return;
    }
    if (gain5in == 0.0 && gain5out == 0.0) {
        copy12WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, iNumSamples);
        return;
    }
    if (gain6in == 0.0 && gain6out == 0.0) {
        copy12WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, iNumSamples);
        return;
    }
    if (gain7in == 0.0 && gain7out == 0.0) {
        copy12WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, iNumSamples);
        return;
    }
    if (gain8in == 0.0 && gain8out == 0.0) {
        copy12WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, iNumSamples);
        return;
    }
    if (gain9in == 0.0 && gain9out == 0.0) {
        copy12WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, iNumSamples);
        return;
    }
    if (gain10in == 0.0 && gain10out == 0.0) {
        copy12WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, iNumSamples);
        return;
    }
    if (gain11in == 0.0 && gain11out == 0.0) {
        copy12WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, iNumSamples);
        return;
    }
    if (gain12in == 0.0 && gain12out == 0.0) {
        copy12WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    CSAMPLE gain2 = gain2in;
    const CSAMPLE delta3 = 2.0 * (gain3out - gain3in) / iNumSamples;
    CSAMPLE gain3 = gain3in;
    const CSAMPLE delta4 = 2.0 * (gain4out - gain4in) / iNumSamples;
    CSAMPLE gain4 = gain4in;
    const CSAMPLE delta5 = 2.0 * (gain5out - gain5in) / iNumSamples;
    CSAMPLE gain5 = gain5in;
    const CSAMPLE delta6 = 2.0 * (gain6out - gain6in) / iNumSamples;
    CSAMPLE gain6 = gain6in;
    const CSAMPLE delta7 = 2.0 * (gain7out - gain7in) / iNumSamples;
    CSAMPLE gain7 = gain7in;
    const CSAMPLE delta8 = 2.0 * (gain8out - gain8in) / iNumSamples;
    CSAMPLE gain8 = gain8in;
    const CSAMPLE delta9 = 2.0 * (gain9out - gain9in) / iNumSamples;
    CSAMPLE gain9 = gain9in;
    const CSAMPLE delta10 = 2.0 * (gain10out - gain10in) / iNumSamples;
    CSAMPLE gain10 = gain10in;
    const CSAMPLE delta11 = 2.0 * (gain11out - gain11in) / iNumSamples;
    CSAMPLE gain11 = gain11in;
    const CSAMPLE delta12 = 2.0 * (gain12out - gain12in) / iNumSamples;
    CSAMPLE gain12 = gain12in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1, gain2 += delta2, gain3 += delta3, gain4 += delta4, gain5 += delta5, gain6 += delta6, gain7 += delta7, gain8 += delta8, gain9 += delta9, gain10 += delta10, gain11 += delta11, gain12 += delta12) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1 +
                       pSrc2[i + 1] * gain2 +
                       pSrc3[i + 1] * gain3 +
                       pSrc4[i + 1] * gain4 +
                       pSrc5[i + 1] * gain5 +
                       pSrc6[i + 1] * gain6 +
                       pSrc7[i + 1] * gain7 +
                       pSrc8[i + 1] * gain8 +
                       pSrc9[i + 1] * gain9 +
                       pSrc10[i + 1] * gain10 +
                       pSrc11[i + 1] * gain11 +
                       pSrc12[i + 1] * gain12;
    }
}
static inline void copy14WithGain(CSAMPLE* pDest,
                                  const CSAMPLE* pSrc0, CSAMPLE gain0,
                                  const CSAMPLE* pSrc1, CSAMPLE gain1,
                                  const CSAMPLE* pSrc2, CSAMPLE gain2,
                                  const CSAMPLE* pSrc3, CSAMPLE gain3,
                                  const CSAMPLE* pSrc4, CSAMPLE gain4,
                                  const CSAMPLE* pSrc5, CSAMPLE gain5,
                                  const CSAMPLE* pSrc6, CSAMPLE gain6,
                                  const CSAMPLE* pSrc7, CSAMPLE gain7,
                                  const CSAMPLE* pSrc8, CSAMPLE gain8,
                                  const CSAMPLE* pSrc9, CSAMPLE gain9,
                                  const CSAMPLE* pSrc10, CSAMPLE gain10,
                                  const CSAMPLE* pSrc11, CSAMPLE gain11,
                                  const CSAMPLE* pSrc12, CSAMPLE gain12,
                                  const CSAMPLE* pSrc13, CSAMPLE gain13,
                                  int iNumSamples) {
    if (gain0 == 0.0) {
        copy13WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy13WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, iNumSamples);
        return;
    }
    if (gain2 == 0.0) {
        copy13WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, iNumSamples);
        return;
    }
    if (gain3 == 0.0) {
        copy13WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, iNumSamples);
        return;
    }
    if (gain4 == 0.0) {
        copy13WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, iNumSamples);
        return;
    }
    if (gain5 == 0.0) {
        copy13WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, iNumSamples);
        return;
    }
    if (gain6 == 0.0) {
        copy13WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, iNumSamples);
        return;
    }
    if (gain7 == 0.0) {
        copy13WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, iNumSamples);
        return;
    }
    if (gain8 == 0.0) {
        copy13WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, iNumSamples);
        return;
    }
    if (gain9 == 0.0) {
        copy13WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, iNumSamples);
        return;
    }
    if (gain10 == 0.0) {
        copy13WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, iNumSamples);
        return;
    }
    if (gain11 == 0.0) {
        copy13WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, iNumSamples);
        return;
    }
    if (gain12 == 0.0) {
        copy13WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, iNumSamples);
        return;
    }
    if (gain13 == 0.0) {
        copy13WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13;
    }
}
static inline void copy14WithRampingGain(CSAMPLE* pDest,
                                         const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                         const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                         const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                                         const CSAMPLE* pSrc3, CSAMPLE gain3in, CSAMPLE gain3out,
                                         const CSAMPLE* pSrc4, CSAMPLE gain4in, CSAMPLE gain4out,
                                         const CSAMPLE* pSrc5, CSAMPLE gain5in, CSAMPLE gain5out,
                                         const CSAMPLE* pSrc6, CSAMPLE gain6in, CSAMPLE gain6out,
                                         const CSAMPLE* pSrc7, CSAMPLE gain7in, CSAMPLE gain7out,
                                         const CSAMPLE* pSrc8, CSAMPLE gain8in, CSAMPLE gain8out,
                                         const CSAMPLE* pSrc9, CSAMPLE gain9in, CSAMPLE gain9out,
                                         const CSAMPLE* pSrc10, CSAMPLE gain10in, CSAMPLE gain10out,
                                         const CSAMPLE* pSrc11, CSAMPLE gain11in, CSAMPLE gain11out,
                                         const CSAMPLE* pSrc12, CSAMPLE gain12in, CSAMPLE gain12out,
                                         const CSAMPLE* pSrc13, CSAMPLE gain13in, CSAMPLE gain13out,
                                         int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy13WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy13WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, iNumSamples);
        return;
    }
    if (gain2in == 0.0 && gain2out == 0.0) {
        copy13WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, iNumSamples);
        return;
    }
    if (gain3in == 0.0 && gain3out == 0.0) {
        copy13WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, iNumSamples);
        return;
    }
    if (gain4in == 0.0 && gain4out == 0.0) {
        copy13WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, iNumSamples);
        return;
    }
    if (gain5in == 0.0 && gain5out == 0.0) {
        copy13WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, iNumSamples);
        return;
    }
    if (gain6in == 0.0 && gain6out == 0.0) {
        copy13WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, iNumSamples);
        return;
    }
    if (gain7in == 0.0 && gain7out == 0.0) {
        copy13WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, iNumSamples);
        return;
    }
    if (gain8in == 0.0 && gain8out == 0.0) {
        copy13WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, iNumSamples);
        return;
    }
    if (gain9in == 0.0 && gain9out == 0.0) {
        copy13WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, iNumSamples);
        return;
    }
    if (gain10in == 0.0 && gain10out == 0.0) {
        copy13WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, iNumSamples);
        return;
    }
    if (gain11in == 0.0 && gain11out == 0.0) {
        copy13WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, iNumSamples);
        return;
    }
    if (gain12in == 0.0 && gain12out == 0.0) {
        copy13WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, iNumSamples);
        return;
    }
    if (gain13in == 0.0 && gain13out == 0.0) {
        copy13WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    CSAMPLE gain2 = gain2in;
    const CSAMPLE delta3 = 2.0 * (gain3out - gain3in) / iNumSamples;
    CSAMPLE gain3 = gain3in;
    const CSAMPLE delta4 = 2.0 * (gain4out - gain4in) / iNumSamples;
    CSAMPLE gain4 = gain4in;
    const CSAMPLE delta5 = 2.0 * (gain5out - gain5in) / iNumSamples;
    CSAMPLE gain5 = gain5in;
    const CSAMPLE delta6 = 2.0 * (gain6out - gain6in) / iNumSamples;
    CSAMPLE gain6 = gain6in;
    const CSAMPLE delta7 = 2.0 * (gain7out - gain7in) / iNumSamples;
    CSAMPLE gain7 = gain7in;
    const CSAMPLE delta8 = 2.0 * (gain8out - gain8in) / iNumSamples;
    CSAMPLE gain8 = gain8in;
    const CSAMPLE delta9 = 2.0 * (gain9out - gain9in) / iNumSamples;
    CSAMPLE gain9 = gain9in;
    const CSAMPLE delta10 = 2.0 * (gain10out - gain10in) / iNumSamples;
    CSAMPLE gain10 = gain10in;
    const CSAMPLE delta11 = 2.0 * (gain11out - gain11in) / iNumSamples;
    CSAMPLE gain11 = gain11in;
    const CSAMPLE delta12 = 2.0 * (gain12out - gain12in) / iNumSamples;
    CSAMPLE gain12 = gain12in;
    const CSAMPLE delta13 = 2.0 * (gain13out - gain13in) / iNumSamples;
    CSAMPLE gain13 = gain13in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1, gain2 += delta2, gain3 += delta3, gain4 += delta4, gain5 += delta5, gain6 += delta6, gain7 += delta7, gain8 += delta8, gain9 += delta9, gain10 += delta10, gain11 += delta11, gain12 += delta12, gain13 += delta13) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1 +
                       pSrc2[i + 1] * gain2 +
                       pSrc3[i + 1] * gain3 +
                       pSrc4[i + 1] * gain4 +
                       pSrc5[i + 1] * gain5 +
                       pSrc6[i + 1] * gain6 +
                       pSrc7[i + 1] * gain7 +
                       pSrc8[i + 1] * gain8 +
                       pSrc9[i + 1] * gain9 +
                       pSrc10[i + 1] * gain10 +
                       pSrc11[i + 1] * gain11 +
                       pSrc12[i + 1] * gain12 +
                       pSrc13[i + 1] * gain13;
    }
}
static inline void copy15WithGain(CSAMPLE* pDest,
                                  const CSAMPLE* pSrc0, CSAMPLE gain0,
                                  const CSAMPLE* pSrc1, CSAMPLE gain1,
                                  const CSAMPLE* pSrc2, CSAMPLE gain2,
                                  const CSAMPLE* pSrc3, CSAMPLE gain3,
                                  const CSAMPLE* pSrc4, CSAMPLE gain4,
                                  const CSAMPLE* pSrc5, CSAMPLE gain5,
                                  const CSAMPLE* pSrc6, CSAMPLE gain6,
                                  const CSAMPLE* pSrc7, CSAMPLE gain7,
                                  const CSAMPLE* pSrc8, CSAMPLE gain8,
                                  const CSAMPLE* pSrc9, CSAMPLE gain9,
                                  const CSAMPLE* pSrc10, CSAMPLE gain10,
                                  const CSAMPLE* pSrc11, CSAMPLE gain11,
                                  const CSAMPLE* pSrc12, CSAMPLE gain12,
                                  const CSAMPLE* pSrc13, CSAMPLE gain13,
                                  const CSAMPLE* pSrc14, CSAMPLE gain14,
                                  int iNumSamples) {
    if (gain0 == 0.0) {
        copy14WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy14WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, iNumSamples);
        return;
    }
    if (gain2 == 0.0) {
        copy14WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, iNumSamples);
        return;
    }
    if (gain3 == 0.0) {
        copy14WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, iNumSamples);
        return;
    }
    if (gain4 == 0.0) {
        copy14WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, iNumSamples);
        return;
    }
    if (gain5 == 0.0) {
        copy14WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, iNumSamples);
        return;
    }
    if (gain6 == 0.0) {
        copy14WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, iNumSamples);
        return;
    }
    if (gain7 == 0.0) {
        copy14WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, iNumSamples);
        return;
    }
    if (gain8 == 0.0) {
        copy14WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, iNumSamples);
        return;
    }
    if (gain9 == 0.0) {
        copy14WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, iNumSamples);
        return;
    }
    if (gain10 == 0.0) {
        copy14WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, iNumSamples);
        return;
    }
    if (gain11 == 0.0) {
        copy14WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, iNumSamples);
        return;
    }
    if (gain12 == 0.0) {
        copy14WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, iNumSamples);
        return;
    }
    if (gain13 == 0.0) {
        copy14WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, iNumSamples);
        return;
    }
    if (gain14 == 0.0) {
        copy14WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14;
    }
}
static inline void copy15WithRampingGain(CSAMPLE* pDest,
                                         const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                         const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                         const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                                         const CSAMPLE* pSrc3, CSAMPLE gain3in, CSAMPLE gain3out,
                                         const CSAMPLE* pSrc4, CSAMPLE gain4in, CSAMPLE gain4out,
                                         const CSAMPLE* pSrc5, CSAMPLE gain5in, CSAMPLE gain5out,
                                         const CSAMPLE* pSrc6, CSAMPLE gain6in, CSAMPLE gain6out,
                                         const CSAMPLE* pSrc7, CSAMPLE gain7in, CSAMPLE gain7out,
                                         const CSAMPLE* pSrc8, CSAMPLE gain8in, CSAMPLE gain8out,
                                         const CSAMPLE* pSrc9, CSAMPLE gain9in, CSAMPLE gain9out,
                                         const CSAMPLE* pSrc10, CSAMPLE gain10in, CSAMPLE gain10out,
                                         const CSAMPLE* pSrc11, CSAMPLE gain11in, CSAMPLE gain11out,
                                         const CSAMPLE* pSrc12, CSAMPLE gain12in, CSAMPLE gain12out,
                                         const CSAMPLE* pSrc13, CSAMPLE gain13in, CSAMPLE gain13out,
                                         const CSAMPLE* pSrc14, CSAMPLE gain14in, CSAMPLE gain14out,
                                         int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy14WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy14WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, iNumSamples);
        return;
    }
    if (gain2in == 0.0 && gain2out == 0.0) {
        copy14WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, iNumSamples);
        return;
    }
    if (gain3in == 0.0 && gain3out == 0.0) {
        copy14WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, iNumSamples);
        return;
    }
    if (gain4in == 0.0 && gain4out == 0.0) {
        copy14WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, iNumSamples);
        return;
    }
    if (gain5in == 0.0 && gain5out == 0.0) {
        copy14WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, iNumSamples);
        return;
    }
    if (gain6in == 0.0 && gain6out == 0.0) {
        copy14WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, iNumSamples);
        return;
    }
    if (gain7in == 0.0 && gain7out == 0.0) {
        copy14WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, iNumSamples);
        return;
    }
    if (gain8in == 0.0 && gain8out == 0.0) {
        copy14WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, iNumSamples);
        return;
    }
    if (gain9in == 0.0 && gain9out == 0.0) {
        copy14WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, iNumSamples);
        return;
    }
    if (gain10in == 0.0 && gain10out == 0.0) {
        copy14WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, iNumSamples);
        return;
    }
    if (gain11in == 0.0 && gain11out == 0.0) {
        copy14WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, iNumSamples);
        return;
    }
    if (gain12in == 0.0 && gain12out == 0.0) {
        copy14WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, iNumSamples);
        return;
    }
    if (gain13in == 0.0 && gain13out == 0.0) {
        copy14WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, iNumSamples);
        return;
    }
    if (gain14in == 0.0 && gain14out == 0.0) {
        copy14WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    CSAMPLE gain2 = gain2in;
    const CSAMPLE delta3 = 2.0 * (gain3out - gain3in) / iNumSamples;
    CSAMPLE gain3 = gain3in;
    const CSAMPLE delta4 = 2.0 * (gain4out - gain4in) / iNumSamples;
    CSAMPLE gain4 = gain4in;
    const CSAMPLE delta5 = 2.0 * (gain5out - gain5in) / iNumSamples;
    CSAMPLE gain5 = gain5in;
    const CSAMPLE delta6 = 2.0 * (gain6out - gain6in) / iNumSamples;
    CSAMPLE gain6 = gain6in;
    const CSAMPLE delta7 = 2.0 * (gain7out - gain7in) / iNumSamples;
    CSAMPLE gain7 = gain7in;
    const CSAMPLE delta8 = 2.0 * (gain8out - gain8in) / iNumSamples;
    CSAMPLE gain8 = gain8in;
    const CSAMPLE delta9 = 2.0 * (gain9out - gain9in) / iNumSamples;
    CSAMPLE gain9 = gain9in;
    const CSAMPLE delta10 = 2.0 * (gain10out - gain10in) / iNumSamples;
    CSAMPLE gain10 = gain10in;
    const CSAMPLE delta11 = 2.0 * (gain11out - gain11in) / iNumSamples;
    CSAMPLE gain11 = gain11in;
    const CSAMPLE delta12 = 2.0 * (gain12out - gain12in) / iNumSamples;
    CSAMPLE gain12 = gain12in;
    const CSAMPLE delta13 = 2.0 * (gain13out - gain13in) / iNumSamples;
    CSAMPLE gain13 = gain13in;
    const CSAMPLE delta14 = 2.0 * (gain14out - gain14in) / iNumSamples;
    CSAMPLE gain14 = gain14in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1, gain2 += delta2, gain3 += delta3, gain4 += delta4, gain5 += delta5, gain6 += delta6, gain7 += delta7, gain8 += delta8, gain9 += delta9, gain10 += delta10, gain11 += delta11, gain12 += delta12, gain13 += delta13, gain14 += delta14) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1 +
                       pSrc2[i + 1] * gain2 +
                       pSrc3[i + 1] * gain3 +
                       pSrc4[i + 1] * gain4 +
                       pSrc5[i + 1] * gain5 +
                       pSrc6[i + 1] * gain6 +
                       pSrc7[i + 1] * gain7 +
                       pSrc8[i + 1] * gain8 +
                       pSrc9[i + 1] * gain9 +
                       pSrc10[i + 1] * gain10 +
                       pSrc11[i + 1] * gain11 +
                       pSrc12[i + 1] * gain12 +
                       pSrc13[i + 1] * gain13 +
                       pSrc14[i + 1] * gain14;
    }
}
static inline void copy16WithGain(CSAMPLE* pDest,
                                  const CSAMPLE* pSrc0, CSAMPLE gain0,
                                  const CSAMPLE* pSrc1, CSAMPLE gain1,
                                  const CSAMPLE* pSrc2, CSAMPLE gain2,
                                  const CSAMPLE* pSrc3, CSAMPLE gain3,
                                  const CSAMPLE* pSrc4, CSAMPLE gain4,
                                  const CSAMPLE* pSrc5, CSAMPLE gain5,
                                  const CSAMPLE* pSrc6, CSAMPLE gain6,
                                  const CSAMPLE* pSrc7, CSAMPLE gain7,
                                  const CSAMPLE* pSrc8, CSAMPLE gain8,
                                  const CSAMPLE* pSrc9, CSAMPLE gain9,
                                  const CSAMPLE* pSrc10, CSAMPLE gain10,
                                  const CSAMPLE* pSrc11, CSAMPLE gain11,
                                  const CSAMPLE* pSrc12, CSAMPLE gain12,
                                  const CSAMPLE* pSrc13, CSAMPLE gain13,
                                  const CSAMPLE* pSrc14, CSAMPLE gain14,
                                  const CSAMPLE* pSrc15, CSAMPLE gain15,
                                  int iNumSamples) {
    if (gain0 == 0.0) {
        copy15WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy15WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, iNumSamples);
        return;
    }
    if (gain2 == 0.0) {
        copy15WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, iNumSamples);
        return;
    }
    if (gain3 == 0.0) {
        copy15WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, iNumSamples);
        return;
    }
    if (gain4 == 0.0) {
        copy15WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, iNumSamples);
        return;
    }
    if (gain5 == 0.0) {
        copy15WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, iNumSamples);
        return;
    }
    if (gain6 == 0.0) {
        copy15WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, iNumSamples);
        return;
    }
    if (gain7 == 0.0) {
        copy15WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, iNumSamples);
        return;
    }
    if (gain8 == 0.0) {
        copy15WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, iNumSamples);
        return;
    }
    if (gain9 == 0.0) {
        copy15WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, iNumSamples);
        return;
    }
    if (gain10 == 0.0) {
        copy15WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, iNumSamples);
        return;
    }
    if (gain11 == 0.0) {
        copy15WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, iNumSamples);
        return;
    }
    if (gain12 == 0.0) {
        copy15WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, iNumSamples);
        return;
    }
    if (gain13 == 0.0) {
        copy15WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, iNumSamples);
        return;
    }
    if (gain14 == 0.0) {
        copy15WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, iNumSamples);
        return;
    }
    if (gain15 == 0.0) {
        copy15WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15;
    }
}
static inline void copy16WithRampingGain(CSAMPLE* pDest,
                                         const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                         const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                         const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                                         const CSAMPLE* pSrc3, CSAMPLE gain3in, CSAMPLE gain3out,
                                         const CSAMPLE* pSrc4, CSAMPLE gain4in, CSAMPLE gain4out,
                                         const CSAMPLE* pSrc5, CSAMPLE gain5in, CSAMPLE gain5out,
                                         const CSAMPLE* pSrc6, CSAMPLE gain6in, CSAMPLE gain6out,
                                         const CSAMPLE* pSrc7, CSAMPLE gain7in, CSAMPLE gain7out,
                                         const CSAMPLE* pSrc8, CSAMPLE gain8in, CSAMPLE gain8out,
                                         const CSAMPLE* pSrc9, CSAMPLE gain9in, CSAMPLE gain9out,
                                         const CSAMPLE* pSrc10, CSAMPLE gain10in, CSAMPLE gain10out,
                                         const CSAMPLE* pSrc11, CSAMPLE gain11in, CSAMPLE gain11out,
                                         const CSAMPLE* pSrc12, CSAMPLE gain12in, CSAMPLE gain12out,
                                         const CSAMPLE* pSrc13, CSAMPLE gain13in, CSAMPLE gain13out,
                                         const CSAMPLE* pSrc14, CSAMPLE gain14in, CSAMPLE gain14out,
                                         const CSAMPLE* pSrc15, CSAMPLE gain15in, CSAMPLE gain15out,
                                         int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy15WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy15WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    if (gain2in == 0.0 && gain2out == 0.0) {
        copy15WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    if (gain3in == 0.0 && gain3out == 0.0) {
        copy15WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    if (gain4in == 0.0 && gain4out == 0.0) {
        copy15WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    if (gain5in == 0.0 && gain5out == 0.0) {
        copy15WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    if (gain6in == 0.0 && gain6out == 0.0) {
        copy15WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    if (gain7in == 0.0 && gain7out == 0.0) {
        copy15WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    if (gain8in == 0.0 && gain8out == 0.0) {
        copy15WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    if (gain9in == 0.0 && gain9out == 0.0) {
        copy15WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    if (gain10in == 0.0 && gain10out == 0.0) {
        copy15WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    if (gain11in == 0.0 && gain11out == 0.0) {
        copy15WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    if (gain12in == 0.0 && gain12out == 0.0) {
        copy15WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    if (gain13in == 0.0 && gain13out == 0.0) {
        copy15WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    if (gain14in == 0.0 && gain14out == 0.0) {
        copy15WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    if (gain15in == 0.0 && gain15out == 0.0) {
        copy15WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    CSAMPLE gain2 = gain2in;
    const CSAMPLE delta3 = 2.0 * (gain3out - gain3in) / iNumSamples;
    CSAMPLE gain3 = gain3in;
    const CSAMPLE delta4 = 2.0 * (gain4out - gain4in) / iNumSamples;
    CSAMPLE gain4 = gain4in;
    const CSAMPLE delta5 = 2.0 * (gain5out - gain5in) / iNumSamples;
    CSAMPLE gain5 = gain5in;
    const CSAMPLE delta6 = 2.0 * (gain6out - gain6in) / iNumSamples;
    CSAMPLE gain6 = gain6in;
    const CSAMPLE delta7 = 2.0 * (gain7out - gain7in) / iNumSamples;
    CSAMPLE gain7 = gain7in;
    const CSAMPLE delta8 = 2.0 * (gain8out - gain8in) / iNumSamples;
    CSAMPLE gain8 = gain8in;
    const CSAMPLE delta9 = 2.0 * (gain9out - gain9in) / iNumSamples;
    CSAMPLE gain9 = gain9in;
    const CSAMPLE delta10 = 2.0 * (gain10out - gain10in) / iNumSamples;
    CSAMPLE gain10 = gain10in;
    const CSAMPLE delta11 = 2.0 * (gain11out - gain11in) / iNumSamples;
    CSAMPLE gain11 = gain11in;
    const CSAMPLE delta12 = 2.0 * (gain12out - gain12in) / iNumSamples;
    CSAMPLE gain12 = gain12in;
    const CSAMPLE delta13 = 2.0 * (gain13out - gain13in) / iNumSamples;
    CSAMPLE gain13 = gain13in;
    const CSAMPLE delta14 = 2.0 * (gain14out - gain14in) / iNumSamples;
    CSAMPLE gain14 = gain14in;
    const CSAMPLE delta15 = 2.0 * (gain15out - gain15in) / iNumSamples;
    CSAMPLE gain15 = gain15in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1, gain2 += delta2, gain3 += delta3, gain4 += delta4, gain5 += delta5, gain6 += delta6, gain7 += delta7, gain8 += delta8, gain9 += delta9, gain10 += delta10, gain11 += delta11, gain12 += delta12, gain13 += delta13, gain14 += delta14, gain15 += delta15) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1 +
                       pSrc2[i + 1] * gain2 +
                       pSrc3[i + 1] * gain3 +
                       pSrc4[i + 1] * gain4 +
                       pSrc5[i + 1] * gain5 +
                       pSrc6[i + 1] * gain6 +
                       pSrc7[i + 1] * gain7 +
                       pSrc8[i + 1] * gain8 +
                       pSrc9[i + 1] * gain9 +
                       pSrc10[i + 1] * gain10 +
                       pSrc11[i + 1] * gain11 +
                       pSrc12[i + 1] * gain12 +
                       pSrc13[i + 1] * gain13 +
                       pSrc14[i + 1] * gain14 +
                       pSrc15[i + 1] * gain15;
    }
}
static inline void copy17WithGain(CSAMPLE* pDest,
                                  const CSAMPLE* pSrc0, CSAMPLE gain0,
                                  const CSAMPLE* pSrc1, CSAMPLE gain1,
                                  const CSAMPLE* pSrc2, CSAMPLE gain2,
                                  const CSAMPLE* pSrc3, CSAMPLE gain3,
                                  const CSAMPLE* pSrc4, CSAMPLE gain4,
                                  const CSAMPLE* pSrc5, CSAMPLE gain5,
                                  const CSAMPLE* pSrc6, CSAMPLE gain6,
                                  const CSAMPLE* pSrc7, CSAMPLE gain7,
                                  const CSAMPLE* pSrc8, CSAMPLE gain8,
                                  const CSAMPLE* pSrc9, CSAMPLE gain9,
                                  const CSAMPLE* pSrc10, CSAMPLE gain10,
                                  const CSAMPLE* pSrc11, CSAMPLE gain11,
                                  const CSAMPLE* pSrc12, CSAMPLE gain12,
                                  const CSAMPLE* pSrc13, CSAMPLE gain13,
                                  const CSAMPLE* pSrc14, CSAMPLE gain14,
                                  const CSAMPLE* pSrc15, CSAMPLE gain15,
                                  const CSAMPLE* pSrc16, CSAMPLE gain16,
                                  int iNumSamples) {
    if (gain0 == 0.0) {
        copy16WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain2 == 0.0) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain3 == 0.0) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain4 == 0.0) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain5 == 0.0) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain6 == 0.0) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain7 == 0.0) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain8 == 0.0) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain9 == 0.0) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain10 == 0.0) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain11 == 0.0) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain12 == 0.0) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain13 == 0.0) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain14 == 0.0) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain15 == 0.0) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain16 == 0.0) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16;
    }
}
static inline void copy17WithRampingGain(CSAMPLE* pDest,
                                         const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                         const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                         const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                                         const CSAMPLE* pSrc3, CSAMPLE gain3in, CSAMPLE gain3out,
                                         const CSAMPLE* pSrc4, CSAMPLE gain4in, CSAMPLE gain4out,
                                         const CSAMPLE* pSrc5, CSAMPLE gain5in, CSAMPLE gain5out,
                                         const CSAMPLE* pSrc6, CSAMPLE gain6in, CSAMPLE gain6out,
                                         const CSAMPLE* pSrc7, CSAMPLE gain7in, CSAMPLE gain7out,
                                         const CSAMPLE* pSrc8, CSAMPLE gain8in, CSAMPLE gain8out,
                                         const CSAMPLE* pSrc9, CSAMPLE gain9in, CSAMPLE gain9out,
                                         const CSAMPLE* pSrc10, CSAMPLE gain10in, CSAMPLE gain10out,
                                         const CSAMPLE* pSrc11, CSAMPLE gain11in, CSAMPLE gain11out,
                                         const CSAMPLE* pSrc12, CSAMPLE gain12in, CSAMPLE gain12out,
                                         const CSAMPLE* pSrc13, CSAMPLE gain13in, CSAMPLE gain13out,
                                         const CSAMPLE* pSrc14, CSAMPLE gain14in, CSAMPLE gain14out,
                                         const CSAMPLE* pSrc15, CSAMPLE gain15in, CSAMPLE gain15out,
                                         const CSAMPLE* pSrc16, CSAMPLE gain16in, CSAMPLE gain16out,
                                         int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy16WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain2in == 0.0 && gain2out == 0.0) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain3in == 0.0 && gain3out == 0.0) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain4in == 0.0 && gain4out == 0.0) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain5in == 0.0 && gain5out == 0.0) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain6in == 0.0 && gain6out == 0.0) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain7in == 0.0 && gain7out == 0.0) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain8in == 0.0 && gain8out == 0.0) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain9in == 0.0 && gain9out == 0.0) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain10in == 0.0 && gain10out == 0.0) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain11in == 0.0 && gain11out == 0.0) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain12in == 0.0 && gain12out == 0.0) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain13in == 0.0 && gain13out == 0.0) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain14in == 0.0 && gain14out == 0.0) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain15in == 0.0 && gain15out == 0.0) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain16in == 0.0 && gain16out == 0.0) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    CSAMPLE gain2 = gain2in;
    const CSAMPLE delta3 = 2.0 * (gain3out - gain3in) / iNumSamples;
    CSAMPLE gain3 = gain3in;
    const CSAMPLE delta4 = 2.0 * (gain4out - gain4in) / iNumSamples;
    CSAMPLE gain4 = gain4in;
    const CSAMPLE delta5 = 2.0 * (gain5out - gain5in) / iNumSamples;
    CSAMPLE gain5 = gain5in;
    const CSAMPLE delta6 = 2.0 * (gain6out - gain6in) / iNumSamples;
    CSAMPLE gain6 = gain6in;
    const CSAMPLE delta7 = 2.0 * (gain7out - gain7in) / iNumSamples;
    CSAMPLE gain7 = gain7in;
    const CSAMPLE delta8 = 2.0 * (gain8out - gain8in) / iNumSamples;
    CSAMPLE gain8 = gain8in;
    const CSAMPLE delta9 = 2.0 * (gain9out - gain9in) / iNumSamples;
    CSAMPLE gain9 = gain9in;
    const CSAMPLE delta10 = 2.0 * (gain10out - gain10in) / iNumSamples;
    CSAMPLE gain10 = gain10in;
    const CSAMPLE delta11 = 2.0 * (gain11out - gain11in) / iNumSamples;
    CSAMPLE gain11 = gain11in;
    const CSAMPLE delta12 = 2.0 * (gain12out - gain12in) / iNumSamples;
    CSAMPLE gain12 = gain12in;
    const CSAMPLE delta13 = 2.0 * (gain13out - gain13in) / iNumSamples;
    CSAMPLE gain13 = gain13in;
    const CSAMPLE delta14 = 2.0 * (gain14out - gain14in) / iNumSamples;
    CSAMPLE gain14 = gain14in;
    const CSAMPLE delta15 = 2.0 * (gain15out - gain15in) / iNumSamples;
    CSAMPLE gain15 = gain15in;
    const CSAMPLE delta16 = 2.0 * (gain16out - gain16in) / iNumSamples;
    CSAMPLE gain16 = gain16in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1, gain2 += delta2, gain3 += delta3, gain4 += delta4, gain5 += delta5, gain6 += delta6, gain7 += delta7, gain8 += delta8, gain9 += delta9, gain10 += delta10, gain11 += delta11, gain12 += delta12, gain13 += delta13, gain14 += delta14, gain15 += delta15, gain16 += delta16) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1 +
                       pSrc2[i + 1] * gain2 +
                       pSrc3[i + 1] * gain3 +
                       pSrc4[i + 1] * gain4 +
                       pSrc5[i + 1] * gain5 +
                       pSrc6[i + 1] * gain6 +
                       pSrc7[i + 1] * gain7 +
                       pSrc8[i + 1] * gain8 +
                       pSrc9[i + 1] * gain9 +
                       pSrc10[i + 1] * gain10 +
                       pSrc11[i + 1] * gain11 +
                       pSrc12[i + 1] * gain12 +
                       pSrc13[i + 1] * gain13 +
                       pSrc14[i + 1] * gain14 +
                       pSrc15[i + 1] * gain15 +
                       pSrc16[i + 1] * gain16;
    }
}
static inline void copy18WithGain(CSAMPLE* pDest,
                                  const CSAMPLE* pSrc0, CSAMPLE gain0,
                                  const CSAMPLE* pSrc1, CSAMPLE gain1,
                                  const CSAMPLE* pSrc2, CSAMPLE gain2,
                                  const CSAMPLE* pSrc3, CSAMPLE gain3,
                                  const CSAMPLE* pSrc4, CSAMPLE gain4,
                                  const CSAMPLE* pSrc5, CSAMPLE gain5,
                                  const CSAMPLE* pSrc6, CSAMPLE gain6,
                                  const CSAMPLE* pSrc7, CSAMPLE gain7,
                                  const CSAMPLE* pSrc8, CSAMPLE gain8,
                                  const CSAMPLE* pSrc9, CSAMPLE gain9,
                                  const CSAMPLE* pSrc10, CSAMPLE gain10,
                                  const CSAMPLE* pSrc11, CSAMPLE gain11,
                                  const CSAMPLE* pSrc12, CSAMPLE gain12,
                                  const CSAMPLE* pSrc13, CSAMPLE gain13,
                                  const CSAMPLE* pSrc14, CSAMPLE gain14,
                                  const CSAMPLE* pSrc15, CSAMPLE gain15,
                                  const CSAMPLE* pSrc16, CSAMPLE gain16,
                                  const CSAMPLE* pSrc17, CSAMPLE gain17,
                                  int iNumSamples) {
    if (gain0 == 0.0) {
        copy17WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain2 == 0.0) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain3 == 0.0) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain4 == 0.0) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain5 == 0.0) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain6 == 0.0) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain7 == 0.0) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain8 == 0.0) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain9 == 0.0) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain10 == 0.0) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain11 == 0.0) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain12 == 0.0) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain13 == 0.0) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain14 == 0.0) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain15 == 0.0) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain16 == 0.0) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain17 == 0.0) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16 +
                   pSrc17[i] * gain17;
    }
}
static inline void copy18WithRampingGain(CSAMPLE* pDest,
                                         const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                         const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                         const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                                         const CSAMPLE* pSrc3, CSAMPLE gain3in, CSAMPLE gain3out,
                                         const CSAMPLE* pSrc4, CSAMPLE gain4in, CSAMPLE gain4out,
                                         const CSAMPLE* pSrc5, CSAMPLE gain5in, CSAMPLE gain5out,
                                         const CSAMPLE* pSrc6, CSAMPLE gain6in, CSAMPLE gain6out,
                                         const CSAMPLE* pSrc7, CSAMPLE gain7in, CSAMPLE gain7out,
                                         const CSAMPLE* pSrc8, CSAMPLE gain8in, CSAMPLE gain8out,
                                         const CSAMPLE* pSrc9, CSAMPLE gain9in, CSAMPLE gain9out,
                                         const CSAMPLE* pSrc10, CSAMPLE gain10in, CSAMPLE gain10out,
                                         const CSAMPLE* pSrc11, CSAMPLE gain11in, CSAMPLE gain11out,
                                         const CSAMPLE* pSrc12, CSAMPLE gain12in, CSAMPLE gain12out,
                                         const CSAMPLE* pSrc13, CSAMPLE gain13in, CSAMPLE gain13out,
                                         const CSAMPLE* pSrc14, CSAMPLE gain14in, CSAMPLE gain14out,
                                         const CSAMPLE* pSrc15, CSAMPLE gain15in, CSAMPLE gain15out,
                                         const CSAMPLE* pSrc16, CSAMPLE gain16in, CSAMPLE gain16out,
                                         const CSAMPLE* pSrc17, CSAMPLE gain17in, CSAMPLE gain17out,
                                         int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy17WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain2in == 0.0 && gain2out == 0.0) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain3in == 0.0 && gain3out == 0.0) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain4in == 0.0 && gain4out == 0.0) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain5in == 0.0 && gain5out == 0.0) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain6in == 0.0 && gain6out == 0.0) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain7in == 0.0 && gain7out == 0.0) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain8in == 0.0 && gain8out == 0.0) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain9in == 0.0 && gain9out == 0.0) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain10in == 0.0 && gain10out == 0.0) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain11in == 0.0 && gain11out == 0.0) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain12in == 0.0 && gain12out == 0.0) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain13in == 0.0 && gain13out == 0.0) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain14in == 0.0 && gain14out == 0.0) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain15in == 0.0 && gain15out == 0.0) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain16in == 0.0 && gain16out == 0.0) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain17in == 0.0 && gain17out == 0.0) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    CSAMPLE gain2 = gain2in;
    const CSAMPLE delta3 = 2.0 * (gain3out - gain3in) / iNumSamples;
    CSAMPLE gain3 = gain3in;
    const CSAMPLE delta4 = 2.0 * (gain4out - gain4in) / iNumSamples;
    CSAMPLE gain4 = gain4in;
    const CSAMPLE delta5 = 2.0 * (gain5out - gain5in) / iNumSamples;
    CSAMPLE gain5 = gain5in;
    const CSAMPLE delta6 = 2.0 * (gain6out - gain6in) / iNumSamples;
    CSAMPLE gain6 = gain6in;
    const CSAMPLE delta7 = 2.0 * (gain7out - gain7in) / iNumSamples;
    CSAMPLE gain7 = gain7in;
    const CSAMPLE delta8 = 2.0 * (gain8out - gain8in) / iNumSamples;
    CSAMPLE gain8 = gain8in;
    const CSAMPLE delta9 = 2.0 * (gain9out - gain9in) / iNumSamples;
    CSAMPLE gain9 = gain9in;
    const CSAMPLE delta10 = 2.0 * (gain10out - gain10in) / iNumSamples;
    CSAMPLE gain10 = gain10in;
    const CSAMPLE delta11 = 2.0 * (gain11out - gain11in) / iNumSamples;
    CSAMPLE gain11 = gain11in;
    const CSAMPLE delta12 = 2.0 * (gain12out - gain12in) / iNumSamples;
    CSAMPLE gain12 = gain12in;
    const CSAMPLE delta13 = 2.0 * (gain13out - gain13in) / iNumSamples;
    CSAMPLE gain13 = gain13in;
    const CSAMPLE delta14 = 2.0 * (gain14out - gain14in) / iNumSamples;
    CSAMPLE gain14 = gain14in;
    const CSAMPLE delta15 = 2.0 * (gain15out - gain15in) / iNumSamples;
    CSAMPLE gain15 = gain15in;
    const CSAMPLE delta16 = 2.0 * (gain16out - gain16in) / iNumSamples;
    CSAMPLE gain16 = gain16in;
    const CSAMPLE delta17 = 2.0 * (gain17out - gain17in) / iNumSamples;
    CSAMPLE gain17 = gain17in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1, gain2 += delta2, gain3 += delta3, gain4 += delta4, gain5 += delta5, gain6 += delta6, gain7 += delta7, gain8 += delta8, gain9 += delta9, gain10 += delta10, gain11 += delta11, gain12 += delta12, gain13 += delta13, gain14 += delta14, gain15 += delta15, gain16 += delta16, gain17 += delta17) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16 +
                   pSrc17[i] * gain17;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1 +
                       pSrc2[i + 1] * gain2 +
                       pSrc3[i + 1] * gain3 +
                       pSrc4[i + 1] * gain4 +
                       pSrc5[i + 1] * gain5 +
                       pSrc6[i + 1] * gain6 +
                       pSrc7[i + 1] * gain7 +
                       pSrc8[i + 1] * gain8 +
                       pSrc9[i + 1] * gain9 +
                       pSrc10[i + 1] * gain10 +
                       pSrc11[i + 1] * gain11 +
                       pSrc12[i + 1] * gain12 +
                       pSrc13[i + 1] * gain13 +
                       pSrc14[i + 1] * gain14 +
                       pSrc15[i + 1] * gain15 +
                       pSrc16[i + 1] * gain16 +
                       pSrc17[i + 1] * gain17;
    }
}
static inline void copy19WithGain(CSAMPLE* pDest,
                                  const CSAMPLE* pSrc0, CSAMPLE gain0,
                                  const CSAMPLE* pSrc1, CSAMPLE gain1,
                                  const CSAMPLE* pSrc2, CSAMPLE gain2,
                                  const CSAMPLE* pSrc3, CSAMPLE gain3,
                                  const CSAMPLE* pSrc4, CSAMPLE gain4,
                                  const CSAMPLE* pSrc5, CSAMPLE gain5,
                                  const CSAMPLE* pSrc6, CSAMPLE gain6,
                                  const CSAMPLE* pSrc7, CSAMPLE gain7,
                                  const CSAMPLE* pSrc8, CSAMPLE gain8,
                                  const CSAMPLE* pSrc9, CSAMPLE gain9,
                                  const CSAMPLE* pSrc10, CSAMPLE gain10,
                                  const CSAMPLE* pSrc11, CSAMPLE gain11,
                                  const CSAMPLE* pSrc12, CSAMPLE gain12,
                                  const CSAMPLE* pSrc13, CSAMPLE gain13,
                                  const CSAMPLE* pSrc14, CSAMPLE gain14,
                                  const CSAMPLE* pSrc15, CSAMPLE gain15,
                                  const CSAMPLE* pSrc16, CSAMPLE gain16,
                                  const CSAMPLE* pSrc17, CSAMPLE gain17,
                                  const CSAMPLE* pSrc18, CSAMPLE gain18,
                                  int iNumSamples) {
    if (gain0 == 0.0) {
        copy18WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain2 == 0.0) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain3 == 0.0) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain4 == 0.0) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain5 == 0.0) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain6 == 0.0) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain7 == 0.0) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain8 == 0.0) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain9 == 0.0) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain10 == 0.0) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain11 == 0.0) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain12 == 0.0) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain13 == 0.0) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain14 == 0.0) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain15 == 0.0) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain16 == 0.0) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain17 == 0.0) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain18 == 0.0) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16 +
                   pSrc17[i] * gain17 +
                   pSrc18[i] * gain18;
    }
}
static inline void copy19WithRampingGain(CSAMPLE* pDest,
                                         const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                         const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                         const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                                         const CSAMPLE* pSrc3, CSAMPLE gain3in, CSAMPLE gain3out,
                                         const CSAMPLE* pSrc4, CSAMPLE gain4in, CSAMPLE gain4out,
                                         const CSAMPLE* pSrc5, CSAMPLE gain5in, CSAMPLE gain5out,
                                         const CSAMPLE* pSrc6, CSAMPLE gain6in, CSAMPLE gain6out,
                                         const CSAMPLE* pSrc7, CSAMPLE gain7in, CSAMPLE gain7out,
                                         const CSAMPLE* pSrc8, CSAMPLE gain8in, CSAMPLE gain8out,
                                         const CSAMPLE* pSrc9, CSAMPLE gain9in, CSAMPLE gain9out,
                                         const CSAMPLE* pSrc10, CSAMPLE gain10in, CSAMPLE gain10out,
                                         const CSAMPLE* pSrc11, CSAMPLE gain11in, CSAMPLE gain11out,
                                         const CSAMPLE* pSrc12, CSAMPLE gain12in, CSAMPLE gain12out,
                                         const CSAMPLE* pSrc13, CSAMPLE gain13in, CSAMPLE gain13out,
                                         const CSAMPLE* pSrc14, CSAMPLE gain14in, CSAMPLE gain14out,
                                         const CSAMPLE* pSrc15, CSAMPLE gain15in, CSAMPLE gain15out,
                                         const CSAMPLE* pSrc16, CSAMPLE gain16in, CSAMPLE gain16out,
                                         const CSAMPLE* pSrc17, CSAMPLE gain17in, CSAMPLE gain17out,
                                         const CSAMPLE* pSrc18, CSAMPLE gain18in, CSAMPLE gain18out,
                                         int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy18WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain2in == 0.0 && gain2out == 0.0) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain3in == 0.0 && gain3out == 0.0) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain4in == 0.0 && gain4out == 0.0) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain5in == 0.0 && gain5out == 0.0) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain6in == 0.0 && gain6out == 0.0) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain7in == 0.0 && gain7out == 0.0) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain8in == 0.0 && gain8out == 0.0) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain9in == 0.0 && gain9out == 0.0) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain10in == 0.0 && gain10out == 0.0) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain11in == 0.0 && gain11out == 0.0) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain12in == 0.0 && gain12out == 0.0) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain13in == 0.0 && gain13out == 0.0) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain14in == 0.0 && gain14out == 0.0) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain15in == 0.0 && gain15out == 0.0) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain16in == 0.0 && gain16out == 0.0) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain17in == 0.0 && gain17out == 0.0) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain18in == 0.0 && gain18out == 0.0) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    CSAMPLE gain2 = gain2in;
    const CSAMPLE delta3 = 2.0 * (gain3out - gain3in) / iNumSamples;
    CSAMPLE gain3 = gain3in;
    const CSAMPLE delta4 = 2.0 * (gain4out - gain4in) / iNumSamples;
    CSAMPLE gain4 = gain4in;
    const CSAMPLE delta5 = 2.0 * (gain5out - gain5in) / iNumSamples;
    CSAMPLE gain5 = gain5in;
    const CSAMPLE delta6 = 2.0 * (gain6out - gain6in) / iNumSamples;
    CSAMPLE gain6 = gain6in;
    const CSAMPLE delta7 = 2.0 * (gain7out - gain7in) / iNumSamples;
    CSAMPLE gain7 = gain7in;
    const CSAMPLE delta8 = 2.0 * (gain8out - gain8in) / iNumSamples;
    CSAMPLE gain8 = gain8in;
    const CSAMPLE delta9 = 2.0 * (gain9out - gain9in) / iNumSamples;
    CSAMPLE gain9 = gain9in;
    const CSAMPLE delta10 = 2.0 * (gain10out - gain10in) / iNumSamples;
    CSAMPLE gain10 = gain10in;
    const CSAMPLE delta11 = 2.0 * (gain11out - gain11in) / iNumSamples;
    CSAMPLE gain11 = gain11in;
    const CSAMPLE delta12 = 2.0 * (gain12out - gain12in) / iNumSamples;
    CSAMPLE gain12 = gain12in;
    const CSAMPLE delta13 = 2.0 * (gain13out - gain13in) / iNumSamples;
    CSAMPLE gain13 = gain13in;
    const CSAMPLE delta14 = 2.0 * (gain14out - gain14in) / iNumSamples;
    CSAMPLE gain14 = gain14in;
    const CSAMPLE delta15 = 2.0 * (gain15out - gain15in) / iNumSamples;
    CSAMPLE gain15 = gain15in;
    const CSAMPLE delta16 = 2.0 * (gain16out - gain16in) / iNumSamples;
    CSAMPLE gain16 = gain16in;
    const CSAMPLE delta17 = 2.0 * (gain17out - gain17in) / iNumSamples;
    CSAMPLE gain17 = gain17in;
    const CSAMPLE delta18 = 2.0 * (gain18out - gain18in) / iNumSamples;
    CSAMPLE gain18 = gain18in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1, gain2 += delta2, gain3 += delta3, gain4 += delta4, gain5 += delta5, gain6 += delta6, gain7 += delta7, gain8 += delta8, gain9 += delta9, gain10 += delta10, gain11 += delta11, gain12 += delta12, gain13 += delta13, gain14 += delta14, gain15 += delta15, gain16 += delta16, gain17 += delta17, gain18 += delta18) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16 +
                   pSrc17[i] * gain17 +
                   pSrc18[i] * gain18;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1 +
                       pSrc2[i + 1] * gain2 +
                       pSrc3[i + 1] * gain3 +
                       pSrc4[i + 1] * gain4 +
                       pSrc5[i + 1] * gain5 +
                       pSrc6[i + 1] * gain6 +
                       pSrc7[i + 1] * gain7 +
                       pSrc8[i + 1] * gain8 +
                       pSrc9[i + 1] * gain9 +
                       pSrc10[i + 1] * gain10 +
                       pSrc11[i + 1] * gain11 +
                       pSrc12[i + 1] * gain12 +
                       pSrc13[i + 1] * gain13 +
                       pSrc14[i + 1] * gain14 +
                       pSrc15[i + 1] * gain15 +
                       pSrc16[i + 1] * gain16 +
                       pSrc17[i + 1] * gain17 +
                       pSrc18[i + 1] * gain18;
    }
}
static inline void copy20WithGain(CSAMPLE* pDest,
                                  const CSAMPLE* pSrc0, CSAMPLE gain0,
                                  const CSAMPLE* pSrc1, CSAMPLE gain1,
                                  const CSAMPLE* pSrc2, CSAMPLE gain2,
                                  const CSAMPLE* pSrc3, CSAMPLE gain3,
                                  const CSAMPLE* pSrc4, CSAMPLE gain4,
                                  const CSAMPLE* pSrc5, CSAMPLE gain5,
                                  const CSAMPLE* pSrc6, CSAMPLE gain6,
                                  const CSAMPLE* pSrc7, CSAMPLE gain7,
                                  const CSAMPLE* pSrc8, CSAMPLE gain8,
                                  const CSAMPLE* pSrc9, CSAMPLE gain9,
                                  const CSAMPLE* pSrc10, CSAMPLE gain10,
                                  const CSAMPLE* pSrc11, CSAMPLE gain11,
                                  const CSAMPLE* pSrc12, CSAMPLE gain12,
                                  const CSAMPLE* pSrc13, CSAMPLE gain13,
                                  const CSAMPLE* pSrc14, CSAMPLE gain14,
                                  const CSAMPLE* pSrc15, CSAMPLE gain15,
                                  const CSAMPLE* pSrc16, CSAMPLE gain16,
                                  const CSAMPLE* pSrc17, CSAMPLE gain17,
                                  const CSAMPLE* pSrc18, CSAMPLE gain18,
                                  const CSAMPLE* pSrc19, CSAMPLE gain19,
                                  int iNumSamples) {
    if (gain0 == 0.0) {
        copy19WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain2 == 0.0) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain3 == 0.0) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain4 == 0.0) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain5 == 0.0) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain6 == 0.0) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain7 == 0.0) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain8 == 0.0) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain9 == 0.0) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain10 == 0.0) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain11 == 0.0) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain12 == 0.0) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain13 == 0.0) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain14 == 0.0) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain15 == 0.0) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain16 == 0.0) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain17 == 0.0) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain18 == 0.0) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain19 == 0.0) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16 +
                   pSrc17[i] * gain17 +
                   pSrc18[i] * gain18 +
                   pSrc19[i] * gain19;
    }
}
static inline void copy20WithRampingGain(CSAMPLE* pDest,
                                         const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                         const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                         const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                                         const CSAMPLE* pSrc3, CSAMPLE gain3in, CSAMPLE gain3out,
                                         const CSAMPLE* pSrc4, CSAMPLE gain4in, CSAMPLE gain4out,
                                         const CSAMPLE* pSrc5, CSAMPLE gain5in, CSAMPLE gain5out,
                                         const CSAMPLE* pSrc6, CSAMPLE gain6in, CSAMPLE gain6out,
                                         const CSAMPLE* pSrc7, CSAMPLE gain7in, CSAMPLE gain7out,
                                         const CSAMPLE* pSrc8, CSAMPLE gain8in, CSAMPLE gain8out,
                                         const CSAMPLE* pSrc9, CSAMPLE gain9in, CSAMPLE gain9out,
                                         const CSAMPLE* pSrc10, CSAMPLE gain10in, CSAMPLE gain10out,
                                         const CSAMPLE* pSrc11, CSAMPLE gain11in, CSAMPLE gain11out,
                                         const CSAMPLE* pSrc12, CSAMPLE gain12in, CSAMPLE gain12out,
                                         const CSAMPLE* pSrc13, CSAMPLE gain13in, CSAMPLE gain13out,
                                         const CSAMPLE* pSrc14, CSAMPLE gain14in, CSAMPLE gain14out,
                                         const CSAMPLE* pSrc15, CSAMPLE gain15in, CSAMPLE gain15out,
                                         const CSAMPLE* pSrc16, CSAMPLE gain16in, CSAMPLE gain16out,
                                         const CSAMPLE* pSrc17, CSAMPLE gain17in, CSAMPLE gain17out,
                                         const CSAMPLE* pSrc18, CSAMPLE gain18in, CSAMPLE gain18out,
                                         const CSAMPLE* pSrc19, CSAMPLE gain19in, CSAMPLE gain19out,
                                         int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy19WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain2in == 0.0 && gain2out == 0.0) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain3in == 0.0 && gain3out == 0.0) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain4in == 0.0 && gain4out == 0.0) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain5in == 0.0 && gain5out == 0.0) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain6in == 0.0 && gain6out == 0.0) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain7in == 0.0 && gain7out == 0.0) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain8in == 0.0 && gain8out == 0.0) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain9in == 0.0 && gain9out == 0.0) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain10in == 0.0 && gain10out == 0.0) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain11in == 0.0 && gain11out == 0.0) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain12in == 0.0 && gain12out == 0.0) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain13in == 0.0 && gain13out == 0.0) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain14in == 0.0 && gain14out == 0.0) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain15in == 0.0 && gain15out == 0.0) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain16in == 0.0 && gain16out == 0.0) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain17in == 0.0 && gain17out == 0.0) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain18in == 0.0 && gain18out == 0.0) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain19in == 0.0 && gain19out == 0.0) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    CSAMPLE gain2 = gain2in;
    const CSAMPLE delta3 = 2.0 * (gain3out - gain3in) / iNumSamples;
    CSAMPLE gain3 = gain3in;
    const CSAMPLE delta4 = 2.0 * (gain4out - gain4in) / iNumSamples;
    CSAMPLE gain4 = gain4in;
    const CSAMPLE delta5 = 2.0 * (gain5out - gain5in) / iNumSamples;
    CSAMPLE gain5 = gain5in;
    const CSAMPLE delta6 = 2.0 * (gain6out - gain6in) / iNumSamples;
    CSAMPLE gain6 = gain6in;
    const CSAMPLE delta7 = 2.0 * (gain7out - gain7in) / iNumSamples;
    CSAMPLE gain7 = gain7in;
    const CSAMPLE delta8 = 2.0 * (gain8out - gain8in) / iNumSamples;
    CSAMPLE gain8 = gain8in;
    const CSAMPLE delta9 = 2.0 * (gain9out - gain9in) / iNumSamples;
    CSAMPLE gain9 = gain9in;
    const CSAMPLE delta10 = 2.0 * (gain10out - gain10in) / iNumSamples;
    CSAMPLE gain10 = gain10in;
    const CSAMPLE delta11 = 2.0 * (gain11out - gain11in) / iNumSamples;
    CSAMPLE gain11 = gain11in;
    const CSAMPLE delta12 = 2.0 * (gain12out - gain12in) / iNumSamples;
    CSAMPLE gain12 = gain12in;
    const CSAMPLE delta13 = 2.0 * (gain13out - gain13in) / iNumSamples;
    CSAMPLE gain13 = gain13in;
    const CSAMPLE delta14 = 2.0 * (gain14out - gain14in) / iNumSamples;
    CSAMPLE gain14 = gain14in;
    const CSAMPLE delta15 = 2.0 * (gain15out - gain15in) / iNumSamples;
    CSAMPLE gain15 = gain15in;
    const CSAMPLE delta16 = 2.0 * (gain16out - gain16in) / iNumSamples;
    CSAMPLE gain16 = gain16in;
    const CSAMPLE delta17 = 2.0 * (gain17out - gain17in) / iNumSamples;
    CSAMPLE gain17 = gain17in;
    const CSAMPLE delta18 = 2.0 * (gain18out - gain18in) / iNumSamples;
    CSAMPLE gain18 = gain18in;
    const CSAMPLE delta19 = 2.0 * (gain19out - gain19in) / iNumSamples;
    CSAMPLE gain19 = gain19in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1, gain2 += delta2, gain3 += delta3, gain4 += delta4, gain5 += delta5, gain6 += delta6, gain7 += delta7, gain8 += delta8, gain9 += delta9, gain10 += delta10, gain11 += delta11, gain12 += delta12, gain13 += delta13, gain14 += delta14, gain15 += delta15, gain16 += delta16, gain17 += delta17, gain18 += delta18, gain19 += delta19) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16 +
                   pSrc17[i] * gain17 +
                   pSrc18[i] * gain18 +
                   pSrc19[i] * gain19;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1 +
                       pSrc2[i + 1] * gain2 +
                       pSrc3[i + 1] * gain3 +
                       pSrc4[i + 1] * gain4 +
                       pSrc5[i + 1] * gain5 +
                       pSrc6[i + 1] * gain6 +
                       pSrc7[i + 1] * gain7 +
                       pSrc8[i + 1] * gain8 +
                       pSrc9[i + 1] * gain9 +
                       pSrc10[i + 1] * gain10 +
                       pSrc11[i + 1] * gain11 +
                       pSrc12[i + 1] * gain12 +
                       pSrc13[i + 1] * gain13 +
                       pSrc14[i + 1] * gain14 +
                       pSrc15[i + 1] * gain15 +
                       pSrc16[i + 1] * gain16 +
                       pSrc17[i + 1] * gain17 +
                       pSrc18[i + 1] * gain18 +
                       pSrc19[i + 1] * gain19;
    }
}
static inline void copy21WithGain(CSAMPLE* pDest,
                                  const CSAMPLE* pSrc0, CSAMPLE gain0,
                                  const CSAMPLE* pSrc1, CSAMPLE gain1,
                                  const CSAMPLE* pSrc2, CSAMPLE gain2,
                                  const CSAMPLE* pSrc3, CSAMPLE gain3,
                                  const CSAMPLE* pSrc4, CSAMPLE gain4,
                                  const CSAMPLE* pSrc5, CSAMPLE gain5,
                                  const CSAMPLE* pSrc6, CSAMPLE gain6,
                                  const CSAMPLE* pSrc7, CSAMPLE gain7,
                                  const CSAMPLE* pSrc8, CSAMPLE gain8,
                                  const CSAMPLE* pSrc9, CSAMPLE gain9,
                                  const CSAMPLE* pSrc10, CSAMPLE gain10,
                                  const CSAMPLE* pSrc11, CSAMPLE gain11,
                                  const CSAMPLE* pSrc12, CSAMPLE gain12,
                                  const CSAMPLE* pSrc13, CSAMPLE gain13,
                                  const CSAMPLE* pSrc14, CSAMPLE gain14,
                                  const CSAMPLE* pSrc15, CSAMPLE gain15,
                                  const CSAMPLE* pSrc16, CSAMPLE gain16,
                                  const CSAMPLE* pSrc17, CSAMPLE gain17,
                                  const CSAMPLE* pSrc18, CSAMPLE gain18,
                                  const CSAMPLE* pSrc19, CSAMPLE gain19,
                                  const CSAMPLE* pSrc20, CSAMPLE gain20,
                                  int iNumSamples) {
    if (gain0 == 0.0) {
        copy20WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain2 == 0.0) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain3 == 0.0) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain4 == 0.0) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain5 == 0.0) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain6 == 0.0) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain7 == 0.0) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain8 == 0.0) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain9 == 0.0) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain10 == 0.0) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain11 == 0.0) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain12 == 0.0) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain13 == 0.0) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain14 == 0.0) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain15 == 0.0) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain16 == 0.0) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain17 == 0.0) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain18 == 0.0) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain19 == 0.0) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain20 == 0.0) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16 +
                   pSrc17[i] * gain17 +
                   pSrc18[i] * gain18 +
                   pSrc19[i] * gain19 +
                   pSrc20[i] * gain20;
    }
}
static inline void copy21WithRampingGain(CSAMPLE* pDest,
                                         const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                         const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                         const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                                         const CSAMPLE* pSrc3, CSAMPLE gain3in, CSAMPLE gain3out,
                                         const CSAMPLE* pSrc4, CSAMPLE gain4in, CSAMPLE gain4out,
                                         const CSAMPLE* pSrc5, CSAMPLE gain5in, CSAMPLE gain5out,
                                         const CSAMPLE* pSrc6, CSAMPLE gain6in, CSAMPLE gain6out,
                                         const CSAMPLE* pSrc7, CSAMPLE gain7in, CSAMPLE gain7out,
                                         const CSAMPLE* pSrc8, CSAMPLE gain8in, CSAMPLE gain8out,
                                         const CSAMPLE* pSrc9, CSAMPLE gain9in, CSAMPLE gain9out,
                                         const CSAMPLE* pSrc10, CSAMPLE gain10in, CSAMPLE gain10out,
                                         const CSAMPLE* pSrc11, CSAMPLE gain11in, CSAMPLE gain11out,
                                         const CSAMPLE* pSrc12, CSAMPLE gain12in, CSAMPLE gain12out,
                                         const CSAMPLE* pSrc13, CSAMPLE gain13in, CSAMPLE gain13out,
                                         const CSAMPLE* pSrc14, CSAMPLE gain14in, CSAMPLE gain14out,
                                         const CSAMPLE* pSrc15, CSAMPLE gain15in, CSAMPLE gain15out,
                                         const CSAMPLE* pSrc16, CSAMPLE gain16in, CSAMPLE gain16out,
                                         const CSAMPLE* pSrc17, CSAMPLE gain17in, CSAMPLE gain17out,
                                         const CSAMPLE* pSrc18, CSAMPLE gain18in, CSAMPLE gain18out,
                                         const CSAMPLE* pSrc19, CSAMPLE gain19in, CSAMPLE gain19out,
                                         const CSAMPLE* pSrc20, CSAMPLE gain20in, CSAMPLE gain20out,
                                         int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy20WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain2in == 0.0 && gain2out == 0.0) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain3in == 0.0 && gain3out == 0.0) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain4in == 0.0 && gain4out == 0.0) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain5in == 0.0 && gain5out == 0.0) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain6in == 0.0 && gain6out == 0.0) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain7in == 0.0 && gain7out == 0.0) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain8in == 0.0 && gain8out == 0.0) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain9in == 0.0 && gain9out == 0.0) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain10in == 0.0 && gain10out == 0.0) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain11in == 0.0 && gain11out == 0.0) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain12in == 0.0 && gain12out == 0.0) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain13in == 0.0 && gain13out == 0.0) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain14in == 0.0 && gain14out == 0.0) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain15in == 0.0 && gain15out == 0.0) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain16in == 0.0 && gain16out == 0.0) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain17in == 0.0 && gain17out == 0.0) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain18in == 0.0 && gain18out == 0.0) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain19in == 0.0 && gain19out == 0.0) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain20in == 0.0 && gain20out == 0.0) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    CSAMPLE gain2 = gain2in;
    const CSAMPLE delta3 = 2.0 * (gain3out - gain3in) / iNumSamples;
    CSAMPLE gain3 = gain3in;
    const CSAMPLE delta4 = 2.0 * (gain4out - gain4in) / iNumSamples;
    CSAMPLE gain4 = gain4in;
    const CSAMPLE delta5 = 2.0 * (gain5out - gain5in) / iNumSamples;
    CSAMPLE gain5 = gain5in;
    const CSAMPLE delta6 = 2.0 * (gain6out - gain6in) / iNumSamples;
    CSAMPLE gain6 = gain6in;
    const CSAMPLE delta7 = 2.0 * (gain7out - gain7in) / iNumSamples;
    CSAMPLE gain7 = gain7in;
    const CSAMPLE delta8 = 2.0 * (gain8out - gain8in) / iNumSamples;
    CSAMPLE gain8 = gain8in;
    const CSAMPLE delta9 = 2.0 * (gain9out - gain9in) / iNumSamples;
    CSAMPLE gain9 = gain9in;
    const CSAMPLE delta10 = 2.0 * (gain10out - gain10in) / iNumSamples;
    CSAMPLE gain10 = gain10in;
    const CSAMPLE delta11 = 2.0 * (gain11out - gain11in) / iNumSamples;
    CSAMPLE gain11 = gain11in;
    const CSAMPLE delta12 = 2.0 * (gain12out - gain12in) / iNumSamples;
    CSAMPLE gain12 = gain12in;
    const CSAMPLE delta13 = 2.0 * (gain13out - gain13in) / iNumSamples;
    CSAMPLE gain13 = gain13in;
    const CSAMPLE delta14 = 2.0 * (gain14out - gain14in) / iNumSamples;
    CSAMPLE gain14 = gain14in;
    const CSAMPLE delta15 = 2.0 * (gain15out - gain15in) / iNumSamples;
    CSAMPLE gain15 = gain15in;
    const CSAMPLE delta16 = 2.0 * (gain16out - gain16in) / iNumSamples;
    CSAMPLE gain16 = gain16in;
    const CSAMPLE delta17 = 2.0 * (gain17out - gain17in) / iNumSamples;
    CSAMPLE gain17 = gain17in;
    const CSAMPLE delta18 = 2.0 * (gain18out - gain18in) / iNumSamples;
    CSAMPLE gain18 = gain18in;
    const CSAMPLE delta19 = 2.0 * (gain19out - gain19in) / iNumSamples;
    CSAMPLE gain19 = gain19in;
    const CSAMPLE delta20 = 2.0 * (gain20out - gain20in) / iNumSamples;
    CSAMPLE gain20 = gain20in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1, gain2 += delta2, gain3 += delta3, gain4 += delta4, gain5 += delta5, gain6 += delta6, gain7 += delta7, gain8 += delta8, gain9 += delta9, gain10 += delta10, gain11 += delta11, gain12 += delta12, gain13 += delta13, gain14 += delta14, gain15 += delta15, gain16 += delta16, gain17 += delta17, gain18 += delta18, gain19 += delta19, gain20 += delta20) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16 +
                   pSrc17[i] * gain17 +
                   pSrc18[i] * gain18 +
                   pSrc19[i] * gain19 +
                   pSrc20[i] * gain20;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1 +
                       pSrc2[i + 1] * gain2 +
                       pSrc3[i + 1] * gain3 +
                       pSrc4[i + 1] * gain4 +
                       pSrc5[i + 1] * gain5 +
                       pSrc6[i + 1] * gain6 +
                       pSrc7[i + 1] * gain7 +
                       pSrc8[i + 1] * gain8 +
                       pSrc9[i + 1] * gain9 +
                       pSrc10[i + 1] * gain10 +
                       pSrc11[i + 1] * gain11 +
                       pSrc12[i + 1] * gain12 +
                       pSrc13[i + 1] * gain13 +
                       pSrc14[i + 1] * gain14 +
                       pSrc15[i + 1] * gain15 +
                       pSrc16[i + 1] * gain16 +
                       pSrc17[i + 1] * gain17 +
                       pSrc18[i + 1] * gain18 +
                       pSrc19[i + 1] * gain19 +
                       pSrc20[i + 1] * gain20;
    }
}
static inline void copy22WithGain(CSAMPLE* pDest,
                                  const CSAMPLE* pSrc0, CSAMPLE gain0,
                                  const CSAMPLE* pSrc1, CSAMPLE gain1,
                                  const CSAMPLE* pSrc2, CSAMPLE gain2,
                                  const CSAMPLE* pSrc3, CSAMPLE gain3,
                                  const CSAMPLE* pSrc4, CSAMPLE gain4,
                                  const CSAMPLE* pSrc5, CSAMPLE gain5,
                                  const CSAMPLE* pSrc6, CSAMPLE gain6,
                                  const CSAMPLE* pSrc7, CSAMPLE gain7,
                                  const CSAMPLE* pSrc8, CSAMPLE gain8,
                                  const CSAMPLE* pSrc9, CSAMPLE gain9,
                                  const CSAMPLE* pSrc10, CSAMPLE gain10,
                                  const CSAMPLE* pSrc11, CSAMPLE gain11,
                                  const CSAMPLE* pSrc12, CSAMPLE gain12,
                                  const CSAMPLE* pSrc13, CSAMPLE gain13,
                                  const CSAMPLE* pSrc14, CSAMPLE gain14,
                                  const CSAMPLE* pSrc15, CSAMPLE gain15,
                                  const CSAMPLE* pSrc16, CSAMPLE gain16,
                                  const CSAMPLE* pSrc17, CSAMPLE gain17,
                                  const CSAMPLE* pSrc18, CSAMPLE gain18,
                                  const CSAMPLE* pSrc19, CSAMPLE gain19,
                                  const CSAMPLE* pSrc20, CSAMPLE gain20,
                                  const CSAMPLE* pSrc21, CSAMPLE gain21,
                                  int iNumSamples) {
    if (gain0 == 0.0) {
        copy21WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain2 == 0.0) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain3 == 0.0) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain4 == 0.0) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain5 == 0.0) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain6 == 0.0) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain7 == 0.0) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain8 == 0.0) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain9 == 0.0) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain10 == 0.0) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain11 == 0.0) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain12 == 0.0) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain13 == 0.0) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain14 == 0.0) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain15 == 0.0) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain16 == 0.0) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain17 == 0.0) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain18 == 0.0) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain19 == 0.0) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain20 == 0.0) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain21 == 0.0) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16 +
                   pSrc17[i] * gain17 +
                   pSrc18[i] * gain18 +
                   pSrc19[i] * gain19 +
                   pSrc20[i] * gain20 +
                   pSrc21[i] * gain21;
    }
}
static inline void copy22WithRampingGain(CSAMPLE* pDest,
                                         const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                         const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                         const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                                         const CSAMPLE* pSrc3, CSAMPLE gain3in, CSAMPLE gain3out,
                                         const CSAMPLE* pSrc4, CSAMPLE gain4in, CSAMPLE gain4out,
                                         const CSAMPLE* pSrc5, CSAMPLE gain5in, CSAMPLE gain5out,
                                         const CSAMPLE* pSrc6, CSAMPLE gain6in, CSAMPLE gain6out,
                                         const CSAMPLE* pSrc7, CSAMPLE gain7in, CSAMPLE gain7out,
                                         const CSAMPLE* pSrc8, CSAMPLE gain8in, CSAMPLE gain8out,
                                         const CSAMPLE* pSrc9, CSAMPLE gain9in, CSAMPLE gain9out,
                                         const CSAMPLE* pSrc10, CSAMPLE gain10in, CSAMPLE gain10out,
                                         const CSAMPLE* pSrc11, CSAMPLE gain11in, CSAMPLE gain11out,
                                         const CSAMPLE* pSrc12, CSAMPLE gain12in, CSAMPLE gain12out,
                                         const CSAMPLE* pSrc13, CSAMPLE gain13in, CSAMPLE gain13out,
                                         const CSAMPLE* pSrc14, CSAMPLE gain14in, CSAMPLE gain14out,
                                         const CSAMPLE* pSrc15, CSAMPLE gain15in, CSAMPLE gain15out,
                                         const CSAMPLE* pSrc16, CSAMPLE gain16in, CSAMPLE gain16out,
                                         const CSAMPLE* pSrc17, CSAMPLE gain17in, CSAMPLE gain17out,
                                         const CSAMPLE* pSrc18, CSAMPLE gain18in, CSAMPLE gain18out,
                                         const CSAMPLE* pSrc19, CSAMPLE gain19in, CSAMPLE gain19out,
                                         const CSAMPLE* pSrc20, CSAMPLE gain20in, CSAMPLE gain20out,
                                         const CSAMPLE* pSrc21, CSAMPLE gain21in, CSAMPLE gain21out,
                                         int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy21WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain2in == 0.0 && gain2out == 0.0) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain3in == 0.0 && gain3out == 0.0) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain4in == 0.0 && gain4out == 0.0) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain5in == 0.0 && gain5out == 0.0) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain6in == 0.0 && gain6out == 0.0) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain7in == 0.0 && gain7out == 0.0) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain8in == 0.0 && gain8out == 0.0) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain9in == 0.0 && gain9out == 0.0) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain10in == 0.0 && gain10out == 0.0) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain11in == 0.0 && gain11out == 0.0) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain12in == 0.0 && gain12out == 0.0) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain13in == 0.0 && gain13out == 0.0) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain14in == 0.0 && gain14out == 0.0) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain15in == 0.0 && gain15out == 0.0) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain16in == 0.0 && gain16out == 0.0) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain17in == 0.0 && gain17out == 0.0) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain18in == 0.0 && gain18out == 0.0) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain19in == 0.0 && gain19out == 0.0) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain20in == 0.0 && gain20out == 0.0) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain21in == 0.0 && gain21out == 0.0) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    CSAMPLE gain2 = gain2in;
    const CSAMPLE delta3 = 2.0 * (gain3out - gain3in) / iNumSamples;
    CSAMPLE gain3 = gain3in;
    const CSAMPLE delta4 = 2.0 * (gain4out - gain4in) / iNumSamples;
    CSAMPLE gain4 = gain4in;
    const CSAMPLE delta5 = 2.0 * (gain5out - gain5in) / iNumSamples;
    CSAMPLE gain5 = gain5in;
    const CSAMPLE delta6 = 2.0 * (gain6out - gain6in) / iNumSamples;
    CSAMPLE gain6 = gain6in;
    const CSAMPLE delta7 = 2.0 * (gain7out - gain7in) / iNumSamples;
    CSAMPLE gain7 = gain7in;
    const CSAMPLE delta8 = 2.0 * (gain8out - gain8in) / iNumSamples;
    CSAMPLE gain8 = gain8in;
    const CSAMPLE delta9 = 2.0 * (gain9out - gain9in) / iNumSamples;
    CSAMPLE gain9 = gain9in;
    const CSAMPLE delta10 = 2.0 * (gain10out - gain10in) / iNumSamples;
    CSAMPLE gain10 = gain10in;
    const CSAMPLE delta11 = 2.0 * (gain11out - gain11in) / iNumSamples;
    CSAMPLE gain11 = gain11in;
    const CSAMPLE delta12 = 2.0 * (gain12out - gain12in) / iNumSamples;
    CSAMPLE gain12 = gain12in;
    const CSAMPLE delta13 = 2.0 * (gain13out - gain13in) / iNumSamples;
    CSAMPLE gain13 = gain13in;
    const CSAMPLE delta14 = 2.0 * (gain14out - gain14in) / iNumSamples;
    CSAMPLE gain14 = gain14in;
    const CSAMPLE delta15 = 2.0 * (gain15out - gain15in) / iNumSamples;
    CSAMPLE gain15 = gain15in;
    const CSAMPLE delta16 = 2.0 * (gain16out - gain16in) / iNumSamples;
    CSAMPLE gain16 = gain16in;
    const CSAMPLE delta17 = 2.0 * (gain17out - gain17in) / iNumSamples;
    CSAMPLE gain17 = gain17in;
    const CSAMPLE delta18 = 2.0 * (gain18out - gain18in) / iNumSamples;
    CSAMPLE gain18 = gain18in;
    const CSAMPLE delta19 = 2.0 * (gain19out - gain19in) / iNumSamples;
    CSAMPLE gain19 = gain19in;
    const CSAMPLE delta20 = 2.0 * (gain20out - gain20in) / iNumSamples;
    CSAMPLE gain20 = gain20in;
    const CSAMPLE delta21 = 2.0 * (gain21out - gain21in) / iNumSamples;
    CSAMPLE gain21 = gain21in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1, gain2 += delta2, gain3 += delta3, gain4 += delta4, gain5 += delta5, gain6 += delta6, gain7 += delta7, gain8 += delta8, gain9 += delta9, gain10 += delta10, gain11 += delta11, gain12 += delta12, gain13 += delta13, gain14 += delta14, gain15 += delta15, gain16 += delta16, gain17 += delta17, gain18 += delta18, gain19 += delta19, gain20 += delta20, gain21 += delta21) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16 +
                   pSrc17[i] * gain17 +
                   pSrc18[i] * gain18 +
                   pSrc19[i] * gain19 +
                   pSrc20[i] * gain20 +
                   pSrc21[i] * gain21;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1 +
                       pSrc2[i + 1] * gain2 +
                       pSrc3[i + 1] * gain3 +
                       pSrc4[i + 1] * gain4 +
                       pSrc5[i + 1] * gain5 +
                       pSrc6[i + 1] * gain6 +
                       pSrc7[i + 1] * gain7 +
                       pSrc8[i + 1] * gain8 +
                       pSrc9[i + 1] * gain9 +
                       pSrc10[i + 1] * gain10 +
                       pSrc11[i + 1] * gain11 +
                       pSrc12[i + 1] * gain12 +
                       pSrc13[i + 1] * gain13 +
                       pSrc14[i + 1] * gain14 +
                       pSrc15[i + 1] * gain15 +
                       pSrc16[i + 1] * gain16 +
                       pSrc17[i + 1] * gain17 +
                       pSrc18[i + 1] * gain18 +
                       pSrc19[i + 1] * gain19 +
                       pSrc20[i + 1] * gain20 +
                       pSrc21[i + 1] * gain21;
    }
}
static inline void copy23WithGain(CSAMPLE* pDest,
                                  const CSAMPLE* pSrc0, CSAMPLE gain0,
                                  const CSAMPLE* pSrc1, CSAMPLE gain1,
                                  const CSAMPLE* pSrc2, CSAMPLE gain2,
                                  const CSAMPLE* pSrc3, CSAMPLE gain3,
                                  const CSAMPLE* pSrc4, CSAMPLE gain4,
                                  const CSAMPLE* pSrc5, CSAMPLE gain5,
                                  const CSAMPLE* pSrc6, CSAMPLE gain6,
                                  const CSAMPLE* pSrc7, CSAMPLE gain7,
                                  const CSAMPLE* pSrc8, CSAMPLE gain8,
                                  const CSAMPLE* pSrc9, CSAMPLE gain9,
                                  const CSAMPLE* pSrc10, CSAMPLE gain10,
                                  const CSAMPLE* pSrc11, CSAMPLE gain11,
                                  const CSAMPLE* pSrc12, CSAMPLE gain12,
                                  const CSAMPLE* pSrc13, CSAMPLE gain13,
                                  const CSAMPLE* pSrc14, CSAMPLE gain14,
                                  const CSAMPLE* pSrc15, CSAMPLE gain15,
                                  const CSAMPLE* pSrc16, CSAMPLE gain16,
                                  const CSAMPLE* pSrc17, CSAMPLE gain17,
                                  const CSAMPLE* pSrc18, CSAMPLE gain18,
                                  const CSAMPLE* pSrc19, CSAMPLE gain19,
                                  const CSAMPLE* pSrc20, CSAMPLE gain20,
                                  const CSAMPLE* pSrc21, CSAMPLE gain21,
                                  const CSAMPLE* pSrc22, CSAMPLE gain22,
                                  int iNumSamples) {
    if (gain0 == 0.0) {
        copy22WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain2 == 0.0) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain3 == 0.0) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain4 == 0.0) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain5 == 0.0) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain6 == 0.0) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain7 == 0.0) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain8 == 0.0) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain9 == 0.0) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain10 == 0.0) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain11 == 0.0) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain12 == 0.0) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain13 == 0.0) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain14 == 0.0) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain15 == 0.0) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain16 == 0.0) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain17 == 0.0) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain18 == 0.0) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain19 == 0.0) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain20 == 0.0) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain21 == 0.0) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain22 == 0.0) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16 +
                   pSrc17[i] * gain17 +
                   pSrc18[i] * gain18 +
                   pSrc19[i] * gain19 +
                   pSrc20[i] * gain20 +
                   pSrc21[i] * gain21 +
                   pSrc22[i] * gain22;
    }
}
static inline void copy23WithRampingGain(CSAMPLE* pDest,
                                         const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                         const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                         const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                                         const CSAMPLE* pSrc3, CSAMPLE gain3in, CSAMPLE gain3out,
                                         const CSAMPLE* pSrc4, CSAMPLE gain4in, CSAMPLE gain4out,
                                         const CSAMPLE* pSrc5, CSAMPLE gain5in, CSAMPLE gain5out,
                                         const CSAMPLE* pSrc6, CSAMPLE gain6in, CSAMPLE gain6out,
                                         const CSAMPLE* pSrc7, CSAMPLE gain7in, CSAMPLE gain7out,
                                         const CSAMPLE* pSrc8, CSAMPLE gain8in, CSAMPLE gain8out,
                                         const CSAMPLE* pSrc9, CSAMPLE gain9in, CSAMPLE gain9out,
                                         const CSAMPLE* pSrc10, CSAMPLE gain10in, CSAMPLE gain10out,
                                         const CSAMPLE* pSrc11, CSAMPLE gain11in, CSAMPLE gain11out,
                                         const CSAMPLE* pSrc12, CSAMPLE gain12in, CSAMPLE gain12out,
                                         const CSAMPLE* pSrc13, CSAMPLE gain13in, CSAMPLE gain13out,
                                         const CSAMPLE* pSrc14, CSAMPLE gain14in, CSAMPLE gain14out,
                                         const CSAMPLE* pSrc15, CSAMPLE gain15in, CSAMPLE gain15out,
                                         const CSAMPLE* pSrc16, CSAMPLE gain16in, CSAMPLE gain16out,
                                         const CSAMPLE* pSrc17, CSAMPLE gain17in, CSAMPLE gain17out,
                                         const CSAMPLE* pSrc18, CSAMPLE gain18in, CSAMPLE gain18out,
                                         const CSAMPLE* pSrc19, CSAMPLE gain19in, CSAMPLE gain19out,
                                         const CSAMPLE* pSrc20, CSAMPLE gain20in, CSAMPLE gain20out,
                                         const CSAMPLE* pSrc21, CSAMPLE gain21in, CSAMPLE gain21out,
                                         const CSAMPLE* pSrc22, CSAMPLE gain22in, CSAMPLE gain22out,
                                         int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy22WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain2in == 0.0 && gain2out == 0.0) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain3in == 0.0 && gain3out == 0.0) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain4in == 0.0 && gain4out == 0.0) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain5in == 0.0 && gain5out == 0.0) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain6in == 0.0 && gain6out == 0.0) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain7in == 0.0 && gain7out == 0.0) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain8in == 0.0 && gain8out == 0.0) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain9in == 0.0 && gain9out == 0.0) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain10in == 0.0 && gain10out == 0.0) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain11in == 0.0 && gain11out == 0.0) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain12in == 0.0 && gain12out == 0.0) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain13in == 0.0 && gain13out == 0.0) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain14in == 0.0 && gain14out == 0.0) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain15in == 0.0 && gain15out == 0.0) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain16in == 0.0 && gain16out == 0.0) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain17in == 0.0 && gain17out == 0.0) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain18in == 0.0 && gain18out == 0.0) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain19in == 0.0 && gain19out == 0.0) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain20in == 0.0 && gain20out == 0.0) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain21in == 0.0 && gain21out == 0.0) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain22in == 0.0 && gain22out == 0.0) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    CSAMPLE gain2 = gain2in;
    const CSAMPLE delta3 = 2.0 * (gain3out - gain3in) / iNumSamples;
    CSAMPLE gain3 = gain3in;
    const CSAMPLE delta4 = 2.0 * (gain4out - gain4in) / iNumSamples;
    CSAMPLE gain4 = gain4in;
    const CSAMPLE delta5 = 2.0 * (gain5out - gain5in) / iNumSamples;
    CSAMPLE gain5 = gain5in;
    const CSAMPLE delta6 = 2.0 * (gain6out - gain6in) / iNumSamples;
    CSAMPLE gain6 = gain6in;
    const CSAMPLE delta7 = 2.0 * (gain7out - gain7in) / iNumSamples;
    CSAMPLE gain7 = gain7in;
    const CSAMPLE delta8 = 2.0 * (gain8out - gain8in) / iNumSamples;
    CSAMPLE gain8 = gain8in;
    const CSAMPLE delta9 = 2.0 * (gain9out - gain9in) / iNumSamples;
    CSAMPLE gain9 = gain9in;
    const CSAMPLE delta10 = 2.0 * (gain10out - gain10in) / iNumSamples;
    CSAMPLE gain10 = gain10in;
    const CSAMPLE delta11 = 2.0 * (gain11out - gain11in) / iNumSamples;
    CSAMPLE gain11 = gain11in;
    const CSAMPLE delta12 = 2.0 * (gain12out - gain12in) / iNumSamples;
    CSAMPLE gain12 = gain12in;
    const CSAMPLE delta13 = 2.0 * (gain13out - gain13in) / iNumSamples;
    CSAMPLE gain13 = gain13in;
    const CSAMPLE delta14 = 2.0 * (gain14out - gain14in) / iNumSamples;
    CSAMPLE gain14 = gain14in;
    const CSAMPLE delta15 = 2.0 * (gain15out - gain15in) / iNumSamples;
    CSAMPLE gain15 = gain15in;
    const CSAMPLE delta16 = 2.0 * (gain16out - gain16in) / iNumSamples;
    CSAMPLE gain16 = gain16in;
    const CSAMPLE delta17 = 2.0 * (gain17out - gain17in) / iNumSamples;
    CSAMPLE gain17 = gain17in;
    const CSAMPLE delta18 = 2.0 * (gain18out - gain18in) / iNumSamples;
    CSAMPLE gain18 = gain18in;
    const CSAMPLE delta19 = 2.0 * (gain19out - gain19in) / iNumSamples;
    CSAMPLE gain19 = gain19in;
    const CSAMPLE delta20 = 2.0 * (gain20out - gain20in) / iNumSamples;
    CSAMPLE gain20 = gain20in;
    const CSAMPLE delta21 = 2.0 * (gain21out - gain21in) / iNumSamples;
    CSAMPLE gain21 = gain21in;
    const CSAMPLE delta22 = 2.0 * (gain22out - gain22in) / iNumSamples;
    CSAMPLE gain22 = gain22in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1, gain2 += delta2, gain3 += delta3, gain4 += delta4, gain5 += delta5, gain6 += delta6, gain7 += delta7, gain8 += delta8, gain9 += delta9, gain10 += delta10, gain11 += delta11, gain12 += delta12, gain13 += delta13, gain14 += delta14, gain15 += delta15, gain16 += delta16, gain17 += delta17, gain18 += delta18, gain19 += delta19, gain20 += delta20, gain21 += delta21, gain22 += delta22) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16 +
                   pSrc17[i] * gain17 +
                   pSrc18[i] * gain18 +
                   pSrc19[i] * gain19 +
                   pSrc20[i] * gain20 +
                   pSrc21[i] * gain21 +
                   pSrc22[i] * gain22;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1 +
                       pSrc2[i + 1] * gain2 +
                       pSrc3[i + 1] * gain3 +
                       pSrc4[i + 1] * gain4 +
                       pSrc5[i + 1] * gain5 +
                       pSrc6[i + 1] * gain6 +
                       pSrc7[i + 1] * gain7 +
                       pSrc8[i + 1] * gain8 +
                       pSrc9[i + 1] * gain9 +
                       pSrc10[i + 1] * gain10 +
                       pSrc11[i + 1] * gain11 +
                       pSrc12[i + 1] * gain12 +
                       pSrc13[i + 1] * gain13 +
                       pSrc14[i + 1] * gain14 +
                       pSrc15[i + 1] * gain15 +
                       pSrc16[i + 1] * gain16 +
                       pSrc17[i + 1] * gain17 +
                       pSrc18[i + 1] * gain18 +
                       pSrc19[i + 1] * gain19 +
                       pSrc20[i + 1] * gain20 +
                       pSrc21[i + 1] * gain21 +
                       pSrc22[i + 1] * gain22;
    }
}
static inline void copy24WithGain(CSAMPLE* pDest,
                                  const CSAMPLE* pSrc0, CSAMPLE gain0,
                                  const CSAMPLE* pSrc1, CSAMPLE gain1,
                                  const CSAMPLE* pSrc2, CSAMPLE gain2,
                                  const CSAMPLE* pSrc3, CSAMPLE gain3,
                                  const CSAMPLE* pSrc4, CSAMPLE gain4,
                                  const CSAMPLE* pSrc5, CSAMPLE gain5,
                                  const CSAMPLE* pSrc6, CSAMPLE gain6,
                                  const CSAMPLE* pSrc7, CSAMPLE gain7,
                                  const CSAMPLE* pSrc8, CSAMPLE gain8,
                                  const CSAMPLE* pSrc9, CSAMPLE gain9,
                                  const CSAMPLE* pSrc10, CSAMPLE gain10,
                                  const CSAMPLE* pSrc11, CSAMPLE gain11,
                                  const CSAMPLE* pSrc12, CSAMPLE gain12,
                                  const CSAMPLE* pSrc13, CSAMPLE gain13,
                                  const CSAMPLE* pSrc14, CSAMPLE gain14,
                                  const CSAMPLE* pSrc15, CSAMPLE gain15,
                                  const CSAMPLE* pSrc16, CSAMPLE gain16,
                                  const CSAMPLE* pSrc17, CSAMPLE gain17,
                                  const CSAMPLE* pSrc18, CSAMPLE gain18,
                                  const CSAMPLE* pSrc19, CSAMPLE gain19,
                                  const CSAMPLE* pSrc20, CSAMPLE gain20,
                                  const CSAMPLE* pSrc21, CSAMPLE gain21,
                                  const CSAMPLE* pSrc22, CSAMPLE gain22,
                                  const CSAMPLE* pSrc23, CSAMPLE gain23,
                                  int iNumSamples) {
    if (gain0 == 0.0) {
        copy23WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain2 == 0.0) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain3 == 0.0) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain4 == 0.0) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain5 == 0.0) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain6 == 0.0) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain7 == 0.0) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain8 == 0.0) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain9 == 0.0) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain10 == 0.0) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain11 == 0.0) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain12 == 0.0) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain13 == 0.0) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain14 == 0.0) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain15 == 0.0) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain16 == 0.0) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain17 == 0.0) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain18 == 0.0) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain19 == 0.0) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain20 == 0.0) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain21 == 0.0) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain22 == 0.0) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain23 == 0.0) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16 +
                   pSrc17[i] * gain17 +
                   pSrc18[i] * gain18 +
                   pSrc19[i] * gain19 +
                   pSrc20[i] * gain20 +
                   pSrc21[i] * gain21 +
                   pSrc22[i] * gain22 +
                   pSrc23[i] * gain23;
    }
}
static inline void copy24WithRampingGain(CSAMPLE* pDest,
                                         const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                         const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                         const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                                         const CSAMPLE* pSrc3, CSAMPLE gain3in, CSAMPLE gain3out,
                                         const CSAMPLE* pSrc4, CSAMPLE gain4in, CSAMPLE gain4out,
                                         const CSAMPLE* pSrc5, CSAMPLE gain5in, CSAMPLE gain5out,
                                         const CSAMPLE* pSrc6, CSAMPLE gain6in, CSAMPLE gain6out,
                                         const CSAMPLE* pSrc7, CSAMPLE gain7in, CSAMPLE gain7out,
                                         const CSAMPLE* pSrc8, CSAMPLE gain8in, CSAMPLE gain8out,
                                         const CSAMPLE* pSrc9, CSAMPLE gain9in, CSAMPLE gain9out,
                                         const CSAMPLE* pSrc10, CSAMPLE gain10in, CSAMPLE gain10out,
                                         const CSAMPLE* pSrc11, CSAMPLE gain11in, CSAMPLE gain11out,
                                         const CSAMPLE* pSrc12, CSAMPLE gain12in, CSAMPLE gain12out,
                                         const CSAMPLE* pSrc13, CSAMPLE gain13in, CSAMPLE gain13out,
                                         const CSAMPLE* pSrc14, CSAMPLE gain14in, CSAMPLE gain14out,
                                         const CSAMPLE* pSrc15, CSAMPLE gain15in, CSAMPLE gain15out,
                                         const CSAMPLE* pSrc16, CSAMPLE gain16in, CSAMPLE gain16out,
                                         const CSAMPLE* pSrc17, CSAMPLE gain17in, CSAMPLE gain17out,
                                         const CSAMPLE* pSrc18, CSAMPLE gain18in, CSAMPLE gain18out,
                                         const CSAMPLE* pSrc19, CSAMPLE gain19in, CSAMPLE gain19out,
                                         const CSAMPLE* pSrc20, CSAMPLE gain20in, CSAMPLE gain20out,
                                         const CSAMPLE* pSrc21, CSAMPLE gain21in, CSAMPLE gain21out,
                                         const CSAMPLE* pSrc22, CSAMPLE gain22in, CSAMPLE gain22out,
                                         const CSAMPLE* pSrc23, CSAMPLE gain23in, CSAMPLE gain23out,
                                         int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy23WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain2in == 0.0 && gain2out == 0.0) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain3in == 0.0 && gain3out == 0.0) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain4in == 0.0 && gain4out == 0.0) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain5in == 0.0 && gain5out == 0.0) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain6in == 0.0 && gain6out == 0.0) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain7in == 0.0 && gain7out == 0.0) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain8in == 0.0 && gain8out == 0.0) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain9in == 0.0 && gain9out == 0.0) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain10in == 0.0 && gain10out == 0.0) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain11in == 0.0 && gain11out == 0.0) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain12in == 0.0 && gain12out == 0.0) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain13in == 0.0 && gain13out == 0.0) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain14in == 0.0 && gain14out == 0.0) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain15in == 0.0 && gain15out == 0.0) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain16in == 0.0 && gain16out == 0.0) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain17in == 0.0 && gain17out == 0.0) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain18in == 0.0 && gain18out == 0.0) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain19in == 0.0 && gain19out == 0.0) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain20in == 0.0 && gain20out == 0.0) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain21in == 0.0 && gain21out == 0.0) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain22in == 0.0 && gain22out == 0.0) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain23in == 0.0 && gain23out == 0.0) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    CSAMPLE gain2 = gain2in;
    const CSAMPLE delta3 = 2.0 * (gain3out - gain3in) / iNumSamples;
    CSAMPLE gain3 = gain3in;
    const CSAMPLE delta4 = 2.0 * (gain4out - gain4in) / iNumSamples;
    CSAMPLE gain4 = gain4in;
    const CSAMPLE delta5 = 2.0 * (gain5out - gain5in) / iNumSamples;
    CSAMPLE gain5 = gain5in;
    const CSAMPLE delta6 = 2.0 * (gain6out - gain6in) / iNumSamples;
    CSAMPLE gain6 = gain6in;
    const CSAMPLE delta7 = 2.0 * (gain7out - gain7in) / iNumSamples;
    CSAMPLE gain7 = gain7in;
    const CSAMPLE delta8 = 2.0 * (gain8out - gain8in) / iNumSamples;
    CSAMPLE gain8 = gain8in;
    const CSAMPLE delta9 = 2.0 * (gain9out - gain9in) / iNumSamples;
    CSAMPLE gain9 = gain9in;
    const CSAMPLE delta10 = 2.0 * (gain10out - gain10in) / iNumSamples;
    CSAMPLE gain10 = gain10in;
    const CSAMPLE delta11 = 2.0 * (gain11out - gain11in) / iNumSamples;
    CSAMPLE gain11 = gain11in;
    const CSAMPLE delta12 = 2.0 * (gain12out - gain12in) / iNumSamples;
    CSAMPLE gain12 = gain12in;
    const CSAMPLE delta13 = 2.0 * (gain13out - gain13in) / iNumSamples;
    CSAMPLE gain13 = gain13in;
    const CSAMPLE delta14 = 2.0 * (gain14out - gain14in) / iNumSamples;
    CSAMPLE gain14 = gain14in;
    const CSAMPLE delta15 = 2.0 * (gain15out - gain15in) / iNumSamples;
    CSAMPLE gain15 = gain15in;
    const CSAMPLE delta16 = 2.0 * (gain16out - gain16in) / iNumSamples;
    CSAMPLE gain16 = gain16in;
    const CSAMPLE delta17 = 2.0 * (gain17out - gain17in) / iNumSamples;
    CSAMPLE gain17 = gain17in;
    const CSAMPLE delta18 = 2.0 * (gain18out - gain18in) / iNumSamples;
    CSAMPLE gain18 = gain18in;
    const CSAMPLE delta19 = 2.0 * (gain19out - gain19in) / iNumSamples;
    CSAMPLE gain19 = gain19in;
    const CSAMPLE delta20 = 2.0 * (gain20out - gain20in) / iNumSamples;
    CSAMPLE gain20 = gain20in;
    const CSAMPLE delta21 = 2.0 * (gain21out - gain21in) / iNumSamples;
    CSAMPLE gain21 = gain21in;
    const CSAMPLE delta22 = 2.0 * (gain22out - gain22in) / iNumSamples;
    CSAMPLE gain22 = gain22in;
    const CSAMPLE delta23 = 2.0 * (gain23out - gain23in) / iNumSamples;
    CSAMPLE gain23 = gain23in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1, gain2 += delta2, gain3 += delta3, gain4 += delta4, gain5 += delta5, gain6 += delta6, gain7 += delta7, gain8 += delta8, gain9 += delta9, gain10 += delta10, gain11 += delta11, gain12 += delta12, gain13 += delta13, gain14 += delta14, gain15 += delta15, gain16 += delta16, gain17 += delta17, gain18 += delta18, gain19 += delta19, gain20 += delta20, gain21 += delta21, gain22 += delta22, gain23 += delta23) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16 +
                   pSrc17[i] * gain17 +
                   pSrc18[i] * gain18 +
                   pSrc19[i] * gain19 +
                   pSrc20[i] * gain20 +
                   pSrc21[i] * gain21 +
                   pSrc22[i] * gain22 +
                   pSrc23[i] * gain23;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1 +
                       pSrc2[i + 1] * gain2 +
                       pSrc3[i + 1] * gain3 +
                       pSrc4[i + 1] * gain4 +
                       pSrc5[i + 1] * gain5 +
                       pSrc6[i + 1] * gain6 +
                       pSrc7[i + 1] * gain7 +
                       pSrc8[i + 1] * gain8 +
                       pSrc9[i + 1] * gain9 +
                       pSrc10[i + 1] * gain10 +
                       pSrc11[i + 1] * gain11 +
                       pSrc12[i + 1] * gain12 +
                       pSrc13[i + 1] * gain13 +
                       pSrc14[i + 1] * gain14 +
                       pSrc15[i + 1] * gain15 +
                       pSrc16[i + 1] * gain16 +
                       pSrc17[i + 1] * gain17 +
                       pSrc18[i + 1] * gain18 +
                       pSrc19[i + 1] * gain19 +
                       pSrc20[i + 1] * gain20 +
                       pSrc21[i + 1] * gain21 +
                       pSrc22[i + 1] * gain22 +
                       pSrc23[i + 1] * gain23;
    }
}
static inline void copy25WithGain(CSAMPLE* pDest,
                                  const CSAMPLE* pSrc0, CSAMPLE gain0,
                                  const CSAMPLE* pSrc1, CSAMPLE gain1,
                                  const CSAMPLE* pSrc2, CSAMPLE gain2,
                                  const CSAMPLE* pSrc3, CSAMPLE gain3,
                                  const CSAMPLE* pSrc4, CSAMPLE gain4,
                                  const CSAMPLE* pSrc5, CSAMPLE gain5,
                                  const CSAMPLE* pSrc6, CSAMPLE gain6,
                                  const CSAMPLE* pSrc7, CSAMPLE gain7,
                                  const CSAMPLE* pSrc8, CSAMPLE gain8,
                                  const CSAMPLE* pSrc9, CSAMPLE gain9,
                                  const CSAMPLE* pSrc10, CSAMPLE gain10,
                                  const CSAMPLE* pSrc11, CSAMPLE gain11,
                                  const CSAMPLE* pSrc12, CSAMPLE gain12,
                                  const CSAMPLE* pSrc13, CSAMPLE gain13,
                                  const CSAMPLE* pSrc14, CSAMPLE gain14,
                                  const CSAMPLE* pSrc15, CSAMPLE gain15,
                                  const CSAMPLE* pSrc16, CSAMPLE gain16,
                                  const CSAMPLE* pSrc17, CSAMPLE gain17,
                                  const CSAMPLE* pSrc18, CSAMPLE gain18,
                                  const CSAMPLE* pSrc19, CSAMPLE gain19,
                                  const CSAMPLE* pSrc20, CSAMPLE gain20,
                                  const CSAMPLE* pSrc21, CSAMPLE gain21,
                                  const CSAMPLE* pSrc22, CSAMPLE gain22,
                                  const CSAMPLE* pSrc23, CSAMPLE gain23,
                                  const CSAMPLE* pSrc24, CSAMPLE gain24,
                                  int iNumSamples) {
    if (gain0 == 0.0) {
        copy24WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain2 == 0.0) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain3 == 0.0) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain4 == 0.0) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain5 == 0.0) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain6 == 0.0) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain7 == 0.0) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain8 == 0.0) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain9 == 0.0) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain10 == 0.0) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain11 == 0.0) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain12 == 0.0) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain13 == 0.0) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain14 == 0.0) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain15 == 0.0) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain16 == 0.0) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain17 == 0.0) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain18 == 0.0) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain19 == 0.0) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain20 == 0.0) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain21 == 0.0) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain22 == 0.0) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain23 == 0.0) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain24 == 0.0) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16 +
                   pSrc17[i] * gain17 +
                   pSrc18[i] * gain18 +
                   pSrc19[i] * gain19 +
                   pSrc20[i] * gain20 +
                   pSrc21[i] * gain21 +
                   pSrc22[i] * gain22 +
                   pSrc23[i] * gain23 +
                   pSrc24[i] * gain24;
    }
}
static inline void copy25WithRampingGain(CSAMPLE* pDest,
                                         const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                         const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                         const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                                         const CSAMPLE* pSrc3, CSAMPLE gain3in, CSAMPLE gain3out,
                                         const CSAMPLE* pSrc4, CSAMPLE gain4in, CSAMPLE gain4out,
                                         const CSAMPLE* pSrc5, CSAMPLE gain5in, CSAMPLE gain5out,
                                         const CSAMPLE* pSrc6, CSAMPLE gain6in, CSAMPLE gain6out,
                                         const CSAMPLE* pSrc7, CSAMPLE gain7in, CSAMPLE gain7out,
                                         const CSAMPLE* pSrc8, CSAMPLE gain8in, CSAMPLE gain8out,
                                         const CSAMPLE* pSrc9, CSAMPLE gain9in, CSAMPLE gain9out,
                                         const CSAMPLE* pSrc10, CSAMPLE gain10in, CSAMPLE gain10out,
                                         const CSAMPLE* pSrc11, CSAMPLE gain11in, CSAMPLE gain11out,
                                         const CSAMPLE* pSrc12, CSAMPLE gain12in, CSAMPLE gain12out,
                                         const CSAMPLE* pSrc13, CSAMPLE gain13in, CSAMPLE gain13out,
                                         const CSAMPLE* pSrc14, CSAMPLE gain14in, CSAMPLE gain14out,
                                         const CSAMPLE* pSrc15, CSAMPLE gain15in, CSAMPLE gain15out,
                                         const CSAMPLE* pSrc16, CSAMPLE gain16in, CSAMPLE gain16out,
                                         const CSAMPLE* pSrc17, CSAMPLE gain17in, CSAMPLE gain17out,
                                         const CSAMPLE* pSrc18, CSAMPLE gain18in, CSAMPLE gain18out,
                                         const CSAMPLE* pSrc19, CSAMPLE gain19in, CSAMPLE gain19out,
                                         const CSAMPLE* pSrc20, CSAMPLE gain20in, CSAMPLE gain20out,
                                         const CSAMPLE* pSrc21, CSAMPLE gain21in, CSAMPLE gain21out,
                                         const CSAMPLE* pSrc22, CSAMPLE gain22in, CSAMPLE gain22out,
                                         const CSAMPLE* pSrc23, CSAMPLE gain23in, CSAMPLE gain23out,
                                         const CSAMPLE* pSrc24, CSAMPLE gain24in, CSAMPLE gain24out,
                                         int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy24WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain2in == 0.0 && gain2out == 0.0) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain3in == 0.0 && gain3out == 0.0) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain4in == 0.0 && gain4out == 0.0) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain5in == 0.0 && gain5out == 0.0) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain6in == 0.0 && gain6out == 0.0) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain7in == 0.0 && gain7out == 0.0) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain8in == 0.0 && gain8out == 0.0) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain9in == 0.0 && gain9out == 0.0) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain10in == 0.0 && gain10out == 0.0) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain11in == 0.0 && gain11out == 0.0) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain12in == 0.0 && gain12out == 0.0) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain13in == 0.0 && gain13out == 0.0) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain14in == 0.0 && gain14out == 0.0) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain15in == 0.0 && gain15out == 0.0) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain16in == 0.0 && gain16out == 0.0) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain17in == 0.0 && gain17out == 0.0) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain18in == 0.0 && gain18out == 0.0) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain19in == 0.0 && gain19out == 0.0) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain20in == 0.0 && gain20out == 0.0) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain21in == 0.0 && gain21out == 0.0) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain22in == 0.0 && gain22out == 0.0) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain23in == 0.0 && gain23out == 0.0) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain24in == 0.0 && gain24out == 0.0) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    CSAMPLE gain2 = gain2in;
    const CSAMPLE delta3 = 2.0 * (gain3out - gain3in) / iNumSamples;
    CSAMPLE gain3 = gain3in;
    const CSAMPLE delta4 = 2.0 * (gain4out - gain4in) / iNumSamples;
    CSAMPLE gain4 = gain4in;
    const CSAMPLE delta5 = 2.0 * (gain5out - gain5in) / iNumSamples;
    CSAMPLE gain5 = gain5in;
    const CSAMPLE delta6 = 2.0 * (gain6out - gain6in) / iNumSamples;
    CSAMPLE gain6 = gain6in;
    const CSAMPLE delta7 = 2.0 * (gain7out - gain7in) / iNumSamples;
    CSAMPLE gain7 = gain7in;
    const CSAMPLE delta8 = 2.0 * (gain8out - gain8in) / iNumSamples;
    CSAMPLE gain8 = gain8in;
    const CSAMPLE delta9 = 2.0 * (gain9out - gain9in) / iNumSamples;
    CSAMPLE gain9 = gain9in;
    const CSAMPLE delta10 = 2.0 * (gain10out - gain10in) / iNumSamples;
    CSAMPLE gain10 = gain10in;
    const CSAMPLE delta11 = 2.0 * (gain11out - gain11in) / iNumSamples;
    CSAMPLE gain11 = gain11in;
    const CSAMPLE delta12 = 2.0 * (gain12out - gain12in) / iNumSamples;
    CSAMPLE gain12 = gain12in;
    const CSAMPLE delta13 = 2.0 * (gain13out - gain13in) / iNumSamples;
    CSAMPLE gain13 = gain13in;
    const CSAMPLE delta14 = 2.0 * (gain14out - gain14in) / iNumSamples;
    CSAMPLE gain14 = gain14in;
    const CSAMPLE delta15 = 2.0 * (gain15out - gain15in) / iNumSamples;
    CSAMPLE gain15 = gain15in;
    const CSAMPLE delta16 = 2.0 * (gain16out - gain16in) / iNumSamples;
    CSAMPLE gain16 = gain16in;
    const CSAMPLE delta17 = 2.0 * (gain17out - gain17in) / iNumSamples;
    CSAMPLE gain17 = gain17in;
    const CSAMPLE delta18 = 2.0 * (gain18out - gain18in) / iNumSamples;
    CSAMPLE gain18 = gain18in;
    const CSAMPLE delta19 = 2.0 * (gain19out - gain19in) / iNumSamples;
    CSAMPLE gain19 = gain19in;
    const CSAMPLE delta20 = 2.0 * (gain20out - gain20in) / iNumSamples;
    CSAMPLE gain20 = gain20in;
    const CSAMPLE delta21 = 2.0 * (gain21out - gain21in) / iNumSamples;
    CSAMPLE gain21 = gain21in;
    const CSAMPLE delta22 = 2.0 * (gain22out - gain22in) / iNumSamples;
    CSAMPLE gain22 = gain22in;
    const CSAMPLE delta23 = 2.0 * (gain23out - gain23in) / iNumSamples;
    CSAMPLE gain23 = gain23in;
    const CSAMPLE delta24 = 2.0 * (gain24out - gain24in) / iNumSamples;
    CSAMPLE gain24 = gain24in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1, gain2 += delta2, gain3 += delta3, gain4 += delta4, gain5 += delta5, gain6 += delta6, gain7 += delta7, gain8 += delta8, gain9 += delta9, gain10 += delta10, gain11 += delta11, gain12 += delta12, gain13 += delta13, gain14 += delta14, gain15 += delta15, gain16 += delta16, gain17 += delta17, gain18 += delta18, gain19 += delta19, gain20 += delta20, gain21 += delta21, gain22 += delta22, gain23 += delta23, gain24 += delta24) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16 +
                   pSrc17[i] * gain17 +
                   pSrc18[i] * gain18 +
                   pSrc19[i] * gain19 +
                   pSrc20[i] * gain20 +
                   pSrc21[i] * gain21 +
                   pSrc22[i] * gain22 +
                   pSrc23[i] * gain23 +
                   pSrc24[i] * gain24;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1 +
                       pSrc2[i + 1] * gain2 +
                       pSrc3[i + 1] * gain3 +
                       pSrc4[i + 1] * gain4 +
                       pSrc5[i + 1] * gain5 +
                       pSrc6[i + 1] * gain6 +
                       pSrc7[i + 1] * gain7 +
                       pSrc8[i + 1] * gain8 +
                       pSrc9[i + 1] * gain9 +
                       pSrc10[i + 1] * gain10 +
                       pSrc11[i + 1] * gain11 +
                       pSrc12[i + 1] * gain12 +
                       pSrc13[i + 1] * gain13 +
                       pSrc14[i + 1] * gain14 +
                       pSrc15[i + 1] * gain15 +
                       pSrc16[i + 1] * gain16 +
                       pSrc17[i + 1] * gain17 +
                       pSrc18[i + 1] * gain18 +
                       pSrc19[i + 1] * gain19 +
                       pSrc20[i + 1] * gain20 +
                       pSrc21[i + 1] * gain21 +
                       pSrc22[i + 1] * gain22 +
                       pSrc23[i + 1] * gain23 +
                       pSrc24[i + 1] * gain24;
    }
}
static inline void copy26WithGain(CSAMPLE* pDest,
                                  const CSAMPLE* pSrc0, CSAMPLE gain0,
                                  const CSAMPLE* pSrc1, CSAMPLE gain1,
                                  const CSAMPLE* pSrc2, CSAMPLE gain2,
                                  const CSAMPLE* pSrc3, CSAMPLE gain3,
                                  const CSAMPLE* pSrc4, CSAMPLE gain4,
                                  const CSAMPLE* pSrc5, CSAMPLE gain5,
                                  const CSAMPLE* pSrc6, CSAMPLE gain6,
                                  const CSAMPLE* pSrc7, CSAMPLE gain7,
                                  const CSAMPLE* pSrc8, CSAMPLE gain8,
                                  const CSAMPLE* pSrc9, CSAMPLE gain9,
                                  const CSAMPLE* pSrc10, CSAMPLE gain10,
                                  const CSAMPLE* pSrc11, CSAMPLE gain11,
                                  const CSAMPLE* pSrc12, CSAMPLE gain12,
                                  const CSAMPLE* pSrc13, CSAMPLE gain13,
                                  const CSAMPLE* pSrc14, CSAMPLE gain14,
                                  const CSAMPLE* pSrc15, CSAMPLE gain15,
                                  const CSAMPLE* pSrc16, CSAMPLE gain16,
                                  const CSAMPLE* pSrc17, CSAMPLE gain17,
                                  const CSAMPLE* pSrc18, CSAMPLE gain18,
                                  const CSAMPLE* pSrc19, CSAMPLE gain19,
                                  const CSAMPLE* pSrc20, CSAMPLE gain20,
                                  const CSAMPLE* pSrc21, CSAMPLE gain21,
                                  const CSAMPLE* pSrc22, CSAMPLE gain22,
                                  const CSAMPLE* pSrc23, CSAMPLE gain23,
                                  const CSAMPLE* pSrc24, CSAMPLE gain24,
                                  const CSAMPLE* pSrc25, CSAMPLE gain25,
                                  int iNumSamples) {
    if (gain0 == 0.0) {
        copy25WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain2 == 0.0) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain3 == 0.0) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain4 == 0.0) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain5 == 0.0) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain6 == 0.0) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain7 == 0.0) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain8 == 0.0) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain9 == 0.0) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain10 == 0.0) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain11 == 0.0) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain12 == 0.0) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain13 == 0.0) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain14 == 0.0) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain15 == 0.0) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain16 == 0.0) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain17 == 0.0) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain18 == 0.0) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain19 == 0.0) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain20 == 0.0) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain21 == 0.0) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain22 == 0.0) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain23 == 0.0) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain24 == 0.0) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain25 == 0.0) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16 +
                   pSrc17[i] * gain17 +
                   pSrc18[i] * gain18 +
                   pSrc19[i] * gain19 +
                   pSrc20[i] * gain20 +
                   pSrc21[i] * gain21 +
                   pSrc22[i] * gain22 +
                   pSrc23[i] * gain23 +
                   pSrc24[i] * gain24 +
                   pSrc25[i] * gain25;
    }
}
static inline void copy26WithRampingGain(CSAMPLE* pDest,
                                         const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                         const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                         const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                                         const CSAMPLE* pSrc3, CSAMPLE gain3in, CSAMPLE gain3out,
                                         const CSAMPLE* pSrc4, CSAMPLE gain4in, CSAMPLE gain4out,
                                         const CSAMPLE* pSrc5, CSAMPLE gain5in, CSAMPLE gain5out,
                                         const CSAMPLE* pSrc6, CSAMPLE gain6in, CSAMPLE gain6out,
                                         const CSAMPLE* pSrc7, CSAMPLE gain7in, CSAMPLE gain7out,
                                         const CSAMPLE* pSrc8, CSAMPLE gain8in, CSAMPLE gain8out,
                                         const CSAMPLE* pSrc9, CSAMPLE gain9in, CSAMPLE gain9out,
                                         const CSAMPLE* pSrc10, CSAMPLE gain10in, CSAMPLE gain10out,
                                         const CSAMPLE* pSrc11, CSAMPLE gain11in, CSAMPLE gain11out,
                                         const CSAMPLE* pSrc12, CSAMPLE gain12in, CSAMPLE gain12out,
                                         const CSAMPLE* pSrc13, CSAMPLE gain13in, CSAMPLE gain13out,
                                         const CSAMPLE* pSrc14, CSAMPLE gain14in, CSAMPLE gain14out,
                                         const CSAMPLE* pSrc15, CSAMPLE gain15in, CSAMPLE gain15out,
                                         const CSAMPLE* pSrc16, CSAMPLE gain16in, CSAMPLE gain16out,
                                         const CSAMPLE* pSrc17, CSAMPLE gain17in, CSAMPLE gain17out,
                                         const CSAMPLE* pSrc18, CSAMPLE gain18in, CSAMPLE gain18out,
                                         const CSAMPLE* pSrc19, CSAMPLE gain19in, CSAMPLE gain19out,
                                         const CSAMPLE* pSrc20, CSAMPLE gain20in, CSAMPLE gain20out,
                                         const CSAMPLE* pSrc21, CSAMPLE gain21in, CSAMPLE gain21out,
                                         const CSAMPLE* pSrc22, CSAMPLE gain22in, CSAMPLE gain22out,
                                         const CSAMPLE* pSrc23, CSAMPLE gain23in, CSAMPLE gain23out,
                                         const CSAMPLE* pSrc24, CSAMPLE gain24in, CSAMPLE gain24out,
                                         const CSAMPLE* pSrc25, CSAMPLE gain25in, CSAMPLE gain25out,
                                         int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy25WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain2in == 0.0 && gain2out == 0.0) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain3in == 0.0 && gain3out == 0.0) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain4in == 0.0 && gain4out == 0.0) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain5in == 0.0 && gain5out == 0.0) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain6in == 0.0 && gain6out == 0.0) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain7in == 0.0 && gain7out == 0.0) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain8in == 0.0 && gain8out == 0.0) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain9in == 0.0 && gain9out == 0.0) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain10in == 0.0 && gain10out == 0.0) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain11in == 0.0 && gain11out == 0.0) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain12in == 0.0 && gain12out == 0.0) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain13in == 0.0 && gain13out == 0.0) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain14in == 0.0 && gain14out == 0.0) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain15in == 0.0 && gain15out == 0.0) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain16in == 0.0 && gain16out == 0.0) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain17in == 0.0 && gain17out == 0.0) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain18in == 0.0 && gain18out == 0.0) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain19in == 0.0 && gain19out == 0.0) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain20in == 0.0 && gain20out == 0.0) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain21in == 0.0 && gain21out == 0.0) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain22in == 0.0 && gain22out == 0.0) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain23in == 0.0 && gain23out == 0.0) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain24in == 0.0 && gain24out == 0.0) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain25in == 0.0 && gain25out == 0.0) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    CSAMPLE gain2 = gain2in;
    const CSAMPLE delta3 = 2.0 * (gain3out - gain3in) / iNumSamples;
    CSAMPLE gain3 = gain3in;
    const CSAMPLE delta4 = 2.0 * (gain4out - gain4in) / iNumSamples;
    CSAMPLE gain4 = gain4in;
    const CSAMPLE delta5 = 2.0 * (gain5out - gain5in) / iNumSamples;
    CSAMPLE gain5 = gain5in;
    const CSAMPLE delta6 = 2.0 * (gain6out - gain6in) / iNumSamples;
    CSAMPLE gain6 = gain6in;
    const CSAMPLE delta7 = 2.0 * (gain7out - gain7in) / iNumSamples;
    CSAMPLE gain7 = gain7in;
    const CSAMPLE delta8 = 2.0 * (gain8out - gain8in) / iNumSamples;
    CSAMPLE gain8 = gain8in;
    const CSAMPLE delta9 = 2.0 * (gain9out - gain9in) / iNumSamples;
    CSAMPLE gain9 = gain9in;
    const CSAMPLE delta10 = 2.0 * (gain10out - gain10in) / iNumSamples;
    CSAMPLE gain10 = gain10in;
    const CSAMPLE delta11 = 2.0 * (gain11out - gain11in) / iNumSamples;
    CSAMPLE gain11 = gain11in;
    const CSAMPLE delta12 = 2.0 * (gain12out - gain12in) / iNumSamples;
    CSAMPLE gain12 = gain12in;
    const CSAMPLE delta13 = 2.0 * (gain13out - gain13in) / iNumSamples;
    CSAMPLE gain13 = gain13in;
    const CSAMPLE delta14 = 2.0 * (gain14out - gain14in) / iNumSamples;
    CSAMPLE gain14 = gain14in;
    const CSAMPLE delta15 = 2.0 * (gain15out - gain15in) / iNumSamples;
    CSAMPLE gain15 = gain15in;
    const CSAMPLE delta16 = 2.0 * (gain16out - gain16in) / iNumSamples;
    CSAMPLE gain16 = gain16in;
    const CSAMPLE delta17 = 2.0 * (gain17out - gain17in) / iNumSamples;
    CSAMPLE gain17 = gain17in;
    const CSAMPLE delta18 = 2.0 * (gain18out - gain18in) / iNumSamples;
    CSAMPLE gain18 = gain18in;
    const CSAMPLE delta19 = 2.0 * (gain19out - gain19in) / iNumSamples;
    CSAMPLE gain19 = gain19in;
    const CSAMPLE delta20 = 2.0 * (gain20out - gain20in) / iNumSamples;
    CSAMPLE gain20 = gain20in;
    const CSAMPLE delta21 = 2.0 * (gain21out - gain21in) / iNumSamples;
    CSAMPLE gain21 = gain21in;
    const CSAMPLE delta22 = 2.0 * (gain22out - gain22in) / iNumSamples;
    CSAMPLE gain22 = gain22in;
    const CSAMPLE delta23 = 2.0 * (gain23out - gain23in) / iNumSamples;
    CSAMPLE gain23 = gain23in;
    const CSAMPLE delta24 = 2.0 * (gain24out - gain24in) / iNumSamples;
    CSAMPLE gain24 = gain24in;
    const CSAMPLE delta25 = 2.0 * (gain25out - gain25in) / iNumSamples;
    CSAMPLE gain25 = gain25in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1, gain2 += delta2, gain3 += delta3, gain4 += delta4, gain5 += delta5, gain6 += delta6, gain7 += delta7, gain8 += delta8, gain9 += delta9, gain10 += delta10, gain11 += delta11, gain12 += delta12, gain13 += delta13, gain14 += delta14, gain15 += delta15, gain16 += delta16, gain17 += delta17, gain18 += delta18, gain19 += delta19, gain20 += delta20, gain21 += delta21, gain22 += delta22, gain23 += delta23, gain24 += delta24, gain25 += delta25) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16 +
                   pSrc17[i] * gain17 +
                   pSrc18[i] * gain18 +
                   pSrc19[i] * gain19 +
                   pSrc20[i] * gain20 +
                   pSrc21[i] * gain21 +
                   pSrc22[i] * gain22 +
                   pSrc23[i] * gain23 +
                   pSrc24[i] * gain24 +
                   pSrc25[i] * gain25;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1 +
                       pSrc2[i + 1] * gain2 +
                       pSrc3[i + 1] * gain3 +
                       pSrc4[i + 1] * gain4 +
                       pSrc5[i + 1] * gain5 +
                       pSrc6[i + 1] * gain6 +
                       pSrc7[i + 1] * gain7 +
                       pSrc8[i + 1] * gain8 +
                       pSrc9[i + 1] * gain9 +
                       pSrc10[i + 1] * gain10 +
                       pSrc11[i + 1] * gain11 +
                       pSrc12[i + 1] * gain12 +
                       pSrc13[i + 1] * gain13 +
                       pSrc14[i + 1] * gain14 +
                       pSrc15[i + 1] * gain15 +
                       pSrc16[i + 1] * gain16 +
                       pSrc17[i + 1] * gain17 +
                       pSrc18[i + 1] * gain18 +
                       pSrc19[i + 1] * gain19 +
                       pSrc20[i + 1] * gain20 +
                       pSrc21[i + 1] * gain21 +
                       pSrc22[i + 1] * gain22 +
                       pSrc23[i + 1] * gain23 +
                       pSrc24[i + 1] * gain24 +
                       pSrc25[i + 1] * gain25;
    }
}
static inline void copy27WithGain(CSAMPLE* pDest,
                                  const CSAMPLE* pSrc0, CSAMPLE gain0,
                                  const CSAMPLE* pSrc1, CSAMPLE gain1,
                                  const CSAMPLE* pSrc2, CSAMPLE gain2,
                                  const CSAMPLE* pSrc3, CSAMPLE gain3,
                                  const CSAMPLE* pSrc4, CSAMPLE gain4,
                                  const CSAMPLE* pSrc5, CSAMPLE gain5,
                                  const CSAMPLE* pSrc6, CSAMPLE gain6,
                                  const CSAMPLE* pSrc7, CSAMPLE gain7,
                                  const CSAMPLE* pSrc8, CSAMPLE gain8,
                                  const CSAMPLE* pSrc9, CSAMPLE gain9,
                                  const CSAMPLE* pSrc10, CSAMPLE gain10,
                                  const CSAMPLE* pSrc11, CSAMPLE gain11,
                                  const CSAMPLE* pSrc12, CSAMPLE gain12,
                                  const CSAMPLE* pSrc13, CSAMPLE gain13,
                                  const CSAMPLE* pSrc14, CSAMPLE gain14,
                                  const CSAMPLE* pSrc15, CSAMPLE gain15,
                                  const CSAMPLE* pSrc16, CSAMPLE gain16,
                                  const CSAMPLE* pSrc17, CSAMPLE gain17,
                                  const CSAMPLE* pSrc18, CSAMPLE gain18,
                                  const CSAMPLE* pSrc19, CSAMPLE gain19,
                                  const CSAMPLE* pSrc20, CSAMPLE gain20,
                                  const CSAMPLE* pSrc21, CSAMPLE gain21,
                                  const CSAMPLE* pSrc22, CSAMPLE gain22,
                                  const CSAMPLE* pSrc23, CSAMPLE gain23,
                                  const CSAMPLE* pSrc24, CSAMPLE gain24,
                                  const CSAMPLE* pSrc25, CSAMPLE gain25,
                                  const CSAMPLE* pSrc26, CSAMPLE gain26,
                                  int iNumSamples) {
    if (gain0 == 0.0) {
        copy26WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain2 == 0.0) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain3 == 0.0) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain4 == 0.0) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain5 == 0.0) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain6 == 0.0) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain7 == 0.0) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain8 == 0.0) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain9 == 0.0) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain10 == 0.0) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain11 == 0.0) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain12 == 0.0) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain13 == 0.0) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain14 == 0.0) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain15 == 0.0) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain16 == 0.0) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain17 == 0.0) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain18 == 0.0) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain19 == 0.0) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain20 == 0.0) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain21 == 0.0) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain22 == 0.0) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain23 == 0.0) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain24 == 0.0) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain25 == 0.0) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain26 == 0.0) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16 +
                   pSrc17[i] * gain17 +
                   pSrc18[i] * gain18 +
                   pSrc19[i] * gain19 +
                   pSrc20[i] * gain20 +
                   pSrc21[i] * gain21 +
                   pSrc22[i] * gain22 +
                   pSrc23[i] * gain23 +
                   pSrc24[i] * gain24 +
                   pSrc25[i] * gain25 +
                   pSrc26[i] * gain26;
    }
}
static inline void copy27WithRampingGain(CSAMPLE* pDest,
                                         const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                         const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                         const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                                         const CSAMPLE* pSrc3, CSAMPLE gain3in, CSAMPLE gain3out,
                                         const CSAMPLE* pSrc4, CSAMPLE gain4in, CSAMPLE gain4out,
                                         const CSAMPLE* pSrc5, CSAMPLE gain5in, CSAMPLE gain5out,
                                         const CSAMPLE* pSrc6, CSAMPLE gain6in, CSAMPLE gain6out,
                                         const CSAMPLE* pSrc7, CSAMPLE gain7in, CSAMPLE gain7out,
                                         const CSAMPLE* pSrc8, CSAMPLE gain8in, CSAMPLE gain8out,
                                         const CSAMPLE* pSrc9, CSAMPLE gain9in, CSAMPLE gain9out,
                                         const CSAMPLE* pSrc10, CSAMPLE gain10in, CSAMPLE gain10out,
                                         const CSAMPLE* pSrc11, CSAMPLE gain11in, CSAMPLE gain11out,
                                         const CSAMPLE* pSrc12, CSAMPLE gain12in, CSAMPLE gain12out,
                                         const CSAMPLE* pSrc13, CSAMPLE gain13in, CSAMPLE gain13out,
                                         const CSAMPLE* pSrc14, CSAMPLE gain14in, CSAMPLE gain14out,
                                         const CSAMPLE* pSrc15, CSAMPLE gain15in, CSAMPLE gain15out,
                                         const CSAMPLE* pSrc16, CSAMPLE gain16in, CSAMPLE gain16out,
                                         const CSAMPLE* pSrc17, CSAMPLE gain17in, CSAMPLE gain17out,
                                         const CSAMPLE* pSrc18, CSAMPLE gain18in, CSAMPLE gain18out,
                                         const CSAMPLE* pSrc19, CSAMPLE gain19in, CSAMPLE gain19out,
                                         const CSAMPLE* pSrc20, CSAMPLE gain20in, CSAMPLE gain20out,
                                         const CSAMPLE* pSrc21, CSAMPLE gain21in, CSAMPLE gain21out,
                                         const CSAMPLE* pSrc22, CSAMPLE gain22in, CSAMPLE gain22out,
                                         const CSAMPLE* pSrc23, CSAMPLE gain23in, CSAMPLE gain23out,
                                         const CSAMPLE* pSrc24, CSAMPLE gain24in, CSAMPLE gain24out,
                                         const CSAMPLE* pSrc25, CSAMPLE gain25in, CSAMPLE gain25out,
                                         const CSAMPLE* pSrc26, CSAMPLE gain26in, CSAMPLE gain26out,
                                         int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy26WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain2in == 0.0 && gain2out == 0.0) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain3in == 0.0 && gain3out == 0.0) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain4in == 0.0 && gain4out == 0.0) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain5in == 0.0 && gain5out == 0.0) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain6in == 0.0 && gain6out == 0.0) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain7in == 0.0 && gain7out == 0.0) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain8in == 0.0 && gain8out == 0.0) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain9in == 0.0 && gain9out == 0.0) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain10in == 0.0 && gain10out == 0.0) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain11in == 0.0 && gain11out == 0.0) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain12in == 0.0 && gain12out == 0.0) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain13in == 0.0 && gain13out == 0.0) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain14in == 0.0 && gain14out == 0.0) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain15in == 0.0 && gain15out == 0.0) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain16in == 0.0 && gain16out == 0.0) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain17in == 0.0 && gain17out == 0.0) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain18in == 0.0 && gain18out == 0.0) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain19in == 0.0 && gain19out == 0.0) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain20in == 0.0 && gain20out == 0.0) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain21in == 0.0 && gain21out == 0.0) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain22in == 0.0 && gain22out == 0.0) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain23in == 0.0 && gain23out == 0.0) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain24in == 0.0 && gain24out == 0.0) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain25in == 0.0 && gain25out == 0.0) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain26in == 0.0 && gain26out == 0.0) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    CSAMPLE gain2 = gain2in;
    const CSAMPLE delta3 = 2.0 * (gain3out - gain3in) / iNumSamples;
    CSAMPLE gain3 = gain3in;
    const CSAMPLE delta4 = 2.0 * (gain4out - gain4in) / iNumSamples;
    CSAMPLE gain4 = gain4in;
    const CSAMPLE delta5 = 2.0 * (gain5out - gain5in) / iNumSamples;
    CSAMPLE gain5 = gain5in;
    const CSAMPLE delta6 = 2.0 * (gain6out - gain6in) / iNumSamples;
    CSAMPLE gain6 = gain6in;
    const CSAMPLE delta7 = 2.0 * (gain7out - gain7in) / iNumSamples;
    CSAMPLE gain7 = gain7in;
    const CSAMPLE delta8 = 2.0 * (gain8out - gain8in) / iNumSamples;
    CSAMPLE gain8 = gain8in;
    const CSAMPLE delta9 = 2.0 * (gain9out - gain9in) / iNumSamples;
    CSAMPLE gain9 = gain9in;
    const CSAMPLE delta10 = 2.0 * (gain10out - gain10in) / iNumSamples;
    CSAMPLE gain10 = gain10in;
    const CSAMPLE delta11 = 2.0 * (gain11out - gain11in) / iNumSamples;
    CSAMPLE gain11 = gain11in;
    const CSAMPLE delta12 = 2.0 * (gain12out - gain12in) / iNumSamples;
    CSAMPLE gain12 = gain12in;
    const CSAMPLE delta13 = 2.0 * (gain13out - gain13in) / iNumSamples;
    CSAMPLE gain13 = gain13in;
    const CSAMPLE delta14 = 2.0 * (gain14out - gain14in) / iNumSamples;
    CSAMPLE gain14 = gain14in;
    const CSAMPLE delta15 = 2.0 * (gain15out - gain15in) / iNumSamples;
    CSAMPLE gain15 = gain15in;
    const CSAMPLE delta16 = 2.0 * (gain16out - gain16in) / iNumSamples;
    CSAMPLE gain16 = gain16in;
    const CSAMPLE delta17 = 2.0 * (gain17out - gain17in) / iNumSamples;
    CSAMPLE gain17 = gain17in;
    const CSAMPLE delta18 = 2.0 * (gain18out - gain18in) / iNumSamples;
    CSAMPLE gain18 = gain18in;
    const CSAMPLE delta19 = 2.0 * (gain19out - gain19in) / iNumSamples;
    CSAMPLE gain19 = gain19in;
    const CSAMPLE delta20 = 2.0 * (gain20out - gain20in) / iNumSamples;
    CSAMPLE gain20 = gain20in;
    const CSAMPLE delta21 = 2.0 * (gain21out - gain21in) / iNumSamples;
    CSAMPLE gain21 = gain21in;
    const CSAMPLE delta22 = 2.0 * (gain22out - gain22in) / iNumSamples;
    CSAMPLE gain22 = gain22in;
    const CSAMPLE delta23 = 2.0 * (gain23out - gain23in) / iNumSamples;
    CSAMPLE gain23 = gain23in;
    const CSAMPLE delta24 = 2.0 * (gain24out - gain24in) / iNumSamples;
    CSAMPLE gain24 = gain24in;
    const CSAMPLE delta25 = 2.0 * (gain25out - gain25in) / iNumSamples;
    CSAMPLE gain25 = gain25in;
    const CSAMPLE delta26 = 2.0 * (gain26out - gain26in) / iNumSamples;
    CSAMPLE gain26 = gain26in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1, gain2 += delta2, gain3 += delta3, gain4 += delta4, gain5 += delta5, gain6 += delta6, gain7 += delta7, gain8 += delta8, gain9 += delta9, gain10 += delta10, gain11 += delta11, gain12 += delta12, gain13 += delta13, gain14 += delta14, gain15 += delta15, gain16 += delta16, gain17 += delta17, gain18 += delta18, gain19 += delta19, gain20 += delta20, gain21 += delta21, gain22 += delta22, gain23 += delta23, gain24 += delta24, gain25 += delta25, gain26 += delta26) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16 +
                   pSrc17[i] * gain17 +
                   pSrc18[i] * gain18 +
                   pSrc19[i] * gain19 +
                   pSrc20[i] * gain20 +
                   pSrc21[i] * gain21 +
                   pSrc22[i] * gain22 +
                   pSrc23[i] * gain23 +
                   pSrc24[i] * gain24 +
                   pSrc25[i] * gain25 +
                   pSrc26[i] * gain26;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1 +
                       pSrc2[i + 1] * gain2 +
                       pSrc3[i + 1] * gain3 +
                       pSrc4[i + 1] * gain4 +
                       pSrc5[i + 1] * gain5 +
                       pSrc6[i + 1] * gain6 +
                       pSrc7[i + 1] * gain7 +
                       pSrc8[i + 1] * gain8 +
                       pSrc9[i + 1] * gain9 +
                       pSrc10[i + 1] * gain10 +
                       pSrc11[i + 1] * gain11 +
                       pSrc12[i + 1] * gain12 +
                       pSrc13[i + 1] * gain13 +
                       pSrc14[i + 1] * gain14 +
                       pSrc15[i + 1] * gain15 +
                       pSrc16[i + 1] * gain16 +
                       pSrc17[i + 1] * gain17 +
                       pSrc18[i + 1] * gain18 +
                       pSrc19[i + 1] * gain19 +
                       pSrc20[i + 1] * gain20 +
                       pSrc21[i + 1] * gain21 +
                       pSrc22[i + 1] * gain22 +
                       pSrc23[i + 1] * gain23 +
                       pSrc24[i + 1] * gain24 +
                       pSrc25[i + 1] * gain25 +
                       pSrc26[i + 1] * gain26;
    }
}
static inline void copy28WithGain(CSAMPLE* pDest,
                                  const CSAMPLE* pSrc0, CSAMPLE gain0,
                                  const CSAMPLE* pSrc1, CSAMPLE gain1,
                                  const CSAMPLE* pSrc2, CSAMPLE gain2,
                                  const CSAMPLE* pSrc3, CSAMPLE gain3,
                                  const CSAMPLE* pSrc4, CSAMPLE gain4,
                                  const CSAMPLE* pSrc5, CSAMPLE gain5,
                                  const CSAMPLE* pSrc6, CSAMPLE gain6,
                                  const CSAMPLE* pSrc7, CSAMPLE gain7,
                                  const CSAMPLE* pSrc8, CSAMPLE gain8,
                                  const CSAMPLE* pSrc9, CSAMPLE gain9,
                                  const CSAMPLE* pSrc10, CSAMPLE gain10,
                                  const CSAMPLE* pSrc11, CSAMPLE gain11,
                                  const CSAMPLE* pSrc12, CSAMPLE gain12,
                                  const CSAMPLE* pSrc13, CSAMPLE gain13,
                                  const CSAMPLE* pSrc14, CSAMPLE gain14,
                                  const CSAMPLE* pSrc15, CSAMPLE gain15,
                                  const CSAMPLE* pSrc16, CSAMPLE gain16,
                                  const CSAMPLE* pSrc17, CSAMPLE gain17,
                                  const CSAMPLE* pSrc18, CSAMPLE gain18,
                                  const CSAMPLE* pSrc19, CSAMPLE gain19,
                                  const CSAMPLE* pSrc20, CSAMPLE gain20,
                                  const CSAMPLE* pSrc21, CSAMPLE gain21,
                                  const CSAMPLE* pSrc22, CSAMPLE gain22,
                                  const CSAMPLE* pSrc23, CSAMPLE gain23,
                                  const CSAMPLE* pSrc24, CSAMPLE gain24,
                                  const CSAMPLE* pSrc25, CSAMPLE gain25,
                                  const CSAMPLE* pSrc26, CSAMPLE gain26,
                                  const CSAMPLE* pSrc27, CSAMPLE gain27,
                                  int iNumSamples) {
    if (gain0 == 0.0) {
        copy27WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain2 == 0.0) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain3 == 0.0) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain4 == 0.0) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain5 == 0.0) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain6 == 0.0) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain7 == 0.0) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain8 == 0.0) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain9 == 0.0) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain10 == 0.0) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain11 == 0.0) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain12 == 0.0) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain13 == 0.0) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain14 == 0.0) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain15 == 0.0) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain16 == 0.0) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain17 == 0.0) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain18 == 0.0) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain19 == 0.0) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain20 == 0.0) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain21 == 0.0) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain22 == 0.0) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain23 == 0.0) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain24 == 0.0) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain25 == 0.0) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain26 == 0.0) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain27 == 0.0) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16 +
                   pSrc17[i] * gain17 +
                   pSrc18[i] * gain18 +
                   pSrc19[i] * gain19 +
                   pSrc20[i] * gain20 +
                   pSrc21[i] * gain21 +
                   pSrc22[i] * gain22 +
                   pSrc23[i] * gain23 +
                   pSrc24[i] * gain24 +
                   pSrc25[i] * gain25 +
                   pSrc26[i] * gain26 +
                   pSrc27[i] * gain27;
    }
}
static inline void copy28WithRampingGain(CSAMPLE* pDest,
                                         const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                         const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                         const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                                         const CSAMPLE* pSrc3, CSAMPLE gain3in, CSAMPLE gain3out,
                                         const CSAMPLE* pSrc4, CSAMPLE gain4in, CSAMPLE gain4out,
                                         const CSAMPLE* pSrc5, CSAMPLE gain5in, CSAMPLE gain5out,
                                         const CSAMPLE* pSrc6, CSAMPLE gain6in, CSAMPLE gain6out,
                                         const CSAMPLE* pSrc7, CSAMPLE gain7in, CSAMPLE gain7out,
                                         const CSAMPLE* pSrc8, CSAMPLE gain8in, CSAMPLE gain8out,
                                         const CSAMPLE* pSrc9, CSAMPLE gain9in, CSAMPLE gain9out,
                                         const CSAMPLE* pSrc10, CSAMPLE gain10in, CSAMPLE gain10out,
                                         const CSAMPLE* pSrc11, CSAMPLE gain11in, CSAMPLE gain11out,
                                         const CSAMPLE* pSrc12, CSAMPLE gain12in, CSAMPLE gain12out,
                                         const CSAMPLE* pSrc13, CSAMPLE gain13in, CSAMPLE gain13out,
                                         const CSAMPLE* pSrc14, CSAMPLE gain14in, CSAMPLE gain14out,
                                         const CSAMPLE* pSrc15, CSAMPLE gain15in, CSAMPLE gain15out,
                                         const CSAMPLE* pSrc16, CSAMPLE gain16in, CSAMPLE gain16out,
                                         const CSAMPLE* pSrc17, CSAMPLE gain17in, CSAMPLE gain17out,
                                         const CSAMPLE* pSrc18, CSAMPLE gain18in, CSAMPLE gain18out,
                                         const CSAMPLE* pSrc19, CSAMPLE gain19in, CSAMPLE gain19out,
                                         const CSAMPLE* pSrc20, CSAMPLE gain20in, CSAMPLE gain20out,
                                         const CSAMPLE* pSrc21, CSAMPLE gain21in, CSAMPLE gain21out,
                                         const CSAMPLE* pSrc22, CSAMPLE gain22in, CSAMPLE gain22out,
                                         const CSAMPLE* pSrc23, CSAMPLE gain23in, CSAMPLE gain23out,
                                         const CSAMPLE* pSrc24, CSAMPLE gain24in, CSAMPLE gain24out,
                                         const CSAMPLE* pSrc25, CSAMPLE gain25in, CSAMPLE gain25out,
                                         const CSAMPLE* pSrc26, CSAMPLE gain26in, CSAMPLE gain26out,
                                         const CSAMPLE* pSrc27, CSAMPLE gain27in, CSAMPLE gain27out,
                                         int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy27WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain2in == 0.0 && gain2out == 0.0) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain3in == 0.0 && gain3out == 0.0) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain4in == 0.0 && gain4out == 0.0) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain5in == 0.0 && gain5out == 0.0) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain6in == 0.0 && gain6out == 0.0) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain7in == 0.0 && gain7out == 0.0) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain8in == 0.0 && gain8out == 0.0) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain9in == 0.0 && gain9out == 0.0) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain10in == 0.0 && gain10out == 0.0) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain11in == 0.0 && gain11out == 0.0) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain12in == 0.0 && gain12out == 0.0) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain13in == 0.0 && gain13out == 0.0) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain14in == 0.0 && gain14out == 0.0) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain15in == 0.0 && gain15out == 0.0) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain16in == 0.0 && gain16out == 0.0) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain17in == 0.0 && gain17out == 0.0) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain18in == 0.0 && gain18out == 0.0) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain19in == 0.0 && gain19out == 0.0) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain20in == 0.0 && gain20out == 0.0) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain21in == 0.0 && gain21out == 0.0) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain22in == 0.0 && gain22out == 0.0) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain23in == 0.0 && gain23out == 0.0) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain24in == 0.0 && gain24out == 0.0) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain25in == 0.0 && gain25out == 0.0) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain26in == 0.0 && gain26out == 0.0) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain27in == 0.0 && gain27out == 0.0) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    CSAMPLE gain2 = gain2in;
    const CSAMPLE delta3 = 2.0 * (gain3out - gain3in) / iNumSamples;
    CSAMPLE gain3 = gain3in;
    const CSAMPLE delta4 = 2.0 * (gain4out - gain4in) / iNumSamples;
    CSAMPLE gain4 = gain4in;
    const CSAMPLE delta5 = 2.0 * (gain5out - gain5in) / iNumSamples;
    CSAMPLE gain5 = gain5in;
    const CSAMPLE delta6 = 2.0 * (gain6out - gain6in) / iNumSamples;
    CSAMPLE gain6 = gain6in;
    const CSAMPLE delta7 = 2.0 * (gain7out - gain7in) / iNumSamples;
    CSAMPLE gain7 = gain7in;
    const CSAMPLE delta8 = 2.0 * (gain8out - gain8in) / iNumSamples;
    CSAMPLE gain8 = gain8in;
    const CSAMPLE delta9 = 2.0 * (gain9out - gain9in) / iNumSamples;
    CSAMPLE gain9 = gain9in;
    const CSAMPLE delta10 = 2.0 * (gain10out - gain10in) / iNumSamples;
    CSAMPLE gain10 = gain10in;
    const CSAMPLE delta11 = 2.0 * (gain11out - gain11in) / iNumSamples;
    CSAMPLE gain11 = gain11in;
    const CSAMPLE delta12 = 2.0 * (gain12out - gain12in) / iNumSamples;
    CSAMPLE gain12 = gain12in;
    const CSAMPLE delta13 = 2.0 * (gain13out - gain13in) / iNumSamples;
    CSAMPLE gain13 = gain13in;
    const CSAMPLE delta14 = 2.0 * (gain14out - gain14in) / iNumSamples;
    CSAMPLE gain14 = gain14in;
    const CSAMPLE delta15 = 2.0 * (gain15out - gain15in) / iNumSamples;
    CSAMPLE gain15 = gain15in;
    const CSAMPLE delta16 = 2.0 * (gain16out - gain16in) / iNumSamples;
    CSAMPLE gain16 = gain16in;
    const CSAMPLE delta17 = 2.0 * (gain17out - gain17in) / iNumSamples;
    CSAMPLE gain17 = gain17in;
    const CSAMPLE delta18 = 2.0 * (gain18out - gain18in) / iNumSamples;
    CSAMPLE gain18 = gain18in;
    const CSAMPLE delta19 = 2.0 * (gain19out - gain19in) / iNumSamples;
    CSAMPLE gain19 = gain19in;
    const CSAMPLE delta20 = 2.0 * (gain20out - gain20in) / iNumSamples;
    CSAMPLE gain20 = gain20in;
    const CSAMPLE delta21 = 2.0 * (gain21out - gain21in) / iNumSamples;
    CSAMPLE gain21 = gain21in;
    const CSAMPLE delta22 = 2.0 * (gain22out - gain22in) / iNumSamples;
    CSAMPLE gain22 = gain22in;
    const CSAMPLE delta23 = 2.0 * (gain23out - gain23in) / iNumSamples;
    CSAMPLE gain23 = gain23in;
    const CSAMPLE delta24 = 2.0 * (gain24out - gain24in) / iNumSamples;
    CSAMPLE gain24 = gain24in;
    const CSAMPLE delta25 = 2.0 * (gain25out - gain25in) / iNumSamples;
    CSAMPLE gain25 = gain25in;
    const CSAMPLE delta26 = 2.0 * (gain26out - gain26in) / iNumSamples;
    CSAMPLE gain26 = gain26in;
    const CSAMPLE delta27 = 2.0 * (gain27out - gain27in) / iNumSamples;
    CSAMPLE gain27 = gain27in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1, gain2 += delta2, gain3 += delta3, gain4 += delta4, gain5 += delta5, gain6 += delta6, gain7 += delta7, gain8 += delta8, gain9 += delta9, gain10 += delta10, gain11 += delta11, gain12 += delta12, gain13 += delta13, gain14 += delta14, gain15 += delta15, gain16 += delta16, gain17 += delta17, gain18 += delta18, gain19 += delta19, gain20 += delta20, gain21 += delta21, gain22 += delta22, gain23 += delta23, gain24 += delta24, gain25 += delta25, gain26 += delta26, gain27 += delta27) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16 +
                   pSrc17[i] * gain17 +
                   pSrc18[i] * gain18 +
                   pSrc19[i] * gain19 +
                   pSrc20[i] * gain20 +
                   pSrc21[i] * gain21 +
                   pSrc22[i] * gain22 +
                   pSrc23[i] * gain23 +
                   pSrc24[i] * gain24 +
                   pSrc25[i] * gain25 +
                   pSrc26[i] * gain26 +
                   pSrc27[i] * gain27;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1 +
                       pSrc2[i + 1] * gain2 +
                       pSrc3[i + 1] * gain3 +
                       pSrc4[i + 1] * gain4 +
                       pSrc5[i + 1] * gain5 +
                       pSrc6[i + 1] * gain6 +
                       pSrc7[i + 1] * gain7 +
                       pSrc8[i + 1] * gain8 +
                       pSrc9[i + 1] * gain9 +
                       pSrc10[i + 1] * gain10 +
                       pSrc11[i + 1] * gain11 +
                       pSrc12[i + 1] * gain12 +
                       pSrc13[i + 1] * gain13 +
                       pSrc14[i + 1] * gain14 +
                       pSrc15[i + 1] * gain15 +
                       pSrc16[i + 1] * gain16 +
                       pSrc17[i + 1] * gain17 +
                       pSrc18[i + 1] * gain18 +
                       pSrc19[i + 1] * gain19 +
                       pSrc20[i + 1] * gain20 +
                       pSrc21[i + 1] * gain21 +
                       pSrc22[i + 1] * gain22 +
                       pSrc23[i + 1] * gain23 +
                       pSrc24[i + 1] * gain24 +
                       pSrc25[i + 1] * gain25 +
                       pSrc26[i + 1] * gain26 +
                       pSrc27[i + 1] * gain27;
    }
}
static inline void copy29WithGain(CSAMPLE* pDest,
                                  const CSAMPLE* pSrc0, CSAMPLE gain0,
                                  const CSAMPLE* pSrc1, CSAMPLE gain1,
                                  const CSAMPLE* pSrc2, CSAMPLE gain2,
                                  const CSAMPLE* pSrc3, CSAMPLE gain3,
                                  const CSAMPLE* pSrc4, CSAMPLE gain4,
                                  const CSAMPLE* pSrc5, CSAMPLE gain5,
                                  const CSAMPLE* pSrc6, CSAMPLE gain6,
                                  const CSAMPLE* pSrc7, CSAMPLE gain7,
                                  const CSAMPLE* pSrc8, CSAMPLE gain8,
                                  const CSAMPLE* pSrc9, CSAMPLE gain9,
                                  const CSAMPLE* pSrc10, CSAMPLE gain10,
                                  const CSAMPLE* pSrc11, CSAMPLE gain11,
                                  const CSAMPLE* pSrc12, CSAMPLE gain12,
                                  const CSAMPLE* pSrc13, CSAMPLE gain13,
                                  const CSAMPLE* pSrc14, CSAMPLE gain14,
                                  const CSAMPLE* pSrc15, CSAMPLE gain15,
                                  const CSAMPLE* pSrc16, CSAMPLE gain16,
                                  const CSAMPLE* pSrc17, CSAMPLE gain17,
                                  const CSAMPLE* pSrc18, CSAMPLE gain18,
                                  const CSAMPLE* pSrc19, CSAMPLE gain19,
                                  const CSAMPLE* pSrc20, CSAMPLE gain20,
                                  const CSAMPLE* pSrc21, CSAMPLE gain21,
                                  const CSAMPLE* pSrc22, CSAMPLE gain22,
                                  const CSAMPLE* pSrc23, CSAMPLE gain23,
                                  const CSAMPLE* pSrc24, CSAMPLE gain24,
                                  const CSAMPLE* pSrc25, CSAMPLE gain25,
                                  const CSAMPLE* pSrc26, CSAMPLE gain26,
                                  const CSAMPLE* pSrc27, CSAMPLE gain27,
                                  const CSAMPLE* pSrc28, CSAMPLE gain28,
                                  int iNumSamples) {
    if (gain0 == 0.0) {
        copy28WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain2 == 0.0) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain3 == 0.0) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain4 == 0.0) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain5 == 0.0) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain6 == 0.0) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain7 == 0.0) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain8 == 0.0) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain9 == 0.0) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain10 == 0.0) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain11 == 0.0) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain12 == 0.0) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain13 == 0.0) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain14 == 0.0) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain15 == 0.0) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain16 == 0.0) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain17 == 0.0) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain18 == 0.0) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain19 == 0.0) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain20 == 0.0) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain21 == 0.0) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain22 == 0.0) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain23 == 0.0) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain24 == 0.0) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain25 == 0.0) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain26 == 0.0) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain27 == 0.0) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain28 == 0.0) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16 +
                   pSrc17[i] * gain17 +
                   pSrc18[i] * gain18 +
                   pSrc19[i] * gain19 +
                   pSrc20[i] * gain20 +
                   pSrc21[i] * gain21 +
                   pSrc22[i] * gain22 +
                   pSrc23[i] * gain23 +
                   pSrc24[i] * gain24 +
                   pSrc25[i] * gain25 +
                   pSrc26[i] * gain26 +
                   pSrc27[i] * gain27 +
                   pSrc28[i] * gain28;
    }
}
static inline void copy29WithRampingGain(CSAMPLE* pDest,
                                         const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                         const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                         const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                                         const CSAMPLE* pSrc3, CSAMPLE gain3in, CSAMPLE gain3out,
                                         const CSAMPLE* pSrc4, CSAMPLE gain4in, CSAMPLE gain4out,
                                         const CSAMPLE* pSrc5, CSAMPLE gain5in, CSAMPLE gain5out,
                                         const CSAMPLE* pSrc6, CSAMPLE gain6in, CSAMPLE gain6out,
                                         const CSAMPLE* pSrc7, CSAMPLE gain7in, CSAMPLE gain7out,
                                         const CSAMPLE* pSrc8, CSAMPLE gain8in, CSAMPLE gain8out,
                                         const CSAMPLE* pSrc9, CSAMPLE gain9in, CSAMPLE gain9out,
                                         const CSAMPLE* pSrc10, CSAMPLE gain10in, CSAMPLE gain10out,
                                         const CSAMPLE* pSrc11, CSAMPLE gain11in, CSAMPLE gain11out,
                                         const CSAMPLE* pSrc12, CSAMPLE gain12in, CSAMPLE gain12out,
                                         const CSAMPLE* pSrc13, CSAMPLE gain13in, CSAMPLE gain13out,
                                         const CSAMPLE* pSrc14, CSAMPLE gain14in, CSAMPLE gain14out,
                                         const CSAMPLE* pSrc15, CSAMPLE gain15in, CSAMPLE gain15out,
                                         const CSAMPLE* pSrc16, CSAMPLE gain16in, CSAMPLE gain16out,
                                         const CSAMPLE* pSrc17, CSAMPLE gain17in, CSAMPLE gain17out,
                                         const CSAMPLE* pSrc18, CSAMPLE gain18in, CSAMPLE gain18out,
                                         const CSAMPLE* pSrc19, CSAMPLE gain19in, CSAMPLE gain19out,
                                         const CSAMPLE* pSrc20, CSAMPLE gain20in, CSAMPLE gain20out,
                                         const CSAMPLE* pSrc21, CSAMPLE gain21in, CSAMPLE gain21out,
                                         const CSAMPLE* pSrc22, CSAMPLE gain22in, CSAMPLE gain22out,
                                         const CSAMPLE* pSrc23, CSAMPLE gain23in, CSAMPLE gain23out,
                                         const CSAMPLE* pSrc24, CSAMPLE gain24in, CSAMPLE gain24out,
                                         const CSAMPLE* pSrc25, CSAMPLE gain25in, CSAMPLE gain25out,
                                         const CSAMPLE* pSrc26, CSAMPLE gain26in, CSAMPLE gain26out,
                                         const CSAMPLE* pSrc27, CSAMPLE gain27in, CSAMPLE gain27out,
                                         const CSAMPLE* pSrc28, CSAMPLE gain28in, CSAMPLE gain28out,
                                         int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy28WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain2in == 0.0 && gain2out == 0.0) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain3in == 0.0 && gain3out == 0.0) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain4in == 0.0 && gain4out == 0.0) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain5in == 0.0 && gain5out == 0.0) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain6in == 0.0 && gain6out == 0.0) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain7in == 0.0 && gain7out == 0.0) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain8in == 0.0 && gain8out == 0.0) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain9in == 0.0 && gain9out == 0.0) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain10in == 0.0 && gain10out == 0.0) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain11in == 0.0 && gain11out == 0.0) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain12in == 0.0 && gain12out == 0.0) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain13in == 0.0 && gain13out == 0.0) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain14in == 0.0 && gain14out == 0.0) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain15in == 0.0 && gain15out == 0.0) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain16in == 0.0 && gain16out == 0.0) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain17in == 0.0 && gain17out == 0.0) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain18in == 0.0 && gain18out == 0.0) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain19in == 0.0 && gain19out == 0.0) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain20in == 0.0 && gain20out == 0.0) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain21in == 0.0 && gain21out == 0.0) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain22in == 0.0 && gain22out == 0.0) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain23in == 0.0 && gain23out == 0.0) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain24in == 0.0 && gain24out == 0.0) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain25in == 0.0 && gain25out == 0.0) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain26in == 0.0 && gain26out == 0.0) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain27in == 0.0 && gain27out == 0.0) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain28in == 0.0 && gain28out == 0.0) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    CSAMPLE gain2 = gain2in;
    const CSAMPLE delta3 = 2.0 * (gain3out - gain3in) / iNumSamples;
    CSAMPLE gain3 = gain3in;
    const CSAMPLE delta4 = 2.0 * (gain4out - gain4in) / iNumSamples;
    CSAMPLE gain4 = gain4in;
    const CSAMPLE delta5 = 2.0 * (gain5out - gain5in) / iNumSamples;
    CSAMPLE gain5 = gain5in;
    const CSAMPLE delta6 = 2.0 * (gain6out - gain6in) / iNumSamples;
    CSAMPLE gain6 = gain6in;
    const CSAMPLE delta7 = 2.0 * (gain7out - gain7in) / iNumSamples;
    CSAMPLE gain7 = gain7in;
    const CSAMPLE delta8 = 2.0 * (gain8out - gain8in) / iNumSamples;
    CSAMPLE gain8 = gain8in;
    const CSAMPLE delta9 = 2.0 * (gain9out - gain9in) / iNumSamples;
    CSAMPLE gain9 = gain9in;
    const CSAMPLE delta10 = 2.0 * (gain10out - gain10in) / iNumSamples;
    CSAMPLE gain10 = gain10in;
    const CSAMPLE delta11 = 2.0 * (gain11out - gain11in) / iNumSamples;
    CSAMPLE gain11 = gain11in;
    const CSAMPLE delta12 = 2.0 * (gain12out - gain12in) / iNumSamples;
    CSAMPLE gain12 = gain12in;
    const CSAMPLE delta13 = 2.0 * (gain13out - gain13in) / iNumSamples;
    CSAMPLE gain13 = gain13in;
    const CSAMPLE delta14 = 2.0 * (gain14out - gain14in) / iNumSamples;
    CSAMPLE gain14 = gain14in;
    const CSAMPLE delta15 = 2.0 * (gain15out - gain15in) / iNumSamples;
    CSAMPLE gain15 = gain15in;
    const CSAMPLE delta16 = 2.0 * (gain16out - gain16in) / iNumSamples;
    CSAMPLE gain16 = gain16in;
    const CSAMPLE delta17 = 2.0 * (gain17out - gain17in) / iNumSamples;
    CSAMPLE gain17 = gain17in;
    const CSAMPLE delta18 = 2.0 * (gain18out - gain18in) / iNumSamples;
    CSAMPLE gain18 = gain18in;
    const CSAMPLE delta19 = 2.0 * (gain19out - gain19in) / iNumSamples;
    CSAMPLE gain19 = gain19in;
    const CSAMPLE delta20 = 2.0 * (gain20out - gain20in) / iNumSamples;
    CSAMPLE gain20 = gain20in;
    const CSAMPLE delta21 = 2.0 * (gain21out - gain21in) / iNumSamples;
    CSAMPLE gain21 = gain21in;
    const CSAMPLE delta22 = 2.0 * (gain22out - gain22in) / iNumSamples;
    CSAMPLE gain22 = gain22in;
    const CSAMPLE delta23 = 2.0 * (gain23out - gain23in) / iNumSamples;
    CSAMPLE gain23 = gain23in;
    const CSAMPLE delta24 = 2.0 * (gain24out - gain24in) / iNumSamples;
    CSAMPLE gain24 = gain24in;
    const CSAMPLE delta25 = 2.0 * (gain25out - gain25in) / iNumSamples;
    CSAMPLE gain25 = gain25in;
    const CSAMPLE delta26 = 2.0 * (gain26out - gain26in) / iNumSamples;
    CSAMPLE gain26 = gain26in;
    const CSAMPLE delta27 = 2.0 * (gain27out - gain27in) / iNumSamples;
    CSAMPLE gain27 = gain27in;
    const CSAMPLE delta28 = 2.0 * (gain28out - gain28in) / iNumSamples;
    CSAMPLE gain28 = gain28in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1, gain2 += delta2, gain3 += delta3, gain4 += delta4, gain5 += delta5, gain6 += delta6, gain7 += delta7, gain8 += delta8, gain9 += delta9, gain10 += delta10, gain11 += delta11, gain12 += delta12, gain13 += delta13, gain14 += delta14, gain15 += delta15, gain16 += delta16, gain17 += delta17, gain18 += delta18, gain19 += delta19, gain20 += delta20, gain21 += delta21, gain22 += delta22, gain23 += delta23, gain24 += delta24, gain25 += delta25, gain26 += delta26, gain27 += delta27, gain28 += delta28) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16 +
                   pSrc17[i] * gain17 +
                   pSrc18[i] * gain18 +
                   pSrc19[i] * gain19 +
                   pSrc20[i] * gain20 +
                   pSrc21[i] * gain21 +
                   pSrc22[i] * gain22 +
                   pSrc23[i] * gain23 +
                   pSrc24[i] * gain24 +
                   pSrc25[i] * gain25 +
                   pSrc26[i] * gain26 +
                   pSrc27[i] * gain27 +
                   pSrc28[i] * gain28;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1 +
                       pSrc2[i + 1] * gain2 +
                       pSrc3[i + 1] * gain3 +
                       pSrc4[i + 1] * gain4 +
                       pSrc5[i + 1] * gain5 +
                       pSrc6[i + 1] * gain6 +
                       pSrc7[i + 1] * gain7 +
                       pSrc8[i + 1] * gain8 +
                       pSrc9[i + 1] * gain9 +
                       pSrc10[i + 1] * gain10 +
                       pSrc11[i + 1] * gain11 +
                       pSrc12[i + 1] * gain12 +
                       pSrc13[i + 1] * gain13 +
                       pSrc14[i + 1] * gain14 +
                       pSrc15[i + 1] * gain15 +
                       pSrc16[i + 1] * gain16 +
                       pSrc17[i + 1] * gain17 +
                       pSrc18[i + 1] * gain18 +
                       pSrc19[i + 1] * gain19 +
                       pSrc20[i + 1] * gain20 +
                       pSrc21[i + 1] * gain21 +
                       pSrc22[i + 1] * gain22 +
                       pSrc23[i + 1] * gain23 +
                       pSrc24[i + 1] * gain24 +
                       pSrc25[i + 1] * gain25 +
                       pSrc26[i + 1] * gain26 +
                       pSrc27[i + 1] * gain27 +
                       pSrc28[i + 1] * gain28;
    }
}
static inline void copy30WithGain(CSAMPLE* pDest,
                                  const CSAMPLE* pSrc0, CSAMPLE gain0,
                                  const CSAMPLE* pSrc1, CSAMPLE gain1,
                                  const CSAMPLE* pSrc2, CSAMPLE gain2,
                                  const CSAMPLE* pSrc3, CSAMPLE gain3,
                                  const CSAMPLE* pSrc4, CSAMPLE gain4,
                                  const CSAMPLE* pSrc5, CSAMPLE gain5,
                                  const CSAMPLE* pSrc6, CSAMPLE gain6,
                                  const CSAMPLE* pSrc7, CSAMPLE gain7,
                                  const CSAMPLE* pSrc8, CSAMPLE gain8,
                                  const CSAMPLE* pSrc9, CSAMPLE gain9,
                                  const CSAMPLE* pSrc10, CSAMPLE gain10,
                                  const CSAMPLE* pSrc11, CSAMPLE gain11,
                                  const CSAMPLE* pSrc12, CSAMPLE gain12,
                                  const CSAMPLE* pSrc13, CSAMPLE gain13,
                                  const CSAMPLE* pSrc14, CSAMPLE gain14,
                                  const CSAMPLE* pSrc15, CSAMPLE gain15,
                                  const CSAMPLE* pSrc16, CSAMPLE gain16,
                                  const CSAMPLE* pSrc17, CSAMPLE gain17,
                                  const CSAMPLE* pSrc18, CSAMPLE gain18,
                                  const CSAMPLE* pSrc19, CSAMPLE gain19,
                                  const CSAMPLE* pSrc20, CSAMPLE gain20,
                                  const CSAMPLE* pSrc21, CSAMPLE gain21,
                                  const CSAMPLE* pSrc22, CSAMPLE gain22,
                                  const CSAMPLE* pSrc23, CSAMPLE gain23,
                                  const CSAMPLE* pSrc24, CSAMPLE gain24,
                                  const CSAMPLE* pSrc25, CSAMPLE gain25,
                                  const CSAMPLE* pSrc26, CSAMPLE gain26,
                                  const CSAMPLE* pSrc27, CSAMPLE gain27,
                                  const CSAMPLE* pSrc28, CSAMPLE gain28,
                                  const CSAMPLE* pSrc29, CSAMPLE gain29,
                                  int iNumSamples) {
    if (gain0 == 0.0) {
        copy29WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain2 == 0.0) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain3 == 0.0) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain4 == 0.0) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain5 == 0.0) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain6 == 0.0) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain7 == 0.0) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain8 == 0.0) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain9 == 0.0) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain10 == 0.0) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain11 == 0.0) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain12 == 0.0) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain13 == 0.0) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain14 == 0.0) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain15 == 0.0) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain16 == 0.0) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain17 == 0.0) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain18 == 0.0) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain19 == 0.0) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain20 == 0.0) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain21 == 0.0) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain22 == 0.0) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain23 == 0.0) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain24 == 0.0) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain25 == 0.0) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain26 == 0.0) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain27 == 0.0) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain28 == 0.0) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain29 == 0.0) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16 +
                   pSrc17[i] * gain17 +
                   pSrc18[i] * gain18 +
                   pSrc19[i] * gain19 +
                   pSrc20[i] * gain20 +
                   pSrc21[i] * gain21 +
                   pSrc22[i] * gain22 +
                   pSrc23[i] * gain23 +
                   pSrc24[i] * gain24 +
                   pSrc25[i] * gain25 +
                   pSrc26[i] * gain26 +
                   pSrc27[i] * gain27 +
                   pSrc28[i] * gain28 +
                   pSrc29[i] * gain29;
    }
}
static inline void copy30WithRampingGain(CSAMPLE* pDest,
                                         const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                         const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                         const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                                         const CSAMPLE* pSrc3, CSAMPLE gain3in, CSAMPLE gain3out,
                                         const CSAMPLE* pSrc4, CSAMPLE gain4in, CSAMPLE gain4out,
                                         const CSAMPLE* pSrc5, CSAMPLE gain5in, CSAMPLE gain5out,
                                         const CSAMPLE* pSrc6, CSAMPLE gain6in, CSAMPLE gain6out,
                                         const CSAMPLE* pSrc7, CSAMPLE gain7in, CSAMPLE gain7out,
                                         const CSAMPLE* pSrc8, CSAMPLE gain8in, CSAMPLE gain8out,
                                         const CSAMPLE* pSrc9, CSAMPLE gain9in, CSAMPLE gain9out,
                                         const CSAMPLE* pSrc10, CSAMPLE gain10in, CSAMPLE gain10out,
                                         const CSAMPLE* pSrc11, CSAMPLE gain11in, CSAMPLE gain11out,
                                         const CSAMPLE* pSrc12, CSAMPLE gain12in, CSAMPLE gain12out,
                                         const CSAMPLE* pSrc13, CSAMPLE gain13in, CSAMPLE gain13out,
                                         const CSAMPLE* pSrc14, CSAMPLE gain14in, CSAMPLE gain14out,
                                         const CSAMPLE* pSrc15, CSAMPLE gain15in, CSAMPLE gain15out,
                                         const CSAMPLE* pSrc16, CSAMPLE gain16in, CSAMPLE gain16out,
                                         const CSAMPLE* pSrc17, CSAMPLE gain17in, CSAMPLE gain17out,
                                         const CSAMPLE* pSrc18, CSAMPLE gain18in, CSAMPLE gain18out,
                                         const CSAMPLE* pSrc19, CSAMPLE gain19in, CSAMPLE gain19out,
                                         const CSAMPLE* pSrc20, CSAMPLE gain20in, CSAMPLE gain20out,
                                         const CSAMPLE* pSrc21, CSAMPLE gain21in, CSAMPLE gain21out,
                                         const CSAMPLE* pSrc22, CSAMPLE gain22in, CSAMPLE gain22out,
                                         const CSAMPLE* pSrc23, CSAMPLE gain23in, CSAMPLE gain23out,
                                         const CSAMPLE* pSrc24, CSAMPLE gain24in, CSAMPLE gain24out,
                                         const CSAMPLE* pSrc25, CSAMPLE gain25in, CSAMPLE gain25out,
                                         const CSAMPLE* pSrc26, CSAMPLE gain26in, CSAMPLE gain26out,
                                         const CSAMPLE* pSrc27, CSAMPLE gain27in, CSAMPLE gain27out,
                                         const CSAMPLE* pSrc28, CSAMPLE gain28in, CSAMPLE gain28out,
                                         const CSAMPLE* pSrc29, CSAMPLE gain29in, CSAMPLE gain29out,
                                         int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy29WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain2in == 0.0 && gain2out == 0.0) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain3in == 0.0 && gain3out == 0.0) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain4in == 0.0 && gain4out == 0.0) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain5in == 0.0 && gain5out == 0.0) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain6in == 0.0 && gain6out == 0.0) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain7in == 0.0 && gain7out == 0.0) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain8in == 0.0 && gain8out == 0.0) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain9in == 0.0 && gain9out == 0.0) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain10in == 0.0 && gain10out == 0.0) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain11in == 0.0 && gain11out == 0.0) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain12in == 0.0 && gain12out == 0.0) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain13in == 0.0 && gain13out == 0.0) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain14in == 0.0 && gain14out == 0.0) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain15in == 0.0 && gain15out == 0.0) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain16in == 0.0 && gain16out == 0.0) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain17in == 0.0 && gain17out == 0.0) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain18in == 0.0 && gain18out == 0.0) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain19in == 0.0 && gain19out == 0.0) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain20in == 0.0 && gain20out == 0.0) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain21in == 0.0 && gain21out == 0.0) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain22in == 0.0 && gain22out == 0.0) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain23in == 0.0 && gain23out == 0.0) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain24in == 0.0 && gain24out == 0.0) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain25in == 0.0 && gain25out == 0.0) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain26in == 0.0 && gain26out == 0.0) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain27in == 0.0 && gain27out == 0.0) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain28in == 0.0 && gain28out == 0.0) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain29in == 0.0 && gain29out == 0.0) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    CSAMPLE gain2 = gain2in;
    const CSAMPLE delta3 = 2.0 * (gain3out - gain3in) / iNumSamples;
    CSAMPLE gain3 = gain3in;
    const CSAMPLE delta4 = 2.0 * (gain4out - gain4in) / iNumSamples;
    CSAMPLE gain4 = gain4in;
    const CSAMPLE delta5 = 2.0 * (gain5out - gain5in) / iNumSamples;
    CSAMPLE gain5 = gain5in;
    const CSAMPLE delta6 = 2.0 * (gain6out - gain6in) / iNumSamples;
    CSAMPLE gain6 = gain6in;
    const CSAMPLE delta7 = 2.0 * (gain7out - gain7in) / iNumSamples;
    CSAMPLE gain7 = gain7in;
    const CSAMPLE delta8 = 2.0 * (gain8out - gain8in) / iNumSamples;
    CSAMPLE gain8 = gain8in;
    const CSAMPLE delta9 = 2.0 * (gain9out - gain9in) / iNumSamples;
    CSAMPLE gain9 = gain9in;
    const CSAMPLE delta10 = 2.0 * (gain10out - gain10in) / iNumSamples;
    CSAMPLE gain10 = gain10in;
    const CSAMPLE delta11 = 2.0 * (gain11out - gain11in) / iNumSamples;
    CSAMPLE gain11 = gain11in;
    const CSAMPLE delta12 = 2.0 * (gain12out - gain12in) / iNumSamples;
    CSAMPLE gain12 = gain12in;
    const CSAMPLE delta13 = 2.0 * (gain13out - gain13in) / iNumSamples;
    CSAMPLE gain13 = gain13in;
    const CSAMPLE delta14 = 2.0 * (gain14out - gain14in) / iNumSamples;
    CSAMPLE gain14 = gain14in;
    const CSAMPLE delta15 = 2.0 * (gain15out - gain15in) / iNumSamples;
    CSAMPLE gain15 = gain15in;
    const CSAMPLE delta16 = 2.0 * (gain16out - gain16in) / iNumSamples;
    CSAMPLE gain16 = gain16in;
    const CSAMPLE delta17 = 2.0 * (gain17out - gain17in) / iNumSamples;
    CSAMPLE gain17 = gain17in;
    const CSAMPLE delta18 = 2.0 * (gain18out - gain18in) / iNumSamples;
    CSAMPLE gain18 = gain18in;
    const CSAMPLE delta19 = 2.0 * (gain19out - gain19in) / iNumSamples;
    CSAMPLE gain19 = gain19in;
    const CSAMPLE delta20 = 2.0 * (gain20out - gain20in) / iNumSamples;
    CSAMPLE gain20 = gain20in;
    const CSAMPLE delta21 = 2.0 * (gain21out - gain21in) / iNumSamples;
    CSAMPLE gain21 = gain21in;
    const CSAMPLE delta22 = 2.0 * (gain22out - gain22in) / iNumSamples;
    CSAMPLE gain22 = gain22in;
    const CSAMPLE delta23 = 2.0 * (gain23out - gain23in) / iNumSamples;
    CSAMPLE gain23 = gain23in;
    const CSAMPLE delta24 = 2.0 * (gain24out - gain24in) / iNumSamples;
    CSAMPLE gain24 = gain24in;
    const CSAMPLE delta25 = 2.0 * (gain25out - gain25in) / iNumSamples;
    CSAMPLE gain25 = gain25in;
    const CSAMPLE delta26 = 2.0 * (gain26out - gain26in) / iNumSamples;
    CSAMPLE gain26 = gain26in;
    const CSAMPLE delta27 = 2.0 * (gain27out - gain27in) / iNumSamples;
    CSAMPLE gain27 = gain27in;
    const CSAMPLE delta28 = 2.0 * (gain28out - gain28in) / iNumSamples;
    CSAMPLE gain28 = gain28in;
    const CSAMPLE delta29 = 2.0 * (gain29out - gain29in) / iNumSamples;
    CSAMPLE gain29 = gain29in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1, gain2 += delta2, gain3 += delta3, gain4 += delta4, gain5 += delta5, gain6 += delta6, gain7 += delta7, gain8 += delta8, gain9 += delta9, gain10 += delta10, gain11 += delta11, gain12 += delta12, gain13 += delta13, gain14 += delta14, gain15 += delta15, gain16 += delta16, gain17 += delta17, gain18 += delta18, gain19 += delta19, gain20 += delta20, gain21 += delta21, gain22 += delta22, gain23 += delta23, gain24 += delta24, gain25 += delta25, gain26 += delta26, gain27 += delta27, gain28 += delta28, gain29 += delta29) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16 +
                   pSrc17[i] * gain17 +
                   pSrc18[i] * gain18 +
                   pSrc19[i] * gain19 +
                   pSrc20[i] * gain20 +
                   pSrc21[i] * gain21 +
                   pSrc22[i] * gain22 +
                   pSrc23[i] * gain23 +
                   pSrc24[i] * gain24 +
                   pSrc25[i] * gain25 +
                   pSrc26[i] * gain26 +
                   pSrc27[i] * gain27 +
                   pSrc28[i] * gain28 +
                   pSrc29[i] * gain29;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1 +
                       pSrc2[i + 1] * gain2 +
                       pSrc3[i + 1] * gain3 +
                       pSrc4[i + 1] * gain4 +
                       pSrc5[i + 1] * gain5 +
                       pSrc6[i + 1] * gain6 +
                       pSrc7[i + 1] * gain7 +
                       pSrc8[i + 1] * gain8 +
                       pSrc9[i + 1] * gain9 +
                       pSrc10[i + 1] * gain10 +
                       pSrc11[i + 1] * gain11 +
                       pSrc12[i + 1] * gain12 +
                       pSrc13[i + 1] * gain13 +
                       pSrc14[i + 1] * gain14 +
                       pSrc15[i + 1] * gain15 +
                       pSrc16[i + 1] * gain16 +
                       pSrc17[i + 1] * gain17 +
                       pSrc18[i + 1] * gain18 +
                       pSrc19[i + 1] * gain19 +
                       pSrc20[i + 1] * gain20 +
                       pSrc21[i + 1] * gain21 +
                       pSrc22[i + 1] * gain22 +
                       pSrc23[i + 1] * gain23 +
                       pSrc24[i + 1] * gain24 +
                       pSrc25[i + 1] * gain25 +
                       pSrc26[i + 1] * gain26 +
                       pSrc27[i + 1] * gain27 +
                       pSrc28[i + 1] * gain28 +
                       pSrc29[i + 1] * gain29;
    }
}
static inline void copy31WithGain(CSAMPLE* pDest,
                                  const CSAMPLE* pSrc0, CSAMPLE gain0,
                                  const CSAMPLE* pSrc1, CSAMPLE gain1,
                                  const CSAMPLE* pSrc2, CSAMPLE gain2,
                                  const CSAMPLE* pSrc3, CSAMPLE gain3,
                                  const CSAMPLE* pSrc4, CSAMPLE gain4,
                                  const CSAMPLE* pSrc5, CSAMPLE gain5,
                                  const CSAMPLE* pSrc6, CSAMPLE gain6,
                                  const CSAMPLE* pSrc7, CSAMPLE gain7,
                                  const CSAMPLE* pSrc8, CSAMPLE gain8,
                                  const CSAMPLE* pSrc9, CSAMPLE gain9,
                                  const CSAMPLE* pSrc10, CSAMPLE gain10,
                                  const CSAMPLE* pSrc11, CSAMPLE gain11,
                                  const CSAMPLE* pSrc12, CSAMPLE gain12,
                                  const CSAMPLE* pSrc13, CSAMPLE gain13,
                                  const CSAMPLE* pSrc14, CSAMPLE gain14,
                                  const CSAMPLE* pSrc15, CSAMPLE gain15,
                                  const CSAMPLE* pSrc16, CSAMPLE gain16,
                                  const CSAMPLE* pSrc17, CSAMPLE gain17,
                                  const CSAMPLE* pSrc18, CSAMPLE gain18,
                                  const CSAMPLE* pSrc19, CSAMPLE gain19,
                                  const CSAMPLE* pSrc20, CSAMPLE gain20,
                                  const CSAMPLE* pSrc21, CSAMPLE gain21,
                                  const CSAMPLE* pSrc22, CSAMPLE gain22,
                                  const CSAMPLE* pSrc23, CSAMPLE gain23,
                                  const CSAMPLE* pSrc24, CSAMPLE gain24,
                                  const CSAMPLE* pSrc25, CSAMPLE gain25,
                                  const CSAMPLE* pSrc26, CSAMPLE gain26,
                                  const CSAMPLE* pSrc27, CSAMPLE gain27,
                                  const CSAMPLE* pSrc28, CSAMPLE gain28,
                                  const CSAMPLE* pSrc29, CSAMPLE gain29,
                                  const CSAMPLE* pSrc30, CSAMPLE gain30,
                                  int iNumSamples) {
    if (gain0 == 0.0) {
        copy30WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain2 == 0.0) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain3 == 0.0) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain4 == 0.0) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain5 == 0.0) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain6 == 0.0) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain7 == 0.0) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain8 == 0.0) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain9 == 0.0) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain10 == 0.0) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain11 == 0.0) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain12 == 0.0) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain13 == 0.0) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain14 == 0.0) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain15 == 0.0) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain16 == 0.0) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain17 == 0.0) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain18 == 0.0) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain19 == 0.0) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain20 == 0.0) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain21 == 0.0) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain22 == 0.0) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain23 == 0.0) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain24 == 0.0) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain25 == 0.0) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain26 == 0.0) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain27 == 0.0) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain28 == 0.0) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain29 == 0.0) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain30 == 0.0) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16 +
                   pSrc17[i] * gain17 +
                   pSrc18[i] * gain18 +
                   pSrc19[i] * gain19 +
                   pSrc20[i] * gain20 +
                   pSrc21[i] * gain21 +
                   pSrc22[i] * gain22 +
                   pSrc23[i] * gain23 +
                   pSrc24[i] * gain24 +
                   pSrc25[i] * gain25 +
                   pSrc26[i] * gain26 +
                   pSrc27[i] * gain27 +
                   pSrc28[i] * gain28 +
                   pSrc29[i] * gain29 +
                   pSrc30[i] * gain30;
    }
}
static inline void copy31WithRampingGain(CSAMPLE* pDest,
                                         const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                         const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                         const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                                         const CSAMPLE* pSrc3, CSAMPLE gain3in, CSAMPLE gain3out,
                                         const CSAMPLE* pSrc4, CSAMPLE gain4in, CSAMPLE gain4out,
                                         const CSAMPLE* pSrc5, CSAMPLE gain5in, CSAMPLE gain5out,
                                         const CSAMPLE* pSrc6, CSAMPLE gain6in, CSAMPLE gain6out,
                                         const CSAMPLE* pSrc7, CSAMPLE gain7in, CSAMPLE gain7out,
                                         const CSAMPLE* pSrc8, CSAMPLE gain8in, CSAMPLE gain8out,
                                         const CSAMPLE* pSrc9, CSAMPLE gain9in, CSAMPLE gain9out,
                                         const CSAMPLE* pSrc10, CSAMPLE gain10in, CSAMPLE gain10out,
                                         const CSAMPLE* pSrc11, CSAMPLE gain11in, CSAMPLE gain11out,
                                         const CSAMPLE* pSrc12, CSAMPLE gain12in, CSAMPLE gain12out,
                                         const CSAMPLE* pSrc13, CSAMPLE gain13in, CSAMPLE gain13out,
                                         const CSAMPLE* pSrc14, CSAMPLE gain14in, CSAMPLE gain14out,
                                         const CSAMPLE* pSrc15, CSAMPLE gain15in, CSAMPLE gain15out,
                                         const CSAMPLE* pSrc16, CSAMPLE gain16in, CSAMPLE gain16out,
                                         const CSAMPLE* pSrc17, CSAMPLE gain17in, CSAMPLE gain17out,
                                         const CSAMPLE* pSrc18, CSAMPLE gain18in, CSAMPLE gain18out,
                                         const CSAMPLE* pSrc19, CSAMPLE gain19in, CSAMPLE gain19out,
                                         const CSAMPLE* pSrc20, CSAMPLE gain20in, CSAMPLE gain20out,
                                         const CSAMPLE* pSrc21, CSAMPLE gain21in, CSAMPLE gain21out,
                                         const CSAMPLE* pSrc22, CSAMPLE gain22in, CSAMPLE gain22out,
                                         const CSAMPLE* pSrc23, CSAMPLE gain23in, CSAMPLE gain23out,
                                         const CSAMPLE* pSrc24, CSAMPLE gain24in, CSAMPLE gain24out,
                                         const CSAMPLE* pSrc25, CSAMPLE gain25in, CSAMPLE gain25out,
                                         const CSAMPLE* pSrc26, CSAMPLE gain26in, CSAMPLE gain26out,
                                         const CSAMPLE* pSrc27, CSAMPLE gain27in, CSAMPLE gain27out,
                                         const CSAMPLE* pSrc28, CSAMPLE gain28in, CSAMPLE gain28out,
                                         const CSAMPLE* pSrc29, CSAMPLE gain29in, CSAMPLE gain29out,
                                         const CSAMPLE* pSrc30, CSAMPLE gain30in, CSAMPLE gain30out,
                                         int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy30WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain2in == 0.0 && gain2out == 0.0) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain3in == 0.0 && gain3out == 0.0) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain4in == 0.0 && gain4out == 0.0) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain5in == 0.0 && gain5out == 0.0) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain6in == 0.0 && gain6out == 0.0) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain7in == 0.0 && gain7out == 0.0) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain8in == 0.0 && gain8out == 0.0) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain9in == 0.0 && gain9out == 0.0) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain10in == 0.0 && gain10out == 0.0) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain11in == 0.0 && gain11out == 0.0) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain12in == 0.0 && gain12out == 0.0) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain13in == 0.0 && gain13out == 0.0) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain14in == 0.0 && gain14out == 0.0) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain15in == 0.0 && gain15out == 0.0) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain16in == 0.0 && gain16out == 0.0) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain17in == 0.0 && gain17out == 0.0) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain18in == 0.0 && gain18out == 0.0) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain19in == 0.0 && gain19out == 0.0) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain20in == 0.0 && gain20out == 0.0) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain21in == 0.0 && gain21out == 0.0) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain22in == 0.0 && gain22out == 0.0) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain23in == 0.0 && gain23out == 0.0) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain24in == 0.0 && gain24out == 0.0) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain25in == 0.0 && gain25out == 0.0) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain26in == 0.0 && gain26out == 0.0) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain27in == 0.0 && gain27out == 0.0) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain28in == 0.0 && gain28out == 0.0) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain29in == 0.0 && gain29out == 0.0) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain30in == 0.0 && gain30out == 0.0) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    CSAMPLE gain2 = gain2in;
    const CSAMPLE delta3 = 2.0 * (gain3out - gain3in) / iNumSamples;
    CSAMPLE gain3 = gain3in;
    const CSAMPLE delta4 = 2.0 * (gain4out - gain4in) / iNumSamples;
    CSAMPLE gain4 = gain4in;
    const CSAMPLE delta5 = 2.0 * (gain5out - gain5in) / iNumSamples;
    CSAMPLE gain5 = gain5in;
    const CSAMPLE delta6 = 2.0 * (gain6out - gain6in) / iNumSamples;
    CSAMPLE gain6 = gain6in;
    const CSAMPLE delta7 = 2.0 * (gain7out - gain7in) / iNumSamples;
    CSAMPLE gain7 = gain7in;
    const CSAMPLE delta8 = 2.0 * (gain8out - gain8in) / iNumSamples;
    CSAMPLE gain8 = gain8in;
    const CSAMPLE delta9 = 2.0 * (gain9out - gain9in) / iNumSamples;
    CSAMPLE gain9 = gain9in;
    const CSAMPLE delta10 = 2.0 * (gain10out - gain10in) / iNumSamples;
    CSAMPLE gain10 = gain10in;
    const CSAMPLE delta11 = 2.0 * (gain11out - gain11in) / iNumSamples;
    CSAMPLE gain11 = gain11in;
    const CSAMPLE delta12 = 2.0 * (gain12out - gain12in) / iNumSamples;
    CSAMPLE gain12 = gain12in;
    const CSAMPLE delta13 = 2.0 * (gain13out - gain13in) / iNumSamples;
    CSAMPLE gain13 = gain13in;
    const CSAMPLE delta14 = 2.0 * (gain14out - gain14in) / iNumSamples;
    CSAMPLE gain14 = gain14in;
    const CSAMPLE delta15 = 2.0 * (gain15out - gain15in) / iNumSamples;
    CSAMPLE gain15 = gain15in;
    const CSAMPLE delta16 = 2.0 * (gain16out - gain16in) / iNumSamples;
    CSAMPLE gain16 = gain16in;
    const CSAMPLE delta17 = 2.0 * (gain17out - gain17in) / iNumSamples;
    CSAMPLE gain17 = gain17in;
    const CSAMPLE delta18 = 2.0 * (gain18out - gain18in) / iNumSamples;
    CSAMPLE gain18 = gain18in;
    const CSAMPLE delta19 = 2.0 * (gain19out - gain19in) / iNumSamples;
    CSAMPLE gain19 = gain19in;
    const CSAMPLE delta20 = 2.0 * (gain20out - gain20in) / iNumSamples;
    CSAMPLE gain20 = gain20in;
    const CSAMPLE delta21 = 2.0 * (gain21out - gain21in) / iNumSamples;
    CSAMPLE gain21 = gain21in;
    const CSAMPLE delta22 = 2.0 * (gain22out - gain22in) / iNumSamples;
    CSAMPLE gain22 = gain22in;
    const CSAMPLE delta23 = 2.0 * (gain23out - gain23in) / iNumSamples;
    CSAMPLE gain23 = gain23in;
    const CSAMPLE delta24 = 2.0 * (gain24out - gain24in) / iNumSamples;
    CSAMPLE gain24 = gain24in;
    const CSAMPLE delta25 = 2.0 * (gain25out - gain25in) / iNumSamples;
    CSAMPLE gain25 = gain25in;
    const CSAMPLE delta26 = 2.0 * (gain26out - gain26in) / iNumSamples;
    CSAMPLE gain26 = gain26in;
    const CSAMPLE delta27 = 2.0 * (gain27out - gain27in) / iNumSamples;
    CSAMPLE gain27 = gain27in;
    const CSAMPLE delta28 = 2.0 * (gain28out - gain28in) / iNumSamples;
    CSAMPLE gain28 = gain28in;
    const CSAMPLE delta29 = 2.0 * (gain29out - gain29in) / iNumSamples;
    CSAMPLE gain29 = gain29in;
    const CSAMPLE delta30 = 2.0 * (gain30out - gain30in) / iNumSamples;
    CSAMPLE gain30 = gain30in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1, gain2 += delta2, gain3 += delta3, gain4 += delta4, gain5 += delta5, gain6 += delta6, gain7 += delta7, gain8 += delta8, gain9 += delta9, gain10 += delta10, gain11 += delta11, gain12 += delta12, gain13 += delta13, gain14 += delta14, gain15 += delta15, gain16 += delta16, gain17 += delta17, gain18 += delta18, gain19 += delta19, gain20 += delta20, gain21 += delta21, gain22 += delta22, gain23 += delta23, gain24 += delta24, gain25 += delta25, gain26 += delta26, gain27 += delta27, gain28 += delta28, gain29 += delta29, gain30 += delta30) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16 +
                   pSrc17[i] * gain17 +
                   pSrc18[i] * gain18 +
                   pSrc19[i] * gain19 +
                   pSrc20[i] * gain20 +
                   pSrc21[i] * gain21 +
                   pSrc22[i] * gain22 +
                   pSrc23[i] * gain23 +
                   pSrc24[i] * gain24 +
                   pSrc25[i] * gain25 +
                   pSrc26[i] * gain26 +
                   pSrc27[i] * gain27 +
                   pSrc28[i] * gain28 +
                   pSrc29[i] * gain29 +
                   pSrc30[i] * gain30;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1 +
                       pSrc2[i + 1] * gain2 +
                       pSrc3[i + 1] * gain3 +
                       pSrc4[i + 1] * gain4 +
                       pSrc5[i + 1] * gain5 +
                       pSrc6[i + 1] * gain6 +
                       pSrc7[i + 1] * gain7 +
                       pSrc8[i + 1] * gain8 +
                       pSrc9[i + 1] * gain9 +
                       pSrc10[i + 1] * gain10 +
                       pSrc11[i + 1] * gain11 +
                       pSrc12[i + 1] * gain12 +
                       pSrc13[i + 1] * gain13 +
                       pSrc14[i + 1] * gain14 +
                       pSrc15[i + 1] * gain15 +
                       pSrc16[i + 1] * gain16 +
                       pSrc17[i + 1] * gain17 +
                       pSrc18[i + 1] * gain18 +
                       pSrc19[i + 1] * gain19 +
                       pSrc20[i + 1] * gain20 +
                       pSrc21[i + 1] * gain21 +
                       pSrc22[i + 1] * gain22 +
                       pSrc23[i + 1] * gain23 +
                       pSrc24[i + 1] * gain24 +
                       pSrc25[i + 1] * gain25 +
                       pSrc26[i + 1] * gain26 +
                       pSrc27[i + 1] * gain27 +
                       pSrc28[i + 1] * gain28 +
                       pSrc29[i + 1] * gain29 +
                       pSrc30[i + 1] * gain30;
    }
}
static inline void copy32WithGain(CSAMPLE* pDest,
                                  const CSAMPLE* pSrc0, CSAMPLE gain0,
                                  const CSAMPLE* pSrc1, CSAMPLE gain1,
                                  const CSAMPLE* pSrc2, CSAMPLE gain2,
                                  const CSAMPLE* pSrc3, CSAMPLE gain3,
                                  const CSAMPLE* pSrc4, CSAMPLE gain4,
                                  const CSAMPLE* pSrc5, CSAMPLE gain5,
                                  const CSAMPLE* pSrc6, CSAMPLE gain6,
                                  const CSAMPLE* pSrc7, CSAMPLE gain7,
                                  const CSAMPLE* pSrc8, CSAMPLE gain8,
                                  const CSAMPLE* pSrc9, CSAMPLE gain9,
                                  const CSAMPLE* pSrc10, CSAMPLE gain10,
                                  const CSAMPLE* pSrc11, CSAMPLE gain11,
                                  const CSAMPLE* pSrc12, CSAMPLE gain12,
                                  const CSAMPLE* pSrc13, CSAMPLE gain13,
                                  const CSAMPLE* pSrc14, CSAMPLE gain14,
                                  const CSAMPLE* pSrc15, CSAMPLE gain15,
                                  const CSAMPLE* pSrc16, CSAMPLE gain16,
                                  const CSAMPLE* pSrc17, CSAMPLE gain17,
                                  const CSAMPLE* pSrc18, CSAMPLE gain18,
                                  const CSAMPLE* pSrc19, CSAMPLE gain19,
                                  const CSAMPLE* pSrc20, CSAMPLE gain20,
                                  const CSAMPLE* pSrc21, CSAMPLE gain21,
                                  const CSAMPLE* pSrc22, CSAMPLE gain22,
                                  const CSAMPLE* pSrc23, CSAMPLE gain23,
                                  const CSAMPLE* pSrc24, CSAMPLE gain24,
                                  const CSAMPLE* pSrc25, CSAMPLE gain25,
                                  const CSAMPLE* pSrc26, CSAMPLE gain26,
                                  const CSAMPLE* pSrc27, CSAMPLE gain27,
                                  const CSAMPLE* pSrc28, CSAMPLE gain28,
                                  const CSAMPLE* pSrc29, CSAMPLE gain29,
                                  const CSAMPLE* pSrc30, CSAMPLE gain30,
                                  const CSAMPLE* pSrc31, CSAMPLE gain31,
                                  int iNumSamples) {
    if (gain0 == 0.0) {
        copy31WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain1 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain2 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain3 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain4 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain5 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain6 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain7 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain8 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain9 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain10 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain11 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain12 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain13 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain14 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain15 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain16 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain17 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain18 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain19 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain20 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain21 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain22 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain23 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain24 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain25 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain26 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain27 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain28 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain29 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain30 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain31 == 0.0) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16 +
                   pSrc17[i] * gain17 +
                   pSrc18[i] * gain18 +
                   pSrc19[i] * gain19 +
                   pSrc20[i] * gain20 +
                   pSrc21[i] * gain21 +
                   pSrc22[i] * gain22 +
                   pSrc23[i] * gain23 +
                   pSrc24[i] * gain24 +
                   pSrc25[i] * gain25 +
                   pSrc26[i] * gain26 +
                   pSrc27[i] * gain27 +
                   pSrc28[i] * gain28 +
                   pSrc29[i] * gain29 +
                   pSrc30[i] * gain30 +
                   pSrc31[i] * gain31;
    }
}
static inline void copy32WithRampingGain(CSAMPLE* pDest,
                                         const CSAMPLE* pSrc0, CSAMPLE gain0in, CSAMPLE gain0out,
                                         const CSAMPLE* pSrc1, CSAMPLE gain1in, CSAMPLE gain1out,
                                         const CSAMPLE* pSrc2, CSAMPLE gain2in, CSAMPLE gain2out,
                                         const CSAMPLE* pSrc3, CSAMPLE gain3in, CSAMPLE gain3out,
                                         const CSAMPLE* pSrc4, CSAMPLE gain4in, CSAMPLE gain4out,
                                         const CSAMPLE* pSrc5, CSAMPLE gain5in, CSAMPLE gain5out,
                                         const CSAMPLE* pSrc6, CSAMPLE gain6in, CSAMPLE gain6out,
                                         const CSAMPLE* pSrc7, CSAMPLE gain7in, CSAMPLE gain7out,
                                         const CSAMPLE* pSrc8, CSAMPLE gain8in, CSAMPLE gain8out,
                                         const CSAMPLE* pSrc9, CSAMPLE gain9in, CSAMPLE gain9out,
                                         const CSAMPLE* pSrc10, CSAMPLE gain10in, CSAMPLE gain10out,
                                         const CSAMPLE* pSrc11, CSAMPLE gain11in, CSAMPLE gain11out,
                                         const CSAMPLE* pSrc12, CSAMPLE gain12in, CSAMPLE gain12out,
                                         const CSAMPLE* pSrc13, CSAMPLE gain13in, CSAMPLE gain13out,
                                         const CSAMPLE* pSrc14, CSAMPLE gain14in, CSAMPLE gain14out,
                                         const CSAMPLE* pSrc15, CSAMPLE gain15in, CSAMPLE gain15out,
                                         const CSAMPLE* pSrc16, CSAMPLE gain16in, CSAMPLE gain16out,
                                         const CSAMPLE* pSrc17, CSAMPLE gain17in, CSAMPLE gain17out,
                                         const CSAMPLE* pSrc18, CSAMPLE gain18in, CSAMPLE gain18out,
                                         const CSAMPLE* pSrc19, CSAMPLE gain19in, CSAMPLE gain19out,
                                         const CSAMPLE* pSrc20, CSAMPLE gain20in, CSAMPLE gain20out,
                                         const CSAMPLE* pSrc21, CSAMPLE gain21in, CSAMPLE gain21out,
                                         const CSAMPLE* pSrc22, CSAMPLE gain22in, CSAMPLE gain22out,
                                         const CSAMPLE* pSrc23, CSAMPLE gain23in, CSAMPLE gain23out,
                                         const CSAMPLE* pSrc24, CSAMPLE gain24in, CSAMPLE gain24out,
                                         const CSAMPLE* pSrc25, CSAMPLE gain25in, CSAMPLE gain25out,
                                         const CSAMPLE* pSrc26, CSAMPLE gain26in, CSAMPLE gain26out,
                                         const CSAMPLE* pSrc27, CSAMPLE gain27in, CSAMPLE gain27out,
                                         const CSAMPLE* pSrc28, CSAMPLE gain28in, CSAMPLE gain28out,
                                         const CSAMPLE* pSrc29, CSAMPLE gain29in, CSAMPLE gain29out,
                                         const CSAMPLE* pSrc30, CSAMPLE gain30in, CSAMPLE gain30out,
                                         const CSAMPLE* pSrc31, CSAMPLE gain31in, CSAMPLE gain31out,
                                         int iNumSamples) {
    if (gain0in == 0.0 && gain0out == 0.0) {
        copy31WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain1in == 0.0 && gain1out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain2in == 0.0 && gain2out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain3in == 0.0 && gain3out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain4in == 0.0 && gain4out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain5in == 0.0 && gain5out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain6in == 0.0 && gain6out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain7in == 0.0 && gain7out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain8in == 0.0 && gain8out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain9in == 0.0 && gain9out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain10in == 0.0 && gain10out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain11in == 0.0 && gain11out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain12in == 0.0 && gain12out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain13in == 0.0 && gain13out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain14in == 0.0 && gain14out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain15in == 0.0 && gain15out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain16in == 0.0 && gain16out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain17in == 0.0 && gain17out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain18in == 0.0 && gain18out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain19in == 0.0 && gain19out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain20in == 0.0 && gain20out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain21in == 0.0 && gain21out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain22in == 0.0 && gain22out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain23in == 0.0 && gain23out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain24in == 0.0 && gain24out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain25in == 0.0 && gain25out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain26in == 0.0 && gain26out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain27in == 0.0 && gain27out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain28in == 0.0 && gain28out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain29in == 0.0 && gain29out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain30in == 0.0 && gain30out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain31in == 0.0 && gain31out == 0.0) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    const CSAMPLE delta0 = 2.0 * (gain0out - gain0in) / iNumSamples;
    CSAMPLE gain0 = gain0in;
    const CSAMPLE delta1 = 2.0 * (gain1out - gain1in) / iNumSamples;
    CSAMPLE gain1 = gain1in;
    const CSAMPLE delta2 = 2.0 * (gain2out - gain2in) / iNumSamples;
    CSAMPLE gain2 = gain2in;
    const CSAMPLE delta3 = 2.0 * (gain3out - gain3in) / iNumSamples;
    CSAMPLE gain3 = gain3in;
    const CSAMPLE delta4 = 2.0 * (gain4out - gain4in) / iNumSamples;
    CSAMPLE gain4 = gain4in;
    const CSAMPLE delta5 = 2.0 * (gain5out - gain5in) / iNumSamples;
    CSAMPLE gain5 = gain5in;
    const CSAMPLE delta6 = 2.0 * (gain6out - gain6in) / iNumSamples;
    CSAMPLE gain6 = gain6in;
    const CSAMPLE delta7 = 2.0 * (gain7out - gain7in) / iNumSamples;
    CSAMPLE gain7 = gain7in;
    const CSAMPLE delta8 = 2.0 * (gain8out - gain8in) / iNumSamples;
    CSAMPLE gain8 = gain8in;
    const CSAMPLE delta9 = 2.0 * (gain9out - gain9in) / iNumSamples;
    CSAMPLE gain9 = gain9in;
    const CSAMPLE delta10 = 2.0 * (gain10out - gain10in) / iNumSamples;
    CSAMPLE gain10 = gain10in;
    const CSAMPLE delta11 = 2.0 * (gain11out - gain11in) / iNumSamples;
    CSAMPLE gain11 = gain11in;
    const CSAMPLE delta12 = 2.0 * (gain12out - gain12in) / iNumSamples;
    CSAMPLE gain12 = gain12in;
    const CSAMPLE delta13 = 2.0 * (gain13out - gain13in) / iNumSamples;
    CSAMPLE gain13 = gain13in;
    const CSAMPLE delta14 = 2.0 * (gain14out - gain14in) / iNumSamples;
    CSAMPLE gain14 = gain14in;
    const CSAMPLE delta15 = 2.0 * (gain15out - gain15in) / iNumSamples;
    CSAMPLE gain15 = gain15in;
    const CSAMPLE delta16 = 2.0 * (gain16out - gain16in) / iNumSamples;
    CSAMPLE gain16 = gain16in;
    const CSAMPLE delta17 = 2.0 * (gain17out - gain17in) / iNumSamples;
    CSAMPLE gain17 = gain17in;
    const CSAMPLE delta18 = 2.0 * (gain18out - gain18in) / iNumSamples;
    CSAMPLE gain18 = gain18in;
    const CSAMPLE delta19 = 2.0 * (gain19out - gain19in) / iNumSamples;
    CSAMPLE gain19 = gain19in;
    const CSAMPLE delta20 = 2.0 * (gain20out - gain20in) / iNumSamples;
    CSAMPLE gain20 = gain20in;
    const CSAMPLE delta21 = 2.0 * (gain21out - gain21in) / iNumSamples;
    CSAMPLE gain21 = gain21in;
    const CSAMPLE delta22 = 2.0 * (gain22out - gain22in) / iNumSamples;
    CSAMPLE gain22 = gain22in;
    const CSAMPLE delta23 = 2.0 * (gain23out - gain23in) / iNumSamples;
    CSAMPLE gain23 = gain23in;
    const CSAMPLE delta24 = 2.0 * (gain24out - gain24in) / iNumSamples;
    CSAMPLE gain24 = gain24in;
    const CSAMPLE delta25 = 2.0 * (gain25out - gain25in) / iNumSamples;
    CSAMPLE gain25 = gain25in;
    const CSAMPLE delta26 = 2.0 * (gain26out - gain26in) / iNumSamples;
    CSAMPLE gain26 = gain26in;
    const CSAMPLE delta27 = 2.0 * (gain27out - gain27in) / iNumSamples;
    CSAMPLE gain27 = gain27in;
    const CSAMPLE delta28 = 2.0 * (gain28out - gain28in) / iNumSamples;
    CSAMPLE gain28 = gain28in;
    const CSAMPLE delta29 = 2.0 * (gain29out - gain29in) / iNumSamples;
    CSAMPLE gain29 = gain29in;
    const CSAMPLE delta30 = 2.0 * (gain30out - gain30in) / iNumSamples;
    CSAMPLE gain30 = gain30in;
    const CSAMPLE delta31 = 2.0 * (gain31out - gain31in) / iNumSamples;
    CSAMPLE gain31 = gain31in;
    for (int i = 0; i < iNumSamples; i += 2, gain0 += delta0, gain1 += delta1, gain2 += delta2, gain3 += delta3, gain4 += delta4, gain5 += delta5, gain6 += delta6, gain7 += delta7, gain8 += delta8, gain9 += delta9, gain10 += delta10, gain11 += delta11, gain12 += delta12, gain13 += delta13, gain14 += delta14, gain15 += delta15, gain16 += delta16, gain17 += delta17, gain18 += delta18, gain19 += delta19, gain20 += delta20, gain21 += delta21, gain22 += delta22, gain23 += delta23, gain24 += delta24, gain25 += delta25, gain26 += delta26, gain27 += delta27, gain28 += delta28, gain29 += delta29, gain30 += delta30, gain31 += delta31) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5 +
                   pSrc6[i] * gain6 +
                   pSrc7[i] * gain7 +
                   pSrc8[i] * gain8 +
                   pSrc9[i] * gain9 +
                   pSrc10[i] * gain10 +
                   pSrc11[i] * gain11 +
                   pSrc12[i] * gain12 +
                   pSrc13[i] * gain13 +
                   pSrc14[i] * gain14 +
                   pSrc15[i] * gain15 +
                   pSrc16[i] * gain16 +
                   pSrc17[i] * gain17 +
                   pSrc18[i] * gain18 +
                   pSrc19[i] * gain19 +
                   pSrc20[i] * gain20 +
                   pSrc21[i] * gain21 +
                   pSrc22[i] * gain22 +
                   pSrc23[i] * gain23 +
                   pSrc24[i] * gain24 +
                   pSrc25[i] * gain25 +
                   pSrc26[i] * gain26 +
                   pSrc27[i] * gain27 +
                   pSrc28[i] * gain28 +
                   pSrc29[i] * gain29 +
                   pSrc30[i] * gain30 +
                   pSrc31[i] * gain31;
        pDest[i + 1] = pSrc0[i + 1] * gain0 +
                       pSrc1[i + 1] * gain1 +
                       pSrc2[i + 1] * gain2 +
                       pSrc3[i + 1] * gain3 +
                       pSrc4[i + 1] * gain4 +
                       pSrc5[i + 1] * gain5 +
                       pSrc6[i + 1] * gain6 +
                       pSrc7[i + 1] * gain7 +
                       pSrc8[i + 1] * gain8 +
                       pSrc9[i + 1] * gain9 +
                       pSrc10[i + 1] * gain10 +
                       pSrc11[i + 1] * gain11 +
                       pSrc12[i + 1] * gain12 +
                       pSrc13[i + 1] * gain13 +
                       pSrc14[i + 1] * gain14 +
                       pSrc15[i + 1] * gain15 +
                       pSrc16[i + 1] * gain16 +
                       pSrc17[i + 1] * gain17 +
                       pSrc18[i + 1] * gain18 +
                       pSrc19[i + 1] * gain19 +
                       pSrc20[i + 1] * gain20 +
                       pSrc21[i + 1] * gain21 +
                       pSrc22[i + 1] * gain22 +
                       pSrc23[i + 1] * gain23 +
                       pSrc24[i + 1] * gain24 +
                       pSrc25[i + 1] * gain25 +
                       pSrc26[i + 1] * gain26 +
                       pSrc27[i + 1] * gain27 +
                       pSrc28[i + 1] * gain28 +
                       pSrc29[i + 1] * gain29 +
                       pSrc30[i + 1] * gain30 +
                       pSrc31[i + 1] * gain31;
    }
}
#endif /* SAMPLEUTILAUTOGEN_H */
