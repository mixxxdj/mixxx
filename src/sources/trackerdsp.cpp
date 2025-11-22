#include "sources/trackerdsp.h"

#include <algorithm>
#include <cstring>

#include "util/math.h"
#include "util/sample.h"

namespace mixxx {

namespace {

inline SINT getMaskFromSize(SINT len) {
    SINT n = 2;
    while (n <= len) {
        n <<= 1;
    }
    return ((n >> 1) - 1);
}

} // anonymous namespace

TrackerDSP::TrackerDSP()
        : m_sampleRate(44100) {
    // allocate buffers
    m_xBassBuffer.resize(XBASSBUFFERSIZE, 0);
    m_xBassDelay.resize(XBASSBUFFERSIZE, 0);
    m_dolbyLoFilterBuffer.resize(64, 0);
    m_dolbyLoFilterDelay.resize(32, 0);
    m_dolbyHiFilterBuffer.resize(FILTERBUFFERSIZE, 0);
    m_surroundBuffer.resize(SURROUNDBUFFERSIZE, 0);
    m_reverbLoFilterBuffer.resize(64, 0);
    m_reverbLoFilterDelay.resize(32, 0);
    m_reverbBuffer.resize(REVERBBUFFERSIZE, 0);
    m_reverbBuffer2.resize(REVERBBUFFERSIZE2, 0);
    m_reverbBuffer3.resize(REVERBBUFFERSIZE3, 0);
    m_reverbBuffer4.resize(REVERBBUFFERSIZE4, 0);
    m_gRvbLowPass.resize(8, 0);
}

TrackerDSP::~TrackerDSP() = default;

void TrackerDSP::configure(const Settings& settings, audio::SampleRate sampleRate) {
    bool needsReset = !m_initialized ||
            (m_sampleRate != sampleRate) ||
            (settings.reverbEnabled != m_settings.reverbEnabled) ||
            (settings.megabassEnabled != m_settings.megabassEnabled) ||
            (settings.surroundEnabled != m_settings.surroundEnabled);

    m_settings = settings;
    m_sampleRate = sampleRate;

    if (needsReset) {
        initializeBuffers(true);
    }

    m_initialized = true;
}

void TrackerDSP::reset() {
    initializeBuffers(true);
}

void TrackerDSP::initializeBuffers(bool fullReset) {
    // map settings to internal parameters
    m_nReverbDelay = m_settings.reverbDelay;
    if (m_nReverbDelay == 0) {
        m_nReverbDelay = 100;
    }
    m_nReverbDepth = m_settings.reverbDepth / 10; // 0-100 -> 0-10

    m_nXBassRange = 14;                                   // fixed at 2.5ms
    m_nXBassDepth = (m_settings.bassDepth * 6) / 100 + 2; // 2-8
    if (m_nXBassDepth > 8)
        m_nXBassDepth = 8;
    if (m_nXBassDepth < 2)
        m_nXBassDepth = 2;

    m_nProLogicDelay = m_settings.surroundDelay;
    if (m_nProLogicDelay == 0) {
        m_nProLogicDelay = 20;
    }
    m_nProLogicDepth = (m_settings.surroundDepth * 12) / 100 + 4; // 4-16

    if (fullReset) {
        // noise reduction
        m_nLeftNR = m_nRightNR = 0;
    }

    // pro-logic surround setup
    m_nSurroundPos = m_nSurroundSize = 0;
    m_nDolbyLoFltPos = m_nDolbyLoFltSum = m_nDolbyLoDlyPos = 0;
    m_nDolbyHiFltPos = m_nDolbyHiFltSum = 0;

    if (m_settings.surroundEnabled) {
        std::fill(m_dolbyLoFilterBuffer.begin(), m_dolbyLoFilterBuffer.end(), 0);
        std::fill(m_dolbyHiFilterBuffer.begin(), m_dolbyHiFilterBuffer.end(), 0);
        std::fill(m_dolbyLoFilterDelay.begin(), m_dolbyLoFilterDelay.end(), 0);
        std::fill(m_surroundBuffer.begin(), m_surroundBuffer.end(), 0);

        m_nSurroundSize = (m_sampleRate * m_nProLogicDelay) / 1000;
        if (m_nSurroundSize > SURROUNDBUFFERSIZE) {
            m_nSurroundSize = SURROUNDBUFFERSIZE;
        }

        if (m_nProLogicDepth < 8) {
            m_nDolbyDepth = (32 >> m_nProLogicDepth) + 32;
        } else {
            m_nDolbyDepth = (m_nProLogicDepth < 16) ? (8 + (m_nProLogicDepth - 8) * 7) : 64;
        }
        m_nDolbyDepth >>= 2;
    }

    // reverb setup
    if (m_settings.reverbEnabled) {
        SINT nrs = (m_sampleRate * m_nReverbDelay) / 1000;
        SINT nfa = m_nReverbDepth + 1;
        if (nrs > REVERBBUFFERSIZE) {
            nrs = REVERBBUFFERSIZE;
        }

        if (fullReset || (nrs != m_nReverbSize) || (nfa != m_nFilterAttn)) {
            m_nFilterAttn = nfa;
            m_nReverbSize = nrs;
            m_nReverbBufferPos = m_nReverbBufferPos2 = m_nReverbBufferPos3 =
                    m_nReverbBufferPos4 = 0;
            m_nReverbLoFltSum = m_nReverbLoFltPos = m_nReverbLoDlyPos = 0;
            m_gRvbLPSum = m_gRvbLPPos = 0;

            m_nReverbSize2 = (nrs * 13) / 17;
            if (m_nReverbSize2 > REVERBBUFFERSIZE2) {
                m_nReverbSize2 = REVERBBUFFERSIZE2;
            }
            m_nReverbSize3 = (nrs * 7) / 13;
            if (m_nReverbSize3 > REVERBBUFFERSIZE3) {
                m_nReverbSize3 = REVERBBUFFERSIZE3;
            }
            m_nReverbSize4 = (nrs * 7) / 19;
            if (m_nReverbSize4 > REVERBBUFFERSIZE4) {
                m_nReverbSize4 = REVERBBUFFERSIZE4;
            }

            std::fill(m_reverbLoFilterBuffer.begin(), m_reverbLoFilterBuffer.end(), 0);
            std::fill(m_reverbLoFilterDelay.begin(), m_reverbLoFilterDelay.end(), 0);
            std::fill(m_reverbBuffer.begin(), m_reverbBuffer.end(), 0);
            std::fill(m_reverbBuffer2.begin(), m_reverbBuffer2.end(), 0);
            std::fill(m_reverbBuffer3.begin(), m_reverbBuffer3.end(), 0);
            std::fill(m_reverbBuffer4.begin(), m_reverbBuffer4.end(), 0);
            std::fill(m_gRvbLowPass.begin(), m_gRvbLowPass.end(), 0);
        }
    } else {
        m_nReverbSize = 0;
    }

    // bass expansion setup
    bool resetBass = false;
    if (m_settings.megabassEnabled) {
        SINT nXBassSamples = (m_sampleRate * m_nXBassRange) / 10000;
        if (nXBassSamples > XBASSBUFFERSIZE) {
            nXBassSamples = XBASSBUFFERSIZE;
        }
        SINT mask = getMaskFromSize(nXBassSamples);
        if (fullReset || (mask != m_nXBassMask)) {
            m_nXBassMask = mask;
            resetBass = true;
        }
    } else {
        m_nXBassMask = 0;
        resetBass = true;
    }

    if (resetBass) {
        m_nXBassSum = m_nXBassBufferPos = m_nXBassDlyPos = 0;
        std::fill(m_xBassBuffer.begin(), m_xBassBuffer.end(), 0);
        std::fill(m_xBassDelay.begin(), m_xBassDelay.end(), 0);
    }
}

void TrackerDSP::processStereo(CSAMPLE* pSamples, SINT frameCount) {
    if (!m_initialized || frameCount <= 0) {
        return;
    }

    // convert float samples to int for DSP processing (libmodplug uses 32-bit ints)
    std::vector<SINT> intBuffer(frameCount * 2);
    for (SINT i = 0; i < frameCount * 2; ++i) {
        intBuffer[i] = static_cast<SINT>(pSamples[i] * 32768.0f);
    }

    // apply effects in order (matching libmodplug order)
    if (m_settings.reverbEnabled) {
        processReverb(reinterpret_cast<CSAMPLE*>(intBuffer.data()), frameCount);
    }
    if (m_settings.surroundEnabled) {
        processSurround(reinterpret_cast<CSAMPLE*>(intBuffer.data()), frameCount);
    }
    if (m_settings.megabassEnabled) {
        processMegabass(reinterpret_cast<CSAMPLE*>(intBuffer.data()), frameCount);
    }
    if (m_settings.noiseReductionEnabled) {
        processNoiseReduction(reinterpret_cast<CSAMPLE*>(intBuffer.data()), frameCount);
    }

    // convert back to float
    for (SINT i = 0; i < frameCount * 2; ++i) {
        pSamples[i] = static_cast<CSAMPLE>(intBuffer[i]) / 32768.0f;
    }
}

void TrackerDSP::processReverb(CSAMPLE* pSamples, SINT frameCount) {
    SINT* pr = reinterpret_cast<SINT*>(pSamples);

    for (SINT i = 0; i < frameCount; ++i) {
        SINT echo = m_reverbBuffer[m_nReverbBufferPos] +
                m_reverbBuffer2[m_nReverbBufferPos2] +
                m_reverbBuffer3[m_nReverbBufferPos3] +
                m_reverbBuffer4[m_nReverbBufferPos4];

        // delay line and remove low frequencies
        SINT echodly = m_reverbLoFilterDelay[m_nReverbLoDlyPos];
        m_reverbLoFilterDelay[m_nReverbLoDlyPos] = echo >> 1;
        m_nReverbLoDlyPos = (m_nReverbLoDlyPos + 1) & 0x1F;

        SINT n = m_nReverbLoFltPos;
        m_nReverbLoFltSum -= m_reverbLoFilterBuffer[n];
        SINT tmp = echo / 128;
        m_reverbLoFilterBuffer[n] = tmp;
        m_nReverbLoFltSum += tmp;
        echodly -= m_nReverbLoFltSum;
        m_nReverbLoFltPos = (n + 1) & 0x3F;

        // reverb
        SINT v = (pr[0] + pr[1]) >> m_nFilterAttn;
        pr[0] += echodly;
        pr[1] += echodly;
        v += echodly >> 2;
        m_reverbBuffer3[m_nReverbBufferPos3] = v;
        m_reverbBuffer4[m_nReverbBufferPos4] = v;
        v += echodly >> 4;
        v >>= 1;

        m_gRvbLPSum -= m_gRvbLowPass[m_gRvbLPPos];
        m_gRvbLPSum += v;
        m_gRvbLowPass[m_gRvbLPPos] = v;
        m_gRvbLPPos = (m_gRvbLPPos + 1) & 7;

        SINT vlp = m_gRvbLPSum >> 2;
        m_reverbBuffer[m_nReverbBufferPos] = vlp;
        m_reverbBuffer2[m_nReverbBufferPos2] = vlp;

        if (++m_nReverbBufferPos >= m_nReverbSize)
            m_nReverbBufferPos = 0;
        if (++m_nReverbBufferPos2 >= m_nReverbSize2)
            m_nReverbBufferPos2 = 0;
        if (++m_nReverbBufferPos3 >= m_nReverbSize3)
            m_nReverbBufferPos3 = 0;
        if (++m_nReverbBufferPos4 >= m_nReverbSize4)
            m_nReverbBufferPos4 = 0;

        pr += 2;
    }
}

void TrackerDSP::processSurround(CSAMPLE* pSamples, SINT frameCount) {
    SINT* pr = reinterpret_cast<SINT*>(pSamples);
    SINT n = m_nDolbyLoFltPos;

    for (SINT i = 0; i < frameCount; ++i) {
        SINT v = (pr[0] + pr[1] + 1) >> 1; // rounding
        v *= m_nDolbyDepth;
        v >>= 8;

        // low-pass filter
        m_nDolbyHiFltSum -= m_dolbyHiFilterBuffer[m_nDolbyHiFltPos];
        m_dolbyHiFilterBuffer[m_nDolbyHiFltPos] = v;
        m_nDolbyHiFltSum += v;
        v = m_nDolbyHiFltSum;
        m_nDolbyHiFltPos = (m_nDolbyHiFltPos + 1) & DOLBY_HIFLT_MASK;

        // surround
        SINT secho = m_surroundBuffer[m_nSurroundPos];
        m_surroundBuffer[m_nSurroundPos] = v;

        // delay line and remove low frequencies
        v = m_dolbyLoFilterDelay[m_nDolbyLoDlyPos];
        m_dolbyLoFilterDelay[m_nDolbyLoDlyPos] = secho;
        m_nDolbyLoDlyPos = (m_nDolbyLoDlyPos + 1) & 0x1F;

        m_nDolbyLoFltSum -= m_dolbyLoFilterBuffer[n];
        SINT tmp = secho / 64;
        m_dolbyLoFilterBuffer[n] = tmp;
        m_nDolbyLoFltSum += tmp;
        v -= m_nDolbyLoFltSum;
        n = (n + 1) & 0x3F;

        // add echo (left positive, right negative for surround effect)
        pr[0] += v;
        pr[1] -= v;

        m_nSurroundPos = (m_nSurroundPos + 1) % m_nSurroundSize;
        pr += 2;
    }

    m_nDolbyLoFltPos = n;
}

void TrackerDSP::processMegabass(CSAMPLE* pSamples, SINT frameCount) {
    SINT* px = reinterpret_cast<SINT*>(pSamples);
    SINT xba = m_nXBassDepth + 1;
    SINT xbamask = (1 << xba) - 1;
    SINT n = m_nXBassBufferPos;

    for (SINT i = 0; i < frameCount; ++i) {
        m_nXBassSum -= m_xBassBuffer[n];
        SINT tmp0 = px[0] + px[1];
        SINT tmp = (tmp0 + ((tmp0 >> 31) & xbamask)) >> xba;
        m_xBassBuffer[n] = tmp;
        m_nXBassSum += tmp;

        SINT v = m_xBassDelay[m_nXBassDlyPos];
        m_xBassDelay[m_nXBassDlyPos] = px[0];
        px[0] = v + m_nXBassSum;

        v = m_xBassDelay[m_nXBassDlyPos + 1];
        m_xBassDelay[m_nXBassDlyPos + 1] = px[1];
        px[1] = v + m_nXBassSum;

        m_nXBassDlyPos = (m_nXBassDlyPos + 2) & m_nXBassMask;
        px += 2;
        n = (n + 1) & m_nXBassMask;
    }

    m_nXBassBufferPos = n;
}

void TrackerDSP::processNoiseReduction(CSAMPLE* pSamples, SINT frameCount) {
    SINT* pnr = reinterpret_cast<SINT*>(pSamples);
    SINT n1 = m_nLeftNR;
    SINT n2 = m_nRightNR;

    for (SINT i = 0; i < frameCount; ++i) {
        SINT vnr = pnr[0] >> 1;
        pnr[0] = vnr + n1;
        n1 = vnr;

        vnr = pnr[1] >> 1;
        pnr[1] = vnr + n2;
        n2 = vnr;

        pnr += 2;
    }

    m_nLeftNR = n1;
    m_nRightNR = n2;
}

} // namespace mixxx
