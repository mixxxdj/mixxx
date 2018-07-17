#include "effects/lv2/lv2effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "control/controlobject.h"
#include "util/sample.h"
#include "util/defs.h"

LV2EffectProcessor::LV2EffectProcessor(LV2EffectManifestPointer pManifest)
            : m_pManifest(pManifest),
              m_pPlugin(pManifest->getPlugin()),
              m_audioPortIndices(pManifest->getAudioPortIndices()),
              m_controlPortIndices(pManifest->getControlPortIndices()) {
    m_inputL = new float[MAX_BUFFER_LEN];
    m_inputR = new float[MAX_BUFFER_LEN];
    m_outputL = new float[MAX_BUFFER_LEN];
    m_outputR = new float[MAX_BUFFER_LEN];
}

void LV2EffectProcessor::loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_LV2parameters = new float[parameters.size()];

    // EngineEffect passes the EngineEffectParameters indexed by ID string, which
    // is used directly by built-in EffectProcessorImpl subclasseses to access
    // specific named parameters. However, LV2EffectProcessor::process iterates
    // over the EngineEffectParameters to copy their values to the LV2 control
    // ports. To avoid slow string comparisons in the audio engine thread in
    // LV2EffectProcessor::process, rearrange the QMap of EngineEffectParameters by
    // ID string to an ordered QList.
    for (const auto& pManifestParameter : m_pManifest->getEffectManifest()->parameters()) {
        m_engineEffectParameters.append(parameters.value(pManifestParameter->id()));
    }
}

LV2EffectProcessor::~LV2EffectProcessor() {
    if (kEffectDebugOutput) {
        qDebug() << "~LV2EffectProcessor" << this;
    }
    int inputChannelHandleNumber = 0;
    for (auto& outputsMap : m_channelStateMatrix) {
        int outputChannelHandleNumber = 0;
        for (LV2EffectGroupState* pState : outputsMap) {
              VERIFY_OR_DEBUG_ASSERT(pState != nullptr) {
                    continue;
              }
              if (kEffectDebugOutput) {
                  qDebug() << "~LV2EffectProcessor deleting LilvInstance" << pState
                           << "for input ChannelHandle(" << inputChannelHandleNumber << ")"
                           << "and output ChannelHandle(" << outputChannelHandleNumber << ")";
              }
              delete pState;
              outputChannelHandleNumber++;
        }
        outputsMap.clear();
    }
    m_channelStateMatrix.clear();

    delete[] m_inputL;
    delete[] m_inputR;
    delete[] m_outputL;
    delete[] m_outputR;
    delete[] m_LV2parameters;
}

void LV2EffectProcessor::initialize(
        const QSet<ChannelHandleAndGroup>& activeInputChannels,
        const QSet<ChannelHandleAndGroup>& registeredOutputChannels,
        const mixxx::EngineParameters& bufferParameters) {
    m_registeredOutputChannels = registeredOutputChannels;
    Q_UNUSED(bufferParameters);

    for (const ChannelHandleAndGroup& inputChannel : activeInputChannels) {
        if (kEffectDebugOutput) {
            qDebug() << this << "LV2EffectProcessor::initialize allocating "
                        "EffectStates for input" << inputChannel;
        }
        ChannelHandleMap<LV2EffectGroupState*> outputChannelMap;
        for (const ChannelHandleAndGroup& outputChannel : registeredOutputChannels) {
            LV2EffectGroupState* pGroupState = createGroupState(bufferParameters);
            if (pGroupState) {
                outputChannelMap.insert(outputChannel.handle(), pGroupState);
                if (kEffectDebugOutput) {
                    qDebug() << this << "EffectProcessorImpl::initialize "
                            "registering output" << outputChannel << outputChannelMap[outputChannel.handle()];
                }
            }
        }
        m_channelStateMatrix.insert(inputChannel.handle(), outputChannelMap);
    }
}

void LV2EffectProcessor::process(const ChannelHandle& inputHandle,
        const ChannelHandle& outputHandle,
        const CSAMPLE* pInput, CSAMPLE* pOutput,
        const mixxx::EngineParameters& bufferParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(groupFeatures);
    Q_UNUSED(enableState);

    LV2EffectGroupState* pState = m_channelStateMatrix[inputHandle][outputHandle];
    VERIFY_OR_DEBUG_ASSERT(pState != nullptr) {
        if (kEffectDebugOutput) {
            qWarning() << "LV2EffectProcessor::process could not retrieve"
                          "handle for input" << inputHandle
                       << "and output" << outputHandle
                       << "Handle should have been preallocated in the"
                          "main thread.";
        }
        pState = createGroupState(bufferParameters);
        m_channelStateMatrix[inputHandle][outputHandle] = pState;
    }

    if (!pState) {
        SampleUtil::copyWithGain(pOutput, pInput, 1.0, bufferParameters.samplesPerBuffer());
        return;
    } 

    for (int i = 0; i < m_engineEffectParameters.size(); i++) {
        m_LV2parameters[i] = m_engineEffectParameters[i]->value();
    }

    int j = 0;
    for (unsigned int i = 0; i < bufferParameters.samplesPerBuffer(); i += 2) {
        m_inputL[j] = pInput[i];
        m_inputR[j] = pInput[i + 1];
        j++;
    }

    lilv_instance_run(pState->lilvIinstance(), bufferParameters.framesPerBuffer());

    j = 0;
    for (unsigned int i = 0; i < bufferParameters.samplesPerBuffer(); i += 2) {
        pOutput[i] = m_outputL[j];
        pOutput[i + 1] = m_outputR[j];
        j++;
    }
}

LV2EffectGroupState* LV2EffectProcessor::createGroupState(const mixxx::EngineParameters& bufferParameters) {
    LV2EffectGroupState * pState = new LV2EffectGroupState(bufferParameters, m_pPlugin);
    LilvInstance* handle = pState->lilvIinstance();
    if (handle) {
        for (int i = 0; i < m_engineEffectParameters.size(); i++) {
            m_LV2parameters[i] = m_engineEffectParameters[i]->value();
            lilv_instance_connect_port(handle, m_controlPortIndices[i], &m_LV2parameters[i]);
        }

        // We assume the audio ports are in the following order:
        // input_left, input_right, output_left, output_right
        lilv_instance_connect_port(handle, m_audioPortIndices[0], m_inputL);
        lilv_instance_connect_port(handle, m_audioPortIndices[1], m_inputR);
        lilv_instance_connect_port(handle, m_audioPortIndices[2], m_outputL);
        lilv_instance_connect_port(handle, m_audioPortIndices[3], m_outputR);

        lilv_instance_activate(handle);
    }
    if (kEffectDebugOutput) {
        qDebug() << this << "LV2EffectProcessor creating EffectState" << pState;
    }
    return pState;
};

EffectState* LV2EffectProcessor::createState(const mixxx::EngineParameters& bufferParameters) {
    return createGroupState(bufferParameters);
};

bool LV2EffectProcessor::loadStatesForInputChannel(const ChannelHandle* inputChannel,
      const EffectStatesMap* pStatesMap) {
    if (kEffectDebugOutput) {
        qDebug() << "LV2EffectProcessor::loadStatesForInputChannel" << this
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
    ChannelHandleMap<LV2EffectGroupState*>& effectSpecificStatesMap =
            m_channelStateMatrix[*inputChannel];

    // deleteStatesForInputChannel should have been called before a new
    // map of EffectStates was sent to this function, or this is the first
    // time states are being loaded for this input channel, so
    // effectSpecificStatesMap should be empty and this loop should
    // not go through any iterations.
    for (LV2EffectGroupState* pState : effectSpecificStatesMap) {
        VERIFY_OR_DEBUG_ASSERT(pState == nullptr) {
            delete pState;
            qDebug() << "for effectSpecificStatesMap";
        }
    }

    QSet<ChannelHandleAndGroup> receivedOutputChannels;
    for (const ChannelHandleAndGroup& outputChannel : m_registeredOutputChannels) {
        if (kEffectDebugOutput) {
            qDebug() << "LV2EffectProcessor::loadStatesForInputChannel"
                     << this << "output" << outputChannel;
        }

        auto pState = dynamic_cast<LV2EffectGroupState*>(
                  pStatesMap->at(outputChannel.handle()));
        VERIFY_OR_DEBUG_ASSERT(pState != nullptr) {
              return false;
        }
        effectSpecificStatesMap.insert(outputChannel.handle(), pState);
        receivedOutputChannels.insert(outputChannel);
    }
    // Output channels are hardcoded in EngineMaster and should not
    // be registered after Mixxx initializes.
    DEBUG_ASSERT(receivedOutputChannels == m_registeredOutputChannels);
    return true;
}

// Called from main thread for garbage collection after the last audio thread
// callback executes process() with EffectEnableState::Disabling
void LV2EffectProcessor::deleteStatesForInputChannel(const ChannelHandle* inputChannel) {
    if (kEffectDebugOutput) {
        qDebug() << "LV2EffectProcessor::deleteStatesForInputChannel"
                 << this << *inputChannel;
    }

    // NOTE: ChannelHandleMap is like a map in that it associates an
    // object with a ChannelHandle key, but it actually backed by a
    // QVarLengthArray, not a QMap. So it is okay that
    // m_channelStateMatrix may be accessed concurrently in the audio
    // engine thread in loadStatesForInputChannel.

    ChannelHandleMap<LV2EffectGroupState*>& stateMap =
            m_channelStateMatrix[*inputChannel];
    for (LV2EffectGroupState* pState : stateMap) {
          VERIFY_OR_DEBUG_ASSERT(pState != nullptr) {
                continue;
          }
          if (kEffectDebugOutput) {
                qDebug() << "LV2EffectProcessor::deleteStatesForInputChannel"
                         << this << "deleting state" << pState;
          }
          delete pState;
          qDebug() << "inputChannel" << inputChannel;
    }
    stateMap.clear();
}
