#include "engine/sync/synccontrol.h"

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "control/controlpushbutton.h"
#include "engine/channels/enginechannel.h"
#include "engine/controls/bpmcontrol.h"
#include "engine/enginebuffer.h"
#include "engine/enginemixer.h"
#include "engine/sync/enginesync.h"
#include "moc_synccontrol.cpp"
#include "track/track.h"
#include "util/assert.h"
#include "util/logger.h"

const double SyncControl::kBpmUnity = 1.0;
const double SyncControl::kBpmHalve = 0.5;
const double SyncControl::kBpmDouble = 2.0;

namespace {
const mixxx::Logger kLogger("SyncControl");
} // namespace

SyncControl::SyncControl(const QString& group,
        UserSettingsPointer pConfig,
        EngineChannel* pChannel,
        EngineSync* pEngineSync)
        : EngineControl(group, pConfig),
          m_sGroup(group),
          m_pChannel(pChannel),
          m_pEngineSync(pEngineSync),
          m_pBpmControl(nullptr),
          m_pRateControl(nullptr),
          m_bOldScratching(false),
          m_leaderBpmAdjustFactor(kBpmUnity),
          m_unmultipliedTargetBeatDistance(0.0),
          m_pBpm(nullptr),
          m_pLocalBpm(nullptr),
          m_pRateRatio(nullptr),
          m_pVCEnabled(nullptr),
          m_pSyncPhaseButton(nullptr) {
    // Play button.  We only listen to this to disable leader if the deck is
    // stopped.
    m_pPlayButton = new ControlProxy(group, "play", this);
    m_pPlayButton->connectValueChanged(this, &SyncControl::slotControlPlay, Qt::DirectConnection);

    m_pSyncMode.reset(new ControlPushButton(ConfigKey(group, "sync_mode")));
    m_pSyncMode->setBehavior(mixxx::control::ButtonMode::Toggle,
            static_cast<int>(SyncMode::NumModes));
    m_pSyncMode->connectValueChangeRequest(
            this, &SyncControl::slotSyncModeChangeRequest, Qt::DirectConnection);

    m_pSyncLeaderEnabled.reset(
            new ControlPushButton(ConfigKey(group, "sync_leader")));
    m_pSyncLeaderEnabled->setBehavior(mixxx::control::ButtonMode::Toggle, 3);
    m_pSyncLeaderEnabled->connectValueChangeRequest(
            this, &SyncControl::slotSyncLeaderEnabledChangeRequest, Qt::DirectConnection);
    m_pSyncLeaderEnabled->addAlias(ConfigKey(group, QStringLiteral("sync_master")));

    m_pSyncEnabled.reset(
            new ControlPushButton(ConfigKey(group, "sync_enabled")));
    m_pSyncEnabled->setButtonMode(mixxx::control::ButtonMode::LongPressLatching);
    m_pSyncEnabled->connectValueChangeRequest(
            this, &SyncControl::slotSyncEnabledChangeRequest, Qt::DirectConnection);

    // Beat sync (scale buffer tempo relative to tempo of other buffer)
    m_pButtonSync = new ControlPushButton(ConfigKey(group, "beatsync"));
    connect(m_pButtonSync,
            &ControlObject::valueChanged,
            this,
            &SyncControl::slotControlBeatSync,
            Qt::DirectConnection);

    m_pButtonSyncPhase = new ControlPushButton(ConfigKey(group, "beatsync_phase"));
    connect(m_pButtonSyncPhase,
            &ControlObject::valueChanged,
            this,
            &SyncControl::slotControlBeatSyncPhase,
            Qt::DirectConnection);

    m_pButtonSyncTempo = new ControlPushButton(ConfigKey(group, "beatsync_tempo"));
    connect(m_pButtonSyncTempo,
            &ControlObject::valueChanged,
            this,
            &SyncControl::slotControlBeatSyncTempo,
            Qt::DirectConnection);

    // The relative position between two beats in the range 0.0 ... 1.0
    m_pBeatDistance.reset(
            new ControlObject(ConfigKey(group, "beat_distance")));

    m_pPassthroughEnabled = new ControlProxy(group, "passthrough", this);
    m_pPassthroughEnabled->connectValueChanged(this,
            &SyncControl::slotPassthroughChanged,
            Qt::DirectConnection);

    m_pQuantize = new ControlProxy(group, "quantize", this);

    // BPMControl and RateControl will be initialized later.
}

SyncControl::~SyncControl() {
    delete m_pButtonSync;
    delete m_pButtonSyncPhase;
    delete m_pButtonSyncTempo;
}

void SyncControl::setEngineControls(RateControl* pRateControl,
        BpmControl* pBpmControl) {
    m_pRateControl = pRateControl;
    m_pBpmControl = pBpmControl;

    // We set this to change the effective BPM in BpmControl. We do not listen
    // to changes from this control because changes in rate, rate_dir, rateRange
    // and file_bpm result in changes to this control.
    m_pBpm = new ControlProxy(getGroup(), "bpm", this);

    m_pLocalBpm = new ControlProxy(getGroup(), "local_bpm", this);

    m_pRateRatio = new ControlProxy(getGroup(), "rate_ratio", this);
    m_pRateRatio->connectValueChanged(this, &SyncControl::slotRateChanged, Qt::DirectConnection);

    m_pSyncPhaseButton = new ControlProxy(getGroup(), "beatsync_phase", this);

#ifdef __VINYLCONTROL__
    m_pVCEnabled = new ControlProxy(
            getGroup(), "vinylcontrol_enabled", this);

    // Throw a hissy fit if somebody moved us such that the vinylcontrol_enabled
    // control doesn't exist yet. This will blow up immediately, won't go unnoticed.
    DEBUG_ASSERT(m_pVCEnabled->valid());

    m_pVCEnabled->connectValueChanged(
            this, &SyncControl::slotVinylControlChanged, Qt::DirectConnection);
#endif
}

SyncMode SyncControl::getSyncMode() const {
    return syncModeFromDouble(m_pSyncMode->get());
}

void SyncControl::setSyncMode(SyncMode mode) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "SyncControl::setSyncMode" << getGroup() << mode;
    }
    // SyncControl has absolutely no say in the matter. This is what EngineSync
    // requires. Bypass confirmation by using setAndConfirm.
    m_pSyncMode->setAndConfirm(static_cast<double>(mode));
    m_pSyncEnabled->setAndConfirm(mode != SyncMode::None);
    m_pSyncLeaderEnabled->setAndConfirm(static_cast<double>(SyncModeToLeaderLight(mode)));
    if (mode == SyncMode::Follower) {
        if (m_pVCEnabled && m_pVCEnabled->toBool()) {
            // If follower mode is enabled, disable vinyl control.
            m_pVCEnabled->set(0.0);
        }
    }
    if (mode != SyncMode::None && m_pPassthroughEnabled->toBool()) {
        // If any sync mode is enabled and passthrough was on somehow, disable passthrough.
        // This is very unlikely to happen so this deserves a warning.
        qWarning() << "Notified of sync mode change when passthrough was "
                      "active -- "
                      "must disable passthrough";
        m_pPassthroughEnabled->set(0.0);
    }
    if (mode == SyncMode::None) {
        m_leaderBpmAdjustFactor = kBpmUnity;
    }
}

void SyncControl::notifyUniquePlaying() {
    // If we are the only remaining playing sync deck, we can reset the user
    // tweak info.
    m_pBpmControl->resetSyncAdjustment();
}

void SyncControl::requestSync() {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "SyncControl::requestSync" << this->getGroup()
                        << isPlaying() << m_pQuantize->toBool();
    }
    if (isPlaying() && m_pQuantize->toBool()) {
        // only sync phase if the deck is playing and if quantize is enabled.
        // this way the it is up to the user to decide if a seek is desired or not.
        // This is helpful if the beatgrid of the track does not fit at the current
        // playposition
        m_pChannel->getEngineBuffer()->requestSyncPhase();
    }
}

bool SyncControl::isPlaying() const {
    return m_pPlayButton->toBool();
}

bool SyncControl::isAudible() const {
    return m_audible;
}

bool SyncControl::isQuantized() const {
    return m_pQuantize->toBool();
}

double SyncControl::adjustSyncBeatDistance(double beatDistance) const {
    // Similar to adjusting the target beat distance, when we report our beat
    // distance we need to adjust it by the leader bpm adjustment factor.  If
    // we've been doubling the leader bpm, we need to divide it in half.  If
    // we've been halving the leader bpm, we need to double it.  Both operations
    // also need to account for if the longer beat is past its halfway point.
    //
    // This is the inverse of the updateTargetBeatDistance function below.
    if (m_leaderBpmAdjustFactor == kBpmDouble) {
        beatDistance /= kBpmDouble;
        if (m_unmultipliedTargetBeatDistance >= 0.5) {
            beatDistance += 0.5;
        }
    } else if (m_leaderBpmAdjustFactor == kBpmHalve) {
        if (beatDistance >= 0.5) {
            beatDistance -= 0.5;
        }
        beatDistance /= kBpmHalve;
    }
    return beatDistance;
}

double SyncControl::getBeatDistance() const {
    double beatDistance = m_pBeatDistance->get();
    return adjustSyncBeatDistance(beatDistance);
}

mixxx::Bpm SyncControl::getBaseBpm() const {
    const mixxx::Bpm bpm = getLocalBpm();
    if (!bpm.isValid()) {
        return {};
    }
    return mixxx::Bpm(bpm.value() / m_leaderBpmAdjustFactor);
}

void SyncControl::updateLeaderBeatDistance(double beatDistance) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << getGroup() << "SyncControl::updateLeaderBeatDistance"
                        << beatDistance;
    }
    // Set the BpmControl target beat distance to beatDistance, adjusted by
    // the multiplier if in effect.  This way all of the multiplier logic
    // is contained in this single class.
    m_unmultipliedTargetBeatDistance = beatDistance;
    // Update the target beat distance based on the multiplier.
    updateTargetBeatDistance();
}

void SyncControl::updateLeaderBpm(mixxx::Bpm bpm) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << getGroup() << "SyncControl::updateLeaderBpm" << bpm;
    }

    VERIFY_OR_DEBUG_ASSERT(isSynchronized()) {
        qWarning() << "WARNING: Logic Error: setBpm called on SyncMode::None syncable.";
        return;
    }

    // Vinyl Control overrides.
    if (m_pVCEnabled && m_pVCEnabled->get() > 0.0) {
        return;
    }

    const auto localBpm = getLocalBpm();
    if (localBpm.isValid()) {
        m_pRateRatio->set(bpm * m_leaderBpmAdjustFactor / localBpm);
    }
}

void SyncControl::notifyLeaderParamSource() {
    m_leaderBpmAdjustFactor = kBpmUnity;
}

void SyncControl::reinitLeaderParams(
        double beatDistance, mixxx::Bpm baseBpm, mixxx::Bpm bpm) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "SyncControl::reinitLeaderParams" << getGroup()
                        << beatDistance << baseBpm << bpm;
    }
    m_leaderBpmAdjustFactor = determineBpmMultiplier(mixxx::Bpm(m_pBpm->get()), bpm);
    updateLeaderBpm(bpm);
    updateLeaderBeatDistance(beatDistance);
}

double SyncControl::determineBpmMultiplier(mixxx::Bpm myBpm, mixxx::Bpm targetBpm) const {
    if (!myBpm.isValid() || !targetBpm.isValid()) {
        return kBpmUnity;
    }
    double unityRatio = myBpm / targetBpm;
    // the square root of 2 (1.414) is the
    // rate threshold that works vice versa for this and the target.
    double unityRatioSquare = unityRatio * unityRatio;
    if (unityRatioSquare > kBpmDouble) {
        return kBpmDouble;
    } else if (unityRatioSquare < kBpmHalve) {
        return kBpmHalve;
    }
    return kBpmUnity;
}

void SyncControl::updateTargetBeatDistance() {
    updateTargetBeatDistance(frameInfo().currentPosition);
}

void SyncControl::updateTargetBeatDistance(mixxx::audio::FramePos refPosition) {
    double targetDistance = m_unmultipliedTargetBeatDistance;
    if (kLogger.traceEnabled()) {
        kLogger.trace()
                << getGroup()
                << "SyncControl::updateTargetBeatDistance, unmult distance"
                << targetDistance
                << m_leaderBpmAdjustFactor
                << refPosition;
    }

    // Determining the target distance is not as simple as x2 or /2.  Since one
    // of the beats is twice the length of the other, we need to know if the
    // position of the longer beat is past its halfway point.  e.g. 0.0 for the
    // long beat is 0.0 of the short beat, but 0.5 for the long beat is also
    // 0.0 for the short beat.
    if (m_leaderBpmAdjustFactor == kBpmDouble) {
        if (targetDistance >= 0.5) {
            targetDistance -= 0.5;
        }
        targetDistance *= kBpmDouble;
    } else if (m_leaderBpmAdjustFactor == kBpmHalve) {
        targetDistance *= kBpmHalve;
        // Our beat distance CO is still a buffer behind, so take the current value.
        if (m_pBpmControl->getBeatDistance(refPosition) >= 0.5) {
            targetDistance += 0.5;
        }
    }
    if (kLogger.traceEnabled()) {
        kLogger.trace()
                << getGroup()
                << "SyncControl::updateTargetBeatDistance, adjusted target is"
                << targetDistance;
    }
    m_pBpmControl->setTargetBeatDistance(targetDistance);
}

mixxx::Bpm SyncControl::getBpm() const {
    const auto bpm = mixxx::Bpm(m_pBpm->get());
    if (kLogger.traceEnabled()) {
        kLogger.trace() << getGroup() << "SyncControl::getBpm()"
                        << bpm << "/" << m_leaderBpmAdjustFactor;
    }
    if (!bpm.isValid()) {
        return {};
    }

    return bpm / m_leaderBpmAdjustFactor;
}

void SyncControl::updateInstantaneousBpm(mixxx::Bpm bpm) {
    // Adjust the incoming bpm by the multiplier.
    const double bpmValue = bpm.valueOr(0.0) * m_leaderBpmAdjustFactor;
    m_pBpmControl->updateInstantaneousBpm(bpmValue);
}

// called from an engine worker thread
void SyncControl::trackLoaded(TrackPointer pNewTrack) {
    // Note: The track is loaded but not yet cued.
    // in case of dynamic tempo tracks we do not know the bpm for syncing yet.

    // This slot is also fired if the user has adjusted the beatgrid.
    if (kLogger.traceEnabled()) {
        kLogger.trace() << getGroup() << "SyncControl::trackLoaded";
    }

    mixxx::BeatsPointer pBeats;
    if (pNewTrack) {
        pBeats = pNewTrack->getBeats();
    }
    m_pBeats = pBeats;
    m_leaderBpmAdjustFactor = kBpmUnity;
}

void SyncControl::trackBeatsUpdated(mixxx::BeatsPointer pBeats) {
    // This slot is fired by if the user has adjusted the beatgrid.
    if (kLogger.traceEnabled()) {
        kLogger.trace() << getGroup() << "SyncControl::trackBeatsUpdated";
    }

    m_pBeats = pBeats;
    m_leaderBpmAdjustFactor = kBpmUnity;

    SyncMode syncMode = getSyncMode();
    if (isLeader(syncMode)) {
        if (!m_pBeats) {
            // If the track was ejected or suddenly has no beats, we can no longer
            // be leader.
            m_pChannel->getEngineBuffer()->requestSyncMode(SyncMode::Follower);
        } else {
            // We should not change playback speed.
            m_pBpmControl->updateLocalBpm();
            m_pEngineSync->notifyBaseBpmChanged(this, getBaseBpm());
        }
    } else if (isFollower(syncMode)) {
        // If we were a follower, requesting sync mode refreshes
        // the soft leader -- if we went from having no bpm to having
        // a bpm, we might need to become leader.
        m_pChannel->getEngineBuffer()->requestSyncMode(syncMode);
        m_pBpmControl->updateLocalBpm();
    }
}

void SyncControl::notifySeek(mixxx::audio::FramePos position) {
    m_pEngineSync->notifySeek(this, position);
}

void SyncControl::slotControlBeatSyncPhase(double value) {
    if (value == 0) {
        return;
    }

    m_pChannel->getEngineBuffer()->requestSyncPhase();
}

void SyncControl::slotControlBeatSyncTempo(double value) {
    if (value == 0) {
        return;
    }
    // This request is a noop if we are already synced.
    const auto localBpm = getLocalBpm();
    if (isSynchronized() || !localBpm.isValid()) {
        return;
    }

    Syncable* target = m_pEngineSync->pickNonSyncSyncTarget(getChannel());
    if (target == nullptr) {
        return;
    }

    double multiplier = determineBpmMultiplier(fileBpm(), target->getBaseBpm());
    m_pRateRatio->set(target->getBpm() * multiplier / localBpm);
}

void SyncControl::slotControlBeatSync(double value) {
    if (value > 0) {
        m_pChannel->getEngineBuffer()->requestEnableSync(true);
        m_pChannel->getEngineBuffer()->requestEnableSync(false);
    }
}

void SyncControl::slotControlPlay(double play) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "SyncControl::slotControlPlay" << getGroup() << getSyncMode() << play;
    }
    m_pEngineSync->notifyPlayingAudible(this, play > 0.0 && m_audible);
}

void SyncControl::slotVinylControlChanged(double enabled) {
    if (enabled != 0 && getSyncMode() == SyncMode::Follower) {
        // If vinyl control was enabled and we're a follower, disable sync mode.
        m_pChannel->getEngineBuffer()->requestSyncMode(SyncMode::None);
    }
}

void SyncControl::slotPassthroughChanged(double enabled) {
    if (enabled != 0 && isSynchronized()) {
        // If passthrough was enabled and sync was on, disable it.
        m_pChannel->getEngineBuffer()->requestSyncMode(SyncMode::None);
    }
}

void SyncControl::slotSyncModeChangeRequest(double state) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << getGroup() << "SyncControl::slotSyncModeChangeRequest";
    }
    SyncMode mode = syncModeFromDouble(state);
    if (m_pPassthroughEnabled->toBool() && mode != SyncMode::None) {
        qDebug() << "Disallowing enabling of sync mode when passthrough active";
    } else {
        m_pChannel->getEngineBuffer()->requestSyncMode(mode);
    }
}

void SyncControl::slotSyncLeaderEnabledChangeRequest(double state) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "SyncControl::slotSyncLeaderEnabledChangeRequest" << getGroup();
    }
    SyncMode mode = getSyncMode();
    if (state > 0.0) {
        if (m_pPassthroughEnabled->toBool()) {
            qDebug() << "Disallowing enabling of sync mode when passthrough active";
            return;
        }
        // NOTE: This branch would normally activate Explicit Leader mode. Due to the large number
        // of side effects and bugs, this mode is disabled. For now, requesting explicit leader mode
        // only activates the chosen deck as the soft leader. See:
        // https://github.com/mixxxdj/mixxx/issues/11788
        m_pChannel->getEngineBuffer()->requestSyncMode(SyncMode::LeaderSoft);
    } else {
        // Turning off leader goes back to follower mode.
        switch (mode) {
        case SyncMode::LeaderExplicit:
            m_pChannel->getEngineBuffer()->requestSyncMode(SyncMode::LeaderSoft);
            break;
        case SyncMode::LeaderSoft:
            m_pChannel->getEngineBuffer()->requestSyncMode(SyncMode::Follower);
            break;
        default:
            return;
        }
    }
}

void SyncControl::slotSyncEnabledChangeRequest(double enabled) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << "SyncControl::slotSyncEnabledChangeRequest" << getGroup();
    }
    bool bEnabled = enabled > 0.0;

    // Allow a request for state change even if it's the same as the current
    // state.  We might have toggled on and off in the space of one buffer.
    if (bEnabled && m_pPassthroughEnabled->toBool()) {
        qDebug() << "Disallowing enabling of sync mode when passthrough active";
        return;
    }
    m_pChannel->getEngineBuffer()->requestEnableSync(bEnabled);
}

void SyncControl::setLocalBpm(mixxx::Bpm localBpm) {
    if (localBpm == m_prevLocalBpm.getValue()) {
        return;
    }
    m_prevLocalBpm.setValue(localBpm);

    SyncMode syncMode = getSyncMode();
    if (syncMode <= SyncMode::None) {
        return;
    }

    const mixxx::Bpm bpm = localBpm * m_pRateRatio->get();

    if (isFollower(syncMode)) {
        // In this case we need an update from the current leader to adjust
        // the rate that we continue with the leader BPM. If there is no
        // leader bpm, our bpm value is adopted and the m_leaderBpmAdjustFactor
        // is reset to 1;
        m_pEngineSync->requestBpmUpdate(this, bpm);
    } else {
        DEBUG_ASSERT(isLeader(syncMode));
        // We might have adopted an adjust factor when becoming leader.
        // Keep it when reporting our bpm.
        m_pEngineSync->notifyBaseBpmChanged(this, bpm / m_leaderBpmAdjustFactor);
    }
}

void SyncControl::updateAudible() {
    int channelIndex = m_pChannel->getChannelIndex();
    if (channelIndex >= 0) {
        CSAMPLE_GAIN gain = getEngineMixer()->getMainGain(channelIndex);
        bool newAudible = gain > CSAMPLE_GAIN_ZERO;
        if (static_cast<bool>(m_audible) != newAudible) {
            m_audible = newAudible;
            m_pEngineSync->notifyPlayingAudible(this, m_pPlayButton->toBool() && m_audible);
        }
    }
}

void SyncControl::slotRateChanged() {
    mixxx::Bpm bpm = getLocalBpm();
    if (!bpm.isValid()) {
        return;
    }

    const double rateRatio = m_pRateRatio->get();
    bpm *= rateRatio;
    if (kLogger.traceEnabled()) {
        kLogger.trace() << getGroup() << "SyncControl::slotRateChanged" << rateRatio << bpm;
    }

    // BPM may be invalid if rateRatio is NaN or infinity.
    if (!bpm.isValid() || !isSynchronized()) {
        return;
    }

    // When reporting our bpm, remove the multiplier so the leaders all
    // think the followers have the same bpm.
    m_pEngineSync->notifyRateChanged(this, bpm / m_leaderBpmAdjustFactor);
}

void SyncControl::reportPlayerSpeed(double speed, bool scratching) {
    if (m_bOldScratching ^ scratching) {
        m_pEngineSync->notifyScratching(this, scratching);
        m_bOldScratching = scratching;
        // No need to disable sync mode while scratching, the engine won't
        // get confused.
    }
    // When reporting our speed, remove the multiplier so the leaders all
    // think the followers have the same bpm.
    mixxx::Bpm localBpm = getLocalBpm();
    if (localBpm.isValid()) {
        const mixxx::Bpm instantaneousBpm = localBpm * (speed / m_leaderBpmAdjustFactor);
        m_pEngineSync->notifyInstantaneousBpmChanged(this, instantaneousBpm);
    }
}

mixxx::Bpm SyncControl::fileBpm() const {
    mixxx::BeatsPointer pBeats = m_pBeats;
    if (pBeats) {
        return pBeats->getBpmInRange(mixxx::audio::kStartFramePos, frameInfo().trackEndPosition);
    }
    return {};
}

mixxx::Bpm SyncControl::getLocalBpm() const {
    if (m_pLocalBpm) {
        return mixxx::Bpm(m_pLocalBpm->get());
    }
    return {};
}
