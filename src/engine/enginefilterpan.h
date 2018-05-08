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
        int delayLeftSourceFrame;
        int delayRightSourceFrame;
        if (m_leftDelayFrames > 0) {
            delayLeftSourceFrame = m_delayFrame + SIZE - m_leftDelayFrames;
            delayRightSourceFrame = m_delayFrame + SIZE;
        } else {
            delayLeftSourceFrame = m_delayFrame + SIZE;
            delayRightSourceFrame = m_delayFrame + SIZE + m_leftDelayFrames;
        }

        VERIFY_OR_DEBUG_ASSERT(delayLeftSourceFrame >= 0 &&
                                delayRightSourceFrame >= 0) {
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
                pOutput[i * 2] = m_buf[(delayLeftSourceFrame % SIZE)* 2];
                pOutput[i * 2 + 1] = m_buf[(delayRightSourceFrame % SIZE) * 2 + 1];
                delayLeftSourceFrame++;
                delayRightSourceFrame++;
            }
        } else {
            int delayOldLeftSourceFrame;
            int delayOldRightSourceFrame;
            if (m_oldLeftDelayFrames > 0) {
                delayOldLeftSourceFrame = m_delayFrame + SIZE - m_oldLeftDelayFrames;
                delayOldRightSourceFrame = m_delayFrame + SIZE;
            } else {
                delayOldLeftSourceFrame = m_delayFrame + SIZE;
                delayOldRightSourceFrame = m_delayFrame + SIZE + m_oldLeftDelayFrames;
            }

            VERIFY_OR_DEBUG_ASSERT(delayOldLeftSourceFrame >= 0 &&
                                    delayOldRightSourceFrame >= 0) {
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

                double rampedLeftSourceFrame = delayLeftSourceFrame * cross_mix +
                                               delayOldLeftSourceFrame * (1 - cross_mix);
                double rampedRightSourceFrame = delayRightSourceFrame * cross_mix +
                                                delayOldRightSourceFrame * (1 - cross_mix);
                double modLeft = fmod(rampedLeftSourceFrame, 1);
                double modRight = fmod(rampedRightSourceFrame, 1);

                pOutput[i * 2] = m_buf[(static_cast<int>(floor(rampedLeftSourceFrame)) % SIZE) * 2] * (1 - modLeft);
                pOutput[i * 2 + 1] = m_buf[(static_cast<int>(floor(rampedRightSourceFrame)) % SIZE) * 2 + 1] * (1 - modRight);
                pOutput[i * 2] += m_buf[(static_cast<int>(ceil(rampedLeftSourceFrame)) % SIZE) * 2] * modLeft;
                pOutput[i * 2 + 1] += m_buf[(static_cast<int>(ceil(rampedRightSourceFrame)) % SIZE) * 2 + 1] * modRight;
                delayLeftSourceFrame++;
                delayRightSourceFrame++;
                delayOldLeftSourceFrame++;
                delayOldRightSourceFrame++;
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
