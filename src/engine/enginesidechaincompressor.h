#pragma once

#include "util/types.h"

class EngineSideChainCompressor {
  public:
    EngineSideChainCompressor(const QString& group);
    virtual ~EngineSideChainCompressor() { };

    void setParameters(CSAMPLE threshold, CSAMPLE strength,
                       unsigned int attack_time, unsigned int decay_time) {
        // TODO(owilliams): There is a race condition here because the parameters
        // are not updated atomically.  This function should instead take a
        // struct.
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

    /// Forces the above threshold flag to the given value without calculations
    void setAboveThreshold(bool value);

    // Every loop, before calling process, first call processKey to feed
    // the compressor the input key signal.  It is safe to call this function
    // multiple times for multiple keys, however they will not be summed together
    // so compression will not be triggered unless at least one buffer would
    // have triggered alone.
    void processKey(const CSAMPLE* pIn, const int iBufferSize);

    // Calculates a new gain value based on the current compression ratio
    // over the given number of frames and whether the current input is above threshold.
    double calculateCompressedGain(int frames);

  private:
    // Update the attack and decay rates.
    void calculateRates();

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
