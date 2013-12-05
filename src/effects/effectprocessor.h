#ifndef EFFECTPROCESSOR_H
#define EFFECTPROCESSOR_H

#include <QString>

#include "defs.h"

class EngineEffect;

class EffectProcessor {
  public:
    virtual ~EffectProcessor() { }

    // Take a buffer of numSamples samples of audio from group, provided as
    // pInput, process the buffer according to Effect-specific logic, and output
    // it to the buffer pOutput. If pInput is equal to pOutput, then the
    // operation must occur in-place. Both pInput and pOutput are represented as
    // stereo interleaved samples. There are numSamples total samples, so
    // numSamples/2 left channel samples and numSamples/2 right channel
    // samples. The group provided allows the effect to maintain state on a
    // per-group basis. This is important because one Effect instance may be
    // used to process the audio of multiple channels.
    virtual void process(const QString& group,
                         const CSAMPLE* pInput, CSAMPLE* pOutput,
                         const unsigned int numSamples) = 0;
};

#endif /* EFFECTPROCESSOR_H */
