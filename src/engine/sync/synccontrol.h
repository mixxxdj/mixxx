#ifndef SYNCCONTROL_H
#define SYNCCONTROL_H

#include <QScopedPointer>
#include <gtest/gtest_prod.h>

#include "engine/controls/enginecontrol.h"
#include "engine/sync/syncable.h"

class EngineChannel;
class BpmControl;
class RateControl;
class ControlObject;
class ControlProxy;
class ControlPushButton;

class SyncControl : public EngineControl, public Syncable {
    Q_OBJECT
  public:
    static const double kBpmUnity;
    static const double kBpmHalve;
    static const double kBpmDouble;
    SyncControl(const QString& group, UserSettingsPointer pConfig,
                EngineChannel* pChannel, SyncableListener* pEngineSync);
    ~SyncControl() override;

    const QString& getGroup() const { return m_sGroup; }
    EngineChannel* getChannel() const { return m_pChannel; }
    double getBpm() const;

    SyncMode getSyncMode() const;
    void notifySyncModeChanged(SyncMode mode);
    void notifyOnlyPlayingSyncable();
    void requestSync();
    bool isPlaying() const;

    double getBeatDistance() const;
    void setBeatDistance(double beatDistance);
    double getBaseBpm() const;
    void setLocalBpm(double local_bpm);

    // Must never result in a call to
    // SyncableListener::notifyBeatDistanceChanged or signal loops could occur.
    void setMasterBeatDistance(double beatDistance);
    void setMasterBaseBpm(double);
    // Must never result in a call to
    // SyncableListener::notifyBpmChanged or signal loops could occur.
    void setMasterBpm(double bpm);
    void setMasterParams(double beatDistance, double baseBpm, double bpm);

    // Must never result in a call to
    // SyncableListener::notifyInstantaneousBpmChanged or signal loops could
    // occur.
    void setInstantaneousBpm(double bpm);

    void setEngineControls(RateControl* pRateControl, BpmControl* pBpmControl);

    void reportTrackPosition(double fractionalPlaypos);
    void reportPlayerSpeed(double speed, bool scratching);
    void trackLoaded(TrackPointer pNewTrack) override;

  private slots:
    // Fired by changes in play.
    void slotControlPlay(double v);

    // Fired by changes in vinyl control status.
    void slotVinylControlChanged(double v);

    // Fired when passthrough mode is enabled or disabled.
    void slotPassthroughChanged(double v);

    // Fired when a track is ejected.
    void slotEjectPushed(double v);

    // Fired by changes in rate, rate_dir, rateRange.
    void slotRateChanged();

    // Fired by changes in file_bpm.
    void slotFileBpmChanged();

    // Change request handlers for sync properties.
    void slotSyncModeChangeRequest(double state);
    void slotSyncEnabledChangeRequest(double enabled);
    void slotSyncMasterEnabledChangeRequest(double state);

  private:
    FRIEND_TEST(SyncControlTest, TestDetermineBpmMultiplier);
    // Sometimes it's best to match bpms based on half or double the target
    // bpm.  e.g. 70 matches better with 140/2.  This function returns the
    // best factor for multiplying the master bpm to get a bpm this syncable
    // should match against.
    double determineBpmMultiplier(double myBpm, double targetBpm) const;
    void updateTargetBeatDistance();
    double calcRateRatio();

    QString m_sGroup;
    // The only reason we have this pointer is an optimzation so that the
    // EngineSync can ask us what our EngineChannel is. EngineMaster in turn
    // asks EngineSync what EngineChannel is the "master" channel.
    EngineChannel* m_pChannel;
    SyncableListener* m_pEngineSync;
    BpmControl* m_pBpmControl;
    RateControl* m_pRateControl;
    bool m_bOldScratching;

    // When syncing, sometimes it's better to match half or double the
    // master bpm.
    FRIEND_TEST(EngineSyncTest, HalfDoubleBpmTest);
    // The amount we should multiply the master BPM to find a good sync match.
    // Sometimes this is 2 or 0.5.
    double m_masterBpmAdjustFactor;
    // It is handy to store the raw reported target beat distance in case the
    // multiplier changes and we need to recalculate the target distance.
    double m_unmultipliedTargetBeatDistance;
    double m_beatDistance;
    ControlValueAtomic<double> m_prevLocalBpm;

    QScopedPointer<ControlPushButton> m_pSyncMode;
    QScopedPointer<ControlPushButton> m_pSyncMasterEnabled;
    QScopedPointer<ControlPushButton> m_pSyncEnabled;
    QScopedPointer<ControlObject> m_pSyncBeatDistance;

    // These ControlProxys are created as parent to this and deleted by
    // the Qt object tree. This helps that they are deleted by the creating
    // thread, which is required to avoid segfaults.
    ControlProxy* m_pPlayButton;
    ControlProxy* m_pBpm;
    ControlProxy* m_pLocalBpm;
    ControlProxy* m_pFileBpm;
    ControlProxy* m_pRateSlider;
    ControlProxy* m_pRateDirection;
    ControlProxy* m_pRateRange;
    ControlProxy* m_pVCEnabled;
    ControlProxy* m_pPassthroughEnabled;
    ControlProxy* m_pEjectButton;
    ControlProxy* m_pSyncPhaseButton;
    ControlProxy* m_pQuantize;
};


#endif /* SYNCCONTROL_H */
