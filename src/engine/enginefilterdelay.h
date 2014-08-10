#ifndef ENGINEFILTERDELAY_H
#define ENGINEFILTERDELAY_H

#include <string.h>

#include "engine/engineobject.h"

template<unsigned int SIZE>
class EngineFilterDelay : public EngineObjectConstIn {
  public:
    EngineFilterDelay()
            : m_delaySamples(0),
              m_oldDelaySamples(0),
              m_delayPos(0),
              m_doRamping(false) {
        // Set the current buffers to 0
        memset(m_buf, 0, sizeof(m_buf));
    }

    virtual ~EngineFilterDelay() {};

    void setDelay(unsigned int delaySamples) {
        m_oldDelaySamples = m_delaySamples;
        m_delaySamples = delaySamples;
        m_doRamping = true;
    }

    virtual void process(const CSAMPLE* pIn, CSAMPLE* pOutput,
                                     const int iBufferSize) {
        //if (!m_doRamping) {
        if (true) {
            int delaySourcePos = (m_delayPos + SIZE - m_delaySamples) % SIZE;

            Q_ASSERT(delaySourcePos >= 0);
            Q_ASSERT(delaySourcePos <= SIZE);

            for (int i = 0; i < iBufferSize; ++i) {
                // put sample into delay buffer:
                m_buf[m_delayPos] = pIn[i];
                m_delayPos = (m_delayPos + 1) % SIZE;

                // Take delayed sample from delay buffer and copy it to dest buffer:
                pOutput[i] = m_buf[delaySourcePos];
                delaySourcePos = (delaySourcePos + 1) % SIZE;
            }
        } else {
            int delaySourcePos = (m_delayPos + SIZE - m_delaySamples) % SIZE;
            int oldDelaySourcePos = (m_delayPos + SIZE - m_oldDelaySamples) % SIZE;

            Q_ASSERT(delaySourcePos >= 0);
            Q_ASSERT(delaySourcePos <= SIZE);

            double cross_mix = 0.0;
            double cross_inc = 2 / static_cast<double>(iBufferSize);

            for (int i = 0; i < iBufferSize; ++i) {
                // put sample into delay buffer:
                m_buf[m_delayPos] = pIn[i];
                m_delayPos = (m_delayPos + 1) % SIZE;

                // Take delayed sample from delay buffer and copy it to dest buffer:
                if (i < iBufferSize / 2) {
                    pOutput[i] = m_buf[oldDelaySourcePos];
                    oldDelaySourcePos = (oldDelaySourcePos + 1) % SIZE;
                } else {
                    pOutput[i] = m_buf[delaySourcePos] * cross_mix;
                    delaySourcePos = (delaySourcePos + 1) % SIZE;
                    pOutput[i] = m_buf[oldDelaySourcePos] * (1.0 - cross_mix);
                    oldDelaySourcePos = (oldDelaySourcePos + 1) % SIZE;
                    cross_mix += cross_inc;
                }
            }
            m_doRamping = false;
        }
    }

  protected:
    int m_delaySamples;
    int m_oldDelaySamples;
    int m_delayPos;
    double m_buf[SIZE];
    bool m_doRamping;
};

#endif // ENGINEFILTERIIR_H
