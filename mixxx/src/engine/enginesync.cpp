/***************************************************************************
                          enginesync.cpp  -  master sync control for 
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

#include <QStringList>

#include "engine/enginesync.h"

EngineSync::EngineSync(EngineMaster *master,
                       const char* _group,
                       ConfigObject<ConfigValue>* _config) :
        EngineControl(_group, _config)
{
    qDebug() << "group should be master" << _group;
    m_pEngineMaster = master;
    m_pSourceRate = NULL;
    m_pSourceBeatDistance = NULL;
    m_dOldMasterRate = 0.0f;
    m_pMasterBpm = new ControlObject(ConfigKey(_group, "sync_bpm"));
    m_pMasterBeatDistance = new ControlObject(ConfigKey(_group, "beat_distance"));
    //m_pMasterRate = new ControlObject(ConfigKey(_group, "rate"));
    //m_pMasterRateEnabled = new ControlObject(ConfigKey(_group, "scratch_enable"));
}

bool EngineSync::setMaster(QString deck)
{
    if (deck == NULL || deck == "")
    {
        qDebug() << "----------------------------------------------------unsetting master (got null)";
        if (m_pSourceRate != NULL)
        {
            //m_pSourceRate->disconnect(); //QT experts -- is this necessary?
            delete m_pSourceRate;
        }
        if (m_pSourceBeatDistance != NULL)
        {
            //m_pSourceBeatDistance->disconnect();
            delete m_pSourceBeatDistance;
        }
        m_pMasterBuffer = NULL;
        emit(setSyncMaster(""));
        return true;
    }
    
    EngineChannel* pChannel = m_pEngineMaster->getChannel(deck);
    // Only consider channels that have a track loaded and are in the master
    // mix.

    qDebug() << "**************************************************************************asked to set a new master:" << deck;
    
    if (pChannel) {
        m_pMasterBuffer = pChannel->getEngineBuffer();
            
        if (m_pSourceRate != NULL) {
            //m_pSourceRate->disconnect();
            delete m_pSourceRate;
        }
        m_pSourceRate = ControlObject::getControl(ConfigKey(deck, "true_rate"));
        if (m_pSourceRate == NULL)
        {
            qDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!! source true rate was null";
            return false;
        }
        connect(m_pSourceRate, SIGNAL(valueChangedFromEngine(double)),
                this, SLOT(slotSourceRateChanged(double)),
                Qt::DirectConnection);

        if (m_pSourceBeatDistance != NULL)
        {
            //m_pSourceBeatDistance->disconnect();
            delete m_pSourceBeatDistance;
        }
        m_pSourceBeatDistance = ControlObject::getControl(ConfigKey(deck, "beat_distance"));
        if (m_pSourceBeatDistance == NULL)
        {
            qDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1 source beat dist was null";
            return false;
        }
        connect(m_pSourceBeatDistance, SIGNAL(valueChangedFromEngine(double)),
                this, SLOT(slotSourceBeatDistanceChanged(double)),
                Qt::DirectConnection);
        
        /*m_pSourceScratch->disconnect();
        delete m_pSourceScratch;
        m_pSourceScratch = ControlObject::getControl(ConfigKey(deck, "scratch2"));
        connect(m_pSourceScratch, SIGNAL(valueChanged(double)),
                this, SLOT(slotScratchChanged(double)),
                Qt::DirectConnection);

        m_pSourceScratchEnabled->disconnect();
        delete m_pSourceScratchEnabled;
        m_pSourceScratchEnabled = ControlObject::getControl(ConfigKey(deck, "scratch2_enable"));
        connect(m_pSourceScratchEnabled, SIGNAL(valueChanged(double)),
                this, SLOT(slotScratchEnabledChanged(double)),
                Qt::DirectConnection);
          */      
        qDebug() << "----------------------------setting new master" << deck;
        emit(setSyncMaster(deck));
        
        return true;
    }
    else
    {
        qDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!did not set master";
        if (pChannel == NULL)
            qDebug() << "well, it was null!";
        else
            qDebug() << pChannel << pChannel->isActive() << pChannel->isMaster();
    }
    
    return false;
}

EngineBuffer* EngineSync::chooseMasterBuffer(void)
{
    //TODO (XXX): n-deck
    QStringList decks;
    decks << "[Channel1]"
          << "[Channel2]"
          << "[Channel3]"
          << "[Channel4]";
    /*QString fallback = "";
    foreach (QString deck, decks)
    {
        EngineChannel* pChannel = pMaster->getChannel(deck);
        if (pChannel && pChannel->isActive() && pChannel->isMaster()) {
            EngineBuffer* pBuffer = pChannel->getEngineBuffer();
            if (pBuffer && pBuffer->getBpm() > 0) {
                // If the deck is playing then go with it immediately.
                if (fabs(pBuffer->getRate()) > 0) {
                    qDebug() << "picked a new master deck:" << fallback;
                    return deck;
                }
                // Otherwise hold out for a deck that might be playing.
                if (fallback == NULL)
                    fallback = deck;
            }
        }
    }    
    
    qDebug() << "picked a new master deck (fallback):" << fallback;
    return fallback;*/
    
    
    return m_pEngineMaster->getChannel("[Channel1]")->getEngineBuffer();
}

void EngineSync::slotSourceRateChanged(double true_rate)
{
    if (true_rate != m_dOldMasterRate)
    {
        m_dOldMasterRate = true_rate;
        
        double filebpm = m_pMasterBuffer->getFileBpm();
        double bpm = true_rate * filebpm;
        m_pMasterBpm->set(bpm); //this will trigger all of the slaves to change rate
    }
}

void EngineSync::slotSourceBeatDistanceChanged(double beat_dist)
{
    //this is absolute samples, so just pass it on
    m_pMasterBeatDistance->set(beat_dist);
}

/*void EngineSync::slotScratchChanged(double scratch)
{
    double scratchbpm = m_pMasterBuffer->getExactBpm();
    m_pMasterRate->set(scratchbpm);
}

void EngineSync::slotScratchEnabledChanged(double enabled)
{
    m_pMasterRateEnabled->set(bool(enabled));
}*/

EngineBuffer* EngineSync::getMaster()
{
    return m_pMasterBuffer;
}
