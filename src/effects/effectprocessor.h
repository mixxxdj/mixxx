#pragma once

#include <QString>
#include <QHash>
#include <QDebug>
#include <QPair>

#include "util/types.h"
#include "engine/engine.h"
#include "effects/defs.h"
#include "engine/effects/groupfeaturestate.h"
#include "engine/effects/message.h"
#include "engine/channelhandle.h"
#include "effects/effectsmanager.h"

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
    virtual bool loadStatesForInputChannel(const ChannelHandle* inputChannel,
          const EffectStatesMap* pStatesMap) = 0;
    // Called from main thread for garbage collection after the last audio thread
    // callback executes process() with EffectEnableState::Disabling
    virtual void deleteStatesForInputChannel(const ChannelHandle* inputChannel) = 0;

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
    EffectProcessorImpl()
      : m_pEffectsManager(nullptr) {
    }
    // Subclasses should not implement their own destructor. All state should
    // be stored in the EffectState subclass, not the EffectProcessorImpl subclass.
    ~EffectProcessorImpl() {
        if (kEffectDebugOutput) {
            qDebug() << "~EffectProcessorImpl" << this;
        }
        int inputChannelHandleNumber = 0;
        for (ChannelHandleMap<EffectSpecificState*>& outputsMap : m_channelStateMatrix) {
            int outputChannelHandleNumber = 0;
            for (EffectSpecificState* pState : outputsMap) {
                VERIFY_OR_DEBUG_ASSERT(pState != nullptr) {
                    continue;
                }
                if (kEffectDebugOutput) {
                    qDebug() << "~EffectProcessorImpl deleting EffectState" << pState
                             << "for input ChannelHandle(" << inputChannelHandleNumber << ")"
                             << "and output ChannelHandle(" << outputChannelHandleNumber << ")";
                }
                delete pState;
                outputChannelHandleNumber++;
            }
            outputsMap.clear();
            inputChannelHandleNumber++;
        }
        m_channelStateMatrix.clear();
    };

    // NOTE: Subclasses must implement the following static methods for
    // EffectInstantiator to work:
    // static QString getId();
    // static EffectManifest getManifest();

    // This is the only non-static method that subclasses need to implement.
    // TODO(Be): remove ChannelHandle& argument? No (built-in) effects use it. Why should
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
            if (kEffectDebugOutput) {
                qWarning() << "EffectProcessorImpl::process could not retrieve"
                              "EffectState for input" << inputHandle
                           << "and output" << outputHandle
                           << "EffectState should have been preallocated in the"
                              "main thread.";
            }
            pState = createSpecificState(bufferParameters);
            m_channelStateMatrix[inputHandle][outputHandle] = pState;
        }
        processChannel(inputHandle, pState, pInput, pOutput, bufferParameters,
                       enableState, groupFeatures);
    }

    void initialize(const QSet<ChannelHandleAndGroup>& activeInputChannels,
            EffectsManager* pEffectsManager,
            const mixxx::EngineParameters& bufferParameters) final {
        for (const ChannelHandleAndGroup& inputChannel : activeInputChannels) {
            if (kEffectDebugOutput) {
                qDebug() << this << "EffectProcessorImpl::initialize allocating "
                            "EffectStates for input" << inputChannel;
            }
            ChannelHandleMap<EffectSpecificState*> outputChannelMap;
            for (const ChannelHandleAndGroup& outputChannel :
                    pEffectsManager->registeredOutputChannels()) {
                outputChannelMap.insert(outputChannel.handle(),
                        createSpecificState(bufferParameters));
                if (kEffectDebugOutput) {
                    qDebug() << this << "EffectProcessorImpl::initialize "
                                "registering output" << outputChannel << outputChannelMap[outputChannel.handle()];
                }
            }
            m_channelStateMatrix.insert(inputChannel.handle(), outputChannelMap);
        }
        m_pEffectsManager = pEffectsManager;
        DEBUG_ASSERT(m_pEffectsManager != nullptr);
    };

    EffectState* createState(const mixxx::EngineParameters& bufferParameters) final {
        return createSpecificState(bufferParameters);
    };

    bool loadStatesForInputChannel(const ChannelHandle* inputChannel,
              const EffectStatesMap* pStatesMap) final {
          if (kEffectDebugOutput) {
              qDebug() << "EffectProcessorImpl::loadStatesForInputChannel" << this
                       << "input" << *inputChannel;
          }

          // NOTE: ChannelHandleMap is like a map in that it associates an
          // object with a ChannelHandle key, but it actually backed by a
          // QVarLengthArray, not a QMap. So it is okay that
          // m_channelStateMatrix may be accessed concurrently in the main
          // thread in deleteStatesForInputChannel.

          // Can't directly cast a ChannelHandleMap from containing the base
          // EffectState* type to EffectSpecificState* type, so iterate through
          // pStatesMap to build a new ChannelHandleMap with
          // dynamic_cast'ed states.
          ChannelHandleMap<EffectSpecificState*>& effectSpecificStatesMap =
                  m_channelStateMatrix[*inputChannel];

          // deleteStatesForInputChannel should have been called before a new
          // map of EffectStates was sent to this function, or this is the first
          // time states are being loaded for this input channel, so
          // effectSpecificStatesMap should be empty and this loop should
          // not go through any iterations.
          for (EffectSpecificState* pState : effectSpecificStatesMap) {
              VERIFY_OR_DEBUG_ASSERT(pState == nullptr) {
                  delete pState;
              }
          }

          for (const ChannelHandleAndGroup& outputChannel :
                  m_pEffectsManager->registeredOutputChannels()) {
              if (kEffectDebugOutput) {
                  qDebug() << "EffectProcessorImpl::loadStatesForInputChannel"
                           << this << "output" << outputChannel;
              }

              auto pState = dynamic_cast<EffectSpecificState*>(
                        pStatesMap->at(outputChannel.handle()));
              VERIFY_OR_DEBUG_ASSERT(pState != nullptr) {
                    return false;
              }
              effectSpecificStatesMap.insert(outputChannel.handle(), pState);
          }
          return true;
    };

    // Called from main thread for garbage collection after an input channel is disabled
    void deleteStatesForInputChannel(const ChannelHandle* inputChannel) final {
          if (kEffectDebugOutput) {
              qDebug() << "EffectProcessorImpl::deleteStatesForInputChannel"
                       << this << *inputChannel;
          }

          // NOTE: ChannelHandleMap is like a map in that it associates an
          // object with a ChannelHandle key, but it actually backed by a
          // QVarLengthArray, not a QMap. So it is okay that
          // m_channelStateMatrix may be accessed concurrently in the audio
          // engine thread in loadStatesForInputChannel.

          ChannelHandleMap<EffectSpecificState*>& stateMap =
                  m_channelStateMatrix[*inputChannel];
          for (EffectSpecificState* pState : stateMap) {
                VERIFY_OR_DEBUG_ASSERT(pState != nullptr) {
                      continue;
                }
                if (kEffectDebugOutput) {
                      qDebug() << "EffectProcessorImpl::deleteStatesForInputChannel"
                               << this << "deleting state" << pState;
                }
                delete pState;
          }
          stateMap.clear();
    };

  private:

    EffectSpecificState* createSpecificState(const mixxx::EngineParameters& bufferParameters) {
        EffectSpecificState* pState = new EffectSpecificState(bufferParameters);
        if (kEffectDebugOutput) {
            qDebug() << this << "EffectProcessorImpl creating EffectState" << pState;
        }
        return pState;
    };

    EffectsManager* m_pEffectsManager;
    ChannelHandleMap<ChannelHandleMap<EffectSpecificState*>> m_channelStateMatrix;
};
