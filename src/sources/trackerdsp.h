#pragma once

#include <vector>

#include "audio/types.h"
#include "util/types.h"

namespace mixxx {

/// @brief post-processing DSP effects for tracker modules
/// ported from libmodplug's public domain DSP implementation
/// to maintain compatibility when using libopenmpt
class TrackerDSP {
  public:
    struct Settings {
        // reverb: 4 delay lines + filters
        bool reverbEnabled{false};
        int reverbDepth{50}; // 0-100
        int reverbDelay{50}; // ms, typically 40-200

        // megabass/bass expansion: low-pass filter
        bool megabassEnabled{false};
        int bassDepth{50};  // 0-100
        int bassCutoff{50}; // hz, 10-100

        // pro-logic surround: delay + filters
        bool surroundEnabled{false};
        int surroundDepth{50}; // 0-100
        int surroundDelay{50}; // ms, typically 5-40

        // noise reduction: simple low-pass
        bool noiseReductionEnabled{false};
    };

    TrackerDSP();
    ~TrackerDSP();

    void configure(const Settings& settings, audio::SampleRate sampleRate);
    void reset();

    /// @brief process stereo interleaved samples in-place
    void processStereo(CSAMPLE* pSamples, SINT frameCount);

  private:
    void initializeBuffers(bool fullReset);

    // DSP effect implementations
    void processReverb(CSAMPLE* pSamples, SINT frameCount);
    void processSurround(CSAMPLE* pSamples, SINT frameCount);
    void processMegabass(CSAMPLE* pSamples, SINT frameCount);
    void processNoiseReduction(CSAMPLE* pSamples, SINT frameCount);

    Settings m_settings;
    audio::SampleRate m_sampleRate;
    bool m_initialized{false};

    // BUFFER SIZE CONSTANTS
    static constexpr int XBASSBUFFERSIZE = 128;
    static constexpr int FILTERBUFFERSIZE = 256;
    static constexpr int SURROUNDBUFFERSIZE = 8192;
    static constexpr int REVERBBUFFERSIZE = 8192;
    static constexpr int REVERBBUFFERSIZE2 = 6128;
    static constexpr int REVERBBUFFERSIZE3 = 4432;
    static constexpr int REVERBBUFFERSIZE4 = 2964;

    // BASS EXPANSION STATE
    SINT m_nXBassDepth{6};
    SINT m_nXBassRange{14}; // 2.5 ms
    SINT m_nXBassSum{0};
    SINT m_nXBassBufferPos{0};
    SINT m_nXBassDlyPos{0};
    SINT m_nXBassMask{0};
    std::vector<SINT> m_xBassBuffer;
    std::vector<SINT> m_xBassDelay;

    // NOISE REDUCTION STATE
    SINT m_nLeftNR{0};
    SINT m_nRightNR{0};

    // SURROUND STATE
    SINT m_nProLogicDepth{12};
    SINT m_nProLogicDelay{20};
    SINT m_nSurroundSize{0};
    SINT m_nSurroundPos{0};
    SINT m_nDolbyDepth{0};
    SINT m_nDolbyLoDlyPos{0};
    SINT m_nDolbyLoFltPos{0};
    SINT m_nDolbyLoFltSum{0};
    SINT m_nDolbyHiFltPos{0};
    SINT m_nDolbyHiFltSum{0};
    static constexpr int DOLBY_HIFLT_SIZE = 32;
    static constexpr int DOLBY_HIFLT_MASK = 31;
    std::vector<SINT> m_dolbyLoFilterBuffer;
    std::vector<SINT> m_dolbyLoFilterDelay;
    std::vector<SINT> m_dolbyHiFilterBuffer;
    std::vector<SINT> m_surroundBuffer;

    // REVERB STATE
    SINT m_nReverbDepth{1};
    SINT m_nReverbDelay{100};
    SINT m_nReverbSize{0};
    SINT m_nReverbBufferPos{0};
    SINT m_nReverbSize2{0};
    SINT m_nReverbBufferPos2{0};
    SINT m_nReverbSize3{0};
    SINT m_nReverbBufferPos3{0};
    SINT m_nReverbSize4{0};
    SINT m_nReverbBufferPos4{0};
    SINT m_nReverbLoFltSum{0};
    SINT m_nReverbLoFltPos{0};
    SINT m_nReverbLoDlyPos{0};
    SINT m_nFilterAttn{0};
    SINT m_gRvbLPPos{0};
    SINT m_gRvbLPSum{0};
    std::vector<SINT> m_reverbLoFilterBuffer;
    std::vector<SINT> m_reverbLoFilterDelay;
    std::vector<SINT> m_reverbBuffer;
    std::vector<SINT> m_reverbBuffer2;
    std::vector<SINT> m_reverbBuffer3;
    std::vector<SINT> m_reverbBuffer4;
    std::vector<SINT> m_gRvbLowPass;
};

} // namespace mixxx
