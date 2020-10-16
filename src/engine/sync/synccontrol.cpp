#include "engine/sync/synccontrol.h"

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "control/controlpushbutton.h"
#include "engine/channels/enginechannel.h"
#include "engine/controls/bpmcontrol.h"
#include "engine/controls/ratecontrol.h"
#include "engine/enginebuffer.h"
#include "track/track.h"
#include "util/assert.h"
#include "util/logger.h"
#include "util/math.h"

const double kTrackPositionMasterHandoff = 0.99;

const double SyncControl::kBpmUnity = 1.0;
const double SyncControl::kBpmHalve = 0.5;
const double SyncControl::kBpmDouble = 2.0;

namespace {
const mixxx::Logger kLogger("SyncControl");
} // namespace

SyncControl::SyncControl(const QString& group, UserSettingsPointer pConfig,
                         EngineChannel* pChannel, SyncableListener* pEngineSync)
        : EngineControl(group, pConfig),
          m_sGroup(group),
          m_pChannel(pChannel),
          m_pEngineSync(pEngineSync),
          m_pBpmControl(nullptr),
          m_pRateControl(nullptr),
          m_bOldScratching(false),
          m_masterBpmAdjustFactor(kBpmUnity),
          m_unmultipliedTargetBeatDistance(0.0),
          m_pBpm(nullptr),
          m_pLocalBpm(nullptr),
          m_pRateRatio(nullptr),
          m_pVCEnabled(nullptr),
          m_pSyncPhaseButton(nullptr) {
    // Play button.  We only listen to this to disable master if the deck is
    // stopped.
    m_pPlayButton = new ControlProxy(group, "play", this);
    m_pPlayButton->connectValueChanged(this, &SyncControl::slotControlPlay,
                                       Qt::DirectConnection);

    m_pSyncMode.reset(new ControlPushButton(ConfigKey(group, "sync_mode")));
    m_pSyncMode->setButtonMode(ControlPushButton::TOGGLE);
    m_pSyncMode->setStates(SYNC_NUM_MODES);
    m_pSyncMode->connectValueChangeRequest(
            this, &SyncControl::slotSyncModeChangeRequest, Qt::DirectConnection);

    m_pSyncMasterEnabled.reset(
            new ControlPushButton(ConfigKey(group, "sync_master")));
    m_pSyncMasterEnabled->setButtonMode(ControlPushButton::TOGGLE);
    m_pSyncMasterEnabled->connectValueChangeRequest(
            this, &SyncControl::slotSyncMasterEnabledChangeRequest, Qt::DirectConnection);

    m_pSyncEnabled.reset(
            new ControlPushButton(ConfigKey(group, "sync_enabled")));
    m_pSyncEnabled->setButtonMode(ControlPushButton::LONGPRESSLATCHING);
    m_pSyncEnabled->connectValueChangeRequest(
            this, &SyncControl::slotSyncEnabledChangeRequest, Qt::DirectConnection);

    // The relative position between two beats in the range 0.0 ... 1.0
    m_pBeatDistance.reset(
            new ControlObject(ConfigKey(group, "beat_distance")));

    m_pPassthroughEnabled = new ControlProxy(group, "passthrough", this);
    m_pPassthroughEnabled->connectValueChanged(this,
            &SyncControl::slotPassthroughChanged, Qt::DirectConnection);

    m_pEjectButton = new ControlProxy(group, "eject", this);
    m_pEjectButton->connectValueChanged(this,
            &SyncControl::slotEjectPushed, Qt::DirectConnection);

    m_pQuantize = new ControlProxy(group, "quantize", this);

    // Adopt an invalid to not ignore the first call setLocalBpm()
    m_prevLocalBpm.setValue(-1);

    // BPMControl and RateControl will be initialized later.
}

SyncControl::~SyncControl() {
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
    m_pRateRatio->connectValueChanged(this, &SyncControl::slotRateChanged,
                                       Qt::DirectConnection);

    m_pSyncPhaseButton = new ControlProxy(getGroup(), "beatsync_phase", this);

#ifdef __VINYLCONTROL__
    m_pVCEnabled = new ControlProxy(
            getGroup(), "vinylcontrol_enabled", this);

    // Throw a hissy fit if somebody moved us such that the vinylcontrol_enabled
    // control doesn't exist yet. This will blow up immediately, won't go unnoticed.
    DEBUG_ASSERT(m_pVCEnabled->valid());

    m_pVCEnabled->connectValueChanged(this, &SyncControl::slotVinylControlChanged,
                                      Qt::DirectConnection);
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
    m_pSyncMode->setAndConfirm(mode);
    m_pSyncEnabled->setAndConfirm(mode != SYNC_NONE);
    m_pSyncMasterEnabled->setAndConfirm(isMaster(mode));
    if (mode == SYNC_FOLLOWER) {
        if (m_pVCEnabled && m_pVCEnabled->get()) {
            // If follower mode is enabled, disable vinyl control.
            m_pVCEnabled->set(0.0);
        }
    }
    if (mode != SYNC_NONE && m_pPassthroughEnabled->get()) {
        // If any sync mode is enabled and passthrough was on somehow, disable passthrough.
        // This is very unlikely to happen so this deserves a warning.
        qWarning() << "Notified of sync mode change when passthrough was "
                      "active -- "
                      "must disable passthrough";
        m_pPassthroughEnabled->set(0.0);
    }
    if (isMaster(mode)) {
        m_pBpmControl->resetSyncAdjustment();
    } else if (mode == SYNC_NONE) {
        m_masterBpmAdjustFactor = kBpmUnity;
    }
}

void SyncControl::notifyOnlyPlayingSyncable() {
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
        // This is helpful if the beatgrid of the track doe not fit at the current
        // playposition
        m_pChannel->getEngineBuffer()->requestSyncPhase();
    }
}

bool SyncControl::isPlaying() const {
    return m_pPlayButton->toBool();
}

double SyncControl::adjustSyncBeatDistance(double beatDistance) const {
    // Similar to adjusting the target beat distance, when we report our beat
    // distance we need to adjust it by the master bpm adjustment factor.  If
    // we've been doubling the master bpm, we need to divide it in half.  If
    // we'be been halving the master bpm, we need to double it.  Both operations
    // also need to account for if the longer beat is past its halfway point.
    //
    // This is the inverse of the updateTargetBeatDistance function below.
    if (m_masterBpmAdjustFactor == kBpmDouble) {
        beatDistance /= kBpmDouble;
        if (m_unmultipliedTargetBeatDistance >= 0.5) {
            beatDistance += 0.5;
        }
    } else if (m_masterBpmAdjustFactor == kBpmHalve) {
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

double SyncControl::getBaseBpm() const {
    return m_pLocalBpm->get() / m_masterBpmAdjustFactor;
}

void SyncControl::setMasterBeatDistance(double beatDistance) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << getGroup() << "SyncControl::setMasterBeatDistance"
                        << beatDistance;
    }
    // Set the BpmControl target beat distance to beatDistance, adjusted by
    // the multiplier if in effect.  This way all of the multiplier logic
    // is contained in this single class.
    m_unmultipliedTargetBeatDistance = beatDistance;
    // Update the target beat distance based on the multiplier.
    updateTargetBeatDistance();
}

void SyncControl::setMasterBpm(double bpm) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << getGroup() << "SyncControl::setMasterBpm" << bpm;
    }

    VERIFY_OR_DEBUG_ASSERT(isSynchronized()) {
        qWarning() << "WARNING: Logic Error: setBpm called on SYNC_NONE syncable.";
        return;
    }

    // Vinyl Control overrides.
    if (m_pVCEnabled && m_pVCEnabled->get() > 0.0) {
        return;
    }

    double localBpm = m_pLocalBpm->get();
    if (localBpm > 0.0) {
        m_pRateRatio->set(bpm * m_masterBpmAdjustFactor / localBpm);
    }
}

void SyncControl::setMasterParams(
        double beatDistance, double baseBpm, double bpm) {
    double masterBpmAdjustFactor = determineBpmMultiplier(fileBpm(), baseBpm);
    if (isMaster(getSyncMode())) {
        // In Master mode we adjust the incoming Bpm for the initial sync.
        bpm *= masterBpmAdjustFactor;
        m_masterBpmAdjustFactor = kBpmUnity;
    } else {
        // in Follower mode we keep the factor when reporting our BPM
        m_masterBpmAdjustFactor = masterBpmAdjustFactor;
    }
    setMasterBpm(bpm);
    setMasterBeatDistance(beatDistance);
}

double SyncControl::determineBpmMultiplier(double myBpm, double targetBpm) const {
    double multiplier = kBpmUnity;
    if (myBpm == 0.0 || targetBpm == 0.0) {
        return multiplier;
    }
    double best_margin = fabs((targetBpm / myBpm) - 1.0);

    double try_margin = fabs((targetBpm * kBpmHalve / myBpm) - 1.0);
    // We really want to prefer unity, so use a float compare with high tolerance.
    if (best_margin - try_margin > .0001) {
        multiplier = kBpmHalve;
        best_margin = try_margin;
    }

    try_margin = fabs((targetBpm * kBpmDouble / myBpm) - 1.0);
    if (best_margin - try_margin > .0001) {
        multiplier = kBpmDouble;
    }
    return multiplier;
}

void SyncControl::updateTargetBeatDistance() {
    double targetDistance = m_unmultipliedTargetBeatDistance;
    if (kLogger.traceEnabled()) {
        kLogger.trace() << getGroup() << "SyncControl::updateTargetBeatDistance, unmult distance" << targetDistance;
    }

    // Determining the target distance is not as simple as x2 or /2.  Since one
    // of the beats is twice the length of the other, we need to know if the
    // position of the longer beat is past its halfway point.  e.g. 0.0 for the
    // long beat is 0.0 of the short beat, but 0.5 for the long beat is also
    // 0.0 for the short beat.
    if (m_masterBpmAdjustFactor == kBpmDouble) {
        if (targetDistance >= 0.5) {
            targetDistance -= 0.5;
        }
        targetDistance *= kBpmDouble;
    } else if (m_masterBpmAdjustFactor == kBpmHalve) {
        targetDistance *= kBpmHalve;
        if (m_pBeatDistance->get() >= 0.5) {
            targetDistance += 0.5;
        }
    }
    if (kLogger.traceEnabled()) {
        kLogger.trace() << getGroup() << "SyncControl::updateTargetBeatDistance, adjusted target is" << targetDistance;
    }
    m_pBpmControl->setTargetBeatDistance(targetDistance);
}

double SyncControl::getBpm() const {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << getGroup() << "SyncControl::getBpm()" << m_pBpm->get();
    }
    return m_pBpm->get() / m_masterBpmAdjustFactor;
}

void SyncControl::setInstantaneousBpm(double bpm) {
    // Adjust the incoming bpm by the multiplier.
    m_pBpmControl->setInstantaneousBpm(bpm * m_masterBpmAdjustFactor);
}

void SyncControl::reportTrackPosition(double fractionalPlaypos) {
    // If we're close to the end, and master, disable master so we don't stop
    // the party.
    if (isMaster(getSyncMode()) &&
            fractionalPlaypos > kTrackPositionMasterHandoff) {
        m_pChannel->getEngineBuffer()->requestSyncMode(SYNC_FOLLOWER);
    }
}

// called from an engine worker thread
void SyncControl::trackLoaded(TrackPointer pNewTrack) {
    mixxx::BeatsPointer pBeats;
    if (pNewTrack) {
        pBeats = pNewTrack->getBeats();
    }
    trackBeatsUpdated(pBeats);
}

void SyncControl::trackBeatsUpdated(mixxx::BeatsPointer pBeats) {
    // This slot is fired by a new file is loaded or if the user
    // has adjusted the beatgrid.
    if (kLogger.traceEnabled()) {
        kLogger.trace() << getGroup() << "SyncControl::trackBeatsUpdated";
    }

    VERIFY_OR_DEBUG_ASSERT(m_pLocalBpm) {
        // object not initialized
        return;
    }

    m_pBeats = pBeats;
    m_masterBpmAdjustFactor = kBpmUnity;

    SyncMode syncMode = getSyncMode();
    if (syncMode == SYNC_MASTER_SOFT || syncMode == SYNC_FOLLOWER) {
        // If we change or remove beats while soft master, hand off.
        // If we were a follower, requesting sync mode refreshes
        // the soft master.
        m_pChannel->getEngineBuffer()->requestSyncMode(SYNC_FOLLOWER);
    }
    setLocalBpm(m_pLocalBpm->get());
}

void SyncControl::slotControlPlay(double play) {
    m_pEngineSync->notifyPlaying(this, play > 0.0);
}

void SyncControl::slotVinylControlChanged(double enabled) {
    if (enabled && getSyncMode() == SYNC_FOLLOWER) {
        // If vinyl control was enabled and we're a follower, disable sync mode.
        m_pChannel->getEngineBuffer()->requestSyncMode(SYNC_NONE);
    }
}

void SyncControl::slotPassthroughChanged(double enabled) {
    if (enabled && isSynchronized()) {
        // If passthrough was enabled and sync was on, disable it.
        m_pChannel->getEngineBuffer()->requestSyncMode(SYNC_NONE);
    }
}

void SyncControl::slotEjectPushed(double enabled) {
    Q_UNUSED(enabled);
    // We can't eject tracks if the decks is playing back, so if we are master
    // and eject was pushed the deck must be stopped.  Handing off in this case
    // actually causes the other decks to start playing, so not doing anything
    // is preferred.
}

void SyncControl::slotSyncModeChangeRequest(double state) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << getGroup() << "SyncControl::slotSyncModeChangeRequest";
    }
    SyncMode mode = syncModeFromDouble(state);
    if (m_pPassthroughEnabled->get() && mode != SYNC_NONE) {
        qDebug() << "Disallowing enabling of sync mode when passthrough active";
    } else {
        m_pChannel->getEngineBuffer()->requestSyncMode(mode);
    }
}

void SyncControl::slotSyncMasterEnabledChangeRequest(double state) {
    if (state > 0.0) {
        if (isMaster(getSyncMode())) {
            // Already master.
            return;
        }
        if (m_pPassthroughEnabled->get()) {
            qDebug() << "Disallowing enabling of sync mode when passthrough active";
            return;
        }
    }
    // TODO(owilliams): Because SYNC_MASTER_EXPLICIT has issues, for now we
    // actually just enable follower mode even when they ask for master. This
    // is equivalent to the behavior in 2.2
    m_pChannel->getEngineBuffer()->requestSyncMode(SYNC_FOLLOWER);
}

void SyncControl::slotSyncEnabledChangeRequest(double enabled) {
    bool bEnabled = enabled > 0.0;

    // Allow a request for state change even if it's the same as the current
    // state.  We might have toggled on and off in the space of one buffer.
    if (bEnabled && m_pPassthroughEnabled->get()) {
        qDebug() << "Disallowing enabling of sync mode when passthrough active";
        return;
    }
    m_pChannel->getEngineBuffer()->requestEnableSync(bEnabled);
}

void SyncControl::setLocalBpm(double local_bpm) {
    if (local_bpm == m_prevLocalBpm.getValue()) {
        return;
    }
    m_prevLocalBpm.setValue(local_bpm);

    SyncMode syncMode = getSyncMode();
    if (syncMode <= SYNC_NONE) {
        return;
    }

    double bpm = local_bpm * m_pRateRatio->get();

    if (syncMode == SYNC_FOLLOWER) {
        // In this case we need an update from the current master to adjust
        // the rate that we continue with the master BPM. If there is no
        // master bpm, our bpm value is adopted and the m_masterBpmAdjustFactor
        // is reset to 1;
        m_pEngineSync->requestBpmUpdate(this, bpm);
    } else {
        DEBUG_ASSERT(isMaster(syncMode));
        // We might have adopted an adjust factor when becoming master.
        // Keep it when reporting our bpm.
        m_pEngineSync->notifyBpmChanged(this, bpm / m_masterBpmAdjustFactor);
    }
}

void SyncControl::slotRateChanged() {
    double bpm = m_pLocalBpm ? m_pLocalBpm->get() * m_pRateRatio->get() : 0.0;
    if (kLogger.traceEnabled()) {
        kLogger.trace() << getGroup() << "SyncControl::slotRateChanged" << m_pRateRatio->get() << bpm;
    }
    if (bpm > 0 && isSynchronized()) {
        // When reporting our bpm, remove the multiplier so the masters all
        // think the followers have the same bpm.
        m_pEngineSync->notifyBpmChanged(this, bpm / m_masterBpmAdjustFactor);
    }
}

void SyncControl::reportPlayerSpeed(double speed, bool scratching) {
    if (m_bOldScratching ^ scratching) {
        m_pEngineSync->notifyScratching(this, scratching);
        m_bOldScratching = scratching;
        // No need to disable sync mode while scratching, the engine won't
        // get confused.
    }
    // When reporting our speed, remove the multiplier so the masters all
    // think the followers have the same bpm.
    double instantaneous_bpm = m_pLocalBpm->get() * speed / m_masterBpmAdjustFactor;
    m_pEngineSync->notifyInstantaneousBpmChanged(this, instantaneous_bpm);
}

void SyncControl::notifySeek(double dNewPlaypos) {
    // qDebug() << "SyncControl::notifySeek" << dNewPlaypos;
    EngineControl::notifySeek(dNewPlaypos);
    m_pBpmControl->notifySeek(dNewPlaypos);
    updateTargetBeatDistance();
}

double SyncControl::fileBpm() const {
    mixxx::BeatsPointer pBeats = m_pBeats;
    if (pBeats) {
        return pBeats->getBpm();
    }
    return 0.0;
}
