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


EngineSync::EngineSync(const char* _group,
                       ConfigObject<ConfigValue>* _config) :
        EngineControl(_group, _config)
{
    qDebug() << "group should be master" << _group;
    m_pMasterRate = new ControlObject(ConfigKey(_group, "rate"));
    m_pMasterScratch = new ControlObject(ConfigKey(_group, "scratch"));
    m_pMasterScratchEnabled = new ControlObject(ConfigKey(_group, "scratch_enable"));
}

bool EngineSync::setMaster(QString deck)
{
    if (deck == NULL || deck == "")
    {
        qDebug() << "unsetting master (got null)";
        m_pSourceRate->disconnect();
        m_pMasterBuffer = NULL;
        emit(setSyncMaster(""));
    }
    
    EngineChannel* pChannel = pMaster->getChannel(deck);
    // Only consider channels that have a track loaded and are in the master
    // mix.
    if (pChannel && pChannel->isActive() && pChannel->isMaster()) {
        m_pMasterBuffer = pChannel->getEngineBuffer();
            
        m_pSourceRate->disconnect();
        delete m_pSourceRate;
        m_pSourceRate = ControlObject::getControl(ConfigKey(deck, "rate"));
        connect(m_pSourceRate, SIGNAL(valueChanged(double)),
                this, SLOT(slotRateChanged(double)),
                Qt::DirectConnection);

        m_pSourceScratch->disconnect();
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
                
        emit(setSyncMaster(deck));
        
        return true;
    }
    
    return false;
}

EngineBuffer* EngineSync::chooseMasterBuffer(void)
{
    //TODO (XXX): n-deck
    decks << "[Channel1]"
          << "[Channel2]"
          << "[Channel3]"
          << "[Channel4]";
    QString fallback = "";
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
    return fallback;
}

void EngineSync::slotRateChanged(double slider_pos)
{
    double bpm = m_pMasterBuffer->getBpm();
    m_pMasterRate->set(bpm); //this will trigger all of the slaves to change rate
}

void EngineSync::slotScratchChanged(double scratch)
{
    double scratchbpm = m_pMasterBuffer->getExactBpm();
    m_pMasterScratch->set(scratchbpm);
}

void EngineSync::slotScratchEnabledChanged(double enabled)
{
    m_pMasterScratchEnabled->set(bool(enabled));
}

EngineBuffer* EngineSync::getMaster()
{
    return m_pMasterBuffer;
}
