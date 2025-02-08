#include "engine/channels/enginedeck.h"

#include <QStringView>

#include "control/controlpushbutton.h"
#include "effects/effectsmanager.h"
#include "engine/controls/bpmcontrol.h"
#include "engine/effects/engineeffectsmanager.h"
#include "engine/effects/groupfeaturestate.h"
#include "engine/enginebuffer.h"
#include "engine/enginepregain.h"
#include "engine/enginevumeter.h"
#include "moc_enginedeck.cpp"
#include "track/track.h"
#include "util/assert.h"
#include "util/sample.h"

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
    m_pPassing->setButtonMode(mixxx::control::ButtonMode::PowerWindow);
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

    m_stemGain.reserve(mixxx::kMaxSupportedStems);
    m_stemMute.reserve(mixxx::kMaxSupportedStems);
    for (int stemIdx = 0; stemIdx < mixxx::kMaxSupportedStems; stemIdx++) {
        m_stemGain.emplace_back(std::make_unique<ControlPotmeter>(
                ConfigKey(getGroupForStem(getGroup(), stemIdx), QStringLiteral("volume"))));
        // The default value is ignored and override with the medium value by
        // ControlPotmeter. This is likely a bug but fixing might have a
        // disruptive impact, so setting the default explicitly
        m_stemGain.back()->set(1.0);
        m_stemGain.back()->setDefaultValue(1.0);
        auto pMuteButton = std::make_unique<ControlPushButton>(
                ConfigKey(getGroupForStem(getGroup(), stemIdx), QStringLiteral("mute")));
        pMuteButton->setButtonMode(mixxx::control::ButtonMode::PowerWindow);
        m_stemMute.push_back(std::move(pMuteButton));
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
        for (int stemIdx = 0; stemIdx < mixxx::kMaxSupportedStems; stemIdx++) {
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
void EngineDeck::addStemHandle(const ChannelHandleAndGroup& stemHandleGroup) {
    m_stems.emplace_back(ChannelHandleAndGroup(stemHandleGroup.handle(), stemHandleGroup.name()));
    m_stemsGainCache.push_back(CSAMPLE_GAIN_ONE);
    if (m_pEffectsManager != nullptr) {
        m_pEffectsManager->registerInputChannel(stemHandleGroup);
    }
}

void EngineDeck::processStem(CSAMPLE* pOut, const std::size_t bufferSize) {
    mixxx::audio::ChannelCount chCount = m_pBuffer->getChannelCount();
    VERIFY_OR_DEBUG_ASSERT(m_stems.size() <= chCount &&
            m_stemMute.size() <= chCount && m_stemGain.size() <= chCount) {
        return;
    };
    mixxx::audio::SampleRate sampleRate = mixxx::audio::SampleRate::fromDouble(m_sampleRate.get());
    unsigned int stemCount = chCount / mixxx::kEngineChannelOutputCount;
    SINT numFrames = bufferSize / mixxx::kEngineChannelOutputCount;
    std::size_t allChannelBufferSize = bufferSize * stemCount;
    if (m_stemBuffer.size() < static_cast<SINT>(allChannelBufferSize)) {
        m_stemBuffer = mixxx::SampleBuffer(allChannelBufferSize);
    }
    m_pBuffer->process(m_stemBuffer.data(), allChannelBufferSize);

    CSAMPLE* pIn = m_stemBuffer.data();

    // TODO(XXX): process stem DSP

    EngineEffectsManager* pEngineEffectsManager = m_pEffectsManager->getEngineEffectsManager();

    VERIFY_OR_DEBUG_ASSERT(pEngineEffectsManager != nullptr) {
        // If we don't have an engine manager to mix the stem together, we mixed
        // the multi channel into stereo and return early.
        SampleUtil::mixMultichannelToStereo(pOut, pIn, numFrames, chCount);
        return;
    }

    // We will now mix each stem (stereo channel) into a single "output"
    // stereo channel. In order to mix the steam, we will use the engine
    // effect manager so we can also apply the individual stem quick FX
    GroupFeatureState featureState;
    collectFeatures(&featureState);
    for (std::size_t stemIdx = 0; stemIdx < stemCount;
            stemIdx++) {
        int chOffset = stemIdx * mixxx::audio::ChannelCount::stereo();
        float stemGain = m_stemMute[stemIdx]->toBool()
                ? 0.0f
                : static_cast<float>(m_stemGain[stemIdx]->get());
        // Extract the stem frames into the output buffer (LR......LR...... -> LRLR)
        SampleUtil::copyOneStereoFromMulti(
                pOut,
                pIn,
                numFrames,
                chCount,
                chOffset);
        // Mix the stem frames with the right gain after proceeding its effect.
        pEngineEffectsManager->processPostFaderInPlace(m_stems[stemIdx].handle(),
                m_pEffectsManager->getMainHandle(),
                pOut,
                bufferSize,
                sampleRate,
                featureState,
                m_stemsGainCache[stemIdx],
                stemGain,
                false);
        // We cache the current gain so we can use it to fade the frame on
        // next iteration. Without this, (e.g using a static "previous"
        // gain) gain changes will yield to audio cracks.
        m_stemsGainCache[stemIdx] = stemGain;

        // Put back the stem frames into the steam buffer (LRLR -> LR......LR......)
        SampleUtil::insertStereoToMulti(
                pIn,
                pOut,
                numFrames,
                chCount,
                chOffset);
    }

    // Mixxx all the stem tracks together
    SampleUtil::mixMultichannelToStereo(pOut, pIn, numFrames, chCount);
}

void EngineDeck::cloneStemState(const EngineDeck* deckToClone) {
    VERIFY_OR_DEBUG_ASSERT(deckToClone) {
        return;
    }
    // Sampler and preview decks don't have stem controls
    if (!isPrimaryDeck() || !deckToClone->isPrimaryDeck()) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_stemGain.size() == mixxx::kMaxSupportedStems &&
            m_stemMute.size() == mixxx::kMaxSupportedStems &&
            deckToClone->m_stemGain.size() == mixxx::kMaxSupportedStems &&
            deckToClone->m_stemMute.size() == mixxx::kMaxSupportedStems) {
        return;
    }
    for (int stemIdx = 0; stemIdx < mixxx::kMaxSupportedStems; stemIdx++) {
        m_stemGain[stemIdx]->set(deckToClone->m_stemGain[stemIdx]->get());
        m_stemMute[stemIdx]->set(deckToClone->m_stemMute[stemIdx]->get());
    }
}
#endif

void EngineDeck::process(CSAMPLE* pOut, const std::size_t bufferSize) {
    // Feed the incoming audio through if passthrough is active
    const CSAMPLE* sampleBuffer = m_sampleBuffer; // save pointer on stack
    if (isPassthroughActive() && sampleBuffer) {
        SampleUtil::copy(pOut, sampleBuffer, bufferSize);
        m_bPassthroughWasActive = true;
        m_sampleBuffer = nullptr;
        m_pPregain->setSpeedAndScratching(1, false);
    } else {
        // If passthrough is no longer enabled, zero out the buffer
        if (m_bPassthroughWasActive) {
            SampleUtil::clear(pOut, bufferSize);
            m_bPassthroughWasActive = false;
            return;
        }

#ifdef __STEM__
        // Process the raw audio
        if (m_pBuffer->getChannelCount() <= mixxx::kEngineChannelOutputCount) {
            // Process a single mono or stereo channel
#endif
            m_pBuffer->process(pOut, bufferSize);
#ifdef __STEM__
        } else {
            // Process multiple stereo channels (stems) and mix them together
            processStem(pOut, bufferSize);
        }
#endif
        m_pPregain->setSpeedAndScratching(m_pBuffer->getSpeed(), m_pBuffer->getScratching());
        m_bPassthroughWasActive = false;
    }

    // Apply pregain
    m_pPregain->process(pOut, bufferSize);

    EngineEffectsManager* pEngineEffectsManager = m_pEffectsManager->getEngineEffectsManager();
    if (pEngineEffectsManager != nullptr) {
        pEngineEffectsManager->processPreFaderInPlace(m_group.handle(),
                m_pEffectsManager->getMainHandle(),
                pOut,
                bufferSize,
                mixxx::audio::SampleRate::fromDouble(m_sampleRate.get()));
    }

    // Update VU meter
    m_vuMeter.process(pOut, bufferSize);
}

void EngineDeck::collectFeatures(GroupFeatureState* pGroupFeatures) const {
    m_pBuffer->collectFeatures(pGroupFeatures);
    m_vuMeter.collectFeatures(pGroupFeatures);
    m_pPregain->collectFeatures(pGroupFeatures);
}

void EngineDeck::postProcessLocalBpm() {
    m_pBuffer->postProcessLocalBpm();
}

void EngineDeck::postProcess(const std::size_t bufferSize) {
    m_pBuffer->postProcess(bufferSize);
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

#ifdef __STEM__
// static
QString EngineDeck::getGroupForStem(QStringView deckGroup, int stemIdx) {
    DEBUG_ASSERT(deckGroup.endsWith(QChar(']')) && stemIdx < 4);
    return deckGroup.chopped(1) + QStringLiteral("_Stem") + QChar('1' + stemIdx) + QChar(']');
}
#endif
