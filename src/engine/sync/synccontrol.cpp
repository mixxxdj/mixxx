#include "engine/sync/synccontrol.h"

#include "controlobject.h"
#include "controlpushbutton.h"
#include "controlobjectslave.h"
#include "engine/bpmcontrol.h"
#include "engine/enginebuffer.h"
#include "engine/enginechannel.h"
#include "engine/ratecontrol.h"

const double kTrackPositionMasterHandoff = 0.99;

SyncControl::SyncControl(const char* pGroup, ConfigObject<ConfigValue>* pConfig,
                         EngineChannel* pChannel, SyncableListener* pEngineSync)
        : EngineControl(pGroup, pConfig),
          m_sGroup(pGroup),
          m_pChannel(pChannel),
          m_pEngineSync(pEngineSync),
          m_pBpmControl(NULL),
          m_pRateControl(NULL),
          m_bOldScratching(false) {
    // Play button.  We only listen to this to disable master if the deck is
    // stopped.
    m_pPlayButton.reset(new ControlObjectSlave(pGroup, "play", this));
    m_pPlayButton->connectValueChanged(this, SLOT(slotControlPlay(double)),
                                       Qt::DirectConnection);

    m_pSyncMode.reset(new ControlPushButton(ConfigKey(pGroup, "sync_mode")));
    m_pSyncMode->setButtonMode(ControlPushButton::TOGGLE);
    m_pSyncMode->setStates(SYNC_NUM_MODES);
    m_pSyncMode->connectValueChangeRequest(
            this, SLOT(slotSyncModeChangeRequest(double)), Qt::DirectConnection);

    m_pSyncMasterEnabled.reset(
            new ControlPushButton(ConfigKey(pGroup, "sync_master")));
    m_pSyncMasterEnabled->setButtonMode(ControlPushButton::TOGGLE);
    m_pSyncMasterEnabled->connectValueChangeRequest(
            this, SLOT(slotSyncMasterEnabledChangeRequest(double)), Qt::DirectConnection);

    m_pSyncEnabled.reset(
            new ControlPushButton(ConfigKey(pGroup, "sync_enabled")));
    m_pSyncEnabled->setButtonMode(ControlPushButton::LONGPRESSLATCHING);
    m_pSyncEnabled->connectValueChangeRequest(
            this, SLOT(slotSyncEnabledChangeRequest(double)), Qt::DirectConnection);

    m_pSyncBeatDistance.reset(
            new ControlObject(ConfigKey(pGroup, "beat_distance")));
    connect(m_pSyncBeatDistance.data(), SIGNAL(valueChanged(double)),
            this, SLOT(slotBeatDistanceChanged(double)), Qt::DirectConnection);

    m_pPassthroughEnabled.reset(new ControlObjectSlave(pGroup, "passthrough", this));
    m_pPassthroughEnabled->connectValueChanged(this, SLOT(slotPassthroughChanged(double)),
                                               Qt::DirectConnection);

    m_pEjectButton.reset(new ControlObjectSlave(pGroup, "eject", this));
    m_pEjectButton->connectValueChanged(this, SLOT(slotEjectPushed(double)),
                                        Qt::DirectConnection);

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
    m_pBpm.reset(new ControlObjectSlave(getGroup(), "bpm", this));

    m_pFileBpm.reset(new ControlObjectSlave(getGroup(), "file_bpm", this));
    m_pFileBpm->connectValueChanged(this, SLOT(slotFileBpmChanged()),
                                    Qt::DirectConnection);

    m_pRateSlider.reset(new ControlObjectSlave(getGroup(), "rate", this));
    m_pRateSlider->connectValueChanged(this, SLOT(slotRateChanged()),
                                       Qt::DirectConnection);

    m_pRateDirection.reset(new ControlObjectSlave(getGroup(), "rate_dir", this));
    m_pRateDirection->connectValueChanged(this, SLOT(slotRateChanged()),
                                          Qt::DirectConnection);

    m_pRateRange.reset(new ControlObjectSlave(getGroup(), "rateRange", this));
    m_pRateRange->connectValueChanged(this, SLOT(slotRateChanged()),
                                      Qt::DirectConnection);

    m_pSyncPhaseButton.reset(new ControlObjectSlave(getGroup(), "beatsync_phase", this));

#ifdef __VINYLCONTROL__
    m_pVCEnabled.reset(new ControlObjectSlave(
        getGroup(), "vinylcontrol_enabled", this));

    // Throw a hissy fit if somebody moved us such that the vinylcontrol_enabled
    // control doesn't exist yet. This will blow up immediately, won't go unnoticed.
    Q_ASSERT(m_pVCEnabled->valid());

    m_pVCEnabled->connectValueChanged(this, SLOT(slotVinylControlChanged(double)),
                                      Qt::DirectConnection);
#endif
}

void SyncControl::notifySyncModeChanged(SyncMode mode) {
    qDebug() << "SyncControl::notifySyncModeChanged" << getGroup() << mode;
    // SyncControl has absolutely no say in the matter. This is what EngineSync
    // requires. Bypass confirmation by using setAndConfirm.
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
}

void SyncControl::requestSyncPhase() {
    m_pChannel->getEngineBuffer()->requestSyncPhase();
}

double SyncControl::getBeatDistance() const {
    return m_pSyncBeatDistance->get();
}

void SyncControl::setBeatDistance(double beatDistance) {
    //qDebug() << "SyncControl::setBeatDistance" << getGroup() << beatDistance;
    // Set the BpmControl target beat distance to beatDistance.
    m_pBpmControl->setTargetBeatDistance(beatDistance);
}

double SyncControl::getBpm() const {
    return m_pBpm->get();
}

void SyncControl::setBpm(double bpm) {
    qDebug() << "SyncControl::setBpm" << getGroup() << bpm;

    if (getSyncMode() == SYNC_NONE) {
        qDebug() << "WARNING: Logic Error: setBpm called on SYNC_NONE syncable.";
        return;
    }

    // Vinyl Control overrides.
    if (m_pVCEnabled && m_pVCEnabled->get() > 0.0) {
        return;
    }

    double fileBpm = m_pFileBpm->get();
    if (fileBpm > 0.0) {
        double newRate = (bpm / m_pFileBpm->get() - 1.0)
                / m_pRateDirection->get() / m_pRateRange->get();
        m_pRateSlider->set(newRate);
    } else {
        m_pRateSlider->set(0);
    }
}

void SyncControl::setInstantaneousBpm(double bpm) {
    m_pBpmControl->setInstantaneousBpm(bpm);
}

void SyncControl::reportTrackPosition(double fractionalPlaypos) {
    // If we're close to the end, and master, disable master so we don't stop
    // the party.
    if (getSyncMode() == SYNC_MASTER &&
            fractionalPlaypos > kTrackPositionMasterHandoff) {
        m_pEngineSync->requestSyncMode(this, SYNC_NONE);
    }
}

bool SyncControl::isPlaying() const {
    return m_pPlayButton->get() > 0.0;
}

void SyncControl::trackLoaded(TrackPointer pTrack) {
    Q_UNUSED(pTrack);
    if (getSyncMode() == SYNC_MASTER) {
        // If we loaded a new track while master, hand off.
        m_pEngineSync->requestSyncMode(this, SYNC_NONE);
    }
}

void SyncControl::trackUnloaded(TrackPointer pTrack) {
    Q_UNUSED(pTrack);
    if (getSyncMode() == SYNC_MASTER) {
        // If we unloaded a new track while master, hand off.
        m_pEngineSync->requestSyncMode(this, SYNC_NONE);
    }
}

void SyncControl::slotControlPlay(double play) {
    m_pEngineSync->notifyPlaying(this, play > 0.0);
}

void SyncControl::slotVinylControlChanged(double enabled) {
    if (enabled && getSyncMode() == SYNC_FOLLOWER) {
        // If vinyl control was enabled and we're a follower, disable sync mode.
        m_pEngineSync->requestSyncMode(this, SYNC_NONE);
    }
}

void SyncControl::slotPassthroughChanged(double enabled) {
    if (enabled && getSyncMode() != SYNC_NONE) {
        // If passthrough was enabled and sync was on, disable it.
        m_pEngineSync->requestSyncMode(this, SYNC_NONE);
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
        m_pEngineSync->requestSyncMode(this, mode);
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
        m_pEngineSync->requestSyncMode(this, SYNC_MASTER);
    } else {
        // Turning off master goes back to follower mode.
        if (!currentlyMaster) {
            // Already not master.
            return;
        }
        m_pEngineSync->requestSyncMode(this, SYNC_FOLLOWER);
    }
}

void SyncControl::slotSyncEnabledChangeRequest(double enabled) {
    bool bEnabled = enabled > 0.0;
    bool syncEnabled = getSyncMode() != SYNC_NONE;
    // syncEnabled == true -> We are follower or master

    // If we are not already in the enabled state requested, request a
    // transition.
    if (bEnabled != syncEnabled) {
        if (bEnabled && m_pPassthroughEnabled->get()) {
            qDebug() << "Disallowing enabling of sync mode when passthrough active";
            return;
        }
        m_pEngineSync->requestEnableSync(this, bEnabled);
    }
}

SyncMode SyncControl::getSyncMode() const {
    return syncModeFromDouble(m_pSyncMode->get());
}

void SyncControl::slotFileBpmChanged() {
    // This slot is fired by file_bpm changes.
    double file_bpm = m_pFileBpm ? m_pFileBpm->get() : 0.0;

    if (file_bpm == 0 && getSyncMode() != SYNC_NONE && m_pPlayButton->get()) {
        // If the file bpm is suddenly zero and sync was active and we are playing,
        // we have to disable sync. The track will keep playing but will not
        // respond to rate changes.
        // I think this can only happen if the beatgrid is reset.
        qWarning() << getGroup() << " Sync is enabled on track with empty or zero bpm, "
                                    "disabling master sync.";
        m_pEngineSync->requestSyncMode(this, SYNC_NONE);
        return;
    }

    const double rate = 1.0 + m_pRateSlider->get() * m_pRateRange->get() * m_pRateDirection->get();
    double bpm = file_bpm * rate;
    m_pEngineSync->notifyBpmChanged(this, bpm, true);
}

void SyncControl::slotRateChanged() {
    // This slot is fired by rate, rate_dir, and rateRange changes.
    const double rate = 1.0 + m_pRateSlider->get() * m_pRateRange->get() * m_pRateDirection->get();
    double bpm = m_pFileBpm ? m_pFileBpm->get() * rate : 0.0;
    m_pEngineSync->notifyBpmChanged(this, bpm, false);
}

void SyncControl::slotBeatDistanceChanged(double beatDistance) {
    // TODO(rryan): This update should not be received over a CO -- BpmControl
    // should call directly.
    m_pEngineSync->notifyBeatDistanceChanged(this, beatDistance);
}

void SyncControl::reportPlayerSpeed(double speed, bool scratching) {
    if (m_bOldScratching ^ scratching) {
        m_pEngineSync->notifyScratching(this, scratching);
        m_bOldScratching = scratching;
    }
    double instantaneous_bpm = m_pFileBpm->get() * speed;
    m_pEngineSync->notifyInstantaneousBpmChanged(this, instantaneous_bpm);
}
