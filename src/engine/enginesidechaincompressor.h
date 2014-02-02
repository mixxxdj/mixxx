#ifndef ENGINECOMPRESSOR_H
#define ENGINECOMPRESSOR_H

#include "defs.h"
#include "configobject.h"
#include "engine/engineobject.h"

class ControlObject;

class EngineSideChainCompressor : public EngineObject {
    Q_OBJECT
  public:
    EngineSideChainCompressor(ConfigObject<ConfigValue>* pConfig, const char* group);
    virtual ~EngineSideChainCompressor() { };

    void setParameters(CSAMPLE threshold, CSAMPLE strength,
                       unsigned int attack_time, unsigned int decay_time) {
        m_threshold = threshold;
        m_strength = strength;
        m_attackTime = attack_time;
        m_decayTime = decay_time;
        calculateRates();
    }

    void setThreshold(CSAMPLE threshold) {
        m_threshold = threshold;
        calculateRates();
    }

    void setStrength(CSAMPLE strength) {
        m_strength = strength;
        calculateRates();
    }

    void setAttackTime(unsigned int attack_time) {
        m_attackTime = attack_time;
        calculateRates();
    }

    void setDecayTime(unsigned int decay_time) {
        m_decayTime = decay_time;
        calculateRates();
    }

    // Every loop, before calling process, first call processKey to feed
    // the compressor the input key signal.
    void processKey(const CSAMPLE* pIn, const int iBufferSize);

    void process(const CSAMPLE* pIn, CSAMPLE* pOut, const int iBufferSize);

  private:
    // Update the attack and decay rates.
    void calculateRates();

    // Calculates a new compression value for the next frame
    // based on the current compression ratio and whether the current input is above threshold.
    inline double calculateCompression(CSAMPLE currentRatio, bool aboveThreshold) const;

    ConfigObject<ConfigValue>* m_pConfig;

    // The current ratio the signal is being compressed.  This is the same as m_strength
    // when the compressor is at maximum engagement (not attacking or decaying).
    CSAMPLE m_compressRatio;

    // True if the input signal is above the threshold.
    bool m_bAboveThreshold;

    // The sample value above which the compressor is triggered.
    CSAMPLE m_threshold;

    // The largest ratio the signal can be compressed.
    CSAMPLE m_strength;

    // The length of time, in frames (samples/2), until maximum compression is reached.
    unsigned int m_attackTime;

    // The length of time, in frames, until compression is completely off.
    unsigned int m_decayTime;

    // These are the delta compression values per sample based on the strengths and timings.
    CSAMPLE m_attackPerFrame;
    CSAMPLE m_decayPerFrame;
};

#endif
