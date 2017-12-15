
#ifndef EFFECTPROCESSOR_H
#define EFFECTPROCESSOR_H

#include <QString>
#include <QHash>
#include <QPair>

#include "util/types.h"
#include "engine/engine.h"
#include "effects/defs.h"
#include "effects/effectsmanager.h"
#include "engine/effects/groupfeaturestate.h"
#include "engine/channelhandle.h"

class EngineEffect;

// Effects are implemented as two separate classes, an EffectState subclass and
// an EffectProcessorImpl subclass. Separating state from the DSP code allows
// memory allocation and deletion, which is slow, to be done on the main thread
// instead of potentially blocking the audio engine callback thread and causing
// audible glitches. EffectStates allocated on the main thread are passed as
// pointers to the EffectProcessorImpl in the audio callback thread via the
// effect MessagePipe FIFO (see EngineEffectsManager::onCallbackStart).

// Each EffectState instance is responsible for one routing of input signal to
// output signal. The base EffectProcessorImpl class handles the management
// of EffectStates. EffectProcessorImpl subclasses only need to be concerned
// with implementing the signal processing logic and providing metadata for
// describing the effect and its parameters.

// Input signals can be any EngineChannel, but output channels are hardcoded in
// EngineMaster as the post-fader processing for the master mix and pre-fader
// processing for headphones. EffectStates are allocated when an input signal is
// enabled for a chain. Also, when a new effect is loaded to a chain,
// EffectStates are only allocated for input signals that are enabled at that
// time. This allows for scaling up to an arbitrary number of input signals
// without wasting a lot of memory.
class EffectState {
  public:
    EffectState(const mixxx::EngineParameters& bufferParameters) {
        // Subclasses should call engineParametersChanged here.
        Q_UNUSED(bufferParameters);
    };
    virtual ~EffectState() {};

    // TODO: implement this for all subclasses and call it when the buffer
    // size and sample rate are configured by SoundManager
    virtual void engineParametersChanged(const mixxx::EngineParameters& bufferParameters) {
        Q_UNUSED(bufferParameters);
    };
    // Subclasses should clear any mixxx::SampleBuffer members and set
    // other values back to their defaults.
    virtual void clear() {};
};

// EffectProcessor is an abstract base class for interfacing with the main
// thread without needing to specify a specific EffectState subclass for the
// template in EffectProcessorImpl.
class EffectProcessor {
  public:
    virtual ~EffectProcessor() { }

    // Called from main thread to avoid allocating memory in the audio callback thread
    virtual void initialize(
            const QSet<ChannelHandleAndGroup>& activeInputChannels,
            EffectsManager* pEffectsManager,
            const mixxx::EngineParameters& bufferParameters) = 0;
    virtual EffectState* createState(const mixxx::EngineParameters& bufferParameters) = 0;
    virtual bool loadStatesForInputChannel(const ChannelHandle& inputChannel,
          const EffectStatesPointer pStates) = 0;
    // Called from main thread for garbage collection after the last audio thread
    // callback executes process() with EffectEnableState::Disabling
    virtual void deleteStatesForInputChannel(const ChannelHandle& inputChannel) = 0;

    // Take a buffer of audio samples as pInput, process the buffer according to
    // Effect-specific logic, and output it to the buffer pOutput. Both pInput
    // and pOutput are represented as stereo interleaved samples for now, but
    // effects should not be written assuming this will remain true. The properties
    // of the buffer necessary for determining how to process it (frames per
    // buffer, number of channels, and sample rate) are available on the
    // mixxx::EngineParameters argument. The provided channel handles allow
    // EffectProcessorImpl::process to fetch the appropriate EffectState and
    // pass it on to EffectProcessorImpl::processChannel, allowing one
    // EffectProcessor instance to process multiple signals simultaneously.
    virtual void process(const ChannelHandle& inputHandle,
                         const ChannelHandle& outputHandle,
                         const CSAMPLE* pInput, CSAMPLE* pOutput,
                         const mixxx::EngineParameters& bufferParameters,
                         const EffectEnableState enableState,
                         const GroupFeatureState& groupFeatures) = 0;
};

// EffectProcessorImpl manages a separate EffectState for every routing of
// input channel to output channel. This allows for processing effects in
// parallel for PFL and post-fader for the master output.
// EffectSpecificState must be a subclass of EffectState.
template <typename EffectSpecificState>
class EffectProcessorImpl : public EffectProcessor {
  public:
    EffectProcessorImpl() {
    }
    // Subclasses should not implement their own destructor. All state should
    // be stored in the EffectState subclass, not the EffectProcessorImpl subclass.
    ~EffectProcessorImpl() {
        //qDebug() << "~EffectProcessorImpl" << this;
        for (ChannelHandleMap<EffectSpecificState*>& outputsMap : m_channelStateMatrix) {
            for (EffectSpecificState* pState : outputsMap) {
                VERIFY_OR_DEBUG_ASSERT(pState != nullptr) {
                    continue;
                }
                //qDebug() << "~EffectProcessorImpl deleting state" << pState;
                delete pState;
            }
            outputsMap.clear();
        }
        m_channelStateMatrix.clear();
    };

    // NOTE: Subclasses must implement the following static methods for
    // EffectInstantiator to work:
    // static QString getId();
    // static EffectManifest getManifest();

    // This is the only non-static method that subclasses need to implement.
    // TODO(Be): remove ChannelHandle& argument? No (native) effects use it. Why should
    // effects be concerned with the ChannelHandle& when process() takes care of giving
    // it the appropriate ChannelStateHolder?
    virtual void processChannel(const ChannelHandle& handle,
                                EffectSpecificState* channelState,
                                const CSAMPLE* pInput, CSAMPLE* pOutput,
                                const mixxx::EngineParameters& bufferParameters,
                                const EffectEnableState enableState,
                                const GroupFeatureState& groupFeatures) = 0;

    void process(const ChannelHandle& inputHandle, const ChannelHandle& outputHandle,
                         const CSAMPLE* pInput, CSAMPLE* pOutput,
                         const mixxx::EngineParameters& bufferParameters,
                         const EffectEnableState enableState,
                         const GroupFeatureState& groupFeatures) final {
        EffectSpecificState* pState = m_channelStateMatrix[inputHandle][outputHandle];
        VERIFY_OR_DEBUG_ASSERT(pState != nullptr) {
            qWarning() << "EffectProcessorImpl::process could not retrieve"
                          "EffectState for input" << inputHandle
                       << "and output" << outputHandle
                       << "EffectState should have been preallocated in the main thread.";
            pState = createState(bufferParameters);
            m_channelStateMatrix[inputHandle][outputHandle] = pState;
        }
        processChannel(inputHandle, pState, pInput, pOutput, bufferParameters,
                       enableState, groupFeatures);
    }

   void initialize(const QSet<ChannelHandleAndGroup>& activeInputChannels,
            EffectsManager* pEffectsManager,
            const mixxx::EngineParameters& bufferParameters) final {
        for (const ChannelHandleAndGroup& inputChannel : activeInputChannels) {
            //qDebug() << this << "EffectProcessorImpl::initialize allocating EffectStates for input" << inputChannel;
            ChannelHandleMap<EffectSpecificState*> outputChannelMap;
            for (const ChannelHandleAndGroup& outputChannel :
                    pEffectsManager->registeredOutputChannels()) {
                outputChannelMap.insert(outputChannel.handle(),
                        createState(bufferParameters));
                //qDebug() << this << "EffectProcessorImpl::initialize registering output" << outputChannel;
            }
            m_channelStateMatrix.insert(inputChannel.handle(), outputChannelMap);
        }
        m_pEffectsManager = pEffectsManager;
        DEBUG_ASSERT(m_pEffectsManager != nullptr);
    };

    EffectSpecificState* createState(const mixxx::EngineParameters& bufferParameters) final {
        return new EffectSpecificState(bufferParameters);
    };

    bool loadStatesForInputChannel(const ChannelHandle& inputChannel,
              const EffectStatesPointer pStates) final {
          // Can't directly cast a ChannelHandleMap from containing the base
          // EffectState* type to EffectSpecificState* type, so iterate through
          // the ChannelHandleMap to build a new ChannelHandleMap with
          // dynamic_cast'ed states.
          ChannelHandleMap<EffectSpecificState*> newMap;
          //qDebug() << "EffectProcessorImpl::loadStatesForInputChannel" << this << "input"
          //         << inputChannel;
          for (const ChannelHandleAndGroup& outputChannel :
                  m_pEffectsManager->registeredOutputChannels()) {
              //qDebug() << "EffectProcessorImpl::loadStatesForInputChannel" << this << "output" << outputChannel;
              auto pState = dynamic_cast<EffectSpecificState*>(
                        pStates->at(outputChannel.handle()));
              VERIFY_OR_DEBUG_ASSERT(pState != nullptr) {
                    return false;
              }
              newMap.insert(outputChannel.handle(), pState);
          }
          m_channelStateMatrix.insert(inputChannel, newMap);
          return true;
    };

    // Called from main thread for garbage collection after an input channel is disabled
    void deleteStatesForInputChannel(const ChannelHandle& inputChannel) final {
          //qDebug() << "EffectProcessorImpl::deleteStatesForInputChannel" << this << inputChannel;
          for (EffectSpecificState* pState : m_channelStateMatrix.at(inputChannel)) {
                //qDebug() << "EffectProcessorImpl::deleteStatesForInputChannel" << this << "deleting state" << pState;
                delete pState;
          }
          m_channelStateMatrix[inputChannel].clear();
    };

  private:
    EffectsManager* m_pEffectsManager;
    ChannelHandleMap<ChannelHandleMap<EffectSpecificState*>> m_channelStateMatrix;
};

#endif /* EFFECTPROCESSOR_H */
