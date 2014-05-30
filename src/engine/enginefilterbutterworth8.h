#ifndef ENGINE_ENGINEFILTERBUTTERWORTH8_H_
#define ENGINE_ENGINEFILTERBUTTERWORTH8_H_

#define MAX_COEFS 17
#define MAX_INTERNAL_BUF 16

#include "engine/enginefilter.h"
#include "util/defs.h"

class EngineFilterButterworth8 : public EngineObjectConstIn {
    Q_OBJECT
  public:
    EngineFilterButterworth8(int sampleRate, int bufSize);
    virtual ~EngineFilterButterworth8();

    // Update filter without recreating it
    void initBuffers();
    virtual void process(const CSAMPLE* pIn, CSAMPLE* pOut,
                         const int iBufferSize) = 0;

  protected:
    int m_sampleRate;

    CSAMPLE m_oldCoef[MAX_COEFS];
    CSAMPLE m_coef[MAX_COEFS];

    int m_bufSize;
    //channel 1 state
    CSAMPLE m_buf1[MAX_INTERNAL_BUF];
    CSAMPLE m_oldBuf1[MAX_INTERNAL_BUF];

    //channel 2 state
    CSAMPLE m_buf2[MAX_INTERNAL_BUF];
    CSAMPLE m_oldBuf2[MAX_INTERNAL_BUF];

    bool m_doRamping;
};

class EngineFilterButterworth8Low : public EngineFilterButterworth8 {
    Q_OBJECT
  public:
    EngineFilterButterworth8Low(int sampleRate, double freqCorner1);

    void setFrequencyCorners(double freqCorner1);
    void process(const CSAMPLE* pIn, CSAMPLE* pOut, const int iBufferSize);
};

class EngineFilterButterworth8Band : public EngineFilterButterworth8 {
    Q_OBJECT
  public:
    EngineFilterButterworth8Band(int sampleRate, double freqCorner1,
                                 double freqCorner2);

    void setFrequencyCorners(double freqCorner1, double freqCorner2 = 0);
    void process(const CSAMPLE* pIn, CSAMPLE* pOut, const int iBufferSize);
};

class EngineFilterButterworth8High : public EngineFilterButterworth8 {
    Q_OBJECT
  public:
    EngineFilterButterworth8High(int sampleRate, double freqCorner1);

    void setFrequencyCorners(double freqCorner1);
    void process(const CSAMPLE* pIn, CSAMPLE* pOut, const int iBufferSize);
};

#endif  // ENGINE_ENGINEFILTERBUTTERWORTH8_H_
