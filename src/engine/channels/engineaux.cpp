#include "engine/channels/engineaux.h"

#include <QtDebug>

#include "control/control.h"
#include "control/controlaudiotaperpot.h"
#include "effects/effectsmanager.h"
#include "engine/effects/engineeffectsmanager.h"
#include "moc_engineaux.cpp"
#include "preferences/usersettings.h"
#include "util/sample.h"

EngineAux::EngineAux(const ChannelHandleAndGroup& handleGroup, EffectsManager* pEffectsManager)
        : EngineChannel(handleGroup, EngineChannel::CENTER, pEffectsManager,
                  /*isTalkoverChannel*/ false,
                  /*isPrimaryDeck*/ false),
          m_pInputConfigured(new ControlObject(ConfigKey(getGroup(), "input_configured"))),
          m_pPregain(new ControlAudioTaperPot(ConfigKey(getGroup(), "pregain"), -12, 12, 0.5)),
          m_wasActive(false) {
    // Make input_configured read-only.
    m_pInputConfigured->setReadOnly();
    ControlDoublePrivate::insertAlias(ConfigKey(getGroup(), "enabled"),
                                      ConfigKey(getGroup(), "input_configured"));

    // by default Aux is disabled on the master and disabled on PFL. User
    // can over-ride by setting the "pfl" or "master" controls.
    // Skins can change that during initialisation, if the master control is not provided.
    setMaster(false);
}

EngineAux::~EngineAux() {
    delete m_pPregain;
}

bool EngineAux::isActive() {
    bool enabled = m_pInputConfigured->toBool();
    if (enabled && m_sampleBuffer) {
        m_wasActive = true;
    } else if (m_wasActive) {
        m_vuMeter.reset();
        m_wasActive = false;
    }
    return m_wasActive;
}

void EngineAux::onInputConfigured(const AudioInput& input) {
    if (input.getType() != AudioPath::AUXILIARY) {
        // This is an error!
        qDebug() << "WARNING: EngineAux connected to AudioInput for a non-auxiliary type!";
        return;
    }
    m_sampleBuffer = nullptr;
    m_pInputConfigured->forceSet(1.0);
}

void EngineAux::onInputUnconfigured(const AudioInput& input) {
    if (input.getType() != AudioPath::AUXILIARY) {
        // This is an error!
        qDebug() << "WARNING: EngineAux connected to AudioInput for a non-auxiliary type!";
        return;
    }
    m_sampleBuffer = nullptr;
    m_pInputConfigured->forceSet(0.0);
}

void EngineAux::receiveBuffer(
        const AudioInput& input, const CSAMPLE* pBuffer, unsigned int nFrames) {
    Q_UNUSED(input);
    Q_UNUSED(nFrames);
    m_sampleBuffer = pBuffer;
}

void EngineAux::process(CSAMPLE* pOut, const int iBufferSize) {
    const CSAMPLE* sampleBuffer = m_sampleBuffer; // save pointer on stack
    CSAMPLE_GAIN pregain = static_cast<CSAMPLE_GAIN>(m_pPregain->get());
    if (sampleBuffer) {
        SampleUtil::copyWithGain(pOut, sampleBuffer, pregain, iBufferSize);
        EngineEffectsManager* pEngineEffectsManager = m_pEffectsManager->getEngineEffectsManager();
        if (pEngineEffectsManager != nullptr) {
            pEngineEffectsManager->processPreFaderInPlace(
                    m_group.handle(), m_pEffectsManager->getMasterHandle(), pOut, iBufferSize,
                    // TODO(jholthuis): Use mixxx::audio::SampleRate instead
                    static_cast<unsigned int>(m_pSampleRate->get()));
        }
        m_sampleBuffer = nullptr;
    } else {
        SampleUtil::clear(pOut, iBufferSize);
    }

    // Update VU meter
    m_vuMeter.process(pOut, iBufferSize);
}

void EngineAux::collectFeatures(GroupFeatureState* pGroupFeatures) const {
    m_vuMeter.collectFeatures(pGroupFeatures);
}
