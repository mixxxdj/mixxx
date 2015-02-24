#ifndef EFFECTPROCESSOR_H
#define EFFECTPROCESSOR_H

#include <QString>
#include <QHash>
#include <QPair>

#include "util/types.h"
#include "engine/effects/groupfeaturestate.h"
#include "engine/channelhandle.h"

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

    virtual void initialize(
            const QSet<ChannelHandleAndGroup>& registeredChannels) = 0;

    // Take a buffer of numSamples samples of audio from a channel, provided as
    // pInput, process the buffer according to Effect-specific logic, and output
    // it to the buffer pOutput. If pInput is equal to pOutput, then the
    // operation must occur in-place. Both pInput and pOutput are represented as
    // stereo interleaved samples. There are numSamples total samples, so
    // numSamples/2 left channel samples and numSamples/2 right channel
    // samples. The provided channel handle allows the effect to maintain state
    // on a per-channel basis. This is important because one Effect instance may
    // be used to process the audio of multiple channels.
    virtual void process(const ChannelHandle& handle,
                         const CSAMPLE* pInput, CSAMPLE* pOutput,
                         const unsigned int numSamples,
                         const unsigned int sampleRate,
                         const enum EnableState enableState,
                         const GroupFeatureState& groupFeatures) = 0;
};

// Helper class for automatically fetching channel state parameters upon receipt
// of a channel-specific process call.
template <typename T>
class PerChannelEffectProcessor : public EffectProcessor {
    struct ChannelStateHolder {
        ChannelStateHolder() : state(NULL) { }
        T* state;
    };
  public:
    PerChannelEffectProcessor() {
    }
    virtual ~PerChannelEffectProcessor() {
        for (typename ChannelHandleMap<ChannelStateHolder>::iterator it =
                     m_channelState.begin();
             it != m_channelState.end(); ++it) {
            T* pState = it->state;
            delete pState;
        }
        m_channelState.clear();
    }

    virtual void initialize(
            const QSet<ChannelHandleAndGroup>& registeredChannels) {
        foreach (const ChannelHandleAndGroup& channel, registeredChannels) {
            getOrCreateChannelState(channel.handle());
        }
    }

    virtual void process(const ChannelHandle& handle,
                         const CSAMPLE* pInput, CSAMPLE* pOutput,
                         const unsigned int numSamples,
                         const unsigned int sampleRate,
                         const EffectProcessor::EnableState enableState,
                         const GroupFeatureState& groupFeatures) {
        T* pState = getOrCreateChannelState(handle);
        processChannel(handle, pState, pInput, pOutput, numSamples, sampleRate,
                       enableState, groupFeatures);
    }

    virtual void processChannel(const ChannelHandle& handle,
                                T* channelState,
                                const CSAMPLE* pInput, CSAMPLE* pOutput,
                                const unsigned int numSamples,
                                const unsigned int sampleRate,
                                const EffectProcessor::EnableState enableState,
                                const GroupFeatureState& groupFeatures) = 0;

  private:
    inline T* getOrCreateChannelState(const ChannelHandle& handle) {
        ChannelStateHolder& holder = m_channelState[handle];
        if (holder.state == NULL) {
            holder.state = new T();
        }
        return holder.state;
    }

    ChannelHandleMap<ChannelStateHolder> m_channelState;
};

#endif /* EFFECTPROCESSOR_H */
