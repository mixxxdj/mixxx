#ifndef BESSELLVMIXEQBASE_H
#define BESSELLVMIXEQBASE_H

#include "util/types.h"
#include "util/defs.h"
#include "util/math.h"
#include "sampleutil.h"
#include "engine/enginefilterdelay.h"

static const unsigned int kMaxDelay = 3300; // allows a 30 Hz filter at 97346;
static const unsigned int kStartupSamplerate = 44100;
static const unsigned int kStartupLoFreq = 246;
static const unsigned int kStartupHiFreq = 2484;

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

    void setFilters(int sampleRate, int lowFreq, int highFreq) {
        const double delayFactor = m_low1->delayFactor();
        double delayLow1 = sampleRate * delayFactor / lowFreq;
        double delayLow2 = sampleRate * delayFactor / highFreq;
        // Since we delay only full samples, we can only allow frequencies
        // producing such delays
        // stepLow/HighFreq must be <= Samplerate /2
        unsigned int iDelayLow1 = math_clamp((unsigned int)(delayLow1 + 0.5),
                math_max(1u, (unsigned int)(delayFactor * 2 + 0.5)),
                kMaxDelay / 2);
        unsigned int iDelayLow2 = math_clamp((unsigned int)(delayLow2 + 0.5),
                math_max(1u, (unsigned int)(delayFactor * 2 + 0.5)),
                kMaxDelay / 2);


        double stepLowFreq;
        if (iDelayLow1 > 1) {
            stepLowFreq = sampleRate * delayFactor / iDelayLow1;
        } else {
            // Delay = 1 needs a special corner frequency
            iDelayLow1 = 1;
            stepLowFreq = sampleRate * delayFactor / 1.5;
        }

        double stepHighFreq;
        if (iDelayLow2 > 1) {
            // 0.3 works best
            stepHighFreq = sampleRate * delayFactor / (iDelayLow2 + 0.3);
        } else {
            // Delay = 1 needs a special corner frequency
            iDelayLow2 = 1;
            stepHighFreq = sampleRate * delayFactor / 1.5;
        }

        m_low1->setFrequencyCorners(sampleRate, stepLowFreq);
        m_low2->setFrequencyCorners(sampleRate, stepHighFreq);
        m_delay2->setDelay((iDelayLow1 - iDelayLow2) * 2);
        m_delay3->setDelay(iDelayLow1 * 2);
    }

    LPF* m_low1;
    LPF* m_low2;
    EngineFilterDelay<kMaxDelay>* m_delay2;
    EngineFilterDelay<kMaxDelay>* m_delay3;

    double old_low;
    double old_mid;
    double old_high;

    unsigned int m_oldSampleRate;
    int m_loFreq;
    int m_hiFreq;

    CSAMPLE* m_pLowBuf;
    CSAMPLE* m_pBandBuf;
    CSAMPLE* m_pHighBuf;
};

#endif // BESSELLVMIXEQBASE_H
