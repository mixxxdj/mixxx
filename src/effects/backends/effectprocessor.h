#pragma once

#include <QDebug>
#include <QHash>
#include <QPair>
#include <QString>

#include "effects/defs.h"
#include "engine/channelhandle.h"
#include "engine/effects/groupfeaturestate.h"
#include "engine/effects/message.h"
#include "engine/engine.h"
#include "util/sample.h"
#include "util/types.h"
#include "util/unique_ptr_vector.h"

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
/// are hardcoded in EngineMixer as the postfader processing for the main mix
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
            const QSet<ChannelHandleAndGroup>& activeInputChannels,
            const QSet<ChannelHandleAndGroup>& registeredOutputChannels,
            const mixxx::EngineParameters& engineParameters) = 0;
    virtual void initializeInputChannel(
            ChannelHandle inputChannel,
            const mixxx::EngineParameters& engineParameters) = 0;
    virtual void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) = 0;
    virtual bool hasStatesForInputChannel(ChannelHandle inputChannel) const = 0;

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
    virtual void process(const ChannelHandle& inputHandle,
            const ChannelHandle& outputHandle,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) = 0;

    /// This method is used for obtaining the delay of the output buffer
    /// compared to the input buffer based on the internal effect processing.
    /// The method returns the number of frames by which the dry signal
    /// needs to be delayed so that buffers for the dry and wet signal (output
    /// of the effect) overlap. The return value represents the current effect
    /// latency. The value is used in the EngineEffectChain::process method
    /// to calculate the resulting latency of the effect chain. Based
    /// on the sum of the delay value of every effect in the effect chain,
    /// the dry signal is delayed to overlap with the output wet signal
    /// after processing all effects in the effects chain.
    virtual SINT getGroupDelayFrames() = 0;
};

/// EffectProcessorImpl manages a separate EffectState for every combination of
/// input channel to output channel. This allows for processing effects in
/// parallel for PFL and post-fader for the main output.
/// EffectSpecificState must be a subclass of EffectState.
template<typename EffectSpecificState>
class EffectProcessorImpl : public EffectProcessor {
  public:
    EffectProcessorImpl() {
    }
    /// Subclasses should not implement their own destructor. All state should
    /// be stored in the EffectState subclass, not the EffectProcessorImpl subclass.
    ~EffectProcessorImpl() override {
        if (kEffectDebugOutput) {
            qDebug() << "~EffectProcessorImpl" << this;
        }
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

    /// By default, the group delay for every effect is zero. The effect implementation
    /// can override this method and set actual number of frames for the effect delay.
    virtual SINT getGroupDelayFrames() override {
        return 0;
    }

    void process(const ChannelHandle& inputHandle,
            const ChannelHandle& outputHandle,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) final {
        EffectSpecificState* pState = nullptr;
        if (inputHandle < m_channelStateMatrix.size()) {
            const auto& outputChannelStates = m_channelStateMatrix[inputHandle];
            if (outputHandle < outputChannelStates.size()) {
                pState = outputChannelStates[outputHandle].get();
            }
        }
        VERIFY_OR_DEBUG_ASSERT(pState != nullptr) {
            if (kEffectDebugOutput) {
                qWarning() << "EffectProcessorImpl::process could not retrieve"
                              "EffectState for input"
                           << inputHandle
                           << "and output" << outputHandle
                           << "EffectState should have been preallocated in the"
                              "main thread.";
            }
            SampleUtil::copy(pOutput, pInput, engineParameters.samplesPerBuffer());
            return;
        }
        processChannel(pState, pInput, pOutput, engineParameters, enableState, groupFeatures);
    }

    void initialize(const QSet<ChannelHandleAndGroup>& activeInputChannels,
            const QSet<ChannelHandleAndGroup>& registeredOutputChannels,
            const mixxx::EngineParameters& engineParameters) final {
        m_registeredOutputChannels = registeredOutputChannels;

        for (const ChannelHandleAndGroup& inputChannel : activeInputChannels) {
            initializeInputChannel(inputChannel.handle(), engineParameters);
        }
    };

    void initializeInputChannel(ChannelHandle inputChannel,
            const mixxx::EngineParameters& engineParameters) final {
        if (kEffectDebugOutput) {
            qDebug() << this << "EffectProcessorImpl::initialize allocating "
                                "EffectStates for input"
                     << inputChannel;
        }

        int requiredVectorSize = 0;
        // For fast lookups we use a vector with index = handle;
        // gaps are filled with nullptr
        for (const ChannelHandleAndGroup& outputChannel :
                std::as_const(m_registeredOutputChannels)) {
            int vectorIndex = outputChannel.handle();
            if (requiredVectorSize <= vectorIndex) {
                requiredVectorSize = vectorIndex + 1;
            }
        }

        auto& outputChannelStates = m_channelStateMatrix[inputChannel];
        DEBUG_ASSERT(outputChannelStates.size() == 0);
        outputChannelStates.reserve(requiredVectorSize);
        outputChannelStates.clear();
        for (int i = 0; i < requiredVectorSize; ++i) {
            outputChannelStates.push_back(std::unique_ptr<EffectSpecificState>());
        }
        for (const ChannelHandleAndGroup& outputChannel :
                std::as_const(m_registeredOutputChannels)) {
            outputChannelStates[outputChannel.handle()].reset(
                    createSpecificState(engineParameters));
            if (kEffectDebugOutput) {
                qDebug() << this
                         << "EffectProcessorImpl::initialize "
                            "registering output"
                         << outputChannel << outputChannel.handle()
                         << outputChannelStates[outputChannel.handle()].get();
            }
        }
    };

    bool hasStatesForInputChannel(ChannelHandle inputChannel) const final {
        if (inputChannel.handle() < m_channelStateMatrix.size()) {
            for (const auto& pState : m_channelStateMatrix.at(inputChannel)) {
                if (pState) {
                    return true;
                }
            }
        }
        return false;
    }

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
    QSet<ChannelHandleAndGroup> m_registeredOutputChannels;
    ChannelHandleMap<unique_ptr_vector<EffectSpecificState>> m_channelStateMatrix;
};
