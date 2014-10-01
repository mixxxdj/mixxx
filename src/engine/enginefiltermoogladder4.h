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
#include <math.h>

enum MoogMode {
    LP,
    HP,
    LP_OVERS,
    HP_OVERS,
};

template<enum MoogMode MODE>
class EngineFilterMoogLadderBase : public EngineObjectConstIn {

  public:
    EngineFilterMoogLadderBase(float cutoff, float resonance) {
        setParameter(cutoff, resonance);
        initBuffers();
    }

    virtual ~EngineFilterMoogLadderBase() {
    }

    void initBuffers() {
        m_az1 = 0;
        m_az2 = 0;
        m_az3 = 0;
        m_az4 = 0;
        m_az5 = 0;
        m_ay1 = 0;
        m_ay2 = 0;
        m_ay3 = 0;
        m_ay4 = 0;
        m_amf = 0;

        m_buffersClear = true;
    }

    void setParameter(float cutoff, float resonance) {
        m_cutoff = cutoff;
        m_resonance = resonance;
    }

    void pauseFilter() {
        if (!m_buffersClear) {
            initBuffers();
        }
    }

    virtual void process(const CSAMPLE* pIn, CSAMPLE* pOutput,
                         const int iBufferSize) {
        for (int i = 0; i < iBufferSize; i += 2) {
            pOutput[i] = processSample(pIn[i], m_cutoff, m_resonance);
            pOutput[i+1] = pOutput[i];
            //pOutput[i] = processSample(m_coef, m_buf1, pIn[i]);
            //pOutput[i+1] = processSample(m_coef, m_buf2, pIn[i + 1]);
        }
        m_buffersClear = false;
    }

    // resonance [0..1]
    // cutoff from 0 (0Hz) to 1 (nyquist)
    inline CSAMPLE processSample(float input, float cutoff, float resonance) {

        float pi = 3.1415926535;
        float v2 = 40000;   // twice the 'thermal voltage of a transistor'
        float sr = 22100;

        float  cutoff_hz = cutoff * sr;

        float kfc = cutoff_hz / sr; // sr is half the actual filter sampling rate
        float kf = cutoff_hz / (sr * 2);

        // frequency & amplitude correction
        float kfcr = 1.8730 * (kfc*kfc*kfc) + 0.4955 * (kfc*kfc) - 0.6490 * kfc + 0.9988;
        float kacr = -3.9364 * (kfc*kfc) + 1.8409 * kfc + 0.9968;

        float x  = -2.0 * pi * kfcr * kf; // input for taylor approximations
        float exp_out  = expf(x);

        float k2vg = v2 * (1 - exp_out); // filter tuning


        // cascade of 4 1st order sections
        float x1 = (input - 4 * resonance * m_amf * kacr) / v2;
        m_ay1 = m_az1 + k2vg * (tanhf (x1) - tanhf (m_az1 / v2));
        m_az1 = m_ay1;

        m_ay2 = m_az2 + k2vg * (tanh(m_ay1 / v2) - tanh(m_az2 / v2));
        m_az2 = m_ay2;

        m_ay3 = m_az3 + k2vg * (tanh(m_ay2 / v2) - tanh(m_az3 / v2));
        m_az3 = m_ay3;

        m_ay4 = m_az4 + k2vg * (tanh(m_ay3 / v2) - tanh(m_az4 / v2));
        m_az4 = m_ay4;

        // Oversampling if requested
        if (MODE == LP_OVERS || MODE == LP_OVERS ) {
            // 1/2-sample delay for phase compensation
            m_amf = (m_ay4 + m_az5) * 0.5;
            m_az5 = m_ay4;

            // oversampling (repeat same block)
            float x1 = (input - 4 * resonance * m_amf * kacr) / v2;
            m_ay1 = m_az1 + k2vg * (tanhf (x1) - tanhf (m_az1 / v2));
            m_az1 = m_ay1;

            m_ay2 = m_az2 + k2vg * (tanh(m_ay1 / v2) - tanh(m_az2 / v2));
            m_az2 = m_ay2;

            m_ay3 = m_az3 + k2vg * (tanh(m_ay2 / v2) - tanh(m_az3 / v2));
            m_az3 = m_ay3;

            m_ay4 = m_az4 + k2vg * (tanh(m_ay3 / v2) - tanh(m_az4 / v2));
            m_az4 = m_ay4;

            // 1/2-sample delay for phase compensation
            m_amf = (m_ay4 + m_az5) * 0.5;
            m_az5 = m_ay4;
        } else {
            m_amf = m_az5;
            m_az5 = m_ay4;
        }
        return m_amf;
    }


  private:

    CSAMPLE m_az1;
    CSAMPLE m_az2;
    CSAMPLE m_az3;
    CSAMPLE m_az4;
    CSAMPLE m_az5;
    CSAMPLE m_ay1;
    CSAMPLE m_ay2;
    CSAMPLE m_ay3;
    CSAMPLE m_ay4;
    CSAMPLE m_amf;

    float m_cutoff;
    float m_resonance;

    bool m_buffersClear;
};

class EngineFilterMoogLadder4Low : public EngineFilterMoogLadderBase<LP> {
    Q_OBJECT
  public:
    EngineFilterMoogLadder4Low(int sampleRate, double freqCorner1, double resonance);
};


class EngineFilterMoogLadder4High : public EngineFilterMoogLadderBase<LP_OVERS> {
    Q_OBJECT
  public:
    EngineFilterMoogLadder4High(int sampleRate, double freqCorner1, double resonance);
};

#endif // ENGINEFILTERMOOGLADDER4_H
