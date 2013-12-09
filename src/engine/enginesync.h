/***************************************************************************
                          enginesync.h  -  master sync control for
                          maintaining beatmatching amongst n decks
                             -------------------
    begin                : Mon Mar 12 2012
    copyright            : (C) 2012 by Owen Williams
    email                : owilliams@mixxx.org
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef ENGINESYNC_H
#define ENGINESYNC_H

#include "engine/enginecontrol.h"

class EngineChannel;
class ControlObject;
class ControlPushButton;
class ControlPotmeter;
class RateControl;

enum SYNC_MODE {
    SYNC_NONE = 0,
    SYNC_FOLLOWER = 1,
    SYNC_MASTER = 2
};

class EngineSync : public EngineControl {
    Q_OBJECT

  public:
    explicit EngineSync(ConfigObject<ConfigValue>* pConfig);
    virtual ~EngineSync();

    void addChannel(EngineChannel* pChannel);
    void addDeck(RateControl* pRate);
    EngineChannel* getMaster() const;
    void process(int bufferSize);
    RateControl* getRateControlForGroup(const QString& group);
    const QString getSyncSource() const { return m_sSyncSource; }
    // Used by RateControl to tell EngineSync it wants to be enabled in a specific mode.
    // EngineSync can override this selection.
    void requestSyncMode(RateControl* pRateControl, int state);
    // Used by RateControl to tell EngineSync it wants to be enabled in any mode (master/follower).
    void notifySyncModeEnabled(RateControl* pRateControl);
    // RateControl notifies EngineSync directly about slider updates instead of using a CO.
    void notifyRateSliderChanged(RateControl* pRateControl, double new_bpm);
    // RateControl notifies EngineSync about play status changes.
    void notifyDeckPlaying(RateControl* pRateControl, bool playing);

  private slots:
    void slotMasterBpmChanged(double);
    void slotSyncRateSliderChanged(double);
    void slotSourceRateEngineChanged(double);
    void slotSourceBpmChanged(double);
    void slotSourceBeatDistanceChanged(double);
    void slotSampleRateChanged(double);
    void slotClockModeChanged(double);

  private:
    // Choices about master selection often hinge on how many decks are playing back.
    int playingSyncDeckCount() const;
    // Activate a specific channel as Master.
    bool activateChannelMaster(RateControl* pRateControl);
    // Activate the internal clock as master.
    void activateClockMaster();
    void findNewMaster(const QString& dontpick);
    void disableCurrentMaster();
    void updateSamplesPerBeat();
    void setClockPosition(double percent);
    // Align the clock's beat distance with the given ratecontrol.
    void initializeClockBeatDistance(RateControl* pRateControl);
    // Align the clock's beat distance with the current master, if any.
    void initializeClockBeatDistance();
    double getClockBeatDistance() const;

    ConfigObject<ConfigValue>* m_pConfig;

    RateControl* m_pChannelMaster;

    ControlObject* m_pMasterBpm;
    ControlObject* m_pMasterBeatDistance;
    ControlObject* m_pSampleRate;
    ControlPushButton* m_pClockMasterEnabled;
    ControlPotmeter* m_pMasterRateSlider;

    QList<RateControl*> m_ratecontrols;
    QString m_sSyncSource;
    bool m_bExplicitMasterSelected;
    double m_dSamplesPerBeat;

    // Used for maintaining internal clock master sync.
    double m_dClockPosition;
};

#endif
