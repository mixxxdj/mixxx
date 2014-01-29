#include "controlobject.h"
#include "engine/enginesidechaincompressor.h"

EngineSideChainCompressor::EngineSideChainCompressor(
        ConfigObject<ConfigValue>* pConfig, const char* group)
    : m_pConfig(pConfig),
      m_compressRatio(0.0),
      m_bAboveThreshold(false),
      m_threshold(1.0),
      m_strength(0.0),
      m_attackTime(0.001),
      m_decayTime(0.001),
      m_attackPerFrame(0.0),
      m_decayPerFrame(0.0) {
    Q_UNUSED(group);
}

void EngineSideChainCompressor::calculateRates() {
    // Don't allow completely zero rates, or else if parameters change
    // we might get stuck on a compression value.
    if (m_attackTime == 0) {
        // Attack really shouldn't be instant, but we allow it.
        m_attackPerFrame = m_strength;
    } else {
        m_attackPerFrame = m_strength / m_attackTime;
    }
    if (m_decayTime == 0) {
        m_decayPerFrame = m_strength / m_decayTime;
    } else {
        m_decayPerFrame = m_strength;
    }
    if (m_attackPerFrame <= 0) {
        m_attackPerFrame = 0.005;
    }
    if (m_decayPerFrame <= 0) {
        m_decayPerFrame = 0.005;
    }
}

void EngineSideChainCompressor::processKey(const CSAMPLE* pIn, const int iBufferSize) {
    for (int i = 0; i + 1 < iBufferSize; i += 2) {
        CSAMPLE val = (pIn[i] + pIn[i + 1]) / 2;
        if (val > m_threshold) {
            m_bAboveThreshold = true;
            return;
        }
    }
    m_bAboveThreshold = false;
}

void EngineSideChainCompressor::process(
        const CSAMPLE* pIn, CSAMPLE* pOut, const int iBufferSize) {
    for (int i = 0; i + 1 < iBufferSize; i += 2) {
        m_compressRatio = calculateCompression(m_compressRatio, m_bAboveThreshold);
        pOut[i] = pIn[i] / (1. - m_compressRatio);
        pOut[i + 1] = pIn[i + 1] / (1. - m_compressRatio);
    }
}

// Called for every frame, so inline.
inline double EngineSideChainCompressor::calculateCompression(
        CSAMPLE currentRatio, bool aboveThreshold) const {
    if (aboveThreshold) {
        if (currentRatio < m_strength) {
            currentRatio += m_attackPerFrame;
            if (currentRatio > m_strength) {
                // If we overshot, clamp.
                currentRatio = m_strength;
            }
        } else if (currentRatio > m_strength) {
            // If the strength param was changed, we might be compressing too much.
            currentRatio -= m_decayPerFrame;
        }
    } else {
        if (currentRatio > 0) {
            currentRatio -= m_decayPerFrame;
            if (currentRatio < 0) {
                // If we overshot, clamp.
                currentRatio = 0;
            }
        } else if (currentRatio < 0) {
            // Complain loudly.
            qWarning() << "Programming error, below-zero compression detected.";
            currentRatio += m_attackPerFrame;
        }
    }
    return currentRatio;
}

