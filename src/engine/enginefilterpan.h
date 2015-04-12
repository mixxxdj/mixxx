#ifndef ENGINEFILTERPAN_H
#define ENGINEFILTERPAN_H

#include <string.h>

#include "engine/engineobject.h"
#include "util/assert.h"

static const int numChannels = 2;

template<unsigned int SIZE>
class EngineFilterPan : public EngineObjectConstIn {
  public:
    EngineFilterPan()
            : m_leftDelayFrames(0),
              m_oldLeftDelayFrames(0),
              m_delayFrame(0),
              m_doRamping(false),
              m_doStart(false) {
        // Set the current buffers to 0
        memset(m_buf, 0, sizeof(m_buf));
    }

    virtual ~EngineFilterPan() {};

    void pauseFilter() {
        // Set the current buffers to 0
        if (!m_doStart) {
            memset(m_buf, 0, sizeof(m_buf));
            m_doStart = true;
        }
    }

    void setLeftDelay(unsigned int leftDelaySamples) {
        if (m_leftDelayFrames != leftDelaySamples) {
            m_oldLeftDelayFrames = m_leftDelayFrames;
            m_leftDelayFrames = leftDelaySamples;
            m_doRamping = true;
        }
    }

    virtual void process(const CSAMPLE* pIn, CSAMPLE* pOutput,
                         const int iBufferSize) {
        int delayLeftSourceFrame = m_delayFrame;
        int delayRightSourceFrame = m_delayFrame;
        if (m_leftDelayFrames > 0) {
            delayLeftSourceFrame = (m_delayFrame + SIZE - m_leftDelayFrames) % SIZE;
        } else {
            delayRightSourceFrame = (m_delayFrame + SIZE + m_leftDelayFrames) % SIZE;
        }

        DEBUG_ASSERT_AND_HANDLE(delayLeftSourceFrame >= 0 &&
                                delayRightSourceFrame >= 0 &&
                                delayLeftSourceFrame <= static_cast<int>(SIZE) &&
                                delayRightSourceFrame <= static_cast<int>(SIZE) ) {
            SampleUtil::copy(pOutput, pIn, iBufferSize);
            return;
        }

        if (!m_doRamping) {
            for (int i = 0; i < iBufferSize / 2; ++i) {
                // put sample into delay buffer:
                m_buf[m_delayFrame * 2] = pIn[i * 2];
                m_buf[m_delayFrame * 2 + 1] = pIn[i * 2 + 1];
                m_delayFrame = (m_delayFrame + 1) % SIZE;

                // Take delayed sample from delay buffer and copy it to dest buffer:
                pOutput[i * 2] = m_buf[delayLeftSourceFrame * 2];
                pOutput[i * 2 + 1] = m_buf[delayRightSourceFrame * 2 + 1];
                delayLeftSourceFrame = (delayLeftSourceFrame + 1) % SIZE;
                delayRightSourceFrame = (delayRightSourceFrame + 1) % SIZE;
            }
        } else {
            int delayOldLeftSourceFrame = m_delayFrame;
            int delayOldRightSourceFrame = m_delayFrame;
            if (m_oldLeftDelayFrames > 0) {
                delayOldLeftSourceFrame = (m_delayFrame + SIZE - m_oldLeftDelayFrames) % SIZE;
            } else {
                delayOldRightSourceFrame = (m_delayFrame + SIZE + m_oldLeftDelayFrames) % SIZE;
            }

            DEBUG_ASSERT_AND_HANDLE(delayOldLeftSourceFrame >= 0 &&
                                    delayOldRightSourceFrame >= 0 &&
                                    delayOldLeftSourceFrame <= static_cast<int>(SIZE) &&
                                    delayOldRightSourceFrame <= static_cast<int>(SIZE) ) {
                SampleUtil::copy(pOutput, pIn, iBufferSize);
                return;
            }

            double cross_mix = 0.0;
            double cross_inc = 2 / static_cast<double>(iBufferSize);

            for (int i = 0; i < iBufferSize / 2; ++i) {
                // put sample into delay buffer:
                m_buf[m_delayFrame * 2] = pIn[i * 2];
                m_buf[m_delayFrame * 2 + 1] = pIn[i * 2 + 1];
                m_delayFrame = (m_delayFrame + 1) % SIZE;

                // Take delayed sample from delay buffer and copy it to dest buffer:
                cross_mix += cross_inc;
                pOutput[i * 2] = m_buf[delayLeftSourceFrame * 2] * cross_mix;
                pOutput[i * 2 + 1] = m_buf[delayRightSourceFrame * 2 + 1] * cross_mix;
                pOutput[i * 2] += m_buf[delayOldLeftSourceFrame * 2] * (1 - cross_mix);
                pOutput[i * 2 + 1] += m_buf[delayOldRightSourceFrame * 2 + 1] * (1 - cross_mix);
                delayLeftSourceFrame = (delayLeftSourceFrame + 1) % SIZE;
                delayRightSourceFrame = (delayRightSourceFrame + 1) % SIZE;
                delayOldLeftSourceFrame = (delayOldLeftSourceFrame + 1) % SIZE;
                delayOldRightSourceFrame = (delayOldRightSourceFrame + 1) % SIZE;
            }
            m_doRamping = false;
        }
        m_doStart = false;
    }

  protected:
    int m_leftDelayFrames;
    int m_oldLeftDelayFrames;
    int m_delayFrame;
    CSAMPLE m_buf[SIZE * numChannels];
    bool m_doRamping;
    bool m_doStart;
};

#endif // ENGINEFILTERPAN_H
