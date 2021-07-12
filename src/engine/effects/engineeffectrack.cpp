#include "engine/effects/engineeffectrack.h"
#include "engine/effects/engineeffectchain.h"
#include "util/defs.h"
#include "util/sample.h"

EngineEffectRack::EngineEffectRack(int iRackNumber)
        : m_iRackNumber(iRackNumber),
          m_buffer1(MAX_BUFFER_LEN),
          m_buffer2(MAX_BUFFER_LEN) {
    // Try to prevent memory allocation.
    m_chains.reserve(256);
}

EngineEffectRack::~EngineEffectRack() {
    //qDebug() << "EngineEffectRack::~EngineEffectRack()" << this;
}

bool EngineEffectRack::processEffectsRequest(EffectsRequest& message,
                                             EffectsResponsePipe* pResponsePipe) {
    EffectsResponse response(message);
    switch (message.type) {
        case EffectsRequest::ADD_CHAIN_TO_RACK:
            if (kEffectDebugOutput) {
                qDebug() << debugString() << "ADD_CHAIN_TO_RACK"
                         << message.AddChainToRack.pChain
                         << message.AddChainToRack.iIndex;
            }
            response.success = addEffectChain(message.AddChainToRack.pChain,
                                              message.AddChainToRack.iIndex);
            break;
        case EffectsRequest::REMOVE_CHAIN_FROM_RACK:
            if (kEffectDebugOutput) {
                qDebug() << debugString() << "REMOVE_CHAIN_FROM_RACK"
                         << message.RemoveChainFromRack.pChain
                         << message.RemoveChainFromRack.iIndex;
            }
            response.success = removeEffectChain(message.RemoveChainFromRack.pChain,
                                                 message.RemoveChainFromRack.iIndex);
            break;
        default:
            return false;
    }
    pResponsePipe->writeMessage(response);
    return true;
}

bool EngineEffectRack::process(const ChannelHandle& inputHandle,
                               const ChannelHandle& outputHandle,
                               CSAMPLE* pIn, CSAMPLE* pOut,
                               const unsigned int numSamples,
                               const unsigned int sampleRate,
                               const GroupFeatureState& groupFeatures) {
    bool processingOccured = false;
    if (pIn == pOut) {
        // Effects are applied to the buffer in place
        for (EngineEffectChain* pChain : qAsConst(m_chains)) {
            if (pChain != nullptr) {
                if (pChain->process(inputHandle, outputHandle,
                                    pIn, pOut,
                                    numSamples, sampleRate, groupFeatures)) {
                    processingOccured = true;
                }
            }
        }
    } else {
        // Do not modify the input buffer; only fill the output buffer.
        CSAMPLE* pIntermediateInput = pIn;
        CSAMPLE* pIntermediateOutput;

        for (EngineEffectChain* pChain : qAsConst(m_chains)) {
            if (pChain != nullptr) {
                // Select an unused intermediate buffer for the next output
                if (pIntermediateInput == m_buffer1.data()) {
                    pIntermediateOutput = m_buffer2.data();
                } else {
                    pIntermediateOutput = m_buffer1.data();
                }

                if (pChain->process(inputHandle, outputHandle,
                                    pIntermediateInput, pIntermediateOutput,
                                    numSamples, sampleRate, groupFeatures)) {
                    processingOccured = true;
                    // Output of this chain becomes the input of the next chain.
                    pIntermediateInput = pIntermediateOutput;
                }
            }
        }
        // pIntermediateInput is the output of the last processed chain. It would be the
        // intermediate input of the next chain if there was one.
        if (processingOccured) {
            SampleUtil::copy(pOut, pIntermediateInput, numSamples);
        }
    }
    return processingOccured;
}

bool EngineEffectRack::addEffectChain(EngineEffectChain* pChain, int iIndex) {
    if (iIndex < 0) {
        if (kEffectDebugOutput) {
            qDebug() << debugString()
                     << "WARNING: ADD_CHAIN_TO_RACK message with invalid index:"
                     << iIndex;
        }
        return false;
    }
    if (m_chains.contains(pChain)) {
        if (kEffectDebugOutput) {
            qDebug() << debugString() << "WARNING: chain already added to EngineEffectRack:"
                     << pChain->id();
        }
        return false;
    }
    while (iIndex >= m_chains.size()) {
        m_chains.append(NULL);
    }
    m_chains.replace(iIndex, pChain);
    return true;
}

bool EngineEffectRack::removeEffectChain(EngineEffectChain* pChain, int iIndex) {
    VERIFY_OR_DEBUG_ASSERT(iIndex < m_chains.size()) {
        if (kEffectDebugOutput) {
            qDebug() << debugString()
                     << "WARNING: REMOVE_CHAIN_FROM_RACK message with invalid index:"
                     << iIndex;
        }
        return false;
    }

    if (m_chains.at(iIndex) != pChain) {
        qDebug() << debugString()
                 << "WARNING: REMOVE_CHAIN_FROM_RACK consistency error"
                 << m_chains.at(iIndex) << "loaded but received request to remove"
                 << pChain;
        return false;
    }

    m_chains.replace(iIndex, NULL);
    return true;
}
