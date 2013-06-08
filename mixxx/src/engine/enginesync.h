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

class EngineBuffer;
class EngineMaster;
class ControlObject;
class ControlPushButton;
class ControlPotmeter;

enum SYNC_STATE {
    SYNC_NONE = 0,
    SYNC_SLAVE = 1,
    SYNC_MASTER = 2
};

class EngineSync : public EngineControl {
    Q_OBJECT

  public:
    EngineSync(EngineMaster *master, ConfigObject<ConfigValue>* pConfig);
    virtual ~EngineSync();
    void addDeck(QString group);
    void setMaster(QString group);
    bool setDeckMaster(QString deck);
    void setInternalMaster(void);
    bool setMidiMaster(void);
    EngineBuffer* getMaster() const;

    void incrementPseudoPosition(int bufferSize);
    double getInternalBeatDistance(void) const;

  private slots:
    void slotMasterBpmChanged(double);
    void slotSourceRateChanged(double);
    void slotSyncRateSliderChanged(double);
    void slotSourceBeatDistanceChanged(double);
    void slotSampleRateChanged(double);
    void slotInternalMasterChanged(double);
    void slotDeck1StateChanged(double);
    void slotDeck2StateChanged(double);
    void slotDeck3StateChanged(double);
    void slotDeck4StateChanged(double);

  protected:
    QString chooseNewMaster(QString dontpick);
    void disconnectMaster(void);
    void disableDeckMaster(QString deck);
    void updateSamplesPerBeat(void);
    void setPseudoPosition(double percent);
    void resetInternalBeatDistance(void);
    void deckXStateChanged(QString group, double);

    EngineMaster* m_pEngineMaster;
    EngineBuffer* m_pMasterBuffer;
    ControlObject* m_pSourceRate;
    ControlObject* m_pMasterBpm;
    ControlObject* m_pSourceBeatDistance;
    ControlObject* m_pMasterBeatDistance;
    ControlObject* m_pSampleRate;
    ControlPushButton* m_pSyncInternalEnabled;
    ControlPotmeter* m_pSyncRateSlider;

    QList<QString> m_sDeckList;

    QString m_sSyncSource;
    int m_iSampleRate;
    double m_dSourceRate, m_dMasterBpm;
    double m_dSamplesPerBeat;
    double m_dPseudoBufferPos;
};

#endif
