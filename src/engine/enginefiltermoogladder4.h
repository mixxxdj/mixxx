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

  public:
    EngineFilterMoogLadderBase(unsigned int sampleRate, float cutoff, float resonance) {
        setParameter(sampleRate, cutoff, resonance);
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
        m_amf = 0;

        m_buffersClear = true;
    }

    void setParameter(int sampleRate, float cutoff, float resonance) {
        m_cutoff = cutoff;
        m_resonance = resonance;
        m_inputSampeRate = sampleRate;
        m_postGain = (1 + resonance / 4 * (1.1f + cutoff / sampleRate * 3.5f))
                * (2 - (1.0f - resonance / 4) * (1.0f - resonance / 4));

        qDebug() << "setParameter" << m_cutoff << m_resonance;
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
    inline CSAMPLE processSample(float input, float cutoff_hz, float resonance) {

        float v2 = 2 + kVt;   // twice the 'thermal voltage of a transistor'

        float kfc = cutoff_hz / m_inputSampeRate;
        float kf = kfc;
        if (MODE == LP_OVERS || MODE == HP_OVERS ) {
            // m_inputSampeRate is half the actual filter sampling rate in oversampling mode
            kf = kfc / 2;
        }

        // frequency & amplitude correction
        float kfcr = 1.8730 * (kfc*kfc*kfc) + 0.4955 * (kfc*kfc) - 0.6490 * kfc + 0.9988;


        float x  = -2.0 * kPi * kfcr * kf; // input for taylor approximations
        float exp_out  = expf(x);
        float k2vg = v2 * (1 - exp_out); // filter tuning

        float kacr;
        if (MODE == LP_OVERS || MODE == LP ) {
            kacr = -3.9364 * (kfc*kfc) + 1.8409 * kfc + 0.9968;
        } else {
            kacr = 10;
        }

        // cascade of 4 1st order sections
        float x1 = input - resonance * m_amf * kacr;
        m_az1 = m_az1 + k2vg * (tanhf (x1 / v2) - tanhf (m_az1 / v2));
        m_az2 = m_az2 + k2vg * (tanhf(m_az1 / v2) - tanhf(m_az2 / v2));
        m_az3 = m_az3 + k2vg * (tanhf(m_az2 / v2) - tanhf(m_az3 / v2));
        m_az4 = m_az4 + k2vg * (tanhf(m_az3 / v2) - tanhf(m_az4 / v2));

        // Oversampling if requested
        if (MODE == LP_OVERS || MODE == HP_OVERS ) {
            // 1/2-sample delay for phase compensation
            m_amf = (m_az4 + m_az5) / 2;
            m_az5 = m_az4;

            // Oversampling (repeat same block)
            x1 = input - resonance * m_amf * kacr;
            m_az1 = m_az1 + k2vg * (tanhf (x1 / v2) - tanhf (m_az1 / v2));
            m_az2 = m_az2 + k2vg * (tanhf(m_az1 / v2) - tanhf(m_az2 / v2));
            m_az3 = m_az3 + k2vg * (tanhf(m_az2 / v2) - tanhf(m_az3 / v2));
            m_az4 = m_az4 + k2vg * (tanhf(m_az3 / v2) - tanhf(m_az4 / v2));

            // 1/2-sample delay for phase compensation
            m_amf = (m_az4 + m_az5) / 2;
            m_az5 = m_az4;
        } else {
            m_amf = m_az4;
        }

        if (MODE == HP_OVERS || MODE == HP ) {
            return x1 - 3 * m_az3 + 2 * m_az4;
        }
        return m_amf * m_postGain;
    }

  private:

    CSAMPLE m_az1;
    CSAMPLE m_az2;
    CSAMPLE m_az3;
    CSAMPLE m_az4;
    CSAMPLE m_az5;
    CSAMPLE m_amf;

    float m_cutoff;
    float m_resonance;
    unsigned int m_inputSampeRate;
    float m_postGain;

    bool m_buffersClear;
};

class EngineFilterMoogLadder4Low : public EngineFilterMoogLadderBase<LP_OVERS> {
    Q_OBJECT
  public:
    EngineFilterMoogLadder4Low(int sampleRate, double freqCorner1, double resonance);
};


class EngineFilterMoogLadder4High : public EngineFilterMoogLadderBase<HP> {
    Q_OBJECT
  public:
    EngineFilterMoogLadder4High(int sampleRate, double freqCorner1, double resonance);
};

#endif // ENGINEFILTERMOOGLADDER4_H
