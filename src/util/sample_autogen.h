#pragma once
////////////////////////////////////////////////////////
// THIS FILE IS AUTO-GENERATED. DO NOT EDIT DIRECTLY! //
// SEE tools/generate_sample_functions.py             //
////////////////////////////////////////////////////////
static inline void copy1WithGain(CSAMPLE* M_RESTRICT pDest,
                                 const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                 int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        clear(pDest, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0;
    }
}
static inline void copy1WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                        const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                        int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        clear(pDest, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0;
    }
}
static inline void copy2WithGain(CSAMPLE* M_RESTRICT pDest,
                                 const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                 const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                 int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy1WithGain(pDest, pSrc1, gain1, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy1WithGain(pDest, pSrc0, gain0, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1;
    }
}
static inline void copy2WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                        const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                        const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                        int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy1WithRampingGain(pDest, pSrc1, gain1in, gain1out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy1WithRampingGain(pDest, pSrc0, gain0in, gain0out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1;
    }
}
static inline void copy3WithGain(CSAMPLE* M_RESTRICT pDest,
                                 const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                 const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                 const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                 int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy2WithGain(pDest, pSrc1, gain1, pSrc2, gain2, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy2WithGain(pDest, pSrc0, gain0, pSrc2, gain2, iNumSamples);
        return;
    }
    if (gain2 == CSAMPLE_GAIN_ZERO) {
        copy2WithGain(pDest, pSrc0, gain0, pSrc1, gain1, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2;
    }
}
static inline void copy3WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                        const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                        const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                        const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                        int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy2WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy2WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, iNumSamples);
        return;
    }
    if (gain2in == CSAMPLE_GAIN_ZERO && gain2out == CSAMPLE_GAIN_ZERO) {
        copy2WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    const CSAMPLE_GAIN gain_delta2 = (gain2out - gain2in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain2 = gain2in + gain_delta2;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        const CSAMPLE_GAIN gain2 = start_gain2 + gain_delta2 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1 +
                       pSrc2[i * 2] * gain2;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1 +
                           pSrc2[i * 2 + 1] * gain2;
    }
}
static inline void copy4WithGain(CSAMPLE* M_RESTRICT pDest,
                                 const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                 const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                 const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                 const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3,
                                 int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy3WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy3WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, iNumSamples);
        return;
    }
    if (gain2 == CSAMPLE_GAIN_ZERO) {
        copy3WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, iNumSamples);
        return;
    }
    if (gain3 == CSAMPLE_GAIN_ZERO) {
        copy3WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3;
    }
}
static inline void copy4WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                        const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                        const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                        const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                        const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3in, CSAMPLE_GAIN gain3out,
                                        int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy3WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy3WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, iNumSamples);
        return;
    }
    if (gain2in == CSAMPLE_GAIN_ZERO && gain2out == CSAMPLE_GAIN_ZERO) {
        copy3WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, iNumSamples);
        return;
    }
    if (gain3in == CSAMPLE_GAIN_ZERO && gain3out == CSAMPLE_GAIN_ZERO) {
        copy3WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    const CSAMPLE_GAIN gain_delta2 = (gain2out - gain2in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain2 = gain2in + gain_delta2;
    const CSAMPLE_GAIN gain_delta3 = (gain3out - gain3in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain3 = gain3in + gain_delta3;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        const CSAMPLE_GAIN gain2 = start_gain2 + gain_delta2 * i;
        const CSAMPLE_GAIN gain3 = start_gain3 + gain_delta3 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1 +
                       pSrc2[i * 2] * gain2 +
                       pSrc3[i * 2] * gain3;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1 +
                           pSrc2[i * 2 + 1] * gain2 +
                           pSrc3[i * 2 + 1] * gain3;
    }
}
static inline void copy5WithGain(CSAMPLE* M_RESTRICT pDest,
                                 const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                 const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                 const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                 const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3,
                                 const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4,
                                 int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy4WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy4WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, iNumSamples);
        return;
    }
    if (gain2 == CSAMPLE_GAIN_ZERO) {
        copy4WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, iNumSamples);
        return;
    }
    if (gain3 == CSAMPLE_GAIN_ZERO) {
        copy4WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, iNumSamples);
        return;
    }
    if (gain4 == CSAMPLE_GAIN_ZERO) {
        copy4WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4;
    }
}
static inline void copy5WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                        const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                        const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                        const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                        const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3in, CSAMPLE_GAIN gain3out,
                                        const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4in, CSAMPLE_GAIN gain4out,
                                        int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy4WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy4WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, iNumSamples);
        return;
    }
    if (gain2in == CSAMPLE_GAIN_ZERO && gain2out == CSAMPLE_GAIN_ZERO) {
        copy4WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, iNumSamples);
        return;
    }
    if (gain3in == CSAMPLE_GAIN_ZERO && gain3out == CSAMPLE_GAIN_ZERO) {
        copy4WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, iNumSamples);
        return;
    }
    if (gain4in == CSAMPLE_GAIN_ZERO && gain4out == CSAMPLE_GAIN_ZERO) {
        copy4WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    const CSAMPLE_GAIN gain_delta2 = (gain2out - gain2in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain2 = gain2in + gain_delta2;
    const CSAMPLE_GAIN gain_delta3 = (gain3out - gain3in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain3 = gain3in + gain_delta3;
    const CSAMPLE_GAIN gain_delta4 = (gain4out - gain4in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain4 = gain4in + gain_delta4;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        const CSAMPLE_GAIN gain2 = start_gain2 + gain_delta2 * i;
        const CSAMPLE_GAIN gain3 = start_gain3 + gain_delta3 * i;
        const CSAMPLE_GAIN gain4 = start_gain4 + gain_delta4 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1 +
                       pSrc2[i * 2] * gain2 +
                       pSrc3[i * 2] * gain3 +
                       pSrc4[i * 2] * gain4;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1 +
                           pSrc2[i * 2 + 1] * gain2 +
                           pSrc3[i * 2 + 1] * gain3 +
                           pSrc4[i * 2 + 1] * gain4;
    }
}
static inline void copy6WithGain(CSAMPLE* M_RESTRICT pDest,
                                 const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                 const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                 const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                 const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3,
                                 const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4,
                                 const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5,
                                 int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy5WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy5WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, iNumSamples);
        return;
    }
    if (gain2 == CSAMPLE_GAIN_ZERO) {
        copy5WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, iNumSamples);
        return;
    }
    if (gain3 == CSAMPLE_GAIN_ZERO) {
        copy5WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, iNumSamples);
        return;
    }
    if (gain4 == CSAMPLE_GAIN_ZERO) {
        copy5WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, iNumSamples);
        return;
    }
    if (gain5 == CSAMPLE_GAIN_ZERO) {
        copy5WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples; ++i) {
        pDest[i] = pSrc0[i] * gain0 +
                   pSrc1[i] * gain1 +
                   pSrc2[i] * gain2 +
                   pSrc3[i] * gain3 +
                   pSrc4[i] * gain4 +
                   pSrc5[i] * gain5;
    }
}
static inline void copy6WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                        const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                        const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                        const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                        const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3in, CSAMPLE_GAIN gain3out,
                                        const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4in, CSAMPLE_GAIN gain4out,
                                        const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5in, CSAMPLE_GAIN gain5out,
                                        int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy5WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy5WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, iNumSamples);
        return;
    }
    if (gain2in == CSAMPLE_GAIN_ZERO && gain2out == CSAMPLE_GAIN_ZERO) {
        copy5WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, iNumSamples);
        return;
    }
    if (gain3in == CSAMPLE_GAIN_ZERO && gain3out == CSAMPLE_GAIN_ZERO) {
        copy5WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, iNumSamples);
        return;
    }
    if (gain4in == CSAMPLE_GAIN_ZERO && gain4out == CSAMPLE_GAIN_ZERO) {
        copy5WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, iNumSamples);
        return;
    }
    if (gain5in == CSAMPLE_GAIN_ZERO && gain5out == CSAMPLE_GAIN_ZERO) {
        copy5WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    const CSAMPLE_GAIN gain_delta2 = (gain2out - gain2in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain2 = gain2in + gain_delta2;
    const CSAMPLE_GAIN gain_delta3 = (gain3out - gain3in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain3 = gain3in + gain_delta3;
    const CSAMPLE_GAIN gain_delta4 = (gain4out - gain4in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain4 = gain4in + gain_delta4;
    const CSAMPLE_GAIN gain_delta5 = (gain5out - gain5in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain5 = gain5in + gain_delta5;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        const CSAMPLE_GAIN gain2 = start_gain2 + gain_delta2 * i;
        const CSAMPLE_GAIN gain3 = start_gain3 + gain_delta3 * i;
        const CSAMPLE_GAIN gain4 = start_gain4 + gain_delta4 * i;
        const CSAMPLE_GAIN gain5 = start_gain5 + gain_delta5 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1 +
                       pSrc2[i * 2] * gain2 +
                       pSrc3[i * 2] * gain3 +
                       pSrc4[i * 2] * gain4 +
                       pSrc5[i * 2] * gain5;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1 +
                           pSrc2[i * 2 + 1] * gain2 +
                           pSrc3[i * 2 + 1] * gain3 +
                           pSrc4[i * 2 + 1] * gain4 +
                           pSrc5[i * 2 + 1] * gain5;
    }
}
static inline void copy7WithGain(CSAMPLE* M_RESTRICT pDest,
                                 const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                 const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                 const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                 const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3,
                                 const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4,
                                 const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5,
                                 const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6,
                                 int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy6WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy6WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, iNumSamples);
        return;
    }
    if (gain2 == CSAMPLE_GAIN_ZERO) {
        copy6WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, iNumSamples);
        return;
    }
    if (gain3 == CSAMPLE_GAIN_ZERO) {
        copy6WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, iNumSamples);
        return;
    }
    if (gain4 == CSAMPLE_GAIN_ZERO) {
        copy6WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, iNumSamples);
        return;
    }
    if (gain5 == CSAMPLE_GAIN_ZERO) {
        copy6WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, iNumSamples);
        return;
    }
    if (gain6 == CSAMPLE_GAIN_ZERO) {
        copy6WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
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
static inline void copy7WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                        const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                        const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                        const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                        const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3in, CSAMPLE_GAIN gain3out,
                                        const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4in, CSAMPLE_GAIN gain4out,
                                        const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5in, CSAMPLE_GAIN gain5out,
                                        const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6in, CSAMPLE_GAIN gain6out,
                                        int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy6WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy6WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, iNumSamples);
        return;
    }
    if (gain2in == CSAMPLE_GAIN_ZERO && gain2out == CSAMPLE_GAIN_ZERO) {
        copy6WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, iNumSamples);
        return;
    }
    if (gain3in == CSAMPLE_GAIN_ZERO && gain3out == CSAMPLE_GAIN_ZERO) {
        copy6WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, iNumSamples);
        return;
    }
    if (gain4in == CSAMPLE_GAIN_ZERO && gain4out == CSAMPLE_GAIN_ZERO) {
        copy6WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, iNumSamples);
        return;
    }
    if (gain5in == CSAMPLE_GAIN_ZERO && gain5out == CSAMPLE_GAIN_ZERO) {
        copy6WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, iNumSamples);
        return;
    }
    if (gain6in == CSAMPLE_GAIN_ZERO && gain6out == CSAMPLE_GAIN_ZERO) {
        copy6WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    const CSAMPLE_GAIN gain_delta2 = (gain2out - gain2in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain2 = gain2in + gain_delta2;
    const CSAMPLE_GAIN gain_delta3 = (gain3out - gain3in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain3 = gain3in + gain_delta3;
    const CSAMPLE_GAIN gain_delta4 = (gain4out - gain4in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain4 = gain4in + gain_delta4;
    const CSAMPLE_GAIN gain_delta5 = (gain5out - gain5in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain5 = gain5in + gain_delta5;
    const CSAMPLE_GAIN gain_delta6 = (gain6out - gain6in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain6 = gain6in + gain_delta6;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        const CSAMPLE_GAIN gain2 = start_gain2 + gain_delta2 * i;
        const CSAMPLE_GAIN gain3 = start_gain3 + gain_delta3 * i;
        const CSAMPLE_GAIN gain4 = start_gain4 + gain_delta4 * i;
        const CSAMPLE_GAIN gain5 = start_gain5 + gain_delta5 * i;
        const CSAMPLE_GAIN gain6 = start_gain6 + gain_delta6 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1 +
                       pSrc2[i * 2] * gain2 +
                       pSrc3[i * 2] * gain3 +
                       pSrc4[i * 2] * gain4 +
                       pSrc5[i * 2] * gain5 +
                       pSrc6[i * 2] * gain6;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1 +
                           pSrc2[i * 2 + 1] * gain2 +
                           pSrc3[i * 2 + 1] * gain3 +
                           pSrc4[i * 2 + 1] * gain4 +
                           pSrc5[i * 2 + 1] * gain5 +
                           pSrc6[i * 2 + 1] * gain6;
    }
}
static inline void copy8WithGain(CSAMPLE* M_RESTRICT pDest,
                                 const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                 const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                 const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                 const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3,
                                 const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4,
                                 const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5,
                                 const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6,
                                 const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7,
                                 int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy7WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy7WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, iNumSamples);
        return;
    }
    if (gain2 == CSAMPLE_GAIN_ZERO) {
        copy7WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, iNumSamples);
        return;
    }
    if (gain3 == CSAMPLE_GAIN_ZERO) {
        copy7WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, iNumSamples);
        return;
    }
    if (gain4 == CSAMPLE_GAIN_ZERO) {
        copy7WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, iNumSamples);
        return;
    }
    if (gain5 == CSAMPLE_GAIN_ZERO) {
        copy7WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, iNumSamples);
        return;
    }
    if (gain6 == CSAMPLE_GAIN_ZERO) {
        copy7WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, iNumSamples);
        return;
    }
    if (gain7 == CSAMPLE_GAIN_ZERO) {
        copy7WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
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
static inline void copy8WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                        const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                        const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                        const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                        const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3in, CSAMPLE_GAIN gain3out,
                                        const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4in, CSAMPLE_GAIN gain4out,
                                        const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5in, CSAMPLE_GAIN gain5out,
                                        const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6in, CSAMPLE_GAIN gain6out,
                                        const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7in, CSAMPLE_GAIN gain7out,
                                        int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy7WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy7WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, iNumSamples);
        return;
    }
    if (gain2in == CSAMPLE_GAIN_ZERO && gain2out == CSAMPLE_GAIN_ZERO) {
        copy7WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, iNumSamples);
        return;
    }
    if (gain3in == CSAMPLE_GAIN_ZERO && gain3out == CSAMPLE_GAIN_ZERO) {
        copy7WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, iNumSamples);
        return;
    }
    if (gain4in == CSAMPLE_GAIN_ZERO && gain4out == CSAMPLE_GAIN_ZERO) {
        copy7WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, iNumSamples);
        return;
    }
    if (gain5in == CSAMPLE_GAIN_ZERO && gain5out == CSAMPLE_GAIN_ZERO) {
        copy7WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, iNumSamples);
        return;
    }
    if (gain6in == CSAMPLE_GAIN_ZERO && gain6out == CSAMPLE_GAIN_ZERO) {
        copy7WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, iNumSamples);
        return;
    }
    if (gain7in == CSAMPLE_GAIN_ZERO && gain7out == CSAMPLE_GAIN_ZERO) {
        copy7WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    const CSAMPLE_GAIN gain_delta2 = (gain2out - gain2in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain2 = gain2in + gain_delta2;
    const CSAMPLE_GAIN gain_delta3 = (gain3out - gain3in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain3 = gain3in + gain_delta3;
    const CSAMPLE_GAIN gain_delta4 = (gain4out - gain4in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain4 = gain4in + gain_delta4;
    const CSAMPLE_GAIN gain_delta5 = (gain5out - gain5in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain5 = gain5in + gain_delta5;
    const CSAMPLE_GAIN gain_delta6 = (gain6out - gain6in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain6 = gain6in + gain_delta6;
    const CSAMPLE_GAIN gain_delta7 = (gain7out - gain7in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain7 = gain7in + gain_delta7;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        const CSAMPLE_GAIN gain2 = start_gain2 + gain_delta2 * i;
        const CSAMPLE_GAIN gain3 = start_gain3 + gain_delta3 * i;
        const CSAMPLE_GAIN gain4 = start_gain4 + gain_delta4 * i;
        const CSAMPLE_GAIN gain5 = start_gain5 + gain_delta5 * i;
        const CSAMPLE_GAIN gain6 = start_gain6 + gain_delta6 * i;
        const CSAMPLE_GAIN gain7 = start_gain7 + gain_delta7 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1 +
                       pSrc2[i * 2] * gain2 +
                       pSrc3[i * 2] * gain3 +
                       pSrc4[i * 2] * gain4 +
                       pSrc5[i * 2] * gain5 +
                       pSrc6[i * 2] * gain6 +
                       pSrc7[i * 2] * gain7;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1 +
                           pSrc2[i * 2 + 1] * gain2 +
                           pSrc3[i * 2 + 1] * gain3 +
                           pSrc4[i * 2 + 1] * gain4 +
                           pSrc5[i * 2 + 1] * gain5 +
                           pSrc6[i * 2 + 1] * gain6 +
                           pSrc7[i * 2 + 1] * gain7;
    }
}
static inline void copy9WithGain(CSAMPLE* M_RESTRICT pDest,
                                 const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                 const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                 const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                 const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3,
                                 const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4,
                                 const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5,
                                 const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6,
                                 const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7,
                                 const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8,
                                 int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy8WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy8WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, iNumSamples);
        return;
    }
    if (gain2 == CSAMPLE_GAIN_ZERO) {
        copy8WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, iNumSamples);
        return;
    }
    if (gain3 == CSAMPLE_GAIN_ZERO) {
        copy8WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, iNumSamples);
        return;
    }
    if (gain4 == CSAMPLE_GAIN_ZERO) {
        copy8WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, iNumSamples);
        return;
    }
    if (gain5 == CSAMPLE_GAIN_ZERO) {
        copy8WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, iNumSamples);
        return;
    }
    if (gain6 == CSAMPLE_GAIN_ZERO) {
        copy8WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, iNumSamples);
        return;
    }
    if (gain7 == CSAMPLE_GAIN_ZERO) {
        copy8WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, iNumSamples);
        return;
    }
    if (gain8 == CSAMPLE_GAIN_ZERO) {
        copy8WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
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
static inline void copy9WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                        const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                        const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                        const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                        const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3in, CSAMPLE_GAIN gain3out,
                                        const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4in, CSAMPLE_GAIN gain4out,
                                        const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5in, CSAMPLE_GAIN gain5out,
                                        const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6in, CSAMPLE_GAIN gain6out,
                                        const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7in, CSAMPLE_GAIN gain7out,
                                        const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8in, CSAMPLE_GAIN gain8out,
                                        int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy8WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy8WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, iNumSamples);
        return;
    }
    if (gain2in == CSAMPLE_GAIN_ZERO && gain2out == CSAMPLE_GAIN_ZERO) {
        copy8WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, iNumSamples);
        return;
    }
    if (gain3in == CSAMPLE_GAIN_ZERO && gain3out == CSAMPLE_GAIN_ZERO) {
        copy8WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, iNumSamples);
        return;
    }
    if (gain4in == CSAMPLE_GAIN_ZERO && gain4out == CSAMPLE_GAIN_ZERO) {
        copy8WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, iNumSamples);
        return;
    }
    if (gain5in == CSAMPLE_GAIN_ZERO && gain5out == CSAMPLE_GAIN_ZERO) {
        copy8WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, iNumSamples);
        return;
    }
    if (gain6in == CSAMPLE_GAIN_ZERO && gain6out == CSAMPLE_GAIN_ZERO) {
        copy8WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, iNumSamples);
        return;
    }
    if (gain7in == CSAMPLE_GAIN_ZERO && gain7out == CSAMPLE_GAIN_ZERO) {
        copy8WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, iNumSamples);
        return;
    }
    if (gain8in == CSAMPLE_GAIN_ZERO && gain8out == CSAMPLE_GAIN_ZERO) {
        copy8WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    const CSAMPLE_GAIN gain_delta2 = (gain2out - gain2in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain2 = gain2in + gain_delta2;
    const CSAMPLE_GAIN gain_delta3 = (gain3out - gain3in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain3 = gain3in + gain_delta3;
    const CSAMPLE_GAIN gain_delta4 = (gain4out - gain4in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain4 = gain4in + gain_delta4;
    const CSAMPLE_GAIN gain_delta5 = (gain5out - gain5in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain5 = gain5in + gain_delta5;
    const CSAMPLE_GAIN gain_delta6 = (gain6out - gain6in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain6 = gain6in + gain_delta6;
    const CSAMPLE_GAIN gain_delta7 = (gain7out - gain7in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain7 = gain7in + gain_delta7;
    const CSAMPLE_GAIN gain_delta8 = (gain8out - gain8in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain8 = gain8in + gain_delta8;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        const CSAMPLE_GAIN gain2 = start_gain2 + gain_delta2 * i;
        const CSAMPLE_GAIN gain3 = start_gain3 + gain_delta3 * i;
        const CSAMPLE_GAIN gain4 = start_gain4 + gain_delta4 * i;
        const CSAMPLE_GAIN gain5 = start_gain5 + gain_delta5 * i;
        const CSAMPLE_GAIN gain6 = start_gain6 + gain_delta6 * i;
        const CSAMPLE_GAIN gain7 = start_gain7 + gain_delta7 * i;
        const CSAMPLE_GAIN gain8 = start_gain8 + gain_delta8 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1 +
                       pSrc2[i * 2] * gain2 +
                       pSrc3[i * 2] * gain3 +
                       pSrc4[i * 2] * gain4 +
                       pSrc5[i * 2] * gain5 +
                       pSrc6[i * 2] * gain6 +
                       pSrc7[i * 2] * gain7 +
                       pSrc8[i * 2] * gain8;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1 +
                           pSrc2[i * 2 + 1] * gain2 +
                           pSrc3[i * 2 + 1] * gain3 +
                           pSrc4[i * 2 + 1] * gain4 +
                           pSrc5[i * 2 + 1] * gain5 +
                           pSrc6[i * 2 + 1] * gain6 +
                           pSrc7[i * 2 + 1] * gain7 +
                           pSrc8[i * 2 + 1] * gain8;
    }
}
static inline void copy10WithGain(CSAMPLE* M_RESTRICT pDest,
                                  const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                  const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                  const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                  const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3,
                                  const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4,
                                  const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5,
                                  const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6,
                                  const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7,
                                  const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8,
                                  const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9,
                                  int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy9WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy9WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, iNumSamples);
        return;
    }
    if (gain2 == CSAMPLE_GAIN_ZERO) {
        copy9WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, iNumSamples);
        return;
    }
    if (gain3 == CSAMPLE_GAIN_ZERO) {
        copy9WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, iNumSamples);
        return;
    }
    if (gain4 == CSAMPLE_GAIN_ZERO) {
        copy9WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, iNumSamples);
        return;
    }
    if (gain5 == CSAMPLE_GAIN_ZERO) {
        copy9WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, iNumSamples);
        return;
    }
    if (gain6 == CSAMPLE_GAIN_ZERO) {
        copy9WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, iNumSamples);
        return;
    }
    if (gain7 == CSAMPLE_GAIN_ZERO) {
        copy9WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, iNumSamples);
        return;
    }
    if (gain8 == CSAMPLE_GAIN_ZERO) {
        copy9WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, iNumSamples);
        return;
    }
    if (gain9 == CSAMPLE_GAIN_ZERO) {
        copy9WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
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
static inline void copy10WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                         const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                         const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                         const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                         const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3in, CSAMPLE_GAIN gain3out,
                                         const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4in, CSAMPLE_GAIN gain4out,
                                         const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5in, CSAMPLE_GAIN gain5out,
                                         const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6in, CSAMPLE_GAIN gain6out,
                                         const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7in, CSAMPLE_GAIN gain7out,
                                         const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8in, CSAMPLE_GAIN gain8out,
                                         const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9in, CSAMPLE_GAIN gain9out,
                                         int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy9WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy9WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, iNumSamples);
        return;
    }
    if (gain2in == CSAMPLE_GAIN_ZERO && gain2out == CSAMPLE_GAIN_ZERO) {
        copy9WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, iNumSamples);
        return;
    }
    if (gain3in == CSAMPLE_GAIN_ZERO && gain3out == CSAMPLE_GAIN_ZERO) {
        copy9WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, iNumSamples);
        return;
    }
    if (gain4in == CSAMPLE_GAIN_ZERO && gain4out == CSAMPLE_GAIN_ZERO) {
        copy9WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, iNumSamples);
        return;
    }
    if (gain5in == CSAMPLE_GAIN_ZERO && gain5out == CSAMPLE_GAIN_ZERO) {
        copy9WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, iNumSamples);
        return;
    }
    if (gain6in == CSAMPLE_GAIN_ZERO && gain6out == CSAMPLE_GAIN_ZERO) {
        copy9WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, iNumSamples);
        return;
    }
    if (gain7in == CSAMPLE_GAIN_ZERO && gain7out == CSAMPLE_GAIN_ZERO) {
        copy9WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, iNumSamples);
        return;
    }
    if (gain8in == CSAMPLE_GAIN_ZERO && gain8out == CSAMPLE_GAIN_ZERO) {
        copy9WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, iNumSamples);
        return;
    }
    if (gain9in == CSAMPLE_GAIN_ZERO && gain9out == CSAMPLE_GAIN_ZERO) {
        copy9WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    const CSAMPLE_GAIN gain_delta2 = (gain2out - gain2in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain2 = gain2in + gain_delta2;
    const CSAMPLE_GAIN gain_delta3 = (gain3out - gain3in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain3 = gain3in + gain_delta3;
    const CSAMPLE_GAIN gain_delta4 = (gain4out - gain4in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain4 = gain4in + gain_delta4;
    const CSAMPLE_GAIN gain_delta5 = (gain5out - gain5in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain5 = gain5in + gain_delta5;
    const CSAMPLE_GAIN gain_delta6 = (gain6out - gain6in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain6 = gain6in + gain_delta6;
    const CSAMPLE_GAIN gain_delta7 = (gain7out - gain7in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain7 = gain7in + gain_delta7;
    const CSAMPLE_GAIN gain_delta8 = (gain8out - gain8in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain8 = gain8in + gain_delta8;
    const CSAMPLE_GAIN gain_delta9 = (gain9out - gain9in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain9 = gain9in + gain_delta9;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        const CSAMPLE_GAIN gain2 = start_gain2 + gain_delta2 * i;
        const CSAMPLE_GAIN gain3 = start_gain3 + gain_delta3 * i;
        const CSAMPLE_GAIN gain4 = start_gain4 + gain_delta4 * i;
        const CSAMPLE_GAIN gain5 = start_gain5 + gain_delta5 * i;
        const CSAMPLE_GAIN gain6 = start_gain6 + gain_delta6 * i;
        const CSAMPLE_GAIN gain7 = start_gain7 + gain_delta7 * i;
        const CSAMPLE_GAIN gain8 = start_gain8 + gain_delta8 * i;
        const CSAMPLE_GAIN gain9 = start_gain9 + gain_delta9 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1 +
                       pSrc2[i * 2] * gain2 +
                       pSrc3[i * 2] * gain3 +
                       pSrc4[i * 2] * gain4 +
                       pSrc5[i * 2] * gain5 +
                       pSrc6[i * 2] * gain6 +
                       pSrc7[i * 2] * gain7 +
                       pSrc8[i * 2] * gain8 +
                       pSrc9[i * 2] * gain9;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1 +
                           pSrc2[i * 2 + 1] * gain2 +
                           pSrc3[i * 2 + 1] * gain3 +
                           pSrc4[i * 2 + 1] * gain4 +
                           pSrc5[i * 2 + 1] * gain5 +
                           pSrc6[i * 2 + 1] * gain6 +
                           pSrc7[i * 2 + 1] * gain7 +
                           pSrc8[i * 2 + 1] * gain8 +
                           pSrc9[i * 2 + 1] * gain9;
    }
}
static inline void copy11WithGain(CSAMPLE* M_RESTRICT pDest,
                                  const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                  const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                  const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                  const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3,
                                  const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4,
                                  const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5,
                                  const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6,
                                  const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7,
                                  const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8,
                                  const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9,
                                  const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10,
                                  int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy10WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy10WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, iNumSamples);
        return;
    }
    if (gain2 == CSAMPLE_GAIN_ZERO) {
        copy10WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, iNumSamples);
        return;
    }
    if (gain3 == CSAMPLE_GAIN_ZERO) {
        copy10WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, iNumSamples);
        return;
    }
    if (gain4 == CSAMPLE_GAIN_ZERO) {
        copy10WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, iNumSamples);
        return;
    }
    if (gain5 == CSAMPLE_GAIN_ZERO) {
        copy10WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, iNumSamples);
        return;
    }
    if (gain6 == CSAMPLE_GAIN_ZERO) {
        copy10WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, iNumSamples);
        return;
    }
    if (gain7 == CSAMPLE_GAIN_ZERO) {
        copy10WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, iNumSamples);
        return;
    }
    if (gain8 == CSAMPLE_GAIN_ZERO) {
        copy10WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, iNumSamples);
        return;
    }
    if (gain9 == CSAMPLE_GAIN_ZERO) {
        copy10WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, iNumSamples);
        return;
    }
    if (gain10 == CSAMPLE_GAIN_ZERO) {
        copy10WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
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
static inline void copy11WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                         const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                         const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                         const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                         const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3in, CSAMPLE_GAIN gain3out,
                                         const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4in, CSAMPLE_GAIN gain4out,
                                         const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5in, CSAMPLE_GAIN gain5out,
                                         const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6in, CSAMPLE_GAIN gain6out,
                                         const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7in, CSAMPLE_GAIN gain7out,
                                         const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8in, CSAMPLE_GAIN gain8out,
                                         const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9in, CSAMPLE_GAIN gain9out,
                                         const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10in, CSAMPLE_GAIN gain10out,
                                         int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy10WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy10WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, iNumSamples);
        return;
    }
    if (gain2in == CSAMPLE_GAIN_ZERO && gain2out == CSAMPLE_GAIN_ZERO) {
        copy10WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, iNumSamples);
        return;
    }
    if (gain3in == CSAMPLE_GAIN_ZERO && gain3out == CSAMPLE_GAIN_ZERO) {
        copy10WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, iNumSamples);
        return;
    }
    if (gain4in == CSAMPLE_GAIN_ZERO && gain4out == CSAMPLE_GAIN_ZERO) {
        copy10WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, iNumSamples);
        return;
    }
    if (gain5in == CSAMPLE_GAIN_ZERO && gain5out == CSAMPLE_GAIN_ZERO) {
        copy10WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, iNumSamples);
        return;
    }
    if (gain6in == CSAMPLE_GAIN_ZERO && gain6out == CSAMPLE_GAIN_ZERO) {
        copy10WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, iNumSamples);
        return;
    }
    if (gain7in == CSAMPLE_GAIN_ZERO && gain7out == CSAMPLE_GAIN_ZERO) {
        copy10WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, iNumSamples);
        return;
    }
    if (gain8in == CSAMPLE_GAIN_ZERO && gain8out == CSAMPLE_GAIN_ZERO) {
        copy10WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, iNumSamples);
        return;
    }
    if (gain9in == CSAMPLE_GAIN_ZERO && gain9out == CSAMPLE_GAIN_ZERO) {
        copy10WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, iNumSamples);
        return;
    }
    if (gain10in == CSAMPLE_GAIN_ZERO && gain10out == CSAMPLE_GAIN_ZERO) {
        copy10WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    const CSAMPLE_GAIN gain_delta2 = (gain2out - gain2in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain2 = gain2in + gain_delta2;
    const CSAMPLE_GAIN gain_delta3 = (gain3out - gain3in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain3 = gain3in + gain_delta3;
    const CSAMPLE_GAIN gain_delta4 = (gain4out - gain4in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain4 = gain4in + gain_delta4;
    const CSAMPLE_GAIN gain_delta5 = (gain5out - gain5in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain5 = gain5in + gain_delta5;
    const CSAMPLE_GAIN gain_delta6 = (gain6out - gain6in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain6 = gain6in + gain_delta6;
    const CSAMPLE_GAIN gain_delta7 = (gain7out - gain7in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain7 = gain7in + gain_delta7;
    const CSAMPLE_GAIN gain_delta8 = (gain8out - gain8in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain8 = gain8in + gain_delta8;
    const CSAMPLE_GAIN gain_delta9 = (gain9out - gain9in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain9 = gain9in + gain_delta9;
    const CSAMPLE_GAIN gain_delta10 = (gain10out - gain10in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain10 = gain10in + gain_delta10;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        const CSAMPLE_GAIN gain2 = start_gain2 + gain_delta2 * i;
        const CSAMPLE_GAIN gain3 = start_gain3 + gain_delta3 * i;
        const CSAMPLE_GAIN gain4 = start_gain4 + gain_delta4 * i;
        const CSAMPLE_GAIN gain5 = start_gain5 + gain_delta5 * i;
        const CSAMPLE_GAIN gain6 = start_gain6 + gain_delta6 * i;
        const CSAMPLE_GAIN gain7 = start_gain7 + gain_delta7 * i;
        const CSAMPLE_GAIN gain8 = start_gain8 + gain_delta8 * i;
        const CSAMPLE_GAIN gain9 = start_gain9 + gain_delta9 * i;
        const CSAMPLE_GAIN gain10 = start_gain10 + gain_delta10 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1 +
                       pSrc2[i * 2] * gain2 +
                       pSrc3[i * 2] * gain3 +
                       pSrc4[i * 2] * gain4 +
                       pSrc5[i * 2] * gain5 +
                       pSrc6[i * 2] * gain6 +
                       pSrc7[i * 2] * gain7 +
                       pSrc8[i * 2] * gain8 +
                       pSrc9[i * 2] * gain9 +
                       pSrc10[i * 2] * gain10;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1 +
                           pSrc2[i * 2 + 1] * gain2 +
                           pSrc3[i * 2 + 1] * gain3 +
                           pSrc4[i * 2 + 1] * gain4 +
                           pSrc5[i * 2 + 1] * gain5 +
                           pSrc6[i * 2 + 1] * gain6 +
                           pSrc7[i * 2 + 1] * gain7 +
                           pSrc8[i * 2 + 1] * gain8 +
                           pSrc9[i * 2 + 1] * gain9 +
                           pSrc10[i * 2 + 1] * gain10;
    }
}
static inline void copy12WithGain(CSAMPLE* M_RESTRICT pDest,
                                  const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                  const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                  const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                  const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3,
                                  const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4,
                                  const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5,
                                  const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6,
                                  const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7,
                                  const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8,
                                  const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9,
                                  const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10,
                                  const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11,
                                  int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy11WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy11WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, iNumSamples);
        return;
    }
    if (gain2 == CSAMPLE_GAIN_ZERO) {
        copy11WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, iNumSamples);
        return;
    }
    if (gain3 == CSAMPLE_GAIN_ZERO) {
        copy11WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, iNumSamples);
        return;
    }
    if (gain4 == CSAMPLE_GAIN_ZERO) {
        copy11WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, iNumSamples);
        return;
    }
    if (gain5 == CSAMPLE_GAIN_ZERO) {
        copy11WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, iNumSamples);
        return;
    }
    if (gain6 == CSAMPLE_GAIN_ZERO) {
        copy11WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, iNumSamples);
        return;
    }
    if (gain7 == CSAMPLE_GAIN_ZERO) {
        copy11WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, iNumSamples);
        return;
    }
    if (gain8 == CSAMPLE_GAIN_ZERO) {
        copy11WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, iNumSamples);
        return;
    }
    if (gain9 == CSAMPLE_GAIN_ZERO) {
        copy11WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, iNumSamples);
        return;
    }
    if (gain10 == CSAMPLE_GAIN_ZERO) {
        copy11WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, iNumSamples);
        return;
    }
    if (gain11 == CSAMPLE_GAIN_ZERO) {
        copy11WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
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
static inline void copy12WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                         const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                         const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                         const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                         const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3in, CSAMPLE_GAIN gain3out,
                                         const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4in, CSAMPLE_GAIN gain4out,
                                         const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5in, CSAMPLE_GAIN gain5out,
                                         const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6in, CSAMPLE_GAIN gain6out,
                                         const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7in, CSAMPLE_GAIN gain7out,
                                         const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8in, CSAMPLE_GAIN gain8out,
                                         const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9in, CSAMPLE_GAIN gain9out,
                                         const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10in, CSAMPLE_GAIN gain10out,
                                         const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11in, CSAMPLE_GAIN gain11out,
                                         int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy11WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy11WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, iNumSamples);
        return;
    }
    if (gain2in == CSAMPLE_GAIN_ZERO && gain2out == CSAMPLE_GAIN_ZERO) {
        copy11WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, iNumSamples);
        return;
    }
    if (gain3in == CSAMPLE_GAIN_ZERO && gain3out == CSAMPLE_GAIN_ZERO) {
        copy11WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, iNumSamples);
        return;
    }
    if (gain4in == CSAMPLE_GAIN_ZERO && gain4out == CSAMPLE_GAIN_ZERO) {
        copy11WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, iNumSamples);
        return;
    }
    if (gain5in == CSAMPLE_GAIN_ZERO && gain5out == CSAMPLE_GAIN_ZERO) {
        copy11WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, iNumSamples);
        return;
    }
    if (gain6in == CSAMPLE_GAIN_ZERO && gain6out == CSAMPLE_GAIN_ZERO) {
        copy11WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, iNumSamples);
        return;
    }
    if (gain7in == CSAMPLE_GAIN_ZERO && gain7out == CSAMPLE_GAIN_ZERO) {
        copy11WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, iNumSamples);
        return;
    }
    if (gain8in == CSAMPLE_GAIN_ZERO && gain8out == CSAMPLE_GAIN_ZERO) {
        copy11WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, iNumSamples);
        return;
    }
    if (gain9in == CSAMPLE_GAIN_ZERO && gain9out == CSAMPLE_GAIN_ZERO) {
        copy11WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, iNumSamples);
        return;
    }
    if (gain10in == CSAMPLE_GAIN_ZERO && gain10out == CSAMPLE_GAIN_ZERO) {
        copy11WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, iNumSamples);
        return;
    }
    if (gain11in == CSAMPLE_GAIN_ZERO && gain11out == CSAMPLE_GAIN_ZERO) {
        copy11WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    const CSAMPLE_GAIN gain_delta2 = (gain2out - gain2in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain2 = gain2in + gain_delta2;
    const CSAMPLE_GAIN gain_delta3 = (gain3out - gain3in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain3 = gain3in + gain_delta3;
    const CSAMPLE_GAIN gain_delta4 = (gain4out - gain4in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain4 = gain4in + gain_delta4;
    const CSAMPLE_GAIN gain_delta5 = (gain5out - gain5in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain5 = gain5in + gain_delta5;
    const CSAMPLE_GAIN gain_delta6 = (gain6out - gain6in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain6 = gain6in + gain_delta6;
    const CSAMPLE_GAIN gain_delta7 = (gain7out - gain7in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain7 = gain7in + gain_delta7;
    const CSAMPLE_GAIN gain_delta8 = (gain8out - gain8in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain8 = gain8in + gain_delta8;
    const CSAMPLE_GAIN gain_delta9 = (gain9out - gain9in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain9 = gain9in + gain_delta9;
    const CSAMPLE_GAIN gain_delta10 = (gain10out - gain10in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain10 = gain10in + gain_delta10;
    const CSAMPLE_GAIN gain_delta11 = (gain11out - gain11in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain11 = gain11in + gain_delta11;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        const CSAMPLE_GAIN gain2 = start_gain2 + gain_delta2 * i;
        const CSAMPLE_GAIN gain3 = start_gain3 + gain_delta3 * i;
        const CSAMPLE_GAIN gain4 = start_gain4 + gain_delta4 * i;
        const CSAMPLE_GAIN gain5 = start_gain5 + gain_delta5 * i;
        const CSAMPLE_GAIN gain6 = start_gain6 + gain_delta6 * i;
        const CSAMPLE_GAIN gain7 = start_gain7 + gain_delta7 * i;
        const CSAMPLE_GAIN gain8 = start_gain8 + gain_delta8 * i;
        const CSAMPLE_GAIN gain9 = start_gain9 + gain_delta9 * i;
        const CSAMPLE_GAIN gain10 = start_gain10 + gain_delta10 * i;
        const CSAMPLE_GAIN gain11 = start_gain11 + gain_delta11 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1 +
                       pSrc2[i * 2] * gain2 +
                       pSrc3[i * 2] * gain3 +
                       pSrc4[i * 2] * gain4 +
                       pSrc5[i * 2] * gain5 +
                       pSrc6[i * 2] * gain6 +
                       pSrc7[i * 2] * gain7 +
                       pSrc8[i * 2] * gain8 +
                       pSrc9[i * 2] * gain9 +
                       pSrc10[i * 2] * gain10 +
                       pSrc11[i * 2] * gain11;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1 +
                           pSrc2[i * 2 + 1] * gain2 +
                           pSrc3[i * 2 + 1] * gain3 +
                           pSrc4[i * 2 + 1] * gain4 +
                           pSrc5[i * 2 + 1] * gain5 +
                           pSrc6[i * 2 + 1] * gain6 +
                           pSrc7[i * 2 + 1] * gain7 +
                           pSrc8[i * 2 + 1] * gain8 +
                           pSrc9[i * 2 + 1] * gain9 +
                           pSrc10[i * 2 + 1] * gain10 +
                           pSrc11[i * 2 + 1] * gain11;
    }
}
static inline void copy13WithGain(CSAMPLE* M_RESTRICT pDest,
                                  const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                  const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                  const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                  const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3,
                                  const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4,
                                  const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5,
                                  const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6,
                                  const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7,
                                  const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8,
                                  const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9,
                                  const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10,
                                  const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11,
                                  const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12,
                                  int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy12WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy12WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, iNumSamples);
        return;
    }
    if (gain2 == CSAMPLE_GAIN_ZERO) {
        copy12WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, iNumSamples);
        return;
    }
    if (gain3 == CSAMPLE_GAIN_ZERO) {
        copy12WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, iNumSamples);
        return;
    }
    if (gain4 == CSAMPLE_GAIN_ZERO) {
        copy12WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, iNumSamples);
        return;
    }
    if (gain5 == CSAMPLE_GAIN_ZERO) {
        copy12WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, iNumSamples);
        return;
    }
    if (gain6 == CSAMPLE_GAIN_ZERO) {
        copy12WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, iNumSamples);
        return;
    }
    if (gain7 == CSAMPLE_GAIN_ZERO) {
        copy12WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, iNumSamples);
        return;
    }
    if (gain8 == CSAMPLE_GAIN_ZERO) {
        copy12WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, iNumSamples);
        return;
    }
    if (gain9 == CSAMPLE_GAIN_ZERO) {
        copy12WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, iNumSamples);
        return;
    }
    if (gain10 == CSAMPLE_GAIN_ZERO) {
        copy12WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, iNumSamples);
        return;
    }
    if (gain11 == CSAMPLE_GAIN_ZERO) {
        copy12WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, iNumSamples);
        return;
    }
    if (gain12 == CSAMPLE_GAIN_ZERO) {
        copy12WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
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
static inline void copy13WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                         const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                         const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                         const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                         const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3in, CSAMPLE_GAIN gain3out,
                                         const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4in, CSAMPLE_GAIN gain4out,
                                         const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5in, CSAMPLE_GAIN gain5out,
                                         const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6in, CSAMPLE_GAIN gain6out,
                                         const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7in, CSAMPLE_GAIN gain7out,
                                         const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8in, CSAMPLE_GAIN gain8out,
                                         const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9in, CSAMPLE_GAIN gain9out,
                                         const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10in, CSAMPLE_GAIN gain10out,
                                         const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11in, CSAMPLE_GAIN gain11out,
                                         const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12in, CSAMPLE_GAIN gain12out,
                                         int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy12WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy12WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, iNumSamples);
        return;
    }
    if (gain2in == CSAMPLE_GAIN_ZERO && gain2out == CSAMPLE_GAIN_ZERO) {
        copy12WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, iNumSamples);
        return;
    }
    if (gain3in == CSAMPLE_GAIN_ZERO && gain3out == CSAMPLE_GAIN_ZERO) {
        copy12WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, iNumSamples);
        return;
    }
    if (gain4in == CSAMPLE_GAIN_ZERO && gain4out == CSAMPLE_GAIN_ZERO) {
        copy12WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, iNumSamples);
        return;
    }
    if (gain5in == CSAMPLE_GAIN_ZERO && gain5out == CSAMPLE_GAIN_ZERO) {
        copy12WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, iNumSamples);
        return;
    }
    if (gain6in == CSAMPLE_GAIN_ZERO && gain6out == CSAMPLE_GAIN_ZERO) {
        copy12WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, iNumSamples);
        return;
    }
    if (gain7in == CSAMPLE_GAIN_ZERO && gain7out == CSAMPLE_GAIN_ZERO) {
        copy12WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, iNumSamples);
        return;
    }
    if (gain8in == CSAMPLE_GAIN_ZERO && gain8out == CSAMPLE_GAIN_ZERO) {
        copy12WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, iNumSamples);
        return;
    }
    if (gain9in == CSAMPLE_GAIN_ZERO && gain9out == CSAMPLE_GAIN_ZERO) {
        copy12WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, iNumSamples);
        return;
    }
    if (gain10in == CSAMPLE_GAIN_ZERO && gain10out == CSAMPLE_GAIN_ZERO) {
        copy12WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, iNumSamples);
        return;
    }
    if (gain11in == CSAMPLE_GAIN_ZERO && gain11out == CSAMPLE_GAIN_ZERO) {
        copy12WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, iNumSamples);
        return;
    }
    if (gain12in == CSAMPLE_GAIN_ZERO && gain12out == CSAMPLE_GAIN_ZERO) {
        copy12WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    const CSAMPLE_GAIN gain_delta2 = (gain2out - gain2in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain2 = gain2in + gain_delta2;
    const CSAMPLE_GAIN gain_delta3 = (gain3out - gain3in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain3 = gain3in + gain_delta3;
    const CSAMPLE_GAIN gain_delta4 = (gain4out - gain4in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain4 = gain4in + gain_delta4;
    const CSAMPLE_GAIN gain_delta5 = (gain5out - gain5in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain5 = gain5in + gain_delta5;
    const CSAMPLE_GAIN gain_delta6 = (gain6out - gain6in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain6 = gain6in + gain_delta6;
    const CSAMPLE_GAIN gain_delta7 = (gain7out - gain7in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain7 = gain7in + gain_delta7;
    const CSAMPLE_GAIN gain_delta8 = (gain8out - gain8in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain8 = gain8in + gain_delta8;
    const CSAMPLE_GAIN gain_delta9 = (gain9out - gain9in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain9 = gain9in + gain_delta9;
    const CSAMPLE_GAIN gain_delta10 = (gain10out - gain10in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain10 = gain10in + gain_delta10;
    const CSAMPLE_GAIN gain_delta11 = (gain11out - gain11in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain11 = gain11in + gain_delta11;
    const CSAMPLE_GAIN gain_delta12 = (gain12out - gain12in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain12 = gain12in + gain_delta12;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        const CSAMPLE_GAIN gain2 = start_gain2 + gain_delta2 * i;
        const CSAMPLE_GAIN gain3 = start_gain3 + gain_delta3 * i;
        const CSAMPLE_GAIN gain4 = start_gain4 + gain_delta4 * i;
        const CSAMPLE_GAIN gain5 = start_gain5 + gain_delta5 * i;
        const CSAMPLE_GAIN gain6 = start_gain6 + gain_delta6 * i;
        const CSAMPLE_GAIN gain7 = start_gain7 + gain_delta7 * i;
        const CSAMPLE_GAIN gain8 = start_gain8 + gain_delta8 * i;
        const CSAMPLE_GAIN gain9 = start_gain9 + gain_delta9 * i;
        const CSAMPLE_GAIN gain10 = start_gain10 + gain_delta10 * i;
        const CSAMPLE_GAIN gain11 = start_gain11 + gain_delta11 * i;
        const CSAMPLE_GAIN gain12 = start_gain12 + gain_delta12 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1 +
                       pSrc2[i * 2] * gain2 +
                       pSrc3[i * 2] * gain3 +
                       pSrc4[i * 2] * gain4 +
                       pSrc5[i * 2] * gain5 +
                       pSrc6[i * 2] * gain6 +
                       pSrc7[i * 2] * gain7 +
                       pSrc8[i * 2] * gain8 +
                       pSrc9[i * 2] * gain9 +
                       pSrc10[i * 2] * gain10 +
                       pSrc11[i * 2] * gain11 +
                       pSrc12[i * 2] * gain12;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1 +
                           pSrc2[i * 2 + 1] * gain2 +
                           pSrc3[i * 2 + 1] * gain3 +
                           pSrc4[i * 2 + 1] * gain4 +
                           pSrc5[i * 2 + 1] * gain5 +
                           pSrc6[i * 2 + 1] * gain6 +
                           pSrc7[i * 2 + 1] * gain7 +
                           pSrc8[i * 2 + 1] * gain8 +
                           pSrc9[i * 2 + 1] * gain9 +
                           pSrc10[i * 2 + 1] * gain10 +
                           pSrc11[i * 2 + 1] * gain11 +
                           pSrc12[i * 2 + 1] * gain12;
    }
}
static inline void copy14WithGain(CSAMPLE* M_RESTRICT pDest,
                                  const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                  const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                  const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                  const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3,
                                  const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4,
                                  const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5,
                                  const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6,
                                  const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7,
                                  const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8,
                                  const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9,
                                  const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10,
                                  const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11,
                                  const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12,
                                  const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13,
                                  int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy13WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy13WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, iNumSamples);
        return;
    }
    if (gain2 == CSAMPLE_GAIN_ZERO) {
        copy13WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, iNumSamples);
        return;
    }
    if (gain3 == CSAMPLE_GAIN_ZERO) {
        copy13WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, iNumSamples);
        return;
    }
    if (gain4 == CSAMPLE_GAIN_ZERO) {
        copy13WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, iNumSamples);
        return;
    }
    if (gain5 == CSAMPLE_GAIN_ZERO) {
        copy13WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, iNumSamples);
        return;
    }
    if (gain6 == CSAMPLE_GAIN_ZERO) {
        copy13WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, iNumSamples);
        return;
    }
    if (gain7 == CSAMPLE_GAIN_ZERO) {
        copy13WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, iNumSamples);
        return;
    }
    if (gain8 == CSAMPLE_GAIN_ZERO) {
        copy13WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, iNumSamples);
        return;
    }
    if (gain9 == CSAMPLE_GAIN_ZERO) {
        copy13WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, iNumSamples);
        return;
    }
    if (gain10 == CSAMPLE_GAIN_ZERO) {
        copy13WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, iNumSamples);
        return;
    }
    if (gain11 == CSAMPLE_GAIN_ZERO) {
        copy13WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, iNumSamples);
        return;
    }
    if (gain12 == CSAMPLE_GAIN_ZERO) {
        copy13WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, iNumSamples);
        return;
    }
    if (gain13 == CSAMPLE_GAIN_ZERO) {
        copy13WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
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
static inline void copy14WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                         const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                         const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                         const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                         const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3in, CSAMPLE_GAIN gain3out,
                                         const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4in, CSAMPLE_GAIN gain4out,
                                         const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5in, CSAMPLE_GAIN gain5out,
                                         const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6in, CSAMPLE_GAIN gain6out,
                                         const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7in, CSAMPLE_GAIN gain7out,
                                         const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8in, CSAMPLE_GAIN gain8out,
                                         const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9in, CSAMPLE_GAIN gain9out,
                                         const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10in, CSAMPLE_GAIN gain10out,
                                         const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11in, CSAMPLE_GAIN gain11out,
                                         const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12in, CSAMPLE_GAIN gain12out,
                                         const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13in, CSAMPLE_GAIN gain13out,
                                         int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy13WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy13WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, iNumSamples);
        return;
    }
    if (gain2in == CSAMPLE_GAIN_ZERO && gain2out == CSAMPLE_GAIN_ZERO) {
        copy13WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, iNumSamples);
        return;
    }
    if (gain3in == CSAMPLE_GAIN_ZERO && gain3out == CSAMPLE_GAIN_ZERO) {
        copy13WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, iNumSamples);
        return;
    }
    if (gain4in == CSAMPLE_GAIN_ZERO && gain4out == CSAMPLE_GAIN_ZERO) {
        copy13WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, iNumSamples);
        return;
    }
    if (gain5in == CSAMPLE_GAIN_ZERO && gain5out == CSAMPLE_GAIN_ZERO) {
        copy13WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, iNumSamples);
        return;
    }
    if (gain6in == CSAMPLE_GAIN_ZERO && gain6out == CSAMPLE_GAIN_ZERO) {
        copy13WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, iNumSamples);
        return;
    }
    if (gain7in == CSAMPLE_GAIN_ZERO && gain7out == CSAMPLE_GAIN_ZERO) {
        copy13WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, iNumSamples);
        return;
    }
    if (gain8in == CSAMPLE_GAIN_ZERO && gain8out == CSAMPLE_GAIN_ZERO) {
        copy13WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, iNumSamples);
        return;
    }
    if (gain9in == CSAMPLE_GAIN_ZERO && gain9out == CSAMPLE_GAIN_ZERO) {
        copy13WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, iNumSamples);
        return;
    }
    if (gain10in == CSAMPLE_GAIN_ZERO && gain10out == CSAMPLE_GAIN_ZERO) {
        copy13WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, iNumSamples);
        return;
    }
    if (gain11in == CSAMPLE_GAIN_ZERO && gain11out == CSAMPLE_GAIN_ZERO) {
        copy13WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, iNumSamples);
        return;
    }
    if (gain12in == CSAMPLE_GAIN_ZERO && gain12out == CSAMPLE_GAIN_ZERO) {
        copy13WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, iNumSamples);
        return;
    }
    if (gain13in == CSAMPLE_GAIN_ZERO && gain13out == CSAMPLE_GAIN_ZERO) {
        copy13WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    const CSAMPLE_GAIN gain_delta2 = (gain2out - gain2in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain2 = gain2in + gain_delta2;
    const CSAMPLE_GAIN gain_delta3 = (gain3out - gain3in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain3 = gain3in + gain_delta3;
    const CSAMPLE_GAIN gain_delta4 = (gain4out - gain4in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain4 = gain4in + gain_delta4;
    const CSAMPLE_GAIN gain_delta5 = (gain5out - gain5in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain5 = gain5in + gain_delta5;
    const CSAMPLE_GAIN gain_delta6 = (gain6out - gain6in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain6 = gain6in + gain_delta6;
    const CSAMPLE_GAIN gain_delta7 = (gain7out - gain7in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain7 = gain7in + gain_delta7;
    const CSAMPLE_GAIN gain_delta8 = (gain8out - gain8in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain8 = gain8in + gain_delta8;
    const CSAMPLE_GAIN gain_delta9 = (gain9out - gain9in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain9 = gain9in + gain_delta9;
    const CSAMPLE_GAIN gain_delta10 = (gain10out - gain10in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain10 = gain10in + gain_delta10;
    const CSAMPLE_GAIN gain_delta11 = (gain11out - gain11in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain11 = gain11in + gain_delta11;
    const CSAMPLE_GAIN gain_delta12 = (gain12out - gain12in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain12 = gain12in + gain_delta12;
    const CSAMPLE_GAIN gain_delta13 = (gain13out - gain13in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain13 = gain13in + gain_delta13;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        const CSAMPLE_GAIN gain2 = start_gain2 + gain_delta2 * i;
        const CSAMPLE_GAIN gain3 = start_gain3 + gain_delta3 * i;
        const CSAMPLE_GAIN gain4 = start_gain4 + gain_delta4 * i;
        const CSAMPLE_GAIN gain5 = start_gain5 + gain_delta5 * i;
        const CSAMPLE_GAIN gain6 = start_gain6 + gain_delta6 * i;
        const CSAMPLE_GAIN gain7 = start_gain7 + gain_delta7 * i;
        const CSAMPLE_GAIN gain8 = start_gain8 + gain_delta8 * i;
        const CSAMPLE_GAIN gain9 = start_gain9 + gain_delta9 * i;
        const CSAMPLE_GAIN gain10 = start_gain10 + gain_delta10 * i;
        const CSAMPLE_GAIN gain11 = start_gain11 + gain_delta11 * i;
        const CSAMPLE_GAIN gain12 = start_gain12 + gain_delta12 * i;
        const CSAMPLE_GAIN gain13 = start_gain13 + gain_delta13 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1 +
                       pSrc2[i * 2] * gain2 +
                       pSrc3[i * 2] * gain3 +
                       pSrc4[i * 2] * gain4 +
                       pSrc5[i * 2] * gain5 +
                       pSrc6[i * 2] * gain6 +
                       pSrc7[i * 2] * gain7 +
                       pSrc8[i * 2] * gain8 +
                       pSrc9[i * 2] * gain9 +
                       pSrc10[i * 2] * gain10 +
                       pSrc11[i * 2] * gain11 +
                       pSrc12[i * 2] * gain12 +
                       pSrc13[i * 2] * gain13;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1 +
                           pSrc2[i * 2 + 1] * gain2 +
                           pSrc3[i * 2 + 1] * gain3 +
                           pSrc4[i * 2 + 1] * gain4 +
                           pSrc5[i * 2 + 1] * gain5 +
                           pSrc6[i * 2 + 1] * gain6 +
                           pSrc7[i * 2 + 1] * gain7 +
                           pSrc8[i * 2 + 1] * gain8 +
                           pSrc9[i * 2 + 1] * gain9 +
                           pSrc10[i * 2 + 1] * gain10 +
                           pSrc11[i * 2 + 1] * gain11 +
                           pSrc12[i * 2 + 1] * gain12 +
                           pSrc13[i * 2 + 1] * gain13;
    }
}
static inline void copy15WithGain(CSAMPLE* M_RESTRICT pDest,
                                  const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                  const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                  const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                  const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3,
                                  const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4,
                                  const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5,
                                  const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6,
                                  const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7,
                                  const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8,
                                  const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9,
                                  const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10,
                                  const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11,
                                  const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12,
                                  const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13,
                                  const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14,
                                  int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy14WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy14WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, iNumSamples);
        return;
    }
    if (gain2 == CSAMPLE_GAIN_ZERO) {
        copy14WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, iNumSamples);
        return;
    }
    if (gain3 == CSAMPLE_GAIN_ZERO) {
        copy14WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, iNumSamples);
        return;
    }
    if (gain4 == CSAMPLE_GAIN_ZERO) {
        copy14WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, iNumSamples);
        return;
    }
    if (gain5 == CSAMPLE_GAIN_ZERO) {
        copy14WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, iNumSamples);
        return;
    }
    if (gain6 == CSAMPLE_GAIN_ZERO) {
        copy14WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, iNumSamples);
        return;
    }
    if (gain7 == CSAMPLE_GAIN_ZERO) {
        copy14WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, iNumSamples);
        return;
    }
    if (gain8 == CSAMPLE_GAIN_ZERO) {
        copy14WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, iNumSamples);
        return;
    }
    if (gain9 == CSAMPLE_GAIN_ZERO) {
        copy14WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, iNumSamples);
        return;
    }
    if (gain10 == CSAMPLE_GAIN_ZERO) {
        copy14WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, iNumSamples);
        return;
    }
    if (gain11 == CSAMPLE_GAIN_ZERO) {
        copy14WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, iNumSamples);
        return;
    }
    if (gain12 == CSAMPLE_GAIN_ZERO) {
        copy14WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, iNumSamples);
        return;
    }
    if (gain13 == CSAMPLE_GAIN_ZERO) {
        copy14WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, iNumSamples);
        return;
    }
    if (gain14 == CSAMPLE_GAIN_ZERO) {
        copy14WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
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
static inline void copy15WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                         const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                         const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                         const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                         const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3in, CSAMPLE_GAIN gain3out,
                                         const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4in, CSAMPLE_GAIN gain4out,
                                         const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5in, CSAMPLE_GAIN gain5out,
                                         const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6in, CSAMPLE_GAIN gain6out,
                                         const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7in, CSAMPLE_GAIN gain7out,
                                         const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8in, CSAMPLE_GAIN gain8out,
                                         const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9in, CSAMPLE_GAIN gain9out,
                                         const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10in, CSAMPLE_GAIN gain10out,
                                         const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11in, CSAMPLE_GAIN gain11out,
                                         const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12in, CSAMPLE_GAIN gain12out,
                                         const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13in, CSAMPLE_GAIN gain13out,
                                         const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14in, CSAMPLE_GAIN gain14out,
                                         int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy14WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy14WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, iNumSamples);
        return;
    }
    if (gain2in == CSAMPLE_GAIN_ZERO && gain2out == CSAMPLE_GAIN_ZERO) {
        copy14WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, iNumSamples);
        return;
    }
    if (gain3in == CSAMPLE_GAIN_ZERO && gain3out == CSAMPLE_GAIN_ZERO) {
        copy14WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, iNumSamples);
        return;
    }
    if (gain4in == CSAMPLE_GAIN_ZERO && gain4out == CSAMPLE_GAIN_ZERO) {
        copy14WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, iNumSamples);
        return;
    }
    if (gain5in == CSAMPLE_GAIN_ZERO && gain5out == CSAMPLE_GAIN_ZERO) {
        copy14WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, iNumSamples);
        return;
    }
    if (gain6in == CSAMPLE_GAIN_ZERO && gain6out == CSAMPLE_GAIN_ZERO) {
        copy14WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, iNumSamples);
        return;
    }
    if (gain7in == CSAMPLE_GAIN_ZERO && gain7out == CSAMPLE_GAIN_ZERO) {
        copy14WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, iNumSamples);
        return;
    }
    if (gain8in == CSAMPLE_GAIN_ZERO && gain8out == CSAMPLE_GAIN_ZERO) {
        copy14WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, iNumSamples);
        return;
    }
    if (gain9in == CSAMPLE_GAIN_ZERO && gain9out == CSAMPLE_GAIN_ZERO) {
        copy14WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, iNumSamples);
        return;
    }
    if (gain10in == CSAMPLE_GAIN_ZERO && gain10out == CSAMPLE_GAIN_ZERO) {
        copy14WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, iNumSamples);
        return;
    }
    if (gain11in == CSAMPLE_GAIN_ZERO && gain11out == CSAMPLE_GAIN_ZERO) {
        copy14WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, iNumSamples);
        return;
    }
    if (gain12in == CSAMPLE_GAIN_ZERO && gain12out == CSAMPLE_GAIN_ZERO) {
        copy14WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, iNumSamples);
        return;
    }
    if (gain13in == CSAMPLE_GAIN_ZERO && gain13out == CSAMPLE_GAIN_ZERO) {
        copy14WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, iNumSamples);
        return;
    }
    if (gain14in == CSAMPLE_GAIN_ZERO && gain14out == CSAMPLE_GAIN_ZERO) {
        copy14WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    const CSAMPLE_GAIN gain_delta2 = (gain2out - gain2in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain2 = gain2in + gain_delta2;
    const CSAMPLE_GAIN gain_delta3 = (gain3out - gain3in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain3 = gain3in + gain_delta3;
    const CSAMPLE_GAIN gain_delta4 = (gain4out - gain4in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain4 = gain4in + gain_delta4;
    const CSAMPLE_GAIN gain_delta5 = (gain5out - gain5in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain5 = gain5in + gain_delta5;
    const CSAMPLE_GAIN gain_delta6 = (gain6out - gain6in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain6 = gain6in + gain_delta6;
    const CSAMPLE_GAIN gain_delta7 = (gain7out - gain7in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain7 = gain7in + gain_delta7;
    const CSAMPLE_GAIN gain_delta8 = (gain8out - gain8in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain8 = gain8in + gain_delta8;
    const CSAMPLE_GAIN gain_delta9 = (gain9out - gain9in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain9 = gain9in + gain_delta9;
    const CSAMPLE_GAIN gain_delta10 = (gain10out - gain10in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain10 = gain10in + gain_delta10;
    const CSAMPLE_GAIN gain_delta11 = (gain11out - gain11in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain11 = gain11in + gain_delta11;
    const CSAMPLE_GAIN gain_delta12 = (gain12out - gain12in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain12 = gain12in + gain_delta12;
    const CSAMPLE_GAIN gain_delta13 = (gain13out - gain13in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain13 = gain13in + gain_delta13;
    const CSAMPLE_GAIN gain_delta14 = (gain14out - gain14in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain14 = gain14in + gain_delta14;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        const CSAMPLE_GAIN gain2 = start_gain2 + gain_delta2 * i;
        const CSAMPLE_GAIN gain3 = start_gain3 + gain_delta3 * i;
        const CSAMPLE_GAIN gain4 = start_gain4 + gain_delta4 * i;
        const CSAMPLE_GAIN gain5 = start_gain5 + gain_delta5 * i;
        const CSAMPLE_GAIN gain6 = start_gain6 + gain_delta6 * i;
        const CSAMPLE_GAIN gain7 = start_gain7 + gain_delta7 * i;
        const CSAMPLE_GAIN gain8 = start_gain8 + gain_delta8 * i;
        const CSAMPLE_GAIN gain9 = start_gain9 + gain_delta9 * i;
        const CSAMPLE_GAIN gain10 = start_gain10 + gain_delta10 * i;
        const CSAMPLE_GAIN gain11 = start_gain11 + gain_delta11 * i;
        const CSAMPLE_GAIN gain12 = start_gain12 + gain_delta12 * i;
        const CSAMPLE_GAIN gain13 = start_gain13 + gain_delta13 * i;
        const CSAMPLE_GAIN gain14 = start_gain14 + gain_delta14 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1 +
                       pSrc2[i * 2] * gain2 +
                       pSrc3[i * 2] * gain3 +
                       pSrc4[i * 2] * gain4 +
                       pSrc5[i * 2] * gain5 +
                       pSrc6[i * 2] * gain6 +
                       pSrc7[i * 2] * gain7 +
                       pSrc8[i * 2] * gain8 +
                       pSrc9[i * 2] * gain9 +
                       pSrc10[i * 2] * gain10 +
                       pSrc11[i * 2] * gain11 +
                       pSrc12[i * 2] * gain12 +
                       pSrc13[i * 2] * gain13 +
                       pSrc14[i * 2] * gain14;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1 +
                           pSrc2[i * 2 + 1] * gain2 +
                           pSrc3[i * 2 + 1] * gain3 +
                           pSrc4[i * 2 + 1] * gain4 +
                           pSrc5[i * 2 + 1] * gain5 +
                           pSrc6[i * 2 + 1] * gain6 +
                           pSrc7[i * 2 + 1] * gain7 +
                           pSrc8[i * 2 + 1] * gain8 +
                           pSrc9[i * 2 + 1] * gain9 +
                           pSrc10[i * 2 + 1] * gain10 +
                           pSrc11[i * 2 + 1] * gain11 +
                           pSrc12[i * 2 + 1] * gain12 +
                           pSrc13[i * 2 + 1] * gain13 +
                           pSrc14[i * 2 + 1] * gain14;
    }
}
static inline void copy16WithGain(CSAMPLE* M_RESTRICT pDest,
                                  const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                  const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                  const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                  const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3,
                                  const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4,
                                  const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5,
                                  const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6,
                                  const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7,
                                  const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8,
                                  const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9,
                                  const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10,
                                  const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11,
                                  const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12,
                                  const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13,
                                  const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14,
                                  const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15,
                                  int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy15WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy15WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, iNumSamples);
        return;
    }
    if (gain2 == CSAMPLE_GAIN_ZERO) {
        copy15WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, iNumSamples);
        return;
    }
    if (gain3 == CSAMPLE_GAIN_ZERO) {
        copy15WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, iNumSamples);
        return;
    }
    if (gain4 == CSAMPLE_GAIN_ZERO) {
        copy15WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, iNumSamples);
        return;
    }
    if (gain5 == CSAMPLE_GAIN_ZERO) {
        copy15WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, iNumSamples);
        return;
    }
    if (gain6 == CSAMPLE_GAIN_ZERO) {
        copy15WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, iNumSamples);
        return;
    }
    if (gain7 == CSAMPLE_GAIN_ZERO) {
        copy15WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, iNumSamples);
        return;
    }
    if (gain8 == CSAMPLE_GAIN_ZERO) {
        copy15WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, iNumSamples);
        return;
    }
    if (gain9 == CSAMPLE_GAIN_ZERO) {
        copy15WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, iNumSamples);
        return;
    }
    if (gain10 == CSAMPLE_GAIN_ZERO) {
        copy15WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, iNumSamples);
        return;
    }
    if (gain11 == CSAMPLE_GAIN_ZERO) {
        copy15WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, iNumSamples);
        return;
    }
    if (gain12 == CSAMPLE_GAIN_ZERO) {
        copy15WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, iNumSamples);
        return;
    }
    if (gain13 == CSAMPLE_GAIN_ZERO) {
        copy15WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, iNumSamples);
        return;
    }
    if (gain14 == CSAMPLE_GAIN_ZERO) {
        copy15WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, iNumSamples);
        return;
    }
    if (gain15 == CSAMPLE_GAIN_ZERO) {
        copy15WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
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
static inline void copy16WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                         const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                         const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                         const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                         const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3in, CSAMPLE_GAIN gain3out,
                                         const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4in, CSAMPLE_GAIN gain4out,
                                         const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5in, CSAMPLE_GAIN gain5out,
                                         const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6in, CSAMPLE_GAIN gain6out,
                                         const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7in, CSAMPLE_GAIN gain7out,
                                         const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8in, CSAMPLE_GAIN gain8out,
                                         const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9in, CSAMPLE_GAIN gain9out,
                                         const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10in, CSAMPLE_GAIN gain10out,
                                         const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11in, CSAMPLE_GAIN gain11out,
                                         const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12in, CSAMPLE_GAIN gain12out,
                                         const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13in, CSAMPLE_GAIN gain13out,
                                         const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14in, CSAMPLE_GAIN gain14out,
                                         const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15in, CSAMPLE_GAIN gain15out,
                                         int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy15WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy15WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    if (gain2in == CSAMPLE_GAIN_ZERO && gain2out == CSAMPLE_GAIN_ZERO) {
        copy15WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    if (gain3in == CSAMPLE_GAIN_ZERO && gain3out == CSAMPLE_GAIN_ZERO) {
        copy15WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    if (gain4in == CSAMPLE_GAIN_ZERO && gain4out == CSAMPLE_GAIN_ZERO) {
        copy15WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    if (gain5in == CSAMPLE_GAIN_ZERO && gain5out == CSAMPLE_GAIN_ZERO) {
        copy15WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    if (gain6in == CSAMPLE_GAIN_ZERO && gain6out == CSAMPLE_GAIN_ZERO) {
        copy15WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    if (gain7in == CSAMPLE_GAIN_ZERO && gain7out == CSAMPLE_GAIN_ZERO) {
        copy15WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    if (gain8in == CSAMPLE_GAIN_ZERO && gain8out == CSAMPLE_GAIN_ZERO) {
        copy15WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    if (gain9in == CSAMPLE_GAIN_ZERO && gain9out == CSAMPLE_GAIN_ZERO) {
        copy15WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    if (gain10in == CSAMPLE_GAIN_ZERO && gain10out == CSAMPLE_GAIN_ZERO) {
        copy15WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    if (gain11in == CSAMPLE_GAIN_ZERO && gain11out == CSAMPLE_GAIN_ZERO) {
        copy15WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    if (gain12in == CSAMPLE_GAIN_ZERO && gain12out == CSAMPLE_GAIN_ZERO) {
        copy15WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    if (gain13in == CSAMPLE_GAIN_ZERO && gain13out == CSAMPLE_GAIN_ZERO) {
        copy15WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    if (gain14in == CSAMPLE_GAIN_ZERO && gain14out == CSAMPLE_GAIN_ZERO) {
        copy15WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    if (gain15in == CSAMPLE_GAIN_ZERO && gain15out == CSAMPLE_GAIN_ZERO) {
        copy15WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    const CSAMPLE_GAIN gain_delta2 = (gain2out - gain2in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain2 = gain2in + gain_delta2;
    const CSAMPLE_GAIN gain_delta3 = (gain3out - gain3in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain3 = gain3in + gain_delta3;
    const CSAMPLE_GAIN gain_delta4 = (gain4out - gain4in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain4 = gain4in + gain_delta4;
    const CSAMPLE_GAIN gain_delta5 = (gain5out - gain5in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain5 = gain5in + gain_delta5;
    const CSAMPLE_GAIN gain_delta6 = (gain6out - gain6in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain6 = gain6in + gain_delta6;
    const CSAMPLE_GAIN gain_delta7 = (gain7out - gain7in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain7 = gain7in + gain_delta7;
    const CSAMPLE_GAIN gain_delta8 = (gain8out - gain8in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain8 = gain8in + gain_delta8;
    const CSAMPLE_GAIN gain_delta9 = (gain9out - gain9in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain9 = gain9in + gain_delta9;
    const CSAMPLE_GAIN gain_delta10 = (gain10out - gain10in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain10 = gain10in + gain_delta10;
    const CSAMPLE_GAIN gain_delta11 = (gain11out - gain11in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain11 = gain11in + gain_delta11;
    const CSAMPLE_GAIN gain_delta12 = (gain12out - gain12in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain12 = gain12in + gain_delta12;
    const CSAMPLE_GAIN gain_delta13 = (gain13out - gain13in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain13 = gain13in + gain_delta13;
    const CSAMPLE_GAIN gain_delta14 = (gain14out - gain14in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain14 = gain14in + gain_delta14;
    const CSAMPLE_GAIN gain_delta15 = (gain15out - gain15in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain15 = gain15in + gain_delta15;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        const CSAMPLE_GAIN gain2 = start_gain2 + gain_delta2 * i;
        const CSAMPLE_GAIN gain3 = start_gain3 + gain_delta3 * i;
        const CSAMPLE_GAIN gain4 = start_gain4 + gain_delta4 * i;
        const CSAMPLE_GAIN gain5 = start_gain5 + gain_delta5 * i;
        const CSAMPLE_GAIN gain6 = start_gain6 + gain_delta6 * i;
        const CSAMPLE_GAIN gain7 = start_gain7 + gain_delta7 * i;
        const CSAMPLE_GAIN gain8 = start_gain8 + gain_delta8 * i;
        const CSAMPLE_GAIN gain9 = start_gain9 + gain_delta9 * i;
        const CSAMPLE_GAIN gain10 = start_gain10 + gain_delta10 * i;
        const CSAMPLE_GAIN gain11 = start_gain11 + gain_delta11 * i;
        const CSAMPLE_GAIN gain12 = start_gain12 + gain_delta12 * i;
        const CSAMPLE_GAIN gain13 = start_gain13 + gain_delta13 * i;
        const CSAMPLE_GAIN gain14 = start_gain14 + gain_delta14 * i;
        const CSAMPLE_GAIN gain15 = start_gain15 + gain_delta15 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1 +
                       pSrc2[i * 2] * gain2 +
                       pSrc3[i * 2] * gain3 +
                       pSrc4[i * 2] * gain4 +
                       pSrc5[i * 2] * gain5 +
                       pSrc6[i * 2] * gain6 +
                       pSrc7[i * 2] * gain7 +
                       pSrc8[i * 2] * gain8 +
                       pSrc9[i * 2] * gain9 +
                       pSrc10[i * 2] * gain10 +
                       pSrc11[i * 2] * gain11 +
                       pSrc12[i * 2] * gain12 +
                       pSrc13[i * 2] * gain13 +
                       pSrc14[i * 2] * gain14 +
                       pSrc15[i * 2] * gain15;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1 +
                           pSrc2[i * 2 + 1] * gain2 +
                           pSrc3[i * 2 + 1] * gain3 +
                           pSrc4[i * 2 + 1] * gain4 +
                           pSrc5[i * 2 + 1] * gain5 +
                           pSrc6[i * 2 + 1] * gain6 +
                           pSrc7[i * 2 + 1] * gain7 +
                           pSrc8[i * 2 + 1] * gain8 +
                           pSrc9[i * 2 + 1] * gain9 +
                           pSrc10[i * 2 + 1] * gain10 +
                           pSrc11[i * 2 + 1] * gain11 +
                           pSrc12[i * 2 + 1] * gain12 +
                           pSrc13[i * 2 + 1] * gain13 +
                           pSrc14[i * 2 + 1] * gain14 +
                           pSrc15[i * 2 + 1] * gain15;
    }
}
static inline void copy17WithGain(CSAMPLE* M_RESTRICT pDest,
                                  const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                  const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                  const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                  const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3,
                                  const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4,
                                  const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5,
                                  const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6,
                                  const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7,
                                  const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8,
                                  const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9,
                                  const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10,
                                  const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11,
                                  const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12,
                                  const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13,
                                  const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14,
                                  const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15,
                                  const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16,
                                  int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy16WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain2 == CSAMPLE_GAIN_ZERO) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain3 == CSAMPLE_GAIN_ZERO) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain4 == CSAMPLE_GAIN_ZERO) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain5 == CSAMPLE_GAIN_ZERO) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain6 == CSAMPLE_GAIN_ZERO) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain7 == CSAMPLE_GAIN_ZERO) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain8 == CSAMPLE_GAIN_ZERO) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain9 == CSAMPLE_GAIN_ZERO) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain10 == CSAMPLE_GAIN_ZERO) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain11 == CSAMPLE_GAIN_ZERO) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain12 == CSAMPLE_GAIN_ZERO) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain13 == CSAMPLE_GAIN_ZERO) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain14 == CSAMPLE_GAIN_ZERO) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain15 == CSAMPLE_GAIN_ZERO) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, iNumSamples);
        return;
    }
    if (gain16 == CSAMPLE_GAIN_ZERO) {
        copy16WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
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
static inline void copy17WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                         const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                         const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                         const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                         const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3in, CSAMPLE_GAIN gain3out,
                                         const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4in, CSAMPLE_GAIN gain4out,
                                         const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5in, CSAMPLE_GAIN gain5out,
                                         const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6in, CSAMPLE_GAIN gain6out,
                                         const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7in, CSAMPLE_GAIN gain7out,
                                         const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8in, CSAMPLE_GAIN gain8out,
                                         const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9in, CSAMPLE_GAIN gain9out,
                                         const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10in, CSAMPLE_GAIN gain10out,
                                         const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11in, CSAMPLE_GAIN gain11out,
                                         const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12in, CSAMPLE_GAIN gain12out,
                                         const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13in, CSAMPLE_GAIN gain13out,
                                         const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14in, CSAMPLE_GAIN gain14out,
                                         const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15in, CSAMPLE_GAIN gain15out,
                                         const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16in, CSAMPLE_GAIN gain16out,
                                         int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy16WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain2in == CSAMPLE_GAIN_ZERO && gain2out == CSAMPLE_GAIN_ZERO) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain3in == CSAMPLE_GAIN_ZERO && gain3out == CSAMPLE_GAIN_ZERO) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain4in == CSAMPLE_GAIN_ZERO && gain4out == CSAMPLE_GAIN_ZERO) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain5in == CSAMPLE_GAIN_ZERO && gain5out == CSAMPLE_GAIN_ZERO) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain6in == CSAMPLE_GAIN_ZERO && gain6out == CSAMPLE_GAIN_ZERO) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain7in == CSAMPLE_GAIN_ZERO && gain7out == CSAMPLE_GAIN_ZERO) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain8in == CSAMPLE_GAIN_ZERO && gain8out == CSAMPLE_GAIN_ZERO) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain9in == CSAMPLE_GAIN_ZERO && gain9out == CSAMPLE_GAIN_ZERO) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain10in == CSAMPLE_GAIN_ZERO && gain10out == CSAMPLE_GAIN_ZERO) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain11in == CSAMPLE_GAIN_ZERO && gain11out == CSAMPLE_GAIN_ZERO) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain12in == CSAMPLE_GAIN_ZERO && gain12out == CSAMPLE_GAIN_ZERO) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain13in == CSAMPLE_GAIN_ZERO && gain13out == CSAMPLE_GAIN_ZERO) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain14in == CSAMPLE_GAIN_ZERO && gain14out == CSAMPLE_GAIN_ZERO) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain15in == CSAMPLE_GAIN_ZERO && gain15out == CSAMPLE_GAIN_ZERO) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    if (gain16in == CSAMPLE_GAIN_ZERO && gain16out == CSAMPLE_GAIN_ZERO) {
        copy16WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    const CSAMPLE_GAIN gain_delta2 = (gain2out - gain2in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain2 = gain2in + gain_delta2;
    const CSAMPLE_GAIN gain_delta3 = (gain3out - gain3in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain3 = gain3in + gain_delta3;
    const CSAMPLE_GAIN gain_delta4 = (gain4out - gain4in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain4 = gain4in + gain_delta4;
    const CSAMPLE_GAIN gain_delta5 = (gain5out - gain5in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain5 = gain5in + gain_delta5;
    const CSAMPLE_GAIN gain_delta6 = (gain6out - gain6in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain6 = gain6in + gain_delta6;
    const CSAMPLE_GAIN gain_delta7 = (gain7out - gain7in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain7 = gain7in + gain_delta7;
    const CSAMPLE_GAIN gain_delta8 = (gain8out - gain8in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain8 = gain8in + gain_delta8;
    const CSAMPLE_GAIN gain_delta9 = (gain9out - gain9in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain9 = gain9in + gain_delta9;
    const CSAMPLE_GAIN gain_delta10 = (gain10out - gain10in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain10 = gain10in + gain_delta10;
    const CSAMPLE_GAIN gain_delta11 = (gain11out - gain11in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain11 = gain11in + gain_delta11;
    const CSAMPLE_GAIN gain_delta12 = (gain12out - gain12in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain12 = gain12in + gain_delta12;
    const CSAMPLE_GAIN gain_delta13 = (gain13out - gain13in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain13 = gain13in + gain_delta13;
    const CSAMPLE_GAIN gain_delta14 = (gain14out - gain14in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain14 = gain14in + gain_delta14;
    const CSAMPLE_GAIN gain_delta15 = (gain15out - gain15in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain15 = gain15in + gain_delta15;
    const CSAMPLE_GAIN gain_delta16 = (gain16out - gain16in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain16 = gain16in + gain_delta16;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        const CSAMPLE_GAIN gain2 = start_gain2 + gain_delta2 * i;
        const CSAMPLE_GAIN gain3 = start_gain3 + gain_delta3 * i;
        const CSAMPLE_GAIN gain4 = start_gain4 + gain_delta4 * i;
        const CSAMPLE_GAIN gain5 = start_gain5 + gain_delta5 * i;
        const CSAMPLE_GAIN gain6 = start_gain6 + gain_delta6 * i;
        const CSAMPLE_GAIN gain7 = start_gain7 + gain_delta7 * i;
        const CSAMPLE_GAIN gain8 = start_gain8 + gain_delta8 * i;
        const CSAMPLE_GAIN gain9 = start_gain9 + gain_delta9 * i;
        const CSAMPLE_GAIN gain10 = start_gain10 + gain_delta10 * i;
        const CSAMPLE_GAIN gain11 = start_gain11 + gain_delta11 * i;
        const CSAMPLE_GAIN gain12 = start_gain12 + gain_delta12 * i;
        const CSAMPLE_GAIN gain13 = start_gain13 + gain_delta13 * i;
        const CSAMPLE_GAIN gain14 = start_gain14 + gain_delta14 * i;
        const CSAMPLE_GAIN gain15 = start_gain15 + gain_delta15 * i;
        const CSAMPLE_GAIN gain16 = start_gain16 + gain_delta16 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1 +
                       pSrc2[i * 2] * gain2 +
                       pSrc3[i * 2] * gain3 +
                       pSrc4[i * 2] * gain4 +
                       pSrc5[i * 2] * gain5 +
                       pSrc6[i * 2] * gain6 +
                       pSrc7[i * 2] * gain7 +
                       pSrc8[i * 2] * gain8 +
                       pSrc9[i * 2] * gain9 +
                       pSrc10[i * 2] * gain10 +
                       pSrc11[i * 2] * gain11 +
                       pSrc12[i * 2] * gain12 +
                       pSrc13[i * 2] * gain13 +
                       pSrc14[i * 2] * gain14 +
                       pSrc15[i * 2] * gain15 +
                       pSrc16[i * 2] * gain16;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1 +
                           pSrc2[i * 2 + 1] * gain2 +
                           pSrc3[i * 2 + 1] * gain3 +
                           pSrc4[i * 2 + 1] * gain4 +
                           pSrc5[i * 2 + 1] * gain5 +
                           pSrc6[i * 2 + 1] * gain6 +
                           pSrc7[i * 2 + 1] * gain7 +
                           pSrc8[i * 2 + 1] * gain8 +
                           pSrc9[i * 2 + 1] * gain9 +
                           pSrc10[i * 2 + 1] * gain10 +
                           pSrc11[i * 2 + 1] * gain11 +
                           pSrc12[i * 2 + 1] * gain12 +
                           pSrc13[i * 2 + 1] * gain13 +
                           pSrc14[i * 2 + 1] * gain14 +
                           pSrc15[i * 2 + 1] * gain15 +
                           pSrc16[i * 2 + 1] * gain16;
    }
}
static inline void copy18WithGain(CSAMPLE* M_RESTRICT pDest,
                                  const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                  const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                  const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                  const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3,
                                  const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4,
                                  const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5,
                                  const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6,
                                  const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7,
                                  const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8,
                                  const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9,
                                  const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10,
                                  const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11,
                                  const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12,
                                  const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13,
                                  const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14,
                                  const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15,
                                  const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16,
                                  const CSAMPLE* M_RESTRICT pSrc17, CSAMPLE_GAIN gain17,
                                  int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy17WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain2 == CSAMPLE_GAIN_ZERO) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain3 == CSAMPLE_GAIN_ZERO) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain4 == CSAMPLE_GAIN_ZERO) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain5 == CSAMPLE_GAIN_ZERO) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain6 == CSAMPLE_GAIN_ZERO) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain7 == CSAMPLE_GAIN_ZERO) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain8 == CSAMPLE_GAIN_ZERO) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain9 == CSAMPLE_GAIN_ZERO) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain10 == CSAMPLE_GAIN_ZERO) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain11 == CSAMPLE_GAIN_ZERO) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain12 == CSAMPLE_GAIN_ZERO) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain13 == CSAMPLE_GAIN_ZERO) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain14 == CSAMPLE_GAIN_ZERO) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain15 == CSAMPLE_GAIN_ZERO) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain16 == CSAMPLE_GAIN_ZERO) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc17, gain17, iNumSamples);
        return;
    }
    if (gain17 == CSAMPLE_GAIN_ZERO) {
        copy17WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
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
static inline void copy18WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                         const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                         const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                         const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                         const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3in, CSAMPLE_GAIN gain3out,
                                         const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4in, CSAMPLE_GAIN gain4out,
                                         const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5in, CSAMPLE_GAIN gain5out,
                                         const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6in, CSAMPLE_GAIN gain6out,
                                         const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7in, CSAMPLE_GAIN gain7out,
                                         const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8in, CSAMPLE_GAIN gain8out,
                                         const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9in, CSAMPLE_GAIN gain9out,
                                         const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10in, CSAMPLE_GAIN gain10out,
                                         const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11in, CSAMPLE_GAIN gain11out,
                                         const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12in, CSAMPLE_GAIN gain12out,
                                         const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13in, CSAMPLE_GAIN gain13out,
                                         const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14in, CSAMPLE_GAIN gain14out,
                                         const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15in, CSAMPLE_GAIN gain15out,
                                         const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16in, CSAMPLE_GAIN gain16out,
                                         const CSAMPLE* M_RESTRICT pSrc17, CSAMPLE_GAIN gain17in, CSAMPLE_GAIN gain17out,
                                         int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy17WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain2in == CSAMPLE_GAIN_ZERO && gain2out == CSAMPLE_GAIN_ZERO) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain3in == CSAMPLE_GAIN_ZERO && gain3out == CSAMPLE_GAIN_ZERO) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain4in == CSAMPLE_GAIN_ZERO && gain4out == CSAMPLE_GAIN_ZERO) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain5in == CSAMPLE_GAIN_ZERO && gain5out == CSAMPLE_GAIN_ZERO) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain6in == CSAMPLE_GAIN_ZERO && gain6out == CSAMPLE_GAIN_ZERO) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain7in == CSAMPLE_GAIN_ZERO && gain7out == CSAMPLE_GAIN_ZERO) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain8in == CSAMPLE_GAIN_ZERO && gain8out == CSAMPLE_GAIN_ZERO) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain9in == CSAMPLE_GAIN_ZERO && gain9out == CSAMPLE_GAIN_ZERO) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain10in == CSAMPLE_GAIN_ZERO && gain10out == CSAMPLE_GAIN_ZERO) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain11in == CSAMPLE_GAIN_ZERO && gain11out == CSAMPLE_GAIN_ZERO) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain12in == CSAMPLE_GAIN_ZERO && gain12out == CSAMPLE_GAIN_ZERO) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain13in == CSAMPLE_GAIN_ZERO && gain13out == CSAMPLE_GAIN_ZERO) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain14in == CSAMPLE_GAIN_ZERO && gain14out == CSAMPLE_GAIN_ZERO) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain15in == CSAMPLE_GAIN_ZERO && gain15out == CSAMPLE_GAIN_ZERO) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain16in == CSAMPLE_GAIN_ZERO && gain16out == CSAMPLE_GAIN_ZERO) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    if (gain17in == CSAMPLE_GAIN_ZERO && gain17out == CSAMPLE_GAIN_ZERO) {
        copy17WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    const CSAMPLE_GAIN gain_delta2 = (gain2out - gain2in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain2 = gain2in + gain_delta2;
    const CSAMPLE_GAIN gain_delta3 = (gain3out - gain3in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain3 = gain3in + gain_delta3;
    const CSAMPLE_GAIN gain_delta4 = (gain4out - gain4in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain4 = gain4in + gain_delta4;
    const CSAMPLE_GAIN gain_delta5 = (gain5out - gain5in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain5 = gain5in + gain_delta5;
    const CSAMPLE_GAIN gain_delta6 = (gain6out - gain6in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain6 = gain6in + gain_delta6;
    const CSAMPLE_GAIN gain_delta7 = (gain7out - gain7in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain7 = gain7in + gain_delta7;
    const CSAMPLE_GAIN gain_delta8 = (gain8out - gain8in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain8 = gain8in + gain_delta8;
    const CSAMPLE_GAIN gain_delta9 = (gain9out - gain9in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain9 = gain9in + gain_delta9;
    const CSAMPLE_GAIN gain_delta10 = (gain10out - gain10in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain10 = gain10in + gain_delta10;
    const CSAMPLE_GAIN gain_delta11 = (gain11out - gain11in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain11 = gain11in + gain_delta11;
    const CSAMPLE_GAIN gain_delta12 = (gain12out - gain12in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain12 = gain12in + gain_delta12;
    const CSAMPLE_GAIN gain_delta13 = (gain13out - gain13in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain13 = gain13in + gain_delta13;
    const CSAMPLE_GAIN gain_delta14 = (gain14out - gain14in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain14 = gain14in + gain_delta14;
    const CSAMPLE_GAIN gain_delta15 = (gain15out - gain15in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain15 = gain15in + gain_delta15;
    const CSAMPLE_GAIN gain_delta16 = (gain16out - gain16in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain16 = gain16in + gain_delta16;
    const CSAMPLE_GAIN gain_delta17 = (gain17out - gain17in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain17 = gain17in + gain_delta17;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        const CSAMPLE_GAIN gain2 = start_gain2 + gain_delta2 * i;
        const CSAMPLE_GAIN gain3 = start_gain3 + gain_delta3 * i;
        const CSAMPLE_GAIN gain4 = start_gain4 + gain_delta4 * i;
        const CSAMPLE_GAIN gain5 = start_gain5 + gain_delta5 * i;
        const CSAMPLE_GAIN gain6 = start_gain6 + gain_delta6 * i;
        const CSAMPLE_GAIN gain7 = start_gain7 + gain_delta7 * i;
        const CSAMPLE_GAIN gain8 = start_gain8 + gain_delta8 * i;
        const CSAMPLE_GAIN gain9 = start_gain9 + gain_delta9 * i;
        const CSAMPLE_GAIN gain10 = start_gain10 + gain_delta10 * i;
        const CSAMPLE_GAIN gain11 = start_gain11 + gain_delta11 * i;
        const CSAMPLE_GAIN gain12 = start_gain12 + gain_delta12 * i;
        const CSAMPLE_GAIN gain13 = start_gain13 + gain_delta13 * i;
        const CSAMPLE_GAIN gain14 = start_gain14 + gain_delta14 * i;
        const CSAMPLE_GAIN gain15 = start_gain15 + gain_delta15 * i;
        const CSAMPLE_GAIN gain16 = start_gain16 + gain_delta16 * i;
        const CSAMPLE_GAIN gain17 = start_gain17 + gain_delta17 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1 +
                       pSrc2[i * 2] * gain2 +
                       pSrc3[i * 2] * gain3 +
                       pSrc4[i * 2] * gain4 +
                       pSrc5[i * 2] * gain5 +
                       pSrc6[i * 2] * gain6 +
                       pSrc7[i * 2] * gain7 +
                       pSrc8[i * 2] * gain8 +
                       pSrc9[i * 2] * gain9 +
                       pSrc10[i * 2] * gain10 +
                       pSrc11[i * 2] * gain11 +
                       pSrc12[i * 2] * gain12 +
                       pSrc13[i * 2] * gain13 +
                       pSrc14[i * 2] * gain14 +
                       pSrc15[i * 2] * gain15 +
                       pSrc16[i * 2] * gain16 +
                       pSrc17[i * 2] * gain17;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1 +
                           pSrc2[i * 2 + 1] * gain2 +
                           pSrc3[i * 2 + 1] * gain3 +
                           pSrc4[i * 2 + 1] * gain4 +
                           pSrc5[i * 2 + 1] * gain5 +
                           pSrc6[i * 2 + 1] * gain6 +
                           pSrc7[i * 2 + 1] * gain7 +
                           pSrc8[i * 2 + 1] * gain8 +
                           pSrc9[i * 2 + 1] * gain9 +
                           pSrc10[i * 2 + 1] * gain10 +
                           pSrc11[i * 2 + 1] * gain11 +
                           pSrc12[i * 2 + 1] * gain12 +
                           pSrc13[i * 2 + 1] * gain13 +
                           pSrc14[i * 2 + 1] * gain14 +
                           pSrc15[i * 2 + 1] * gain15 +
                           pSrc16[i * 2 + 1] * gain16 +
                           pSrc17[i * 2 + 1] * gain17;
    }
}
static inline void copy19WithGain(CSAMPLE* M_RESTRICT pDest,
                                  const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                  const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                  const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                  const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3,
                                  const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4,
                                  const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5,
                                  const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6,
                                  const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7,
                                  const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8,
                                  const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9,
                                  const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10,
                                  const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11,
                                  const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12,
                                  const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13,
                                  const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14,
                                  const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15,
                                  const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16,
                                  const CSAMPLE* M_RESTRICT pSrc17, CSAMPLE_GAIN gain17,
                                  const CSAMPLE* M_RESTRICT pSrc18, CSAMPLE_GAIN gain18,
                                  int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy18WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain2 == CSAMPLE_GAIN_ZERO) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain3 == CSAMPLE_GAIN_ZERO) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain4 == CSAMPLE_GAIN_ZERO) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain5 == CSAMPLE_GAIN_ZERO) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain6 == CSAMPLE_GAIN_ZERO) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain7 == CSAMPLE_GAIN_ZERO) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain8 == CSAMPLE_GAIN_ZERO) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain9 == CSAMPLE_GAIN_ZERO) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain10 == CSAMPLE_GAIN_ZERO) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain11 == CSAMPLE_GAIN_ZERO) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain12 == CSAMPLE_GAIN_ZERO) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain13 == CSAMPLE_GAIN_ZERO) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain14 == CSAMPLE_GAIN_ZERO) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain15 == CSAMPLE_GAIN_ZERO) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain16 == CSAMPLE_GAIN_ZERO) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain17 == CSAMPLE_GAIN_ZERO) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc18, gain18, iNumSamples);
        return;
    }
    if (gain18 == CSAMPLE_GAIN_ZERO) {
        copy18WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
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
static inline void copy19WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                         const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                         const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                         const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                         const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3in, CSAMPLE_GAIN gain3out,
                                         const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4in, CSAMPLE_GAIN gain4out,
                                         const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5in, CSAMPLE_GAIN gain5out,
                                         const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6in, CSAMPLE_GAIN gain6out,
                                         const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7in, CSAMPLE_GAIN gain7out,
                                         const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8in, CSAMPLE_GAIN gain8out,
                                         const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9in, CSAMPLE_GAIN gain9out,
                                         const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10in, CSAMPLE_GAIN gain10out,
                                         const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11in, CSAMPLE_GAIN gain11out,
                                         const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12in, CSAMPLE_GAIN gain12out,
                                         const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13in, CSAMPLE_GAIN gain13out,
                                         const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14in, CSAMPLE_GAIN gain14out,
                                         const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15in, CSAMPLE_GAIN gain15out,
                                         const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16in, CSAMPLE_GAIN gain16out,
                                         const CSAMPLE* M_RESTRICT pSrc17, CSAMPLE_GAIN gain17in, CSAMPLE_GAIN gain17out,
                                         const CSAMPLE* M_RESTRICT pSrc18, CSAMPLE_GAIN gain18in, CSAMPLE_GAIN gain18out,
                                         int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy18WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain2in == CSAMPLE_GAIN_ZERO && gain2out == CSAMPLE_GAIN_ZERO) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain3in == CSAMPLE_GAIN_ZERO && gain3out == CSAMPLE_GAIN_ZERO) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain4in == CSAMPLE_GAIN_ZERO && gain4out == CSAMPLE_GAIN_ZERO) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain5in == CSAMPLE_GAIN_ZERO && gain5out == CSAMPLE_GAIN_ZERO) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain6in == CSAMPLE_GAIN_ZERO && gain6out == CSAMPLE_GAIN_ZERO) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain7in == CSAMPLE_GAIN_ZERO && gain7out == CSAMPLE_GAIN_ZERO) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain8in == CSAMPLE_GAIN_ZERO && gain8out == CSAMPLE_GAIN_ZERO) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain9in == CSAMPLE_GAIN_ZERO && gain9out == CSAMPLE_GAIN_ZERO) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain10in == CSAMPLE_GAIN_ZERO && gain10out == CSAMPLE_GAIN_ZERO) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain11in == CSAMPLE_GAIN_ZERO && gain11out == CSAMPLE_GAIN_ZERO) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain12in == CSAMPLE_GAIN_ZERO && gain12out == CSAMPLE_GAIN_ZERO) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain13in == CSAMPLE_GAIN_ZERO && gain13out == CSAMPLE_GAIN_ZERO) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain14in == CSAMPLE_GAIN_ZERO && gain14out == CSAMPLE_GAIN_ZERO) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain15in == CSAMPLE_GAIN_ZERO && gain15out == CSAMPLE_GAIN_ZERO) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain16in == CSAMPLE_GAIN_ZERO && gain16out == CSAMPLE_GAIN_ZERO) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain17in == CSAMPLE_GAIN_ZERO && gain17out == CSAMPLE_GAIN_ZERO) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    if (gain18in == CSAMPLE_GAIN_ZERO && gain18out == CSAMPLE_GAIN_ZERO) {
        copy18WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    const CSAMPLE_GAIN gain_delta2 = (gain2out - gain2in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain2 = gain2in + gain_delta2;
    const CSAMPLE_GAIN gain_delta3 = (gain3out - gain3in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain3 = gain3in + gain_delta3;
    const CSAMPLE_GAIN gain_delta4 = (gain4out - gain4in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain4 = gain4in + gain_delta4;
    const CSAMPLE_GAIN gain_delta5 = (gain5out - gain5in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain5 = gain5in + gain_delta5;
    const CSAMPLE_GAIN gain_delta6 = (gain6out - gain6in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain6 = gain6in + gain_delta6;
    const CSAMPLE_GAIN gain_delta7 = (gain7out - gain7in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain7 = gain7in + gain_delta7;
    const CSAMPLE_GAIN gain_delta8 = (gain8out - gain8in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain8 = gain8in + gain_delta8;
    const CSAMPLE_GAIN gain_delta9 = (gain9out - gain9in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain9 = gain9in + gain_delta9;
    const CSAMPLE_GAIN gain_delta10 = (gain10out - gain10in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain10 = gain10in + gain_delta10;
    const CSAMPLE_GAIN gain_delta11 = (gain11out - gain11in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain11 = gain11in + gain_delta11;
    const CSAMPLE_GAIN gain_delta12 = (gain12out - gain12in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain12 = gain12in + gain_delta12;
    const CSAMPLE_GAIN gain_delta13 = (gain13out - gain13in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain13 = gain13in + gain_delta13;
    const CSAMPLE_GAIN gain_delta14 = (gain14out - gain14in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain14 = gain14in + gain_delta14;
    const CSAMPLE_GAIN gain_delta15 = (gain15out - gain15in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain15 = gain15in + gain_delta15;
    const CSAMPLE_GAIN gain_delta16 = (gain16out - gain16in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain16 = gain16in + gain_delta16;
    const CSAMPLE_GAIN gain_delta17 = (gain17out - gain17in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain17 = gain17in + gain_delta17;
    const CSAMPLE_GAIN gain_delta18 = (gain18out - gain18in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain18 = gain18in + gain_delta18;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        const CSAMPLE_GAIN gain2 = start_gain2 + gain_delta2 * i;
        const CSAMPLE_GAIN gain3 = start_gain3 + gain_delta3 * i;
        const CSAMPLE_GAIN gain4 = start_gain4 + gain_delta4 * i;
        const CSAMPLE_GAIN gain5 = start_gain5 + gain_delta5 * i;
        const CSAMPLE_GAIN gain6 = start_gain6 + gain_delta6 * i;
        const CSAMPLE_GAIN gain7 = start_gain7 + gain_delta7 * i;
        const CSAMPLE_GAIN gain8 = start_gain8 + gain_delta8 * i;
        const CSAMPLE_GAIN gain9 = start_gain9 + gain_delta9 * i;
        const CSAMPLE_GAIN gain10 = start_gain10 + gain_delta10 * i;
        const CSAMPLE_GAIN gain11 = start_gain11 + gain_delta11 * i;
        const CSAMPLE_GAIN gain12 = start_gain12 + gain_delta12 * i;
        const CSAMPLE_GAIN gain13 = start_gain13 + gain_delta13 * i;
        const CSAMPLE_GAIN gain14 = start_gain14 + gain_delta14 * i;
        const CSAMPLE_GAIN gain15 = start_gain15 + gain_delta15 * i;
        const CSAMPLE_GAIN gain16 = start_gain16 + gain_delta16 * i;
        const CSAMPLE_GAIN gain17 = start_gain17 + gain_delta17 * i;
        const CSAMPLE_GAIN gain18 = start_gain18 + gain_delta18 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1 +
                       pSrc2[i * 2] * gain2 +
                       pSrc3[i * 2] * gain3 +
                       pSrc4[i * 2] * gain4 +
                       pSrc5[i * 2] * gain5 +
                       pSrc6[i * 2] * gain6 +
                       pSrc7[i * 2] * gain7 +
                       pSrc8[i * 2] * gain8 +
                       pSrc9[i * 2] * gain9 +
                       pSrc10[i * 2] * gain10 +
                       pSrc11[i * 2] * gain11 +
                       pSrc12[i * 2] * gain12 +
                       pSrc13[i * 2] * gain13 +
                       pSrc14[i * 2] * gain14 +
                       pSrc15[i * 2] * gain15 +
                       pSrc16[i * 2] * gain16 +
                       pSrc17[i * 2] * gain17 +
                       pSrc18[i * 2] * gain18;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1 +
                           pSrc2[i * 2 + 1] * gain2 +
                           pSrc3[i * 2 + 1] * gain3 +
                           pSrc4[i * 2 + 1] * gain4 +
                           pSrc5[i * 2 + 1] * gain5 +
                           pSrc6[i * 2 + 1] * gain6 +
                           pSrc7[i * 2 + 1] * gain7 +
                           pSrc8[i * 2 + 1] * gain8 +
                           pSrc9[i * 2 + 1] * gain9 +
                           pSrc10[i * 2 + 1] * gain10 +
                           pSrc11[i * 2 + 1] * gain11 +
                           pSrc12[i * 2 + 1] * gain12 +
                           pSrc13[i * 2 + 1] * gain13 +
                           pSrc14[i * 2 + 1] * gain14 +
                           pSrc15[i * 2 + 1] * gain15 +
                           pSrc16[i * 2 + 1] * gain16 +
                           pSrc17[i * 2 + 1] * gain17 +
                           pSrc18[i * 2 + 1] * gain18;
    }
}
static inline void copy20WithGain(CSAMPLE* M_RESTRICT pDest,
                                  const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                  const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                  const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                  const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3,
                                  const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4,
                                  const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5,
                                  const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6,
                                  const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7,
                                  const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8,
                                  const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9,
                                  const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10,
                                  const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11,
                                  const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12,
                                  const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13,
                                  const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14,
                                  const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15,
                                  const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16,
                                  const CSAMPLE* M_RESTRICT pSrc17, CSAMPLE_GAIN gain17,
                                  const CSAMPLE* M_RESTRICT pSrc18, CSAMPLE_GAIN gain18,
                                  const CSAMPLE* M_RESTRICT pSrc19, CSAMPLE_GAIN gain19,
                                  int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy19WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain2 == CSAMPLE_GAIN_ZERO) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain3 == CSAMPLE_GAIN_ZERO) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain4 == CSAMPLE_GAIN_ZERO) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain5 == CSAMPLE_GAIN_ZERO) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain6 == CSAMPLE_GAIN_ZERO) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain7 == CSAMPLE_GAIN_ZERO) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain8 == CSAMPLE_GAIN_ZERO) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain9 == CSAMPLE_GAIN_ZERO) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain10 == CSAMPLE_GAIN_ZERO) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain11 == CSAMPLE_GAIN_ZERO) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain12 == CSAMPLE_GAIN_ZERO) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain13 == CSAMPLE_GAIN_ZERO) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain14 == CSAMPLE_GAIN_ZERO) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain15 == CSAMPLE_GAIN_ZERO) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain16 == CSAMPLE_GAIN_ZERO) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain17 == CSAMPLE_GAIN_ZERO) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain18 == CSAMPLE_GAIN_ZERO) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc19, gain19, iNumSamples);
        return;
    }
    if (gain19 == CSAMPLE_GAIN_ZERO) {
        copy19WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
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
static inline void copy20WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                         const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                         const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                         const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                         const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3in, CSAMPLE_GAIN gain3out,
                                         const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4in, CSAMPLE_GAIN gain4out,
                                         const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5in, CSAMPLE_GAIN gain5out,
                                         const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6in, CSAMPLE_GAIN gain6out,
                                         const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7in, CSAMPLE_GAIN gain7out,
                                         const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8in, CSAMPLE_GAIN gain8out,
                                         const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9in, CSAMPLE_GAIN gain9out,
                                         const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10in, CSAMPLE_GAIN gain10out,
                                         const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11in, CSAMPLE_GAIN gain11out,
                                         const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12in, CSAMPLE_GAIN gain12out,
                                         const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13in, CSAMPLE_GAIN gain13out,
                                         const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14in, CSAMPLE_GAIN gain14out,
                                         const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15in, CSAMPLE_GAIN gain15out,
                                         const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16in, CSAMPLE_GAIN gain16out,
                                         const CSAMPLE* M_RESTRICT pSrc17, CSAMPLE_GAIN gain17in, CSAMPLE_GAIN gain17out,
                                         const CSAMPLE* M_RESTRICT pSrc18, CSAMPLE_GAIN gain18in, CSAMPLE_GAIN gain18out,
                                         const CSAMPLE* M_RESTRICT pSrc19, CSAMPLE_GAIN gain19in, CSAMPLE_GAIN gain19out,
                                         int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy19WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain2in == CSAMPLE_GAIN_ZERO && gain2out == CSAMPLE_GAIN_ZERO) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain3in == CSAMPLE_GAIN_ZERO && gain3out == CSAMPLE_GAIN_ZERO) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain4in == CSAMPLE_GAIN_ZERO && gain4out == CSAMPLE_GAIN_ZERO) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain5in == CSAMPLE_GAIN_ZERO && gain5out == CSAMPLE_GAIN_ZERO) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain6in == CSAMPLE_GAIN_ZERO && gain6out == CSAMPLE_GAIN_ZERO) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain7in == CSAMPLE_GAIN_ZERO && gain7out == CSAMPLE_GAIN_ZERO) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain8in == CSAMPLE_GAIN_ZERO && gain8out == CSAMPLE_GAIN_ZERO) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain9in == CSAMPLE_GAIN_ZERO && gain9out == CSAMPLE_GAIN_ZERO) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain10in == CSAMPLE_GAIN_ZERO && gain10out == CSAMPLE_GAIN_ZERO) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain11in == CSAMPLE_GAIN_ZERO && gain11out == CSAMPLE_GAIN_ZERO) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain12in == CSAMPLE_GAIN_ZERO && gain12out == CSAMPLE_GAIN_ZERO) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain13in == CSAMPLE_GAIN_ZERO && gain13out == CSAMPLE_GAIN_ZERO) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain14in == CSAMPLE_GAIN_ZERO && gain14out == CSAMPLE_GAIN_ZERO) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain15in == CSAMPLE_GAIN_ZERO && gain15out == CSAMPLE_GAIN_ZERO) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain16in == CSAMPLE_GAIN_ZERO && gain16out == CSAMPLE_GAIN_ZERO) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain17in == CSAMPLE_GAIN_ZERO && gain17out == CSAMPLE_GAIN_ZERO) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain18in == CSAMPLE_GAIN_ZERO && gain18out == CSAMPLE_GAIN_ZERO) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    if (gain19in == CSAMPLE_GAIN_ZERO && gain19out == CSAMPLE_GAIN_ZERO) {
        copy19WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    const CSAMPLE_GAIN gain_delta2 = (gain2out - gain2in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain2 = gain2in + gain_delta2;
    const CSAMPLE_GAIN gain_delta3 = (gain3out - gain3in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain3 = gain3in + gain_delta3;
    const CSAMPLE_GAIN gain_delta4 = (gain4out - gain4in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain4 = gain4in + gain_delta4;
    const CSAMPLE_GAIN gain_delta5 = (gain5out - gain5in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain5 = gain5in + gain_delta5;
    const CSAMPLE_GAIN gain_delta6 = (gain6out - gain6in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain6 = gain6in + gain_delta6;
    const CSAMPLE_GAIN gain_delta7 = (gain7out - gain7in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain7 = gain7in + gain_delta7;
    const CSAMPLE_GAIN gain_delta8 = (gain8out - gain8in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain8 = gain8in + gain_delta8;
    const CSAMPLE_GAIN gain_delta9 = (gain9out - gain9in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain9 = gain9in + gain_delta9;
    const CSAMPLE_GAIN gain_delta10 = (gain10out - gain10in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain10 = gain10in + gain_delta10;
    const CSAMPLE_GAIN gain_delta11 = (gain11out - gain11in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain11 = gain11in + gain_delta11;
    const CSAMPLE_GAIN gain_delta12 = (gain12out - gain12in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain12 = gain12in + gain_delta12;
    const CSAMPLE_GAIN gain_delta13 = (gain13out - gain13in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain13 = gain13in + gain_delta13;
    const CSAMPLE_GAIN gain_delta14 = (gain14out - gain14in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain14 = gain14in + gain_delta14;
    const CSAMPLE_GAIN gain_delta15 = (gain15out - gain15in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain15 = gain15in + gain_delta15;
    const CSAMPLE_GAIN gain_delta16 = (gain16out - gain16in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain16 = gain16in + gain_delta16;
    const CSAMPLE_GAIN gain_delta17 = (gain17out - gain17in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain17 = gain17in + gain_delta17;
    const CSAMPLE_GAIN gain_delta18 = (gain18out - gain18in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain18 = gain18in + gain_delta18;
    const CSAMPLE_GAIN gain_delta19 = (gain19out - gain19in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain19 = gain19in + gain_delta19;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        const CSAMPLE_GAIN gain2 = start_gain2 + gain_delta2 * i;
        const CSAMPLE_GAIN gain3 = start_gain3 + gain_delta3 * i;
        const CSAMPLE_GAIN gain4 = start_gain4 + gain_delta4 * i;
        const CSAMPLE_GAIN gain5 = start_gain5 + gain_delta5 * i;
        const CSAMPLE_GAIN gain6 = start_gain6 + gain_delta6 * i;
        const CSAMPLE_GAIN gain7 = start_gain7 + gain_delta7 * i;
        const CSAMPLE_GAIN gain8 = start_gain8 + gain_delta8 * i;
        const CSAMPLE_GAIN gain9 = start_gain9 + gain_delta9 * i;
        const CSAMPLE_GAIN gain10 = start_gain10 + gain_delta10 * i;
        const CSAMPLE_GAIN gain11 = start_gain11 + gain_delta11 * i;
        const CSAMPLE_GAIN gain12 = start_gain12 + gain_delta12 * i;
        const CSAMPLE_GAIN gain13 = start_gain13 + gain_delta13 * i;
        const CSAMPLE_GAIN gain14 = start_gain14 + gain_delta14 * i;
        const CSAMPLE_GAIN gain15 = start_gain15 + gain_delta15 * i;
        const CSAMPLE_GAIN gain16 = start_gain16 + gain_delta16 * i;
        const CSAMPLE_GAIN gain17 = start_gain17 + gain_delta17 * i;
        const CSAMPLE_GAIN gain18 = start_gain18 + gain_delta18 * i;
        const CSAMPLE_GAIN gain19 = start_gain19 + gain_delta19 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1 +
                       pSrc2[i * 2] * gain2 +
                       pSrc3[i * 2] * gain3 +
                       pSrc4[i * 2] * gain4 +
                       pSrc5[i * 2] * gain5 +
                       pSrc6[i * 2] * gain6 +
                       pSrc7[i * 2] * gain7 +
                       pSrc8[i * 2] * gain8 +
                       pSrc9[i * 2] * gain9 +
                       pSrc10[i * 2] * gain10 +
                       pSrc11[i * 2] * gain11 +
                       pSrc12[i * 2] * gain12 +
                       pSrc13[i * 2] * gain13 +
                       pSrc14[i * 2] * gain14 +
                       pSrc15[i * 2] * gain15 +
                       pSrc16[i * 2] * gain16 +
                       pSrc17[i * 2] * gain17 +
                       pSrc18[i * 2] * gain18 +
                       pSrc19[i * 2] * gain19;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1 +
                           pSrc2[i * 2 + 1] * gain2 +
                           pSrc3[i * 2 + 1] * gain3 +
                           pSrc4[i * 2 + 1] * gain4 +
                           pSrc5[i * 2 + 1] * gain5 +
                           pSrc6[i * 2 + 1] * gain6 +
                           pSrc7[i * 2 + 1] * gain7 +
                           pSrc8[i * 2 + 1] * gain8 +
                           pSrc9[i * 2 + 1] * gain9 +
                           pSrc10[i * 2 + 1] * gain10 +
                           pSrc11[i * 2 + 1] * gain11 +
                           pSrc12[i * 2 + 1] * gain12 +
                           pSrc13[i * 2 + 1] * gain13 +
                           pSrc14[i * 2 + 1] * gain14 +
                           pSrc15[i * 2 + 1] * gain15 +
                           pSrc16[i * 2 + 1] * gain16 +
                           pSrc17[i * 2 + 1] * gain17 +
                           pSrc18[i * 2 + 1] * gain18 +
                           pSrc19[i * 2 + 1] * gain19;
    }
}
static inline void copy21WithGain(CSAMPLE* M_RESTRICT pDest,
                                  const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                  const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                  const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                  const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3,
                                  const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4,
                                  const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5,
                                  const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6,
                                  const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7,
                                  const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8,
                                  const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9,
                                  const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10,
                                  const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11,
                                  const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12,
                                  const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13,
                                  const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14,
                                  const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15,
                                  const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16,
                                  const CSAMPLE* M_RESTRICT pSrc17, CSAMPLE_GAIN gain17,
                                  const CSAMPLE* M_RESTRICT pSrc18, CSAMPLE_GAIN gain18,
                                  const CSAMPLE* M_RESTRICT pSrc19, CSAMPLE_GAIN gain19,
                                  const CSAMPLE* M_RESTRICT pSrc20, CSAMPLE_GAIN gain20,
                                  int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy20WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain2 == CSAMPLE_GAIN_ZERO) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain3 == CSAMPLE_GAIN_ZERO) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain4 == CSAMPLE_GAIN_ZERO) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain5 == CSAMPLE_GAIN_ZERO) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain6 == CSAMPLE_GAIN_ZERO) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain7 == CSAMPLE_GAIN_ZERO) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain8 == CSAMPLE_GAIN_ZERO) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain9 == CSAMPLE_GAIN_ZERO) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain10 == CSAMPLE_GAIN_ZERO) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain11 == CSAMPLE_GAIN_ZERO) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain12 == CSAMPLE_GAIN_ZERO) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain13 == CSAMPLE_GAIN_ZERO) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain14 == CSAMPLE_GAIN_ZERO) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain15 == CSAMPLE_GAIN_ZERO) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain16 == CSAMPLE_GAIN_ZERO) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain17 == CSAMPLE_GAIN_ZERO) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain18 == CSAMPLE_GAIN_ZERO) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain19 == CSAMPLE_GAIN_ZERO) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc20, gain20, iNumSamples);
        return;
    }
    if (gain20 == CSAMPLE_GAIN_ZERO) {
        copy20WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
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
static inline void copy21WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                         const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                         const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                         const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                         const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3in, CSAMPLE_GAIN gain3out,
                                         const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4in, CSAMPLE_GAIN gain4out,
                                         const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5in, CSAMPLE_GAIN gain5out,
                                         const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6in, CSAMPLE_GAIN gain6out,
                                         const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7in, CSAMPLE_GAIN gain7out,
                                         const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8in, CSAMPLE_GAIN gain8out,
                                         const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9in, CSAMPLE_GAIN gain9out,
                                         const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10in, CSAMPLE_GAIN gain10out,
                                         const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11in, CSAMPLE_GAIN gain11out,
                                         const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12in, CSAMPLE_GAIN gain12out,
                                         const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13in, CSAMPLE_GAIN gain13out,
                                         const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14in, CSAMPLE_GAIN gain14out,
                                         const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15in, CSAMPLE_GAIN gain15out,
                                         const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16in, CSAMPLE_GAIN gain16out,
                                         const CSAMPLE* M_RESTRICT pSrc17, CSAMPLE_GAIN gain17in, CSAMPLE_GAIN gain17out,
                                         const CSAMPLE* M_RESTRICT pSrc18, CSAMPLE_GAIN gain18in, CSAMPLE_GAIN gain18out,
                                         const CSAMPLE* M_RESTRICT pSrc19, CSAMPLE_GAIN gain19in, CSAMPLE_GAIN gain19out,
                                         const CSAMPLE* M_RESTRICT pSrc20, CSAMPLE_GAIN gain20in, CSAMPLE_GAIN gain20out,
                                         int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy20WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain2in == CSAMPLE_GAIN_ZERO && gain2out == CSAMPLE_GAIN_ZERO) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain3in == CSAMPLE_GAIN_ZERO && gain3out == CSAMPLE_GAIN_ZERO) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain4in == CSAMPLE_GAIN_ZERO && gain4out == CSAMPLE_GAIN_ZERO) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain5in == CSAMPLE_GAIN_ZERO && gain5out == CSAMPLE_GAIN_ZERO) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain6in == CSAMPLE_GAIN_ZERO && gain6out == CSAMPLE_GAIN_ZERO) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain7in == CSAMPLE_GAIN_ZERO && gain7out == CSAMPLE_GAIN_ZERO) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain8in == CSAMPLE_GAIN_ZERO && gain8out == CSAMPLE_GAIN_ZERO) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain9in == CSAMPLE_GAIN_ZERO && gain9out == CSAMPLE_GAIN_ZERO) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain10in == CSAMPLE_GAIN_ZERO && gain10out == CSAMPLE_GAIN_ZERO) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain11in == CSAMPLE_GAIN_ZERO && gain11out == CSAMPLE_GAIN_ZERO) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain12in == CSAMPLE_GAIN_ZERO && gain12out == CSAMPLE_GAIN_ZERO) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain13in == CSAMPLE_GAIN_ZERO && gain13out == CSAMPLE_GAIN_ZERO) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain14in == CSAMPLE_GAIN_ZERO && gain14out == CSAMPLE_GAIN_ZERO) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain15in == CSAMPLE_GAIN_ZERO && gain15out == CSAMPLE_GAIN_ZERO) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain16in == CSAMPLE_GAIN_ZERO && gain16out == CSAMPLE_GAIN_ZERO) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain17in == CSAMPLE_GAIN_ZERO && gain17out == CSAMPLE_GAIN_ZERO) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain18in == CSAMPLE_GAIN_ZERO && gain18out == CSAMPLE_GAIN_ZERO) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain19in == CSAMPLE_GAIN_ZERO && gain19out == CSAMPLE_GAIN_ZERO) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    if (gain20in == CSAMPLE_GAIN_ZERO && gain20out == CSAMPLE_GAIN_ZERO) {
        copy20WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    const CSAMPLE_GAIN gain_delta2 = (gain2out - gain2in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain2 = gain2in + gain_delta2;
    const CSAMPLE_GAIN gain_delta3 = (gain3out - gain3in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain3 = gain3in + gain_delta3;
    const CSAMPLE_GAIN gain_delta4 = (gain4out - gain4in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain4 = gain4in + gain_delta4;
    const CSAMPLE_GAIN gain_delta5 = (gain5out - gain5in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain5 = gain5in + gain_delta5;
    const CSAMPLE_GAIN gain_delta6 = (gain6out - gain6in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain6 = gain6in + gain_delta6;
    const CSAMPLE_GAIN gain_delta7 = (gain7out - gain7in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain7 = gain7in + gain_delta7;
    const CSAMPLE_GAIN gain_delta8 = (gain8out - gain8in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain8 = gain8in + gain_delta8;
    const CSAMPLE_GAIN gain_delta9 = (gain9out - gain9in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain9 = gain9in + gain_delta9;
    const CSAMPLE_GAIN gain_delta10 = (gain10out - gain10in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain10 = gain10in + gain_delta10;
    const CSAMPLE_GAIN gain_delta11 = (gain11out - gain11in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain11 = gain11in + gain_delta11;
    const CSAMPLE_GAIN gain_delta12 = (gain12out - gain12in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain12 = gain12in + gain_delta12;
    const CSAMPLE_GAIN gain_delta13 = (gain13out - gain13in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain13 = gain13in + gain_delta13;
    const CSAMPLE_GAIN gain_delta14 = (gain14out - gain14in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain14 = gain14in + gain_delta14;
    const CSAMPLE_GAIN gain_delta15 = (gain15out - gain15in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain15 = gain15in + gain_delta15;
    const CSAMPLE_GAIN gain_delta16 = (gain16out - gain16in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain16 = gain16in + gain_delta16;
    const CSAMPLE_GAIN gain_delta17 = (gain17out - gain17in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain17 = gain17in + gain_delta17;
    const CSAMPLE_GAIN gain_delta18 = (gain18out - gain18in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain18 = gain18in + gain_delta18;
    const CSAMPLE_GAIN gain_delta19 = (gain19out - gain19in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain19 = gain19in + gain_delta19;
    const CSAMPLE_GAIN gain_delta20 = (gain20out - gain20in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain20 = gain20in + gain_delta20;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        const CSAMPLE_GAIN gain2 = start_gain2 + gain_delta2 * i;
        const CSAMPLE_GAIN gain3 = start_gain3 + gain_delta3 * i;
        const CSAMPLE_GAIN gain4 = start_gain4 + gain_delta4 * i;
        const CSAMPLE_GAIN gain5 = start_gain5 + gain_delta5 * i;
        const CSAMPLE_GAIN gain6 = start_gain6 + gain_delta6 * i;
        const CSAMPLE_GAIN gain7 = start_gain7 + gain_delta7 * i;
        const CSAMPLE_GAIN gain8 = start_gain8 + gain_delta8 * i;
        const CSAMPLE_GAIN gain9 = start_gain9 + gain_delta9 * i;
        const CSAMPLE_GAIN gain10 = start_gain10 + gain_delta10 * i;
        const CSAMPLE_GAIN gain11 = start_gain11 + gain_delta11 * i;
        const CSAMPLE_GAIN gain12 = start_gain12 + gain_delta12 * i;
        const CSAMPLE_GAIN gain13 = start_gain13 + gain_delta13 * i;
        const CSAMPLE_GAIN gain14 = start_gain14 + gain_delta14 * i;
        const CSAMPLE_GAIN gain15 = start_gain15 + gain_delta15 * i;
        const CSAMPLE_GAIN gain16 = start_gain16 + gain_delta16 * i;
        const CSAMPLE_GAIN gain17 = start_gain17 + gain_delta17 * i;
        const CSAMPLE_GAIN gain18 = start_gain18 + gain_delta18 * i;
        const CSAMPLE_GAIN gain19 = start_gain19 + gain_delta19 * i;
        const CSAMPLE_GAIN gain20 = start_gain20 + gain_delta20 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1 +
                       pSrc2[i * 2] * gain2 +
                       pSrc3[i * 2] * gain3 +
                       pSrc4[i * 2] * gain4 +
                       pSrc5[i * 2] * gain5 +
                       pSrc6[i * 2] * gain6 +
                       pSrc7[i * 2] * gain7 +
                       pSrc8[i * 2] * gain8 +
                       pSrc9[i * 2] * gain9 +
                       pSrc10[i * 2] * gain10 +
                       pSrc11[i * 2] * gain11 +
                       pSrc12[i * 2] * gain12 +
                       pSrc13[i * 2] * gain13 +
                       pSrc14[i * 2] * gain14 +
                       pSrc15[i * 2] * gain15 +
                       pSrc16[i * 2] * gain16 +
                       pSrc17[i * 2] * gain17 +
                       pSrc18[i * 2] * gain18 +
                       pSrc19[i * 2] * gain19 +
                       pSrc20[i * 2] * gain20;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1 +
                           pSrc2[i * 2 + 1] * gain2 +
                           pSrc3[i * 2 + 1] * gain3 +
                           pSrc4[i * 2 + 1] * gain4 +
                           pSrc5[i * 2 + 1] * gain5 +
                           pSrc6[i * 2 + 1] * gain6 +
                           pSrc7[i * 2 + 1] * gain7 +
                           pSrc8[i * 2 + 1] * gain8 +
                           pSrc9[i * 2 + 1] * gain9 +
                           pSrc10[i * 2 + 1] * gain10 +
                           pSrc11[i * 2 + 1] * gain11 +
                           pSrc12[i * 2 + 1] * gain12 +
                           pSrc13[i * 2 + 1] * gain13 +
                           pSrc14[i * 2 + 1] * gain14 +
                           pSrc15[i * 2 + 1] * gain15 +
                           pSrc16[i * 2 + 1] * gain16 +
                           pSrc17[i * 2 + 1] * gain17 +
                           pSrc18[i * 2 + 1] * gain18 +
                           pSrc19[i * 2 + 1] * gain19 +
                           pSrc20[i * 2 + 1] * gain20;
    }
}
static inline void copy22WithGain(CSAMPLE* M_RESTRICT pDest,
                                  const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                  const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                  const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                  const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3,
                                  const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4,
                                  const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5,
                                  const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6,
                                  const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7,
                                  const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8,
                                  const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9,
                                  const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10,
                                  const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11,
                                  const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12,
                                  const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13,
                                  const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14,
                                  const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15,
                                  const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16,
                                  const CSAMPLE* M_RESTRICT pSrc17, CSAMPLE_GAIN gain17,
                                  const CSAMPLE* M_RESTRICT pSrc18, CSAMPLE_GAIN gain18,
                                  const CSAMPLE* M_RESTRICT pSrc19, CSAMPLE_GAIN gain19,
                                  const CSAMPLE* M_RESTRICT pSrc20, CSAMPLE_GAIN gain20,
                                  const CSAMPLE* M_RESTRICT pSrc21, CSAMPLE_GAIN gain21,
                                  int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy21WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain2 == CSAMPLE_GAIN_ZERO) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain3 == CSAMPLE_GAIN_ZERO) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain4 == CSAMPLE_GAIN_ZERO) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain5 == CSAMPLE_GAIN_ZERO) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain6 == CSAMPLE_GAIN_ZERO) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain7 == CSAMPLE_GAIN_ZERO) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain8 == CSAMPLE_GAIN_ZERO) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain9 == CSAMPLE_GAIN_ZERO) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain10 == CSAMPLE_GAIN_ZERO) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain11 == CSAMPLE_GAIN_ZERO) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain12 == CSAMPLE_GAIN_ZERO) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain13 == CSAMPLE_GAIN_ZERO) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain14 == CSAMPLE_GAIN_ZERO) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain15 == CSAMPLE_GAIN_ZERO) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain16 == CSAMPLE_GAIN_ZERO) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain17 == CSAMPLE_GAIN_ZERO) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain18 == CSAMPLE_GAIN_ZERO) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain19 == CSAMPLE_GAIN_ZERO) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain20 == CSAMPLE_GAIN_ZERO) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc21, gain21, iNumSamples);
        return;
    }
    if (gain21 == CSAMPLE_GAIN_ZERO) {
        copy21WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
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
static inline void copy22WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                         const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                         const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                         const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                         const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3in, CSAMPLE_GAIN gain3out,
                                         const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4in, CSAMPLE_GAIN gain4out,
                                         const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5in, CSAMPLE_GAIN gain5out,
                                         const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6in, CSAMPLE_GAIN gain6out,
                                         const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7in, CSAMPLE_GAIN gain7out,
                                         const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8in, CSAMPLE_GAIN gain8out,
                                         const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9in, CSAMPLE_GAIN gain9out,
                                         const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10in, CSAMPLE_GAIN gain10out,
                                         const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11in, CSAMPLE_GAIN gain11out,
                                         const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12in, CSAMPLE_GAIN gain12out,
                                         const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13in, CSAMPLE_GAIN gain13out,
                                         const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14in, CSAMPLE_GAIN gain14out,
                                         const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15in, CSAMPLE_GAIN gain15out,
                                         const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16in, CSAMPLE_GAIN gain16out,
                                         const CSAMPLE* M_RESTRICT pSrc17, CSAMPLE_GAIN gain17in, CSAMPLE_GAIN gain17out,
                                         const CSAMPLE* M_RESTRICT pSrc18, CSAMPLE_GAIN gain18in, CSAMPLE_GAIN gain18out,
                                         const CSAMPLE* M_RESTRICT pSrc19, CSAMPLE_GAIN gain19in, CSAMPLE_GAIN gain19out,
                                         const CSAMPLE* M_RESTRICT pSrc20, CSAMPLE_GAIN gain20in, CSAMPLE_GAIN gain20out,
                                         const CSAMPLE* M_RESTRICT pSrc21, CSAMPLE_GAIN gain21in, CSAMPLE_GAIN gain21out,
                                         int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy21WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain2in == CSAMPLE_GAIN_ZERO && gain2out == CSAMPLE_GAIN_ZERO) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain3in == CSAMPLE_GAIN_ZERO && gain3out == CSAMPLE_GAIN_ZERO) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain4in == CSAMPLE_GAIN_ZERO && gain4out == CSAMPLE_GAIN_ZERO) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain5in == CSAMPLE_GAIN_ZERO && gain5out == CSAMPLE_GAIN_ZERO) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain6in == CSAMPLE_GAIN_ZERO && gain6out == CSAMPLE_GAIN_ZERO) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain7in == CSAMPLE_GAIN_ZERO && gain7out == CSAMPLE_GAIN_ZERO) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain8in == CSAMPLE_GAIN_ZERO && gain8out == CSAMPLE_GAIN_ZERO) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain9in == CSAMPLE_GAIN_ZERO && gain9out == CSAMPLE_GAIN_ZERO) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain10in == CSAMPLE_GAIN_ZERO && gain10out == CSAMPLE_GAIN_ZERO) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain11in == CSAMPLE_GAIN_ZERO && gain11out == CSAMPLE_GAIN_ZERO) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain12in == CSAMPLE_GAIN_ZERO && gain12out == CSAMPLE_GAIN_ZERO) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain13in == CSAMPLE_GAIN_ZERO && gain13out == CSAMPLE_GAIN_ZERO) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain14in == CSAMPLE_GAIN_ZERO && gain14out == CSAMPLE_GAIN_ZERO) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain15in == CSAMPLE_GAIN_ZERO && gain15out == CSAMPLE_GAIN_ZERO) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain16in == CSAMPLE_GAIN_ZERO && gain16out == CSAMPLE_GAIN_ZERO) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain17in == CSAMPLE_GAIN_ZERO && gain17out == CSAMPLE_GAIN_ZERO) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain18in == CSAMPLE_GAIN_ZERO && gain18out == CSAMPLE_GAIN_ZERO) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain19in == CSAMPLE_GAIN_ZERO && gain19out == CSAMPLE_GAIN_ZERO) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain20in == CSAMPLE_GAIN_ZERO && gain20out == CSAMPLE_GAIN_ZERO) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    if (gain21in == CSAMPLE_GAIN_ZERO && gain21out == CSAMPLE_GAIN_ZERO) {
        copy21WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    const CSAMPLE_GAIN gain_delta2 = (gain2out - gain2in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain2 = gain2in + gain_delta2;
    const CSAMPLE_GAIN gain_delta3 = (gain3out - gain3in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain3 = gain3in + gain_delta3;
    const CSAMPLE_GAIN gain_delta4 = (gain4out - gain4in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain4 = gain4in + gain_delta4;
    const CSAMPLE_GAIN gain_delta5 = (gain5out - gain5in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain5 = gain5in + gain_delta5;
    const CSAMPLE_GAIN gain_delta6 = (gain6out - gain6in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain6 = gain6in + gain_delta6;
    const CSAMPLE_GAIN gain_delta7 = (gain7out - gain7in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain7 = gain7in + gain_delta7;
    const CSAMPLE_GAIN gain_delta8 = (gain8out - gain8in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain8 = gain8in + gain_delta8;
    const CSAMPLE_GAIN gain_delta9 = (gain9out - gain9in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain9 = gain9in + gain_delta9;
    const CSAMPLE_GAIN gain_delta10 = (gain10out - gain10in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain10 = gain10in + gain_delta10;
    const CSAMPLE_GAIN gain_delta11 = (gain11out - gain11in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain11 = gain11in + gain_delta11;
    const CSAMPLE_GAIN gain_delta12 = (gain12out - gain12in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain12 = gain12in + gain_delta12;
    const CSAMPLE_GAIN gain_delta13 = (gain13out - gain13in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain13 = gain13in + gain_delta13;
    const CSAMPLE_GAIN gain_delta14 = (gain14out - gain14in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain14 = gain14in + gain_delta14;
    const CSAMPLE_GAIN gain_delta15 = (gain15out - gain15in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain15 = gain15in + gain_delta15;
    const CSAMPLE_GAIN gain_delta16 = (gain16out - gain16in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain16 = gain16in + gain_delta16;
    const CSAMPLE_GAIN gain_delta17 = (gain17out - gain17in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain17 = gain17in + gain_delta17;
    const CSAMPLE_GAIN gain_delta18 = (gain18out - gain18in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain18 = gain18in + gain_delta18;
    const CSAMPLE_GAIN gain_delta19 = (gain19out - gain19in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain19 = gain19in + gain_delta19;
    const CSAMPLE_GAIN gain_delta20 = (gain20out - gain20in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain20 = gain20in + gain_delta20;
    const CSAMPLE_GAIN gain_delta21 = (gain21out - gain21in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain21 = gain21in + gain_delta21;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        const CSAMPLE_GAIN gain2 = start_gain2 + gain_delta2 * i;
        const CSAMPLE_GAIN gain3 = start_gain3 + gain_delta3 * i;
        const CSAMPLE_GAIN gain4 = start_gain4 + gain_delta4 * i;
        const CSAMPLE_GAIN gain5 = start_gain5 + gain_delta5 * i;
        const CSAMPLE_GAIN gain6 = start_gain6 + gain_delta6 * i;
        const CSAMPLE_GAIN gain7 = start_gain7 + gain_delta7 * i;
        const CSAMPLE_GAIN gain8 = start_gain8 + gain_delta8 * i;
        const CSAMPLE_GAIN gain9 = start_gain9 + gain_delta9 * i;
        const CSAMPLE_GAIN gain10 = start_gain10 + gain_delta10 * i;
        const CSAMPLE_GAIN gain11 = start_gain11 + gain_delta11 * i;
        const CSAMPLE_GAIN gain12 = start_gain12 + gain_delta12 * i;
        const CSAMPLE_GAIN gain13 = start_gain13 + gain_delta13 * i;
        const CSAMPLE_GAIN gain14 = start_gain14 + gain_delta14 * i;
        const CSAMPLE_GAIN gain15 = start_gain15 + gain_delta15 * i;
        const CSAMPLE_GAIN gain16 = start_gain16 + gain_delta16 * i;
        const CSAMPLE_GAIN gain17 = start_gain17 + gain_delta17 * i;
        const CSAMPLE_GAIN gain18 = start_gain18 + gain_delta18 * i;
        const CSAMPLE_GAIN gain19 = start_gain19 + gain_delta19 * i;
        const CSAMPLE_GAIN gain20 = start_gain20 + gain_delta20 * i;
        const CSAMPLE_GAIN gain21 = start_gain21 + gain_delta21 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1 +
                       pSrc2[i * 2] * gain2 +
                       pSrc3[i * 2] * gain3 +
                       pSrc4[i * 2] * gain4 +
                       pSrc5[i * 2] * gain5 +
                       pSrc6[i * 2] * gain6 +
                       pSrc7[i * 2] * gain7 +
                       pSrc8[i * 2] * gain8 +
                       pSrc9[i * 2] * gain9 +
                       pSrc10[i * 2] * gain10 +
                       pSrc11[i * 2] * gain11 +
                       pSrc12[i * 2] * gain12 +
                       pSrc13[i * 2] * gain13 +
                       pSrc14[i * 2] * gain14 +
                       pSrc15[i * 2] * gain15 +
                       pSrc16[i * 2] * gain16 +
                       pSrc17[i * 2] * gain17 +
                       pSrc18[i * 2] * gain18 +
                       pSrc19[i * 2] * gain19 +
                       pSrc20[i * 2] * gain20 +
                       pSrc21[i * 2] * gain21;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1 +
                           pSrc2[i * 2 + 1] * gain2 +
                           pSrc3[i * 2 + 1] * gain3 +
                           pSrc4[i * 2 + 1] * gain4 +
                           pSrc5[i * 2 + 1] * gain5 +
                           pSrc6[i * 2 + 1] * gain6 +
                           pSrc7[i * 2 + 1] * gain7 +
                           pSrc8[i * 2 + 1] * gain8 +
                           pSrc9[i * 2 + 1] * gain9 +
                           pSrc10[i * 2 + 1] * gain10 +
                           pSrc11[i * 2 + 1] * gain11 +
                           pSrc12[i * 2 + 1] * gain12 +
                           pSrc13[i * 2 + 1] * gain13 +
                           pSrc14[i * 2 + 1] * gain14 +
                           pSrc15[i * 2 + 1] * gain15 +
                           pSrc16[i * 2 + 1] * gain16 +
                           pSrc17[i * 2 + 1] * gain17 +
                           pSrc18[i * 2 + 1] * gain18 +
                           pSrc19[i * 2 + 1] * gain19 +
                           pSrc20[i * 2 + 1] * gain20 +
                           pSrc21[i * 2 + 1] * gain21;
    }
}
static inline void copy23WithGain(CSAMPLE* M_RESTRICT pDest,
                                  const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                  const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                  const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                  const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3,
                                  const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4,
                                  const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5,
                                  const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6,
                                  const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7,
                                  const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8,
                                  const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9,
                                  const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10,
                                  const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11,
                                  const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12,
                                  const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13,
                                  const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14,
                                  const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15,
                                  const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16,
                                  const CSAMPLE* M_RESTRICT pSrc17, CSAMPLE_GAIN gain17,
                                  const CSAMPLE* M_RESTRICT pSrc18, CSAMPLE_GAIN gain18,
                                  const CSAMPLE* M_RESTRICT pSrc19, CSAMPLE_GAIN gain19,
                                  const CSAMPLE* M_RESTRICT pSrc20, CSAMPLE_GAIN gain20,
                                  const CSAMPLE* M_RESTRICT pSrc21, CSAMPLE_GAIN gain21,
                                  const CSAMPLE* M_RESTRICT pSrc22, CSAMPLE_GAIN gain22,
                                  int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy22WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain2 == CSAMPLE_GAIN_ZERO) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain3 == CSAMPLE_GAIN_ZERO) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain4 == CSAMPLE_GAIN_ZERO) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain5 == CSAMPLE_GAIN_ZERO) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain6 == CSAMPLE_GAIN_ZERO) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain7 == CSAMPLE_GAIN_ZERO) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain8 == CSAMPLE_GAIN_ZERO) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain9 == CSAMPLE_GAIN_ZERO) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain10 == CSAMPLE_GAIN_ZERO) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain11 == CSAMPLE_GAIN_ZERO) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain12 == CSAMPLE_GAIN_ZERO) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain13 == CSAMPLE_GAIN_ZERO) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain14 == CSAMPLE_GAIN_ZERO) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain15 == CSAMPLE_GAIN_ZERO) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain16 == CSAMPLE_GAIN_ZERO) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain17 == CSAMPLE_GAIN_ZERO) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain18 == CSAMPLE_GAIN_ZERO) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain19 == CSAMPLE_GAIN_ZERO) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain20 == CSAMPLE_GAIN_ZERO) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain21 == CSAMPLE_GAIN_ZERO) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc22, gain22, iNumSamples);
        return;
    }
    if (gain22 == CSAMPLE_GAIN_ZERO) {
        copy22WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
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
static inline void copy23WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                         const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                         const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                         const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                         const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3in, CSAMPLE_GAIN gain3out,
                                         const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4in, CSAMPLE_GAIN gain4out,
                                         const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5in, CSAMPLE_GAIN gain5out,
                                         const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6in, CSAMPLE_GAIN gain6out,
                                         const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7in, CSAMPLE_GAIN gain7out,
                                         const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8in, CSAMPLE_GAIN gain8out,
                                         const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9in, CSAMPLE_GAIN gain9out,
                                         const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10in, CSAMPLE_GAIN gain10out,
                                         const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11in, CSAMPLE_GAIN gain11out,
                                         const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12in, CSAMPLE_GAIN gain12out,
                                         const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13in, CSAMPLE_GAIN gain13out,
                                         const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14in, CSAMPLE_GAIN gain14out,
                                         const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15in, CSAMPLE_GAIN gain15out,
                                         const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16in, CSAMPLE_GAIN gain16out,
                                         const CSAMPLE* M_RESTRICT pSrc17, CSAMPLE_GAIN gain17in, CSAMPLE_GAIN gain17out,
                                         const CSAMPLE* M_RESTRICT pSrc18, CSAMPLE_GAIN gain18in, CSAMPLE_GAIN gain18out,
                                         const CSAMPLE* M_RESTRICT pSrc19, CSAMPLE_GAIN gain19in, CSAMPLE_GAIN gain19out,
                                         const CSAMPLE* M_RESTRICT pSrc20, CSAMPLE_GAIN gain20in, CSAMPLE_GAIN gain20out,
                                         const CSAMPLE* M_RESTRICT pSrc21, CSAMPLE_GAIN gain21in, CSAMPLE_GAIN gain21out,
                                         const CSAMPLE* M_RESTRICT pSrc22, CSAMPLE_GAIN gain22in, CSAMPLE_GAIN gain22out,
                                         int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy22WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain2in == CSAMPLE_GAIN_ZERO && gain2out == CSAMPLE_GAIN_ZERO) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain3in == CSAMPLE_GAIN_ZERO && gain3out == CSAMPLE_GAIN_ZERO) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain4in == CSAMPLE_GAIN_ZERO && gain4out == CSAMPLE_GAIN_ZERO) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain5in == CSAMPLE_GAIN_ZERO && gain5out == CSAMPLE_GAIN_ZERO) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain6in == CSAMPLE_GAIN_ZERO && gain6out == CSAMPLE_GAIN_ZERO) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain7in == CSAMPLE_GAIN_ZERO && gain7out == CSAMPLE_GAIN_ZERO) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain8in == CSAMPLE_GAIN_ZERO && gain8out == CSAMPLE_GAIN_ZERO) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain9in == CSAMPLE_GAIN_ZERO && gain9out == CSAMPLE_GAIN_ZERO) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain10in == CSAMPLE_GAIN_ZERO && gain10out == CSAMPLE_GAIN_ZERO) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain11in == CSAMPLE_GAIN_ZERO && gain11out == CSAMPLE_GAIN_ZERO) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain12in == CSAMPLE_GAIN_ZERO && gain12out == CSAMPLE_GAIN_ZERO) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain13in == CSAMPLE_GAIN_ZERO && gain13out == CSAMPLE_GAIN_ZERO) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain14in == CSAMPLE_GAIN_ZERO && gain14out == CSAMPLE_GAIN_ZERO) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain15in == CSAMPLE_GAIN_ZERO && gain15out == CSAMPLE_GAIN_ZERO) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain16in == CSAMPLE_GAIN_ZERO && gain16out == CSAMPLE_GAIN_ZERO) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain17in == CSAMPLE_GAIN_ZERO && gain17out == CSAMPLE_GAIN_ZERO) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain18in == CSAMPLE_GAIN_ZERO && gain18out == CSAMPLE_GAIN_ZERO) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain19in == CSAMPLE_GAIN_ZERO && gain19out == CSAMPLE_GAIN_ZERO) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain20in == CSAMPLE_GAIN_ZERO && gain20out == CSAMPLE_GAIN_ZERO) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain21in == CSAMPLE_GAIN_ZERO && gain21out == CSAMPLE_GAIN_ZERO) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    if (gain22in == CSAMPLE_GAIN_ZERO && gain22out == CSAMPLE_GAIN_ZERO) {
        copy22WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    const CSAMPLE_GAIN gain_delta2 = (gain2out - gain2in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain2 = gain2in + gain_delta2;
    const CSAMPLE_GAIN gain_delta3 = (gain3out - gain3in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain3 = gain3in + gain_delta3;
    const CSAMPLE_GAIN gain_delta4 = (gain4out - gain4in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain4 = gain4in + gain_delta4;
    const CSAMPLE_GAIN gain_delta5 = (gain5out - gain5in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain5 = gain5in + gain_delta5;
    const CSAMPLE_GAIN gain_delta6 = (gain6out - gain6in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain6 = gain6in + gain_delta6;
    const CSAMPLE_GAIN gain_delta7 = (gain7out - gain7in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain7 = gain7in + gain_delta7;
    const CSAMPLE_GAIN gain_delta8 = (gain8out - gain8in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain8 = gain8in + gain_delta8;
    const CSAMPLE_GAIN gain_delta9 = (gain9out - gain9in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain9 = gain9in + gain_delta9;
    const CSAMPLE_GAIN gain_delta10 = (gain10out - gain10in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain10 = gain10in + gain_delta10;
    const CSAMPLE_GAIN gain_delta11 = (gain11out - gain11in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain11 = gain11in + gain_delta11;
    const CSAMPLE_GAIN gain_delta12 = (gain12out - gain12in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain12 = gain12in + gain_delta12;
    const CSAMPLE_GAIN gain_delta13 = (gain13out - gain13in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain13 = gain13in + gain_delta13;
    const CSAMPLE_GAIN gain_delta14 = (gain14out - gain14in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain14 = gain14in + gain_delta14;
    const CSAMPLE_GAIN gain_delta15 = (gain15out - gain15in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain15 = gain15in + gain_delta15;
    const CSAMPLE_GAIN gain_delta16 = (gain16out - gain16in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain16 = gain16in + gain_delta16;
    const CSAMPLE_GAIN gain_delta17 = (gain17out - gain17in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain17 = gain17in + gain_delta17;
    const CSAMPLE_GAIN gain_delta18 = (gain18out - gain18in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain18 = gain18in + gain_delta18;
    const CSAMPLE_GAIN gain_delta19 = (gain19out - gain19in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain19 = gain19in + gain_delta19;
    const CSAMPLE_GAIN gain_delta20 = (gain20out - gain20in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain20 = gain20in + gain_delta20;
    const CSAMPLE_GAIN gain_delta21 = (gain21out - gain21in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain21 = gain21in + gain_delta21;
    const CSAMPLE_GAIN gain_delta22 = (gain22out - gain22in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain22 = gain22in + gain_delta22;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        const CSAMPLE_GAIN gain2 = start_gain2 + gain_delta2 * i;
        const CSAMPLE_GAIN gain3 = start_gain3 + gain_delta3 * i;
        const CSAMPLE_GAIN gain4 = start_gain4 + gain_delta4 * i;
        const CSAMPLE_GAIN gain5 = start_gain5 + gain_delta5 * i;
        const CSAMPLE_GAIN gain6 = start_gain6 + gain_delta6 * i;
        const CSAMPLE_GAIN gain7 = start_gain7 + gain_delta7 * i;
        const CSAMPLE_GAIN gain8 = start_gain8 + gain_delta8 * i;
        const CSAMPLE_GAIN gain9 = start_gain9 + gain_delta9 * i;
        const CSAMPLE_GAIN gain10 = start_gain10 + gain_delta10 * i;
        const CSAMPLE_GAIN gain11 = start_gain11 + gain_delta11 * i;
        const CSAMPLE_GAIN gain12 = start_gain12 + gain_delta12 * i;
        const CSAMPLE_GAIN gain13 = start_gain13 + gain_delta13 * i;
        const CSAMPLE_GAIN gain14 = start_gain14 + gain_delta14 * i;
        const CSAMPLE_GAIN gain15 = start_gain15 + gain_delta15 * i;
        const CSAMPLE_GAIN gain16 = start_gain16 + gain_delta16 * i;
        const CSAMPLE_GAIN gain17 = start_gain17 + gain_delta17 * i;
        const CSAMPLE_GAIN gain18 = start_gain18 + gain_delta18 * i;
        const CSAMPLE_GAIN gain19 = start_gain19 + gain_delta19 * i;
        const CSAMPLE_GAIN gain20 = start_gain20 + gain_delta20 * i;
        const CSAMPLE_GAIN gain21 = start_gain21 + gain_delta21 * i;
        const CSAMPLE_GAIN gain22 = start_gain22 + gain_delta22 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1 +
                       pSrc2[i * 2] * gain2 +
                       pSrc3[i * 2] * gain3 +
                       pSrc4[i * 2] * gain4 +
                       pSrc5[i * 2] * gain5 +
                       pSrc6[i * 2] * gain6 +
                       pSrc7[i * 2] * gain7 +
                       pSrc8[i * 2] * gain8 +
                       pSrc9[i * 2] * gain9 +
                       pSrc10[i * 2] * gain10 +
                       pSrc11[i * 2] * gain11 +
                       pSrc12[i * 2] * gain12 +
                       pSrc13[i * 2] * gain13 +
                       pSrc14[i * 2] * gain14 +
                       pSrc15[i * 2] * gain15 +
                       pSrc16[i * 2] * gain16 +
                       pSrc17[i * 2] * gain17 +
                       pSrc18[i * 2] * gain18 +
                       pSrc19[i * 2] * gain19 +
                       pSrc20[i * 2] * gain20 +
                       pSrc21[i * 2] * gain21 +
                       pSrc22[i * 2] * gain22;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1 +
                           pSrc2[i * 2 + 1] * gain2 +
                           pSrc3[i * 2 + 1] * gain3 +
                           pSrc4[i * 2 + 1] * gain4 +
                           pSrc5[i * 2 + 1] * gain5 +
                           pSrc6[i * 2 + 1] * gain6 +
                           pSrc7[i * 2 + 1] * gain7 +
                           pSrc8[i * 2 + 1] * gain8 +
                           pSrc9[i * 2 + 1] * gain9 +
                           pSrc10[i * 2 + 1] * gain10 +
                           pSrc11[i * 2 + 1] * gain11 +
                           pSrc12[i * 2 + 1] * gain12 +
                           pSrc13[i * 2 + 1] * gain13 +
                           pSrc14[i * 2 + 1] * gain14 +
                           pSrc15[i * 2 + 1] * gain15 +
                           pSrc16[i * 2 + 1] * gain16 +
                           pSrc17[i * 2 + 1] * gain17 +
                           pSrc18[i * 2 + 1] * gain18 +
                           pSrc19[i * 2 + 1] * gain19 +
                           pSrc20[i * 2 + 1] * gain20 +
                           pSrc21[i * 2 + 1] * gain21 +
                           pSrc22[i * 2 + 1] * gain22;
    }
}
static inline void copy24WithGain(CSAMPLE* M_RESTRICT pDest,
                                  const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                  const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                  const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                  const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3,
                                  const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4,
                                  const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5,
                                  const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6,
                                  const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7,
                                  const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8,
                                  const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9,
                                  const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10,
                                  const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11,
                                  const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12,
                                  const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13,
                                  const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14,
                                  const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15,
                                  const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16,
                                  const CSAMPLE* M_RESTRICT pSrc17, CSAMPLE_GAIN gain17,
                                  const CSAMPLE* M_RESTRICT pSrc18, CSAMPLE_GAIN gain18,
                                  const CSAMPLE* M_RESTRICT pSrc19, CSAMPLE_GAIN gain19,
                                  const CSAMPLE* M_RESTRICT pSrc20, CSAMPLE_GAIN gain20,
                                  const CSAMPLE* M_RESTRICT pSrc21, CSAMPLE_GAIN gain21,
                                  const CSAMPLE* M_RESTRICT pSrc22, CSAMPLE_GAIN gain22,
                                  const CSAMPLE* M_RESTRICT pSrc23, CSAMPLE_GAIN gain23,
                                  int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy23WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain2 == CSAMPLE_GAIN_ZERO) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain3 == CSAMPLE_GAIN_ZERO) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain4 == CSAMPLE_GAIN_ZERO) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain5 == CSAMPLE_GAIN_ZERO) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain6 == CSAMPLE_GAIN_ZERO) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain7 == CSAMPLE_GAIN_ZERO) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain8 == CSAMPLE_GAIN_ZERO) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain9 == CSAMPLE_GAIN_ZERO) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain10 == CSAMPLE_GAIN_ZERO) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain11 == CSAMPLE_GAIN_ZERO) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain12 == CSAMPLE_GAIN_ZERO) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain13 == CSAMPLE_GAIN_ZERO) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain14 == CSAMPLE_GAIN_ZERO) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain15 == CSAMPLE_GAIN_ZERO) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain16 == CSAMPLE_GAIN_ZERO) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain17 == CSAMPLE_GAIN_ZERO) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain18 == CSAMPLE_GAIN_ZERO) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain19 == CSAMPLE_GAIN_ZERO) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain20 == CSAMPLE_GAIN_ZERO) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain21 == CSAMPLE_GAIN_ZERO) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain22 == CSAMPLE_GAIN_ZERO) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc23, gain23, iNumSamples);
        return;
    }
    if (gain23 == CSAMPLE_GAIN_ZERO) {
        copy23WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
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
static inline void copy24WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                         const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                         const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                         const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                         const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3in, CSAMPLE_GAIN gain3out,
                                         const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4in, CSAMPLE_GAIN gain4out,
                                         const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5in, CSAMPLE_GAIN gain5out,
                                         const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6in, CSAMPLE_GAIN gain6out,
                                         const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7in, CSAMPLE_GAIN gain7out,
                                         const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8in, CSAMPLE_GAIN gain8out,
                                         const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9in, CSAMPLE_GAIN gain9out,
                                         const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10in, CSAMPLE_GAIN gain10out,
                                         const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11in, CSAMPLE_GAIN gain11out,
                                         const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12in, CSAMPLE_GAIN gain12out,
                                         const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13in, CSAMPLE_GAIN gain13out,
                                         const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14in, CSAMPLE_GAIN gain14out,
                                         const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15in, CSAMPLE_GAIN gain15out,
                                         const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16in, CSAMPLE_GAIN gain16out,
                                         const CSAMPLE* M_RESTRICT pSrc17, CSAMPLE_GAIN gain17in, CSAMPLE_GAIN gain17out,
                                         const CSAMPLE* M_RESTRICT pSrc18, CSAMPLE_GAIN gain18in, CSAMPLE_GAIN gain18out,
                                         const CSAMPLE* M_RESTRICT pSrc19, CSAMPLE_GAIN gain19in, CSAMPLE_GAIN gain19out,
                                         const CSAMPLE* M_RESTRICT pSrc20, CSAMPLE_GAIN gain20in, CSAMPLE_GAIN gain20out,
                                         const CSAMPLE* M_RESTRICT pSrc21, CSAMPLE_GAIN gain21in, CSAMPLE_GAIN gain21out,
                                         const CSAMPLE* M_RESTRICT pSrc22, CSAMPLE_GAIN gain22in, CSAMPLE_GAIN gain22out,
                                         const CSAMPLE* M_RESTRICT pSrc23, CSAMPLE_GAIN gain23in, CSAMPLE_GAIN gain23out,
                                         int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy23WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain2in == CSAMPLE_GAIN_ZERO && gain2out == CSAMPLE_GAIN_ZERO) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain3in == CSAMPLE_GAIN_ZERO && gain3out == CSAMPLE_GAIN_ZERO) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain4in == CSAMPLE_GAIN_ZERO && gain4out == CSAMPLE_GAIN_ZERO) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain5in == CSAMPLE_GAIN_ZERO && gain5out == CSAMPLE_GAIN_ZERO) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain6in == CSAMPLE_GAIN_ZERO && gain6out == CSAMPLE_GAIN_ZERO) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain7in == CSAMPLE_GAIN_ZERO && gain7out == CSAMPLE_GAIN_ZERO) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain8in == CSAMPLE_GAIN_ZERO && gain8out == CSAMPLE_GAIN_ZERO) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain9in == CSAMPLE_GAIN_ZERO && gain9out == CSAMPLE_GAIN_ZERO) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain10in == CSAMPLE_GAIN_ZERO && gain10out == CSAMPLE_GAIN_ZERO) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain11in == CSAMPLE_GAIN_ZERO && gain11out == CSAMPLE_GAIN_ZERO) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain12in == CSAMPLE_GAIN_ZERO && gain12out == CSAMPLE_GAIN_ZERO) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain13in == CSAMPLE_GAIN_ZERO && gain13out == CSAMPLE_GAIN_ZERO) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain14in == CSAMPLE_GAIN_ZERO && gain14out == CSAMPLE_GAIN_ZERO) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain15in == CSAMPLE_GAIN_ZERO && gain15out == CSAMPLE_GAIN_ZERO) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain16in == CSAMPLE_GAIN_ZERO && gain16out == CSAMPLE_GAIN_ZERO) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain17in == CSAMPLE_GAIN_ZERO && gain17out == CSAMPLE_GAIN_ZERO) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain18in == CSAMPLE_GAIN_ZERO && gain18out == CSAMPLE_GAIN_ZERO) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain19in == CSAMPLE_GAIN_ZERO && gain19out == CSAMPLE_GAIN_ZERO) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain20in == CSAMPLE_GAIN_ZERO && gain20out == CSAMPLE_GAIN_ZERO) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain21in == CSAMPLE_GAIN_ZERO && gain21out == CSAMPLE_GAIN_ZERO) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain22in == CSAMPLE_GAIN_ZERO && gain22out == CSAMPLE_GAIN_ZERO) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    if (gain23in == CSAMPLE_GAIN_ZERO && gain23out == CSAMPLE_GAIN_ZERO) {
        copy23WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    const CSAMPLE_GAIN gain_delta2 = (gain2out - gain2in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain2 = gain2in + gain_delta2;
    const CSAMPLE_GAIN gain_delta3 = (gain3out - gain3in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain3 = gain3in + gain_delta3;
    const CSAMPLE_GAIN gain_delta4 = (gain4out - gain4in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain4 = gain4in + gain_delta4;
    const CSAMPLE_GAIN gain_delta5 = (gain5out - gain5in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain5 = gain5in + gain_delta5;
    const CSAMPLE_GAIN gain_delta6 = (gain6out - gain6in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain6 = gain6in + gain_delta6;
    const CSAMPLE_GAIN gain_delta7 = (gain7out - gain7in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain7 = gain7in + gain_delta7;
    const CSAMPLE_GAIN gain_delta8 = (gain8out - gain8in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain8 = gain8in + gain_delta8;
    const CSAMPLE_GAIN gain_delta9 = (gain9out - gain9in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain9 = gain9in + gain_delta9;
    const CSAMPLE_GAIN gain_delta10 = (gain10out - gain10in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain10 = gain10in + gain_delta10;
    const CSAMPLE_GAIN gain_delta11 = (gain11out - gain11in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain11 = gain11in + gain_delta11;
    const CSAMPLE_GAIN gain_delta12 = (gain12out - gain12in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain12 = gain12in + gain_delta12;
    const CSAMPLE_GAIN gain_delta13 = (gain13out - gain13in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain13 = gain13in + gain_delta13;
    const CSAMPLE_GAIN gain_delta14 = (gain14out - gain14in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain14 = gain14in + gain_delta14;
    const CSAMPLE_GAIN gain_delta15 = (gain15out - gain15in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain15 = gain15in + gain_delta15;
    const CSAMPLE_GAIN gain_delta16 = (gain16out - gain16in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain16 = gain16in + gain_delta16;
    const CSAMPLE_GAIN gain_delta17 = (gain17out - gain17in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain17 = gain17in + gain_delta17;
    const CSAMPLE_GAIN gain_delta18 = (gain18out - gain18in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain18 = gain18in + gain_delta18;
    const CSAMPLE_GAIN gain_delta19 = (gain19out - gain19in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain19 = gain19in + gain_delta19;
    const CSAMPLE_GAIN gain_delta20 = (gain20out - gain20in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain20 = gain20in + gain_delta20;
    const CSAMPLE_GAIN gain_delta21 = (gain21out - gain21in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain21 = gain21in + gain_delta21;
    const CSAMPLE_GAIN gain_delta22 = (gain22out - gain22in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain22 = gain22in + gain_delta22;
    const CSAMPLE_GAIN gain_delta23 = (gain23out - gain23in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain23 = gain23in + gain_delta23;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        const CSAMPLE_GAIN gain2 = start_gain2 + gain_delta2 * i;
        const CSAMPLE_GAIN gain3 = start_gain3 + gain_delta3 * i;
        const CSAMPLE_GAIN gain4 = start_gain4 + gain_delta4 * i;
        const CSAMPLE_GAIN gain5 = start_gain5 + gain_delta5 * i;
        const CSAMPLE_GAIN gain6 = start_gain6 + gain_delta6 * i;
        const CSAMPLE_GAIN gain7 = start_gain7 + gain_delta7 * i;
        const CSAMPLE_GAIN gain8 = start_gain8 + gain_delta8 * i;
        const CSAMPLE_GAIN gain9 = start_gain9 + gain_delta9 * i;
        const CSAMPLE_GAIN gain10 = start_gain10 + gain_delta10 * i;
        const CSAMPLE_GAIN gain11 = start_gain11 + gain_delta11 * i;
        const CSAMPLE_GAIN gain12 = start_gain12 + gain_delta12 * i;
        const CSAMPLE_GAIN gain13 = start_gain13 + gain_delta13 * i;
        const CSAMPLE_GAIN gain14 = start_gain14 + gain_delta14 * i;
        const CSAMPLE_GAIN gain15 = start_gain15 + gain_delta15 * i;
        const CSAMPLE_GAIN gain16 = start_gain16 + gain_delta16 * i;
        const CSAMPLE_GAIN gain17 = start_gain17 + gain_delta17 * i;
        const CSAMPLE_GAIN gain18 = start_gain18 + gain_delta18 * i;
        const CSAMPLE_GAIN gain19 = start_gain19 + gain_delta19 * i;
        const CSAMPLE_GAIN gain20 = start_gain20 + gain_delta20 * i;
        const CSAMPLE_GAIN gain21 = start_gain21 + gain_delta21 * i;
        const CSAMPLE_GAIN gain22 = start_gain22 + gain_delta22 * i;
        const CSAMPLE_GAIN gain23 = start_gain23 + gain_delta23 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1 +
                       pSrc2[i * 2] * gain2 +
                       pSrc3[i * 2] * gain3 +
                       pSrc4[i * 2] * gain4 +
                       pSrc5[i * 2] * gain5 +
                       pSrc6[i * 2] * gain6 +
                       pSrc7[i * 2] * gain7 +
                       pSrc8[i * 2] * gain8 +
                       pSrc9[i * 2] * gain9 +
                       pSrc10[i * 2] * gain10 +
                       pSrc11[i * 2] * gain11 +
                       pSrc12[i * 2] * gain12 +
                       pSrc13[i * 2] * gain13 +
                       pSrc14[i * 2] * gain14 +
                       pSrc15[i * 2] * gain15 +
                       pSrc16[i * 2] * gain16 +
                       pSrc17[i * 2] * gain17 +
                       pSrc18[i * 2] * gain18 +
                       pSrc19[i * 2] * gain19 +
                       pSrc20[i * 2] * gain20 +
                       pSrc21[i * 2] * gain21 +
                       pSrc22[i * 2] * gain22 +
                       pSrc23[i * 2] * gain23;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1 +
                           pSrc2[i * 2 + 1] * gain2 +
                           pSrc3[i * 2 + 1] * gain3 +
                           pSrc4[i * 2 + 1] * gain4 +
                           pSrc5[i * 2 + 1] * gain5 +
                           pSrc6[i * 2 + 1] * gain6 +
                           pSrc7[i * 2 + 1] * gain7 +
                           pSrc8[i * 2 + 1] * gain8 +
                           pSrc9[i * 2 + 1] * gain9 +
                           pSrc10[i * 2 + 1] * gain10 +
                           pSrc11[i * 2 + 1] * gain11 +
                           pSrc12[i * 2 + 1] * gain12 +
                           pSrc13[i * 2 + 1] * gain13 +
                           pSrc14[i * 2 + 1] * gain14 +
                           pSrc15[i * 2 + 1] * gain15 +
                           pSrc16[i * 2 + 1] * gain16 +
                           pSrc17[i * 2 + 1] * gain17 +
                           pSrc18[i * 2 + 1] * gain18 +
                           pSrc19[i * 2 + 1] * gain19 +
                           pSrc20[i * 2 + 1] * gain20 +
                           pSrc21[i * 2 + 1] * gain21 +
                           pSrc22[i * 2 + 1] * gain22 +
                           pSrc23[i * 2 + 1] * gain23;
    }
}
static inline void copy25WithGain(CSAMPLE* M_RESTRICT pDest,
                                  const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                  const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                  const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                  const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3,
                                  const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4,
                                  const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5,
                                  const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6,
                                  const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7,
                                  const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8,
                                  const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9,
                                  const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10,
                                  const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11,
                                  const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12,
                                  const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13,
                                  const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14,
                                  const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15,
                                  const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16,
                                  const CSAMPLE* M_RESTRICT pSrc17, CSAMPLE_GAIN gain17,
                                  const CSAMPLE* M_RESTRICT pSrc18, CSAMPLE_GAIN gain18,
                                  const CSAMPLE* M_RESTRICT pSrc19, CSAMPLE_GAIN gain19,
                                  const CSAMPLE* M_RESTRICT pSrc20, CSAMPLE_GAIN gain20,
                                  const CSAMPLE* M_RESTRICT pSrc21, CSAMPLE_GAIN gain21,
                                  const CSAMPLE* M_RESTRICT pSrc22, CSAMPLE_GAIN gain22,
                                  const CSAMPLE* M_RESTRICT pSrc23, CSAMPLE_GAIN gain23,
                                  const CSAMPLE* M_RESTRICT pSrc24, CSAMPLE_GAIN gain24,
                                  int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy24WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain2 == CSAMPLE_GAIN_ZERO) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain3 == CSAMPLE_GAIN_ZERO) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain4 == CSAMPLE_GAIN_ZERO) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain5 == CSAMPLE_GAIN_ZERO) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain6 == CSAMPLE_GAIN_ZERO) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain7 == CSAMPLE_GAIN_ZERO) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain8 == CSAMPLE_GAIN_ZERO) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain9 == CSAMPLE_GAIN_ZERO) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain10 == CSAMPLE_GAIN_ZERO) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain11 == CSAMPLE_GAIN_ZERO) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain12 == CSAMPLE_GAIN_ZERO) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain13 == CSAMPLE_GAIN_ZERO) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain14 == CSAMPLE_GAIN_ZERO) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain15 == CSAMPLE_GAIN_ZERO) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain16 == CSAMPLE_GAIN_ZERO) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain17 == CSAMPLE_GAIN_ZERO) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain18 == CSAMPLE_GAIN_ZERO) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain19 == CSAMPLE_GAIN_ZERO) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain20 == CSAMPLE_GAIN_ZERO) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain21 == CSAMPLE_GAIN_ZERO) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain22 == CSAMPLE_GAIN_ZERO) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain23 == CSAMPLE_GAIN_ZERO) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc24, gain24, iNumSamples);
        return;
    }
    if (gain24 == CSAMPLE_GAIN_ZERO) {
        copy24WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
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
static inline void copy25WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                         const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                         const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                         const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                         const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3in, CSAMPLE_GAIN gain3out,
                                         const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4in, CSAMPLE_GAIN gain4out,
                                         const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5in, CSAMPLE_GAIN gain5out,
                                         const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6in, CSAMPLE_GAIN gain6out,
                                         const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7in, CSAMPLE_GAIN gain7out,
                                         const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8in, CSAMPLE_GAIN gain8out,
                                         const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9in, CSAMPLE_GAIN gain9out,
                                         const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10in, CSAMPLE_GAIN gain10out,
                                         const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11in, CSAMPLE_GAIN gain11out,
                                         const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12in, CSAMPLE_GAIN gain12out,
                                         const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13in, CSAMPLE_GAIN gain13out,
                                         const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14in, CSAMPLE_GAIN gain14out,
                                         const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15in, CSAMPLE_GAIN gain15out,
                                         const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16in, CSAMPLE_GAIN gain16out,
                                         const CSAMPLE* M_RESTRICT pSrc17, CSAMPLE_GAIN gain17in, CSAMPLE_GAIN gain17out,
                                         const CSAMPLE* M_RESTRICT pSrc18, CSAMPLE_GAIN gain18in, CSAMPLE_GAIN gain18out,
                                         const CSAMPLE* M_RESTRICT pSrc19, CSAMPLE_GAIN gain19in, CSAMPLE_GAIN gain19out,
                                         const CSAMPLE* M_RESTRICT pSrc20, CSAMPLE_GAIN gain20in, CSAMPLE_GAIN gain20out,
                                         const CSAMPLE* M_RESTRICT pSrc21, CSAMPLE_GAIN gain21in, CSAMPLE_GAIN gain21out,
                                         const CSAMPLE* M_RESTRICT pSrc22, CSAMPLE_GAIN gain22in, CSAMPLE_GAIN gain22out,
                                         const CSAMPLE* M_RESTRICT pSrc23, CSAMPLE_GAIN gain23in, CSAMPLE_GAIN gain23out,
                                         const CSAMPLE* M_RESTRICT pSrc24, CSAMPLE_GAIN gain24in, CSAMPLE_GAIN gain24out,
                                         int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy24WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain2in == CSAMPLE_GAIN_ZERO && gain2out == CSAMPLE_GAIN_ZERO) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain3in == CSAMPLE_GAIN_ZERO && gain3out == CSAMPLE_GAIN_ZERO) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain4in == CSAMPLE_GAIN_ZERO && gain4out == CSAMPLE_GAIN_ZERO) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain5in == CSAMPLE_GAIN_ZERO && gain5out == CSAMPLE_GAIN_ZERO) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain6in == CSAMPLE_GAIN_ZERO && gain6out == CSAMPLE_GAIN_ZERO) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain7in == CSAMPLE_GAIN_ZERO && gain7out == CSAMPLE_GAIN_ZERO) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain8in == CSAMPLE_GAIN_ZERO && gain8out == CSAMPLE_GAIN_ZERO) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain9in == CSAMPLE_GAIN_ZERO && gain9out == CSAMPLE_GAIN_ZERO) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain10in == CSAMPLE_GAIN_ZERO && gain10out == CSAMPLE_GAIN_ZERO) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain11in == CSAMPLE_GAIN_ZERO && gain11out == CSAMPLE_GAIN_ZERO) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain12in == CSAMPLE_GAIN_ZERO && gain12out == CSAMPLE_GAIN_ZERO) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain13in == CSAMPLE_GAIN_ZERO && gain13out == CSAMPLE_GAIN_ZERO) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain14in == CSAMPLE_GAIN_ZERO && gain14out == CSAMPLE_GAIN_ZERO) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain15in == CSAMPLE_GAIN_ZERO && gain15out == CSAMPLE_GAIN_ZERO) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain16in == CSAMPLE_GAIN_ZERO && gain16out == CSAMPLE_GAIN_ZERO) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain17in == CSAMPLE_GAIN_ZERO && gain17out == CSAMPLE_GAIN_ZERO) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain18in == CSAMPLE_GAIN_ZERO && gain18out == CSAMPLE_GAIN_ZERO) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain19in == CSAMPLE_GAIN_ZERO && gain19out == CSAMPLE_GAIN_ZERO) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain20in == CSAMPLE_GAIN_ZERO && gain20out == CSAMPLE_GAIN_ZERO) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain21in == CSAMPLE_GAIN_ZERO && gain21out == CSAMPLE_GAIN_ZERO) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain22in == CSAMPLE_GAIN_ZERO && gain22out == CSAMPLE_GAIN_ZERO) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain23in == CSAMPLE_GAIN_ZERO && gain23out == CSAMPLE_GAIN_ZERO) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    if (gain24in == CSAMPLE_GAIN_ZERO && gain24out == CSAMPLE_GAIN_ZERO) {
        copy24WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    const CSAMPLE_GAIN gain_delta2 = (gain2out - gain2in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain2 = gain2in + gain_delta2;
    const CSAMPLE_GAIN gain_delta3 = (gain3out - gain3in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain3 = gain3in + gain_delta3;
    const CSAMPLE_GAIN gain_delta4 = (gain4out - gain4in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain4 = gain4in + gain_delta4;
    const CSAMPLE_GAIN gain_delta5 = (gain5out - gain5in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain5 = gain5in + gain_delta5;
    const CSAMPLE_GAIN gain_delta6 = (gain6out - gain6in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain6 = gain6in + gain_delta6;
    const CSAMPLE_GAIN gain_delta7 = (gain7out - gain7in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain7 = gain7in + gain_delta7;
    const CSAMPLE_GAIN gain_delta8 = (gain8out - gain8in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain8 = gain8in + gain_delta8;
    const CSAMPLE_GAIN gain_delta9 = (gain9out - gain9in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain9 = gain9in + gain_delta9;
    const CSAMPLE_GAIN gain_delta10 = (gain10out - gain10in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain10 = gain10in + gain_delta10;
    const CSAMPLE_GAIN gain_delta11 = (gain11out - gain11in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain11 = gain11in + gain_delta11;
    const CSAMPLE_GAIN gain_delta12 = (gain12out - gain12in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain12 = gain12in + gain_delta12;
    const CSAMPLE_GAIN gain_delta13 = (gain13out - gain13in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain13 = gain13in + gain_delta13;
    const CSAMPLE_GAIN gain_delta14 = (gain14out - gain14in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain14 = gain14in + gain_delta14;
    const CSAMPLE_GAIN gain_delta15 = (gain15out - gain15in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain15 = gain15in + gain_delta15;
    const CSAMPLE_GAIN gain_delta16 = (gain16out - gain16in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain16 = gain16in + gain_delta16;
    const CSAMPLE_GAIN gain_delta17 = (gain17out - gain17in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain17 = gain17in + gain_delta17;
    const CSAMPLE_GAIN gain_delta18 = (gain18out - gain18in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain18 = gain18in + gain_delta18;
    const CSAMPLE_GAIN gain_delta19 = (gain19out - gain19in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain19 = gain19in + gain_delta19;
    const CSAMPLE_GAIN gain_delta20 = (gain20out - gain20in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain20 = gain20in + gain_delta20;
    const CSAMPLE_GAIN gain_delta21 = (gain21out - gain21in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain21 = gain21in + gain_delta21;
    const CSAMPLE_GAIN gain_delta22 = (gain22out - gain22in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain22 = gain22in + gain_delta22;
    const CSAMPLE_GAIN gain_delta23 = (gain23out - gain23in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain23 = gain23in + gain_delta23;
    const CSAMPLE_GAIN gain_delta24 = (gain24out - gain24in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain24 = gain24in + gain_delta24;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        const CSAMPLE_GAIN gain2 = start_gain2 + gain_delta2 * i;
        const CSAMPLE_GAIN gain3 = start_gain3 + gain_delta3 * i;
        const CSAMPLE_GAIN gain4 = start_gain4 + gain_delta4 * i;
        const CSAMPLE_GAIN gain5 = start_gain5 + gain_delta5 * i;
        const CSAMPLE_GAIN gain6 = start_gain6 + gain_delta6 * i;
        const CSAMPLE_GAIN gain7 = start_gain7 + gain_delta7 * i;
        const CSAMPLE_GAIN gain8 = start_gain8 + gain_delta8 * i;
        const CSAMPLE_GAIN gain9 = start_gain9 + gain_delta9 * i;
        const CSAMPLE_GAIN gain10 = start_gain10 + gain_delta10 * i;
        const CSAMPLE_GAIN gain11 = start_gain11 + gain_delta11 * i;
        const CSAMPLE_GAIN gain12 = start_gain12 + gain_delta12 * i;
        const CSAMPLE_GAIN gain13 = start_gain13 + gain_delta13 * i;
        const CSAMPLE_GAIN gain14 = start_gain14 + gain_delta14 * i;
        const CSAMPLE_GAIN gain15 = start_gain15 + gain_delta15 * i;
        const CSAMPLE_GAIN gain16 = start_gain16 + gain_delta16 * i;
        const CSAMPLE_GAIN gain17 = start_gain17 + gain_delta17 * i;
        const CSAMPLE_GAIN gain18 = start_gain18 + gain_delta18 * i;
        const CSAMPLE_GAIN gain19 = start_gain19 + gain_delta19 * i;
        const CSAMPLE_GAIN gain20 = start_gain20 + gain_delta20 * i;
        const CSAMPLE_GAIN gain21 = start_gain21 + gain_delta21 * i;
        const CSAMPLE_GAIN gain22 = start_gain22 + gain_delta22 * i;
        const CSAMPLE_GAIN gain23 = start_gain23 + gain_delta23 * i;
        const CSAMPLE_GAIN gain24 = start_gain24 + gain_delta24 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1 +
                       pSrc2[i * 2] * gain2 +
                       pSrc3[i * 2] * gain3 +
                       pSrc4[i * 2] * gain4 +
                       pSrc5[i * 2] * gain5 +
                       pSrc6[i * 2] * gain6 +
                       pSrc7[i * 2] * gain7 +
                       pSrc8[i * 2] * gain8 +
                       pSrc9[i * 2] * gain9 +
                       pSrc10[i * 2] * gain10 +
                       pSrc11[i * 2] * gain11 +
                       pSrc12[i * 2] * gain12 +
                       pSrc13[i * 2] * gain13 +
                       pSrc14[i * 2] * gain14 +
                       pSrc15[i * 2] * gain15 +
                       pSrc16[i * 2] * gain16 +
                       pSrc17[i * 2] * gain17 +
                       pSrc18[i * 2] * gain18 +
                       pSrc19[i * 2] * gain19 +
                       pSrc20[i * 2] * gain20 +
                       pSrc21[i * 2] * gain21 +
                       pSrc22[i * 2] * gain22 +
                       pSrc23[i * 2] * gain23 +
                       pSrc24[i * 2] * gain24;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1 +
                           pSrc2[i * 2 + 1] * gain2 +
                           pSrc3[i * 2 + 1] * gain3 +
                           pSrc4[i * 2 + 1] * gain4 +
                           pSrc5[i * 2 + 1] * gain5 +
                           pSrc6[i * 2 + 1] * gain6 +
                           pSrc7[i * 2 + 1] * gain7 +
                           pSrc8[i * 2 + 1] * gain8 +
                           pSrc9[i * 2 + 1] * gain9 +
                           pSrc10[i * 2 + 1] * gain10 +
                           pSrc11[i * 2 + 1] * gain11 +
                           pSrc12[i * 2 + 1] * gain12 +
                           pSrc13[i * 2 + 1] * gain13 +
                           pSrc14[i * 2 + 1] * gain14 +
                           pSrc15[i * 2 + 1] * gain15 +
                           pSrc16[i * 2 + 1] * gain16 +
                           pSrc17[i * 2 + 1] * gain17 +
                           pSrc18[i * 2 + 1] * gain18 +
                           pSrc19[i * 2 + 1] * gain19 +
                           pSrc20[i * 2 + 1] * gain20 +
                           pSrc21[i * 2 + 1] * gain21 +
                           pSrc22[i * 2 + 1] * gain22 +
                           pSrc23[i * 2 + 1] * gain23 +
                           pSrc24[i * 2 + 1] * gain24;
    }
}
static inline void copy26WithGain(CSAMPLE* M_RESTRICT pDest,
                                  const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                  const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                  const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                  const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3,
                                  const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4,
                                  const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5,
                                  const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6,
                                  const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7,
                                  const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8,
                                  const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9,
                                  const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10,
                                  const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11,
                                  const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12,
                                  const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13,
                                  const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14,
                                  const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15,
                                  const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16,
                                  const CSAMPLE* M_RESTRICT pSrc17, CSAMPLE_GAIN gain17,
                                  const CSAMPLE* M_RESTRICT pSrc18, CSAMPLE_GAIN gain18,
                                  const CSAMPLE* M_RESTRICT pSrc19, CSAMPLE_GAIN gain19,
                                  const CSAMPLE* M_RESTRICT pSrc20, CSAMPLE_GAIN gain20,
                                  const CSAMPLE* M_RESTRICT pSrc21, CSAMPLE_GAIN gain21,
                                  const CSAMPLE* M_RESTRICT pSrc22, CSAMPLE_GAIN gain22,
                                  const CSAMPLE* M_RESTRICT pSrc23, CSAMPLE_GAIN gain23,
                                  const CSAMPLE* M_RESTRICT pSrc24, CSAMPLE_GAIN gain24,
                                  const CSAMPLE* M_RESTRICT pSrc25, CSAMPLE_GAIN gain25,
                                  int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy25WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain2 == CSAMPLE_GAIN_ZERO) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain3 == CSAMPLE_GAIN_ZERO) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain4 == CSAMPLE_GAIN_ZERO) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain5 == CSAMPLE_GAIN_ZERO) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain6 == CSAMPLE_GAIN_ZERO) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain7 == CSAMPLE_GAIN_ZERO) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain8 == CSAMPLE_GAIN_ZERO) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain9 == CSAMPLE_GAIN_ZERO) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain10 == CSAMPLE_GAIN_ZERO) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain11 == CSAMPLE_GAIN_ZERO) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain12 == CSAMPLE_GAIN_ZERO) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain13 == CSAMPLE_GAIN_ZERO) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain14 == CSAMPLE_GAIN_ZERO) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain15 == CSAMPLE_GAIN_ZERO) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain16 == CSAMPLE_GAIN_ZERO) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain17 == CSAMPLE_GAIN_ZERO) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain18 == CSAMPLE_GAIN_ZERO) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain19 == CSAMPLE_GAIN_ZERO) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain20 == CSAMPLE_GAIN_ZERO) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain21 == CSAMPLE_GAIN_ZERO) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain22 == CSAMPLE_GAIN_ZERO) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain23 == CSAMPLE_GAIN_ZERO) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain24 == CSAMPLE_GAIN_ZERO) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc25, gain25, iNumSamples);
        return;
    }
    if (gain25 == CSAMPLE_GAIN_ZERO) {
        copy25WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
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
static inline void copy26WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                         const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                         const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                         const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                         const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3in, CSAMPLE_GAIN gain3out,
                                         const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4in, CSAMPLE_GAIN gain4out,
                                         const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5in, CSAMPLE_GAIN gain5out,
                                         const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6in, CSAMPLE_GAIN gain6out,
                                         const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7in, CSAMPLE_GAIN gain7out,
                                         const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8in, CSAMPLE_GAIN gain8out,
                                         const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9in, CSAMPLE_GAIN gain9out,
                                         const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10in, CSAMPLE_GAIN gain10out,
                                         const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11in, CSAMPLE_GAIN gain11out,
                                         const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12in, CSAMPLE_GAIN gain12out,
                                         const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13in, CSAMPLE_GAIN gain13out,
                                         const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14in, CSAMPLE_GAIN gain14out,
                                         const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15in, CSAMPLE_GAIN gain15out,
                                         const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16in, CSAMPLE_GAIN gain16out,
                                         const CSAMPLE* M_RESTRICT pSrc17, CSAMPLE_GAIN gain17in, CSAMPLE_GAIN gain17out,
                                         const CSAMPLE* M_RESTRICT pSrc18, CSAMPLE_GAIN gain18in, CSAMPLE_GAIN gain18out,
                                         const CSAMPLE* M_RESTRICT pSrc19, CSAMPLE_GAIN gain19in, CSAMPLE_GAIN gain19out,
                                         const CSAMPLE* M_RESTRICT pSrc20, CSAMPLE_GAIN gain20in, CSAMPLE_GAIN gain20out,
                                         const CSAMPLE* M_RESTRICT pSrc21, CSAMPLE_GAIN gain21in, CSAMPLE_GAIN gain21out,
                                         const CSAMPLE* M_RESTRICT pSrc22, CSAMPLE_GAIN gain22in, CSAMPLE_GAIN gain22out,
                                         const CSAMPLE* M_RESTRICT pSrc23, CSAMPLE_GAIN gain23in, CSAMPLE_GAIN gain23out,
                                         const CSAMPLE* M_RESTRICT pSrc24, CSAMPLE_GAIN gain24in, CSAMPLE_GAIN gain24out,
                                         const CSAMPLE* M_RESTRICT pSrc25, CSAMPLE_GAIN gain25in, CSAMPLE_GAIN gain25out,
                                         int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy25WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain2in == CSAMPLE_GAIN_ZERO && gain2out == CSAMPLE_GAIN_ZERO) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain3in == CSAMPLE_GAIN_ZERO && gain3out == CSAMPLE_GAIN_ZERO) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain4in == CSAMPLE_GAIN_ZERO && gain4out == CSAMPLE_GAIN_ZERO) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain5in == CSAMPLE_GAIN_ZERO && gain5out == CSAMPLE_GAIN_ZERO) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain6in == CSAMPLE_GAIN_ZERO && gain6out == CSAMPLE_GAIN_ZERO) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain7in == CSAMPLE_GAIN_ZERO && gain7out == CSAMPLE_GAIN_ZERO) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain8in == CSAMPLE_GAIN_ZERO && gain8out == CSAMPLE_GAIN_ZERO) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain9in == CSAMPLE_GAIN_ZERO && gain9out == CSAMPLE_GAIN_ZERO) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain10in == CSAMPLE_GAIN_ZERO && gain10out == CSAMPLE_GAIN_ZERO) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain11in == CSAMPLE_GAIN_ZERO && gain11out == CSAMPLE_GAIN_ZERO) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain12in == CSAMPLE_GAIN_ZERO && gain12out == CSAMPLE_GAIN_ZERO) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain13in == CSAMPLE_GAIN_ZERO && gain13out == CSAMPLE_GAIN_ZERO) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain14in == CSAMPLE_GAIN_ZERO && gain14out == CSAMPLE_GAIN_ZERO) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain15in == CSAMPLE_GAIN_ZERO && gain15out == CSAMPLE_GAIN_ZERO) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain16in == CSAMPLE_GAIN_ZERO && gain16out == CSAMPLE_GAIN_ZERO) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain17in == CSAMPLE_GAIN_ZERO && gain17out == CSAMPLE_GAIN_ZERO) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain18in == CSAMPLE_GAIN_ZERO && gain18out == CSAMPLE_GAIN_ZERO) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain19in == CSAMPLE_GAIN_ZERO && gain19out == CSAMPLE_GAIN_ZERO) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain20in == CSAMPLE_GAIN_ZERO && gain20out == CSAMPLE_GAIN_ZERO) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain21in == CSAMPLE_GAIN_ZERO && gain21out == CSAMPLE_GAIN_ZERO) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain22in == CSAMPLE_GAIN_ZERO && gain22out == CSAMPLE_GAIN_ZERO) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain23in == CSAMPLE_GAIN_ZERO && gain23out == CSAMPLE_GAIN_ZERO) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain24in == CSAMPLE_GAIN_ZERO && gain24out == CSAMPLE_GAIN_ZERO) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    if (gain25in == CSAMPLE_GAIN_ZERO && gain25out == CSAMPLE_GAIN_ZERO) {
        copy25WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    const CSAMPLE_GAIN gain_delta2 = (gain2out - gain2in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain2 = gain2in + gain_delta2;
    const CSAMPLE_GAIN gain_delta3 = (gain3out - gain3in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain3 = gain3in + gain_delta3;
    const CSAMPLE_GAIN gain_delta4 = (gain4out - gain4in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain4 = gain4in + gain_delta4;
    const CSAMPLE_GAIN gain_delta5 = (gain5out - gain5in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain5 = gain5in + gain_delta5;
    const CSAMPLE_GAIN gain_delta6 = (gain6out - gain6in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain6 = gain6in + gain_delta6;
    const CSAMPLE_GAIN gain_delta7 = (gain7out - gain7in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain7 = gain7in + gain_delta7;
    const CSAMPLE_GAIN gain_delta8 = (gain8out - gain8in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain8 = gain8in + gain_delta8;
    const CSAMPLE_GAIN gain_delta9 = (gain9out - gain9in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain9 = gain9in + gain_delta9;
    const CSAMPLE_GAIN gain_delta10 = (gain10out - gain10in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain10 = gain10in + gain_delta10;
    const CSAMPLE_GAIN gain_delta11 = (gain11out - gain11in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain11 = gain11in + gain_delta11;
    const CSAMPLE_GAIN gain_delta12 = (gain12out - gain12in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain12 = gain12in + gain_delta12;
    const CSAMPLE_GAIN gain_delta13 = (gain13out - gain13in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain13 = gain13in + gain_delta13;
    const CSAMPLE_GAIN gain_delta14 = (gain14out - gain14in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain14 = gain14in + gain_delta14;
    const CSAMPLE_GAIN gain_delta15 = (gain15out - gain15in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain15 = gain15in + gain_delta15;
    const CSAMPLE_GAIN gain_delta16 = (gain16out - gain16in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain16 = gain16in + gain_delta16;
    const CSAMPLE_GAIN gain_delta17 = (gain17out - gain17in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain17 = gain17in + gain_delta17;
    const CSAMPLE_GAIN gain_delta18 = (gain18out - gain18in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain18 = gain18in + gain_delta18;
    const CSAMPLE_GAIN gain_delta19 = (gain19out - gain19in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain19 = gain19in + gain_delta19;
    const CSAMPLE_GAIN gain_delta20 = (gain20out - gain20in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain20 = gain20in + gain_delta20;
    const CSAMPLE_GAIN gain_delta21 = (gain21out - gain21in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain21 = gain21in + gain_delta21;
    const CSAMPLE_GAIN gain_delta22 = (gain22out - gain22in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain22 = gain22in + gain_delta22;
    const CSAMPLE_GAIN gain_delta23 = (gain23out - gain23in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain23 = gain23in + gain_delta23;
    const CSAMPLE_GAIN gain_delta24 = (gain24out - gain24in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain24 = gain24in + gain_delta24;
    const CSAMPLE_GAIN gain_delta25 = (gain25out - gain25in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain25 = gain25in + gain_delta25;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        const CSAMPLE_GAIN gain2 = start_gain2 + gain_delta2 * i;
        const CSAMPLE_GAIN gain3 = start_gain3 + gain_delta3 * i;
        const CSAMPLE_GAIN gain4 = start_gain4 + gain_delta4 * i;
        const CSAMPLE_GAIN gain5 = start_gain5 + gain_delta5 * i;
        const CSAMPLE_GAIN gain6 = start_gain6 + gain_delta6 * i;
        const CSAMPLE_GAIN gain7 = start_gain7 + gain_delta7 * i;
        const CSAMPLE_GAIN gain8 = start_gain8 + gain_delta8 * i;
        const CSAMPLE_GAIN gain9 = start_gain9 + gain_delta9 * i;
        const CSAMPLE_GAIN gain10 = start_gain10 + gain_delta10 * i;
        const CSAMPLE_GAIN gain11 = start_gain11 + gain_delta11 * i;
        const CSAMPLE_GAIN gain12 = start_gain12 + gain_delta12 * i;
        const CSAMPLE_GAIN gain13 = start_gain13 + gain_delta13 * i;
        const CSAMPLE_GAIN gain14 = start_gain14 + gain_delta14 * i;
        const CSAMPLE_GAIN gain15 = start_gain15 + gain_delta15 * i;
        const CSAMPLE_GAIN gain16 = start_gain16 + gain_delta16 * i;
        const CSAMPLE_GAIN gain17 = start_gain17 + gain_delta17 * i;
        const CSAMPLE_GAIN gain18 = start_gain18 + gain_delta18 * i;
        const CSAMPLE_GAIN gain19 = start_gain19 + gain_delta19 * i;
        const CSAMPLE_GAIN gain20 = start_gain20 + gain_delta20 * i;
        const CSAMPLE_GAIN gain21 = start_gain21 + gain_delta21 * i;
        const CSAMPLE_GAIN gain22 = start_gain22 + gain_delta22 * i;
        const CSAMPLE_GAIN gain23 = start_gain23 + gain_delta23 * i;
        const CSAMPLE_GAIN gain24 = start_gain24 + gain_delta24 * i;
        const CSAMPLE_GAIN gain25 = start_gain25 + gain_delta25 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1 +
                       pSrc2[i * 2] * gain2 +
                       pSrc3[i * 2] * gain3 +
                       pSrc4[i * 2] * gain4 +
                       pSrc5[i * 2] * gain5 +
                       pSrc6[i * 2] * gain6 +
                       pSrc7[i * 2] * gain7 +
                       pSrc8[i * 2] * gain8 +
                       pSrc9[i * 2] * gain9 +
                       pSrc10[i * 2] * gain10 +
                       pSrc11[i * 2] * gain11 +
                       pSrc12[i * 2] * gain12 +
                       pSrc13[i * 2] * gain13 +
                       pSrc14[i * 2] * gain14 +
                       pSrc15[i * 2] * gain15 +
                       pSrc16[i * 2] * gain16 +
                       pSrc17[i * 2] * gain17 +
                       pSrc18[i * 2] * gain18 +
                       pSrc19[i * 2] * gain19 +
                       pSrc20[i * 2] * gain20 +
                       pSrc21[i * 2] * gain21 +
                       pSrc22[i * 2] * gain22 +
                       pSrc23[i * 2] * gain23 +
                       pSrc24[i * 2] * gain24 +
                       pSrc25[i * 2] * gain25;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1 +
                           pSrc2[i * 2 + 1] * gain2 +
                           pSrc3[i * 2 + 1] * gain3 +
                           pSrc4[i * 2 + 1] * gain4 +
                           pSrc5[i * 2 + 1] * gain5 +
                           pSrc6[i * 2 + 1] * gain6 +
                           pSrc7[i * 2 + 1] * gain7 +
                           pSrc8[i * 2 + 1] * gain8 +
                           pSrc9[i * 2 + 1] * gain9 +
                           pSrc10[i * 2 + 1] * gain10 +
                           pSrc11[i * 2 + 1] * gain11 +
                           pSrc12[i * 2 + 1] * gain12 +
                           pSrc13[i * 2 + 1] * gain13 +
                           pSrc14[i * 2 + 1] * gain14 +
                           pSrc15[i * 2 + 1] * gain15 +
                           pSrc16[i * 2 + 1] * gain16 +
                           pSrc17[i * 2 + 1] * gain17 +
                           pSrc18[i * 2 + 1] * gain18 +
                           pSrc19[i * 2 + 1] * gain19 +
                           pSrc20[i * 2 + 1] * gain20 +
                           pSrc21[i * 2 + 1] * gain21 +
                           pSrc22[i * 2 + 1] * gain22 +
                           pSrc23[i * 2 + 1] * gain23 +
                           pSrc24[i * 2 + 1] * gain24 +
                           pSrc25[i * 2 + 1] * gain25;
    }
}
static inline void copy27WithGain(CSAMPLE* M_RESTRICT pDest,
                                  const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                  const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                  const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                  const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3,
                                  const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4,
                                  const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5,
                                  const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6,
                                  const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7,
                                  const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8,
                                  const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9,
                                  const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10,
                                  const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11,
                                  const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12,
                                  const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13,
                                  const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14,
                                  const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15,
                                  const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16,
                                  const CSAMPLE* M_RESTRICT pSrc17, CSAMPLE_GAIN gain17,
                                  const CSAMPLE* M_RESTRICT pSrc18, CSAMPLE_GAIN gain18,
                                  const CSAMPLE* M_RESTRICT pSrc19, CSAMPLE_GAIN gain19,
                                  const CSAMPLE* M_RESTRICT pSrc20, CSAMPLE_GAIN gain20,
                                  const CSAMPLE* M_RESTRICT pSrc21, CSAMPLE_GAIN gain21,
                                  const CSAMPLE* M_RESTRICT pSrc22, CSAMPLE_GAIN gain22,
                                  const CSAMPLE* M_RESTRICT pSrc23, CSAMPLE_GAIN gain23,
                                  const CSAMPLE* M_RESTRICT pSrc24, CSAMPLE_GAIN gain24,
                                  const CSAMPLE* M_RESTRICT pSrc25, CSAMPLE_GAIN gain25,
                                  const CSAMPLE* M_RESTRICT pSrc26, CSAMPLE_GAIN gain26,
                                  int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy26WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain2 == CSAMPLE_GAIN_ZERO) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain3 == CSAMPLE_GAIN_ZERO) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain4 == CSAMPLE_GAIN_ZERO) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain5 == CSAMPLE_GAIN_ZERO) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain6 == CSAMPLE_GAIN_ZERO) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain7 == CSAMPLE_GAIN_ZERO) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain8 == CSAMPLE_GAIN_ZERO) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain9 == CSAMPLE_GAIN_ZERO) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain10 == CSAMPLE_GAIN_ZERO) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain11 == CSAMPLE_GAIN_ZERO) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain12 == CSAMPLE_GAIN_ZERO) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain13 == CSAMPLE_GAIN_ZERO) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain14 == CSAMPLE_GAIN_ZERO) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain15 == CSAMPLE_GAIN_ZERO) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain16 == CSAMPLE_GAIN_ZERO) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain17 == CSAMPLE_GAIN_ZERO) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain18 == CSAMPLE_GAIN_ZERO) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain19 == CSAMPLE_GAIN_ZERO) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain20 == CSAMPLE_GAIN_ZERO) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain21 == CSAMPLE_GAIN_ZERO) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain22 == CSAMPLE_GAIN_ZERO) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain23 == CSAMPLE_GAIN_ZERO) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain24 == CSAMPLE_GAIN_ZERO) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain25 == CSAMPLE_GAIN_ZERO) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc26, gain26, iNumSamples);
        return;
    }
    if (gain26 == CSAMPLE_GAIN_ZERO) {
        copy26WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
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
static inline void copy27WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                         const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                         const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                         const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                         const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3in, CSAMPLE_GAIN gain3out,
                                         const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4in, CSAMPLE_GAIN gain4out,
                                         const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5in, CSAMPLE_GAIN gain5out,
                                         const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6in, CSAMPLE_GAIN gain6out,
                                         const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7in, CSAMPLE_GAIN gain7out,
                                         const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8in, CSAMPLE_GAIN gain8out,
                                         const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9in, CSAMPLE_GAIN gain9out,
                                         const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10in, CSAMPLE_GAIN gain10out,
                                         const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11in, CSAMPLE_GAIN gain11out,
                                         const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12in, CSAMPLE_GAIN gain12out,
                                         const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13in, CSAMPLE_GAIN gain13out,
                                         const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14in, CSAMPLE_GAIN gain14out,
                                         const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15in, CSAMPLE_GAIN gain15out,
                                         const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16in, CSAMPLE_GAIN gain16out,
                                         const CSAMPLE* M_RESTRICT pSrc17, CSAMPLE_GAIN gain17in, CSAMPLE_GAIN gain17out,
                                         const CSAMPLE* M_RESTRICT pSrc18, CSAMPLE_GAIN gain18in, CSAMPLE_GAIN gain18out,
                                         const CSAMPLE* M_RESTRICT pSrc19, CSAMPLE_GAIN gain19in, CSAMPLE_GAIN gain19out,
                                         const CSAMPLE* M_RESTRICT pSrc20, CSAMPLE_GAIN gain20in, CSAMPLE_GAIN gain20out,
                                         const CSAMPLE* M_RESTRICT pSrc21, CSAMPLE_GAIN gain21in, CSAMPLE_GAIN gain21out,
                                         const CSAMPLE* M_RESTRICT pSrc22, CSAMPLE_GAIN gain22in, CSAMPLE_GAIN gain22out,
                                         const CSAMPLE* M_RESTRICT pSrc23, CSAMPLE_GAIN gain23in, CSAMPLE_GAIN gain23out,
                                         const CSAMPLE* M_RESTRICT pSrc24, CSAMPLE_GAIN gain24in, CSAMPLE_GAIN gain24out,
                                         const CSAMPLE* M_RESTRICT pSrc25, CSAMPLE_GAIN gain25in, CSAMPLE_GAIN gain25out,
                                         const CSAMPLE* M_RESTRICT pSrc26, CSAMPLE_GAIN gain26in, CSAMPLE_GAIN gain26out,
                                         int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy26WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain2in == CSAMPLE_GAIN_ZERO && gain2out == CSAMPLE_GAIN_ZERO) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain3in == CSAMPLE_GAIN_ZERO && gain3out == CSAMPLE_GAIN_ZERO) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain4in == CSAMPLE_GAIN_ZERO && gain4out == CSAMPLE_GAIN_ZERO) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain5in == CSAMPLE_GAIN_ZERO && gain5out == CSAMPLE_GAIN_ZERO) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain6in == CSAMPLE_GAIN_ZERO && gain6out == CSAMPLE_GAIN_ZERO) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain7in == CSAMPLE_GAIN_ZERO && gain7out == CSAMPLE_GAIN_ZERO) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain8in == CSAMPLE_GAIN_ZERO && gain8out == CSAMPLE_GAIN_ZERO) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain9in == CSAMPLE_GAIN_ZERO && gain9out == CSAMPLE_GAIN_ZERO) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain10in == CSAMPLE_GAIN_ZERO && gain10out == CSAMPLE_GAIN_ZERO) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain11in == CSAMPLE_GAIN_ZERO && gain11out == CSAMPLE_GAIN_ZERO) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain12in == CSAMPLE_GAIN_ZERO && gain12out == CSAMPLE_GAIN_ZERO) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain13in == CSAMPLE_GAIN_ZERO && gain13out == CSAMPLE_GAIN_ZERO) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain14in == CSAMPLE_GAIN_ZERO && gain14out == CSAMPLE_GAIN_ZERO) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain15in == CSAMPLE_GAIN_ZERO && gain15out == CSAMPLE_GAIN_ZERO) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain16in == CSAMPLE_GAIN_ZERO && gain16out == CSAMPLE_GAIN_ZERO) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain17in == CSAMPLE_GAIN_ZERO && gain17out == CSAMPLE_GAIN_ZERO) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain18in == CSAMPLE_GAIN_ZERO && gain18out == CSAMPLE_GAIN_ZERO) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain19in == CSAMPLE_GAIN_ZERO && gain19out == CSAMPLE_GAIN_ZERO) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain20in == CSAMPLE_GAIN_ZERO && gain20out == CSAMPLE_GAIN_ZERO) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain21in == CSAMPLE_GAIN_ZERO && gain21out == CSAMPLE_GAIN_ZERO) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain22in == CSAMPLE_GAIN_ZERO && gain22out == CSAMPLE_GAIN_ZERO) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain23in == CSAMPLE_GAIN_ZERO && gain23out == CSAMPLE_GAIN_ZERO) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain24in == CSAMPLE_GAIN_ZERO && gain24out == CSAMPLE_GAIN_ZERO) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain25in == CSAMPLE_GAIN_ZERO && gain25out == CSAMPLE_GAIN_ZERO) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    if (gain26in == CSAMPLE_GAIN_ZERO && gain26out == CSAMPLE_GAIN_ZERO) {
        copy26WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    const CSAMPLE_GAIN gain_delta2 = (gain2out - gain2in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain2 = gain2in + gain_delta2;
    const CSAMPLE_GAIN gain_delta3 = (gain3out - gain3in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain3 = gain3in + gain_delta3;
    const CSAMPLE_GAIN gain_delta4 = (gain4out - gain4in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain4 = gain4in + gain_delta4;
    const CSAMPLE_GAIN gain_delta5 = (gain5out - gain5in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain5 = gain5in + gain_delta5;
    const CSAMPLE_GAIN gain_delta6 = (gain6out - gain6in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain6 = gain6in + gain_delta6;
    const CSAMPLE_GAIN gain_delta7 = (gain7out - gain7in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain7 = gain7in + gain_delta7;
    const CSAMPLE_GAIN gain_delta8 = (gain8out - gain8in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain8 = gain8in + gain_delta8;
    const CSAMPLE_GAIN gain_delta9 = (gain9out - gain9in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain9 = gain9in + gain_delta9;
    const CSAMPLE_GAIN gain_delta10 = (gain10out - gain10in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain10 = gain10in + gain_delta10;
    const CSAMPLE_GAIN gain_delta11 = (gain11out - gain11in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain11 = gain11in + gain_delta11;
    const CSAMPLE_GAIN gain_delta12 = (gain12out - gain12in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain12 = gain12in + gain_delta12;
    const CSAMPLE_GAIN gain_delta13 = (gain13out - gain13in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain13 = gain13in + gain_delta13;
    const CSAMPLE_GAIN gain_delta14 = (gain14out - gain14in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain14 = gain14in + gain_delta14;
    const CSAMPLE_GAIN gain_delta15 = (gain15out - gain15in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain15 = gain15in + gain_delta15;
    const CSAMPLE_GAIN gain_delta16 = (gain16out - gain16in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain16 = gain16in + gain_delta16;
    const CSAMPLE_GAIN gain_delta17 = (gain17out - gain17in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain17 = gain17in + gain_delta17;
    const CSAMPLE_GAIN gain_delta18 = (gain18out - gain18in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain18 = gain18in + gain_delta18;
    const CSAMPLE_GAIN gain_delta19 = (gain19out - gain19in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain19 = gain19in + gain_delta19;
    const CSAMPLE_GAIN gain_delta20 = (gain20out - gain20in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain20 = gain20in + gain_delta20;
    const CSAMPLE_GAIN gain_delta21 = (gain21out - gain21in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain21 = gain21in + gain_delta21;
    const CSAMPLE_GAIN gain_delta22 = (gain22out - gain22in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain22 = gain22in + gain_delta22;
    const CSAMPLE_GAIN gain_delta23 = (gain23out - gain23in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain23 = gain23in + gain_delta23;
    const CSAMPLE_GAIN gain_delta24 = (gain24out - gain24in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain24 = gain24in + gain_delta24;
    const CSAMPLE_GAIN gain_delta25 = (gain25out - gain25in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain25 = gain25in + gain_delta25;
    const CSAMPLE_GAIN gain_delta26 = (gain26out - gain26in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain26 = gain26in + gain_delta26;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        const CSAMPLE_GAIN gain2 = start_gain2 + gain_delta2 * i;
        const CSAMPLE_GAIN gain3 = start_gain3 + gain_delta3 * i;
        const CSAMPLE_GAIN gain4 = start_gain4 + gain_delta4 * i;
        const CSAMPLE_GAIN gain5 = start_gain5 + gain_delta5 * i;
        const CSAMPLE_GAIN gain6 = start_gain6 + gain_delta6 * i;
        const CSAMPLE_GAIN gain7 = start_gain7 + gain_delta7 * i;
        const CSAMPLE_GAIN gain8 = start_gain8 + gain_delta8 * i;
        const CSAMPLE_GAIN gain9 = start_gain9 + gain_delta9 * i;
        const CSAMPLE_GAIN gain10 = start_gain10 + gain_delta10 * i;
        const CSAMPLE_GAIN gain11 = start_gain11 + gain_delta11 * i;
        const CSAMPLE_GAIN gain12 = start_gain12 + gain_delta12 * i;
        const CSAMPLE_GAIN gain13 = start_gain13 + gain_delta13 * i;
        const CSAMPLE_GAIN gain14 = start_gain14 + gain_delta14 * i;
        const CSAMPLE_GAIN gain15 = start_gain15 + gain_delta15 * i;
        const CSAMPLE_GAIN gain16 = start_gain16 + gain_delta16 * i;
        const CSAMPLE_GAIN gain17 = start_gain17 + gain_delta17 * i;
        const CSAMPLE_GAIN gain18 = start_gain18 + gain_delta18 * i;
        const CSAMPLE_GAIN gain19 = start_gain19 + gain_delta19 * i;
        const CSAMPLE_GAIN gain20 = start_gain20 + gain_delta20 * i;
        const CSAMPLE_GAIN gain21 = start_gain21 + gain_delta21 * i;
        const CSAMPLE_GAIN gain22 = start_gain22 + gain_delta22 * i;
        const CSAMPLE_GAIN gain23 = start_gain23 + gain_delta23 * i;
        const CSAMPLE_GAIN gain24 = start_gain24 + gain_delta24 * i;
        const CSAMPLE_GAIN gain25 = start_gain25 + gain_delta25 * i;
        const CSAMPLE_GAIN gain26 = start_gain26 + gain_delta26 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1 +
                       pSrc2[i * 2] * gain2 +
                       pSrc3[i * 2] * gain3 +
                       pSrc4[i * 2] * gain4 +
                       pSrc5[i * 2] * gain5 +
                       pSrc6[i * 2] * gain6 +
                       pSrc7[i * 2] * gain7 +
                       pSrc8[i * 2] * gain8 +
                       pSrc9[i * 2] * gain9 +
                       pSrc10[i * 2] * gain10 +
                       pSrc11[i * 2] * gain11 +
                       pSrc12[i * 2] * gain12 +
                       pSrc13[i * 2] * gain13 +
                       pSrc14[i * 2] * gain14 +
                       pSrc15[i * 2] * gain15 +
                       pSrc16[i * 2] * gain16 +
                       pSrc17[i * 2] * gain17 +
                       pSrc18[i * 2] * gain18 +
                       pSrc19[i * 2] * gain19 +
                       pSrc20[i * 2] * gain20 +
                       pSrc21[i * 2] * gain21 +
                       pSrc22[i * 2] * gain22 +
                       pSrc23[i * 2] * gain23 +
                       pSrc24[i * 2] * gain24 +
                       pSrc25[i * 2] * gain25 +
                       pSrc26[i * 2] * gain26;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1 +
                           pSrc2[i * 2 + 1] * gain2 +
                           pSrc3[i * 2 + 1] * gain3 +
                           pSrc4[i * 2 + 1] * gain4 +
                           pSrc5[i * 2 + 1] * gain5 +
                           pSrc6[i * 2 + 1] * gain6 +
                           pSrc7[i * 2 + 1] * gain7 +
                           pSrc8[i * 2 + 1] * gain8 +
                           pSrc9[i * 2 + 1] * gain9 +
                           pSrc10[i * 2 + 1] * gain10 +
                           pSrc11[i * 2 + 1] * gain11 +
                           pSrc12[i * 2 + 1] * gain12 +
                           pSrc13[i * 2 + 1] * gain13 +
                           pSrc14[i * 2 + 1] * gain14 +
                           pSrc15[i * 2 + 1] * gain15 +
                           pSrc16[i * 2 + 1] * gain16 +
                           pSrc17[i * 2 + 1] * gain17 +
                           pSrc18[i * 2 + 1] * gain18 +
                           pSrc19[i * 2 + 1] * gain19 +
                           pSrc20[i * 2 + 1] * gain20 +
                           pSrc21[i * 2 + 1] * gain21 +
                           pSrc22[i * 2 + 1] * gain22 +
                           pSrc23[i * 2 + 1] * gain23 +
                           pSrc24[i * 2 + 1] * gain24 +
                           pSrc25[i * 2 + 1] * gain25 +
                           pSrc26[i * 2 + 1] * gain26;
    }
}
static inline void copy28WithGain(CSAMPLE* M_RESTRICT pDest,
                                  const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                  const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                  const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                  const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3,
                                  const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4,
                                  const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5,
                                  const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6,
                                  const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7,
                                  const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8,
                                  const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9,
                                  const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10,
                                  const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11,
                                  const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12,
                                  const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13,
                                  const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14,
                                  const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15,
                                  const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16,
                                  const CSAMPLE* M_RESTRICT pSrc17, CSAMPLE_GAIN gain17,
                                  const CSAMPLE* M_RESTRICT pSrc18, CSAMPLE_GAIN gain18,
                                  const CSAMPLE* M_RESTRICT pSrc19, CSAMPLE_GAIN gain19,
                                  const CSAMPLE* M_RESTRICT pSrc20, CSAMPLE_GAIN gain20,
                                  const CSAMPLE* M_RESTRICT pSrc21, CSAMPLE_GAIN gain21,
                                  const CSAMPLE* M_RESTRICT pSrc22, CSAMPLE_GAIN gain22,
                                  const CSAMPLE* M_RESTRICT pSrc23, CSAMPLE_GAIN gain23,
                                  const CSAMPLE* M_RESTRICT pSrc24, CSAMPLE_GAIN gain24,
                                  const CSAMPLE* M_RESTRICT pSrc25, CSAMPLE_GAIN gain25,
                                  const CSAMPLE* M_RESTRICT pSrc26, CSAMPLE_GAIN gain26,
                                  const CSAMPLE* M_RESTRICT pSrc27, CSAMPLE_GAIN gain27,
                                  int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy27WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain2 == CSAMPLE_GAIN_ZERO) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain3 == CSAMPLE_GAIN_ZERO) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain4 == CSAMPLE_GAIN_ZERO) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain5 == CSAMPLE_GAIN_ZERO) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain6 == CSAMPLE_GAIN_ZERO) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain7 == CSAMPLE_GAIN_ZERO) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain8 == CSAMPLE_GAIN_ZERO) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain9 == CSAMPLE_GAIN_ZERO) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain10 == CSAMPLE_GAIN_ZERO) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain11 == CSAMPLE_GAIN_ZERO) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain12 == CSAMPLE_GAIN_ZERO) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain13 == CSAMPLE_GAIN_ZERO) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain14 == CSAMPLE_GAIN_ZERO) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain15 == CSAMPLE_GAIN_ZERO) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain16 == CSAMPLE_GAIN_ZERO) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain17 == CSAMPLE_GAIN_ZERO) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain18 == CSAMPLE_GAIN_ZERO) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain19 == CSAMPLE_GAIN_ZERO) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain20 == CSAMPLE_GAIN_ZERO) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain21 == CSAMPLE_GAIN_ZERO) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain22 == CSAMPLE_GAIN_ZERO) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain23 == CSAMPLE_GAIN_ZERO) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain24 == CSAMPLE_GAIN_ZERO) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain25 == CSAMPLE_GAIN_ZERO) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain26 == CSAMPLE_GAIN_ZERO) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc27, gain27, iNumSamples);
        return;
    }
    if (gain27 == CSAMPLE_GAIN_ZERO) {
        copy27WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
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
static inline void copy28WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                         const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                         const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                         const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                         const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3in, CSAMPLE_GAIN gain3out,
                                         const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4in, CSAMPLE_GAIN gain4out,
                                         const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5in, CSAMPLE_GAIN gain5out,
                                         const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6in, CSAMPLE_GAIN gain6out,
                                         const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7in, CSAMPLE_GAIN gain7out,
                                         const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8in, CSAMPLE_GAIN gain8out,
                                         const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9in, CSAMPLE_GAIN gain9out,
                                         const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10in, CSAMPLE_GAIN gain10out,
                                         const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11in, CSAMPLE_GAIN gain11out,
                                         const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12in, CSAMPLE_GAIN gain12out,
                                         const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13in, CSAMPLE_GAIN gain13out,
                                         const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14in, CSAMPLE_GAIN gain14out,
                                         const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15in, CSAMPLE_GAIN gain15out,
                                         const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16in, CSAMPLE_GAIN gain16out,
                                         const CSAMPLE* M_RESTRICT pSrc17, CSAMPLE_GAIN gain17in, CSAMPLE_GAIN gain17out,
                                         const CSAMPLE* M_RESTRICT pSrc18, CSAMPLE_GAIN gain18in, CSAMPLE_GAIN gain18out,
                                         const CSAMPLE* M_RESTRICT pSrc19, CSAMPLE_GAIN gain19in, CSAMPLE_GAIN gain19out,
                                         const CSAMPLE* M_RESTRICT pSrc20, CSAMPLE_GAIN gain20in, CSAMPLE_GAIN gain20out,
                                         const CSAMPLE* M_RESTRICT pSrc21, CSAMPLE_GAIN gain21in, CSAMPLE_GAIN gain21out,
                                         const CSAMPLE* M_RESTRICT pSrc22, CSAMPLE_GAIN gain22in, CSAMPLE_GAIN gain22out,
                                         const CSAMPLE* M_RESTRICT pSrc23, CSAMPLE_GAIN gain23in, CSAMPLE_GAIN gain23out,
                                         const CSAMPLE* M_RESTRICT pSrc24, CSAMPLE_GAIN gain24in, CSAMPLE_GAIN gain24out,
                                         const CSAMPLE* M_RESTRICT pSrc25, CSAMPLE_GAIN gain25in, CSAMPLE_GAIN gain25out,
                                         const CSAMPLE* M_RESTRICT pSrc26, CSAMPLE_GAIN gain26in, CSAMPLE_GAIN gain26out,
                                         const CSAMPLE* M_RESTRICT pSrc27, CSAMPLE_GAIN gain27in, CSAMPLE_GAIN gain27out,
                                         int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy27WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain2in == CSAMPLE_GAIN_ZERO && gain2out == CSAMPLE_GAIN_ZERO) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain3in == CSAMPLE_GAIN_ZERO && gain3out == CSAMPLE_GAIN_ZERO) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain4in == CSAMPLE_GAIN_ZERO && gain4out == CSAMPLE_GAIN_ZERO) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain5in == CSAMPLE_GAIN_ZERO && gain5out == CSAMPLE_GAIN_ZERO) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain6in == CSAMPLE_GAIN_ZERO && gain6out == CSAMPLE_GAIN_ZERO) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain7in == CSAMPLE_GAIN_ZERO && gain7out == CSAMPLE_GAIN_ZERO) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain8in == CSAMPLE_GAIN_ZERO && gain8out == CSAMPLE_GAIN_ZERO) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain9in == CSAMPLE_GAIN_ZERO && gain9out == CSAMPLE_GAIN_ZERO) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain10in == CSAMPLE_GAIN_ZERO && gain10out == CSAMPLE_GAIN_ZERO) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain11in == CSAMPLE_GAIN_ZERO && gain11out == CSAMPLE_GAIN_ZERO) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain12in == CSAMPLE_GAIN_ZERO && gain12out == CSAMPLE_GAIN_ZERO) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain13in == CSAMPLE_GAIN_ZERO && gain13out == CSAMPLE_GAIN_ZERO) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain14in == CSAMPLE_GAIN_ZERO && gain14out == CSAMPLE_GAIN_ZERO) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain15in == CSAMPLE_GAIN_ZERO && gain15out == CSAMPLE_GAIN_ZERO) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain16in == CSAMPLE_GAIN_ZERO && gain16out == CSAMPLE_GAIN_ZERO) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain17in == CSAMPLE_GAIN_ZERO && gain17out == CSAMPLE_GAIN_ZERO) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain18in == CSAMPLE_GAIN_ZERO && gain18out == CSAMPLE_GAIN_ZERO) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain19in == CSAMPLE_GAIN_ZERO && gain19out == CSAMPLE_GAIN_ZERO) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain20in == CSAMPLE_GAIN_ZERO && gain20out == CSAMPLE_GAIN_ZERO) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain21in == CSAMPLE_GAIN_ZERO && gain21out == CSAMPLE_GAIN_ZERO) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain22in == CSAMPLE_GAIN_ZERO && gain22out == CSAMPLE_GAIN_ZERO) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain23in == CSAMPLE_GAIN_ZERO && gain23out == CSAMPLE_GAIN_ZERO) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain24in == CSAMPLE_GAIN_ZERO && gain24out == CSAMPLE_GAIN_ZERO) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain25in == CSAMPLE_GAIN_ZERO && gain25out == CSAMPLE_GAIN_ZERO) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain26in == CSAMPLE_GAIN_ZERO && gain26out == CSAMPLE_GAIN_ZERO) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    if (gain27in == CSAMPLE_GAIN_ZERO && gain27out == CSAMPLE_GAIN_ZERO) {
        copy27WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    const CSAMPLE_GAIN gain_delta2 = (gain2out - gain2in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain2 = gain2in + gain_delta2;
    const CSAMPLE_GAIN gain_delta3 = (gain3out - gain3in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain3 = gain3in + gain_delta3;
    const CSAMPLE_GAIN gain_delta4 = (gain4out - gain4in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain4 = gain4in + gain_delta4;
    const CSAMPLE_GAIN gain_delta5 = (gain5out - gain5in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain5 = gain5in + gain_delta5;
    const CSAMPLE_GAIN gain_delta6 = (gain6out - gain6in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain6 = gain6in + gain_delta6;
    const CSAMPLE_GAIN gain_delta7 = (gain7out - gain7in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain7 = gain7in + gain_delta7;
    const CSAMPLE_GAIN gain_delta8 = (gain8out - gain8in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain8 = gain8in + gain_delta8;
    const CSAMPLE_GAIN gain_delta9 = (gain9out - gain9in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain9 = gain9in + gain_delta9;
    const CSAMPLE_GAIN gain_delta10 = (gain10out - gain10in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain10 = gain10in + gain_delta10;
    const CSAMPLE_GAIN gain_delta11 = (gain11out - gain11in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain11 = gain11in + gain_delta11;
    const CSAMPLE_GAIN gain_delta12 = (gain12out - gain12in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain12 = gain12in + gain_delta12;
    const CSAMPLE_GAIN gain_delta13 = (gain13out - gain13in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain13 = gain13in + gain_delta13;
    const CSAMPLE_GAIN gain_delta14 = (gain14out - gain14in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain14 = gain14in + gain_delta14;
    const CSAMPLE_GAIN gain_delta15 = (gain15out - gain15in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain15 = gain15in + gain_delta15;
    const CSAMPLE_GAIN gain_delta16 = (gain16out - gain16in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain16 = gain16in + gain_delta16;
    const CSAMPLE_GAIN gain_delta17 = (gain17out - gain17in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain17 = gain17in + gain_delta17;
    const CSAMPLE_GAIN gain_delta18 = (gain18out - gain18in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain18 = gain18in + gain_delta18;
    const CSAMPLE_GAIN gain_delta19 = (gain19out - gain19in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain19 = gain19in + gain_delta19;
    const CSAMPLE_GAIN gain_delta20 = (gain20out - gain20in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain20 = gain20in + gain_delta20;
    const CSAMPLE_GAIN gain_delta21 = (gain21out - gain21in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain21 = gain21in + gain_delta21;
    const CSAMPLE_GAIN gain_delta22 = (gain22out - gain22in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain22 = gain22in + gain_delta22;
    const CSAMPLE_GAIN gain_delta23 = (gain23out - gain23in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain23 = gain23in + gain_delta23;
    const CSAMPLE_GAIN gain_delta24 = (gain24out - gain24in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain24 = gain24in + gain_delta24;
    const CSAMPLE_GAIN gain_delta25 = (gain25out - gain25in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain25 = gain25in + gain_delta25;
    const CSAMPLE_GAIN gain_delta26 = (gain26out - gain26in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain26 = gain26in + gain_delta26;
    const CSAMPLE_GAIN gain_delta27 = (gain27out - gain27in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain27 = gain27in + gain_delta27;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        const CSAMPLE_GAIN gain2 = start_gain2 + gain_delta2 * i;
        const CSAMPLE_GAIN gain3 = start_gain3 + gain_delta3 * i;
        const CSAMPLE_GAIN gain4 = start_gain4 + gain_delta4 * i;
        const CSAMPLE_GAIN gain5 = start_gain5 + gain_delta5 * i;
        const CSAMPLE_GAIN gain6 = start_gain6 + gain_delta6 * i;
        const CSAMPLE_GAIN gain7 = start_gain7 + gain_delta7 * i;
        const CSAMPLE_GAIN gain8 = start_gain8 + gain_delta8 * i;
        const CSAMPLE_GAIN gain9 = start_gain9 + gain_delta9 * i;
        const CSAMPLE_GAIN gain10 = start_gain10 + gain_delta10 * i;
        const CSAMPLE_GAIN gain11 = start_gain11 + gain_delta11 * i;
        const CSAMPLE_GAIN gain12 = start_gain12 + gain_delta12 * i;
        const CSAMPLE_GAIN gain13 = start_gain13 + gain_delta13 * i;
        const CSAMPLE_GAIN gain14 = start_gain14 + gain_delta14 * i;
        const CSAMPLE_GAIN gain15 = start_gain15 + gain_delta15 * i;
        const CSAMPLE_GAIN gain16 = start_gain16 + gain_delta16 * i;
        const CSAMPLE_GAIN gain17 = start_gain17 + gain_delta17 * i;
        const CSAMPLE_GAIN gain18 = start_gain18 + gain_delta18 * i;
        const CSAMPLE_GAIN gain19 = start_gain19 + gain_delta19 * i;
        const CSAMPLE_GAIN gain20 = start_gain20 + gain_delta20 * i;
        const CSAMPLE_GAIN gain21 = start_gain21 + gain_delta21 * i;
        const CSAMPLE_GAIN gain22 = start_gain22 + gain_delta22 * i;
        const CSAMPLE_GAIN gain23 = start_gain23 + gain_delta23 * i;
        const CSAMPLE_GAIN gain24 = start_gain24 + gain_delta24 * i;
        const CSAMPLE_GAIN gain25 = start_gain25 + gain_delta25 * i;
        const CSAMPLE_GAIN gain26 = start_gain26 + gain_delta26 * i;
        const CSAMPLE_GAIN gain27 = start_gain27 + gain_delta27 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1 +
                       pSrc2[i * 2] * gain2 +
                       pSrc3[i * 2] * gain3 +
                       pSrc4[i * 2] * gain4 +
                       pSrc5[i * 2] * gain5 +
                       pSrc6[i * 2] * gain6 +
                       pSrc7[i * 2] * gain7 +
                       pSrc8[i * 2] * gain8 +
                       pSrc9[i * 2] * gain9 +
                       pSrc10[i * 2] * gain10 +
                       pSrc11[i * 2] * gain11 +
                       pSrc12[i * 2] * gain12 +
                       pSrc13[i * 2] * gain13 +
                       pSrc14[i * 2] * gain14 +
                       pSrc15[i * 2] * gain15 +
                       pSrc16[i * 2] * gain16 +
                       pSrc17[i * 2] * gain17 +
                       pSrc18[i * 2] * gain18 +
                       pSrc19[i * 2] * gain19 +
                       pSrc20[i * 2] * gain20 +
                       pSrc21[i * 2] * gain21 +
                       pSrc22[i * 2] * gain22 +
                       pSrc23[i * 2] * gain23 +
                       pSrc24[i * 2] * gain24 +
                       pSrc25[i * 2] * gain25 +
                       pSrc26[i * 2] * gain26 +
                       pSrc27[i * 2] * gain27;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1 +
                           pSrc2[i * 2 + 1] * gain2 +
                           pSrc3[i * 2 + 1] * gain3 +
                           pSrc4[i * 2 + 1] * gain4 +
                           pSrc5[i * 2 + 1] * gain5 +
                           pSrc6[i * 2 + 1] * gain6 +
                           pSrc7[i * 2 + 1] * gain7 +
                           pSrc8[i * 2 + 1] * gain8 +
                           pSrc9[i * 2 + 1] * gain9 +
                           pSrc10[i * 2 + 1] * gain10 +
                           pSrc11[i * 2 + 1] * gain11 +
                           pSrc12[i * 2 + 1] * gain12 +
                           pSrc13[i * 2 + 1] * gain13 +
                           pSrc14[i * 2 + 1] * gain14 +
                           pSrc15[i * 2 + 1] * gain15 +
                           pSrc16[i * 2 + 1] * gain16 +
                           pSrc17[i * 2 + 1] * gain17 +
                           pSrc18[i * 2 + 1] * gain18 +
                           pSrc19[i * 2 + 1] * gain19 +
                           pSrc20[i * 2 + 1] * gain20 +
                           pSrc21[i * 2 + 1] * gain21 +
                           pSrc22[i * 2 + 1] * gain22 +
                           pSrc23[i * 2 + 1] * gain23 +
                           pSrc24[i * 2 + 1] * gain24 +
                           pSrc25[i * 2 + 1] * gain25 +
                           pSrc26[i * 2 + 1] * gain26 +
                           pSrc27[i * 2 + 1] * gain27;
    }
}
static inline void copy29WithGain(CSAMPLE* M_RESTRICT pDest,
                                  const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                  const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                  const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                  const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3,
                                  const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4,
                                  const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5,
                                  const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6,
                                  const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7,
                                  const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8,
                                  const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9,
                                  const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10,
                                  const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11,
                                  const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12,
                                  const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13,
                                  const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14,
                                  const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15,
                                  const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16,
                                  const CSAMPLE* M_RESTRICT pSrc17, CSAMPLE_GAIN gain17,
                                  const CSAMPLE* M_RESTRICT pSrc18, CSAMPLE_GAIN gain18,
                                  const CSAMPLE* M_RESTRICT pSrc19, CSAMPLE_GAIN gain19,
                                  const CSAMPLE* M_RESTRICT pSrc20, CSAMPLE_GAIN gain20,
                                  const CSAMPLE* M_RESTRICT pSrc21, CSAMPLE_GAIN gain21,
                                  const CSAMPLE* M_RESTRICT pSrc22, CSAMPLE_GAIN gain22,
                                  const CSAMPLE* M_RESTRICT pSrc23, CSAMPLE_GAIN gain23,
                                  const CSAMPLE* M_RESTRICT pSrc24, CSAMPLE_GAIN gain24,
                                  const CSAMPLE* M_RESTRICT pSrc25, CSAMPLE_GAIN gain25,
                                  const CSAMPLE* M_RESTRICT pSrc26, CSAMPLE_GAIN gain26,
                                  const CSAMPLE* M_RESTRICT pSrc27, CSAMPLE_GAIN gain27,
                                  const CSAMPLE* M_RESTRICT pSrc28, CSAMPLE_GAIN gain28,
                                  int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy28WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain2 == CSAMPLE_GAIN_ZERO) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain3 == CSAMPLE_GAIN_ZERO) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain4 == CSAMPLE_GAIN_ZERO) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain5 == CSAMPLE_GAIN_ZERO) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain6 == CSAMPLE_GAIN_ZERO) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain7 == CSAMPLE_GAIN_ZERO) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain8 == CSAMPLE_GAIN_ZERO) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain9 == CSAMPLE_GAIN_ZERO) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain10 == CSAMPLE_GAIN_ZERO) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain11 == CSAMPLE_GAIN_ZERO) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain12 == CSAMPLE_GAIN_ZERO) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain13 == CSAMPLE_GAIN_ZERO) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain14 == CSAMPLE_GAIN_ZERO) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain15 == CSAMPLE_GAIN_ZERO) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain16 == CSAMPLE_GAIN_ZERO) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain17 == CSAMPLE_GAIN_ZERO) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain18 == CSAMPLE_GAIN_ZERO) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain19 == CSAMPLE_GAIN_ZERO) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain20 == CSAMPLE_GAIN_ZERO) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain21 == CSAMPLE_GAIN_ZERO) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain22 == CSAMPLE_GAIN_ZERO) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain23 == CSAMPLE_GAIN_ZERO) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain24 == CSAMPLE_GAIN_ZERO) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain25 == CSAMPLE_GAIN_ZERO) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain26 == CSAMPLE_GAIN_ZERO) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain27 == CSAMPLE_GAIN_ZERO) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc28, gain28, iNumSamples);
        return;
    }
    if (gain28 == CSAMPLE_GAIN_ZERO) {
        copy28WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
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
static inline void copy29WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                         const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                         const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                         const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                         const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3in, CSAMPLE_GAIN gain3out,
                                         const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4in, CSAMPLE_GAIN gain4out,
                                         const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5in, CSAMPLE_GAIN gain5out,
                                         const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6in, CSAMPLE_GAIN gain6out,
                                         const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7in, CSAMPLE_GAIN gain7out,
                                         const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8in, CSAMPLE_GAIN gain8out,
                                         const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9in, CSAMPLE_GAIN gain9out,
                                         const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10in, CSAMPLE_GAIN gain10out,
                                         const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11in, CSAMPLE_GAIN gain11out,
                                         const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12in, CSAMPLE_GAIN gain12out,
                                         const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13in, CSAMPLE_GAIN gain13out,
                                         const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14in, CSAMPLE_GAIN gain14out,
                                         const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15in, CSAMPLE_GAIN gain15out,
                                         const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16in, CSAMPLE_GAIN gain16out,
                                         const CSAMPLE* M_RESTRICT pSrc17, CSAMPLE_GAIN gain17in, CSAMPLE_GAIN gain17out,
                                         const CSAMPLE* M_RESTRICT pSrc18, CSAMPLE_GAIN gain18in, CSAMPLE_GAIN gain18out,
                                         const CSAMPLE* M_RESTRICT pSrc19, CSAMPLE_GAIN gain19in, CSAMPLE_GAIN gain19out,
                                         const CSAMPLE* M_RESTRICT pSrc20, CSAMPLE_GAIN gain20in, CSAMPLE_GAIN gain20out,
                                         const CSAMPLE* M_RESTRICT pSrc21, CSAMPLE_GAIN gain21in, CSAMPLE_GAIN gain21out,
                                         const CSAMPLE* M_RESTRICT pSrc22, CSAMPLE_GAIN gain22in, CSAMPLE_GAIN gain22out,
                                         const CSAMPLE* M_RESTRICT pSrc23, CSAMPLE_GAIN gain23in, CSAMPLE_GAIN gain23out,
                                         const CSAMPLE* M_RESTRICT pSrc24, CSAMPLE_GAIN gain24in, CSAMPLE_GAIN gain24out,
                                         const CSAMPLE* M_RESTRICT pSrc25, CSAMPLE_GAIN gain25in, CSAMPLE_GAIN gain25out,
                                         const CSAMPLE* M_RESTRICT pSrc26, CSAMPLE_GAIN gain26in, CSAMPLE_GAIN gain26out,
                                         const CSAMPLE* M_RESTRICT pSrc27, CSAMPLE_GAIN gain27in, CSAMPLE_GAIN gain27out,
                                         const CSAMPLE* M_RESTRICT pSrc28, CSAMPLE_GAIN gain28in, CSAMPLE_GAIN gain28out,
                                         int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy28WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain2in == CSAMPLE_GAIN_ZERO && gain2out == CSAMPLE_GAIN_ZERO) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain3in == CSAMPLE_GAIN_ZERO && gain3out == CSAMPLE_GAIN_ZERO) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain4in == CSAMPLE_GAIN_ZERO && gain4out == CSAMPLE_GAIN_ZERO) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain5in == CSAMPLE_GAIN_ZERO && gain5out == CSAMPLE_GAIN_ZERO) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain6in == CSAMPLE_GAIN_ZERO && gain6out == CSAMPLE_GAIN_ZERO) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain7in == CSAMPLE_GAIN_ZERO && gain7out == CSAMPLE_GAIN_ZERO) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain8in == CSAMPLE_GAIN_ZERO && gain8out == CSAMPLE_GAIN_ZERO) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain9in == CSAMPLE_GAIN_ZERO && gain9out == CSAMPLE_GAIN_ZERO) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain10in == CSAMPLE_GAIN_ZERO && gain10out == CSAMPLE_GAIN_ZERO) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain11in == CSAMPLE_GAIN_ZERO && gain11out == CSAMPLE_GAIN_ZERO) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain12in == CSAMPLE_GAIN_ZERO && gain12out == CSAMPLE_GAIN_ZERO) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain13in == CSAMPLE_GAIN_ZERO && gain13out == CSAMPLE_GAIN_ZERO) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain14in == CSAMPLE_GAIN_ZERO && gain14out == CSAMPLE_GAIN_ZERO) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain15in == CSAMPLE_GAIN_ZERO && gain15out == CSAMPLE_GAIN_ZERO) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain16in == CSAMPLE_GAIN_ZERO && gain16out == CSAMPLE_GAIN_ZERO) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain17in == CSAMPLE_GAIN_ZERO && gain17out == CSAMPLE_GAIN_ZERO) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain18in == CSAMPLE_GAIN_ZERO && gain18out == CSAMPLE_GAIN_ZERO) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain19in == CSAMPLE_GAIN_ZERO && gain19out == CSAMPLE_GAIN_ZERO) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain20in == CSAMPLE_GAIN_ZERO && gain20out == CSAMPLE_GAIN_ZERO) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain21in == CSAMPLE_GAIN_ZERO && gain21out == CSAMPLE_GAIN_ZERO) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain22in == CSAMPLE_GAIN_ZERO && gain22out == CSAMPLE_GAIN_ZERO) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain23in == CSAMPLE_GAIN_ZERO && gain23out == CSAMPLE_GAIN_ZERO) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain24in == CSAMPLE_GAIN_ZERO && gain24out == CSAMPLE_GAIN_ZERO) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain25in == CSAMPLE_GAIN_ZERO && gain25out == CSAMPLE_GAIN_ZERO) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain26in == CSAMPLE_GAIN_ZERO && gain26out == CSAMPLE_GAIN_ZERO) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain27in == CSAMPLE_GAIN_ZERO && gain27out == CSAMPLE_GAIN_ZERO) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    if (gain28in == CSAMPLE_GAIN_ZERO && gain28out == CSAMPLE_GAIN_ZERO) {
        copy28WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    const CSAMPLE_GAIN gain_delta2 = (gain2out - gain2in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain2 = gain2in + gain_delta2;
    const CSAMPLE_GAIN gain_delta3 = (gain3out - gain3in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain3 = gain3in + gain_delta3;
    const CSAMPLE_GAIN gain_delta4 = (gain4out - gain4in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain4 = gain4in + gain_delta4;
    const CSAMPLE_GAIN gain_delta5 = (gain5out - gain5in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain5 = gain5in + gain_delta5;
    const CSAMPLE_GAIN gain_delta6 = (gain6out - gain6in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain6 = gain6in + gain_delta6;
    const CSAMPLE_GAIN gain_delta7 = (gain7out - gain7in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain7 = gain7in + gain_delta7;
    const CSAMPLE_GAIN gain_delta8 = (gain8out - gain8in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain8 = gain8in + gain_delta8;
    const CSAMPLE_GAIN gain_delta9 = (gain9out - gain9in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain9 = gain9in + gain_delta9;
    const CSAMPLE_GAIN gain_delta10 = (gain10out - gain10in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain10 = gain10in + gain_delta10;
    const CSAMPLE_GAIN gain_delta11 = (gain11out - gain11in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain11 = gain11in + gain_delta11;
    const CSAMPLE_GAIN gain_delta12 = (gain12out - gain12in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain12 = gain12in + gain_delta12;
    const CSAMPLE_GAIN gain_delta13 = (gain13out - gain13in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain13 = gain13in + gain_delta13;
    const CSAMPLE_GAIN gain_delta14 = (gain14out - gain14in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain14 = gain14in + gain_delta14;
    const CSAMPLE_GAIN gain_delta15 = (gain15out - gain15in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain15 = gain15in + gain_delta15;
    const CSAMPLE_GAIN gain_delta16 = (gain16out - gain16in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain16 = gain16in + gain_delta16;
    const CSAMPLE_GAIN gain_delta17 = (gain17out - gain17in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain17 = gain17in + gain_delta17;
    const CSAMPLE_GAIN gain_delta18 = (gain18out - gain18in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain18 = gain18in + gain_delta18;
    const CSAMPLE_GAIN gain_delta19 = (gain19out - gain19in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain19 = gain19in + gain_delta19;
    const CSAMPLE_GAIN gain_delta20 = (gain20out - gain20in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain20 = gain20in + gain_delta20;
    const CSAMPLE_GAIN gain_delta21 = (gain21out - gain21in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain21 = gain21in + gain_delta21;
    const CSAMPLE_GAIN gain_delta22 = (gain22out - gain22in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain22 = gain22in + gain_delta22;
    const CSAMPLE_GAIN gain_delta23 = (gain23out - gain23in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain23 = gain23in + gain_delta23;
    const CSAMPLE_GAIN gain_delta24 = (gain24out - gain24in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain24 = gain24in + gain_delta24;
    const CSAMPLE_GAIN gain_delta25 = (gain25out - gain25in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain25 = gain25in + gain_delta25;
    const CSAMPLE_GAIN gain_delta26 = (gain26out - gain26in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain26 = gain26in + gain_delta26;
    const CSAMPLE_GAIN gain_delta27 = (gain27out - gain27in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain27 = gain27in + gain_delta27;
    const CSAMPLE_GAIN gain_delta28 = (gain28out - gain28in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain28 = gain28in + gain_delta28;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        const CSAMPLE_GAIN gain2 = start_gain2 + gain_delta2 * i;
        const CSAMPLE_GAIN gain3 = start_gain3 + gain_delta3 * i;
        const CSAMPLE_GAIN gain4 = start_gain4 + gain_delta4 * i;
        const CSAMPLE_GAIN gain5 = start_gain5 + gain_delta5 * i;
        const CSAMPLE_GAIN gain6 = start_gain6 + gain_delta6 * i;
        const CSAMPLE_GAIN gain7 = start_gain7 + gain_delta7 * i;
        const CSAMPLE_GAIN gain8 = start_gain8 + gain_delta8 * i;
        const CSAMPLE_GAIN gain9 = start_gain9 + gain_delta9 * i;
        const CSAMPLE_GAIN gain10 = start_gain10 + gain_delta10 * i;
        const CSAMPLE_GAIN gain11 = start_gain11 + gain_delta11 * i;
        const CSAMPLE_GAIN gain12 = start_gain12 + gain_delta12 * i;
        const CSAMPLE_GAIN gain13 = start_gain13 + gain_delta13 * i;
        const CSAMPLE_GAIN gain14 = start_gain14 + gain_delta14 * i;
        const CSAMPLE_GAIN gain15 = start_gain15 + gain_delta15 * i;
        const CSAMPLE_GAIN gain16 = start_gain16 + gain_delta16 * i;
        const CSAMPLE_GAIN gain17 = start_gain17 + gain_delta17 * i;
        const CSAMPLE_GAIN gain18 = start_gain18 + gain_delta18 * i;
        const CSAMPLE_GAIN gain19 = start_gain19 + gain_delta19 * i;
        const CSAMPLE_GAIN gain20 = start_gain20 + gain_delta20 * i;
        const CSAMPLE_GAIN gain21 = start_gain21 + gain_delta21 * i;
        const CSAMPLE_GAIN gain22 = start_gain22 + gain_delta22 * i;
        const CSAMPLE_GAIN gain23 = start_gain23 + gain_delta23 * i;
        const CSAMPLE_GAIN gain24 = start_gain24 + gain_delta24 * i;
        const CSAMPLE_GAIN gain25 = start_gain25 + gain_delta25 * i;
        const CSAMPLE_GAIN gain26 = start_gain26 + gain_delta26 * i;
        const CSAMPLE_GAIN gain27 = start_gain27 + gain_delta27 * i;
        const CSAMPLE_GAIN gain28 = start_gain28 + gain_delta28 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1 +
                       pSrc2[i * 2] * gain2 +
                       pSrc3[i * 2] * gain3 +
                       pSrc4[i * 2] * gain4 +
                       pSrc5[i * 2] * gain5 +
                       pSrc6[i * 2] * gain6 +
                       pSrc7[i * 2] * gain7 +
                       pSrc8[i * 2] * gain8 +
                       pSrc9[i * 2] * gain9 +
                       pSrc10[i * 2] * gain10 +
                       pSrc11[i * 2] * gain11 +
                       pSrc12[i * 2] * gain12 +
                       pSrc13[i * 2] * gain13 +
                       pSrc14[i * 2] * gain14 +
                       pSrc15[i * 2] * gain15 +
                       pSrc16[i * 2] * gain16 +
                       pSrc17[i * 2] * gain17 +
                       pSrc18[i * 2] * gain18 +
                       pSrc19[i * 2] * gain19 +
                       pSrc20[i * 2] * gain20 +
                       pSrc21[i * 2] * gain21 +
                       pSrc22[i * 2] * gain22 +
                       pSrc23[i * 2] * gain23 +
                       pSrc24[i * 2] * gain24 +
                       pSrc25[i * 2] * gain25 +
                       pSrc26[i * 2] * gain26 +
                       pSrc27[i * 2] * gain27 +
                       pSrc28[i * 2] * gain28;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1 +
                           pSrc2[i * 2 + 1] * gain2 +
                           pSrc3[i * 2 + 1] * gain3 +
                           pSrc4[i * 2 + 1] * gain4 +
                           pSrc5[i * 2 + 1] * gain5 +
                           pSrc6[i * 2 + 1] * gain6 +
                           pSrc7[i * 2 + 1] * gain7 +
                           pSrc8[i * 2 + 1] * gain8 +
                           pSrc9[i * 2 + 1] * gain9 +
                           pSrc10[i * 2 + 1] * gain10 +
                           pSrc11[i * 2 + 1] * gain11 +
                           pSrc12[i * 2 + 1] * gain12 +
                           pSrc13[i * 2 + 1] * gain13 +
                           pSrc14[i * 2 + 1] * gain14 +
                           pSrc15[i * 2 + 1] * gain15 +
                           pSrc16[i * 2 + 1] * gain16 +
                           pSrc17[i * 2 + 1] * gain17 +
                           pSrc18[i * 2 + 1] * gain18 +
                           pSrc19[i * 2 + 1] * gain19 +
                           pSrc20[i * 2 + 1] * gain20 +
                           pSrc21[i * 2 + 1] * gain21 +
                           pSrc22[i * 2 + 1] * gain22 +
                           pSrc23[i * 2 + 1] * gain23 +
                           pSrc24[i * 2 + 1] * gain24 +
                           pSrc25[i * 2 + 1] * gain25 +
                           pSrc26[i * 2 + 1] * gain26 +
                           pSrc27[i * 2 + 1] * gain27 +
                           pSrc28[i * 2 + 1] * gain28;
    }
}
static inline void copy30WithGain(CSAMPLE* M_RESTRICT pDest,
                                  const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                  const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                  const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                  const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3,
                                  const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4,
                                  const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5,
                                  const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6,
                                  const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7,
                                  const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8,
                                  const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9,
                                  const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10,
                                  const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11,
                                  const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12,
                                  const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13,
                                  const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14,
                                  const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15,
                                  const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16,
                                  const CSAMPLE* M_RESTRICT pSrc17, CSAMPLE_GAIN gain17,
                                  const CSAMPLE* M_RESTRICT pSrc18, CSAMPLE_GAIN gain18,
                                  const CSAMPLE* M_RESTRICT pSrc19, CSAMPLE_GAIN gain19,
                                  const CSAMPLE* M_RESTRICT pSrc20, CSAMPLE_GAIN gain20,
                                  const CSAMPLE* M_RESTRICT pSrc21, CSAMPLE_GAIN gain21,
                                  const CSAMPLE* M_RESTRICT pSrc22, CSAMPLE_GAIN gain22,
                                  const CSAMPLE* M_RESTRICT pSrc23, CSAMPLE_GAIN gain23,
                                  const CSAMPLE* M_RESTRICT pSrc24, CSAMPLE_GAIN gain24,
                                  const CSAMPLE* M_RESTRICT pSrc25, CSAMPLE_GAIN gain25,
                                  const CSAMPLE* M_RESTRICT pSrc26, CSAMPLE_GAIN gain26,
                                  const CSAMPLE* M_RESTRICT pSrc27, CSAMPLE_GAIN gain27,
                                  const CSAMPLE* M_RESTRICT pSrc28, CSAMPLE_GAIN gain28,
                                  const CSAMPLE* M_RESTRICT pSrc29, CSAMPLE_GAIN gain29,
                                  int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy29WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain2 == CSAMPLE_GAIN_ZERO) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain3 == CSAMPLE_GAIN_ZERO) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain4 == CSAMPLE_GAIN_ZERO) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain5 == CSAMPLE_GAIN_ZERO) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain6 == CSAMPLE_GAIN_ZERO) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain7 == CSAMPLE_GAIN_ZERO) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain8 == CSAMPLE_GAIN_ZERO) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain9 == CSAMPLE_GAIN_ZERO) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain10 == CSAMPLE_GAIN_ZERO) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain11 == CSAMPLE_GAIN_ZERO) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain12 == CSAMPLE_GAIN_ZERO) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain13 == CSAMPLE_GAIN_ZERO) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain14 == CSAMPLE_GAIN_ZERO) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain15 == CSAMPLE_GAIN_ZERO) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain16 == CSAMPLE_GAIN_ZERO) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain17 == CSAMPLE_GAIN_ZERO) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain18 == CSAMPLE_GAIN_ZERO) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain19 == CSAMPLE_GAIN_ZERO) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain20 == CSAMPLE_GAIN_ZERO) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain21 == CSAMPLE_GAIN_ZERO) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain22 == CSAMPLE_GAIN_ZERO) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain23 == CSAMPLE_GAIN_ZERO) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain24 == CSAMPLE_GAIN_ZERO) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain25 == CSAMPLE_GAIN_ZERO) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain26 == CSAMPLE_GAIN_ZERO) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain27 == CSAMPLE_GAIN_ZERO) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain28 == CSAMPLE_GAIN_ZERO) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc29, gain29, iNumSamples);
        return;
    }
    if (gain29 == CSAMPLE_GAIN_ZERO) {
        copy29WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
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
static inline void copy30WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                         const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                         const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                         const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                         const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3in, CSAMPLE_GAIN gain3out,
                                         const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4in, CSAMPLE_GAIN gain4out,
                                         const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5in, CSAMPLE_GAIN gain5out,
                                         const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6in, CSAMPLE_GAIN gain6out,
                                         const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7in, CSAMPLE_GAIN gain7out,
                                         const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8in, CSAMPLE_GAIN gain8out,
                                         const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9in, CSAMPLE_GAIN gain9out,
                                         const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10in, CSAMPLE_GAIN gain10out,
                                         const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11in, CSAMPLE_GAIN gain11out,
                                         const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12in, CSAMPLE_GAIN gain12out,
                                         const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13in, CSAMPLE_GAIN gain13out,
                                         const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14in, CSAMPLE_GAIN gain14out,
                                         const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15in, CSAMPLE_GAIN gain15out,
                                         const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16in, CSAMPLE_GAIN gain16out,
                                         const CSAMPLE* M_RESTRICT pSrc17, CSAMPLE_GAIN gain17in, CSAMPLE_GAIN gain17out,
                                         const CSAMPLE* M_RESTRICT pSrc18, CSAMPLE_GAIN gain18in, CSAMPLE_GAIN gain18out,
                                         const CSAMPLE* M_RESTRICT pSrc19, CSAMPLE_GAIN gain19in, CSAMPLE_GAIN gain19out,
                                         const CSAMPLE* M_RESTRICT pSrc20, CSAMPLE_GAIN gain20in, CSAMPLE_GAIN gain20out,
                                         const CSAMPLE* M_RESTRICT pSrc21, CSAMPLE_GAIN gain21in, CSAMPLE_GAIN gain21out,
                                         const CSAMPLE* M_RESTRICT pSrc22, CSAMPLE_GAIN gain22in, CSAMPLE_GAIN gain22out,
                                         const CSAMPLE* M_RESTRICT pSrc23, CSAMPLE_GAIN gain23in, CSAMPLE_GAIN gain23out,
                                         const CSAMPLE* M_RESTRICT pSrc24, CSAMPLE_GAIN gain24in, CSAMPLE_GAIN gain24out,
                                         const CSAMPLE* M_RESTRICT pSrc25, CSAMPLE_GAIN gain25in, CSAMPLE_GAIN gain25out,
                                         const CSAMPLE* M_RESTRICT pSrc26, CSAMPLE_GAIN gain26in, CSAMPLE_GAIN gain26out,
                                         const CSAMPLE* M_RESTRICT pSrc27, CSAMPLE_GAIN gain27in, CSAMPLE_GAIN gain27out,
                                         const CSAMPLE* M_RESTRICT pSrc28, CSAMPLE_GAIN gain28in, CSAMPLE_GAIN gain28out,
                                         const CSAMPLE* M_RESTRICT pSrc29, CSAMPLE_GAIN gain29in, CSAMPLE_GAIN gain29out,
                                         int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy29WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain2in == CSAMPLE_GAIN_ZERO && gain2out == CSAMPLE_GAIN_ZERO) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain3in == CSAMPLE_GAIN_ZERO && gain3out == CSAMPLE_GAIN_ZERO) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain4in == CSAMPLE_GAIN_ZERO && gain4out == CSAMPLE_GAIN_ZERO) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain5in == CSAMPLE_GAIN_ZERO && gain5out == CSAMPLE_GAIN_ZERO) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain6in == CSAMPLE_GAIN_ZERO && gain6out == CSAMPLE_GAIN_ZERO) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain7in == CSAMPLE_GAIN_ZERO && gain7out == CSAMPLE_GAIN_ZERO) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain8in == CSAMPLE_GAIN_ZERO && gain8out == CSAMPLE_GAIN_ZERO) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain9in == CSAMPLE_GAIN_ZERO && gain9out == CSAMPLE_GAIN_ZERO) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain10in == CSAMPLE_GAIN_ZERO && gain10out == CSAMPLE_GAIN_ZERO) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain11in == CSAMPLE_GAIN_ZERO && gain11out == CSAMPLE_GAIN_ZERO) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain12in == CSAMPLE_GAIN_ZERO && gain12out == CSAMPLE_GAIN_ZERO) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain13in == CSAMPLE_GAIN_ZERO && gain13out == CSAMPLE_GAIN_ZERO) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain14in == CSAMPLE_GAIN_ZERO && gain14out == CSAMPLE_GAIN_ZERO) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain15in == CSAMPLE_GAIN_ZERO && gain15out == CSAMPLE_GAIN_ZERO) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain16in == CSAMPLE_GAIN_ZERO && gain16out == CSAMPLE_GAIN_ZERO) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain17in == CSAMPLE_GAIN_ZERO && gain17out == CSAMPLE_GAIN_ZERO) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain18in == CSAMPLE_GAIN_ZERO && gain18out == CSAMPLE_GAIN_ZERO) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain19in == CSAMPLE_GAIN_ZERO && gain19out == CSAMPLE_GAIN_ZERO) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain20in == CSAMPLE_GAIN_ZERO && gain20out == CSAMPLE_GAIN_ZERO) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain21in == CSAMPLE_GAIN_ZERO && gain21out == CSAMPLE_GAIN_ZERO) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain22in == CSAMPLE_GAIN_ZERO && gain22out == CSAMPLE_GAIN_ZERO) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain23in == CSAMPLE_GAIN_ZERO && gain23out == CSAMPLE_GAIN_ZERO) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain24in == CSAMPLE_GAIN_ZERO && gain24out == CSAMPLE_GAIN_ZERO) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain25in == CSAMPLE_GAIN_ZERO && gain25out == CSAMPLE_GAIN_ZERO) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain26in == CSAMPLE_GAIN_ZERO && gain26out == CSAMPLE_GAIN_ZERO) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain27in == CSAMPLE_GAIN_ZERO && gain27out == CSAMPLE_GAIN_ZERO) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain28in == CSAMPLE_GAIN_ZERO && gain28out == CSAMPLE_GAIN_ZERO) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    if (gain29in == CSAMPLE_GAIN_ZERO && gain29out == CSAMPLE_GAIN_ZERO) {
        copy29WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    const CSAMPLE_GAIN gain_delta2 = (gain2out - gain2in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain2 = gain2in + gain_delta2;
    const CSAMPLE_GAIN gain_delta3 = (gain3out - gain3in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain3 = gain3in + gain_delta3;
    const CSAMPLE_GAIN gain_delta4 = (gain4out - gain4in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain4 = gain4in + gain_delta4;
    const CSAMPLE_GAIN gain_delta5 = (gain5out - gain5in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain5 = gain5in + gain_delta5;
    const CSAMPLE_GAIN gain_delta6 = (gain6out - gain6in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain6 = gain6in + gain_delta6;
    const CSAMPLE_GAIN gain_delta7 = (gain7out - gain7in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain7 = gain7in + gain_delta7;
    const CSAMPLE_GAIN gain_delta8 = (gain8out - gain8in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain8 = gain8in + gain_delta8;
    const CSAMPLE_GAIN gain_delta9 = (gain9out - gain9in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain9 = gain9in + gain_delta9;
    const CSAMPLE_GAIN gain_delta10 = (gain10out - gain10in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain10 = gain10in + gain_delta10;
    const CSAMPLE_GAIN gain_delta11 = (gain11out - gain11in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain11 = gain11in + gain_delta11;
    const CSAMPLE_GAIN gain_delta12 = (gain12out - gain12in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain12 = gain12in + gain_delta12;
    const CSAMPLE_GAIN gain_delta13 = (gain13out - gain13in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain13 = gain13in + gain_delta13;
    const CSAMPLE_GAIN gain_delta14 = (gain14out - gain14in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain14 = gain14in + gain_delta14;
    const CSAMPLE_GAIN gain_delta15 = (gain15out - gain15in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain15 = gain15in + gain_delta15;
    const CSAMPLE_GAIN gain_delta16 = (gain16out - gain16in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain16 = gain16in + gain_delta16;
    const CSAMPLE_GAIN gain_delta17 = (gain17out - gain17in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain17 = gain17in + gain_delta17;
    const CSAMPLE_GAIN gain_delta18 = (gain18out - gain18in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain18 = gain18in + gain_delta18;
    const CSAMPLE_GAIN gain_delta19 = (gain19out - gain19in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain19 = gain19in + gain_delta19;
    const CSAMPLE_GAIN gain_delta20 = (gain20out - gain20in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain20 = gain20in + gain_delta20;
    const CSAMPLE_GAIN gain_delta21 = (gain21out - gain21in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain21 = gain21in + gain_delta21;
    const CSAMPLE_GAIN gain_delta22 = (gain22out - gain22in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain22 = gain22in + gain_delta22;
    const CSAMPLE_GAIN gain_delta23 = (gain23out - gain23in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain23 = gain23in + gain_delta23;
    const CSAMPLE_GAIN gain_delta24 = (gain24out - gain24in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain24 = gain24in + gain_delta24;
    const CSAMPLE_GAIN gain_delta25 = (gain25out - gain25in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain25 = gain25in + gain_delta25;
    const CSAMPLE_GAIN gain_delta26 = (gain26out - gain26in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain26 = gain26in + gain_delta26;
    const CSAMPLE_GAIN gain_delta27 = (gain27out - gain27in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain27 = gain27in + gain_delta27;
    const CSAMPLE_GAIN gain_delta28 = (gain28out - gain28in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain28 = gain28in + gain_delta28;
    const CSAMPLE_GAIN gain_delta29 = (gain29out - gain29in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain29 = gain29in + gain_delta29;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        const CSAMPLE_GAIN gain2 = start_gain2 + gain_delta2 * i;
        const CSAMPLE_GAIN gain3 = start_gain3 + gain_delta3 * i;
        const CSAMPLE_GAIN gain4 = start_gain4 + gain_delta4 * i;
        const CSAMPLE_GAIN gain5 = start_gain5 + gain_delta5 * i;
        const CSAMPLE_GAIN gain6 = start_gain6 + gain_delta6 * i;
        const CSAMPLE_GAIN gain7 = start_gain7 + gain_delta7 * i;
        const CSAMPLE_GAIN gain8 = start_gain8 + gain_delta8 * i;
        const CSAMPLE_GAIN gain9 = start_gain9 + gain_delta9 * i;
        const CSAMPLE_GAIN gain10 = start_gain10 + gain_delta10 * i;
        const CSAMPLE_GAIN gain11 = start_gain11 + gain_delta11 * i;
        const CSAMPLE_GAIN gain12 = start_gain12 + gain_delta12 * i;
        const CSAMPLE_GAIN gain13 = start_gain13 + gain_delta13 * i;
        const CSAMPLE_GAIN gain14 = start_gain14 + gain_delta14 * i;
        const CSAMPLE_GAIN gain15 = start_gain15 + gain_delta15 * i;
        const CSAMPLE_GAIN gain16 = start_gain16 + gain_delta16 * i;
        const CSAMPLE_GAIN gain17 = start_gain17 + gain_delta17 * i;
        const CSAMPLE_GAIN gain18 = start_gain18 + gain_delta18 * i;
        const CSAMPLE_GAIN gain19 = start_gain19 + gain_delta19 * i;
        const CSAMPLE_GAIN gain20 = start_gain20 + gain_delta20 * i;
        const CSAMPLE_GAIN gain21 = start_gain21 + gain_delta21 * i;
        const CSAMPLE_GAIN gain22 = start_gain22 + gain_delta22 * i;
        const CSAMPLE_GAIN gain23 = start_gain23 + gain_delta23 * i;
        const CSAMPLE_GAIN gain24 = start_gain24 + gain_delta24 * i;
        const CSAMPLE_GAIN gain25 = start_gain25 + gain_delta25 * i;
        const CSAMPLE_GAIN gain26 = start_gain26 + gain_delta26 * i;
        const CSAMPLE_GAIN gain27 = start_gain27 + gain_delta27 * i;
        const CSAMPLE_GAIN gain28 = start_gain28 + gain_delta28 * i;
        const CSAMPLE_GAIN gain29 = start_gain29 + gain_delta29 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1 +
                       pSrc2[i * 2] * gain2 +
                       pSrc3[i * 2] * gain3 +
                       pSrc4[i * 2] * gain4 +
                       pSrc5[i * 2] * gain5 +
                       pSrc6[i * 2] * gain6 +
                       pSrc7[i * 2] * gain7 +
                       pSrc8[i * 2] * gain8 +
                       pSrc9[i * 2] * gain9 +
                       pSrc10[i * 2] * gain10 +
                       pSrc11[i * 2] * gain11 +
                       pSrc12[i * 2] * gain12 +
                       pSrc13[i * 2] * gain13 +
                       pSrc14[i * 2] * gain14 +
                       pSrc15[i * 2] * gain15 +
                       pSrc16[i * 2] * gain16 +
                       pSrc17[i * 2] * gain17 +
                       pSrc18[i * 2] * gain18 +
                       pSrc19[i * 2] * gain19 +
                       pSrc20[i * 2] * gain20 +
                       pSrc21[i * 2] * gain21 +
                       pSrc22[i * 2] * gain22 +
                       pSrc23[i * 2] * gain23 +
                       pSrc24[i * 2] * gain24 +
                       pSrc25[i * 2] * gain25 +
                       pSrc26[i * 2] * gain26 +
                       pSrc27[i * 2] * gain27 +
                       pSrc28[i * 2] * gain28 +
                       pSrc29[i * 2] * gain29;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1 +
                           pSrc2[i * 2 + 1] * gain2 +
                           pSrc3[i * 2 + 1] * gain3 +
                           pSrc4[i * 2 + 1] * gain4 +
                           pSrc5[i * 2 + 1] * gain5 +
                           pSrc6[i * 2 + 1] * gain6 +
                           pSrc7[i * 2 + 1] * gain7 +
                           pSrc8[i * 2 + 1] * gain8 +
                           pSrc9[i * 2 + 1] * gain9 +
                           pSrc10[i * 2 + 1] * gain10 +
                           pSrc11[i * 2 + 1] * gain11 +
                           pSrc12[i * 2 + 1] * gain12 +
                           pSrc13[i * 2 + 1] * gain13 +
                           pSrc14[i * 2 + 1] * gain14 +
                           pSrc15[i * 2 + 1] * gain15 +
                           pSrc16[i * 2 + 1] * gain16 +
                           pSrc17[i * 2 + 1] * gain17 +
                           pSrc18[i * 2 + 1] * gain18 +
                           pSrc19[i * 2 + 1] * gain19 +
                           pSrc20[i * 2 + 1] * gain20 +
                           pSrc21[i * 2 + 1] * gain21 +
                           pSrc22[i * 2 + 1] * gain22 +
                           pSrc23[i * 2 + 1] * gain23 +
                           pSrc24[i * 2 + 1] * gain24 +
                           pSrc25[i * 2 + 1] * gain25 +
                           pSrc26[i * 2 + 1] * gain26 +
                           pSrc27[i * 2 + 1] * gain27 +
                           pSrc28[i * 2 + 1] * gain28 +
                           pSrc29[i * 2 + 1] * gain29;
    }
}
static inline void copy31WithGain(CSAMPLE* M_RESTRICT pDest,
                                  const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                  const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                  const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                  const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3,
                                  const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4,
                                  const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5,
                                  const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6,
                                  const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7,
                                  const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8,
                                  const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9,
                                  const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10,
                                  const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11,
                                  const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12,
                                  const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13,
                                  const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14,
                                  const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15,
                                  const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16,
                                  const CSAMPLE* M_RESTRICT pSrc17, CSAMPLE_GAIN gain17,
                                  const CSAMPLE* M_RESTRICT pSrc18, CSAMPLE_GAIN gain18,
                                  const CSAMPLE* M_RESTRICT pSrc19, CSAMPLE_GAIN gain19,
                                  const CSAMPLE* M_RESTRICT pSrc20, CSAMPLE_GAIN gain20,
                                  const CSAMPLE* M_RESTRICT pSrc21, CSAMPLE_GAIN gain21,
                                  const CSAMPLE* M_RESTRICT pSrc22, CSAMPLE_GAIN gain22,
                                  const CSAMPLE* M_RESTRICT pSrc23, CSAMPLE_GAIN gain23,
                                  const CSAMPLE* M_RESTRICT pSrc24, CSAMPLE_GAIN gain24,
                                  const CSAMPLE* M_RESTRICT pSrc25, CSAMPLE_GAIN gain25,
                                  const CSAMPLE* M_RESTRICT pSrc26, CSAMPLE_GAIN gain26,
                                  const CSAMPLE* M_RESTRICT pSrc27, CSAMPLE_GAIN gain27,
                                  const CSAMPLE* M_RESTRICT pSrc28, CSAMPLE_GAIN gain28,
                                  const CSAMPLE* M_RESTRICT pSrc29, CSAMPLE_GAIN gain29,
                                  const CSAMPLE* M_RESTRICT pSrc30, CSAMPLE_GAIN gain30,
                                  int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain2 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain3 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain4 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain5 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain6 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain7 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain8 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain9 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain10 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain11 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain12 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain13 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain14 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain15 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain16 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain17 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain18 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain19 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain20 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain21 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain22 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain23 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain24 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain25 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain26 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain27 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain28 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain29 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc30, gain30, iNumSamples);
        return;
    }
    if (gain30 == CSAMPLE_GAIN_ZERO) {
        copy30WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
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
static inline void copy31WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                         const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                         const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                         const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                         const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3in, CSAMPLE_GAIN gain3out,
                                         const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4in, CSAMPLE_GAIN gain4out,
                                         const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5in, CSAMPLE_GAIN gain5out,
                                         const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6in, CSAMPLE_GAIN gain6out,
                                         const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7in, CSAMPLE_GAIN gain7out,
                                         const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8in, CSAMPLE_GAIN gain8out,
                                         const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9in, CSAMPLE_GAIN gain9out,
                                         const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10in, CSAMPLE_GAIN gain10out,
                                         const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11in, CSAMPLE_GAIN gain11out,
                                         const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12in, CSAMPLE_GAIN gain12out,
                                         const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13in, CSAMPLE_GAIN gain13out,
                                         const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14in, CSAMPLE_GAIN gain14out,
                                         const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15in, CSAMPLE_GAIN gain15out,
                                         const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16in, CSAMPLE_GAIN gain16out,
                                         const CSAMPLE* M_RESTRICT pSrc17, CSAMPLE_GAIN gain17in, CSAMPLE_GAIN gain17out,
                                         const CSAMPLE* M_RESTRICT pSrc18, CSAMPLE_GAIN gain18in, CSAMPLE_GAIN gain18out,
                                         const CSAMPLE* M_RESTRICT pSrc19, CSAMPLE_GAIN gain19in, CSAMPLE_GAIN gain19out,
                                         const CSAMPLE* M_RESTRICT pSrc20, CSAMPLE_GAIN gain20in, CSAMPLE_GAIN gain20out,
                                         const CSAMPLE* M_RESTRICT pSrc21, CSAMPLE_GAIN gain21in, CSAMPLE_GAIN gain21out,
                                         const CSAMPLE* M_RESTRICT pSrc22, CSAMPLE_GAIN gain22in, CSAMPLE_GAIN gain22out,
                                         const CSAMPLE* M_RESTRICT pSrc23, CSAMPLE_GAIN gain23in, CSAMPLE_GAIN gain23out,
                                         const CSAMPLE* M_RESTRICT pSrc24, CSAMPLE_GAIN gain24in, CSAMPLE_GAIN gain24out,
                                         const CSAMPLE* M_RESTRICT pSrc25, CSAMPLE_GAIN gain25in, CSAMPLE_GAIN gain25out,
                                         const CSAMPLE* M_RESTRICT pSrc26, CSAMPLE_GAIN gain26in, CSAMPLE_GAIN gain26out,
                                         const CSAMPLE* M_RESTRICT pSrc27, CSAMPLE_GAIN gain27in, CSAMPLE_GAIN gain27out,
                                         const CSAMPLE* M_RESTRICT pSrc28, CSAMPLE_GAIN gain28in, CSAMPLE_GAIN gain28out,
                                         const CSAMPLE* M_RESTRICT pSrc29, CSAMPLE_GAIN gain29in, CSAMPLE_GAIN gain29out,
                                         const CSAMPLE* M_RESTRICT pSrc30, CSAMPLE_GAIN gain30in, CSAMPLE_GAIN gain30out,
                                         int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain2in == CSAMPLE_GAIN_ZERO && gain2out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain3in == CSAMPLE_GAIN_ZERO && gain3out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain4in == CSAMPLE_GAIN_ZERO && gain4out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain5in == CSAMPLE_GAIN_ZERO && gain5out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain6in == CSAMPLE_GAIN_ZERO && gain6out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain7in == CSAMPLE_GAIN_ZERO && gain7out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain8in == CSAMPLE_GAIN_ZERO && gain8out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain9in == CSAMPLE_GAIN_ZERO && gain9out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain10in == CSAMPLE_GAIN_ZERO && gain10out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain11in == CSAMPLE_GAIN_ZERO && gain11out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain12in == CSAMPLE_GAIN_ZERO && gain12out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain13in == CSAMPLE_GAIN_ZERO && gain13out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain14in == CSAMPLE_GAIN_ZERO && gain14out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain15in == CSAMPLE_GAIN_ZERO && gain15out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain16in == CSAMPLE_GAIN_ZERO && gain16out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain17in == CSAMPLE_GAIN_ZERO && gain17out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain18in == CSAMPLE_GAIN_ZERO && gain18out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain19in == CSAMPLE_GAIN_ZERO && gain19out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain20in == CSAMPLE_GAIN_ZERO && gain20out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain21in == CSAMPLE_GAIN_ZERO && gain21out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain22in == CSAMPLE_GAIN_ZERO && gain22out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain23in == CSAMPLE_GAIN_ZERO && gain23out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain24in == CSAMPLE_GAIN_ZERO && gain24out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain25in == CSAMPLE_GAIN_ZERO && gain25out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain26in == CSAMPLE_GAIN_ZERO && gain26out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain27in == CSAMPLE_GAIN_ZERO && gain27out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain28in == CSAMPLE_GAIN_ZERO && gain28out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain29in == CSAMPLE_GAIN_ZERO && gain29out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    if (gain30in == CSAMPLE_GAIN_ZERO && gain30out == CSAMPLE_GAIN_ZERO) {
        copy30WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    const CSAMPLE_GAIN gain_delta2 = (gain2out - gain2in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain2 = gain2in + gain_delta2;
    const CSAMPLE_GAIN gain_delta3 = (gain3out - gain3in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain3 = gain3in + gain_delta3;
    const CSAMPLE_GAIN gain_delta4 = (gain4out - gain4in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain4 = gain4in + gain_delta4;
    const CSAMPLE_GAIN gain_delta5 = (gain5out - gain5in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain5 = gain5in + gain_delta5;
    const CSAMPLE_GAIN gain_delta6 = (gain6out - gain6in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain6 = gain6in + gain_delta6;
    const CSAMPLE_GAIN gain_delta7 = (gain7out - gain7in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain7 = gain7in + gain_delta7;
    const CSAMPLE_GAIN gain_delta8 = (gain8out - gain8in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain8 = gain8in + gain_delta8;
    const CSAMPLE_GAIN gain_delta9 = (gain9out - gain9in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain9 = gain9in + gain_delta9;
    const CSAMPLE_GAIN gain_delta10 = (gain10out - gain10in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain10 = gain10in + gain_delta10;
    const CSAMPLE_GAIN gain_delta11 = (gain11out - gain11in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain11 = gain11in + gain_delta11;
    const CSAMPLE_GAIN gain_delta12 = (gain12out - gain12in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain12 = gain12in + gain_delta12;
    const CSAMPLE_GAIN gain_delta13 = (gain13out - gain13in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain13 = gain13in + gain_delta13;
    const CSAMPLE_GAIN gain_delta14 = (gain14out - gain14in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain14 = gain14in + gain_delta14;
    const CSAMPLE_GAIN gain_delta15 = (gain15out - gain15in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain15 = gain15in + gain_delta15;
    const CSAMPLE_GAIN gain_delta16 = (gain16out - gain16in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain16 = gain16in + gain_delta16;
    const CSAMPLE_GAIN gain_delta17 = (gain17out - gain17in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain17 = gain17in + gain_delta17;
    const CSAMPLE_GAIN gain_delta18 = (gain18out - gain18in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain18 = gain18in + gain_delta18;
    const CSAMPLE_GAIN gain_delta19 = (gain19out - gain19in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain19 = gain19in + gain_delta19;
    const CSAMPLE_GAIN gain_delta20 = (gain20out - gain20in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain20 = gain20in + gain_delta20;
    const CSAMPLE_GAIN gain_delta21 = (gain21out - gain21in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain21 = gain21in + gain_delta21;
    const CSAMPLE_GAIN gain_delta22 = (gain22out - gain22in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain22 = gain22in + gain_delta22;
    const CSAMPLE_GAIN gain_delta23 = (gain23out - gain23in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain23 = gain23in + gain_delta23;
    const CSAMPLE_GAIN gain_delta24 = (gain24out - gain24in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain24 = gain24in + gain_delta24;
    const CSAMPLE_GAIN gain_delta25 = (gain25out - gain25in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain25 = gain25in + gain_delta25;
    const CSAMPLE_GAIN gain_delta26 = (gain26out - gain26in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain26 = gain26in + gain_delta26;
    const CSAMPLE_GAIN gain_delta27 = (gain27out - gain27in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain27 = gain27in + gain_delta27;
    const CSAMPLE_GAIN gain_delta28 = (gain28out - gain28in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain28 = gain28in + gain_delta28;
    const CSAMPLE_GAIN gain_delta29 = (gain29out - gain29in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain29 = gain29in + gain_delta29;
    const CSAMPLE_GAIN gain_delta30 = (gain30out - gain30in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain30 = gain30in + gain_delta30;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        const CSAMPLE_GAIN gain2 = start_gain2 + gain_delta2 * i;
        const CSAMPLE_GAIN gain3 = start_gain3 + gain_delta3 * i;
        const CSAMPLE_GAIN gain4 = start_gain4 + gain_delta4 * i;
        const CSAMPLE_GAIN gain5 = start_gain5 + gain_delta5 * i;
        const CSAMPLE_GAIN gain6 = start_gain6 + gain_delta6 * i;
        const CSAMPLE_GAIN gain7 = start_gain7 + gain_delta7 * i;
        const CSAMPLE_GAIN gain8 = start_gain8 + gain_delta8 * i;
        const CSAMPLE_GAIN gain9 = start_gain9 + gain_delta9 * i;
        const CSAMPLE_GAIN gain10 = start_gain10 + gain_delta10 * i;
        const CSAMPLE_GAIN gain11 = start_gain11 + gain_delta11 * i;
        const CSAMPLE_GAIN gain12 = start_gain12 + gain_delta12 * i;
        const CSAMPLE_GAIN gain13 = start_gain13 + gain_delta13 * i;
        const CSAMPLE_GAIN gain14 = start_gain14 + gain_delta14 * i;
        const CSAMPLE_GAIN gain15 = start_gain15 + gain_delta15 * i;
        const CSAMPLE_GAIN gain16 = start_gain16 + gain_delta16 * i;
        const CSAMPLE_GAIN gain17 = start_gain17 + gain_delta17 * i;
        const CSAMPLE_GAIN gain18 = start_gain18 + gain_delta18 * i;
        const CSAMPLE_GAIN gain19 = start_gain19 + gain_delta19 * i;
        const CSAMPLE_GAIN gain20 = start_gain20 + gain_delta20 * i;
        const CSAMPLE_GAIN gain21 = start_gain21 + gain_delta21 * i;
        const CSAMPLE_GAIN gain22 = start_gain22 + gain_delta22 * i;
        const CSAMPLE_GAIN gain23 = start_gain23 + gain_delta23 * i;
        const CSAMPLE_GAIN gain24 = start_gain24 + gain_delta24 * i;
        const CSAMPLE_GAIN gain25 = start_gain25 + gain_delta25 * i;
        const CSAMPLE_GAIN gain26 = start_gain26 + gain_delta26 * i;
        const CSAMPLE_GAIN gain27 = start_gain27 + gain_delta27 * i;
        const CSAMPLE_GAIN gain28 = start_gain28 + gain_delta28 * i;
        const CSAMPLE_GAIN gain29 = start_gain29 + gain_delta29 * i;
        const CSAMPLE_GAIN gain30 = start_gain30 + gain_delta30 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1 +
                       pSrc2[i * 2] * gain2 +
                       pSrc3[i * 2] * gain3 +
                       pSrc4[i * 2] * gain4 +
                       pSrc5[i * 2] * gain5 +
                       pSrc6[i * 2] * gain6 +
                       pSrc7[i * 2] * gain7 +
                       pSrc8[i * 2] * gain8 +
                       pSrc9[i * 2] * gain9 +
                       pSrc10[i * 2] * gain10 +
                       pSrc11[i * 2] * gain11 +
                       pSrc12[i * 2] * gain12 +
                       pSrc13[i * 2] * gain13 +
                       pSrc14[i * 2] * gain14 +
                       pSrc15[i * 2] * gain15 +
                       pSrc16[i * 2] * gain16 +
                       pSrc17[i * 2] * gain17 +
                       pSrc18[i * 2] * gain18 +
                       pSrc19[i * 2] * gain19 +
                       pSrc20[i * 2] * gain20 +
                       pSrc21[i * 2] * gain21 +
                       pSrc22[i * 2] * gain22 +
                       pSrc23[i * 2] * gain23 +
                       pSrc24[i * 2] * gain24 +
                       pSrc25[i * 2] * gain25 +
                       pSrc26[i * 2] * gain26 +
                       pSrc27[i * 2] * gain27 +
                       pSrc28[i * 2] * gain28 +
                       pSrc29[i * 2] * gain29 +
                       pSrc30[i * 2] * gain30;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1 +
                           pSrc2[i * 2 + 1] * gain2 +
                           pSrc3[i * 2 + 1] * gain3 +
                           pSrc4[i * 2 + 1] * gain4 +
                           pSrc5[i * 2 + 1] * gain5 +
                           pSrc6[i * 2 + 1] * gain6 +
                           pSrc7[i * 2 + 1] * gain7 +
                           pSrc8[i * 2 + 1] * gain8 +
                           pSrc9[i * 2 + 1] * gain9 +
                           pSrc10[i * 2 + 1] * gain10 +
                           pSrc11[i * 2 + 1] * gain11 +
                           pSrc12[i * 2 + 1] * gain12 +
                           pSrc13[i * 2 + 1] * gain13 +
                           pSrc14[i * 2 + 1] * gain14 +
                           pSrc15[i * 2 + 1] * gain15 +
                           pSrc16[i * 2 + 1] * gain16 +
                           pSrc17[i * 2 + 1] * gain17 +
                           pSrc18[i * 2 + 1] * gain18 +
                           pSrc19[i * 2 + 1] * gain19 +
                           pSrc20[i * 2 + 1] * gain20 +
                           pSrc21[i * 2 + 1] * gain21 +
                           pSrc22[i * 2 + 1] * gain22 +
                           pSrc23[i * 2 + 1] * gain23 +
                           pSrc24[i * 2 + 1] * gain24 +
                           pSrc25[i * 2 + 1] * gain25 +
                           pSrc26[i * 2 + 1] * gain26 +
                           pSrc27[i * 2 + 1] * gain27 +
                           pSrc28[i * 2 + 1] * gain28 +
                           pSrc29[i * 2 + 1] * gain29 +
                           pSrc30[i * 2 + 1] * gain30;
    }
}
static inline void copy32WithGain(CSAMPLE* M_RESTRICT pDest,
                                  const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                  const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                  const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                  const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3,
                                  const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4,
                                  const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5,
                                  const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6,
                                  const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7,
                                  const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8,
                                  const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9,
                                  const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10,
                                  const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11,
                                  const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12,
                                  const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13,
                                  const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14,
                                  const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15,
                                  const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16,
                                  const CSAMPLE* M_RESTRICT pSrc17, CSAMPLE_GAIN gain17,
                                  const CSAMPLE* M_RESTRICT pSrc18, CSAMPLE_GAIN gain18,
                                  const CSAMPLE* M_RESTRICT pSrc19, CSAMPLE_GAIN gain19,
                                  const CSAMPLE* M_RESTRICT pSrc20, CSAMPLE_GAIN gain20,
                                  const CSAMPLE* M_RESTRICT pSrc21, CSAMPLE_GAIN gain21,
                                  const CSAMPLE* M_RESTRICT pSrc22, CSAMPLE_GAIN gain22,
                                  const CSAMPLE* M_RESTRICT pSrc23, CSAMPLE_GAIN gain23,
                                  const CSAMPLE* M_RESTRICT pSrc24, CSAMPLE_GAIN gain24,
                                  const CSAMPLE* M_RESTRICT pSrc25, CSAMPLE_GAIN gain25,
                                  const CSAMPLE* M_RESTRICT pSrc26, CSAMPLE_GAIN gain26,
                                  const CSAMPLE* M_RESTRICT pSrc27, CSAMPLE_GAIN gain27,
                                  const CSAMPLE* M_RESTRICT pSrc28, CSAMPLE_GAIN gain28,
                                  const CSAMPLE* M_RESTRICT pSrc29, CSAMPLE_GAIN gain29,
                                  const CSAMPLE* M_RESTRICT pSrc30, CSAMPLE_GAIN gain30,
                                  const CSAMPLE* M_RESTRICT pSrc31, CSAMPLE_GAIN gain31,
                                  int iNumSamples) {
    if (gain0 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain1 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain2 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain3 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain4 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain5 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain6 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain7 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain8 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain9 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain10 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain11 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain12 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain13 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain14 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain15 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain16 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain17 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain18 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain19 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain20 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain21 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain22 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain23 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain24 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain25 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain26 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain27 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain28 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc29, gain29, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain29 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc30, gain30, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain30 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc31, gain31, iNumSamples);
        return;
    }
    if (gain31 == CSAMPLE_GAIN_ZERO) {
        copy31WithGain(pDest, pSrc0, gain0, pSrc1, gain1, pSrc2, gain2, pSrc3, gain3, pSrc4, gain4, pSrc5, gain5, pSrc6, gain6, pSrc7, gain7, pSrc8, gain8, pSrc9, gain9, pSrc10, gain10, pSrc11, gain11, pSrc12, gain12, pSrc13, gain13, pSrc14, gain14, pSrc15, gain15, pSrc16, gain16, pSrc17, gain17, pSrc18, gain18, pSrc19, gain19, pSrc20, gain20, pSrc21, gain21, pSrc22, gain22, pSrc23, gain23, pSrc24, gain24, pSrc25, gain25, pSrc26, gain26, pSrc27, gain27, pSrc28, gain28, pSrc29, gain29, pSrc30, gain30, iNumSamples);
        return;
    }
    // note: LOOP VECTORIZED.
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
static inline void copy32WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                         const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                         const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                         const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                         const CSAMPLE* M_RESTRICT pSrc3, CSAMPLE_GAIN gain3in, CSAMPLE_GAIN gain3out,
                                         const CSAMPLE* M_RESTRICT pSrc4, CSAMPLE_GAIN gain4in, CSAMPLE_GAIN gain4out,
                                         const CSAMPLE* M_RESTRICT pSrc5, CSAMPLE_GAIN gain5in, CSAMPLE_GAIN gain5out,
                                         const CSAMPLE* M_RESTRICT pSrc6, CSAMPLE_GAIN gain6in, CSAMPLE_GAIN gain6out,
                                         const CSAMPLE* M_RESTRICT pSrc7, CSAMPLE_GAIN gain7in, CSAMPLE_GAIN gain7out,
                                         const CSAMPLE* M_RESTRICT pSrc8, CSAMPLE_GAIN gain8in, CSAMPLE_GAIN gain8out,
                                         const CSAMPLE* M_RESTRICT pSrc9, CSAMPLE_GAIN gain9in, CSAMPLE_GAIN gain9out,
                                         const CSAMPLE* M_RESTRICT pSrc10, CSAMPLE_GAIN gain10in, CSAMPLE_GAIN gain10out,
                                         const CSAMPLE* M_RESTRICT pSrc11, CSAMPLE_GAIN gain11in, CSAMPLE_GAIN gain11out,
                                         const CSAMPLE* M_RESTRICT pSrc12, CSAMPLE_GAIN gain12in, CSAMPLE_GAIN gain12out,
                                         const CSAMPLE* M_RESTRICT pSrc13, CSAMPLE_GAIN gain13in, CSAMPLE_GAIN gain13out,
                                         const CSAMPLE* M_RESTRICT pSrc14, CSAMPLE_GAIN gain14in, CSAMPLE_GAIN gain14out,
                                         const CSAMPLE* M_RESTRICT pSrc15, CSAMPLE_GAIN gain15in, CSAMPLE_GAIN gain15out,
                                         const CSAMPLE* M_RESTRICT pSrc16, CSAMPLE_GAIN gain16in, CSAMPLE_GAIN gain16out,
                                         const CSAMPLE* M_RESTRICT pSrc17, CSAMPLE_GAIN gain17in, CSAMPLE_GAIN gain17out,
                                         const CSAMPLE* M_RESTRICT pSrc18, CSAMPLE_GAIN gain18in, CSAMPLE_GAIN gain18out,
                                         const CSAMPLE* M_RESTRICT pSrc19, CSAMPLE_GAIN gain19in, CSAMPLE_GAIN gain19out,
                                         const CSAMPLE* M_RESTRICT pSrc20, CSAMPLE_GAIN gain20in, CSAMPLE_GAIN gain20out,
                                         const CSAMPLE* M_RESTRICT pSrc21, CSAMPLE_GAIN gain21in, CSAMPLE_GAIN gain21out,
                                         const CSAMPLE* M_RESTRICT pSrc22, CSAMPLE_GAIN gain22in, CSAMPLE_GAIN gain22out,
                                         const CSAMPLE* M_RESTRICT pSrc23, CSAMPLE_GAIN gain23in, CSAMPLE_GAIN gain23out,
                                         const CSAMPLE* M_RESTRICT pSrc24, CSAMPLE_GAIN gain24in, CSAMPLE_GAIN gain24out,
                                         const CSAMPLE* M_RESTRICT pSrc25, CSAMPLE_GAIN gain25in, CSAMPLE_GAIN gain25out,
                                         const CSAMPLE* M_RESTRICT pSrc26, CSAMPLE_GAIN gain26in, CSAMPLE_GAIN gain26out,
                                         const CSAMPLE* M_RESTRICT pSrc27, CSAMPLE_GAIN gain27in, CSAMPLE_GAIN gain27out,
                                         const CSAMPLE* M_RESTRICT pSrc28, CSAMPLE_GAIN gain28in, CSAMPLE_GAIN gain28out,
                                         const CSAMPLE* M_RESTRICT pSrc29, CSAMPLE_GAIN gain29in, CSAMPLE_GAIN gain29out,
                                         const CSAMPLE* M_RESTRICT pSrc30, CSAMPLE_GAIN gain30in, CSAMPLE_GAIN gain30out,
                                         const CSAMPLE* M_RESTRICT pSrc31, CSAMPLE_GAIN gain31in, CSAMPLE_GAIN gain31out,
                                         int iNumSamples) {
    if (gain0in == CSAMPLE_GAIN_ZERO && gain0out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain1in == CSAMPLE_GAIN_ZERO && gain1out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain2in == CSAMPLE_GAIN_ZERO && gain2out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain3in == CSAMPLE_GAIN_ZERO && gain3out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain4in == CSAMPLE_GAIN_ZERO && gain4out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain5in == CSAMPLE_GAIN_ZERO && gain5out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain6in == CSAMPLE_GAIN_ZERO && gain6out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain7in == CSAMPLE_GAIN_ZERO && gain7out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain8in == CSAMPLE_GAIN_ZERO && gain8out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain9in == CSAMPLE_GAIN_ZERO && gain9out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain10in == CSAMPLE_GAIN_ZERO && gain10out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain11in == CSAMPLE_GAIN_ZERO && gain11out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain12in == CSAMPLE_GAIN_ZERO && gain12out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain13in == CSAMPLE_GAIN_ZERO && gain13out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain14in == CSAMPLE_GAIN_ZERO && gain14out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain15in == CSAMPLE_GAIN_ZERO && gain15out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain16in == CSAMPLE_GAIN_ZERO && gain16out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain17in == CSAMPLE_GAIN_ZERO && gain17out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain18in == CSAMPLE_GAIN_ZERO && gain18out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain19in == CSAMPLE_GAIN_ZERO && gain19out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain20in == CSAMPLE_GAIN_ZERO && gain20out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain21in == CSAMPLE_GAIN_ZERO && gain21out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain22in == CSAMPLE_GAIN_ZERO && gain22out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain23in == CSAMPLE_GAIN_ZERO && gain23out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain24in == CSAMPLE_GAIN_ZERO && gain24out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain25in == CSAMPLE_GAIN_ZERO && gain25out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain26in == CSAMPLE_GAIN_ZERO && gain26out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain27in == CSAMPLE_GAIN_ZERO && gain27out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain28in == CSAMPLE_GAIN_ZERO && gain28out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain29in == CSAMPLE_GAIN_ZERO && gain29out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc30, gain30in, gain30out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain30in == CSAMPLE_GAIN_ZERO && gain30out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc31, gain31in, gain31out, iNumSamples);
        return;
    }
    if (gain31in == CSAMPLE_GAIN_ZERO && gain31out == CSAMPLE_GAIN_ZERO) {
        copy31WithRampingGain(pDest, pSrc0, gain0in, gain0out, pSrc1, gain1in, gain1out, pSrc2, gain2in, gain2out, pSrc3, gain3in, gain3out, pSrc4, gain4in, gain4out, pSrc5, gain5in, gain5out, pSrc6, gain6in, gain6out, pSrc7, gain7in, gain7out, pSrc8, gain8in, gain8out, pSrc9, gain9in, gain9out, pSrc10, gain10in, gain10out, pSrc11, gain11in, gain11out, pSrc12, gain12in, gain12out, pSrc13, gain13in, gain13out, pSrc14, gain14in, gain14out, pSrc15, gain15in, gain15out, pSrc16, gain16in, gain16out, pSrc17, gain17in, gain17out, pSrc18, gain18in, gain18out, pSrc19, gain19in, gain19out, pSrc20, gain20in, gain20out, pSrc21, gain21in, gain21out, pSrc22, gain22in, gain22out, pSrc23, gain23in, gain23out, pSrc24, gain24in, gain24out, pSrc25, gain25in, gain25out, pSrc26, gain26in, gain26out, pSrc27, gain27in, gain27out, pSrc28, gain28in, gain28out, pSrc29, gain29in, gain29out, pSrc30, gain30in, gain30out, iNumSamples);
        return;
    }
    const CSAMPLE_GAIN gain_delta0 = (gain0out - gain0in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain0 = gain0in + gain_delta0;
    const CSAMPLE_GAIN gain_delta1 = (gain1out - gain1in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain1 = gain1in + gain_delta1;
    const CSAMPLE_GAIN gain_delta2 = (gain2out - gain2in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain2 = gain2in + gain_delta2;
    const CSAMPLE_GAIN gain_delta3 = (gain3out - gain3in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain3 = gain3in + gain_delta3;
    const CSAMPLE_GAIN gain_delta4 = (gain4out - gain4in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain4 = gain4in + gain_delta4;
    const CSAMPLE_GAIN gain_delta5 = (gain5out - gain5in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain5 = gain5in + gain_delta5;
    const CSAMPLE_GAIN gain_delta6 = (gain6out - gain6in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain6 = gain6in + gain_delta6;
    const CSAMPLE_GAIN gain_delta7 = (gain7out - gain7in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain7 = gain7in + gain_delta7;
    const CSAMPLE_GAIN gain_delta8 = (gain8out - gain8in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain8 = gain8in + gain_delta8;
    const CSAMPLE_GAIN gain_delta9 = (gain9out - gain9in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain9 = gain9in + gain_delta9;
    const CSAMPLE_GAIN gain_delta10 = (gain10out - gain10in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain10 = gain10in + gain_delta10;
    const CSAMPLE_GAIN gain_delta11 = (gain11out - gain11in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain11 = gain11in + gain_delta11;
    const CSAMPLE_GAIN gain_delta12 = (gain12out - gain12in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain12 = gain12in + gain_delta12;
    const CSAMPLE_GAIN gain_delta13 = (gain13out - gain13in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain13 = gain13in + gain_delta13;
    const CSAMPLE_GAIN gain_delta14 = (gain14out - gain14in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain14 = gain14in + gain_delta14;
    const CSAMPLE_GAIN gain_delta15 = (gain15out - gain15in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain15 = gain15in + gain_delta15;
    const CSAMPLE_GAIN gain_delta16 = (gain16out - gain16in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain16 = gain16in + gain_delta16;
    const CSAMPLE_GAIN gain_delta17 = (gain17out - gain17in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain17 = gain17in + gain_delta17;
    const CSAMPLE_GAIN gain_delta18 = (gain18out - gain18in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain18 = gain18in + gain_delta18;
    const CSAMPLE_GAIN gain_delta19 = (gain19out - gain19in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain19 = gain19in + gain_delta19;
    const CSAMPLE_GAIN gain_delta20 = (gain20out - gain20in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain20 = gain20in + gain_delta20;
    const CSAMPLE_GAIN gain_delta21 = (gain21out - gain21in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain21 = gain21in + gain_delta21;
    const CSAMPLE_GAIN gain_delta22 = (gain22out - gain22in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain22 = gain22in + gain_delta22;
    const CSAMPLE_GAIN gain_delta23 = (gain23out - gain23in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain23 = gain23in + gain_delta23;
    const CSAMPLE_GAIN gain_delta24 = (gain24out - gain24in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain24 = gain24in + gain_delta24;
    const CSAMPLE_GAIN gain_delta25 = (gain25out - gain25in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain25 = gain25in + gain_delta25;
    const CSAMPLE_GAIN gain_delta26 = (gain26out - gain26in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain26 = gain26in + gain_delta26;
    const CSAMPLE_GAIN gain_delta27 = (gain27out - gain27in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain27 = gain27in + gain_delta27;
    const CSAMPLE_GAIN gain_delta28 = (gain28out - gain28in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain28 = gain28in + gain_delta28;
    const CSAMPLE_GAIN gain_delta29 = (gain29out - gain29in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain29 = gain29in + gain_delta29;
    const CSAMPLE_GAIN gain_delta30 = (gain30out - gain30in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain30 = gain30in + gain_delta30;
    const CSAMPLE_GAIN gain_delta31 = (gain31out - gain31in) / (iNumSamples / 2);
    const CSAMPLE_GAIN start_gain31 = gain31in + gain_delta31;
    // note: LOOP VECTORIZED.
    for (int i = 0; i < iNumSamples / 2; ++i) {
        const CSAMPLE_GAIN gain0 = start_gain0 + gain_delta0 * i;
        const CSAMPLE_GAIN gain1 = start_gain1 + gain_delta1 * i;
        const CSAMPLE_GAIN gain2 = start_gain2 + gain_delta2 * i;
        const CSAMPLE_GAIN gain3 = start_gain3 + gain_delta3 * i;
        const CSAMPLE_GAIN gain4 = start_gain4 + gain_delta4 * i;
        const CSAMPLE_GAIN gain5 = start_gain5 + gain_delta5 * i;
        const CSAMPLE_GAIN gain6 = start_gain6 + gain_delta6 * i;
        const CSAMPLE_GAIN gain7 = start_gain7 + gain_delta7 * i;
        const CSAMPLE_GAIN gain8 = start_gain8 + gain_delta8 * i;
        const CSAMPLE_GAIN gain9 = start_gain9 + gain_delta9 * i;
        const CSAMPLE_GAIN gain10 = start_gain10 + gain_delta10 * i;
        const CSAMPLE_GAIN gain11 = start_gain11 + gain_delta11 * i;
        const CSAMPLE_GAIN gain12 = start_gain12 + gain_delta12 * i;
        const CSAMPLE_GAIN gain13 = start_gain13 + gain_delta13 * i;
        const CSAMPLE_GAIN gain14 = start_gain14 + gain_delta14 * i;
        const CSAMPLE_GAIN gain15 = start_gain15 + gain_delta15 * i;
        const CSAMPLE_GAIN gain16 = start_gain16 + gain_delta16 * i;
        const CSAMPLE_GAIN gain17 = start_gain17 + gain_delta17 * i;
        const CSAMPLE_GAIN gain18 = start_gain18 + gain_delta18 * i;
        const CSAMPLE_GAIN gain19 = start_gain19 + gain_delta19 * i;
        const CSAMPLE_GAIN gain20 = start_gain20 + gain_delta20 * i;
        const CSAMPLE_GAIN gain21 = start_gain21 + gain_delta21 * i;
        const CSAMPLE_GAIN gain22 = start_gain22 + gain_delta22 * i;
        const CSAMPLE_GAIN gain23 = start_gain23 + gain_delta23 * i;
        const CSAMPLE_GAIN gain24 = start_gain24 + gain_delta24 * i;
        const CSAMPLE_GAIN gain25 = start_gain25 + gain_delta25 * i;
        const CSAMPLE_GAIN gain26 = start_gain26 + gain_delta26 * i;
        const CSAMPLE_GAIN gain27 = start_gain27 + gain_delta27 * i;
        const CSAMPLE_GAIN gain28 = start_gain28 + gain_delta28 * i;
        const CSAMPLE_GAIN gain29 = start_gain29 + gain_delta29 * i;
        const CSAMPLE_GAIN gain30 = start_gain30 + gain_delta30 * i;
        const CSAMPLE_GAIN gain31 = start_gain31 + gain_delta31 * i;
        pDest[i * 2] = pSrc0[i * 2] * gain0 +
                       pSrc1[i * 2] * gain1 +
                       pSrc2[i * 2] * gain2 +
                       pSrc3[i * 2] * gain3 +
                       pSrc4[i * 2] * gain4 +
                       pSrc5[i * 2] * gain5 +
                       pSrc6[i * 2] * gain6 +
                       pSrc7[i * 2] * gain7 +
                       pSrc8[i * 2] * gain8 +
                       pSrc9[i * 2] * gain9 +
                       pSrc10[i * 2] * gain10 +
                       pSrc11[i * 2] * gain11 +
                       pSrc12[i * 2] * gain12 +
                       pSrc13[i * 2] * gain13 +
                       pSrc14[i * 2] * gain14 +
                       pSrc15[i * 2] * gain15 +
                       pSrc16[i * 2] * gain16 +
                       pSrc17[i * 2] * gain17 +
                       pSrc18[i * 2] * gain18 +
                       pSrc19[i * 2] * gain19 +
                       pSrc20[i * 2] * gain20 +
                       pSrc21[i * 2] * gain21 +
                       pSrc22[i * 2] * gain22 +
                       pSrc23[i * 2] * gain23 +
                       pSrc24[i * 2] * gain24 +
                       pSrc25[i * 2] * gain25 +
                       pSrc26[i * 2] * gain26 +
                       pSrc27[i * 2] * gain27 +
                       pSrc28[i * 2] * gain28 +
                       pSrc29[i * 2] * gain29 +
                       pSrc30[i * 2] * gain30 +
                       pSrc31[i * 2] * gain31;
        pDest[i * 2 + 1] = pSrc0[i * 2 + 1] * gain0 +
                           pSrc1[i * 2 + 1] * gain1 +
                           pSrc2[i * 2 + 1] * gain2 +
                           pSrc3[i * 2 + 1] * gain3 +
                           pSrc4[i * 2 + 1] * gain4 +
                           pSrc5[i * 2 + 1] * gain5 +
                           pSrc6[i * 2 + 1] * gain6 +
                           pSrc7[i * 2 + 1] * gain7 +
                           pSrc8[i * 2 + 1] * gain8 +
                           pSrc9[i * 2 + 1] * gain9 +
                           pSrc10[i * 2 + 1] * gain10 +
                           pSrc11[i * 2 + 1] * gain11 +
                           pSrc12[i * 2 + 1] * gain12 +
                           pSrc13[i * 2 + 1] * gain13 +
                           pSrc14[i * 2 + 1] * gain14 +
                           pSrc15[i * 2 + 1] * gain15 +
                           pSrc16[i * 2 + 1] * gain16 +
                           pSrc17[i * 2 + 1] * gain17 +
                           pSrc18[i * 2 + 1] * gain18 +
                           pSrc19[i * 2 + 1] * gain19 +
                           pSrc20[i * 2 + 1] * gain20 +
                           pSrc21[i * 2 + 1] * gain21 +
                           pSrc22[i * 2 + 1] * gain22 +
                           pSrc23[i * 2 + 1] * gain23 +
                           pSrc24[i * 2 + 1] * gain24 +
                           pSrc25[i * 2 + 1] * gain25 +
                           pSrc26[i * 2 + 1] * gain26 +
                           pSrc27[i * 2 + 1] * gain27 +
                           pSrc28[i * 2 + 1] * gain28 +
                           pSrc29[i * 2 + 1] * gain29 +
                           pSrc30[i * 2 + 1] * gain30 +
                           pSrc31[i * 2 + 1] * gain31;
    }
}
