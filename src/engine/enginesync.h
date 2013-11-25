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

enum SYNC_STATE {
    SYNC_NONE = 0,
    SYNC_SLAVE = 1,
    SYNC_MASTER = 2
};

class EngineSync : public EngineControl {
    Q_OBJECT

  public:
    explicit EngineSync(ConfigObject<ConfigValue>* pConfig);
    virtual ~EngineSync();

    void addChannel(EngineChannel* pChannel);
    RateControl* addDeck(const char* group);
    EngineChannel* getMaster() const;
    void onCallbackStart(int bufferSize);
    RateControl* getRateControlForGroup(const QString& group);
    const QString getSyncSource() const { return m_sSyncSource; }

  private slots:
    void slotMasterBpmChanged(double);
    void slotSyncRateSliderChanged(double);
    void slotSourceRateChanged(double);
    void slotChannelRateSliderChanged(RateControl*, double);
    void slotSourceBeatDistanceChanged(double);
    void slotSampleRateChanged(double);
    void slotInternalMasterChanged(double);
    void slotChannelSyncStateChanged(RateControl*, double);

  private:
    void setMaster(const QString& group);
    bool setChannelMaster(RateControl* pRateControl);
    void setInternalMaster();
    QString chooseNewMaster(const QString& dontpick);
    void disableChannelMaster();
    void updateSamplesPerBeat();
    void setPseudoPosition(double percent);
    void resetInternalBeatDistance();
    double getInternalBeatDistance() const;

    ConfigObject<ConfigValue>* m_pConfig;

    RateControl* m_pChannelMaster;

    ControlObject* m_pMasterBpm;
    ControlObject* m_pMasterBeatDistance;
    ControlObject* m_pSampleRate;
    ControlPushButton* m_pSyncInternalEnabled;
    ControlPotmeter* m_pSyncRateSlider;

    QList<RateControl*> m_ratecontrols;
    QString m_sSyncSource;
    int m_iSampleRate;
    double m_dSourceRate;
    double m_dMasterBpm;
    double m_dSamplesPerBeat;
    double m_dPseudoBufferPos;
};

#endif
