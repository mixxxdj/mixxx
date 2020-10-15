#ifndef ENGINEFILTERIIR_H
#define ENGINEFILTERIIR_H

#define MIXXX
#include <cstdio>
#include <fidlib.h>

#include "engine/engineobject.h"
#include "util/sample.h"

// set to 1 to print some analysis data using qDebug()
// It prints the resulting delay after 50 % of impulse have passed
// and the gain and phase shift at some sample frequencies
// You may also use the app fiview for analysis
#define IIR_ANALYSIS 0

enum IIRPass {
    IIR_LP,
    IIR_BP,
    IIR_HP,
    IIR_LPMO,
    IIR_HPMO,
    IIR_LP2,
    IIR_HP2,
};


class EngineFilterIIRBase : public EngineObjectConstIn {
  public:
    virtual void assumeSettled() = 0;
};


// length of the 3rd argument to fid_design_coef
#define FIDSPEC_LENGTH 40

template<unsigned int SIZE, enum IIRPass PASS>
class EngineFilterIIR : public EngineFilterIIRBase {
  public:
    EngineFilterIIR()
            : m_doRamping(false),
              m_doStart(false),
              m_startFromDry(false) {
        memset(m_coef, 0, sizeof(m_coef));
        pauseFilter();
    }

    virtual ~EngineFilterIIR() {};

    // this can be called continuously for Filters that have own ramping
    // or need no fade when disabling
    void pauseFilter() {
        if (!m_doStart) {
            pauseFilterInner();
        }
    }

    void setStartFromDry(bool val) {
        m_startFromDry = val;
    }

    // this is can be used instead off a final process() call before pause
    // It fades to dry or 0 according to the m_startFromDry parameter
    // it is an alternative for using pauseFillter() calls
    void processAndPauseFilter(
            const CSAMPLE* pIn,
            CSAMPLE* pOutput,
            int iBufferSize) {
        process(pIn, pOutput, iBufferSize);
        if (m_startFromDry) {
            SampleUtil::linearCrossfadeBuffersOut(
                    pOutput, // fade out filtered
                    pIn,     // fade in dry
                    iBufferSize);
        } else {
            SampleUtil::applyRampingGain(
                    pOutput, 1.0, 0, // fade out filtered
                    iBufferSize);
        }
        pauseFilterInner();
    }

    void initBuffers() {
        // Copy the current buffers into the old buffers
        memcpy(m_oldBuf1, m_buf1, sizeof(m_buf1));
        memcpy(m_oldBuf2, m_buf2, sizeof(m_buf2));
        // Set the current buffers to 0
        memset(m_buf1, 0, sizeof(m_buf1));
        memset(m_buf2, 0, sizeof(m_buf2));
        m_doRamping = true;
    }

    void setCoefs(const char* spec, double sampleRate,
            double freq0, double freq1 = 0, int adj = 0) {

        char spec_d[FIDSPEC_LENGTH];
        if (strlen(spec) < sizeof(spec_d)) {
            // Copy to dynamic-ish memory to prevent fidlib API breakage.
            strcpy(spec_d, spec);

            // Copy the old coefficients into m_oldCoef
            memcpy(m_oldCoef, m_coef, sizeof(m_coef));

            m_coef[0] = fid_design_coef(m_coef + 1, SIZE,
                    spec_d, sampleRate, freq0, freq1, adj);

            initBuffers();

#if(IIR_ANALYSIS)
            char* desc;
            FidFilter* filt = fid_design(spec_d, sampleRate, freq0, freq1, adj, &desc);
            int delay = fid_calc_delay(filt);
            qDebug() << QString().fromLatin1(desc) << "delay:" << delay;
            double resp0, phase0;
            resp0 = fid_response_pha(filt, freq0 / sampleRate, &phase0);
            qDebug() << "freq0:" << freq0 << resp0 << phase0;
            if (freq1) {
                double resp1, phase1;
                resp1 = fid_response_pha(filt, freq1 / sampleRate, &phase1);
                qDebug() << "freq1:" << freq1 << resp1 << phase1;
            }
            double resp2, phase2;
            resp2 = fid_response_pha(filt, freq0 / sampleRate / 2, &phase2);
            qDebug() << "freq2:" << freq0 / 2 << resp2 << phase0;
            double resp3, phase3;
            resp3 = fid_response_pha(filt, freq0 / sampleRate * 2, &phase3);
            qDebug() << "freq3:" << freq0 * 2 << resp3 << phase0;
            double resp4, phase4;
            resp4 = fid_response_pha(filt, freq0 / sampleRate / 2.2, &phase4);
            qDebug() << "freq4:" << freq0 / 2.2 << resp2 << phase0;
            double resp5, phase5;
            resp5 = fid_response_pha(filt, freq0 / sampleRate * 2.2, &phase5);
            qDebug() << "freq5:" << freq0 * 2.2 << resp3 << phase0;
            free(filt);
#endif
        }
    }

    void setCoefs2(double sampleRate, int n_coef1,
            const char* spec1, double freq01, double freq11, int adj1,
            const char* spec2, double freq02, double freq12, int adj2) {
        char spec1_d[FIDSPEC_LENGTH];
        char spec2_d[FIDSPEC_LENGTH];
        if (strlen(spec1) < sizeof(spec1_d) &&
                strlen(spec2) < sizeof(spec2_d)) {
            // Copy to dynamic-ish memory to prevent fidlib API breakage.
            strcpy(spec1_d, spec1);
            strcpy(spec2_d, spec2);

            // Copy the old coefficients into m_oldCoef
            memcpy(m_oldCoef, m_coef, sizeof(m_coef));
            m_coef[0] = fid_design_coef(m_coef + 1, n_coef1,
                    spec1, sampleRate, freq01, freq11, adj1) *
                        fid_design_coef(m_coef + 1 + n_coef1, SIZE - n_coef1,
                    spec2, sampleRate, freq02, freq12, adj2);

            initBuffers();

#if(IIR_ANALYSIS)
            char* desc1;
            char* desc2;
            FidFilter* filt1 = fid_design(spec1, sampleRate, freq01, freq11, adj1, &desc1);
            FidFilter* filt2 = fid_design(spec2, sampleRate, freq02, freq12, adj2, &desc2);
            FidFilter* filt = fid_cat(1, filt1, filt2, NULL);
            int delay = fid_calc_delay(filt);
            qDebug() << QString().fromLatin1(desc1) << "X" << QString().fromLatin1(desc2) << "delay:" << delay;
            double resp0, phase0;
            resp0 = fid_response_pha(filt, freq01 / sampleRate, &phase0);
            qDebug() << "freq01:" << freq01 << resp0 << phase0;
            resp0 = fid_response_pha(filt, freq01 / sampleRate, &phase0);
            qDebug() << "freq02:" << freq02 << resp0 << phase0;
            if (freq11) {
                double resp1, phase1;
                resp1 = fid_response_pha(filt, freq11 / sampleRate, &phase1);
                qDebug() << "freq1:" << freq11 << resp1 << phase1;
            }
            if (freq12) {
                double resp1, phase1;
                resp1 = fid_response_pha(filt, freq12 / sampleRate, &phase1);
                qDebug() << "freq1:" << freq12 << resp1 << phase1;
            }
            double resp2, phase2;
            resp2 = fid_response_pha(filt, freq01 / sampleRate / 2, &phase2);
            qDebug() << "freq2:" << freq01 / 2 << resp2 << phase0;
            double resp3, phase3;
            resp3 = fid_response_pha(filt, freq01 / sampleRate * 2, &phase3);
            qDebug() << "freq3:" << freq01 * 2 << resp3 << phase0;
            double resp4, phase4;
            resp4 = fid_response_pha(filt, freq01 / sampleRate / 2.2, &phase4);
            qDebug() << "freq4:" << freq01 / 2.2 << resp2 << phase0;
            double resp5, phase5;
            resp5 = fid_response_pha(filt, freq01 / sampleRate * 2.2, &phase5);
            qDebug() << "freq5:" << freq01 * 2.2 << resp3 << phase0;
            free(filt);
#endif
        }
    }

    virtual void assumeSettled() {
        m_doRamping = false;
        m_doStart = false;
    }

    virtual void process(const CSAMPLE* pIn, CSAMPLE* pOutput,
                         const int iBufferSize) {
        if (!m_doRamping) {
            for (int i = 0; i < iBufferSize; i += 2) {
                pOutput[i] = static_cast<CSAMPLE>(processSample(m_coef, m_buf1, pIn[i]));
                pOutput[i + 1] = static_cast<CSAMPLE>(processSample(m_coef, m_buf2, pIn[i + 1]));
            }
        } else {
            double cross_mix = 0.0;
            double cross_inc = 4.0 / static_cast<double>(iBufferSize);
            for (int i = 0; i < iBufferSize; i += 2) {
                // Do a linear cross fade between the output of the old
                // Filter and the new filter.
                // The new filter is settled for Input = 0 and it sees
                // all frequencies of the rectangular start impulse.
                // Since the group delay, after which the start impulse
                // has passed is unknown here, we just what the half
                // iBufferSize until we use the samples of the new filter.
                // In one of the previous version we have faded the Input
                // of the new filter but it turns out that this produces
                // a gain drop due to the filter delay which is more
                // conspicuous than the settling noise.
                double old1;
                double old2;
                if (!m_doStart) {
                    // Process old filter, but only if we do not do a fresh start
                    old1 = static_cast<CSAMPLE>(processSample(m_oldCoef, m_oldBuf1, pIn[i]));
                    old2 = static_cast<CSAMPLE>(processSample(m_oldCoef, m_oldBuf2, pIn[i + 1]));
                } else {
                    if (m_startFromDry) {
                        old1 = pIn[i];
                        old2 = pIn[i + 1];
                    } else {
                        old1 = 0;
                        old2 = 0;
                    }
                }
                double new1 = static_cast<CSAMPLE>(processSample(m_coef, m_buf1, pIn[i]));
                double new2 = static_cast<CSAMPLE>(processSample(m_coef, m_buf2, pIn[i + 1]));

                if (i < iBufferSize / 2) {
                    pOutput[i] = static_cast<CSAMPLE>(old1);
                    pOutput[i + 1] = static_cast<CSAMPLE>(old2);
                } else {
                    pOutput[i] = static_cast<CSAMPLE>(new1 * cross_mix + old1 * (1.0 - cross_mix));
                    pOutput[i + 1] = static_cast<CSAMPLE>(
                            new2 * cross_mix + old2 * (1.0 - cross_mix));
                    cross_mix += cross_inc;
                }
            }
            m_doRamping = false;
            m_doStart = false;
        }
    }

  protected:
    inline double processSample(double* coef, double* buf, double val);
    inline void pauseFilterInner() {
        // Set the current buffers to 0
        memset(m_buf1, 0, sizeof(m_buf1));
        memset(m_buf2, 0, sizeof(m_buf2));
        m_doRamping = true;
        m_doStart = true;
    }

    double m_coef[SIZE + 1];
    // Old coefficients needed for ramping
    double m_oldCoef[SIZE + 1];

    // Channel 1 state
    double m_buf1[SIZE];
    // Old channel 1 buffer needed for ramping
    double m_oldBuf1[SIZE];

    // Channel 2 state
    double m_buf2[SIZE];
    // Old channel 2 buffer needed for ramping
    double m_oldBuf2[SIZE];

    // Flag set to true if ramping needs to be done
    bool m_doRamping;
    // Flag set to true if old filter is invalid
    bool m_doStart;
    // Flag set to true if this is a chained filter
    bool m_startFromDry;
};

template<>
inline double EngineFilterIIR<2, IIR_LP>::processSample(double* coef,
                                                        double* buf,
                                                        double val) {
    double tmp, fir, iir;
    tmp = buf[0]; buf[0] = buf[1];
    iir = val * coef[0];
    iir -= coef[1] * tmp; fir = tmp;
    iir -= coef[2] * buf[0]; fir += buf[0] + buf[0];
    fir += iir;
    buf[1] = iir; val = fir;
    return val;
}

template<>
inline double EngineFilterIIR<2, IIR_BP>::processSample(double* coef,
                                                        double* buf,
                                                        double val) {
    double tmp, fir, iir;
    tmp = buf[0]; buf[0] = buf[1];
    iir = val * coef[0];
    iir -= coef[1] * tmp; fir = -tmp;
    iir -= coef[2] * buf[0];
    fir += iir;
    buf[1] = iir; val = fir;
    return val;
}

template<>
inline double EngineFilterIIR<2, IIR_HP>::processSample(double* coef,
                                                        double* buf,
                                                        double val) {
    double tmp, fir, iir;
    tmp = buf[0]; buf[0] = buf[1];
    iir = val * coef[0];
    iir -= coef[1] * tmp; fir = tmp;
    iir -= coef[2] * buf[0]; fir += -buf[0] - buf[0];
    fir += iir;
    buf[1] = iir; val = fir;
    return val;
}

template<>
inline double EngineFilterIIR<4, IIR_LP>::processSample(double* coef,
                                                        double* buf,
                                                        double val) {
    double tmp, fir, iir;
    tmp = buf[0]; buf[0] = buf[1]; buf[1] = buf[2]; buf[2] = buf[3];
    iir = val * coef[0];
    iir -= coef[1] * tmp; fir = tmp;
    iir -= coef[2] * buf[0]; fir += buf[0] + buf[0];
    fir += iir;
    tmp = buf[1]; buf[1] = iir; val = fir;
    iir = val;
    iir -= coef[3] * tmp; fir = tmp;
    iir -= coef[4] * buf[2]; fir += buf[2] + buf[2];
    fir += iir;
    buf[3] = iir; val = fir;
    return val;
}

template<>
inline double EngineFilterIIR<8, IIR_BP>::processSample(double* coef,
                                                        double* buf,
                                                        double val) {
    double tmp, fir, iir;
    tmp = buf[0]; buf[0] = buf[1]; buf[1] = buf[2]; buf[2] = buf[3];
    buf[3] = buf[4]; buf[4] = buf[5]; buf[5] = buf[6]; buf[6] = buf[7];
    iir = val * coef[0];
    iir -= coef[1] * tmp; fir = tmp;
    iir -= coef[2] * buf[0]; fir += -buf[0] - buf[0];
    fir += iir;
    tmp = buf[1]; buf[1] = iir; val= fir;
    iir = val;
    iir -= coef[3] * tmp; fir = tmp;
    iir -= coef[4] * buf[2]; fir += -buf[2] - buf[2];
    fir += iir;
    tmp = buf[3]; buf[3] = iir; val= fir;
    iir = val;
    iir -= coef[5] * tmp; fir = tmp;
    iir -= coef[6] * buf[4]; fir += buf[4] + buf[4];
    fir += iir;
    tmp = buf[5]; buf[5] = iir; val= fir;
    iir = val;
    iir -= coef[7] * tmp; fir = tmp;
    iir -= coef[8] * buf[6]; fir += buf[6] + buf[6];
    fir += iir;
    buf[7] = iir; val = fir;
    return val;
}

template<>
inline double EngineFilterIIR<4, IIR_HP>::processSample(double* coef,
                                                        double* buf,
                                                        double val) {
    double tmp, fir, iir;
    tmp = buf[0]; buf[0] = buf[1]; buf[1] = buf[2]; buf[2] = buf[3];
    iir= val * coef[0];
    iir -= coef[1] * tmp; fir = tmp;
    iir -= coef[2] * buf[0]; fir += -buf[0] - buf[0];
    fir += iir;
    tmp = buf[1]; buf[1] = iir; val = fir;
    iir = val;
    iir -= coef[3] * tmp; fir = tmp;
    iir -= coef[4] * buf[2]; fir += -buf[2] - buf[2];
    fir += iir;
    buf[3] = iir; val = fir;
    return val;
}

template<>
inline double EngineFilterIIR<8, IIR_LP>::processSample(double* coef,
                                                        double* buf,
                                                        double val) {
    double tmp, fir, iir;
    tmp = buf[0]; buf[0] = buf[1]; buf[1] = buf[2]; buf[2] = buf[3];
    buf[3] = buf[4]; buf[4] = buf[5]; buf[5] = buf[6]; buf[6] = buf[7];
    iir = val * coef[0];
    iir -= coef[1] * tmp; fir = tmp;
    iir -= coef[2] * buf[0]; fir += buf[0] + buf[0];
    fir += iir;
    tmp = buf[1]; buf[1] = iir; val = fir;
    iir = val;
    iir -= coef[3] * tmp; fir = tmp;
    iir -= coef[4] * buf[2]; fir += buf[2] + buf[2];
    fir += iir;
    tmp = buf[3]; buf[3] = iir; val = fir;
    iir = val;
    iir -= coef[5] * tmp; fir = tmp;
    iir -= coef[6] * buf[4]; fir += buf[4] + buf[4];
    fir += iir;
    tmp = buf[5]; buf[5] = iir; val = fir;
    iir = val;
    iir -= coef[7] * tmp; fir = tmp;
    iir -= coef[8] * buf[6]; fir += buf[6] + buf[6];
    fir += iir;
    buf[7] = iir; val = fir;
    return val;
}

template<>
inline double EngineFilterIIR<16, IIR_BP>::processSample(double* coef,
                                                         double* buf,
                                                         double val) {
    double tmp, fir, iir;
    tmp = buf[0]; buf[0] = buf[1]; buf[1] = buf[2]; buf[2] = buf[3];
    buf[3] = buf[4]; buf[4] = buf[5]; buf[5] = buf[6]; buf[6] = buf[7];
    buf[7] = buf[8]; buf[8] = buf[9]; buf[9] = buf[10]; buf[10] = buf[11];
    buf[11] = buf[12]; buf[12] = buf[13]; buf[13] = buf[14]; buf[14] = buf[15];
    iir = val * coef[0];
    iir -= coef[1] * tmp; fir = tmp;
    iir -= coef[2] * buf[0]; fir += -buf[0] - buf[0];
    fir += iir;
    tmp = buf[1]; buf[1] = iir; val = fir;
    iir = val;
    iir -= coef[3] * tmp; fir = tmp;
    iir -= coef[4] * buf[2]; fir += -buf[2] - buf[2];
    fir += iir;
    tmp = buf[3]; buf[3] = iir; val = fir;
    iir = val;
    iir -= coef[5] * tmp; fir = tmp;
    iir -= coef[6] * buf[4]; fir += -buf[4] - buf[4];
    fir += iir;
    tmp = buf[5]; buf[5] = iir; val = fir;
    iir = val;
    iir -= coef[7] * tmp; fir = tmp;
    iir -= coef[8] * buf[6]; fir += -buf[6] - buf[6];
    fir += iir;
    tmp = buf[7]; buf[7]= iir; val= fir;
    iir = val;
    iir -= coef[9] * tmp; fir = tmp;
    iir -= coef[10] * buf[8]; fir += buf[8] + buf[8];
    fir += iir;
    tmp = buf[9]; buf[9] = iir; val = fir;
    iir = val;
    iir -= coef[11] * tmp; fir = tmp;
    iir -= coef[12] * buf[10]; fir += buf[10] + buf[10];
    fir += iir;
    tmp = buf[11]; buf[11] = iir; val = fir;
    iir = val;
    iir -= coef[13] * tmp; fir = tmp;
    iir -= coef[14] * buf[12]; fir += buf[12] + buf[12];
    fir += iir;
    tmp = buf[13]; buf[13] = iir; val = fir;
    iir = val;
    iir -= coef[15] * tmp; fir = tmp;
    iir -= coef[16] * buf[14]; fir += buf[14] + buf[14];
    fir += iir;
    buf[15] = iir; val = fir;
    return val;
}

template<>
inline double EngineFilterIIR<8, IIR_HP>::processSample(double* coef,
                                                        double* buf,
                                                        double val) {
    double tmp, fir, iir;
    tmp = buf[0]; buf[0] = buf[1]; buf[1] = buf[2]; buf[2] = buf[3];
    buf[3] = buf[4]; buf[4] = buf[5]; buf[5] = buf[6]; buf[6] = buf[7];
    iir = val * coef[0];
    iir -= coef[1] * tmp; fir = tmp;
    iir -= coef[2] * buf[0]; fir += -buf[0] - buf[0];
    fir += iir;
    tmp = buf[1]; buf[1] = iir; val = fir;
    iir = val;
    iir -= coef[3] * tmp; fir = tmp;
    iir -= coef[4] * buf[2]; fir += -buf[2] - buf[2];
    fir += iir;
    tmp = buf[3]; buf[3] = iir; val = fir;
    iir = val;
    iir -= coef[5] * tmp; fir = tmp;
    iir -= coef[6] * buf[4]; fir += -buf[4] - buf[4];
    fir += iir;
    tmp = buf[5]; buf[5] = iir; val = fir;
    iir = val;
    iir -= coef[7] * tmp; fir = tmp;
    iir -= coef[8] * buf[6]; fir += -buf[6] - buf[6];
    fir += iir;
    buf[7] = iir; val = fir;
    return val;
}

// IIR_LP and IIR_HP use the same processSample routine
template<>
inline double EngineFilterIIR<5, IIR_BP>::processSample(double* coef,
                                                        double* buf,
                                                        double val) {
    double tmp, fir, iir;
    tmp = buf[0]; buf[0] = buf[1];
    iir = val * coef[0];
    iir -= coef[1] * tmp; fir = coef[2] * tmp;
    iir -= coef[3] * buf[0]; fir += coef[4] * buf[0];
    fir += coef[5] * iir;
    buf[1] = iir; val = fir;
    return val;
}

template<>
inline double EngineFilterIIR<4, IIR_LPMO>::processSample(double* coef,
                                                        double* buf,
                                                        double val) {
   double tmp, fir, iir;
   tmp= buf[0]; buf[0] = buf[1]; buf[1] = buf[2]; buf[2] = buf[3];
   iir= val * coef[0];
   iir -= coef[1]*tmp; fir= tmp;
   fir += iir;
   tmp= buf[0]; buf[0]= iir; val= fir;
   iir= val;
   iir -= coef[2]*tmp; fir= tmp;
   fir += iir;
   tmp= buf[1]; buf[1]= iir; val= fir;
   iir= val;
   iir -= coef[3]*tmp; fir= tmp;
   fir += iir;
   tmp= buf[2]; buf[2]= iir; val= fir;
   iir= val;
   iir -= coef[4]*tmp; fir= tmp;
   fir += iir;
   buf[3]= iir; val= fir;
   return val;
}


template<>
inline double EngineFilterIIR<4, IIR_HPMO>::processSample(double* coef,
                                                        double* buf,
                                                        double val) {
   double tmp, fir, iir;
   tmp= buf[0]; buf[0] = buf[1]; buf[1] = buf[2]; buf[2] = buf[3];
   iir= val * coef[0];
   iir -= coef[1]*tmp; fir= -tmp;
   fir += iir;
   tmp= buf[0]; buf[0]= iir; val= fir;
   iir= val;
   iir -= coef[2]*tmp; fir= -tmp;
   fir += iir;
   tmp= buf[1]; buf[1]= iir; val= fir;
   iir= val;
   iir -= coef[3]*tmp; fir= -tmp;
   fir += iir;
   tmp= buf[2]; buf[2]= iir; val= fir;
   iir= val;
   iir -= coef[4]*tmp; fir= -tmp;
   fir += iir;
   buf[3]= iir; val= fir;
   return val;
}

template<>
inline double EngineFilterIIR<2, IIR_LP2>::processSample(double* coef,
                                                        double* buf,
                                                        double val) {
    double tmp, fir, iir;
    tmp = buf[0];
    iir = val * coef[0];
    iir -= coef[1] * tmp; fir = tmp;
    fir += iir;
    buf[0] = iir; val = fir;

    tmp = buf[1];
    iir = val;
    iir -= coef[2] * tmp; fir = tmp;
    fir += iir;
    buf[1] = iir; val = fir;

    return val;
}


template<>
inline double EngineFilterIIR<2, IIR_HP2>::processSample(double* coef,
                                                        double* buf,
                                                        double val) {
    double tmp, fir, iir;
    tmp = buf[0];
    iir = val * -coef[0]; // swap gain to be in phase with LP2
    iir -= coef[1] * tmp; fir = -tmp;
    fir += iir;
    buf[0] = iir; val = fir;

    tmp = buf[1];
    iir = val;
    iir -= coef[2] * tmp; fir = -tmp;
    fir += iir;
    buf[1] = iir; val = fir;

    return val;
}


#endif // ENGINEFILTERIIR_H
