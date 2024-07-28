#include "engine/channels/enginedeck.h"

#include "control/controlpushbutton.h"
#include "effects/effectsmanager.h"
#include "engine/effects/engineeffectsmanager.h"
#include "engine/enginebuffer.h"
#include "engine/enginepregain.h"
#include "engine/enginevumeter.h"
#include "moc_enginedeck.cpp"
#include "track/track.h"
#include "util/sample.h"

#ifdef __STEM__
namespace {
constexpr int kMaxSupportedStems = 4;
} // anonymous namespace
#endif

EngineDeck::EngineDeck(
        const ChannelHandleAndGroup& handleGroup,
        UserSettingsPointer pConfig,
        EngineMixer* pMixingEngine,
        EffectsManager* pEffectsManager,
        EngineChannel::ChannelOrientation defaultOrientation,
        bool primaryDeck)
        : EngineChannel(handleGroup, defaultOrientation, pEffectsManager,
                  /*isTalkoverChannel*/ false,
                  primaryDeck),
          m_pConfig(pConfig),
          m_pInputConfigured(new ControlObject(ConfigKey(getGroup(), "input_configured"))),
          m_pPassing(new ControlPushButton(ConfigKey(getGroup(), "passthrough"))) {
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
    m_pBuffer = new EngineBuffer(getGroup(),
            pConfig,
            this,
            pMixingEngine,
#ifdef __STEM__
            primaryDeck ? mixxx::audio::ChannelCount::stem()
                        : mixxx::audio::ChannelCount::stereo()

#else
            mixxx::audio::ChannelCount::stereo()
#endif
    );

#ifdef __STEM__
    if (!primaryDeck) {
        return;
    }

    connect(m_pBuffer, &EngineBuffer::trackLoaded, this, &EngineDeck::slotTrackLoaded);

    m_pStemCount = std::make_unique<ControlObject>(ConfigKey(getGroup(), "stem_count"));
    m_pStemCount->setReadOnly();

    m_stemGain.reserve(kMaxSupportedStems);
    m_stemMute.reserve(kMaxSupportedStems);
    for (int stemIdx = 1; stemIdx <= kMaxSupportedStems; stemIdx++) {
        m_stemGain.emplace_back(std::make_unique<ControlPotmeter>(
                ConfigKey(getGroup(), QString("stem_%1_volume").arg(stemIdx))));
        // The default value is ignored and override with the medium value by
        // ControlPotmeter. This is likely a bug but fixing might have a
        // disruptive impact, so setting the default explicitly
        m_stemGain.back()->set(1.0);
        m_stemGain.back()->setDefaultValue(1.0);
        m_stemMute.emplace_back(std::make_unique<ControlPushButton>(
                ConfigKey(getGroup(), QString("stem_%1_mute").arg(stemIdx))));
    }
#endif
}

#ifdef __STEM__
void EngineDeck::slotTrackLoaded(TrackPointer pNewTrack,
        TrackPointer) {
    VERIFY_OR_DEBUG_ASSERT(m_pStemCount) {
        return;
    }
    if (m_pConfig->getValue(
                ConfigKey("[Mixer Profile]", "stem_auto_reset"), true)) {
        for (int stemIdx = 0; stemIdx < kMaxSupportedStems; stemIdx++) {
            m_stemGain[stemIdx]->set(1.0);
            m_stemMute[stemIdx]->set(0.0);
            ;
        }
    }
    if (pNewTrack) {
        int stemCount = pNewTrack->getStemInfo().size();
        m_pStemCount->forceSet(stemCount);
    } else {
        m_pStemCount->forceSet(0);
    }
}
#endif

EngineDeck::~EngineDeck() {
    delete m_pPassing;
    delete m_pBuffer;
    delete m_pPregain;
}

#ifdef __STEM__
void EngineDeck::processStem(CSAMPLE* pOut, const int iBufferSize) {
    int stemCount = m_pBuffer->getChannelCount() / mixxx::kEngineChannelOutputCount;
    auto allChannelBufferSize = iBufferSize * stemCount;
    if (m_stemBuffer.size() < allChannelBufferSize) {
        m_stemBuffer = mixxx::SampleBuffer(allChannelBufferSize);
    }
    m_pBuffer->process(m_stemBuffer.data(), allChannelBufferSize);

    // TODO(XXX): process effects per stems
    SampleUtil::clear(pOut, iBufferSize);
    const CSAMPLE* pIn = m_stemBuffer.data();
    for (int i = 0; i < iBufferSize; i += mixxx::kEngineChannelOutputCount) {
        for (int stemIdx = 0; stemIdx < stemCount;
                stemIdx++) {
            if (m_stemMute[stemIdx]->toBool()) {
                continue;
            }
            float gain = static_cast<float>(m_stemGain[stemIdx]->get());
            pOut[i] += pIn[stemCount * i + mixxx::kEngineChannelOutputCount * stemIdx] * gain;
            pOut[i + 1] +=
                    pIn[stemCount * i +
                            mixxx::kEngineChannelOutputCount * stemIdx + 1] *
                    gain;
        }
    }
    // TODO(XXX): process stem DSP
}

void EngineDeck::cloneStemState(const EngineDeck* deckToClone) {
    VERIFY_OR_DEBUG_ASSERT(deckToClone) {
        return;
    }
    for (int stemIdx = 0; stemIdx < kMaxSupportedStems; stemIdx++) {
        m_stemGain[stemIdx]->set(deckToClone->m_stemGain[stemIdx]->get());
        m_stemMute[stemIdx]->set(deckToClone->m_stemMute[stemIdx]->get());
    }
}
#endif

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

#ifdef __STEM__
        // Process the raw audio
        if (m_pBuffer->getChannelCount() <= mixxx::kEngineChannelOutputCount) {
            // Process a single mono or stereo channel
#endif
            m_pBuffer->process(pOut, iBufferSize);
#ifdef __STEM__
        } else {
            // Process multiple stereo channels (stems) and mix them together
            processStem(pOut, iBufferSize);
        }
#endif
        m_pPregain->setSpeedAndScratching(m_pBuffer->getSpeed(), m_pBuffer->getScratching());
        m_bPassthroughWasActive = false;
    }

    // Apply pregain
    m_pPregain->process(pOut, iBufferSize);

    EngineEffectsManager* pEngineEffectsManager = m_pEffectsManager->getEngineEffectsManager();
    if (pEngineEffectsManager != nullptr) {
        pEngineEffectsManager->processPreFaderInPlace(m_group.handle(),
                m_pEffectsManager->getMainHandle(),
                pOut,
                iBufferSize,
                mixxx::audio::SampleRate::fromDouble(m_sampleRate.get()));
    }

    // Update VU meter
    m_vuMeter.process(pOut, iBufferSize);
}

void EngineDeck::collectFeatures(GroupFeatureState* pGroupFeatures) const {
    m_pBuffer->collectFeatures(pGroupFeatures);
    m_vuMeter.collectFeatures(pGroupFeatures);
    m_pPregain->collectFeatures(pGroupFeatures);
}

void EngineDeck::postProcessLocalBpm() {
    m_pBuffer->postProcessLocalBpm();
}

void EngineDeck::postProcess(const int iBufferSize) {
    m_pBuffer->postProcess(iBufferSize);
}

EngineBuffer* EngineDeck::getEngineBuffer() {
    return m_pBuffer;
}

EngineChannel::ActiveState EngineDeck::updateActiveState() {
    bool active = false;
    if (m_bPassthroughWasActive && !m_bPassthroughIsActive) {
        active = true;
    } else {
        active = m_pBuffer->isTrackLoaded() || isPassthroughActive();
    }

    if (active) {
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
    if (input.getType() != AudioPathType::VinylControl) {
        // This is an error!
        qDebug() << "WARNING: EngineDeck connected to AudioInput for a non-vinylcontrol type!";
        return;
    }
    m_pInputConfigured->forceSet(1.0);
    m_sampleBuffer = nullptr;
}

void EngineDeck::onInputUnconfigured(const AudioInput& input) {
    if (input.getType() != AudioPathType::VinylControl) {
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

void EngineDeck::slotPassthroughToggle(double v) {
    m_bPassthroughIsActive = v > 0;
}

void EngineDeck::slotPassthroughChangeRequest(double v) {
    if (v <= 0 || m_pInputConfigured->get() > 0) {
        m_pPassing->setAndConfirm(v);

        // Pass confirmed value to slotPassthroughToggle. We cannot use the
        // valueChanged signal for this, because the change originates from the
        // same ControlObject instance.
        slotPassthroughToggle(v);
    } else {
        emit noPassthroughInputConfigured();
    }
}
