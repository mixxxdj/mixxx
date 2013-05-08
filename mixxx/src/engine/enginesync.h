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

#include "controlpotmeter.h"
#include "engine/enginecontrol.h"
#include "engine/enginebuffer.h"
#include "engine/enginechannel.h"
#include "engine/enginemaster.h"

enum SYNC_STATE {
    SYNC_NONE = 0,
    SYNC_SLAVE = 1,
    SYNC_MASTER = 2
};  

enum SYNC_SOURCE {
    SYNC_INTERNAL = 0,
    SYNC_DECK = 1,
    SYNC_MIDI = 2    
};

class EngineSync : public EngineControl { 
    Q_OBJECT
    
    public:
        EngineSync(EngineMaster *master, ConfigObject<ConfigValue>* pConfig);
        ~EngineSync();
        void addDeck(QString group);
        bool setMaster(QString group);
        bool setDeckMaster(QString deck);
        bool setInternalMaster(void);
        bool setMidiMaster(void);
        bool setNone(QString deck);
        EngineBuffer* getMaster();
        
        void incrementPseudoPosition(int bufferSize);
        double getInternalBeatDistance(void);
        
    private slots:
        void slotMasterBpmChanged(double);
        void slotSourceRateChanged(double);
        void slotSyncRateSliderChanged(double);
        void slotSourceBeatDistanceChanged(double);
        void slotSampleRateChanged(double);
        void slotInternalMasterChanged(double);
        void slotDeckMasterChanged(double);
        void slotDeckSlaveChanged(double);
        
    protected:
        QString chooseNewMaster(QString dontpick);
        void disconnectMaster(void);
        void disableDeckMaster(QString deck);
        void updateSamplesPerBeat(void);
        void setPseudoPosition(double percent);
        void resetInternalBeatDistance(void);

        EngineMaster* m_pEngineMaster;    
        EngineBuffer* m_pMasterBuffer;
        ControlObject *m_pSourceRate, *m_pMasterBpm;
        ControlObject *m_pSourceBeatDistance, *m_pMasterBeatDistance;
        ControlObject *m_pSampleRate;
        ControlPushButton *m_pSyncInternalEnabled;
        ControlPotmeter* m_pSyncRateSlider;
        
        QList<QString> m_sDeckList;
        
        int m_iSyncSource;
        int m_iSampleRate;
        double m_dSourceRate, m_dMasterBpm;
        double m_dSamplesPerBeat;
        double m_dPseudoBufferPos;
        
};

#endif
