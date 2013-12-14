#ifndef SYNCCONTROL_H
#define SYNCCONTROL_H

#include <QScopedPointer>

#include "engine/enginecontrol.h"
#include "engine/sync/syncable.h"

class EngineChannel;
class BpmControl;
class RateControl;
class ControlObject;
class ControlObjectSlave;
class ControlPushButton;

class SyncControl : public EngineControl, public Syncable {
    Q_OBJECT
  public:
    SyncControl(const char* pGroup, ConfigObject<ConfigValue>* pConfig,
                EngineChannel* pChannel, SyncableListener* pEngineSync);
    virtual ~SyncControl();

    const QString& getGroup() const { return m_sGroup; }
    EngineChannel* getChannel() const { return m_pChannel; }

    SyncMode getSyncMode() const;
    void notifySyncModeChanged(SyncMode mode);
    bool isPlaying() const;

    double getBeatDistance() const;
    // Must never result in a call to
    // SyncableListener::notifyBeatDistanceChanged or signal loops could occur.
    void setBeatDistance(double beatDistance);

    double getBpm() const;
    // Must never result in a call to
    // SyncableListener::notifyBpmChanged or signal loops could occur.
    void setBpm(double bpm);

    // Must never result in a call to
    // SyncableListener::notifyInstantaneousBpmChanged or signal loops could
    // occur.
    void setInstantaneousBpm(double bpm);

    void setEngineControls(RateControl* pRateControl, BpmControl* pBpmControl);

    void reportTrackPosition(double fractionalPlaypos);
    void reportPlayerSpeed(double speed, bool scratching);

  private slots:
    // Fired by changes in play.
    void slotControlPlay(double v);

    // Fired by changes in rate, rate_dir, rateRange.
    void slotRateChanged();

    // Fired by changes in file_bpm.
    void slotFileBpmChanged();

    // Fired by changed to beat_distance (typically only from BpmControl during
    // BpmControl::process()).
    void slotBeatDistanceChanged(double beatDistance);

    // Change request handlers for sync properties.
    void slotSyncModeChangeRequest(double state);
    void slotSyncEnabledChangeRequest(double enabled);
    void slotSyncMasterEnabledChangeRequest(double state);

  private:
    QString m_sGroup;
    // The only reason we have this pointer is an optimzation so that the
    // EngineSync can ask us what our EngineChannel is. EngineMaster in turn
    // asks EngineSync what EngineChannel is the "master" channel.
    EngineChannel* m_pChannel;
    SyncableListener* m_pEngineSync;
    BpmControl* m_pBpmControl;
    RateControl* m_pRateControl;
    bool m_bOldScratching;

    QScopedPointer<ControlObject> m_pSyncMode;
    QScopedPointer<ControlPushButton> m_pSyncMasterEnabled;
    QScopedPointer<ControlPushButton> m_pSyncEnabled;
    QScopedPointer<ControlObject> m_pSyncBeatDistance;

    QScopedPointer<ControlObjectSlave> m_pPlayButton;
    QScopedPointer<ControlObjectSlave> m_pBpm;
    QScopedPointer<ControlObjectSlave> m_pFileBpm;
    QScopedPointer<ControlObjectSlave> m_pRateSlider;
    QScopedPointer<ControlObjectSlave> m_pRateDirection;
    QScopedPointer<ControlObjectSlave> m_pRateRange;
    QScopedPointer<ControlObjectSlave> m_pVCEnabled;
};


#endif /* SYNCCONTROL_H */
