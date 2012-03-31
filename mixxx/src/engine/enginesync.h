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
#include "engine/enginebuffer.h"
#include "engine/enginechannel.h"
#include "engine/enginemaster.h"

enum SYNC_STATE {
        SYNC_NONE = 0,
        SYNC_SLAVE = 1,
        SYNC_MASTER = 2
};  

class EngineSync : public EngineControl { 
    Q_OBJECT
    
    public:
        EngineSync(EngineMaster *master, const char* pGroup, ConfigObject<ConfigValue>* pConfig);
        bool setMaster(QString deck);
        bool setNone(QString deck);
        EngineBuffer* getMaster();
        
    signals:
        void setSyncMaster(QString deck);
        
    private slots:
        void slotSourceRateChanged(double);
        void slotSourceBeatDistanceChanged(double);
        //void slotScratchChanged(double);
        //void slotScratchEnabledChanged(double);
        
    protected:
        EngineBuffer* chooseMasterBuffer(void);

        EngineMaster* m_pEngineMaster;    
        EngineBuffer* m_pMasterBuffer;
        ControlObject *m_pSourceRate, *m_pMasterBpm;
        ControlObject *m_pSourceBeatDistance, *m_pMasterBeatDistance;
        /*ControlObject* m_pSourceRate, m_pSourceScratch, m_pSourceScratchEnabled;
        ControlObject* m_pMasterRate, m_pMasterScratch, m_pMasterScratchEnabled;*/
        
        double m_dOldMasterRate;
};

#endif
