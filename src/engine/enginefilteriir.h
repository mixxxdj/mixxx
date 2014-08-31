#ifndef ENGINEFILTERIIR_H
#define ENGINEFILTERIIR_H

#include <string.h>

#include "engine/engineobject.h"
#define MIXXX
#include <fidlib.h>

enum IIRPass {
    IIR_LP,
    IIR_BP,
    IIR_HP
};

// length of the 3rd argument to fid_design_coef
#define FIDSPEC_LENGTH 40

template<unsigned int SIZE, enum IIRPass PASS>
class EngineFilterIIR : public EngineObjectConstIn {
  public:
    EngineFilterIIR()
            : m_doRamping(false),
              m_doStart(false),
              m_startFromDry(false) {
        memset(m_coef, 0, sizeof(m_coef));
        pauseFilter();
    }

    virtual ~EngineFilterIIR() {};

    void pauseFilter() {
        if (!m_doStart) {
            // Set the current buffers to 0
            memset(m_buf1, 0, sizeof(m_buf1));
            memset(m_buf2, 0, sizeof(m_buf2));
            m_doRamping = true;
            m_doStart = true;
        }
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
        }
    }

    virtual void process(const CSAMPLE* pIn, CSAMPLE* pOutput,
                         const int iBufferSize) {
        if (!m_doRamping) {
            for (int i = 0; i < iBufferSize; i += 2) {
                pOutput[i] = processSample(m_coef, m_buf1, pIn[i]);
                pOutput[i+1] = processSample(m_coef, m_buf2, pIn[i + 1]);
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
                    // Process old filter only if we do not do a fresh start
                    old1 = processSample(m_oldCoef, m_oldBuf1, pIn[i]);
                    old2 = processSample(m_oldCoef, m_oldBuf2, pIn[i + 1]);
                } else {
                    if (m_startFromDry) {
                        old1 = pIn[i];
                        old2 = pIn[i + 1];
                    } else {
                        old1 = 0;
                        old2 = 0;
                    }
                }
                double new1 = processSample(m_coef, m_buf1, pIn[i]);
                double new2 = processSample(m_coef, m_buf2, pIn[i + 1]);

                if (i < iBufferSize / 2) {
                    pOutput[i] = old1;
                    pOutput[i + 1] = old2;
                } else {
                    pOutput[i] = new1 * cross_mix +
                                 old1 * (1.0 - cross_mix);
                    pOutput[i + 1] = new2  * cross_mix +
                                     old2 * (1.0 - cross_mix);
                    cross_mix += cross_inc;
                }
            }
            m_doRamping = false;
            m_doStart = false;
        }
    }

  protected:
    inline double processSample(double* coef, double* buf, register double val);

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
                                                        register double val) {
    register double tmp, fir, iir;
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
                                                        register double val) {
    register double tmp, fir, iir;
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
                                                        register double val) {
    register double tmp, fir, iir;
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
                                                        register double val) {
    register double tmp, fir, iir;
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
                                                        register double val) {
    register double tmp, fir, iir;
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
                                                        register double val) {
    register double tmp, fir, iir;
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
                                                        register double val) {
    register double tmp, fir, iir;
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
                                                         register double val) {
    register double tmp, fir, iir;
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
                                                        register double val) {
    register double tmp, fir, iir;
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
                                                        register double val) {
    register double tmp, fir, iir;
    tmp = buf[0]; buf[0] = buf[1];
    iir = val * coef[0];
    iir -= coef[1] * tmp; fir = coef[2] * tmp;
    iir -= coef[3] * buf[0]; fir += coef[4] * buf[0];
    fir += coef[5] * iir;
    buf[1] = iir; val = fir;
    return val;
}

#endif // ENGINEFILTERIIR_H
