#ifndef EFFECTPROCESSOR_H
#define EFFECTPROCESSOR_H

#include <QString>
#include <QMap>

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

// Helper class for automatically fetching group state parameters upon receipt
// of a group-specific process call.
template <typename T>
class GroupEffectProcessor : public EffectProcessor {
  public:
    GroupEffectProcessor() {}
    virtual ~GroupEffectProcessor() {
        for (typename QMap<QString, T*>::iterator it = m_groupState.begin();
             it != m_groupState.end();) {
            T* pState = it.value();
            it = m_groupState.erase(it);
            delete pState;
        }
    }

    virtual void process(const QString& group,
                         const CSAMPLE* pInput, CSAMPLE* pOutput,
                         const unsigned int numSamples) {
        T* pState = m_groupState.value(group, NULL);
        if (pState == NULL) {
            pState = new T();
            m_groupState[group] = pState;
        }
        processGroup(group, pState, pInput, pOutput, numSamples);
    }

    virtual void processGroup(const QString& group,
                              T* groupState,
                              const CSAMPLE* pInput, CSAMPLE* pOutput,
                              const unsigned int numSamples) = 0;

  private:
    QMap<QString, T*> m_groupState;
};

#endif /* EFFECTPROCESSOR_H */
