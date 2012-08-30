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
        m_dSourceRate(0.0f), //has to be zero so that master bpm gets set correctly on startup
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
            this, SLOT(slotInternalMasterChanged(double)),
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

void EngineSync::addDeck(QString deck)
{
    if (m_sDeckList.contains(deck))
    {
        qDebug() << "EngineSync: already has deck for deck" << deck;
        return;
    }
    m_sDeckList.append(deck);
    
    // Connect objects so we can react when the user changes the settings
    ControlObject *deck_master_enabled = ControlObject::getControl(ConfigKey(deck, "sync_master"));
    connect(deck_master_enabled, SIGNAL(valueChanged(double)),
                this, SLOT(slotDeckMasterChanged(double)));
    //connect(deck_master_enabled, SIGNAL(valueChangedFromEngine(double)),
    //            this, SLOT(slotDeckMasterChanged(double)));
    ControlObject *deck_slave_enabled = ControlObject::getControl(ConfigKey(deck, "sync_slave"));
    connect(deck_slave_enabled, SIGNAL(valueChanged(double)),
                this, SLOT(slotDeckSlaveChanged(double)));
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


void EngineSync::disableDeckMaster(QString deck)
{
    //this merely unsets the control objects (midi lights)
    if (deck == "")
    {
        foreach (deck, m_sDeckList)
        {
            ControlObject *sync_master = ControlObject::getControl(ConfigKey(deck, "sync_master"));
            if (sync_master != NULL)
            {
                if (sync_master->get()) {
                    //qDebug() << "disabledeckmaster found" << deck << "was the old master, shutting it down";
                    sync_master->set(FALSE);
                    ControlObject *sync_slave = ControlObject::getControl(ConfigKey(deck, "sync_slave"));
                    Q_ASSERT(sync_slave);
                    sync_slave->set(TRUE);
                }
            }
        }
    }
    else
    {
        //qDebug() << "disabling" << deck << "as master";
        ControlObject *sync_master = ControlObject::getControl(ConfigKey(deck, "sync_master"));
        Q_ASSERT(sync_master); //would be a programming error
        if (sync_master->get()) {
            //qDebug() << deck << "notifying deck it is not master";
            sync_master->set(FALSE);
            ControlObject *sync_slave = ControlObject::getControl(ConfigKey(deck, "sync_slave"));
            Q_ASSERT(sync_slave);
            sync_slave->set(TRUE);
        }
        //else {
        //    qDebug() << deck << "already wasn't master????";
        //}
    }
}

bool EngineSync::setMaster(QString group)
{
    // Convenience function that can split out to either set internal
    // or set deck master.
    //TODO: midi master? or is that just internal?
    if (group == "[Master]") {
        return setInternalMaster();
    } else {
        if (!setDeckMaster(group)) {
            qDebug() << "WARNING: failed to set selected master" << group << ", going with Internal instead";
            return setInternalMaster();
        }
    }
    return FALSE;
}

bool EngineSync::setInternalMaster(void)
{
    m_dMasterBpm = m_pMasterBpm->get();
    resetInternalBeatDistance();
    disableDeckMaster("");
    disconnectMaster();
    updateSamplesPerBeat();
    
    //qDebug() << "*****************WHEEEEEEEEEEEEEEE INTERNAL";
    //this is all we have to do, we'll start using the pseudoposition right away
    m_iSyncSource = SYNC_INTERNAL;
    m_pSyncInternalEnabled->set(TRUE);
    return true;
}

bool EngineSync::setDeckMaster(QString deck)
{
    if (deck == NULL || deck == "")
    {
        //qDebug() << "----------------------------------------------------unsetting master (got null)";
        disconnectMaster();
        setInternalMaster();
        return true;
    }
    
    EngineChannel* pChannel = m_pEngineMaster->getChannel(deck);
    // Only consider channels that have a track loaded and are in the master
    // mix.

    //qDebug() << "**************************************************************************asked to set a new master:" << deck;
    
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
        //qDebug() << "----------------------------setting new master" << deck;
        m_iSyncSource = SYNC_DECK;
        m_pSyncInternalEnabled->set(FALSE);
        //this is not redundant, I swear.  Make sure lights are all up to date
        ControlObject::getControl(ConfigKey(deck, "sync_master"))->set(TRUE);
        ControlObject::getControl(ConfigKey(deck, "sync_slave"))->set(FALSE);
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

QString EngineSync::chooseNewMaster(QString dontpick="")
{
    //qDebug() << "----------=-=-=-=-=-=-=-finding a new master";
    QString fallback = "[Master]";
    foreach (QString deck, m_sDeckList)
    {
        if (deck == dontpick) {
            continue;
        }

        ControlObject *sync_master = ControlObject::getControl(ConfigKey(deck, "sync_master"));
        if (sync_master != NULL)
        {
            if (sync_master->get()) {
                qDebug() << "already have a new master" << deck;
                return deck;
            }
        }
        ControlObject *sync_slave = ControlObject::getControl(ConfigKey(deck, "sync_slave"));
        if (sync_slave == NULL) {
            continue;
        }
        if (!sync_slave->get()) {
            //qDebug() << deck << "is not a slave, so no point";
            continue;
        }
        EngineChannel* pChannel = m_pEngineMaster->getChannel(deck);
        if (pChannel && pChannel->isActive() && pChannel->isMaster()) {
            EngineBuffer* pBuffer = pChannel->getEngineBuffer();
            if (pBuffer && pBuffer->getBpm() > 0) {
                // If the deck is playing then go with it immediately.
                if (fabs(pBuffer->getRate()) > 0) {
                    //qDebug() << "picked a new master deck:" << deck;
                    return deck;
                }
            }
        }
    }    
    
    //qDebug() << "picked a new master deck (fallback):" << fallback;
    return fallback;
}

void EngineSync::slotSourceRateChanged(double true_rate)
{
    //qDebug() << "got a true rate update";
    //master buffer can be null due to timing issues
    if (m_pMasterBuffer == NULL)
        qDebug() << "but master buffer is null";
    
    if (true_rate != m_dSourceRate && m_pMasterBuffer != NULL)
    {
        m_dSourceRate = true_rate;
        
        double filebpm = m_pMasterBuffer->getFileBpm();
        m_dMasterBpm = true_rate * filebpm;
        //qDebug()<< "announcing a master bpm of" <<  m_dMasterBpm;
        
        m_pMasterBpm->set(m_dMasterBpm); //this will trigger all of the slaves to change rate
    }
}

void EngineSync::slotSourceBeatDistanceChanged(double beat_dist)
{
    //pass it on to slaves and update internal position marker
    m_pMasterBeatDistance->set(beat_dist);
    setPseudoPosition(beat_dist);
}

void EngineSync::slotMasterBpmChanged(double new_bpm)
{
    //qDebug() << "~~~~~~~~~~~~~~~~~~~~~~new master bpm" << new_bpm;
    if (new_bpm != m_dMasterBpm)
    {
        if (m_iSyncSource != SYNC_INTERNAL)
        {
            //qDebug() << "can't set master sync when sync isn't internal";
            //XXX(Owen):
            //it looks like this is Good Enough for preventing accidental
            //tweaking of rate.  But maybe it should set master to internal?
            
            //Changing to internal is weird, feels like a bug having master
            //designation turn off
            //setInternalMaster();
            
            //how about just setting the bpm value for the deck master?
            //problem with that is here we have bpm, but deck expects
            //a percentage.  Let's keep this to "no you can't do that" for now
            
            m_pMasterBpm->set(m_dMasterBpm);
            return;
        }
        //qDebug() << "using it";
        m_dMasterBpm = new_bpm;
        updateSamplesPerBeat();
        
        //this change could hypothetically push us over distance 1.0, so check
        //XXX: is this code correct?  I think it'll work but it seems off
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
        //qDebug() << "new samplerate!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << srate;
        m_iSampleRate = new_rate;
        //recalculate pseudo buffer position based on new sample rate
        m_dPseudoBufferPos = new_rate * internal_position / m_dSamplesPerBeat;
        updateSamplesPerBeat();
    }
    
}

void EngineSync::slotInternalMasterChanged(double state)
{
    //qDebug() << "internal master toggled" << state;
    if (state) {
        setInternalMaster();
    } else {
        //internal has been turned off.  pick a slave
        setMaster(chooseNewMaster());
    }
}

void EngineSync::slotDeckMasterChanged(double state)
{
    //figure out who called us
    ControlObject *caller = qobject_cast<ControlObject* >(QObject::sender());
    Q_ASSERT(caller); //this will only fail because of a programming error
    //get the group from that
    QString group = caller->getKey().group;
    //qDebug() << "got a master state change from" << group;
    
    if (state) {
        // Figure out who the old master was and turn them off
        foreach (QString deck, m_sDeckList)
        {
            ControlObject *sync_master = ControlObject::getControl(ConfigKey(deck, "sync_master"));
            if (sync_master == NULL) {
                continue;
            }
            if (deck == group) {
                continue;
            }
            if (sync_master->get()) {
                //qDebug() << deck << "unsetting old master";
                //XXX: I think this may cause flicker:
                //     1. user clicked master-enable for, say, deck 2
                //     2. mixxx notices deck 3 is already master, disables it
                //     3. mixxx chooses another deck to be master, say 1
                //     4. control returns here, and we forcibly set deck 2.
                //  So flicker could happen when master flips from auto-set to actual-set
                //  Added a check in choose() so that if it finds a master, it stops
                
                disableDeckMaster(deck);
            }
        }
    
        //qDebug() << "setting" << group << "to master";
        setDeckMaster(group);
    } else {
        //qDebug() << "disabled master" << group;
        //turned a master off
        setMaster(chooseNewMaster());
    }
}

void EngineSync::slotDeckSlaveChanged(double state)
{
    //figure out who called us
    ControlObject *caller = qobject_cast<ControlObject* >(QObject::sender());
    Q_ASSERT(caller); //this will only fail because of a programming error
    //get the group from that
    QString group = caller->getKey().group;
    //qDebug() << "got a slave state change from" << group;
    
    if (state) {
        // Was this deck master before?  If so do a handoff
        ControlObject *sync_master = ControlObject::getControl(ConfigKey(group, "sync_master"));
        if (sync_master->get()) {
            sync_master->set(FALSE);
            //choose a new master, but don't pick the current one!
            setMaster(chooseNewMaster(group));
        }
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
        //qDebug() << "Resetting internal beat distance to new master" << m_dPseudoBufferPos;
    }
    else
    {
        //qDebug() << "Resetting internal beat distance to 0 (no master)";
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
        qDebug() << "something went horribly wrong setting samples per beat";
        m_dSamplesPerBeat = m_iSampleRate;
    }
    //qDebug() << "~~~~~~~~~~~~~~~~~~~~~~~new samples per beat" << m_dSamplesPerBeat << m_iSampleRate;
}

void EngineSync::incrementPseudoPosition(int bufferSize)
{
    //enginemaster calls this function, it is used to keep track of the internal
    //clock (when there is no other master like a deck or MIDI
    //the pseudo position is a double because we want to be precise,
    //and bpms may not line up exactly with samples.
    
    if (m_iSyncSource != SYNC_INTERNAL) {
        //we don't care, it will get set in setPseudoPosition
        return;
    }
    
    m_dPseudoBufferPos += bufferSize / 2; //stereo samples, so divide by 2
    
    //can't use mod because we're in double land
    Q_ASSERT(m_dSamplesPerBeat > 0);
    while (m_dPseudoBufferPos >= m_dSamplesPerBeat)
    {
        m_dPseudoBufferPos -= m_dSamplesPerBeat;
    }
    
    m_pMasterBeatDistance->set(getInternalBeatDistance());
}

void EngineSync::setPseudoPosition(double percent)
{
    m_dPseudoBufferPos = percent * m_dSamplesPerBeat;
}

EngineBuffer* EngineSync::getMaster()
{
    return m_pMasterBuffer;
}
