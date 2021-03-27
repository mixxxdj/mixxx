#include "engine/channels/enginedeck.h"

#include "control/controlpushbutton.h"
#include "effects/effectsmanager.h"
#include "engine/effects/engineeffectsmanager.h"
#include "engine/enginebuffer.h"
#include "engine/enginepregain.h"
#include "engine/enginevumeter.h"
#include "moc_enginedeck.cpp"
#include "util/sample.h"
#include "waveform/waveformwidgetfactory.h"

EngineDeck::EngineDeck(
        const ChannelHandleAndGroup& handleGroup,
        UserSettingsPointer pConfig,
        EngineMaster* pMixingEngine,
        EffectsManager* pEffectsManager,
        EngineChannel::ChannelOrientation defaultOrientation,
        bool primaryDeck)
        : EngineChannel(handleGroup, defaultOrientation, pEffectsManager,
                  /*isTalkoverChannel*/ false,
                  primaryDeck),
          m_pConfig(pConfig),
          m_pInputConfigured(new ControlObject(ConfigKey(getGroup(), "input_configured"))),
          m_pPassing(new ControlPushButton(ConfigKey(getGroup(), "passthrough"))),
          // Need a +1 here because the CircularBuffer only allows its size-1
          // items to be held at once (it keeps a blank spot open persistently)
          m_wasActive(false) {
    m_pInputConfigured->setReadOnly();
    // Set up passthrough utilities and fields
    m_pPassing->setButtonMode(ControlPushButton::POWERWINDOW);
    m_bPassthroughIsActive = false;
    m_bPassthroughWasActive = false;

    // Ensure that input is configured before enabling passthrough
    m_pPassing->connectValueChangeRequest(
            this,
            &EngineDeck::slotPassthroughChangeRequest,
            Qt::DirectConnection);

    m_pPregain = new EnginePregain(getGroup());
    m_pBuffer = new EngineBuffer(getGroup(), pConfig, this, pMixingEngine);
}

EngineDeck::~EngineDeck() {
    delete m_pPassing;
    delete m_pBuffer;
    delete m_pPregain;
}

void EngineDeck::process(CSAMPLE* pOut, const int iBufferSize) {
    // Feed the incoming audio through if passthrough is active
    const CSAMPLE* sampleBuffer = m_sampleBuffer; // save pointer on stack
    if (isPassthroughActive() && sampleBuffer) {
        SampleUtil::copy(pOut, sampleBuffer, iBufferSize);
        m_bPassthroughWasActive = true;
        m_sampleBuffer = nullptr;
        m_pPregain->setSpeedAndScratching(1, false);
    } else {
        // If passthrough is no longer enabled, zero out the buffer
        if (m_bPassthroughWasActive) {
            SampleUtil::clear(pOut, iBufferSize);
            m_bPassthroughWasActive = false;
            return;
        }

        // Process the raw audio
        m_pBuffer->process(pOut, iBufferSize);
        m_pPregain->setSpeedAndScratching(m_pBuffer->getSpeed(), m_pBuffer->getScratching());
        m_bPassthroughWasActive = false;
    }

    // Apply pregain
    m_pPregain->process(pOut, iBufferSize);

    EngineEffectsManager* pEngineEffectsManager = m_pEffectsManager->getEngineEffectsManager();
    if (pEngineEffectsManager != nullptr) {
        pEngineEffectsManager->processPreFaderInPlace(m_group.handle(),
                m_pEffectsManager->getMasterHandle(),
                pOut,
                iBufferSize,
                // TODO(jholthuis): Use mixxx::audio::SampleRate instead
                static_cast<unsigned int>(m_pSampleRate->get()));
    }

    // Update VU meter
    m_vuMeter.process(pOut, iBufferSize);
}

void EngineDeck::collectFeatures(GroupFeatureState* pGroupFeatures) const {
    m_pBuffer->collectFeatures(pGroupFeatures);
    m_vuMeter.collectFeatures(pGroupFeatures);
    m_pPregain->collectFeatures(pGroupFeatures);
}

void EngineDeck::postProcess(const int iBufferSize) {
    m_pBuffer->postProcess(iBufferSize);
}

EngineBuffer* EngineDeck::getEngineBuffer() {
    return m_pBuffer;
}

bool EngineDeck::isActive() {
    bool active = false;
    if (m_bPassthroughWasActive && !m_bPassthroughIsActive) {
        active = true;
    } else {
        active = m_pBuffer->isTrackLoaded() || isPassthroughActive();
    }

    if (!active && m_wasActive) {
        m_vuMeter.reset();
    }
    m_wasActive = active;
    return active;
}

void EngineDeck::receiveBuffer(
        const AudioInput& input, const CSAMPLE* pBuffer, unsigned int nFrames) {
    Q_UNUSED(input);
    Q_UNUSED(nFrames);
    // Skip receiving audio input if passthrough is not active
    if (!m_bPassthroughIsActive) {
        m_sampleBuffer = nullptr;
        return;
    } else {
        m_sampleBuffer = pBuffer;
    }
}

void EngineDeck::onInputConfigured(const AudioInput& input) {
    if (input.getType() != AudioPath::VINYLCONTROL) {
        // This is an error!
        qDebug() << "WARNING: EngineDeck connected to AudioInput for a non-vinylcontrol type!";
        return;
    }
    m_pInputConfigured->forceSet(1.0);
    m_sampleBuffer = nullptr;
}

void EngineDeck::onInputUnconfigured(const AudioInput& input) {
    if (input.getType() != AudioPath::VINYLCONTROL) {
        // This is an error!
        qDebug() << "WARNING: EngineDeck connected to AudioInput for a non-vinylcontrol type!";
        return;
    }
    m_pInputConfigured->forceSet(0.0);
    m_sampleBuffer = nullptr;
}

bool EngineDeck::isPassthroughActive() const {
    return (m_bPassthroughIsActive && m_sampleBuffer);
}

void EngineDeck::slotPassingToggle(double v) {
    m_bPassthroughIsActive = v > 0;
}

void EngineDeck::slotPassthroughChangeRequest(double v) {
    if (v <= 0 || m_pInputConfigured->get() > 0) {
        m_pPassing->setAndConfirm(v);

        // Pass confirmed value to slotPassingToggle. We cannot use the
        // valueChanged signal for this, because the change originates from the
        // same ControlObject instance.
        slotPassingToggle(v);
    } else {
        emit noPassthroughInputConfigured();
    }
}
