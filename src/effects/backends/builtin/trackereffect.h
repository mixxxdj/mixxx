// Tracker DSP effect — reverb, megabass, surround, noise reduction
// Ported from libmodplug's public domain DSP implementation.
// This effect provides the libmodplug DSP effects as a Mixxx effect rack
// processor, allowing them to be applied during mixing rather than baked
// into the decoded audio.

#pragma once

#include <vector>

#include "effects/backends/effectprocessor.h"
#include "util/class.h"
#include "util/types.h"

class TrackerEffectGroupState : public EffectState {
  public:
    explicit TrackerEffectGroupState(const mixxx::EngineParameters& engineParameters)
            : EffectState(engineParameters),
              m_sampleRate(engineParameters.sampleRate()) {
        // pre-allocate all DSP buffers (no allocations in audio thread)
        m_xBassBuffer.assign(128, 0);
        m_xBassDelay.assign(128, 0);
        m_dolbyLoFilterBuffer.assign(64, 0);
        m_dolbyLoFilterDelay.assign(32, 0);
        m_dolbyHiFilterBuffer.assign(256, 0);
        m_surroundBuffer.assign(8192, 0);
        m_reverbLoFilterBuffer.assign(64, 0);
        m_reverbLoFilterDelay.assign(32, 0);
        m_reverbBuffer.assign(8192, 0);
        m_reverbBuffer2.assign(6128, 0);
        m_reverbBuffer3.assign(4432, 0);
        m_reverbBuffer4.assign(2964, 0);
        m_gRvbLowPass.assign(8, 0);
        // pre-allocate int conversion buffer (max engine buffer size)
        m_intBuffer.assign(engineParameters.samplesPerBuffer() * 2, 0);
    }
    ~TrackerEffectGroupState() override = default;

    void setEngineParameters(const mixxx::EngineParameters& engineParameters) {
        m_sampleRate = engineParameters.sampleRate();
        const SINT requiredSize = engineParameters.samplesPerBuffer() * 2;
        if (static_cast<SINT>(m_intBuffer.size()) < requiredSize) {
            m_intBuffer.assign(requiredSize, 0);
        }
        reset();
    }

    void reset();

    mixxx::audio::SampleRate m_sampleRate;

    // shared int conversion buffer (no per-call allocation)
    std::vector<SINT> m_intBuffer;

    // bass expansion state
    SINT m_nXBassDepth{6};
    SINT m_nXBassRange{14};
    SINT m_nXBassSum{0};
    SINT m_nXBassBufferPos{0};
    SINT m_nXBassDlyPos{0};
    SINT m_nXBassMask{0};
    std::vector<SINT> m_xBassBuffer;
    std::vector<SINT> m_xBassDelay;

    // noise reduction state
    SINT m_nLeftNR{0};
    SINT m_nRightNR{0};

    // pro-logic surround state
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
    std::vector<SINT> m_dolbyLoFilterBuffer;
    std::vector<SINT> m_dolbyLoFilterDelay;
    std::vector<SINT> m_dolbyHiFilterBuffer;
    std::vector<SINT> m_surroundBuffer;

    // reverb state
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

class TrackerEffect : public EffectProcessorImpl<TrackerEffectGroupState> {
  public:
    TrackerEffect() = default;
    ~TrackerEffect() override = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            TrackerEffectGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

  private:
    QString debugString() const {
        return getId();
    }

    void processReverb(TrackerEffectGroupState* pState,
            SINT* pr,
            SINT frameCount);
    void processSurround(TrackerEffectGroupState* pState,
            SINT* pr,
            SINT frameCount);
    void processMegabass(TrackerEffectGroupState* pState,
            SINT* px,
            SINT frameCount);
    void processNoiseReduction(TrackerEffectGroupState* pState,
            SINT* pnr,
            SINT frameCount);

    void configureReverb(TrackerEffectGroupState* pState,
            int depth,
            int delayMs);
    void configureSurround(TrackerEffectGroupState* pState,
            int depth,
            int delayMs);
    void configureMegabass(TrackerEffectGroupState* pState,
            int depth);

    EngineEffectParameterPointer m_pReverbParameter;
    EngineEffectParameterPointer m_pReverbDepthParameter;
    EngineEffectParameterPointer m_pReverbDelayParameter;
    EngineEffectParameterPointer m_pMegabassParameter;
    EngineEffectParameterPointer m_pBassDepthParameter;
    EngineEffectParameterPointer m_pSurroundParameter;
    EngineEffectParameterPointer m_pSurroundDepthParameter;
    EngineEffectParameterPointer m_pSurroundDelayParameter;
    EngineEffectParameterPointer m_pNoiseReductionParameter;

    DISALLOW_COPY_AND_ASSIGN(TrackerEffect);
};
