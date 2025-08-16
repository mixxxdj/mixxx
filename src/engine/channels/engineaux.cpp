#include "engine/channels/engineaux.h"

#include <QtDebug>

#include "control/controlaudiotaperpot.h"
#include "effects/effectsmanager.h"
#include "engine/effects/engineeffectsmanager.h"
#include "moc_engineaux.cpp"
#include "util/sample.h"

EngineAux::EngineAux(const ChannelHandleAndGroup& handleGroup, EffectsManager* pEffectsManager)
        : EngineChannel(handleGroup, EngineChannel::CENTER, pEffectsManager,
                  /*isTalkoverChannel*/ false,
                  /*isPrimaryDeck*/ false),
          m_pInputConfigured(new ControlObject(ConfigKey(getGroup(), "input_configured"))),
          m_pPregain(new ControlAudioTaperPot(ConfigKey(getGroup(), "pregain"), -12, 12, 0.5)) {
    // Make input_configured read-only.
    m_pInputConfigured->setReadOnly();
    m_pInputConfigured->addAlias(ConfigKey(getGroup(), QStringLiteral("enabled")));

    // by default Aux is disabled on the main and disabled on PFL. User
    // can over-ride by setting the "pfl" or "main_mix" controls.
    // Skins can change that during initialisation, if the main control is not provided.
    setMainMix(false);
}

EngineAux::~EngineAux() {
    delete m_pPregain;
}

EngineChannel::ActiveState EngineAux::updateActiveState() {
    bool enabled = m_pInputConfigured->toBool();
    if (enabled && m_sampleBuffer) {
        m_active = true;
        return ActiveState::Active;
    }
    if (m_active) {
        m_vuMeter.reset();
        m_active = false;
        return ActiveState::WasActive;
    }
    return ActiveState::Inactive;
}

void EngineAux::onInputConfigured(const AudioInput& input) {
    if (input.getType() != AudioPathType::Auxiliary) {
        // This is an error!
        qDebug() << "WARNING: EngineAux connected to AudioInput for a non-auxiliary type!";
        return;
    }
    m_sampleBuffer = nullptr;
    m_pInputConfigured->forceSet(1.0);
}

void EngineAux::onInputUnconfigured(const AudioInput& input) {
    if (input.getType() != AudioPathType::Auxiliary) {
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

void EngineAux::process(CSAMPLE* pOut, const std::size_t bufferSize) {
    const CSAMPLE* sampleBuffer = m_sampleBuffer; // save pointer on stack
    CSAMPLE_GAIN pregain = static_cast<CSAMPLE_GAIN>(m_pPregain->get());
    if (sampleBuffer) {
        SampleUtil::copyWithGain(pOut, sampleBuffer, pregain, bufferSize);
        EngineEffectsManager* pEngineEffectsManager = m_pEffectsManager->getEngineEffectsManager();
        if (pEngineEffectsManager != nullptr) {
            pEngineEffectsManager->processPreFaderInPlace(m_group.handle(),
                    m_pEffectsManager->getMainHandle(),
                    pOut,
                    bufferSize,
                    mixxx::audio::SampleRate::fromDouble(m_sampleRate.get()));
        }
        m_sampleBuffer = nullptr;
    } else {
        SampleUtil::clear(pOut, bufferSize);
    }

    // Update VU meter
    m_vuMeter.process(pOut, bufferSize);
}

void EngineAux::collectFeatures(GroupFeatureState* pGroupFeatures) const {
    m_vuMeter.collectFeatures(pGroupFeatures);
}
