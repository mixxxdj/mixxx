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
            const QSet<ChannelHandleAndGroup>& registeredInputChannels,
            const QSet<ChannelHandleAndGroup>& registeredOutputChannels) = 0;

    // Take a buffer of numSamples samples of audio from a channel, provided as
    // pInput, process the buffer according to Effect-specific logic, and output
    // it to the buffer pOutput. If pInput is equal to pOutput, then the
    // operation must occur in-place. Both pInput and pOutput are represented as
    // stereo interleaved samples. There are numSamples total samples, so
    // numSamples/2 left channel samples and numSamples/2 right channel
    // samples. The provided channel handle allows the effect to maintain state
    // on a per-channel basis. This is important because one Effect instance may
    // be used to process the audio of multiple channels.
    virtual void process(const ChannelHandle& inputHandle, const ChannelHandle& outputHandle,
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
        // loop over each output channel
        for (auto&& outputsMap : m_channelStateMatrix) {
            // loop over each input channel
            for (typename ChannelHandleMap<ChannelStateHolder>::iterator it =
                        outputsMap.begin();
                        it != outputsMap.end(); ++it) {
                T* pState = it->state;
                delete pState;
            }
            outputsMap.clear();
        }
        m_channelStateMatrix.clear();
    }

    virtual void initialize(
            const QSet<ChannelHandleAndGroup>& registeredInputChannels,
            const QSet<ChannelHandleAndGroup>& registeredOutputChannels) {
        for (const ChannelHandleAndGroup& inputChannel : registeredInputChannels) {
            ChannelHandleMap<ChannelStateHolder> outputChannelMap;
            m_channelStateMatrix.insert(inputChannel.handle(), outputChannelMap);
            for (const ChannelHandleAndGroup& outputChannel : registeredOutputChannels) {
                getOrCreateChannelState(inputChannel.handle(), outputChannel.handle());
            }
        }
    }

    virtual void process(const ChannelHandle& inputHandle, const ChannelHandle& outputHandle,
                         const CSAMPLE* pInput, CSAMPLE* pOutput,
                         const unsigned int numSamples,
                         const unsigned int sampleRate,
                         const EffectProcessor::EnableState enableState,
                         const GroupFeatureState& groupFeatures) {
        T* pState = getOrCreateChannelState(inputHandle, outputHandle);
        processChannel(inputHandle, pState, pInput, pOutput, numSamples, sampleRate,
                       enableState, groupFeatures);
    }

    // TODO(Be): remove ChannelHandle& argument? No (native) effects use it. Why should
    // effects be concerned with the ChannelHandle& when process() takes care of giving
    // it the appropriate ChannelStateHolder?
    virtual void processChannel(const ChannelHandle& handle,
                                T* channelState,
                                const CSAMPLE* pInput, CSAMPLE* pOutput,
                                const unsigned int numSamples,
                                const unsigned int sampleRate,
                                const EffectProcessor::EnableState enableState,
                                const GroupFeatureState& groupFeatures) = 0;

  private:
    inline T* getOrCreateChannelState(const ChannelHandle& inputHandle,
                                      const ChannelHandle& outputHandle) {
        ChannelStateHolder& holder = m_channelStateMatrix[inputHandle][outputHandle];
        if (holder.state == NULL) {
            holder.state = new T();
        }
        return holder.state;
    }

    ChannelHandleMap<ChannelHandleMap<ChannelStateHolder>> m_channelStateMatrix;
};

#endif /* EFFECTPROCESSOR_H */
