#include "engine/effects/engineeffectsmanager.h"

#include "audio/types.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectchain.h"
#include "util/defs.h"
#include "util/sample.h"

EngineEffectsManager::EngineEffectsManager(EffectsResponsePipe&& responsePipe)
        : m_responsePipe(std::move(responsePipe)),
          m_buffer1(kMaxEngineSamples),
          m_buffer2(kMaxEngineSamples) {
    // Try to prevent memory allocation.
    m_effects.reserve(256);
}

void EngineEffectsManager::onCallbackStart() {
    EffectsRequest* request = nullptr;
    while (m_responsePipe.readMessage(&request)) {
        EffectsResponse response(*request);
        bool processed = false;
        switch (request->type) {
        case EffectsRequest::ADD_EFFECT_CHAIN:
        case EffectsRequest::REMOVE_EFFECT_CHAIN:
            if (processEffectsRequest(*request, &m_responsePipe)) {
                processed = true;
            }
            break;
        case EffectsRequest::ADD_EFFECT_TO_CHAIN:
        case EffectsRequest::REMOVE_EFFECT_FROM_CHAIN:
        case EffectsRequest::SET_EFFECT_CHAIN_PARAMETERS:
        case EffectsRequest::ENABLE_EFFECT_CHAIN_FOR_INPUT_CHANNEL:
        case EffectsRequest::DISABLE_EFFECT_CHAIN_FOR_INPUT_CHANNEL: {
            bool chainExists = false;
            for (const auto& chains : std::as_const(m_chainsByStage)) {
                if (chains.contains(request->pTargetChain)) {
                    chainExists = true;
                }
            }

            VERIFY_OR_DEBUG_ASSERT(chainExists) {
                response.success = false;
                response.status = EffectsResponse::NO_SUCH_CHAIN;
                break;
            }
            processed = request->pTargetChain->processEffectsRequest(
                    *request, &m_responsePipe);
            if (processed) {
                // When an effect becomes active (part of a chain), keep
                // it in our main list so that we can respond to
                // requests about it.
                if (request->type == EffectsRequest::ADD_EFFECT_TO_CHAIN) {
                    m_effects.append(request->AddEffectToChain.pEffect);
                } else if (request->type == EffectsRequest::REMOVE_EFFECT_FROM_CHAIN) {
                    m_effects.removeAll(request->RemoveEffectFromChain.pEffect);
                }
            } else {
                // If we got here, the message was not handled for
                // an unknown reason.
                response.success = false;
                response.status = EffectsResponse::INVALID_REQUEST;
            }
            break;
        }
        case EffectsRequest::SET_EFFECT_PARAMETERS:
        case EffectsRequest::SET_PARAMETER_PARAMETERS:
            VERIFY_OR_DEBUG_ASSERT(m_effects.contains(request->pTargetEffect)) {
                response.success = false;
                response.status = EffectsResponse::NO_SUCH_EFFECT;
                break;
            }

            processed = request->pTargetEffect
                                ->processEffectsRequest(*request, &m_responsePipe);

            if (!processed) {
                // If we got here, the message was not handled for an
                // unknown reason.
                response.success = false;
                response.status = EffectsResponse::INVALID_REQUEST;
            }
            break;
        default:
            response.success = false;
            response.status = EffectsResponse::UNHANDLED_MESSAGE_TYPE;
            break;
        }

        if (!processed) {
            m_responsePipe.writeMessage(response);
        }
    }
}

void EngineEffectsManager::processPreFaderInPlace(const ChannelHandle& inputHandle,
        const ChannelHandle& outputHandle,
        CSAMPLE* pInOut,
        std::size_t numSamples,
        mixxx::audio::SampleRate sampleRate) {
    // Feature state is gathered after prefader effects processing.
    // This is okay because the equalizer effects do not make use of it.
    GroupFeatureState featureState;
    processInner(SignalProcessingStage::Prefader,
            inputHandle,
            outputHandle,
            pInOut,
            pInOut,
            numSamples,
            sampleRate,
            featureState);
}

void EngineEffectsManager::processPostFaderInPlace(
        const ChannelHandle& inputHandle,
        const ChannelHandle& outputHandle,
        CSAMPLE* pInOut,
        std::size_t numSamples,
        mixxx::audio::SampleRate sampleRate,
        const GroupFeatureState& groupFeatures,
        CSAMPLE_GAIN oldGain,
        CSAMPLE_GAIN newGain,
        bool fadeout) {
    processInner(SignalProcessingStage::Postfader,
            inputHandle,
            outputHandle,
            pInOut,
            pInOut,
            numSamples,
            sampleRate,
            groupFeatures,
            oldGain,
            newGain,
            fadeout);
}

void EngineEffectsManager::processPostFaderAndMix(
        const ChannelHandle& inputHandle,
        const ChannelHandle& outputHandle,
        CSAMPLE* pIn,
        CSAMPLE* pOut,
        std::size_t numSamples,
        mixxx::audio::SampleRate sampleRate,
        const GroupFeatureState& groupFeatures,
        CSAMPLE_GAIN oldGain,
        CSAMPLE_GAIN newGain,
        bool fadeout) {
    processInner(SignalProcessingStage::Postfader,
            inputHandle,
            outputHandle,
            pIn,
            pOut,
            numSamples,
            sampleRate,
            groupFeatures,
            oldGain,
            newGain,
            fadeout);
}

void EngineEffectsManager::processInner(
        const SignalProcessingStage stage,
        const ChannelHandle& inputHandle,
        const ChannelHandle& outputHandle,
        CSAMPLE* pIn,
        CSAMPLE* pOut,
        std::size_t numSamples,
        mixxx::audio::SampleRate sampleRate,
        const GroupFeatureState& groupFeatures,
        CSAMPLE_GAIN oldGain,
        CSAMPLE_GAIN newGain,
        bool fadeout) {
    const QList<EngineEffectChain*>& chains = m_chainsByStage.value(stage);

    if (pIn == pOut) {
        // Gain and effects are applied to the buffer in place,
        // modifying the original input buffer
        SampleUtil::applyRampingGain(pIn, oldGain, newGain, numSamples);
        for (EngineEffectChain* pChain : chains) {
            if (pChain) {
                if (pChain->process(inputHandle,
                            outputHandle,
                            pIn,
                            pOut,
                            numSamples,
                            sampleRate,
                            groupFeatures,
                            fadeout)) {
                }
            }
        }
    } else {
        // Do not modify the input buffer.
        // 1. Copy input buffer to a temporary buffer
        // 2. Apply gain to temporary buffer
        // 2. Process temporary buffer with each effect chain in series
        // 3. Mix the temporary buffer into pOut
        //    ChannelMixer::applyEffectsAndMixChannels use
        //    this to mix channels into pOut regardless of whether any effects were processed.
        CSAMPLE* pIntermediateInput = m_buffer1.data();
        if (oldGain == CSAMPLE_GAIN_ONE && newGain == CSAMPLE_GAIN_ONE) {
            // Avoid an unnecessary copy. EngineEffectChain::process does not modify the
            // input buffer when its input & output buffers are different, so this is okay.
            pIntermediateInput = pIn;
        } else {
            SampleUtil::copyWithRampingGain(pIntermediateInput, pIn, oldGain, newGain, numSamples);
        }

        CSAMPLE* pIntermediateOutput;
        for (EngineEffectChain* pChain : chains) {
            if (pChain) {
                // Select an unused intermediate buffer for the next output
                if (pIntermediateInput == m_buffer1.data()) {
                    pIntermediateOutput = m_buffer2.data();
                } else {
                    pIntermediateOutput = m_buffer1.data();
                }

                if (pChain->process(inputHandle,
                            outputHandle,
                            pIntermediateInput,
                            pIntermediateOutput,
                            numSamples,
                            sampleRate,
                            groupFeatures,
                            fadeout)) {
                    // Output of this chain becomes the input of the next chain.
                    pIntermediateInput = pIntermediateOutput;
                }
            }
        }
        // pIntermediateInput is the output of the last processed chain. It would
        // be the intermediate input of the next chain if there was one.
        SampleUtil::add(pOut, pIntermediateInput, numSamples);
    }
}

bool EngineEffectsManager::addEffectChain(EngineEffectChain* pChain,
        SignalProcessingStage stage) {
    QList<EngineEffectChain*>& chains = m_chainsByStage[stage];
    VERIFY_OR_DEBUG_ASSERT(!chains.contains(pChain)) {
        return false;
    }
    // This might allocate in the audio thread, but it is only used when Mixxx
    // is starting up so there is no issue.
    chains.append(pChain);
    return true;
}

bool EngineEffectsManager::removeEffectChain(EngineEffectChain* pChain,
        SignalProcessingStage stage) {
    QList<EngineEffectChain*>& chains = m_chainsByStage[stage];
    VERIFY_OR_DEBUG_ASSERT(chains.contains(pChain)) {
        return false;
    }
    return chains.removeAll(pChain) > 0;
}

bool EngineEffectsManager::processEffectsRequest(const EffectsRequest& message,
        EffectsResponsePipe* pResponsePipe) {
    EffectsResponse response(message);
    switch (message.type) {
    case EffectsRequest::ADD_EFFECT_CHAIN:
        if (kEffectDebugOutput) {
            qDebug() << debugString() << "ADD_EFFECT_CHAIN"
                     << message.AddEffectChain.pChain;
        }
        response.success = addEffectChain(message.AddEffectChain.pChain,
                message.AddEffectChain.signalProcessingStage);
        break;
    case EffectsRequest::REMOVE_EFFECT_CHAIN:
        if (kEffectDebugOutput) {
            qDebug() << debugString() << "REMOVE_EFFECT_CHAIN"
                     << message.RemoveEffectChain.pChain;
        }
        response.success = removeEffectChain(message.RemoveEffectChain.pChain,
                message.RemoveEffectChain.signalProcessingStage);
        break;
    default:
        return false;
    }
    pResponsePipe->writeMessage(response);
    return true;
}
