#include <gtest/gtest.h>

#include <random>
#include <vector>

#include "engine/filters/enginefilterdelay.h"

namespace {

constexpr unsigned int kSize = 3300;

// The sample by sample ring buffer of EngineFilterDelay::process() for an
// unchanged delay, used as the reference for the block copy path.
class ReferenceDelay {
  public:
    ReferenceDelay()
            : m_buf(kSize, 0.0f),
              m_pos(0) {
    }

    // Only feeds the ring (the output of a block with a delay change is a
    // cross-fade that is not compared here).
    void write(const CSAMPLE* pIn, std::size_t bufferSize) {
        for (std::size_t i = 0; i < bufferSize; ++i) {
            m_buf[m_pos] = pIn[i];
            m_pos = (m_pos + 1) % kSize;
        }
    }

    void process(const CSAMPLE* pIn,
            CSAMPLE* pOutput,
            std::size_t bufferSize,
            unsigned int delaySamples) {
        std::size_t src = (m_pos + kSize - delaySamples) % kSize;
        for (std::size_t i = 0; i < bufferSize; ++i) {
            m_buf[m_pos] = pIn[i];
            m_pos = (m_pos + 1) % kSize;
            pOutput[i] = m_buf[src];
            src = (src + 1) % kSize;
        }
    }

  private:
    std::vector<CSAMPLE> m_buf;
    std::size_t m_pos;
};

TEST(EngineFilterDelayTest, MatchesSampleBySampleLoop) {
    std::mt19937 rng(1234);
    std::uniform_real_distribution<float> sampleDist(-1.0f, 1.0f);
    // Even buffer sizes (stereo), including sizes where buffer + delay do
    // not fit into the ring and the sample by sample loop is used
    std::uniform_int_distribution<int> framesDist(1, 2048);
    std::uniform_int_distribution<int> delayFramesDist(0, kSize / 2 - 1);

    for (int trial = 0; trial < 20; ++trial) {
        EngineFilterDelay<kSize> filter;
        ReferenceDelay reference;
        unsigned int delay = 2 * delayFramesDist(rng);
        filter.setDelay(delay);
        bool delayChanged = true;
        for (int block = 0; block < 200; ++block) {
            const std::size_t bufferSize = 2 * framesDist(rng);
            std::vector<CSAMPLE> input(bufferSize);
            for (auto& sample : input) {
                sample = sampleDist(rng);
            }
            std::vector<CSAMPLE> output(bufferSize);
            std::vector<CSAMPLE> expected(bufferSize);
            const bool inPlace = block % 3 == 0;
            if (inPlace) {
                output = input;
                filter.process(output.data(), output.data(), bufferSize);
            } else {
                filter.process(input.data(), output.data(), bufferSize);
            }
            if (delayChanged) {
                reference.write(input.data(), bufferSize);
                delayChanged = false;
            } else {
                reference.process(input.data(), expected.data(), bufferSize, delay);
                ASSERT_EQ(expected, output) << "trial " << trial << " block " << block
                                            << " bufferSize " << bufferSize
                                            << " delay " << delay;
            }
            if (block % 50 == 49) {
                delay = 2 * delayFramesDist(rng);
                filter.setDelay(delay);
                delayChanged = true;
            }
        }
    }
}

} // namespace
