#include "mixer/basetrackplayer.h"

#include <QMessageBox>
#include <QMetaMethod>
#include <memory>

#include "control/controlencoder.h"
#include "control/controlobject.h"
#include "engine/channels/enginedeck.h"
#include "engine/controls/enginecontrol.h"
#include "engine/engine.h"
#include "engine/enginebuffer.h"
#include "engine/enginemixer.h"
#include "engine/sync/enginesync.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "moc_basetrackplayer.cpp"
#include "track/track.h"
#include "util/sandbox.h"
#include "vinylcontrol/defs_vinylcontrol.h"
#include "waveform/renderers/waveformwidgetrenderer.h"

namespace {

constexpr double kNoTrackColor = -1;
constexpr double kShiftCuesOffsetMillis = 10;
constexpr double kShiftCuesOffsetSmallMillis = 1;
const QString kEffectGroupFormat = QStringLiteral("[EqualizerRack1_%1_Effect1]");

inline double trackColorToDouble(mixxx::RgbColor::optional_t color) {
    return (color ? static_cast<double>(*color) : kNoTrackColor);
}
} // namespace

// EveOSC
extern std::atomic<bool> s_oscEnabled;
void oscChangedPlayState(
        const QString& oscGroup,
        float playstate);
// EveOSC

BaseTrackPlayer::BaseTrackPlayer(PlayerManager* pParent, const QString& group)
        : BasePlayer(pParent, group) {
}

BaseTrackPlayerImpl::BaseTrackPlayerImpl(
        PlayerManager* pParent,
        UserSettingsPointer pConfig,
        EngineMixer* pMixingEngine,
        EffectsManager* pEffectsManager,
        EngineChannel::ChannelOrientation defaultOrientation,
        const ChannelHandleAndGroup& handleGroup,
        bool defaultMainMix,
        bool defaultHeadphones,
        bool primaryDeck)
        : BaseTrackPlayer(pParent, handleGroup.name()),
          m_pConfig(pConfig),
          m_pEngineMixer(pMixingEngine),
          m_pLoadedTrack(),
          m_pPrevFailedTrackId(),
          m_replaygainPending(false),
          m_pChannelToCloneFrom(nullptr) {
    auto channel = std::make_unique<EngineDeck>(handleGroup,
            pConfig,
            pMixingEngine,
            pEffectsManager,
            defaultOrientation,
            primaryDeck);
    m_pChannel = channel.get();

    m_pInputConfigured = make_parented<ControlProxy>(getGroup(), "input_configured", this);
#ifdef __VINYLCONTROL__
    m_pVinylControlEnabled = make_parented<ControlProxy>(getGroup(), "vinylcontrol_enabled", this);
    m_pVinylControlEnabled->connectValueChanged(this, &BaseTrackPlayerImpl::slotVinylControlEnabled);
    m_pVinylControlStatus = make_parented<ControlProxy>(getGroup(), "vinylcontrol_status", this);
#endif

    EngineBuffer* pEngineBuffer = m_pChannel->getEngineBuffer();
    pMixingEngine->addChannel(std::move(channel));

    // Set the routing option defaults for the main and headphone mixes.
    m_pChannel->setMainMix(defaultMainMix);
    m_pChannel->setPfl(defaultHeadphones);

    // Connect our signals and slots with the EngineBuffer's signals and
    // slots. This will let us know when the reader is done loading a track, and
    // let us request that the reader load a track.
    connect(pEngineBuffer, &EngineBuffer::trackLoaded, this, &BaseTrackPlayerImpl::slotTrackLoaded);
    connect(pEngineBuffer,
            &EngineBuffer::trackLoadFailed,
            this,
            &BaseTrackPlayerImpl::slotLoadFailed);

    m_pEject = std::make_unique<ControlPushButton>(ConfigKey(getGroup(), "eject"));
    connect(m_pEject.get(),
            &ControlObject::valueChanged,
            this,
            &BaseTrackPlayerImpl::slotEjectTrack);

    // Get loop point control objects
    m_pLoopInPoint = make_parented<ControlProxy>(
            getGroup(), "loop_start_position", this);
    m_pLoopOutPoint = make_parented<ControlProxy>(
            getGroup(), "loop_end_position", this);

    // Duration of the current song, we create this one because nothing else does.
    m_pDuration = std::make_unique<ControlObject>(
        ConfigKey(getGroup(), "duration"));

    // Track color of the current track
    m_pTrackColor = std::make_unique<ControlObject>(
            ConfigKey(getGroup(), "track_color"));
    m_pTrackColor->set(kNoTrackColor);
    m_pTrackColor->connectValueChangeRequest(
            this, &BaseTrackPlayerImpl::slotTrackColorChangeRequest);

    m_pTrackColorPrev = std::make_unique<ControlPushButton>(
            ConfigKey(getGroup(), "track_color_prev"));
    connect(m_pTrackColorPrev.get(),
            &ControlPushButton::valueChanged,
            this,
            [this](double value) {
                if (value > 0) {
                    BaseTrackPlayerImpl::slotTrackColorSelector(-1);
                }
            });

    m_pTrackColorNext = std::make_unique<ControlPushButton>(
            ConfigKey(getGroup(), "track_color_next"));
    connect(m_pTrackColorNext.get(),
            &ControlPushButton::valueChanged,
            this,
            [this](double value) {
                if (value > 0) {
                    BaseTrackPlayerImpl::slotTrackColorSelector(1);
                }
            });

    m_pTrackColorSelect = std::make_unique<ControlEncoder>(
            ConfigKey(getGroup(), "track_color_selector"), false);
    connect(m_pTrackColorSelect.get(),
            &ControlEncoder::valueChanged,
            this,
            [this](double steps) {
                int iSteps = static_cast<int>(steps);
                BaseTrackPlayerImpl::slotTrackColorSelector(iSteps);
            });

    m_pStarsUp = std::make_unique<ControlPushButton>(ConfigKey(getGroup(), "stars_up"));
    connect(m_pStarsUp.get(),
            &ControlObject::valueChanged,
            this,
            [this](double value) {
                if (value > 0) {
                    slotTrackRatingChangeRequestRelative(1);
                }
            });
    m_pStarsDown = std::make_unique<ControlPushButton>(ConfigKey(getGroup(), "stars_down"));
    connect(m_pStarsDown.get(),
            &ControlObject::valueChanged,
            this,
            [this](double value) {
                if (value > 0) {
                    slotTrackRatingChangeRequestRelative(-1);
                }
            });

    // Deck cloning
    m_pCloneFromDeck = std::make_unique<ControlObject>(
            ConfigKey(getGroup(), "CloneFromDeck"),
            false);
    connect(m_pCloneFromDeck.get(),
            &ControlObject::valueChanged,
            this,
            &BaseTrackPlayerImpl::slotCloneFromDeck);

    // Sampler cloning
    m_pCloneFromSampler = std::make_unique<ControlObject>(
            ConfigKey(getGroup(), "CloneFromSampler"),
            false);
    connect(m_pCloneFromSampler.get(),
            &ControlObject::valueChanged,
            this,
            &BaseTrackPlayerImpl::slotCloneFromSampler);

    // Load track from other deck/sampler
    m_pLoadTrackFromDeck = std::make_unique<ControlObject>(
            ConfigKey(getGroup(), "LoadTrackFromDeck"),
            false);
    connect(m_pLoadTrackFromDeck.get(),
            &ControlObject::valueChanged,
            this,
            &BaseTrackPlayerImpl::slotLoadTrackFromDeck);
    m_pLoadTrackFromSampler = std::make_unique<ControlObject>(
            ConfigKey(getGroup(), "LoadTrackFromSampler"),
            false);
    connect(m_pLoadTrackFromSampler.get(),
            &ControlObject::valueChanged,
            this,
            &BaseTrackPlayerImpl::slotLoadTrackFromSampler);

    // Waveform controls
    // This acts somewhat like a ControlPotmeter, but the normal _up/_down methods
    // do not work properly with this CO.
    m_pWaveformZoom =
            std::make_unique<ControlObject>(ConfigKey(getGroup(), "waveform_zoom"));
    m_pWaveformZoom->connectValueChangeRequest(this,
            &BaseTrackPlayerImpl::slotWaveformZoomValueChangeRequest,
            Qt::DirectConnection);
    m_pWaveformZoom->set(1.0);
    m_pWaveformZoomUp = std::make_unique<ControlPushButton>(
            ConfigKey(getGroup(), "waveform_zoom_up"));
    connect(m_pWaveformZoomUp.get(),
            &ControlPushButton::valueChanged,
            this,
            &BaseTrackPlayerImpl::slotWaveformZoomUp);
    m_pWaveformZoomDown = std::make_unique<ControlPushButton>(
            ConfigKey(getGroup(), "waveform_zoom_down"));
    connect(m_pWaveformZoomDown.get(),
            &ControlPushButton::valueChanged,
            this,
            &BaseTrackPlayerImpl::slotWaveformZoomDown);
    m_pWaveformZoomSetDefault = std::make_unique<ControlPushButton>(
            ConfigKey(getGroup(), "waveform_zoom_set_default"));
    connect(m_pWaveformZoomSetDefault.get(),
            &ControlPushButton::valueChanged,
            this,
            &BaseTrackPlayerImpl::slotWaveformZoomSetDefault);

    m_pPreGain = make_parented<ControlProxy>(getGroup(), "pregain", this);

    m_pShiftCuesEarlier = std::make_unique<ControlPushButton>(
            ConfigKey(getGroup(), "shift_cues_earlier"));
    connect(m_pShiftCuesEarlier.get(),
            &ControlObject::valueChanged,
            this,
            [this](double value) { slotShiftCuesMillisButton(value, -kShiftCuesOffsetMillis); });
    m_pShiftCuesLater = std::make_unique<ControlPushButton>(
            ConfigKey(getGroup(), "shift_cues_later"));
    connect(m_pShiftCuesLater.get(),
            &ControlObject::valueChanged,
            this,
            [this](double value) { slotShiftCuesMillisButton(value, kShiftCuesOffsetMillis); });
    m_pShiftCuesEarlierSmall = std::make_unique<ControlPushButton>(
            ConfigKey(getGroup(), "shift_cues_earlier_small"));
    connect(m_pShiftCuesEarlierSmall.get(),
            &ControlObject::valueChanged,
            this,
            [this](double value) {
                slotShiftCuesMillisButton(value, -kShiftCuesOffsetSmallMillis);
            });
    m_pShiftCuesLaterSmall = std::make_unique<ControlPushButton>(
            ConfigKey(getGroup(), "shift_cues_later_small"));
    connect(m_pShiftCuesLaterSmall.get(),
            &ControlObject::valueChanged,
            this,
            [this](double value) {
                slotShiftCuesMillisButton(value, kShiftCuesOffsetSmallMillis);
            });
    m_pShiftCues = std::make_unique<ControlObject>(
            ConfigKey(getGroup(), "shift_cues"));
    connect(m_pShiftCues.get(),
            &ControlObject::valueChanged,
            this,
            &BaseTrackPlayerImpl::slotShiftCuesMillis);

    // BPM and key of the current song
    m_pFileBPM = std::make_unique<ControlObject>(ConfigKey(getGroup(), "file_bpm"));
    m_pVisualBpm = std::make_unique<ControlObject>(ConfigKey(getGroup(), "visual_bpm"));
    m_pKey = make_parented<ControlProxy>(getGroup(), "file_key", this);
    m_pVisualKey = std::make_unique<ControlObject>(ConfigKey(getGroup(), "visual_key"));

    m_pTimeElapsed = std::make_unique<ControlObject>(ConfigKey(getGroup(), "time_elapsed"));
    m_pTimeRemaining = std::make_unique<ControlObject>(ConfigKey(getGroup(), "time_remaining"));
    m_pEndOfTrack = std::make_unique<ControlObject>(ConfigKey(getGroup(), "end_of_track"));

    m_pReplayGain = make_parented<ControlProxy>(getGroup(), "replaygain", this);
    m_pPlay = make_parented<ControlProxy>(getGroup(), "play", this);
    m_pPlay->connectValueChanged(this, &BaseTrackPlayerImpl::slotPlayToggled);

    m_pRateRatio = make_parented<ControlProxy>(getGroup(), "rate_ratio", this);
    m_pPitchAdjust = make_parented<ControlProxy>(getGroup(), "pitch_adjust", this);

    m_pUpdateReplayGainFromPregain = std::make_unique<ControlPushButton>(
            ConfigKey(getGroup(), "update_replaygain_from_pregain"));
    m_pUpdateReplayGainFromPregain->connectValueChangeRequest(this,
            &BaseTrackPlayerImpl::slotUpdateReplayGainFromPregain);

    m_ejectTimer.start();

    if (!primaryDeck) {
        return;
    }

#ifdef __STEM__
    m_pStemColors.reserve(mixxx::kMaxSupportedStems);
    QString group = getGroup();
    for (int stemIdx = 0; stemIdx < mixxx::kMaxSupportedStems; stemIdx++) {
        QString stemGroup = EngineDeck::getGroupForStem(group, stemIdx);
        m_pStemColors.emplace_back(std::make_unique<ControlObject>(
                ConfigKey(stemGroup, QStringLiteral("color"))));
        m_pStemColors.back()->set(kNoTrackColor);
        m_pStemColors.back()->setReadOnly();
    }
#endif
}

BaseTrackPlayerImpl::~BaseTrackPlayerImpl() {
    unloadTrack();
}

TrackPointer BaseTrackPlayerImpl::loadFakeTrack(bool bPlay, double filebpm) {
    TrackPointer pTrack = Track::newTemporary();
    pTrack->setAudioProperties(
            mixxx::kEngineChannelOutputCount,
            mixxx::audio::SampleRate(44100),
            mixxx::audio::Bitrate(),
            mixxx::Duration::fromSeconds(10));
    if (filebpm > 0) {
        pTrack->trySetBpm(filebpm);
    }

    TrackPointer pOldTrack = m_pLoadedTrack;
    m_pLoadedTrack = pTrack;
    if (m_pLoadedTrack) {
        // Listen for updates to the file's BPM
        connectLoadedTrack();
    }

    // Request a new track from EngineBuffer
    EngineBuffer* pEngineBuffer = m_pChannel->getEngineBuffer();
    pEngineBuffer->loadFakeTrack(pTrack, bPlay);

    // await slotTrackLoaded()/slotLoadFailed()
    emit loadingTrack(pTrack, pOldTrack);

    return pTrack;
}

void BaseTrackPlayerImpl::loadTrack(TrackPointer pTrack) {
    DEBUG_ASSERT(!m_pLoadedTrack);

    m_pLoadedTrack = std::move(pTrack);
    if (!m_pLoadedTrack) {
        // nothing to
        return;
    }

    // Clear loop
    // It seems that the trick is to first clear the loop out point, and then
    // the loop in point. If we first clear the loop in point, the loop out point
    // does not get cleared.
    m_pLoopOutPoint->set(kNoTrigger);
    m_pLoopInPoint->set(kNoTrigger);

    // The loop in and out points must be set here and not in slotTrackLoaded
    // so LoopingControl::trackLoaded can access them.
    if (!m_pChannelToCloneFrom) {
        // Restore loop from the first loop cue with minimum hotcue number.
        // For the volatile "most recent loop" the hotcue number will be -1.
        // If no such loop exists, restore a saved loop cue.
        CuePointer pLoopCue;
        const QList<CuePointer> trackCues = m_pLoadedTrack->getCuePoints();
        for (const auto& pCue : trackCues) {
            if (pCue->getType() != mixxx::CueType::Loop) {
                continue;
            }
            if (pLoopCue && pLoopCue->getHotCue() <= pCue->getHotCue()) {
                continue;
            }
            pLoopCue = pCue;
        }

        if (pLoopCue) {
            const auto loop = pLoopCue->getStartAndEndPosition();
            if (loop.startPosition.isValid() && loop.endPosition.isValid() &&
                    loop.startPosition <= loop.endPosition) {
                // TODO: For all loop cues, both end and start positions should
                // be valid and the end position should be greater than the
                // start position. We should use a VERIFY_OR_DEBUG_ASSERT to
                // check this. To make this possible, we need to ensure that
                // all invalid cues are discarded when saving cues to the
                // database first.
                m_pLoopInPoint->set(loop.startPosition.toEngineSamplePos());
                m_pLoopOutPoint->set(loop.endPosition.toEngineSamplePos());
            }
        }
    } else {
        // copy loop in and out points from other deck because any new loops
        // won't be saved yet
        m_pLoopInPoint->set(ControlObject::get(
                ConfigKey(m_pChannelToCloneFrom->getGroup(), "loop_start_position")));
        m_pLoopOutPoint->set(ControlObject::get(
                ConfigKey(m_pChannelToCloneFrom->getGroup(), "loop_end_position")));

#ifdef __STEM__
        auto* pDeckToClone = qobject_cast<EngineDeck*>(m_pChannelToCloneFrom);
        if (pDeckToClone && m_pLoadedTrack && m_pLoadedTrack->hasStem() && m_pChannel) {
            m_pChannel->cloneStemState(pDeckToClone);
        }
#endif
    }

    connectLoadedTrack();
}

void BaseTrackPlayerImpl::slotEjectTrack(double v) {
    if (v <= 0) {
        return;
    }

    // Don't allow eject while playing a track. We don't need to lock to
    // call ControlObject::get() so this is fine.
    if (m_pPlay->toBool()) {
        return;
    }

    mixxx::Duration elapsed = m_ejectTimer.restart();

    // Double-click always restores the last replaced track, i.e. un-eject the second
    // last track: the first click ejects or unejects, and the second click reloads.
    if (elapsed < mixxx::Duration::fromMillis(kUnreplaceDelay)) {
        TrackPointer lastEjected = m_pPlayerManager->getSecondLastEjectedTrack();
        if (lastEjected) {
            slotLoadTrack(lastEjected,
#ifdef __STEM__
                    mixxx::StemChannelSelection(),
#endif
                    false);
        }
        return;
    }

    // With no loaded track a single click reloads the last ejected track.
    if (!m_pLoadedTrack) {
        TrackPointer lastEjected = m_pPlayerManager->getLastEjectedTrack();
        if (lastEjected) {
            slotLoadTrack(lastEjected,
#ifdef __STEM__
                    mixxx::StemChannelSelection(),
#endif
                    false);
        }
        return;
    }

    m_pChannel->getEngineBuffer()->ejectTrack();
}

TrackPointer BaseTrackPlayerImpl::unloadTrack() {
    if (!m_pLoadedTrack) {
        // nothing to do
        return TrackPointer();
    }
    PlayerInfo::instance().setTrackInfo(getGroup(), TrackPointer());

    // Save the loop that is currently to the loop cue. If no loop cue is
    // currently on the track, create a new one.
    // If the loop is invalid and a loop cue exists, remove it.
    const auto loopStart =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pLoopInPoint->get());
    const auto loopEnd =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pLoopOutPoint->get());
    CuePointer pLoopCue;
    const QList<CuePointer> cuePoints = m_pLoadedTrack->getCuePoints();
    for (const auto& pCue : cuePoints) {
        if (pCue->getType() == mixxx::CueType::Loop && pCue->getHotCue() == Cue::kNoHotCue) {
            pLoopCue = pCue;
            break;
        }
    }
    if (loopStart.isValid() && loopEnd.isValid() && loopStart <= loopEnd) {
        if (pLoopCue) {
            pLoopCue->setStartAndEndPosition(loopStart, loopEnd);
        } else {
            pLoopCue = m_pLoadedTrack->createAndAddCue(
                    mixxx::CueType::Loop,
                    Cue::kNoHotCue,
                    loopStart,
                    loopEnd);
        }
    } else if (pLoopCue) {
        m_pLoadedTrack->removeCue(pLoopCue);
    }

    disconnectLoadedTrack();

    // Do not reset m_pReplayGain here, because the track might be still
    // playing and the last buffer will be processed.

    m_pPlay->set(0.0);

    TrackPointer pUnloadedTrack(std::move(m_pLoadedTrack));
    DEBUG_ASSERT(!m_pLoadedTrack);
    emit trackUnloaded(pUnloadedTrack);
    return pUnloadedTrack;
}

void BaseTrackPlayerImpl::connectLoadedTrack() {
    connect(m_pLoadedTrack.get(),
            &Track::bpmChanged,
            this,
            [this] {
                TrackPointer pTrack = m_pLoadedTrack;
                if (pTrack) {
                    m_pFileBPM->set(pTrack->getBpm());
                }
            });

    connect(m_pLoadedTrack.get(),
            &Track::keyChanged,
            this,
            [this] {
                TrackPointer pTrack = m_pLoadedTrack;
                if (pTrack) {
                    const auto key = pTrack->getKeys().getGlobalKey();
                    m_pKey->set(static_cast<double>(key));
                }
            });

    // Listen for updates to the file's Replay Gain
    connect(m_pLoadedTrack.get(),
            &Track::replayGainUpdated,
            this,
            &BaseTrackPlayerImpl::slotSetReplayGain);
    connect(m_pLoadedTrack.get(),
            &Track::replayGainAdjusted,
            this,
            &BaseTrackPlayerImpl::slotAdjustReplayGain);

    connect(m_pLoadedTrack.get(),
            &Track::colorUpdated,
            this,
            &BaseTrackPlayerImpl::slotSetTrackColor);

    // Forward the update signal, i.e. use BaseTrackPlayer as relay.
    // Currently only used by WStarRating which is connected in
    // LegacySkinParser::parseStarRating
    connect(m_pLoadedTrack.get(),
            &Track::ratingUpdated,
            this,
            &BaseTrackPlayerImpl::trackRatingChanged);
}

void BaseTrackPlayerImpl::disconnectLoadedTrack() {
    // WARNING: Never. Ever. call bare disconnect() on an object. Mixxx
    // relies on signals and slots to get tons of things done. Don't
    // randomly disconnect things.
    disconnect(m_pLoadedTrack.get(), nullptr, m_pFileBPM.get(), nullptr);
    disconnect(m_pLoadedTrack.get(), nullptr, this, nullptr);
    disconnect(m_pLoadedTrack.get(), nullptr, m_pKey.get(), nullptr);
}

#ifdef __STEM__
void BaseTrackPlayerImpl::slotLoadTrack(TrackPointer pNewTrack,
        mixxx::StemChannelSelection stemMask,
        bool bPlay) {
#else
void BaseTrackPlayerImpl::slotLoadTrack(TrackPointer pNewTrack,
        bool bPlay) {
#endif
    //qDebug() << "BaseTrackPlayerImpl::slotLoadTrack" << getGroup() << pNewTrack.get();
    // Before loading the track, ensure we have access. This uses lazy
    // evaluation to make sure track isn't NULL before we dereference it.
    if (pNewTrack) {
        auto fileInfo = pNewTrack->getFileInfo();
        if (!Sandbox::askForAccess(&fileInfo)) {
            // We don't have access.
            return;
        }
    }

    auto pOldTrack = unloadTrack();

    loadTrack(pNewTrack);

    // await slotTrackLoaded()/slotLoadFailed()
    // emit this before pEngineBuffer->loadTrack() to avoid receiving
    // unexpected slotTrackLoaded() before, in case the track is still cached #10504.
    emit loadingTrack(pNewTrack, pOldTrack);

    // Request a new track from EngineBuffer
    EngineBuffer* pEngineBuffer = m_pChannel->getEngineBuffer();
#ifdef __STEM__
    pEngineBuffer->loadTrack(pNewTrack,
            stemMask,
            bPlay,
            m_pChannelToCloneFrom);

    // Select a specific stem if requested
    emit selectedStems(stemMask);
#else
    pEngineBuffer->loadTrack(pNewTrack, bPlay, m_pChannelToCloneFrom);
#endif
}

void BaseTrackPlayerImpl::slotLoadFailed(TrackPointer pTrack, const QString& reason) {
    // Note: This slot can be a load failure from the current track or a
    // a delayed signal from a previous load.
    // We have probably received a slotTrackLoaded signal, of an old track that
    // was loaded before. Here we must unload the
    // We must unload the track m_pLoadedTrack as well
    if (pTrack == m_pLoadedTrack) {
        qDebug() << "Failed to load track" << pTrack->getFileInfo() << reason;
        slotTrackLoaded(TrackPointer(), pTrack);
    } else if (pTrack) {
        qDebug() << "Stray failed to load track" << pTrack->getFileInfo() << reason;
    } else {
        qDebug() << "Failed to load track (NULL track object)" << reason;
    }
    m_pChannelToCloneFrom = nullptr;

    // Alert user.
    // The QMessageBox blocks the event loop (and the GUI since it's modal dialog),
    // though if a controller's Load button was pressed repeatedly we may get
    // multiple identical messages for the same track.
    // Avoid this and show only one message per track track at a time.
    if (pTrack && m_pPrevFailedTrackId == pTrack->getId()) {
        return;
    } else if (pTrack) {
        m_pPrevFailedTrackId = pTrack->getId();
    }
    QMessageBox::warning(nullptr, tr("Couldn't load track."), reason);
    m_pPrevFailedTrackId = TrackId();
}

void BaseTrackPlayerImpl::slotTrackLoaded(TrackPointer pNewTrack,
                                          TrackPointer pOldTrack) {
    //qDebug() << "BaseTrackPlayerImpl::slotTrackLoaded" << pNewTrack.get() << pOldTrack.get();
    if (!pNewTrack &&
            pOldTrack &&
            pOldTrack == m_pLoadedTrack) {
        // eject Track
        unloadTrack();

        // Causes the track's data to be saved back to the library database and
        // for all the widgets to change the track and update themselves.
        emit loadingTrack(pNewTrack, pOldTrack);
        m_pDuration->set(0);
        m_pFileBPM->set(0);
        m_pKey->set(0);
        slotSetTrackColor(std::nullopt);
        m_pLoopInPoint->set(kNoTrigger);
        m_pLoopOutPoint->set(kNoTrigger);
        m_pLoadedTrack.reset();
        emit playerEmpty();
        emit trackRatingChanged(0);
    } else if (pNewTrack && pNewTrack == m_pLoadedTrack) {
        // NOTE(uklotzde): In a previous version track metadata was reloaded
        // from the source file at this point again. This is no longer necessary
        // since track objects will always be created in a controlled manner
        // and populated from the database and their source file as required
        // before handing them out to application code.
        // TODO(XXX): Don't hesitate to delete the preceding NOTE if you think
        // that it is not needed anymore.

        // Update the BPM and duration values that are stored in ControlObjects
        m_pDuration->set(m_pLoadedTrack->getDuration());
        m_pFileBPM->set(m_pLoadedTrack->getBpm());
        m_pKey->set(m_pLoadedTrack->getKey());
        slotSetTrackColor(m_pLoadedTrack->getColor());

        if(m_pConfig->getValue(
                ConfigKey("[Mixer Profile]", "EqAutoReset"), false)) {
            if (m_pLowFilter) {
                m_pLowFilter->set(1.0);
            }
            if (m_pMidFilter) {
                m_pMidFilter->set(1.0);
            }
            if (m_pHighFilter) {
                m_pHighFilter->set(1.0);
            }
            if (m_pLowFilterKill) {
                m_pLowFilterKill->set(0.0);
            }
            if (m_pMidFilterKill) {
                m_pMidFilterKill->set(0.0);
            }
            if (m_pHighFilterKill) {
                m_pHighFilterKill->set(0.0);
            }
        }
        if (m_pConfig->getValue(
                ConfigKey("[Mixer Profile]", "GainAutoReset"), false)) {
            m_pPreGain->set(1.0);
        }

        if (!m_pChannelToCloneFrom) {
            BaseTrackPlayer::TrackLoadReset reset = m_pConfig->getValue(
                    ConfigKey("[Controls]", "SpeedAutoReset"), RESET_PITCH);
            if (reset == RESET_SPEED || reset == RESET_PITCH_AND_SPEED) {
                // Avoid resetting speed if sync lock is enabled and other decks with sync enabled
                // are playing, as this would change the speed of already playing decks.
                if (!m_pEngineMixer->getEngineSync()->otherSyncedPlaying(getGroup())) {
                    m_pRateRatio->set(1.0);
                }
            }
            if (reset == RESET_PITCH || reset == RESET_PITCH_AND_SPEED) {
                m_pPitchAdjust->set(0.0);
            }
        } else {
            // perform a clone of the given channel

            // Don't touch the rate_ratio if it is follower under sync control
            // During sync this is applied to all synced decks
            if (ControlObject::get(ConfigKey(getGroup(), "sync_mode")) !=
                    static_cast<double>(SyncMode::Follower)) {
                m_pRateRatio->set(ControlObject::get(ConfigKey(
                        m_pChannelToCloneFrom->getGroup(), "rate_ratio")));
            }

            // copy pitch
            m_pPitchAdjust->set(ControlObject::get(ConfigKey(
                    m_pChannelToCloneFrom->getGroup(), "pitch_adjust")));

            // copy the loop state
            if (ControlObject::get(ConfigKey(m_pChannelToCloneFrom->getGroup(), "loop_enabled")) == 1.0) {
                ControlObject::set(ConfigKey(getGroup(), "reloop_toggle"), 1.0);
            }
        }

#ifdef __STEM__
        if (m_pStemColors.size()) {
            const auto& stemInfo = m_pLoadedTrack->getStemInfo();
            DEBUG_ASSERT(stemInfo.size() <= mixxx::kMaxSupportedStems);
            int stemIdx = 0;
            for (const auto& stemColorCo : m_pStemColors) {
                auto color = kNoTrackColor;
                if (stemIdx < stemInfo.size()) {
                    color = trackColorToDouble(mixxx::RgbColor::fromQColor(
                            stemInfo.at(stemIdx).getColor()));
                }
                stemColorCo->forceSet(color);
                stemIdx++;
            }
        }
#endif

        emit newTrackLoaded(m_pLoadedTrack);
        emit trackRatingChanged(m_pLoadedTrack->getRating());
    } else {
        // this is the result from an outdated load or unload signal
        // A new load is already pending
        // Ignore this signal and wait for the new one
        qDebug() << "stray BaseTrackPlayerImpl::slotTrackLoaded()";
    }

    m_pChannelToCloneFrom = nullptr;

    // Update the PlayerInfo class that is used in EngineBroadcast to replace
    // the metadata of a stream
    PlayerInfo::instance().setTrackInfo(getGroup(), m_pLoadedTrack);
}

TrackPointer BaseTrackPlayerImpl::getLoadedTrack() const {
    return m_pLoadedTrack;
}

void BaseTrackPlayerImpl::slotCloneDeck() {
    Syncable* syncable = m_pEngineMixer->getEngineSync()->pickNonSyncSyncTarget(m_pChannel);
    if (syncable) {
        slotCloneChannel(syncable->getChannel());
    }
}

void BaseTrackPlayerImpl::slotCloneFromGroup(const QString& group) {
    EngineChannel* pChannel = m_pEngineMixer->getChannel(group);
    if (!pChannel) {
        return;
    }

    slotCloneChannel(pChannel);
}

void BaseTrackPlayerImpl::slotCloneFromDeck(double d) {
    int deck = static_cast<int>(d);
    if (deck < 1) {
        slotCloneDeck();
    } else {
        slotCloneFromGroup(PlayerManager::groupForDeck(deck - 1));
    }
}

void BaseTrackPlayerImpl::slotCloneFromSampler(double d) {
    int sampler = static_cast<int>(d);
    if (sampler >= 1) {
        slotCloneFromGroup(PlayerManager::groupForSampler(sampler - 1));
    }
}

void BaseTrackPlayerImpl::slotCloneChannel(EngineChannel* pChannel) {
    // don't clone from ourselves
    if (!pChannel || pChannel == m_pChannel) {
        return;
    }

    TrackPointer pTrack = pChannel->getEngineBuffer()->getLoadedTrack();
    if (!pTrack) {
        return;
    }

    m_pChannelToCloneFrom = pChannel;
    bool play = ControlObject::toBool(ConfigKey(m_pChannelToCloneFrom->getGroup(), "play"));
    slotLoadTrack(pTrack,
#ifdef __STEM__
            mixxx::StemChannelSelection(),
#endif
            play);
}

void BaseTrackPlayerImpl::slotLoadTrackFromDeck(double d) {
    int deck = static_cast<int>(d);
    loadTrackFromGroup(PlayerManager::groupForDeck(deck - 1));
}

void BaseTrackPlayerImpl::slotLoadTrackFromSampler(double d) {
    int sampler = static_cast<int>(d);
    loadTrackFromGroup(PlayerManager::groupForSampler(sampler - 1));
}

void BaseTrackPlayerImpl::loadTrackFromGroup(const QString& group) {
    EngineChannel* pChannel = m_pEngineMixer->getChannel(group);
    if (!pChannel) {
        return;
    }

    TrackPointer pTrack = pChannel->getEngineBuffer()->getLoadedTrack();
    if (!pTrack) {
        return;
    }

    slotLoadTrack(pTrack,
#ifdef __STEM__
            mixxx::StemChannelSelection(),
#endif
            false);
}

bool BaseTrackPlayerImpl::isTrackMenuControlAvailable() {
    if (m_pShowTrackMenuControl == nullptr) {
        // Create the control and return true so LegacySkinParser knows it should
        // connect our signal to WTrackProperty.
        m_pShowTrackMenuControl = std::make_unique<ControlPushButton>(
                ConfigKey(getGroup(), "show_track_menu"));
        m_pShowTrackMenuControl->connectValueChangeRequest(
                this,
                [this](double value) {
                    emit trackMenuChangeRequest(value > 0);
                });
        return true;
    } else if (isSignalConnected(
                       QMetaMethod::fromSignal(&BaseTrackPlayer::trackMenuChangeRequest))) {
        // Control exists and we're already connected.
        // This means the request was made while creating the 2nd or later WTrackProperty.
        return false;
    } else {
        // Control already exists but signal is not connected, which is the case
        // after loading a skin. Return true so LegacySkinParser makes a new connection.
        return true;
    }
}

void BaseTrackPlayerImpl::slotSetAndConfirmTrackMenuControl(bool visible) {
    VERIFY_OR_DEBUG_ASSERT(m_pShowTrackMenuControl) {
        return;
    }
    m_pShowTrackMenuControl->setAndConfirm(visible ? 1.0 : 0.0);
}

void BaseTrackPlayerImpl::slotSetReplayGain(mixxx::ReplayGain replayGain) {
    // Do not change replay gain when track is playing because
    // this may lead to an unexpected volume change.
    if (m_pPlay->get() == 0.0) {
        setReplayGain(replayGain.getRatio());
    } else {
        m_replaygainPending = true;
    }
}

void BaseTrackPlayerImpl::slotAdjustReplayGain(mixxx::ReplayGain replayGain) {
    const double factor = m_pReplayGain->get() / replayGain.getRatio();
    const double newPregain = m_pPreGain->get() * factor;

    // There is a very slight chance that there will be a buffer call in between these sets.
    // Therefore, we first adjust the control that is being lowered before the control
    // that is being raised.  Worst case, the volume goes down briefly before rectifying.
    if (factor < 1.0) {
        m_pPreGain->set(newPregain);
        setReplayGain(replayGain.getRatio());
    } else {
        setReplayGain(replayGain.getRatio());
        m_pPreGain->set(newPregain);
    }
}

void BaseTrackPlayerImpl::slotSetTrackColor(const mixxx::RgbColor::optional_t& color) {
    m_pTrackColor->forceSet(trackColorToDouble(color));
}

void BaseTrackPlayerImpl::slotTrackColorSelector(int steps) {
    if (!m_pLoadedTrack || steps == 0) {
        return;
    }

    ColorPaletteSettings colorPaletteSettings(m_pConfig);
    ColorPalette colorPalette = colorPaletteSettings.getTrackColorPalette();
    mixxx::RgbColor::optional_t color = m_pLoadedTrack->getColor();

    while (steps != 0) {
        if (steps > 0) {
            color = colorPalette.nextColor(color);
            steps--;
        } else {
            color = colorPalette.previousColor(color);
            steps++;
        }
    }
    m_pLoadedTrack->setColor(color);
}

void BaseTrackPlayerImpl::slotTrackColorChangeRequest(double v) {
    if (!m_pLoadedTrack) {
        return;
    }

    mixxx::RgbColor::optional_t color = std::nullopt;
    if (v != kNoTrackColor) {
        auto colorCode = static_cast<mixxx::RgbColor::code_t>(v);
        if (!mixxx::RgbColor::isValidCode(colorCode)) {
            return;
        }
        color = mixxx::RgbColor::optional(colorCode);
    }
    m_pLoadedTrack->setColor(color);
}

void BaseTrackPlayerImpl::slotTrackRatingChangeRequest(int rating) {
    if (!m_pLoadedTrack) {
        return;
    }
    if (mixxx::TrackRecord::isValidRating(rating) &&
            rating != m_pLoadedTrack->getRating()) {
        m_pLoadedTrack->setRating(rating);
        emit trackRatingChanged(rating);
    }
}

void BaseTrackPlayerImpl::slotTrackRatingChangeRequestRelative(int change) {
    if (!m_pLoadedTrack) {
        return;
    }
    slotTrackRatingChangeRequest(m_pLoadedTrack->getRating() + change);
}

void BaseTrackPlayerImpl::slotPlayToggled(double value) {
    if (value == 0 && m_replaygainPending) {
        setReplayGain(m_pLoadedTrack->getReplayGain().getRatio());
    }
    //  EveOSC begin
    if (s_oscEnabled.load()) {
        oscChangedPlayState(getGroup(), (float)value);
    }
    //  EveOSC end
}

EngineDeck* BaseTrackPlayerImpl::getEngineDeck() const {
    return m_pChannel;
}

void BaseTrackPlayerImpl::setupEqControls() {
    const QString group = kEffectGroupFormat.arg(getGroup());
    m_pLowFilter = make_parented<ControlProxy>(group, QStringLiteral("parameter1"), this);
    m_pMidFilter = make_parented<ControlProxy>(group, QStringLiteral("parameter2"), this);
    m_pHighFilter = make_parented<ControlProxy>(group, QStringLiteral("parameter3"), this);
    m_pLowFilterKill = make_parented<ControlProxy>(
            group, QStringLiteral("button_parameter1"), this);
    m_pMidFilterKill = make_parented<ControlProxy>(
            group, QStringLiteral("button_parameter2"), this);
    m_pHighFilterKill = make_parented<ControlProxy>(
            group, QStringLiteral("button_parameter3"), this);
}

void BaseTrackPlayerImpl::slotVinylControlEnabled(double v) {
#ifdef __VINYLCONTROL__
    bool configured = m_pInputConfigured->toBool();
    bool vinylcontrol_enabled = v > 0.0;

    // Warn the user if they try to enable vinyl control on a player with no
    // configured input.
    if (!configured && vinylcontrol_enabled) {
        m_pVinylControlEnabled->set(0.0);
        m_pVinylControlStatus->set(VINYL_STATUS_DISABLED);
        emit noVinylControlInputConfigured();
    }
#else
    Q_UNUSED(v);
#endif
}

void BaseTrackPlayerImpl::slotWaveformZoomValueChangeRequest(double v) {
    if (v <= WaveformWidgetRenderer::s_waveformMaxZoom
            && v >= WaveformWidgetRenderer::s_waveformMinZoom) {
        m_pWaveformZoom->setAndConfirm(v);
    }
}

void BaseTrackPlayerImpl::slotWaveformZoomUp(double pressed) {
    if (pressed <= 0.0) {
        return;
    }

    m_pWaveformZoom->set(m_pWaveformZoom->get() + 1.0);
}

void BaseTrackPlayerImpl::slotWaveformZoomDown(double pressed) {
    if (pressed <= 0.0) {
        return;
    }

    m_pWaveformZoom->set(m_pWaveformZoom->get() - 1.0);
}

void BaseTrackPlayerImpl::slotWaveformZoomSetDefault(double pressed) {
    if (pressed <= 0.0) {
        return;
    }

    double defaultZoom = m_pConfig->getValue(ConfigKey("[Waveform]", "DefaultZoom"),
            WaveformWidgetRenderer::s_waveformDefaultZoom);
    m_pWaveformZoom->set(defaultZoom);
}

void BaseTrackPlayerImpl::slotShiftCuesMillis(double milliseconds) {
    if (!m_pLoadedTrack) {
        return;
    }
    m_pLoadedTrack->shiftCuePositionsMillis(milliseconds);
}

void BaseTrackPlayerImpl::slotShiftCuesMillisButton(double value, double milliseconds) {
    if (value <= 0) {
        return;
    }
    slotShiftCuesMillis(milliseconds);
}

void BaseTrackPlayerImpl::slotUpdateReplayGainFromPregain(double pressed) {
    if (pressed <= 0) {
        return;
    }
    if (!m_pLoadedTrack) {
        return;
    }
    const double gain = m_pPreGain->get();
    // Gain is at unity already, ignore and return.
    if (gain == 1.0) {
        return;
    }
    m_pLoadedTrack->adjustReplayGainFromPregain(gain);
}

void BaseTrackPlayerImpl::setReplayGain(double value) {
    m_pReplayGain->set(value);
    m_replaygainPending = false;
}
