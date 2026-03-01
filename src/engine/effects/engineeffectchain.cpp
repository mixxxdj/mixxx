#include "engine/effects/engineeffectchain.h"

#include "engine/effects/engineeffect.h"
#include "util/defs.h"
#include "util/sample.h"

EngineEffectChain::EngineEffectChain(const QString& group,
        const QSet<ChannelHandleAndGroup>& registeredInputChannels,
        const QSet<ChannelHandleAndGroup>& registeredOutputChannels)
        : m_group(group),
          m_enableState(true),
          m_mixMode(EffectChainMixMode::DrySlashWet),
          m_dMix(0),
          m_buffer1(kMaxEngineSamples),
          m_buffer2(kMaxEngineSamples) {
    // Try to prevent memory allocation.
    m_effects.reserve(256);

    for (const ChannelHandleAndGroup& inputChannel : registeredInputChannels) {
        ChannelHandleMap<ChannelStatus> outputChannelMap;
        for (const ChannelHandleAndGroup& outputChannel : registeredOutputChannels) {
            outputChannelMap.insert(outputChannel.handle(), ChannelStatus());
        }
        m_chainStatusForChannelMatrix.insert(inputChannel.handle(), outputChannelMap);
    }
}

EngineEffectChain::~EngineEffectChain() {
}

bool EngineEffectChain::addEffect(EngineEffect* pEffect, int iIndex) {
    if (iIndex < 0) {
        if (kEffectDebugOutput) {
            qDebug() << debugString()
                     << "WARNING: ADD_EFFECT_TO_CHAIN message with invalid index:"
                     << iIndex;
        }
        return false;
    }
    if (m_effects.contains(pEffect)) {
        if (kEffectDebugOutput) {
            qDebug() << debugString() << "WARNING: effect already added to EngineEffectChain:"
                     << pEffect->name();
        }
        return false;
    }

    while (iIndex >= m_effects.size()) {
        m_effects.append(nullptr);
    }
    m_effects.replace(iIndex, pEffect);
    return true;
}

bool EngineEffectChain::removeEffect(EngineEffect* pEffect, int iIndex) {
    if (iIndex < 0) {
        if (kEffectDebugOutput) {
            qDebug() << debugString()
                     << "WARNING: REMOVE_EFFECT_FROM_CHAIN message with invalid index:"
                     << iIndex;
        }
        return false;
    }
    if (m_effects.at(iIndex) != pEffect) {
        qDebug() << debugString()
                 << "WARNING: REMOVE_EFFECT_FROM_CHAIN consistency error"
                 << m_effects.at(iIndex) << "loaded but received request to remove"
                 << pEffect;
        return false;
    }

    m_effects.replace(iIndex, nullptr);
    return true;
}

// this is called from the engine thread onCallbackStart()
bool EngineEffectChain::updateParameters(const EffectsRequest& message) {
    // TODO(rryan): Parameter interpolation.
    m_mixMode = message.SetEffectChainParameters.mix_mode;
    m_dMix = static_cast<CSAMPLE>(message.SetEffectChainParameters.mix);
    m_enableState = message.SetEffectParameters.enabled;
    return true;
}

bool EngineEffectChain::processEffectsRequest(const EffectsRequest& message,
        EffectsResponsePipe* pResponsePipe) {
    EffectsResponse response(message);
    switch (message.type) {
    case EffectsRequest::ADD_EFFECT_TO_CHAIN:
        if (kEffectDebugOutput) {
            qDebug() << debugString() << this << "ADD_EFFECT_TO_CHAIN"
                     << message.AddEffectToChain.pEffect
                     << message.AddEffectToChain.iIndex;
        }
        response.success = addEffect(message.AddEffectToChain.pEffect,
                message.AddEffectToChain.iIndex);
        break;
    case EffectsRequest::REMOVE_EFFECT_FROM_CHAIN:
        if (kEffectDebugOutput) {
            qDebug() << debugString() << this << "REMOVE_EFFECT_FROM_CHAIN"
                     << message.RemoveEffectFromChain.pEffect
                     << message.RemoveEffectFromChain.iIndex;
        }
        response.success = removeEffect(message.RemoveEffectFromChain.pEffect,
                message.RemoveEffectFromChain.iIndex);
        break;
    case EffectsRequest::SET_EFFECT_CHAIN_PARAMETERS:
        if (kEffectDebugOutput) {
            qDebug() << debugString() << this << "SET_EFFECT_CHAIN_PARAMETERS"
                     << "enabled =" << message.SetEffectChainParameters.enabled
                     << "mix =" << message.SetEffectChainParameters.mix
                     << "mix_mode =" << static_cast<int>(message.SetEffectChainParameters.mix_mode);
        }
        response.success = updateParameters(message);
        break;
    case EffectsRequest::ENABLE_EFFECT_CHAIN_FOR_INPUT_CHANNEL:
        if (kEffectDebugOutput) {
            qDebug() << debugString() << this
                     << "ENABLE_EFFECT_CHAIN_FOR_INPUT_CHANNEL"
                     << message.pTargetChain
                     << message.EnableInputChannelForChain.channelHandle;
        }
        response.success = enableForInputChannel(
                message.EnableInputChannelForChain.channelHandle);
        break;
    case EffectsRequest::DISABLE_EFFECT_CHAIN_FOR_INPUT_CHANNEL:
        if (kEffectDebugOutput) {
            qDebug() << debugString() << this
                     << "DISABLE_EFFECT_CHAIN_FOR_INPUT_CHANNEL"
                     << message.pTargetChain
                     << message.DisableInputChannelForChain.channelHandle;
        }
        response.success = disableForInputChannel(
                message.DisableInputChannelForChain.channelHandle);
        break;
    default:
        return false;
    }
    pResponsePipe->writeMessage(response);
    return true;
}

bool EngineEffectChain::enableForInputChannel(ChannelHandle inputHandle) {
    if (kEffectDebugOutput) {
        qDebug() << "EngineEffectChain::enableForInputChannel" << this << inputHandle;
    }
    auto& outputMap = m_chainStatusForChannelMatrix[inputHandle];
    for (auto&& outputChannelStatus : outputMap) {
        DEBUG_ASSERT(outputChannelStatus.enableState != EffectEnableState::Enabled);
        outputChannelStatus.enableState = EffectEnableState::Enabling;
    }
    return true;
}

bool EngineEffectChain::disableForInputChannel(ChannelHandle inputHandle) {
    auto& outputMap = m_chainStatusForChannelMatrix[inputHandle];
    for (auto&& outputChannelStatus : outputMap) {
        if (outputChannelStatus.enableState == EffectEnableState::Enabling) {
            // Channel has never been processed and can be disabled immediately
            outputChannelStatus.enableState = EffectEnableState::Disabled;
        } else if (outputChannelStatus.enableState == EffectEnableState::Enabled) {
            // Channel was enabled, fade effect out via Disabling state
            outputChannelStatus.enableState = EffectEnableState::Disabling;
        }
    }
    return true;
}

bool EngineEffectChain::process(const ChannelHandle& inputHandle,
        const ChannelHandle& outputHandle,
        CSAMPLE* pIn,
        CSAMPLE* pOut,
        const std::size_t numSamples,
        const mixxx::audio::SampleRate sampleRate,
        const GroupFeatureState& groupFeatures,
        bool fadeout) {
    DEBUG_ASSERT(numSamples <= kMaxEngineSamples);

    // Compute the effective enable state from the channel input routing switch and
    // the chain's enable state. When either of these are turned on/off, send the
    // effects the intermediate enabling/disabling signal.
    // If the EngineEffect is not disabled for the channel, it will pass the
    // intermediate state down to the EffectProcessor, which is then responsible for reacting
    // appropriately, for example the Echo effect clears its internal buffer for the channel
    // when it gets the intermediate disabling signal.

    ChannelStatus& channelStatus = m_chainStatusForChannelMatrix[inputHandle][outputHandle];
    EffectEnableState effectiveChainEnableState = channelStatus.enableState;

    if (channelStatus.enableState == EffectEnableState::Disabling) {
        // Disabled via disableForInputChannel().
        channelStatus.enableState = EffectEnableState::Disabled;
    } else if (!m_enableState || fadeout) {
        if (channelStatus.enableState == EffectEnableState::Enabled) {
            // fadeout is true during the last callback before the track is paused.
            // The track is ramped to zero to avoid clicks.
            // It can started again without further notice.
            // Make sure the effect is paused as well.
            effectiveChainEnableState = EffectEnableState::Disabling;
            // Effect will be paused now, ramp up next callback which may happen later
            // (Enabling is a standby mode).
            channelStatus.enableState = EffectEnableState::Enabling;
        } else if (channelStatus.enableState == EffectEnableState::Enabling) {
            // effect is still disabled
            effectiveChainEnableState = EffectEnableState::Disabled;
        }
    } else if (channelStatus.enableState == EffectEnableState::Enabling) {
        channelStatus.enableState = EffectEnableState::Enabled;
    }

    CSAMPLE currentMixKnob = m_dMix;
    CSAMPLE lastCallbackMixKnob = channelStatus.oldMixKnob;

    bool processingOccured = false;
    if (effectiveChainEnableState != EffectEnableState::Disabled) {
        // Ramping code inside the effects need to access the original samples
        // after writing to the output buffer. This requires not to use the same buffer
        // for in and output: Also, ChannelMixer::applyEffectsAndMixChannels
        // requires that the input buffer does not get modified.
        CSAMPLE* pIntermediateInput = pIn;
        CSAMPLE* pIntermediateOutput;
        SINT effectChainGroupDelayFrames = 0;
        bool firstAddDryToWetEffectProcessed = false;

        for (EngineEffect* pEffect : std::as_const(m_effects)) {
            if (pEffect != nullptr) {
                // Select an unused intermediate buffer for the next output
                if (pIntermediateInput == m_buffer1.data()) {
                    pIntermediateOutput = m_buffer2.data();
                } else {
                    pIntermediateOutput = m_buffer1.data();
                }

                if (pEffect->process(inputHandle,
                            outputHandle,
                            pIntermediateInput,
                            pIntermediateOutput,
                            numSamples,
                            sampleRate,
                            effectiveChainEnableState,
                            groupFeatures)) {
                    if (pEffect->getManifest()->addDryToWet()) {
                        // Skip adding the dry signal to the effect's wet output
                        // when it is the first addDryToWet type effect in
                        // a DryPlusWet mode chain. This allows effects after
                        // it to process only the wet output. For example,
                        // when chaining Echo then Reverb in DryPlusWet mode,
                        // the Reverb effect will get only the wet output of
                        // Echo to process instead of the echoed signal mixed
                        // with the input to Echo. The dry signal that entered
                        // the first effect in the chain will be mixed back in
                        // below after all effects in the chain have been processed.
                        bool skipAddingDry = !firstAddDryToWetEffectProcessed &&
                                m_mixMode == EffectChainMixMode::DryPlusWet;

                        if (!skipAddingDry) {
                            for (SINT i = 0; i <= static_cast<SINT>(numSamples); ++i) {
                                pIntermediateOutput[i] += pIntermediateInput[i];
                            }
                        }

                        firstAddDryToWetEffectProcessed = true;
                    }

                    processingOccured = true;
                    effectChainGroupDelayFrames += pEffect->getGroupDelayFrames();

                    // Output of this effect becomes the input of the next effect
                    pIntermediateInput = pIntermediateOutput;
                }
            }
        }

        m_effectsDelay.setDelayFrames(effectChainGroupDelayFrames);
        m_effectsDelay.process(pIn, numSamples);

        if (processingOccured) {
            // pIntermediateInput is the output of the last processed effect. It would be the
            // intermediate input of the next effect if there was one.
            if (m_mixMode == EffectChainMixMode::DrySlashWet) {
                // Dry/Wet mode: output = (input * (1-mix knob)) + (wet * mix knob)
                SampleUtil::copy2WithRampingGain(
                        pOut,
                        pIn,
                        1.0f - lastCallbackMixKnob,
                        1.0f - currentMixKnob,
                        pIntermediateInput,
                        lastCallbackMixKnob,
                        currentMixKnob,
                        static_cast<int>(numSamples));
            } else {
                // Dry+Wet mode: output = input + (wet * mix knob)
                SampleUtil::copy2WithRampingGain(
                        pOut,
                        pIn,
                        1.0f,
                        1.0f,
                        pIntermediateInput,
                        lastCallbackMixKnob,
                        currentMixKnob,
                        static_cast<int>(numSamples));
            }
        }
    }

    channelStatus.oldMixKnob = currentMixKnob;

    return processingOccured;
}
