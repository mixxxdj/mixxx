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
class EngineMaster;
class ControlObject;
class ControlPushButton;
class ControlPotmeter;

enum SYNC_STATE {
    SYNC_NONE = 0,
    SYNC_SLAVE = 1,
    SYNC_MASTER = 2
};

class SyncChannel : public QObject {
    Q_OBJECT
  public:
    SyncChannel(EngineChannel* pChannel);
    virtual ~SyncChannel();

    const QString& getGroup() const {
        return m_group;
    }

    EngineChannel* getChannel();
    void setState(double state);
    double getState() const;

  signals:
    void channelSyncStateChanged(SyncChannel*, double);

  private slots:
    void slotChannelSyncStateChanged(double);

  private:
    EngineChannel* m_pChannel;
    QString m_group;
    ControlObject* m_pChannelSyncState;
};

class EngineSync : public EngineControl {
    Q_OBJECT

  public:
    EngineSync(EngineMaster *master, ConfigObject<ConfigValue>* pConfig);
    virtual ~EngineSync();

    void addChannel(EngineChannel* pChannel);
    EngineChannel* getMaster() const;
    void onCallbackStart(int bufferSize);

  private slots:
    void slotMasterBpmChanged(double);
    void slotSourceRateChanged(double);
    void slotSyncRateSliderChanged(double);
    void slotSourceBeatDistanceChanged(double);
    void slotSampleRateChanged(double);
    void slotInternalMasterChanged(double);
    void slotChannelSyncStateChanged(SyncChannel*, double);

  private:
    void setMaster(const QString& group);
    bool setChannelMaster(SyncChannel* pSyncChannel);
    void setInternalMaster();
    QString chooseNewMaster(const QString& dontpick);
    void disconnectMaster();
    void disableChannelMaster(const QString& deck);
    void updateSamplesPerBeat();
    void setPseudoPosition(double percent);
    void resetInternalBeatDistance();
    double getInternalBeatDistance() const;
    SyncChannel* getSyncChannelForGroup(const QString& group);

    EngineMaster* m_pEngineMaster;
    EngineChannel* m_pMasterChannel;
    ControlObject* m_pSourceRate;
    ControlObject* m_pMasterBpm;
    ControlObject* m_pSourceBeatDistance;
    ControlObject* m_pMasterBeatDistance;
    ControlObject* m_pSampleRate;
    ControlPushButton* m_pSyncInternalEnabled;
    ControlPotmeter* m_pSyncRateSlider;

    QList<SyncChannel*> m_channels;
    QString m_sSyncSource;
    int m_iSampleRate;
    double m_dSourceRate;
    double m_dMasterBpm;
    double m_dSamplesPerBeat;
    double m_dPseudoBufferPos;
};

#endif
