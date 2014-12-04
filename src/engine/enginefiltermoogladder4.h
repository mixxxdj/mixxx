#ifndef ENGINEFILTERMOOGLADDER4_H
#define ENGINEFILTERMOOGLADDER4_H

// Filter based on the text "Non linear digital implementation of the moog ladder filter"
// by Antti Houvilainen
// This implementation is probably a more accurate digital representation of the original analogue filter.
// This is version 2 (revised 14/DEC/04), with improved amplitude/resonance scaling and frequency
// correction using a couple of polynomials,as suggested by Antti.

// Adopted from Csound code at http://www.kunstmusik.com/udo/cache/moogladder.udo
// Based on C Source from R. Lindner published at public domain
// http://musicdsp.org/showArchiveComment.php?ArchiveID=196

#include "engine/engineobject.h"

#include <memory.h>
#include <stdio.h>
#include "util/math.h"
#include "util/timer.h"
#include "sampleutil.h"

#include <QDebug>

// 'thermal voltage of a transistor'
// defines the strange of the non linearity
// 1.2 = drives the transistor in full range, giving a maximum Waveshaper effect
// big values disables the non linearity
static const float kVt = 1.2;
static const float kPi = 3.14159265358979323846;

enum MoogMode {
    LP,
    HP,
    LP_OVERS,
    HP_OVERS,
};

template<enum MoogMode MODE>
class EngineFilterMoogLadderBase : public EngineObjectConstIn {

  private:
    struct Buffer {
         CSAMPLE m_azt1;
         CSAMPLE m_azt2;
         CSAMPLE m_azt3;
         CSAMPLE m_azt4;
         CSAMPLE m_az5;
         CSAMPLE m_amf;
    };

  public:
    EngineFilterMoogLadderBase(unsigned int sampleRate, float cutoff, float resonance) {
        setParameter(sampleRate, cutoff, resonance);
        initBuffers();
    }

    virtual ~EngineFilterMoogLadderBase() {
    }

    void initBuffers() {
        memset(&m_buf, 0, sizeof(m_buf));
        m_buffersClear = true;
    }

    // cutoff  in Hz
    // resonance  range 0 ... 4 (4 = self resonance)
    void setParameter(int sampleRate, float cutoff, float resonance) {
        const float v2 = 2 + kVt;   // twice the 'thermal voltage of a transistor'

        float kfc = cutoff / sampleRate;
        float kf = kfc;
        if (MODE == LP_OVERS || MODE == HP_OVERS) {
            // m_inputSampeRate is half the actual filter sampling rate in oversampling mode
            kf = kfc / 2;
        }

        // frequency & amplitude correction
        float kfcr = 1.8730 * (kfc*kfc*kfc) + 0.4955 * (kfc*kfc) - 0.6490 * kfc + 0.9988;

        float x  = -2.0 * kPi * kfcr * kf; // input for taylor approximations
        float exp_out  = expf(x);
        m_k2vg = v2 * (1 - exp_out); // filter tuning

        // Resonance correction for self oscillation ~4
        m_kacr = resonance * (-3.9364 * (kfc*kfc) + 1.8409 * kfc + 0.9968);

        if (MODE == HP_OVERS || MODE == HP) {
            m_postGain = 1;
        } else {
            m_postGain = (1 + resonance / 4 * (1.1f + cutoff / sampleRate * 3.5f))
                    * (2 - (1.0f - resonance / 4) * (1.0f - resonance / 4));
        }

        // qDebug() << "setParameter" << m_cutoff << m_resonance;
    }

    // this is can be used instead off a final process() call before pause
    // It fades to dry or 0 according to the m_startFromDry parameter
    // it is an alternative for using pauseFillter() calls
    void processAndPauseFilter(const CSAMPLE* pIn, CSAMPLE* pOutput,
                       const int iBufferSize) {
        process(pIn, pOutput, iBufferSize);
        SampleUtil::copy2WithRampingGain(pOutput,
                pOutput, 1.0, 0,  // fade out filtered
                pIn, 0, 1.0,  // fade in dry
                iBufferSize);
        initBuffers();
    }

    virtual void process(const CSAMPLE* pIn, CSAMPLE* pOutput,
                         const int iBufferSize) {
        if (!m_buffersClear) {
            for (int i = 0; i < iBufferSize; i += 2) {
                pOutput[i] = processSample(pIn[i], &m_buf[0]);
                pOutput[i+1] = processSample(pIn[i+1], &m_buf[1]);
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
                double old1 = pIn[i];
                double old2 = pIn[i + 1];
                double new1 = processSample(pIn[i], &m_buf[0]);
                double new2 = processSample(pIn[i+1], &m_buf[1]);

                if (i < iBufferSize / 2) {
                    pOutput[i] = old1;
                    pOutput[i + 1] = old2;
                } else {
                    pOutput[i] = new1 * cross_mix + old1 * (1.0 - cross_mix);
                    pOutput[i + 1] = new2 * cross_mix
                            + old2 * (1.0 - cross_mix);
                    cross_mix += cross_inc;
                }
            }
            m_buffersClear = false;
        }
    }

    inline CSAMPLE processSample(float input, struct Buffer* pB) {

        const float v2 = 2 + kVt;   // twice the 'thermal voltage of a transistor'

        // cascade of 4 1st order sections
        float x1 = input - pB->m_amf * m_kacr;
        float az1 = pB->m_azt1 + m_k2vg * tanh_approx(x1 / v2);
        float at1 = m_k2vg * tanh_approx(az1 / v2);
        pB->m_azt1 = az1 - at1;
        float az2 = pB->m_azt2 + at1;
        float at2 = m_k2vg * tanh_approx(az2 / v2);
        pB->m_azt2 = az2 - at2;
        float az3 = pB->m_azt3 + at2;
        float at3 = m_k2vg * tanh_approx(az3 / v2);
        pB->m_azt3 = az3 - at3;
        float az4 = pB->m_azt4 + at3;
        float at4 = m_k2vg * tanh_approx(az4 / v2);
        pB->m_azt4 = az4 - at4;

        // Oversampling if requested
        if (MODE == LP_OVERS || MODE == HP_OVERS) {
            // 1/2-sample delay for phase compensation
            pB->m_amf = (az4 + pB->m_az5) / 2;
            pB->m_az5 = az4;

            // Oversampling (repeat same block)
            float x1 = input - pB->m_amf * m_kacr;
            float az1 = pB->m_azt1 + m_k2vg * tanh_approx(x1 / v2);
            float at1 = m_k2vg * tanh_approx(az1 / v2);
            pB->m_azt1 = az1 - at1;
            float az2 = pB->m_azt2 + at1;
            float at2 = m_k2vg * tanh_approx(az2 / v2);
            pB->m_azt2 = az2 - at2;
            float az3 = pB->m_azt3 + at2;
            float at3 = m_k2vg * tanh_approx(az3 / v2);
            pB->m_azt3 = az3 - at3;
            float az4 = pB->m_azt4 + at3;
            float at4 = m_k2vg * tanh_approx(az4 / v2);
            pB->m_azt4 = az4 - at4;

            // 1/2-sample delay for phase compensation
            pB->m_amf = (az4 + pB->m_az5) / 2;
            pB->m_az5 = az4;
        } else {
            pB->m_amf = az4;
        }

        if (MODE == HP_OVERS || MODE == HP) {
            return (x1 - 3 * az3 + 2 * az4) * m_postGain;
        }
        return pB->m_amf * m_postGain;
    }

    inline float tanh_approx(float input) {
        // return tanhf(input); // 142ns for process;
        return input / (1 + input * input / (3 + input * input / 5)); // 119ns for process
    }


  private:

    struct Buffer m_buf[2];

    float m_postGain;
    float m_kacr; // resonance factor
    float m_k2vg; // IIF factor

    bool m_buffersClear;
};

class EngineFilterMoogLadder4Low : public EngineFilterMoogLadderBase<LP_OVERS> {
    Q_OBJECT
  public:
    EngineFilterMoogLadder4Low(int sampleRate, double freqCorner1, double resonance);
};


class EngineFilterMoogLadder4High : public EngineFilterMoogLadderBase<HP_OVERS> {
    Q_OBJECT
  public:
    EngineFilterMoogLadder4High(int sampleRate, double freqCorner1, double resonance);
};

#endif // ENGINEFILTERMOOGLADDER4_H
