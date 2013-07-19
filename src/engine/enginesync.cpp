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

#include "controlobject.h"
#include "controlpotmeter.h"
#include "controlpushbutton.h"
#include "engine/enginebuffer.h"
#include "engine/enginechannel.h"
#include "engine/enginecontrol.h"
#include "engine/enginesync.h"

static const char* kMasterSyncGroup = "[Master]";

SyncChannel::SyncChannel(EngineChannel* pChannel)
        : m_pChannel(pChannel),
          m_group(pChannel->getGroup()) {
    m_pChannelSyncState = new ControlObject(ConfigKey(m_group, "sync_state"));
    connect(m_pChannelSyncState, SIGNAL(valueChanged(double)),
            this, SLOT(slotChannelSyncStateChanged(double)),
            Qt::DirectConnection);
    connect(m_pChannelSyncState, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotChannelSyncStateChanged(double)),
            Qt::DirectConnection);

    m_pFileBpm = ControlObject::getControl(ConfigKey(m_group, "file_bpm"));
    m_pRateEngine = ControlObject::getControl(ConfigKey(m_group, "rateEngine"));
    m_pBeatDistance = ControlObject::getControl(ConfigKey(m_group, "beat_distance"));
}

SyncChannel::~SyncChannel() {
    delete m_pChannelSyncState;
}

EngineChannel* SyncChannel::getChannel() {
    return m_pChannel;
}

double SyncChannel::getFileBpm() const {
    return m_pFileBpm ? m_pFileBpm->get() : 0.0;
}

double SyncChannel::getState() const {
    return m_pChannelSyncState->get();
}

void SyncChannel::setState(double state) {
    m_pChannelSyncState->set(state);
}

void SyncChannel::slotChannelSyncStateChanged(double v) {
    emit(channelSyncStateChanged(this, v));
}

ControlObject* SyncChannel::getRateEngineControl() {
    return m_pRateEngine;
}

ControlObject* SyncChannel::getBeatDistanceControl() {
    return m_pBeatDistance;
}

EngineSync::EngineSync(ConfigObject<ConfigValue>* _config)
        : EngineControl(kMasterSyncGroup, _config),
          m_sSyncSource(kMasterSyncGroup),
          m_dSourceRate(0.0f), //has to be zero so that master bpm gets set correctly on startup
          m_dMasterBpm(124.0f),
          m_dPseudoBufferPos(0.0f) {
    m_pMasterBeatDistance = new ControlObject(ConfigKey(kMasterSyncGroup, "beat_distance"));

    m_pSampleRate = ControlObject::getControl(ConfigKey(kMasterSyncGroup, "samplerate"));
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

    m_pMasterBpm = new ControlObject(ConfigKey(kMasterSyncGroup, "sync_bpm"));
    connect(m_pMasterBpm, SIGNAL(valueChanged(double)),
            this, SLOT(slotMasterBpmChanged(double)),
            Qt::DirectConnection);
    connect(m_pMasterBpm, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotMasterBpmChanged(double)),
            Qt::DirectConnection);

    m_pSyncInternalEnabled = new ControlPushButton(ConfigKey(kMasterSyncGroup, "sync_master"));
    m_pSyncInternalEnabled->setButtonMode(ControlPushButton::TOGGLE);
    connect(m_pSyncInternalEnabled, SIGNAL(valueChanged(double)),
            this, SLOT(slotInternalMasterChanged(double)),
            Qt::DirectConnection);

    m_pSyncRateSlider = new ControlPotmeter(ConfigKey(kMasterSyncGroup, "rate"), 40.0, 200.0);
    connect(m_pSyncRateSlider, SIGNAL(valueChanged(double)),
            this, SLOT(slotSyncRateSliderChanged(double)),
            Qt::DirectConnection);

    //TODO: get this from configuration
    m_pMasterBpm->set(m_dMasterBpm); //this will initialize all our values
    updateSamplesPerBeat();
}

EngineSync::~EngineSync() {
    while (!m_channels.isEmpty()) {
        delete m_channels.takeLast();
    }
    delete m_pMasterBpm;
    delete m_pMasterBeatDistance;
    delete m_pSyncRateSlider;
}

void EngineSync::addChannel(EngineChannel* pChannel) {
    const QString& group = pChannel->getGroup();
    foreach (const SyncChannel* pChannel, m_channels) {
        if (pChannel->getGroup() == group) {
            qDebug() << "EngineSync: already has channel for" << group;
            return;
        }
    }
    SyncChannel* pSyncChannel = new SyncChannel(pChannel);
    connect(pSyncChannel, SIGNAL(channelSyncStateChanged(SyncChannel*, double)),
            this, SLOT(slotChannelSyncStateChanged(SyncChannel*, double)),
            Qt::DirectConnection);
    m_channels.append(pSyncChannel);
}

void EngineSync::disableChannelMaster(const QString& channel) {
    qDebug() << "UNSETTING master channel (disconnected master)";

    SyncChannel* pOldChannelMaster = m_pChannelMaster;
    if (pOldChannelMaster) {
        ControlObject* pSourceRate = pOldChannelMaster->getRateEngineControl();
        if (pSourceRate != NULL) {
            disconnect(pSourceRate, SIGNAL(valueChangedFromEngine(double)),
                       this, SLOT(slotSourceRateChanged(double)));
        }
        ControlObject* pSourceBeatDistance = pOldChannelMaster->getBeatDistanceControl();
        if (pSourceBeatDistance != NULL) {
            disconnect(pSourceBeatDistance, SIGNAL(valueChangedFromEngine(double)),
                       this, SLOT(slotSourceBeatDistanceChanged(double)));
        }
    }
    m_pChannelMaster = NULL;

    bool channelIsEmpty = channel.isEmpty();
    foreach (SyncChannel* pChannel, m_channels) {
        // If channel is empty, unset master on *all* other channels -- sometimes we
        // end up with two masters for some reason.
        if (channelIsEmpty || pChannel->getGroup() == channel) {
            if (pChannel->getState() == SYNC_MASTER) {
                pChannel->setState(SYNC_SLAVE);
            }
        }
    }
}

void EngineSync::setMaster(const QString& group) {
    // Convenience function that can split out to either set internal
    // or set deck master.
    if (group == kMasterSyncGroup) {
        setInternalMaster();
    } else {
        SyncChannel* pSyncChannel = getSyncChannelForGroup(group);
        if (!setChannelMaster(pSyncChannel)) {
            qDebug() << "WARNING: failed to set selected master" << group << ", going with Internal instead";
            setInternalMaster();
        }
    }
}

void EngineSync::setInternalMaster() {
    if (m_sSyncSource == kMasterSyncGroup) {
        qDebug() << "already internal master";
        return;
    }
    m_dMasterBpm = m_pMasterBpm->get();
    QString old_master = m_sSyncSource;
    m_sSyncSource = kMasterSyncGroup;
    resetInternalBeatDistance();
    disableChannelMaster(old_master);
    updateSamplesPerBeat();

    // This is all we have to do, we'll start using the pseudoposition right away.
    m_pSyncInternalEnabled->set(TRUE);
}

bool EngineSync::setChannelMaster(SyncChannel* pSyncChannel) {
    // If a channel is master, disable it.
    disableChannelMaster(m_sSyncSource);

    if (pSyncChannel == NULL) {
        return false;
    }

    // Only accept channels with an EngineBuffer.
    EngineChannel* pChannel = pSyncChannel->getChannel();
    if (pChannel == NULL || pChannel->getEngineBuffer() == NULL) {
        return false;
    }

    const QString& group = pChannel->getGroup();

    // Only consider channels that have a track loaded and are in the master
    // mix.
    //qDebug() << "***********************************************asked to set a new master:" << deck;
    m_pChannelMaster = pSyncChannel;
    m_sSyncSource = group;


    ControlObject* pSourceRate = pSyncChannel->getRateEngineControl();
    if (pSourceRate == NULL) {
        //qDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!! source true rate was null";
        return false;
    }
    connect(pSourceRate, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotSourceRateChanged(double)),
            Qt::DirectConnection);

    ControlObject* pSourceBeatDistance = pSyncChannel->getBeatDistanceControl();
    if (pSourceBeatDistance == NULL) {
        qDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1 source beat dist was null";
        return false;
    }
    connect(pSourceBeatDistance, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotSourceBeatDistanceChanged(double)),
            Qt::DirectConnection);

    // reset internal beat distance to equal the new master
    resetInternalBeatDistance();

    //qDebug() << "----------------------------setting new master" << group;

    m_pSyncInternalEnabled->set(FALSE);
    slotSourceRateChanged(pSourceRate->get());

    // This is not redundant, I swear.  Make sure lights are all up to date
    ControlObject::getControl(ConfigKey(group, "sync_master"))->set(TRUE);
    ControlObject::getControl(ConfigKey(group, "sync_slave"))->set(FALSE);

    return true;
}

QString EngineSync::chooseNewMaster(const QString& dontpick) {
    //qDebug() << "----------=-=-=-=-=-=-=-finding a new master";
    QString fallback = kMasterSyncGroup;
    foreach (SyncChannel* pSyncChannel, m_channels) {
        const QString& group = pSyncChannel->getGroup();
        if (group == dontpick) {
            continue;
        }

        double sync_state = pSyncChannel->getState();
        if (sync_state == SYNC_MASTER) {
            qDebug() << "already have a new master" << group;
            return group;
        } else if (sync_state == SYNC_NONE) {
            continue;
        }

        EngineChannel* pChannel = pSyncChannel->getChannel();
        if (pChannel && pChannel->isActive() && pChannel->isMaster()) {
            EngineBuffer* pBuffer = pChannel->getEngineBuffer();
            if (pBuffer && pBuffer->getBpm() > 0) {
                // If the channel is playing then go with it immediately.
                if (fabs(pBuffer->getRate()) > 0) {
                    //qDebug() << "picked a new master deck:" << group;
                    return group;
                }
            }
        }
    }
    return fallback;
}

void EngineSync::slotSourceRateChanged(double rate_engine) {
    //master buffer can be null due to timing issues
    if (m_pChannelMaster != NULL && rate_engine != m_dSourceRate) {
        m_dSourceRate = rate_engine;
        double filebpm = m_pChannelMaster->getFileBpm();
        m_dMasterBpm = rate_engine * filebpm;

        //qDebug() << "file bpm " << filebpm;
        //qDebug()<< "announcing a master bpm of" <<  m_dMasterBpm;

        if (m_dMasterBpm != 0) {
            m_pSyncRateSlider->set(m_dMasterBpm);
        }
        // This will trigger all of the slaves to change rate.
        m_pMasterBpm->set(m_dMasterBpm);
    }
}

void EngineSync::slotSourceBeatDistanceChanged(double beat_dist) {
    // Pass it on to slaves and update internal position marker.
    m_pMasterBeatDistance->set(beat_dist);
    setPseudoPosition(beat_dist);
}

void EngineSync::slotSyncRateSliderChanged(double new_bpm) {
    if (m_sSyncSource != kMasterSyncGroup) {
        // TODO: this should be prevented by setting the slider to disabled.
        m_pSyncRateSlider->set(m_dMasterBpm);
        return;
    }
    //qDebug() << "trying to set internal master to " << new_bpm;
    m_pMasterBpm->set(new_bpm);
}

void EngineSync::slotMasterBpmChanged(double new_bpm) {
//    qDebug() << "~~~~~~~~~~~~~~~~~~~~~~new master bpm" << new_bpm;
    m_pSyncRateSlider->set(new_bpm);
    if (new_bpm != m_dMasterBpm) {
  //      qDebug() << "set slider";
        if (m_sSyncSource != kMasterSyncGroup) {
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
    //    qDebug() << "using it";
        m_dMasterBpm = new_bpm;
        updateSamplesPerBeat();

        //this change could hypothetically push us over distance 1.0, so check
        //XXX: is this code correct?  I think it'll work but it seems off
        if (m_dSamplesPerBeat <= 0) {
            qDebug() << "ERROR: Calculated <= 0 samples per beat which is impossible.  Forcibly "
                     << "setting to about 124bpm at 44.1Khz.";
            m_dSamplesPerBeat = 21338;
        }
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
        setMaster(chooseNewMaster(""));
    }
}

void EngineSync::slotChannelSyncStateChanged(SyncChannel* pSyncChannel, double state) {
    if (!pSyncChannel) {
        return;
    }
    const QString& group = pSyncChannel->getGroup();
    qDebug() << "got a master state change from" << group << state;

    const bool channelIsMaster = m_sSyncSource == group;

    // In the following logic, m_sSyncSource acts like "previous sync source".
    if (state == SYNC_MASTER) {
        // TODO: don't allow setting of master if not playing
        // If setting this channel as master fails, pick a new master.
        if (!setChannelMaster(pSyncChannel)) {
            setMaster(chooseNewMaster(group));
        }
    } else if (state == SYNC_SLAVE) {
        // Was this deck master before?  If so do a handoff
        if (channelIsMaster) {
            qDebug() << group << " current master, setting us to slave (choose new)";
            // TODO(rryan) isn't this redundant? Is this because of MIDI light
            // breakage?
            pSyncChannel->setState(SYNC_SLAVE);
            //choose a new master, but don't pick the current one!
            setMaster(chooseNewMaster(group));
        }
    } else {
        // if we were the master, choose a new one.
        if (channelIsMaster) {
            qDebug() << group << " current master being set to none, choose new";
            setMaster(chooseNewMaster(""));
        }
    }
}

double EngineSync::getInternalBeatDistance() const {
    //returns number of samples distance from the last beat.
    if (m_dPseudoBufferPos < 0) {
        qDebug() << "ERROR: Internal beat distance should never be less than zero";
        return 0.0;
    }
    return m_dPseudoBufferPos / m_dSamplesPerBeat;
}

void EngineSync::resetInternalBeatDistance() {
    ControlObject* pSourceBeatDistance = m_pChannelMaster != NULL ?
            m_pChannelMaster->getBeatDistanceControl() : NULL;
    double beat_distance = pSourceBeatDistance ? pSourceBeatDistance->get() : 0;

    m_dPseudoBufferPos = beat_distance * m_dSamplesPerBeat;
    qDebug() << "Resetting internal beat distance to new master"
             << m_dPseudoBufferPos << beat_distance;
}

void EngineSync::updateSamplesPerBeat() {
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
    m_dSamplesPerBeat = (m_iSampleRate * 60.0) / m_dMasterBpm;
    if (m_dSamplesPerBeat <= 0) {
        qDebug() << "something went horribly wrong setting samples per beat";
        m_dSamplesPerBeat = m_iSampleRate;
    }
}

void EngineSync::onCallbackStart(int bufferSize) {
    // EngineMaster calls this function, it is used to keep track of the
    // internal clock (when there is no other master like a deck or MIDI) the
    // pseudo position is a double because we want to be precise, and beats may
    // not line up exactly with samples.

    if (m_sSyncSource != kMasterSyncGroup) {
        // We don't care, it will get set in setPseudoPosition.
        return;
    }

    m_dPseudoBufferPos += bufferSize / 2; // stereo samples, so divide by 2

    // can't use mod because we're in double land
    if (m_dSamplesPerBeat <= 0) {
        qDebug() << "ERROR: Calculated <= 0 samples per beat which is impossible.  Forcibly "
                 << "setting to about 124 bpm at 44.1Khz.";
        m_dSamplesPerBeat = 21338;
    }
    while (m_dPseudoBufferPos >= m_dSamplesPerBeat) {
        m_dPseudoBufferPos -= m_dSamplesPerBeat;
    }

    m_pMasterBeatDistance->set(getInternalBeatDistance());
}

void EngineSync::setPseudoPosition(double percent) {
    m_dPseudoBufferPos = percent * m_dSamplesPerBeat;
}

EngineChannel* EngineSync::getMaster() const {
    return m_pChannelMaster ? m_pChannelMaster->getChannel() : NULL;
}

SyncChannel* EngineSync::getSyncChannelForGroup(const QString& group) {
    foreach (SyncChannel* pChannel, m_channels) {
        if (pChannel->getGroup() == group) {
            return pChannel;
        }
    }
    return NULL;
}
