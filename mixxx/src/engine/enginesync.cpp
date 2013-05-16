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
        m_sSyncSource("[Master]"),
        m_dSourceRate(0.0f), //has to be zero so that master bpm gets set correctly on startup
        m_dMasterBpm(124.0f),
        m_dPseudoBufferPos(0.0f)
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
    if (m_iSampleRate == 0) {
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

    m_pSyncRateSlider = new ControlPotmeter(ConfigKey("[Master]", "rate"), 40.0, 200.0);
    connect(m_pSyncRateSlider, SIGNAL(valueChanged(double)),
            this, SLOT(slotSyncRateSliderChanged(double)),
            Qt::DirectConnection);

    //TODO: get this from configuration
    m_pMasterBpm->set(m_dMasterBpm); //this will initialize all our values
    updateSamplesPerBeat();
}

EngineSync::~EngineSync() {
    delete m_pMasterBpm;
    delete m_pMasterBeatDistance;
    delete m_pSyncRateSlider;
}

void EngineSync::addDeck(QString deck) {
    if (m_sDeckList.contains(deck)) {
        return;
    }
    m_sDeckList.append(deck);

    // Connect objects so we can react when the user changes the settings
    ControlObject *deck_sync_state = ControlObject::getControl(ConfigKey(deck, "sync_state"));
    connect(deck_sync_state, SIGNAL(valueChanged(double)),
                this, SLOT(slotDeckStateChanged(double)));
    connect(deck_sync_state, SIGNAL(valueChangedFromEngine(double)),
                this, SLOT(slotDeckStateChanged(double)));
}

void EngineSync::disconnectMaster() {
    if (m_pSourceRate != NULL) {
        m_pSourceRate->disconnect();
        m_pSourceRate = NULL;
    }
    if (m_pSourceBeatDistance != NULL) {
        m_pSourceBeatDistance->disconnect();
        m_pSourceBeatDistance = NULL;
    }
    qDebug() << "UNSETTING master buffer (disconnected master)";
    m_pMasterBuffer = NULL;
}


void EngineSync::disableDeckMaster(QString deck) {
    if (deck == "") {
        foreach (QString deck, m_sDeckList) {
            if (deck != "[Master]") {
                // Unset master on *all* other decks -- sometimes we end up with two masters
                // for some reason.
                ControlObject *sync_state = ControlObject::getControl(ConfigKey(deck, "sync_state"));
                if (sync_state != NULL && sync_state->get() == SYNC_MASTER) {
                    sync_state->set(SYNC_SLAVE);
                }
            }
        }
    } else {
        ControlObject *sync_state = ControlObject::getControl(ConfigKey(deck, "sync_state"));
        Q_ASSERT(sync_state); //would be a programming error
        if (sync_state->get() == SYNC_MASTER) {
            sync_state->set(SYNC_SLAVE);
        }
    }
}

void EngineSync::setMaster(QString group) {
    // Convenience function that can split out to either set internal
    // or set deck master.
    // TODO(owen): midi master? or is that just internal?
    if (group == "[Master]") {
        setInternalMaster();
        return;
    } else {
        if (!setDeckMaster(group)) {
            qDebug() << "WARNING: failed to set selected master" << group << ", going with Internal instead";
            setInternalMaster();
            return;
        }
    }
    return;
}

void EngineSync::setInternalMaster(void) {
    if (m_sSyncSource == "[Master]") {
        return;
    }
    m_dMasterBpm = m_pMasterBpm->get();
    QString old_master = m_sSyncSource;
    m_sSyncSource = "[Master]";
    resetInternalBeatDistance();
    disableDeckMaster(old_master);
    disconnectMaster();
    updateSamplesPerBeat();

    // This is all we have to do, we'll start using the pseudoposition right away.
    m_pSyncInternalEnabled->set(TRUE);
    return;
}

bool EngineSync::setDeckMaster(QString deck) {
    if (deck == NULL || deck == "") {
        disconnectMaster();
        setInternalMaster();
        return true;
    }

    EngineChannel* pChannel = m_pEngineMaster->getChannel(deck);
    // Only consider channels that have a track loaded and are in the master
    // mix.

    if (pChannel) {
        disconnectMaster();
        m_pMasterBuffer = pChannel->getEngineBuffer();
        if (m_pMasterBuffer == NULL) {
            return false;
        }

        m_pSourceRate = ControlObject::getControl(ConfigKey(deck, "rateEngine"));
        if (m_pSourceRate == NULL) {
            return false;
        }
        connect(m_pSourceRate, SIGNAL(valueChangedFromEngine(double)),
                this, SLOT(slotSourceRateChanged(double)),
                Qt::DirectConnection);

        m_pSourceBeatDistance = ControlObject::getControl(ConfigKey(deck, "beat_distance"));
        if (m_pSourceBeatDistance == NULL) {
            return false;
        }
        connect(m_pSourceBeatDistance, SIGNAL(valueChangedFromEngine(double)),
                this, SLOT(slotSourceBeatDistanceChanged(double)),
                Qt::DirectConnection);

        resetInternalBeatDistance(); //reset internal beat distance to equal the new master
        m_sSyncSource = deck;
        m_pSyncInternalEnabled->set(FALSE);
        slotSourceRateChanged(m_pSourceRate->get());
        // This is not redundant, I swear.  Make sure lights are all up to date
        ControlObject::getControl(ConfigKey(deck, "sync_master"))->set(TRUE);
        ControlObject::getControl(ConfigKey(deck, "sync_slave"))->set(FALSE);
        return true;
    }

    return false;
}

bool EngineSync::setMidiMaster() {
    // Stub for now.
    return false;
}

QString EngineSync::chooseNewMaster(QString dontpick="") {
    QString fallback = "[Master]";
    foreach (QString deck, m_sDeckList) {
        if (deck == dontpick) {
            continue;
        }

        ControlObject *sync_state = ControlObject::getControl(ConfigKey(deck, "sync_state"));
        if (sync_state != NULL) {
            double state = sync_state->get();
            if (state == SYNC_MASTER) {
                return deck;
            } else if (state == SYNC_NONE) {
                continue;
            }
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

    return fallback;
}

void EngineSync::slotSourceRateChanged(double rate_engine) {
    //master buffer can be null due to timing issues
    if (m_pMasterBuffer != NULL && rate_engine != m_dSourceRate) {
        m_dSourceRate = rate_engine;

        double filebpm = m_pMasterBuffer->getFileBpm();
        m_dMasterBpm = rate_engine * filebpm;

        if (m_dMasterBpm != 0) {
            m_pSyncRateSlider->set(m_dMasterBpm);
        }
        m_pMasterBpm->set(m_dMasterBpm); //this will trigger all of the slaves to change rate
    }
}

void EngineSync::slotSourceBeatDistanceChanged(double beat_dist) {
    //pass it on to slaves and update internal position marker
    m_pMasterBeatDistance->set(beat_dist);
    setPseudoPosition(beat_dist);
}

void EngineSync::slotSyncRateSliderChanged(double new_bpm) {
    if (m_sSyncSource != "[Master]") {
        // TODO: this should be prevented by setting the slider to disabled.
        m_pSyncRateSlider->set(m_dMasterBpm);
        return;
    }
    m_pMasterBpm->set(new_bpm);
}

void EngineSync::slotMasterBpmChanged(double new_bpm) {
    m_pSyncRateSlider->set(new_bpm);
    if (new_bpm != m_dMasterBpm) {
        if (m_sSyncSource != "[Master]") {
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
        m_dMasterBpm = new_bpm;
        updateSamplesPerBeat();

        //this change could hypothetically push us over distance 1.0, so check
        //XXX: is this code correct?  I think it'll work but it seems off
        Q_ASSERT(m_dSamplesPerBeat > 0);
        while (m_dPseudoBufferPos >= m_dSamplesPerBeat) {
            m_dPseudoBufferPos -= m_dSamplesPerBeat;
        }
    }
}

void EngineSync::slotSampleRateChanged(double srate) {
    int new_rate = static_cast<int>(srate);
    double internal_position = getInternalBeatDistance();
    if (new_rate != m_iSampleRate) {
        m_iSampleRate = new_rate;
        //recalculate pseudo buffer position based on new sample rate
        m_dPseudoBufferPos = new_rate * internal_position / m_dSamplesPerBeat;
        updateSamplesPerBeat();
    }
}

void EngineSync::slotInternalMasterChanged(double state) {
    if (state) {
        setInternalMaster();
    } else {
        //internal has been turned off.  pick a slave
        setMaster(chooseNewMaster());
    }
}

void EngineSync::slotDeckStateChanged(double state) {
    //figure out who called us
    ControlObject *caller = qobject_cast<ControlObject* >(QObject::sender());
    Q_ASSERT(caller); //this will only fail because of a programming error
    QString group = caller->getKey().group;

    // In the following logic, m_sSyncSource acts like "previous sync source".
    if (state == SYNC_MASTER) {
        // TODO: don't allow setting of master if not playing
        // Figure out who the old master was and turn them off
        QString old_master = m_sSyncSource;
        setDeckMaster(group);
        if (old_master != "[Master]") {
            disableDeckMaster(old_master);
        }
    } else if (state == SYNC_SLAVE) {
        // Was this deck master before?  If so do a handoff
        ControlObject *sync_state = ControlObject::getControl(ConfigKey(group, "sync_state"));
        if (m_sSyncSource == group) {
            sync_state->set(SYNC_SLAVE);
            //choose a new master, but don't pick the current one!
            setMaster(chooseNewMaster(group));
        }
    } else {
        // if we were the master, choose a new one.
        if (m_sSyncSource == group) {
            setMaster(chooseNewMaster());
        }
    }
}

double EngineSync::getInternalBeatDistance(void) const {
    //returns number of samples distance from the last beat.
    Q_ASSERT(m_dPseudoBufferPos >= 0);
    return m_dPseudoBufferPos / m_dSamplesPerBeat;
}

void EngineSync::resetInternalBeatDistance() {
    if (m_pSourceBeatDistance != NULL) {
        m_dPseudoBufferPos = m_pSourceBeatDistance->get() * m_dSamplesPerBeat;
    } else {
        m_dPseudoBufferPos = 0;
    }
}

void EngineSync::updateSamplesPerBeat(void) {
    //to get samples per beat, do:
    //
    // samples   samples     60 seconds     minutes
    // ------- = -------  *  ----------  *  -------
    //   beat    second       1 minute       beats

    // that last term is 1 over bpm.
    if (m_dMasterBpm == 0) {
        m_dSamplesPerBeat = m_iSampleRate;
        return;
    }
    m_dSamplesPerBeat = static_cast<double>(m_iSampleRate * 60.0) / m_dMasterBpm;
    if (m_dSamplesPerBeat <= 0) {
        m_dSamplesPerBeat = m_iSampleRate;
    }
}

void EngineSync::incrementPseudoPosition(int bufferSize) {
    // Enginemaster calls this function, it is used to keep track of the internal
    // clock (when there is no other master like a deck or MIDI
    // the pseudo position is a double because we want to be precise,
    // and bpms may not line up exactly with samples.

    if (m_sSyncSource != "[Master]") {
        //we don't care, it will get set in setPseudoPosition
        return;
    }

    m_dPseudoBufferPos += bufferSize / 2; //stereo samples, so divide by 2

    //can't use mod because we're in double land
    Q_ASSERT(m_dSamplesPerBeat > 0);
    while (m_dPseudoBufferPos >= m_dSamplesPerBeat) {
        m_dPseudoBufferPos -= m_dSamplesPerBeat;
    }

    m_pMasterBeatDistance->set(getInternalBeatDistance());
}

void EngineSync::setPseudoPosition(double percent) {
    m_dPseudoBufferPos = percent * m_dSamplesPerBeat;
}

EngineBuffer* EngineSync::getMaster() const {
    return m_pMasterBuffer;
}
