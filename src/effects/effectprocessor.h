#ifndef EFFECTPROCESSOR_H
#define EFFECTPROCESSOR_H

#include <QString>
#include <QLinkedList>

#include "util/types.h"
#include "engine/effects/groupfeaturestate.h"

class EngineEffect;

class EffectProcessor {
  public:
    enum EnableState {
        DISABLED = 0x00,
        ENABLED = 0x01,
        DISABLING = 0x02,
        ENABLING = 0x03
    };


    virtual ~EffectProcessor() { }

    virtual void initialize(const QSet<QString>& registeredGroups) = 0;

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
                         const unsigned int numSamples,
                         const unsigned int sampleRate,
                         const enum EnableState enableState,
                         const GroupFeatureState& groupFeatures) = 0;
};

// Helper class for automatically fetching group state parameters upon receipt
// of a group-specific process call.
template <typename T>
class GroupEffectProcessor : public EffectProcessor {
    struct GroupStatePair {
        QString group;
        T* state;
    };
  public:
    GroupEffectProcessor() {
    }
    virtual ~GroupEffectProcessor() {
        for (typename QLinkedList<GroupStatePair>::iterator it = m_groupState.begin();
                it != m_groupState.end();) {
            T* pState = it->state;
            it = m_groupState.erase(it);
            delete pState;
        }
    }

    virtual void initialize(const QSet<QString>& registeredGroups) {
        foreach (const QString& group, registeredGroups) {
            getOrCreateGroupState(group);
        }
    }

    virtual void process(const QString& group,
                         const CSAMPLE* pInput, CSAMPLE* pOutput,
                         const unsigned int numSamples,
                         const unsigned int sampleRate,
                         const EffectProcessor::EnableState enableState,
                         const GroupFeatureState& groupFeatures) {
        T* pState = getOrCreateGroupState(group);
        processGroup(group, pState, pInput, pOutput, numSamples, sampleRate,
                     enableState, groupFeatures);
    }

    virtual void processGroup(const QString& group,
                              T* groupState,
                              const CSAMPLE* pInput, CSAMPLE* pOutput,
                              const unsigned int numSamples,
                              const unsigned int sampleRate,
                              const EffectProcessor::EnableState enableState,
                              const GroupFeatureState& groupFeatures) = 0;

  private:
    T* getOrCreateGroupState(const QString& group) {
        for (typename QLinkedList<GroupStatePair>::const_iterator it = m_groupState.constBegin(),
                     end = m_groupState.constEnd(); it != end; ++it) {
            const GroupStatePair& pair = *it;
            if (pair.group == group) {
                return pair.state;
            }
        }
        GroupStatePair pair;
        pair.group = group;
        pair.state = new T();
        m_groupState.append(pair);
        return pair.state;
    }

    QLinkedList<GroupStatePair> m_groupState;
};

#endif /* EFFECTPROCESSOR_H */
