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

#include "controlpushbutton.h"
#include "engine/enginesync.h"

EngineSync::EngineSync(EngineMaster *master,
                       ConfigObject<ConfigValue>* _config) :
        EngineControl("[Master]", _config),
        m_pEngineMaster(master),
        m_pSourceRate(NULL),
        m_pSourceBeatDistance(NULL),
        m_iSyncSource(SYNC_INTERNAL),
        m_dPseudoBufferPos(0.0f),
        m_dSourceRate(1.0f),
        m_dMasterBpm(124.0f)
{
    m_pMasterBeatDistance = new ControlObject(ConfigKey("[Master]", "beat_distance"));
    
    m_pSampleRate = ControlObject::getControl(ConfigKey("[Master]","samplerate"));
    connect(m_pSampleRate, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotSampleRateChanged(double)),
            Qt::DirectConnection);
    connect(m_pSampleRate, SIGNAL(valueChanged(double)),
            this, SLOT(slotSampleRateChanged(double)),
            Qt::DirectConnection);
            
    m_iSampleRate = m_pSampleRate->get();
    if (m_iSampleRate == 0)
    {
        m_iSampleRate = 44100;
    }
    
    m_pMasterBpm = new ControlObject(ConfigKey("[Master]", "sync_bpm"));
    connect(m_pMasterBpm, SIGNAL(valueChanged(double)),
            this, SLOT(slotMasterBpmChanged(double)),
            Qt::DirectConnection);
    connect(m_pMasterBpm, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotMasterBpmChanged(double)),
            Qt::DirectConnection);
            
    m_pSyncInternalEnabled = new ControlPushButton(ConfigKey("[Master]", "sync_master"));
    m_pSyncInternalEnabled->setButtonMode(ControlPushButton::TOGGLE);
    connect(m_pSyncInternalEnabled, SIGNAL(valueChanged(double)),
            this, SLOT(slotSyncMasterChanged(double)),
            Qt::DirectConnection);
            
    //TODO: get this from configuration
    m_pMasterBpm->set(m_dMasterBpm); //this will initialize all our values
    updateSamplesPerBeat();
}

EngineSync::~EngineSync()
{
    delete m_pMasterBpm;
    delete m_pMasterBeatDistance;
}

void EngineSync::disconnectMaster()
{
    if (m_pSourceRate != NULL)
    {
        m_pSourceRate->disconnect();
        m_pSourceRate = NULL;
    }
    if (m_pSourceBeatDistance != NULL)
    {
        m_pSourceBeatDistance->disconnect();
        m_pSourceBeatDistance = NULL;
    }
    qDebug() << "UNSETTING master buffer (disconnected master)";
    m_pMasterBuffer = NULL;
}

bool EngineSync::setInternalMaster(void)
{
    disconnectMaster();
    m_dMasterBpm = m_pMasterBpm->get();
    updateSamplesPerBeat();
    
    qDebug() << "*****************WHEEEEEEEEEEEEEEE INTERNAL";
    //this is all we have to do, we'll start using the pseudoposition right away
    m_iSyncSource = SYNC_INTERNAL;
    m_pSyncInternalEnabled->set(TRUE);
    return true;
}

bool EngineSync::setDeckMaster(QString deck)
{
    if (deck == NULL || deck == "")
    {
        qDebug() << "----------------------------------------------------unsetting master (got null)";
        disconnectMaster();
        setInternalMaster();
        return true;
    }
    
    EngineChannel* pChannel = m_pEngineMaster->getChannel(deck);
    // Only consider channels that have a track loaded and are in the master
    // mix.

    qDebug() << "**************************************************************************asked to set a new master:" << deck;
    
    if (pChannel) {
        disconnectMaster();
        m_pMasterBuffer = pChannel->getEngineBuffer();
        if (m_pMasterBuffer == NULL)
            qDebug() << "master buffer is null????";    
            
        m_pSourceRate = ControlObject::getControl(ConfigKey(deck, "true_rate"));
        if (m_pSourceRate == NULL)
        {
            qDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!! source true rate was null";
            return false;
        }
        connect(m_pSourceRate, SIGNAL(valueChangedFromEngine(double)),
                this, SLOT(slotSourceRateChanged(double)),
                Qt::DirectConnection);

        m_pSourceBeatDistance = ControlObject::getControl(ConfigKey(deck, "beat_distance"));
        if (m_pSourceBeatDistance == NULL)
        {
            qDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1 source beat dist was null";
            return false;
        }
        connect(m_pSourceBeatDistance, SIGNAL(valueChangedFromEngine(double)),
                this, SLOT(slotSourceBeatDistanceChanged(double)),
                Qt::DirectConnection);
        
        resetInternalBeatDistance(); //reset internal beat distance to equal the new master
        qDebug() << "----------------------------setting new master" << deck;
        m_iSyncSource = SYNC_DECK;
        m_pSyncInternalEnabled->set(FALSE);
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

bool EngineSync::setMidiMaster()
{
    //stub
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
    qDebug() << "got a true rate update";
    //master buffer can be null due to timing issues
    if (m_pMasterBuffer == NULL)
        qDebug() << "but master buffer is null";
    if (true_rate != m_dSourceRate && m_pMasterBuffer != NULL)
    {
        m_dSourceRate = true_rate;
        
        double filebpm = m_pMasterBuffer->getFileBpm();
        m_dMasterBpm = true_rate * filebpm;
        
        m_pMasterBpm->set(m_dMasterBpm); //this will trigger all of the slaves to change rate
    }
}

void EngineSync::slotSourceBeatDistanceChanged(double beat_dist)
{
    //just pass it on
    m_pMasterBeatDistance->set(beat_dist);
}

void EngineSync::slotMasterBpmChanged(double new_bpm)
{
    qDebug() << "~~~~~~~~~~~~~~~~~~~~~~new master bpm" << new_bpm;
    if (new_bpm != m_dMasterBpm)
    {
        if (m_iSyncSource != SYNC_INTERNAL)
        {
            qDebug() << "can't set master sync when sync isn't internal";
            return;
        }
        qDebug() << "using it";
        m_dMasterBpm = new_bpm;
        updateSamplesPerBeat();
        
        //this change could hypothetically push us over distance 1.0, so check
        Q_ASSERT(m_dSamplesPerBeat > 0);
        while (m_dPseudoBufferPos >= m_dSamplesPerBeat)
        {
            m_dPseudoBufferPos -= m_dSamplesPerBeat;
        }
    }
}

void EngineSync::slotSampleRateChanged(double srate)
{
    int new_rate = static_cast<int>(srate);
    double internal_position = getInternalBeatDistance();
    if (new_rate != m_iSampleRate)
    {
        qDebug() << "new samplerate!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << srate;
        m_iSampleRate = new_rate;
        //recalculate pseudo buffer position based on new sample rate
        m_dPseudoBufferPos = new_rate * internal_position / m_dSamplesPerBeat;
        updateSamplesPerBeat();
    }
    
}

void EngineSync::slotSyncMasterChanged(double state)
{
    if (state) {
        setInternalMaster();
    } else {
        // XXX: TEMP
        //setDeckMaster("[Channel1]");
        // need to choose a new master, here.
    }
}

double EngineSync::getInternalBeatDistance(void)
{
    //returns number of samples distance from the last beat.
    Q_ASSERT(m_dPseudoBufferPos >= 0);
    return m_dPseudoBufferPos / m_dSamplesPerBeat;
}

void EngineSync::resetInternalBeatDistance()
{
    if (m_pSourceBeatDistance != NULL)
    {
        m_dPseudoBufferPos = m_pSourceBeatDistance->get() * m_dSamplesPerBeat;
        qDebug() << "Resetting internal beat distance to new master" << m_dPseudoBufferPos;
    }
    else
    {
        qDebug() << "Resetting internal beat distance to 0 (no master)";
        m_dPseudoBufferPos = 0;
    }
}

void EngineSync::updateSamplesPerBeat(void)
{
    //to get samples per beat, do:
    //
    // samples   samples     60 seconds     minutes
    // ------- = -------  *  ----------  *  -------
    //   beat    second       1 minute       beats
    
    // that last term is 1 over bpm.
    if (m_dMasterBpm == 0)
    {
        m_dSamplesPerBeat = m_iSampleRate;
        return;
    }
    m_dSamplesPerBeat = static_cast<double>(m_iSampleRate * 60.0) / m_dMasterBpm;
    if (m_dSamplesPerBeat <= 0)
    {
        qDebug() << "something went horribly wrong";
        m_dSamplesPerBeat = m_iSampleRate;
    }
    qDebug() << "~~~~~~~~~~~~~~~~~~~~~~~new samples per beat" << m_dSamplesPerBeat << m_iSampleRate;
}

void EngineSync::incrementPseudoPosition(int bufferSize)
{
    //the pseudo position is a double because we want to be precise,
    //and bpms may not line up exactly with samples.
    
    m_dPseudoBufferPos += bufferSize / 2; //stereo samples, so divide by 2
    
    //can't use mod because we're in double land
    Q_ASSERT(m_dSamplesPerBeat > 0);
    while (m_dPseudoBufferPos >= m_dSamplesPerBeat)
    {
        m_dPseudoBufferPos -= m_dSamplesPerBeat;
    }
    
    if (m_iSyncSource == SYNC_INTERNAL) {
        m_pMasterBeatDistance->set(getInternalBeatDistance());
    }
}

EngineBuffer* EngineSync::getMaster()
{
    return m_pMasterBuffer;
}
