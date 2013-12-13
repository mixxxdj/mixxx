#include "engine/sync/synccontrol.h"

#include "controlobject.h"
#include "controlpushbutton.h"
#include "controlobjectslave.h"
#include "engine/ratecontrol.h"
#include "engine/bpmcontrol.h"

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

    m_pSyncMode.reset(new ControlObject(ConfigKey(pGroup, "sync_mode")));
    m_pSyncMode->connectValueChangeRequest(
        this, SLOT(slotSyncModeChangeRequest(double)),
        Qt::DirectConnection);

    m_pSyncMasterEnabled.reset(
        new ControlPushButton(ConfigKey(pGroup, "sync_master")));
    m_pSyncMasterEnabled->setButtonMode(ControlPushButton::TOGGLE);
    m_pSyncMasterEnabled->connectValueChangeRequest(
        this, SLOT(slotSyncMasterEnabledChangeRequest(double)),
        Qt::DirectConnection);

    m_pSyncEnabled.reset(
        new ControlPushButton(ConfigKey(pGroup, "sync_enabled")));
    m_pSyncEnabled->setButtonMode(ControlPushButton::LONGPRESSLATCHING);
    m_pSyncEnabled->connectValueChangeRequest(
        this, SLOT(slotSyncEnabledChangeRequest(double)),
        Qt::DirectConnection);

    m_pSyncBeatDistance.reset(
        new ControlObject(ConfigKey(pGroup, "beat_distance")));
    connect(m_pSyncBeatDistance.data(), SIGNAL(valueChanged(double)),
            this, SLOT(slotBeatDistanceChanged(double)),
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

#ifdef __VINYLCONTROL__
    m_pVCEnabled.reset(new ControlObjectSlave(
        getGroup(), "vinylcontrol_enabled", this));
    // Throw a hissy fit if somebody moved us such that the vinylcontrol_enabled
    // control doesn't exist yet. This will blow up immediately, won't go unnoticed.
    Q_ASSERT(m_pVCEnabled->valid());
#endif
}

void SyncControl::notifySyncModeChanged(SyncMode mode) {
    qDebug() << "SyncControl::notifySyncModeChanged" << getGroup() << mode;
    // SyncControl has absolutely no say in the matter. This is what EngineSync
    // requires. Bypass confirmation by using setAndConfirm.
    m_pSyncMode->setAndConfirm(mode);
    m_pSyncEnabled->setAndConfirm(mode != SYNC_NONE);
    m_pSyncMasterEnabled->setAndConfirm(mode == SYNC_MASTER);
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
    if (m_pVCEnabled->get() > 0.0) {
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

void SyncControl::slotControlPlay(double play) {
    m_pEngineSync->notifyPlaying(this, play > 0.0);
}

void SyncControl::slotSyncModeChangeRequest(double state) {
    m_pEngineSync->requestSyncMode(this, syncModeFromDouble(state));
}

void SyncControl::slotSyncMasterEnabledChangeRequest(double state) {
    bool currentlyMaster = getSyncMode() == SYNC_MASTER;

    if (state > 0.0) {
        if (currentlyMaster) {
            // Already master.
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

    // If we are not already in the enabled state requested, request a
    // transition.
    if (bEnabled ^ syncEnabled) {
        m_pEngineSync->requestEnableSync(this, bEnabled);
    }
}

SyncMode SyncControl::getSyncMode() const {
    return syncModeFromDouble(m_pSyncMode->get());
}

void SyncControl::slotFileBpmChanged() {
    // This slot is fired by file_bpm changes.
    const double rate = 1.0 + m_pRateSlider->get() * m_pRateRange->get() * m_pRateDirection->get();
    double bpm = m_pFileBpm ? m_pFileBpm->get() * rate : 0.0;
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
