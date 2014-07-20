
#ifndef ENGINEFILTERBESSEL4_H
#define ENGINEFILTERBESSEL4_H

#define MAX_COEFS 17
#define MAX_INTERNAL_BUF 16

#include "engine/engineobject.h"
#include "util/types.h"

class EngineFilterBessel4 : public EngineObjectConstIn {
    Q_OBJECT
  public:
    EngineFilterBessel4(int bufSize);
    virtual ~EngineFilterBessel4();

    void initBuffers();
    virtual void process(const CSAMPLE* pIn, CSAMPLE* pOut,
                         const int iBufferSize) = 0;

  protected:
    int m_sampleRate;

    double m_coef[MAX_COEFS];
    // Old coefficients needed for ramping
    double m_oldCoef[MAX_COEFS];

    int m_bufSize;
    // Channel 1 state
    double m_buf1[MAX_INTERNAL_BUF];
    // Old channel 1 buffer needed for ramping
    double m_oldBuf1[MAX_INTERNAL_BUF];

    // Channel 2 state
    double m_buf2[MAX_INTERNAL_BUF];
    // Old channel 2 buffer needed for ramping
    double m_oldBuf2[MAX_INTERNAL_BUF];

    // Flag set to true if ramping needs to be done
    bool m_doRamping;
};

class EngineFilterBessel4Low : public EngineFilterBessel4 {
    Q_OBJECT
  public:
    EngineFilterBessel4Low(int sampleRate, double freqCorner1);
    void setFrequencyCorners(int sampleRate, double freqCorner1);
    void process(const CSAMPLE* pIn, CSAMPLE* pOut, const int iBufferSize);
};

class EngineFilterBessel4Band : public EngineFilterBessel4 {
    Q_OBJECT
  public:
    EngineFilterBessel4Band(int sampleRate, double freqCorner1,
                        double freqCorner2);
    void setFrequencyCorners(int sampleRate, double freqCorner1,
                             double freqCorner2);
    void process(const CSAMPLE* pIn, CSAMPLE* pOut, const int iBufferSize);
};

class EngineFilterBessel4High : public EngineFilterBessel4 {
    Q_OBJECT
  public:
    EngineFilterBessel4High(int sampleRate, double freqCorner1);
    void setFrequencyCorners(int sampleRate, double freqCorner1);
    void process(const CSAMPLE* pIn, CSAMPLE* pOut, const int iBufferSize);
};

#endif // ENGINEFILTERBESSEL4_H
