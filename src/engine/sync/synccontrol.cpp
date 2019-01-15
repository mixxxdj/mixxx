#include "engine/sync/synccontrol.h"

#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "control/controlproxy.h"
#include "engine/controls/bpmcontrol.h"
#include "engine/controls/ratecontrol.h"
#include "engine/enginebuffer.h"
#include "engine/channels/enginechannel.h"
#include "util/math.h"
#include "util/assert.h"

const double kTrackPositionMasterHandoff = 0.99;

const double SyncControl::kBpmUnity = 1.0;
const double SyncControl::kBpmHalve = 0.5;
const double SyncControl::kBpmDouble = 2.0;

SyncControl::SyncControl(const QString& group, UserSettingsPointer pConfig,
                         EngineChannel* pChannel, SyncableListener* pEngineSync)
        : EngineControl(group, pConfig),
          m_sGroup(group),
          m_pChannel(pChannel),
          m_pEngineSync(pEngineSync),
          m_pBpmControl(NULL),
          m_pRateControl(NULL),
          m_bOldScratching(false),
          m_masterBpmAdjustFactor(kBpmUnity),
          m_unmultipliedTargetBeatDistance(0.0),
          m_beatDistance(0.0),
          m_pBpm(NULL),
          m_pLocalBpm(NULL),
          m_pFileBpm(NULL),
          m_pRateSlider(NULL),
          m_pRateDirection(NULL),
          m_pRateRange(NULL),
          m_pVCEnabled(NULL),
          m_pSyncPhaseButton(NULL) {
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

    m_pSyncBeatDistance.reset(
            new ControlObject(ConfigKey(group, "beat_distance")));

    m_pPassthroughEnabled = new ControlProxy(group, "passthrough", this);
    m_pPassthroughEnabled->connectValueChanged(this,
            &SyncControl::slotPassthroughChanged, Qt::DirectConnection);

    m_pEjectButton = new ControlProxy(group, "eject", this);
    m_pEjectButton->connectValueChanged(this,
            &SyncControl::slotEjectPushed, Qt::DirectConnection);

    m_pQuantize = new ControlProxy(group, "quantize", this);

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

    m_pFileBpm = new ControlProxy(getGroup(), "file_bpm", this);
    m_pFileBpm->connectValueChanged(this, &SyncControl::slotFileBpmChanged,
                                    Qt::DirectConnection);

    m_pRateSlider = new ControlProxy(getGroup(), "rate", this);
    m_pRateSlider->connectValueChanged(this, &SyncControl::slotRateChanged,
                                       Qt::DirectConnection);

    m_pRateDirection = new ControlProxy(getGroup(), "rate_dir", this);
    m_pRateDirection->connectValueChanged(this, &SyncControl::slotRateChanged,
                                          Qt::DirectConnection);

    m_pRateRange = new ControlProxy(getGroup(), "rateRange", this);
    m_pRateRange->connectValueChanged(this, &SyncControl::slotRateChanged,
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

void SyncControl::notifySyncModeChanged(SyncMode mode) {
    //qDebug() << "SyncControl::notifySyncModeChanged" << getGroup() << mode;
    // SyncControl has absolutely no say in the matter. This is what EngineSync
    // requires. Bypass confirmation by using setAndConfirm.
    m_masterBpmAdjustFactor = kBpmUnity;
    m_pSyncMode->setAndConfirm(mode);
    m_pSyncEnabled->setAndConfirm(mode != SYNC_NONE);
    m_pSyncMasterEnabled->setAndConfirm(mode == SYNC_MASTER);
    if (mode == SYNC_FOLLOWER) {
        if (m_pVCEnabled && m_pVCEnabled->get()) {
            // If follower mode is enabled, disable vinyl control.
            m_pVCEnabled->set(0.0);
        }
    }
    if (mode != SYNC_NONE && m_pPassthroughEnabled->get()) {
        // If any sync mode is enabled and passthrough was on somehow, disable passthrough.
        // This is very unlikely to happen so this deserves a warning.
        qWarning() << "Notified of sync mode change when passthrough was active -- "
                      "must disable passthrough";
        m_pPassthroughEnabled->set(0.0);
    }
    if (mode == SYNC_MASTER) {
        // Make sure all the followers update based on our current rate.
        slotRateChanged();
        double rateRatio = calcRateRatio();
        m_pEngineSync->notifyBeatDistanceChanged(this, getBeatDistance());
        m_pBpm->set(m_pLocalBpm->get() * rateRatio);
    }
}

void SyncControl::notifyOnlyPlayingSyncable() {
    // If we are the only remaining playing sync deck, we can reset the user
    // tweak info.
    m_pBpmControl->resetSyncAdjustment();
}

void SyncControl::requestSync() {
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

double SyncControl::getBeatDistance() const {
    double beatDistance = m_pSyncBeatDistance->get();
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

double SyncControl::getBaseBpm() const {
    return m_pLocalBpm->get();
}

void SyncControl::setBeatDistance(double beatDistance) {
    m_beatDistance = beatDistance;
    // The target distance may change based on our beat distance.
    updateTargetBeatDistance();
}

void SyncControl::setMasterBeatDistance(double beatDistance) {
    // Set the BpmControl target beat distance to beatDistance, adjusted by
    // the multiplier if in effect.  This way all of the multiplier logic
    // is contained in this single class.
    m_unmultipliedTargetBeatDistance = beatDistance;
    // Update the target beat distance based on the multiplier.
    updateTargetBeatDistance();
}

void SyncControl::setMasterBaseBpm(double bpm) {
    m_masterBpmAdjustFactor = determineBpmMultiplier(m_pFileBpm->get(), bpm);
    // Update the target beat distance in case the multiplier changed.
    updateTargetBeatDistance();
}

void SyncControl::setMasterBpm(double bpm) {
    //qDebug() << "SyncControl::setMasterBpm" << getGroup() << bpm;

    if (!isSynchronized()) {
        qDebug() << "WARNING: Logic Error: setBpm called on SYNC_NONE syncable.";
        return;
    }

    // Vinyl Control overrides.
    if (m_pVCEnabled && m_pVCEnabled->get() > 0.0) {
        return;
    }

    double localBpm = m_pLocalBpm->get();
    double rateRange = m_pRateRange->get();
    if (localBpm > 0.0 && rateRange > 0.0) {
        double newRate = m_pRateDirection->get() *
                ((bpm * m_masterBpmAdjustFactor / localBpm) - 1.0) /
                rateRange;
        m_pRateSlider->set(newRate);
    }
}

void SyncControl::setMasterParams(double beatDistance, double baseBpm, double bpm) {
    m_unmultipliedTargetBeatDistance = beatDistance;
    m_masterBpmAdjustFactor = determineBpmMultiplier(m_pFileBpm->get(), baseBpm);
    setMasterBpm(bpm);
    updateTargetBeatDistance();
}

double SyncControl::determineBpmMultiplier(double myBpm, double targetBpm) const {
    double multiplier = kBpmUnity;
    if (myBpm == 0.0) {
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
        if (m_beatDistance >= 0.5) {
            targetDistance += 0.5;
        }
    }
    m_pBpmControl->setTargetBeatDistance(targetDistance);
}

double SyncControl::getBpm() const {
    //qDebug() << getGroup() << "SyncControl::getBpm()" << m_pBpm->get();
    return m_pBpm->get();
}

void SyncControl::setInstantaneousBpm(double bpm) {
    // Adjust the incoming bpm by the multiplier.
    m_pBpmControl->setInstantaneousBpm(bpm * m_masterBpmAdjustFactor);
}

void SyncControl::reportTrackPosition(double fractionalPlaypos) {
    // If we're close to the end, and master, disable master so we don't stop
    // the party.
    if (getSyncMode() == SYNC_MASTER &&
            fractionalPlaypos > kTrackPositionMasterHandoff) {
        m_pChannel->getEngineBuffer()->requestSyncMode(SYNC_NONE);
    }
}

void SyncControl::trackLoaded(TrackPointer pNewTrack) {
    //qDebug() << getGroup() << "SyncControl::trackLoaded";
    if (getSyncMode() == SYNC_MASTER) {
        // If we change or remove a new track while master, hand off.
        m_pChannel->getEngineBuffer()->requestSyncMode(SYNC_NONE);
    }
    if (pNewTrack) {
        m_masterBpmAdjustFactor = kBpmUnity;
        if (isSynchronized()) {
            // Because of the order signals get processed, the file/local_bpm COs and
            // rate slider are not updated as soon as we need them, so do that now.
            m_pFileBpm->set(pNewTrack->getBpm());
            m_pLocalBpm->set(pNewTrack->getBpm());
            double dRate = calcRateRatio();
            // We used to set the m_pBpm here, but that causes a signal loop whereby
            // that was interpretted as a rate slider tweak, and the master bpm
            // was changed.  Instead, now we pass the suggested bpm to enginesync
            // explicitly, and it can decide what to do with it.
            m_pEngineSync->notifyTrackLoaded(this, m_pLocalBpm->get() * dRate);
        }
    }
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
    SyncMode mode(syncModeFromDouble(state));
    if (m_pPassthroughEnabled->get() && mode != SYNC_NONE) {
        qDebug() << "Disallowing enabling of sync mode when passthrough active";
    } else {
        m_pChannel->getEngineBuffer()->requestSyncMode(mode);
    }
}

void SyncControl::slotSyncMasterEnabledChangeRequest(double state) {
    bool currentlyMaster = getSyncMode() == SYNC_MASTER;

    if (state > 0.0) {
        if (currentlyMaster) {
            // Already master.
            return;
        }
        if (m_pPassthroughEnabled->get()) {
            qDebug() << "Disallowing enabling of sync mode when passthrough active";
            return;
        }
        m_pChannel->getEngineBuffer()->requestSyncMode(SYNC_MASTER);
    } else {
        // Turning off master goes back to follower mode.
        if (!currentlyMaster) {
            // Already not master.
            return;
        }
        m_pChannel->getEngineBuffer()->requestSyncMode(SYNC_FOLLOWER);
    }
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

    // FIXME: This recalculating of the rate is duplicated in bpmcontrol.
    const double rateRatio = calcRateRatio();
    double bpm = local_bpm * rateRatio;
    m_pBpm->set(bpm);

    if (syncMode == SYNC_FOLLOWER) {
        // In this case we need an update from the current master to adjust
        // the rate that we continue with the master BPM. If there is no
        // master bpm, our bpm value is adopted.
        m_pEngineSync->requestBpmUpdate(this, bpm);
    } else {
        DEBUG_ASSERT(syncMode == SYNC_MASTER);
        m_pEngineSync->notifyBpmChanged(this, bpm);
    }
}

void SyncControl::slotFileBpmChanged() {
    // This slot is fired by a new file is loaded or if the user
    // has adjusted the beatgrid.
    //qDebug() << "SyncControl::slotFileBpmChanged";
    
    // Note: bpmcontrol has updated local_bpm just before
    double local_bpm = m_pLocalBpm ? m_pLocalBpm->get() : 0.0;

    if (!isSynchronized()) {
        const double rateRatio = calcRateRatio();
        double bpm = local_bpm * rateRatio;
        m_pBpm->set(bpm);
    } else {
        setLocalBpm(local_bpm);
    }
}

void SyncControl::slotRateChanged() {
    // This slot is fired by rate, rate_dir, and rateRange changes.
    const double rateRatio = calcRateRatio();
    double bpm = m_pLocalBpm ? m_pLocalBpm->get() * rateRatio : 0.0;
    //qDebug() << getGroup() << "SyncControl::slotRateChanged" << rate << bpm;
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

double SyncControl::calcRateRatio() {
    double rateRatio = 1.0 + m_pRateDirection->get() * m_pRateRange->get() *
            m_pRateSlider->get();
    return rateRatio;
}
