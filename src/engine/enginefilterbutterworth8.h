#define MAX_COEFS 17
#define MAX_INTERNAL_BUF 16

#include "engine/enginefilter.h"

class EngineFilterButterworth8 : public EngineObject {
  public:
    EngineFilterButterworth8(int sampleRate, int bufSize);
    virtual ~EngineFilterButterworth8();

    // Update filter without recreating it
    void initBuffers();
    virtual void process(const CSAMPLE *pIn, const CSAMPLE *ppOut,
                         const int iBufferSize) = 0;

  protected:
    int m_sampleRate;

    double m_coef[MAX_COEFS];

    int m_bufSize;
    //channel 1 state
    double m_buf1[MAX_INTERNAL_BUF];

    //channel 2 state
    double m_buf2[MAX_INTERNAL_BUF];
};

class EngineFilterButterworth8Low : public EngineFilterButterworth8 {
  public:
    EngineFilterButterworth8Low(int sampleRate, double freqCorner1);

    void setFrequencyCorners(double freqCorner1);
    void process(const CSAMPLE *pIn, const CSAMPLE *ppOut, const int iBufferSize);
};

class EngineFilterButterworth8Band : public EngineFilterButterworth8 {
  public:
    EngineFilterButterworth8Band(int sampleRate, double freqCorner1,
                                 double freqCorner2);

    void setFrequencyCorners(double freqCorner1, double freqCorner2 = 0);
    void process(const CSAMPLE *pIn, const CSAMPLE *ppOut, const int iBufferSize);
};

class EngineFilterButterworth8High : public EngineFilterButterworth8 {
  public:
    EngineFilterButterworth8High(int sampleRate, double freqCorner1);

    void setFrequencyCorners(double freqCorner1);
    void process(const CSAMPLE *pIn, const CSAMPLE *ppOut, const int iBufferSize);
};
