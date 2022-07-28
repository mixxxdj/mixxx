#pragma once

#include <QDebug>
#include <QHash>
#include <QPair>
#include <QString>

#include "effects/defs.h"
#include "engine/effects/groupfeaturestate.h"
#include "engine/effects/message.h"
#include "engine/engine.h"
#include "util/types.h"

/// Effects are implemented as two separate classes, an EffectState subclass and
/// an EffectProcessorImpl subclass. Separating state from the DSP code allows
/// memory allocation and deletion on the heap, which is slow, to be done on the
/// main thread instead of potentially blocking the audio engine callback thread
/// and causing audible glitches.
///
/// The base EffectState class does nothing on its own; each effect is responsible
/// for subclassing it with whatever state it needs to maintain between cycles of
/// the audio thread. The base EffectProcessorImpl class handles the management of
/// EffectStates. EffectProcessorImpl subclasses only need to be concerned with
/// implementing the signal processing logic and providing metadata for describing
/// the effect and its parameters with an EffectManifest.
///
/// EffectProcessorImpl interfaces with the main thread through the EffectProcessor
/// abstract base class. EngineEffect passes state changes between the
/// ControlObjects in EffectSlot on the main thread and EffectProcessor in the audio
/// thread via EffectsMessenger.
///
/// Each EffectState instance tracks the state for one combination of input signal
/// and output signal. Input signals can be any EngineChannel, but output channels
/// are hardcoded in EngineMaster as the postfader processing for the main mix
/// and prefader processing for headphones. There can be many EffectStates for one
/// EffectProcessorImpl, allowing a single EffectProcessorImpl to maintain
/// independent state for each combination of input and output signal. This allows
/// each EffectProcessor to handle an arbitrary number of input signals. Tracking
/// state separately for the main mix and the headphone output allows effects to be
/// processed postfader for the main mix and prefader for the headphone output in
/// parallel so there is no need for a prefader/postfader toggle switch.
///
/// EffectStates allocated on the main thread are passed as pointers to the
/// EffectProcessorImpl in the audio callback thread via the EffectsMessenger.
/// EffectStates are allocated and deallocated when a routing switch for an
/// EffectChain is toggled and when a new EngineEffect is loaded into an EffectSlot.
/// This allows for scaling up to an arbitrary number of input signals
/// without wasting a lot of memory. (EffectStates could be (de)allocated when toggling
/// the enable switches for EffectSlots as well, but the memory savings would be
/// relatively small compared to the additional code complexity.)
class EffectState {
  public:
    EffectState(const mixxx::EngineParameters& engineParameters) {
        // Subclasses should call engineParametersChanged here.
        Q_UNUSED(engineParameters);
    };
    virtual ~EffectState(){};
};

/// EffectProcessor is an abstract base class for interfacing with an EffectSlot
/// in the main thread without needing to specify a specific EffectState subclass
/// for the template in EffectProcessorImpl.
class EffectProcessor {
  public:
    virtual ~EffectProcessor() {
    }

    /// These methods are called from the main thread
    virtual void initialize(
            const QSet<GroupHandle>& activeInputChannels,
            const QSet<GroupHandle>& registeredOutputChannels,
            const mixxx::EngineParameters& engineParameters) = 0;
    virtual void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) = 0;
    virtual EffectState* createState(const mixxx::EngineParameters& engineParameters) = 0;
    virtual void deleteStatesForInputChannel(GroupHandle pInputChannel) = 0;

    // Called from the audio thread
    virtual bool loadStatesForInputChannel(GroupHandle inputChannel,
            const EffectStatesMap* pStatesMap) = 0;

    /// Called from the audio thread
    /// This method takes a buffer of audio samples as pInput, processes the buffer
    /// according to effect-specific logic, and outputs it to the buffer pOutput.
    /// Both pInput and pOutput are represented as stereo interleaved samples for now,
    /// but effects should not be written assuming this will remain true. The properties
    /// of the buffer necessary for determining how to process it (frames per
    /// buffer, number of channels, and sample rate) are available on the
    /// mixxx::EngineParameters argument. The provided channel handles allow
    /// EffectProcessorImpl::process to fetch the appropriate EffectState and
    /// pass it on to EffectProcessorImpl::processChannel, allowing one
    /// EffectProcessor instance to process multiple signals simultaneously.
    virtual void process(GroupHandle inputHandle,
            GroupHandle outputHandle,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) = 0;
};

/// EffectProcessorImpl manages a separate EffectState for every combination of
/// input channel to output channel. This allows for processing effects in
/// parallel for PFL and post-fader for the master output.
/// EffectSpecificState must be a subclass of EffectState.
template<typename EffectSpecificState>
class EffectProcessorImpl : public EffectProcessor {
  public:
    EffectProcessorImpl() {
    }
    /// Subclasses should not implement their own destructor. All state should
    /// be stored in the EffectState subclass, not the EffectProcessorImpl subclass.
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

    /// NOTE: Subclasses for Built-In effects must implement the following static methods for
    /// BuiltInBackend to work:
    /// static QString getId();
    /// static EffectManifestPointer getManifest();

    /// This is the only non-static method that subclasses need to implement.
    virtual void processChannel(EffectSpecificState* channelState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) = 0;

    void process(GroupHandle inputHandle,
            GroupHandle outputHandle,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) final {
        EffectSpecificState* pState = m_channelStateMatrix[inputHandle][outputHandle];
        VERIFY_OR_DEBUG_ASSERT(pState != nullptr) {
            if (kEffectDebugOutput) {
                qWarning() << "EffectProcessorImpl::process could not retrieve"
                              "EffectState for input"
                           << *inputHandle
                           << "and output" << *outputHandle
                           << "EffectState should have been preallocated in the"
                              "main thread.";
            }
            pState = createSpecificState(engineParameters);
            m_channelStateMatrix[inputHandle][outputHandle] = pState;
        }
        processChannel(pState, pInput, pOutput, engineParameters, enableState, groupFeatures);
    }

    void initialize(const QSet<GroupHandle>& activeInputChannels,
            const QSet<GroupHandle>& registeredOutputChannels,
            const mixxx::EngineParameters& engineParameters) final {
        m_registeredOutputChannels = registeredOutputChannels;

        for (GroupHandle inputChannel : activeInputChannels) {
            if (kEffectDebugOutput) {
                qDebug() << this << "EffectProcessorImpl::initialize allocating "
                                    "EffectStates for input"
                         << inputChannel;
            }
            ChannelHandleMap<EffectSpecificState*> outputChannelMap;
            for (GroupHandle outputChannel :
                    std::as_const(m_registeredOutputChannels)) {
                outputChannelMap.insert(outputChannel,
                        createSpecificState(engineParameters));
                if (kEffectDebugOutput) {
                    qDebug() << this << "EffectProcessorImpl::initialize "
                                        "registering output"
                             << outputChannel << outputChannelMap[outputChannel];
                }
            }
            m_channelStateMatrix.insert(inputChannel, outputChannelMap);
        }
    };

    EffectState* createState(const mixxx::EngineParameters& engineParameters) final {
        return createSpecificState(engineParameters);
    };

    bool loadStatesForInputChannel(GroupHandle inputChannel,
            const EffectStatesMap* pStatesMap) final {
        if (kEffectDebugOutput) {
            qDebug() << "EffectProcessorImpl::loadStatesForInputChannel" << this
                     << "input" << inputChannel;
        }

        // NOTE: ChannelHandleMap is like a map in that it associates an
        // object with a GroupHandle key, but it is actually backed by a
        // QVarLengthArray, not a QMap. So it is okay that
        // m_channelStateMatrix may be accessed concurrently in the main
        // thread in deleteStatesForInputChannel.

        // Can't directly cast a ChannelHandleMap from containing the base
        // EffectState* type to EffectSpecificState* type, so iterate through
        // pStatesMap to build a new ChannelHandleMap with
        // dynamic_cast'ed states.
        ChannelHandleMap<EffectSpecificState*>& effectSpecificStatesMap =
                m_channelStateMatrix[inputChannel];

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

        QSet<GroupHandle> receivedOutputChannels = m_registeredOutputChannels;
        for (GroupHandle outputChannel :
                std::as_const(m_registeredOutputChannels)) {
            if (kEffectDebugOutput) {
                qDebug() << "EffectProcessorImpl::loadStatesForInputChannel"
                         << this << "output" << outputChannel;
            }

            auto pState = dynamic_cast<EffectSpecificState*>(
                    pStatesMap->at(outputChannel));
            VERIFY_OR_DEBUG_ASSERT(pState != nullptr) {
                return false;
            }
            effectSpecificStatesMap.insert(outputChannel, pState);
            receivedOutputChannels.insert(outputChannel);
        }
        // Output channels are hardcoded in EngineMaster and should not
        // be registered after Mixxx initializes.
        DEBUG_ASSERT(receivedOutputChannels == m_registeredOutputChannels);
        return true;
    };

    /// Called from main thread for garbage collection after an input channel is disabled
    void deleteStatesForInputChannel(GroupHandle inputChannel) final {
        if (kEffectDebugOutput) {
            qDebug() << "EffectProcessorImpl::deleteStatesForInputChannel"
                     << this << inputChannel;
        }

        // NOTE: ChannelHandleMap is like a map in that it associates an
        // object with a GroupHandle key, but it is actually backed by a
        // QVarLengthArray, not a QMap. So it is okay that
        // m_channelStateMatrix may be accessed concurrently in the audio
        // engine thread in loadStatesForInputChannel.

        ChannelHandleMap<EffectSpecificState*>& stateMap =
                m_channelStateMatrix[inputChannel];
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

  protected:
    /// Subclasses for external effects plugins may reimplement this, but
    /// subclasses for built-in effects should not.
    virtual EffectSpecificState* createSpecificState(
            const mixxx::EngineParameters& engineParameters) {
        EffectSpecificState* pState = new EffectSpecificState(engineParameters);
        if (kEffectDebugOutput) {
            qDebug() << this << "EffectProcessorImpl creating EffectState" << pState;
        }
        return pState;
    };

  private:
    QSet<GroupHandle> m_registeredOutputChannels;
    ChannelHandleMap<ChannelHandleMap<EffectSpecificState*>> m_channelStateMatrix;
};
