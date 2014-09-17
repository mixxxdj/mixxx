#ifndef BESSELLVMIXEQBASE_H
#define BESSELLVMIXEQBASE_H

#include "util/types.h"
#include "util/defs.h"
#include "util/math.h"
#include "sampleutil.h"
#include "engine/enginefilterdelay.h"

static const int kMaxDelay = 3300; // allows a 30 Hz filter at 97346;
static const unsigned int kStartupSamplerate = 44100;
static const double kStartupLoFreq = 246;
static const double kStartupHiFreq = 2484;

template<class LPF>
class LVMixEQEffectGroupState {
  public:
    LVMixEQEffectGroupState()
        : old_low(1.0),
          old_mid(1.0),
          old_high(1.0),
          m_oldSampleRate(kStartupSamplerate),
          m_loFreq(kStartupLoFreq),
          m_hiFreq(kStartupHiFreq) {
        m_pLowBuf = SampleUtil::alloc(MAX_BUFFER_LEN);
        m_pBandBuf = SampleUtil::alloc(MAX_BUFFER_LEN);
        m_pHighBuf = SampleUtil::alloc(MAX_BUFFER_LEN);

        m_low1 = new LPF(kStartupSamplerate, kStartupLoFreq);
        m_low2 = new LPF(kStartupSamplerate, kStartupHiFreq);
        m_delay2 = new EngineFilterDelay<kMaxDelay>();
        m_delay3 = new EngineFilterDelay<kMaxDelay>();
        setFilters(kStartupSamplerate, kStartupLoFreq, kStartupHiFreq);
    }

    virtual ~LVMixEQEffectGroupState() {
        delete m_low1;
        delete m_low2;
        delete m_delay2;
        delete m_delay3;
        SampleUtil::free(m_pLowBuf);
        SampleUtil::free(m_pBandBuf);
        SampleUtil::free(m_pHighBuf);
    }

    void setFilters(int sampleRate, double lowFreq, double highFreq) {
        int delayLow1 = m_low1->setFrequencyCornersForIntDelay(
                lowFreq / sampleRate, kMaxDelay);
        int delayLow2 = m_low2->setFrequencyCornersForIntDelay(
                highFreq / sampleRate, kMaxDelay);

        m_delay2->setDelay((delayLow1 - delayLow2) * 2);
        m_delay3->setDelay(delayLow1 * 2);
    }

    LPF* m_low1;
    LPF* m_low2;
    EngineFilterDelay<kMaxDelay>* m_delay2;
    EngineFilterDelay<kMaxDelay>* m_delay3;

    double old_low;
    double old_mid;
    double old_high;

    unsigned int m_oldSampleRate;
    double m_loFreq;
    double m_hiFreq;

    CSAMPLE* m_pLowBuf;
    CSAMPLE* m_pBandBuf;
    CSAMPLE* m_pHighBuf;
};

#endif // BESSELLVMIXEQBASE_H
