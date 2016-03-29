#ifndef BESSELLVMIXEQBASE_H
#define BESSELLVMIXEQBASE_H

#include "engine/enginefilterdelay.h"
#include "util/defs.h"
#include "util/math.h"
#include "util/sample.h"
#include "util/types.h"

static const int kMaxDelay = 3300; // allows a 30 Hz filter at 97346;
static const int kRampDone = -1;
static const unsigned int kStartupSamplerate = 44100;
static const double kStartupLoFreq = 246;
static const double kStartupHiFreq = 2484;


template<class LPF>
class LVMixEQEffectGroupState {
  public:
    LVMixEQEffectGroupState()
        : m_oldLow(1.0),
          m_oldMid(1.0),
          m_oldHigh(1.0),
          m_rampHoldOff(kRampDone),
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
        m_groupDelay = delayLow1 * 2;
    }

    void processChannel(const CSAMPLE* pInput, CSAMPLE* pOutput,
                        const int numSamples,
                        const unsigned int sampleRate,
                        double fLow, double fMid, double fHigh,
                        double loFreq, double hiFreq) {
        if (m_oldSampleRate != sampleRate ||
                (m_loFreq != loFreq) ||
                (m_hiFreq != hiFreq)) {
            m_loFreq = loFreq;
            m_hiFreq = hiFreq;
            m_oldSampleRate = sampleRate;
            setFilters(sampleRate, loFreq, hiFreq);
        }

        // Since a Bessel Low pass Filter has a constant group delay in the pass band,
        // we can subtract or add the filtered signal to the dry signal if we compensate this delay
        // The dry signal represents the high gain
        // Then the higher low pass is added and at least the lower low pass result.
        fLow = fLow - fMid;
        fMid = fMid - fHigh;

        // Note: We do not call pauseFilter() here because this will introduce a
        // buffer size-dependent start delay. During such start delay some unwanted
        // frequencies are slipping though or wanted frequencies are damped.
        // We know the exact group delay here so we can just hold off the ramping.
        if (fHigh || m_oldHigh) {
            m_delay3->process(pInput, m_pHighBuf, numSamples);
        }

        if (fMid || m_oldMid) {
            m_delay2->process(pInput, m_pBandBuf, numSamples);
            m_low2->process(m_pBandBuf, m_pBandBuf, numSamples);
        }

        if (fLow || m_oldLow) {
            m_low1->process(pInput, m_pLowBuf, numSamples);
        }

        // Test code for comparing streams as two stereo channels
        //for (unsigned int i = 0; i < numSamples; i +=2) {
        //    pOutput[i] = pState->m_pLowBuf[i];
        //    pOutput[i + 1] = pState->m_pBandBuf[i];
        //}

        if (fLow == m_oldLow &&
                fMid == m_oldMid &&
                fHigh == m_oldHigh) {
            SampleUtil::copy3WithGain(pOutput,
                    m_pLowBuf, fLow,
                    m_pBandBuf, fMid,
                    m_pHighBuf, fHigh,
                    numSamples);
        } else {
            int copySamples = 0;
            int rampingSamples = numSamples;
            if ((fLow && !m_oldLow) ||
                    (fMid && !m_oldMid) ||
                    (fHigh && !m_oldHigh)) {
                // we have just switched at least one filter on
                // Hold off ramping for the group delay
                if (m_rampHoldOff == kRampDone) {
                    // multiply the group delay * 2 to ensure that the filter is
                    // settled it is actually at a factor of 1,8 at default setting
                    m_rampHoldOff = m_groupDelay * 2;
                    // ensure that we have at least 128 samples for ramping
                    // (the smallest buffer, that suits for de-clicking)
                    int rampingSamples = numSamples - (m_rampHoldOff % numSamples);
                    if (rampingSamples < 128) {
                        m_rampHoldOff += rampingSamples;
                    }
                }

                // ramping is done in one of the following calls if
                // pState->m_rampHoldOff >= numSamples;
                copySamples = math_min<int>(m_rampHoldOff, numSamples);
                m_rampHoldOff -= copySamples;
                rampingSamples = numSamples - copySamples;

                SampleUtil::copy3WithGain(pOutput,
                        m_pLowBuf, m_oldLow,
                        m_pBandBuf, m_oldMid,
                        m_pHighBuf, m_oldHigh,
                        copySamples);
            }

            if (rampingSamples) {
                SampleUtil::copy3WithRampingGain(&pOutput[copySamples],
                        &m_pLowBuf[copySamples], m_oldLow, fLow,
                        &m_pBandBuf[copySamples], m_oldMid, fMid,
                        &m_pHighBuf[copySamples], m_oldHigh, fHigh,
                        rampingSamples);

                m_oldLow = fLow;
                m_oldMid = fMid;
                m_oldHigh = fHigh;
                m_rampHoldOff = kRampDone;

            }
        }
    }

  private:
    LPF* m_low1;
    LPF* m_low2;
    EngineFilterDelay<kMaxDelay>* m_delay2;
    EngineFilterDelay<kMaxDelay>* m_delay3;

    double m_oldLow;
    double m_oldMid;
    double m_oldHigh;

    int m_rampHoldOff;
    int m_groupDelay;

    unsigned int m_oldSampleRate;
    double m_loFreq;
    double m_hiFreq;

    CSAMPLE* m_pLowBuf;
    CSAMPLE* m_pBandBuf;
    CSAMPLE* m_pHighBuf;
};

#endif // BESSELLVMIXEQBASE_H
