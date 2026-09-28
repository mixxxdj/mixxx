#include "engine/channels/enginedeck.h"

#include <QStringView>
#include <cmath>

#include "control/controlpushbutton.h"
#include "effects/effectsmanager.h"
#include "engine/controls/bpmcontrol.h"
#include "engine/effects/engineeffectsmanager.h"
#include "engine/effects/groupfeaturestate.h"
#include "engine/enginebuffer.h"
#include "engine/enginepregain.h"
#include "moc_enginedeck.cpp"
#include "sources/soundsource.h"
#include "track/track.h"
#include "util/assert.h"
#include "util/logger.h"
#include "util/sample.h"

#ifdef __STEM_SEPARATOR__
#include "engine/stems/offlineseparator.h"
#endif
#include "engine/stems/stemcachemanager.h"
#include "engine/stems/enginestemmixer.h"
#include "engine/stems/virtualstemsource.h"
#include "engine/stems/virtualstemsoundsource.h"

namespace {
const mixxx::Logger kLogger("EngineDeck");
}

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
#ifdef __STEM__
          m_stemClonedState(false),
#endif
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
    m_stemSolo.reserve(mixxx::kMaxSupportedStems);
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
        auto pSoloButton = std::make_unique<ControlPushButton>(
                ConfigKey(getGroupForStem(getGroup(), stemIdx), QStringLiteral("solo")));
        pSoloButton->setButtonMode(mixxx::control::ButtonMode::PowerWindow);
        m_stemSolo.push_back(std::move(pSoloButton));
    }

    m_pStemSeparatorEnabled = std::make_unique<ControlPushButton>(
            ConfigKey(getGroup(), "stem_separator_enabled"));
    m_pStemSeparatorEnabled->setButtonMode(mixxx::control::ButtonMode::PowerWindow);
    m_pStemSeparatorEnabled->set(0.0);
#endif
}

#ifdef __STEM__
void EngineDeck::slotTrackLoaded(TrackPointer pNewTrack,
        TrackPointer) {
    VERIFY_OR_DEBUG_ASSERT(m_pStemCount) {
        return;
    }
    if (m_pConfig->getValue(
                ConfigKey("[Mixer Profile]", "stem_auto_reset"), true) &&
            !m_stemClonedState) {
        for (int stemIdx = 0; stemIdx < mixxx::kMaxSupportedStems; stemIdx++) {
            m_stemGain[stemIdx]->set(1.0);
            m_stemMute[stemIdx]->set(0.0);
            m_stemSolo[stemIdx]->set(0.0);
        }
    }
    m_stemClonedState = false;
    if (pNewTrack) {
        int stemCount = pNewTrack->getStemInfo().size();
        m_pStemCount->forceSet(stemCount);
        // If no native stems, try AI stem separation
        if (stemCount == 0) {
            checkAndLoadAIStems(pNewTrack);
        }
    } else {
        m_pStemCount->forceSet(0);
        m_pVirtualStemSource.reset();
        m_aiStemsLoading = false;
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
            m_stemMute.size() <= chCount && m_stemGain.size() <= chCount &&
            m_stemSolo.size() <= chCount) {
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
    // Solo semantics: if any stem solo is active, only soloed stems sound.
    // Mute is applied afterwards, so a soloed but muted stem stays silent.
    bool anySolo = false;
    for (unsigned int stemIdx = 0; stemIdx < stemCount; stemIdx++) {
        if (m_stemSolo[stemIdx]->toBool()) {
            anySolo = true;
            break;
        }
    }
    // S3 fast path: 8ch nativos (4 stems) en CPU con AVX2+FMA. El FX por
    // stem se conserva con ganancia unidad via processPostFaderInPlace y
    // la ganancia + rampa click-free + downmix los hace EngineStemMixer
    // en un solo pass SIMD (vs escalar, tol 1e-6). La rampa reutiliza
    // m_stemsGainCache como oldGains. Solo/mute (S4) se respetan igual
    // que en el legacy path de abajo.
    if (chCount == mixxx::audio::ChannelCount::stem() &&
            stemCount == static_cast<unsigned int>(mixxx::kMaxSupportedStems) &&
            m_stems.size() >= stemCount &&
            m_stemsGainCache.size() >= stemCount &&
            EngineStemMixer::hasAVX2()) {
        EngineStemMixer::GainArray oldGains, newGains;
        for (unsigned int stemIdx = 0; stemIdx < stemCount;
                stemIdx++) {
            int chOffset = stemIdx * mixxx::audio::ChannelCount::stereo();
            float stemGain;
            if (anySolo && !m_stemSolo[stemIdx]->toBool()) {
                stemGain = 0.0f;
            } else if (m_stemMute[stemIdx]->toBool()) {
                stemGain = 0.0f;
            } else {
                stemGain = static_cast<float>(m_stemGain[stemIdx]->get());
            }
            // Extract the stem frames into the output buffer (LR......LR...... -> LRLR)
            SampleUtil::copyOneStereoFromMulti(
                    pOut,
                    pIn,
                    numFrames,
                    chCount,
                    chOffset);
            // Apply the stem FX with unity gain; the gain ramp lives in the mixer.
            pEngineEffectsManager->processPostFaderInPlace(m_stems[stemIdx].handle(),
                    m_pEffectsManager->getMainHandle(),
                    pOut,
                    bufferSize,
                    sampleRate,
                    featureState,
                    CSAMPLE_GAIN_ONE,
                    CSAMPLE_GAIN_ONE,
                    false);
            // Put back the stem frames into the steam buffer (LRLR -> LR......LR......)
            SampleUtil::insertStereoToMulti(
                    pIn,
                    pOut,
                    numFrames,
                    chCount,
                    chOffset);
            oldGains[stemIdx] = m_stemsGainCache[stemIdx];
            newGains[stemIdx] = stemGain;
        }
        EngineStemMixer::process(pOut, pIn, oldGains, newGains, numFrames);
        // We cache the current gain so we can use it to fade the frame on
        // next iteration. Without this, (e.g using a static "previous"
        // gain) gain changes will yield to audio cracks.
        for (unsigned int stemIdx = 0; stemIdx < stemCount; stemIdx++) {
            m_stemsGainCache[stemIdx] = newGains[stemIdx];
        }
        return;
    }
    for (unsigned int stemIdx = 0; stemIdx < stemCount;
            stemIdx++) {
        int chOffset = stemIdx * mixxx::audio::ChannelCount::stereo();
        float stemGain;
        if (anySolo && !m_stemSolo[stemIdx]->toBool()) {
            stemGain = 0.0f;
        } else if (m_stemMute[stemIdx]->toBool()) {
            stemGain = 0.0f;
        } else {
            stemGain = static_cast<float>(m_stemGain[stemIdx]->get());
        }
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
            m_stemSolo.size() == mixxx::kMaxSupportedStems &&
            deckToClone->m_stemGain.size() == mixxx::kMaxSupportedStems &&
            deckToClone->m_stemMute.size() == mixxx::kMaxSupportedStems &&
            deckToClone->m_stemSolo.size() == mixxx::kMaxSupportedStems) {
        return;
    }
    for (int stemIdx = 0; stemIdx < mixxx::kMaxSupportedStems; stemIdx++) {
        m_stemGain[stemIdx]->set(deckToClone->m_stemGain[stemIdx]->get());
        m_stemMute[stemIdx]->set(deckToClone->m_stemMute[stemIdx]->get());
        m_stemSolo[stemIdx]->set(deckToClone->m_stemSolo[stemIdx]->get());
    }
    m_stemClonedState = true;
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
        if (m_pStemSeparatorEnabled && m_pStemSeparatorEnabled->toBool() &&
                m_pVirtualStemSource && m_pVirtualStemSource->isLoaded()) {
            // Use AI-separated virtual stems
            processVirtualStems(pOut, bufferSize);
        } else if (m_pBuffer->getChannelCount() <= mixxx::kEngineChannelOutputCount) {
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

void EngineDeck::checkAndLoadAIStems(TrackPointer pTrack) {
    if (!pTrack || m_aiStemsLoading) {
        return;
    }

    // Never separate a track that is itself a native stem file (this also
    // guards against re-entering this path when the deck is reloaded with the
    // .stem.mp4 generated by a previous separation).
    if (mixxx::SoundSource::getTypeFromUrl(QUrl::fromLocalFile(pTrack->getLocation())) ==
            QStringLiteral("stem.mp4")) {
        return;
    }

    StemCacheManager::CacheKey key = StemCacheManager::generateKey(pTrack->getLocation());
    // N18: version the lookup key with the configured stem mode so mode-3
    // artifacts never poison mode-4 lookups (mode 4 keys are unchanged).
    // N19: versioning intentionally unchanged after the default flip to
    // mode 3 — legacy mode-4 entries stay valid but are only served when
    // stem_mode=4 is set manually; default mode-3 entries live under the
    // "|mode=3" key. The 8-channel reader layout is identical in both
    // modes (still 4 slots in the file).
    // NOTE: changing stem_mode requires a re-separation; stale entries of
    // the other mode stay in the cache dir and are pruned by size, never
    // served.
    int aiStemMode = 3;
    double aiOverlap = 0.25;
    if (m_pConfig) {
        aiStemMode = m_pConfig->getValue(
                ConfigKey("[StemSeparation]", "stem_mode"), 3) == 4
                ? 4
                : 3;
        const double ov = m_pConfig->getValue(
                ConfigKey("[StemSeparation]", "overlap"), 0.25);
        aiOverlap = (std::abs(ov - 0.5) < 1e-9) ? 0.5 : 0.25;
        if (aiStemMode == 3) {
            key = StemCacheManager::generateKeyForMode(
                    pTrack->getLocation(), aiStemMode);
        }
    }

    if (StemCacheManager::instance().hasStems(key)) {
        // Already cached - load the native stem file if we have one, else
        // fall back to virtual stems.
        m_aiStemCacheKey = key;
        StemCacheManager::StemFiles files =
                StemCacheManager::instance().getStemFiles(key);
        if (!files.stemFile.isEmpty() && QFile::exists(files.stemFile)) {
            kLogger.info() << "Loading cached native stem file:" << files.stemFile;
            emit aiStemFileReady(files.stemFile);
            return;
        }
        m_aiStemsLoading = true;
        loadVirtualStems();
        return;
    }

    // Not cached - check if stem files exist on disk (manual placement)
    QString trackBase = QFileInfo(pTrack->getLocation()).completeBaseName();
    QString trackDir = QFileInfo(pTrack->getLocation()).absolutePath();
    QStringList possibleStems = {
        trackDir + "/" + trackBase + "_vocals.wav",
        trackDir + "/" + trackBase + "_drums.wav",
        trackDir + "/" + trackBase + "_bass.wav",
        trackDir + "/" + trackBase + "_other.wav"
    };
    bool manualStemsExist = true;
    for (const auto& path : possibleStems) {
        if (!QFile::exists(path)) {
            manualStemsExist = false;
            break;
        }
    }

    if (manualStemsExist) {
        // Load manually placed stems
        StemCacheManager::StemFiles files;
        files.vocals = possibleStems[0];
        files.drums = possibleStems[1];
        files.bass = possibleStems[2];
        files.other = possibleStems[3];
        files.complete = true;
        files.created = QDateTime::currentDateTime();
        StemCacheManager::instance().markComplete(key, files);
        m_aiStemCacheKey = key;
        m_aiStemsLoading = true;
        loadVirtualStems();
        return;
    }

#ifdef __STEM_SEPARATOR__
    // Neither cached nor manual stems found - launch offline separation
    if (!StemCacheManager::instance().tryMarkProcessing(key)) {
        return; // Already processing
    }

    m_aiStemCacheKey = key;
    m_aiStemsLoading = true;

    mixxx::OfflineSeparator::Config sepConfig;
    sepConfig.inputPath = pTrack->getLocation();
    // Model path: override via env var, else default install location.
    // The OfflineSeparator falls back to the default path if this is empty.
    sepConfig.modelPath = qEnvironmentVariable("MIXXX_STEM_MODEL");
    sepConfig.outputDir = StemCacheManager::stemDir(key);
    sepConfig.sampleRate = 44100;
    sepConfig.overlap = aiOverlap;
    sepConfig.stemMode = aiStemMode;

    auto* separator = new mixxx::OfflineSeparator(sepConfig);

    // Connect signals with QueuedConnection for thread safety
    QObject::connect(separator, &mixxx::OfflineSeparator::finished,
            this, [this, key, separator](bool success, const StemCacheManager::StemFiles& files) {
                if (success) {
                    kLogger.info() << "AI stem separation completed for:" << key;
                    StemCacheManager::instance().markComplete(key, files);
                    if (!files.stemFile.isEmpty()) {
                        // A native .stem.mp4 was generated: hand it over to
                        // Mixxx's native stem system (SoundSourceSTEM etc).
                        kLogger.info() << "Loading native stem file:" << files.stemFile;
                        m_aiStemsLoading = false;
                        emit aiStemFileReady(files.stemFile);
                    } else {
                        // No native file (e.g. writing failed) - fall back to
                        // the virtual-stems path.
                        loadVirtualStems();
                    }
                } else {
                    kLogger.warning() << "AI stem separation failed for:" << key;
                    StemCacheManager::instance().markFailed(key);
                    m_aiStemsLoading = false;
                    m_aiStemCacheKey = {};
                }
                separator->deleteLater();
            },
            Qt::QueuedConnection);

    QObject::connect(separator, &mixxx::OfflineSeparator::progressChanged,
            this, [this](float progress) {
                Q_UNUSED(progress);
                // Could update a ControlObject for progress display
            },
            Qt::QueuedConnection);

    // N21: chunk-streaming preview. The separator emits partialReady()
    // once after kPartialChunks chunks with a playable
    // {hash}.partial.stem.mp4. Hand it to the native stem system
    // immediately (early playback) while the job keeps running in
    // background; finished() later reloads the deck with the full file.
    // m_aiStemsLoading stays true until the full result arrives.
    QObject::connect(separator, &mixxx::OfflineSeparator::partialReady,
            this, [this, key](bool success, const StemCacheManager::StemFiles& files) {
                if (success && !files.stemFile.isEmpty() &&
                        QFile::exists(files.stemFile)) {
                    kLogger.info() << "AI stem partial ready, early playback:"
                                   << files.stemFile;
                    StemCacheManager::instance().markPartial(key, files);
                    emit aiStemFileReady(files.stemFile);
                }
            },
            Qt::QueuedConnection);

    separator->start();
#endif // __STEM_SEPARATOR__
}

void EngineDeck::loadVirtualStems() {
    if (m_aiStemCacheKey.isEmpty()) {
        return;
    }

    StemCacheManager::StemFiles files = StemCacheManager::instance().getStemFiles(m_aiStemCacheKey);
    if (!files.complete) {
        kLogger.warning() << "Cannot load incomplete virtual stems for:" << m_aiStemCacheKey;
        m_aiStemsLoading = false;
        return;
    }

    m_pVirtualStemSource = std::make_unique<VirtualStemSource>(files, 44100);
    if (m_pVirtualStemSource->load()) {
        kLogger.info() << "Virtual stems loaded successfully";
        // Reset read positions
        for (int i = 0; i < 4; ++i) {
            m_virtualStemReadPosition[i] = 0;
        }
    } else {
        kLogger.warning() << "Failed to load virtual stems";
        m_pVirtualStemSource.reset();
    }
    m_aiStemsLoading = false;
}

void EngineDeck::processVirtualStems(CSAMPLE* pOutput, const std::size_t bufferSize) {
    if (!m_pVirtualStemSource || !m_pVirtualStemSource->isLoaded()) {
        return;
    }

    int numFrames = bufferSize / mixxx::kEngineChannelOutputCount;
    int stemCount = qMin(4, mixxx::kMaxSupportedStems);

    // Clear output buffer
    SampleUtil::clear(pOutput, bufferSize);

    // Mix each stem according to gain/mute/solo settings
    // Solo: if any solo active, only soloed stems sound (mute applied after).
    bool anySolo = false;
    for (int stemIdx = 0; stemIdx < stemCount; ++stemIdx) {
        if (stemIdx < static_cast<int>(m_stemSolo.size()) &&
                m_stemSolo[stemIdx]->toBool()) {
            anySolo = true;
            break;
        }
    }
    for (int stemIdx = 0; stemIdx < stemCount; ++stemIdx) {
        if (stemIdx >= static_cast<int>(m_stemMute.size()) ||
            stemIdx >= static_cast<int>(m_stemGain.size()) ||
            stemIdx >= static_cast<int>(m_stemSolo.size())) {
            continue;
        }

        if (anySolo && !m_stemSolo[stemIdx]->toBool()) {
            continue;
        }

        // Check if stem is muted
        if (m_stemMute[stemIdx]->toBool()) {
            continue;
        }

        float gain = static_cast<float>(m_stemGain[stemIdx]->get());
        if (gain <= 0.0f) {
            continue;
        }

        auto* stemSource = m_pVirtualStemSource->getStemSource(stemIdx);
        if (!stemSource) {
            continue;
        }

        // Read stem audio into temp buffer
        mixxx::SampleBuffer stemBuf(bufferSize);
        stemSource->readFromBuffer(stemBuf.data(), m_virtualStemReadPosition[stemIdx], numFrames);

        // Apply gain and mix into output
        if (gain != 1.0f) {
            SampleUtil::applyGain(stemBuf.data(), gain, bufferSize);
        }
        SampleUtil::add(pOutput, stemBuf.data(), bufferSize);
    }

    // Advance read positions
    for (int stemIdx = 0; stemIdx < stemCount; ++stemIdx) {
        m_virtualStemReadPosition[stemIdx] += numFrames;
    }

    // Wrap around if we reached the end
    qint64 totalFrames = m_pVirtualStemSource->getTotalFrames();
    for (int stemIdx = 0; stemIdx < stemCount; ++stemIdx) {
        if (m_virtualStemReadPosition[stemIdx] >= totalFrames) {
            m_virtualStemReadPosition[stemIdx] = 0; // Loop
        }
    }
}
#endif
