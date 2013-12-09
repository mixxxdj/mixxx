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
#include "engine/ratecontrol.h"

static const char* kMasterSyncGroup = "[Master]";

EngineSync::EngineSync(ConfigObject<ConfigValue>* _config)
        : EngineControl(kMasterSyncGroup, _config),
          m_pConfig(_config),
          m_pChannelMaster(NULL),
          m_sSyncSource(""),
          m_bExplicitMasterSelected(false),
          m_dInternalClockPosition(0.0f) {
    m_pMasterBeatDistance = new ControlObject(ConfigKey(kMasterSyncGroup, "beat_distance"));

    m_pSampleRate = ControlObject::getControl(ConfigKey(kMasterSyncGroup, "samplerate"));
    connect(m_pSampleRate, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotSampleRateChanged(double)),
            Qt::DirectConnection);
    connect(m_pSampleRate, SIGNAL(valueChanged(double)),
            this, SLOT(slotSampleRateChanged(double)),
            Qt::DirectConnection);

    if (m_pSampleRate->get()) {
        m_pSampleRate->set(44100);
    }

    m_pMasterBpm = new ControlObject(ConfigKey(kMasterSyncGroup, "sync_bpm"));
    // Initialize with a default value (will get overridden by config).
    m_pMasterBpm->set(124.0);
    connect(m_pMasterBpm, SIGNAL(valueChanged(double)),
            this, SLOT(slotMasterBpmChanged(double)),
            Qt::DirectConnection);
    connect(m_pMasterBpm, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotMasterBpmChanged(double)),
            Qt::DirectConnection);

    m_pInternalClockMasterEnabled =
            new ControlPushButton(ConfigKey(kMasterSyncGroup, "sync_master"));
    m_pInternalClockMasterEnabled->setButtonMode(ControlPushButton::TOGGLE);
    connect(m_pInternalClockMasterEnabled, SIGNAL(valueChanged(double)),
            this, SLOT(slotClockSyncModeChanged(double)),
            Qt::DirectConnection);

    m_pMasterRateSlider = new ControlPotmeter(ConfigKey(kMasterSyncGroup, "sync_slider"),
                                                40.0, 200.0);
    connect(m_pMasterRateSlider, SIGNAL(valueChanged(double)),
            this, SLOT(slotSyncRateSliderChanged(double)),
            Qt::DirectConnection);
    connect(m_pMasterRateSlider, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotSyncRateSliderChanged(double)),
            Qt::DirectConnection);

    updateSamplesPerBeat();
}

EngineSync::~EngineSync() {
    // We use the slider value because that is never set to 0.0.
    m_pConfig->set(ConfigKey("[Master]", "sync_bpm"), ConfigValue(m_pMasterRateSlider->get()));
    delete m_pMasterBpm;
    delete m_pMasterBeatDistance;
    delete m_pMasterRateSlider;
}

void EngineSync::addChannel(EngineChannel* pChannel) {
    foreach (RateControl* pRate, m_ratecontrols) {
        if (pRate->getGroup() == pChannel->getGroup()) {
            pRate->setEngineChannel(pChannel);
            return;
        }
    }
    qDebug() << "No RateControl found for group (probably not a playback deck) "
             << pChannel->getGroup();
}

void EngineSync::addDeck(RateControl *pNewRate) {
    foreach (RateControl* pRate, m_ratecontrols) {
        if (pRate->getGroup() == pNewRate->getGroup()) {
            // BUG -- pRate is local, this doesn't work.
            qDebug() << "EngineSync: already has channel for" << pRate->getGroup() << ", replacing";
            pRate = pNewRate;
            return;
        }
    }
    m_ratecontrols.append(pNewRate);
}

void EngineSync::requestSyncMode(RateControl* pRateControl, int state) {
    // Based on the call hierarchy I don't think this is possible. (Famous last words.)
    Q_ASSERT(pRateControl);

    const QString& group = pRateControl->getGroup();
    const bool channelIsMaster = m_sSyncSource == group;

    // In the following logic, m_sSyncSource acts like "previous sync source".
    if (state == SYNC_MASTER) {
        // RateControl is explicitly requesting master, so we'll honor that.
        m_bExplicitMasterSelected = true;
        // If setting this channel as master fails, pick a new master.
        if (!activateChannelMaster(pRateControl)) {
            findNewMaster(group);
        }
    } else if (state == SYNC_FOLLOWER) {
        // Was this deck master before?  If so do a handoff.
        if (channelIsMaster) {
            // Choose a new master, but don't pick the current one.
            findNewMaster(group);
        } else if (m_bExplicitMasterSelected) {
            // Do nothing.
            return;
        }

        if (m_sSyncSource == "") {
            // If there is no current master, set to master.
            pRateControl->notifyModeChanged(SYNC_MASTER);
            if (!activateChannelMaster(pRateControl)) {
                findNewMaster(group);
            }
        } else if (!m_bExplicitMasterSelected) {
            if (m_sSyncSource == kMasterSyncGroup) {
                if (playingSyncDeckCount() == 1) {
                    // We should be master now.
                    pRateControl->notifyModeChanged(SYNC_MASTER);
                    if (!activateChannelMaster(pRateControl)) {
                        findNewMaster(group);
                    }
                }
            } else {
                // If there was a deck master, set to internal clock.
                if (playingSyncDeckCount() > 1) {
                    activateInternalClockMaster();
                }
            }
        }
    } else {
        // if we were the master, choose a new one.
        if (channelIsMaster) {
            disableCurrentMaster();
        }
        pRateControl->notifyModeChanged(SYNC_NONE);
        findNewMaster("");
    }
}

void EngineSync::notifySyncModeEnabled(RateControl* pRateControl) {
    if (m_sSyncSource == "") {
        // There is no sync source.  If any other deck is playing we will match the first
        // available bpm even if sync is not enabled, although we will still be a master,
        RateControl* sync_source = NULL;
        foreach (RateControl* other_deck, m_ratecontrols) {
            if (other_deck->getGroup() == pRateControl->getGroup()) {
                continue;
            }
            ControlObject *playing = ControlObject::getControl(ConfigKey(other_deck->getGroup(),
                                                                         "play"));
            if (playing && playing->get()) {
                sync_source = other_deck;
                ControlObject *other_bpm =
                        ControlObject::getControl(ConfigKey(other_deck->getGroup(), "bpm"));
                m_pMasterRateSlider->set(other_bpm->get());
                pRateControl->notifyModeChanged(SYNC_FOLLOWER);
                break;
            }
        }

        pRateControl->notifyModeChanged(SYNC_MASTER);
        if (!activateChannelMaster(pRateControl)) {
            findNewMaster(pRateControl->getGroup());
        }
    } else {
        pRateControl->notifyModeChanged(SYNC_FOLLOWER);
        requestSyncMode(pRateControl, SYNC_FOLLOWER);
    }
}

void EngineSync::notifyDeckPlaying(RateControl* pRateControl, bool playing) {
    // For now we don't care if the deck is now playing or stopping.
    if (pRateControl->getMode() != SYNC_NONE) {
        int playing_deck_count = playingSyncDeckCount();

        if (!m_bExplicitMasterSelected) {
            if (playing_deck_count == 0) {
                if (playing) {
                    // Nothing was playing, so set self as master
                    if (activateChannelMaster(pRateControl)) {
                        pRateControl->notifyModeChanged(SYNC_MASTER);
                    }
                } else {
                    // Everything has now stopped.
                    disableCurrentMaster();
                }
            } else if (playing_deck_count == 1) {
                if (!playing && m_sSyncSource == kMasterSyncGroup) {
                    // If a deck has stopped, and only one deck is now playing,
                    // and we were internal clock, pick a new master (the playing deck).
                    findNewMaster(kMasterSyncGroup);
                }
            } else {
                activateInternalClockMaster();
            }
        }
    }
}

void EngineSync::notifyRateSliderChanged(RateControl* pRateControl, double new_bpm) {
    // Note that this is not a slot.
    if (pRateControl->getMode() != SYNC_NONE) {
        m_pMasterRateSlider->set(new_bpm);
        m_pMasterBpm->set(new_bpm);
    }
}

int EngineSync::playingSyncDeckCount() const {
    int playing_sync_decks = 0;

    foreach (const RateControl* pRateControl, m_ratecontrols) {
        double sync_mode = pRateControl->getMode();
        if (sync_mode == SYNC_NONE) {
            continue;
        }

        const ControlObject *playing = ControlObject::getControl(ConfigKey(pRateControl->getGroup(),
                                                                     "play"));
        if (playing && playing->get()) {
            ++playing_sync_decks;
        }
    }
    return playing_sync_decks;
}

void EngineSync::disableCurrentMaster() {
    RateControl* pOldChannelMaster = m_pChannelMaster;
    if (m_sSyncSource == kMasterSyncGroup) {
        m_pInternalClockMasterEnabled->set(false);
    }
    if (pOldChannelMaster) {
        ControlObject* pSourceRateEngine =
                ControlObject::getControl(ConfigKey(pOldChannelMaster->getGroup(), "rateEngine"));
        if (pSourceRateEngine) {
            disconnect(pSourceRateEngine, SIGNAL(valueChangedFromEngine(double)),
                       this, SLOT(slotSourceRateEngineChanged(double)));
        }
        ControlObject* pSourceBpm =
                ControlObject::getControl(ConfigKey(pOldChannelMaster->getGroup(), "bpm"));
        if (pSourceBpm) {
            disconnect(pSourceBpm, SIGNAL(valueChangedFromEngine(double)),
                       this, SLOT(slotSourceBpmChanged(double)));
        }
        ControlObject* pSourceBeatDistance =
                ControlObject::getControl(ConfigKey(pOldChannelMaster->getGroup(),
                                                    "beat_distance"));
        if (pSourceBeatDistance) {
            disconnect(pSourceBeatDistance, SIGNAL(valueChangedFromEngine(double)),
                       this, SLOT(slotSourceBeatDistanceChanged(double)));
        }
    }
    m_sSyncSource = "";
    m_pChannelMaster = NULL;
}

void EngineSync::activateInternalClockMaster() {
    if (m_sSyncSource == kMasterSyncGroup) {
        return;
    }
    double master_bpm = m_pMasterBpm->get();
    if (!qFuzzyCompare(master_bpm, 0)) {
        m_pMasterRateSlider->set(master_bpm);
    }
    QString old_master = m_sSyncSource;
    initializeInternalClockBeatDistance();
    RateControl* pOldChannelMaster = m_pChannelMaster;
    disableCurrentMaster();
    if (pOldChannelMaster) {
        pOldChannelMaster->notifyModeChanged(SYNC_FOLLOWER);
    }
    m_sSyncSource = kMasterSyncGroup;
    updateSamplesPerBeat();

    // This is all we have to do, we'll start using the clock position right away.
    m_pInternalClockMasterEnabled->set(true);
}

bool EngineSync::activateChannelMaster(RateControl* pRateControl) {
    if (!pRateControl) {
        return false;
    }

    const QString& group = pRateControl->getGroup();

    // Already master, no need to do anything.
    if (m_sSyncSource == group) {
        return true;
    }

    // If a channel is master, disable it.
    RateControl* pOldChannelMaster = m_pChannelMaster;
    disableCurrentMaster();
    if (pOldChannelMaster) {
        pOldChannelMaster->notifyModeChanged(SYNC_FOLLOWER);
    }

    m_sSyncSource = group;

    // Only consider channels that have a track loaded and are in the master
    // mix.
    m_pChannelMaster = pRateControl;

    qDebug() << "Setting up master " << m_sSyncSource;

    ControlObject *pSourceRateEngine =
            ControlObject::getControl(ConfigKey(pRateControl->getGroup(), "rateEngine"));
    Q_ASSERT(pSourceRateEngine);
    connect(pSourceRateEngine, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotSourceRateEngineChanged(double)),
            Qt::DirectConnection);

    ControlObject *pSourceBpm =
            ControlObject::getControl(ConfigKey(pRateControl->getGroup(), "bpm"));
    Q_ASSERT(pSourceBpm);
    connect(pSourceBpm, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotSourceBpmChanged(double)),
            Qt::DirectConnection);

    ControlObject *pSourceBeatDistance =
            ControlObject::getControl(ConfigKey(pRateControl->getGroup(), "beat_distance"));
    Q_ASSERT(pSourceBeatDistance);
    connect(pSourceBeatDistance, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotSourceBeatDistanceChanged(double)),
            Qt::DirectConnection);

    // Reset internal beat distance to equal the new master
    initializeInternalClockBeatDistance();

    m_pInternalClockMasterEnabled->set(false);
    slotSourceRateEngineChanged(pSourceRateEngine->get());
    slotSourceBpmChanged(pSourceBpm->get());

    return true;
}

void EngineSync::findNewMaster(const QString& dontpick) {
    int playing_sync_decks = 0;
    int paused_sync_decks = 0;
    RateControl *new_master = NULL;

    foreach (RateControl* pRateControl, m_ratecontrols) {
        const QString& group = pRateControl->getGroup();
        if (group == dontpick) {
            continue;
        }

        double sync_mode = pRateControl->getMode();
        if (sync_mode == SYNC_NONE) {
            continue;
        }
        if (sync_mode == SYNC_MASTER) {
            qDebug() << "Already have a new master" << group;
            m_sSyncSource = group;
            return;
        }

        ControlObject *playing = ControlObject::getControl(ConfigKey(pRateControl->getGroup(),
                                                                     "play"));
        if (playing && playing->get()) {
            ++playing_sync_decks;
            new_master = pRateControl;
        } else {
            ++paused_sync_decks;
        }
    }

    if (playing_sync_decks == 1) {
        Q_ASSERT(new_master != NULL);
        new_master->notifyModeChanged(SYNC_MASTER);
        requestSyncMode(new_master, SYNC_MASTER);
    } else if (dontpick != kMasterSyncGroup) {
        // If there are no more synced decks, there is no need for a master.
        if (playing_sync_decks + paused_sync_decks > 0) {
            activateInternalClockMaster();
        }
    } else {
        // Clock master was specifically disabled.  Just go with new_master if it exists,
        // otherwise give up and pick nothing.
        if (new_master != NULL) {
            new_master->notifyModeChanged(SYNC_MASTER);
            requestSyncMode(new_master, SYNC_MASTER);
        }
    }
    // Even if we didn't successfully find a new master, unset this value.
    m_bExplicitMasterSelected = false;
}

void EngineSync::slotSourceRateEngineChanged(double rate_engine) {
    // Master buffer can be null due to timing issues
    if (m_pChannelMaster) {
        // This will trigger all of the slaves to change rateEngine.
        m_pMasterBpm->set(rate_engine * m_pChannelMaster->getFileBpm());
    }
}

void EngineSync::slotSourceBpmChanged(double bpm) {
    // Master buffer can be null due to timing issues
    if (m_pChannelMaster) {
        m_pMasterRateSlider->set(bpm);
    }
}

void EngineSync::slotSourceBeatDistanceChanged(double beat_dist) {
    // Pass it on to slaves and update clock position marker.
    m_pMasterBeatDistance->set(beat_dist);
    setInternalClockPosition(beat_dist);
}

void EngineSync::slotSyncRateSliderChanged(double new_bpm) {
    if (m_sSyncSource == kMasterSyncGroup && !qFuzzyCompare(m_pMasterBpm->get(), new_bpm)) {
        m_pMasterBpm->set(new_bpm);
    }
}

void EngineSync::slotMasterBpmChanged(double new_bpm) {
    if (!qFuzzyCompare(new_bpm, m_pMasterBpm->get())) {
        updateSamplesPerBeat();

        // This change could hypothetically push us over distance 1.0, so check
        // XXX: is this code correct?  I think it'll work but it seems off
        if (m_dSamplesPerBeat <= 0) {
            qDebug() << "ERROR: Calculated <= 0 samples per beat which is impossible.  Forcibly "
                     << "setting to about 124bpm at 44.1Khz.";
            m_dSamplesPerBeat = 21338;
        }
        while (m_dInternalClockPosition >= m_dSamplesPerBeat) {
            m_dInternalClockPosition -= m_dSamplesPerBeat;
        }
    }
}

void EngineSync::slotSampleRateChanged(double srate) {
    int new_rate = static_cast<int>(srate);
    double internal_position = getInternalClockBeatDistance();
    // Recalculate clock buffer position based on new sample rate.
    m_dInternalClockPosition = new_rate * internal_position / m_dSamplesPerBeat;
    updateSamplesPerBeat();
}

void EngineSync::slotInternalClockModeChanged(double state) {
    if (state) {
        activateInternalClockMaster();
    } else {
        // Internal has been turned off. Pick a slave.
        m_sSyncSource = "";
        findNewMaster(kMasterSyncGroup);
    }
}

double EngineSync::getInternalClockBeatDistance() const {
    // Returns number of samples distance from the last beat.
    if (m_dInternalClockPosition < 0) {
        qDebug() << "ERROR: Internal beat distance should never be less than zero";
        return 0.0;
    }
    return m_dInternalClockPosition / m_dSamplesPerBeat;
}

void EngineSync::initializeInternalClockBeatDistance() {
    if (m_pChannelMaster) {
        initializeInternalClockBeatDistance(m_pChannelMaster);
    }
}

void EngineSync::initializeInternalClockBeatDistance(RateControl* pRateControl) {
    ControlObject* pSourceBeatDistance =
            ControlObject::getControl(ConfigKey(pRateControl->getGroup(), "beat_distance"));
    double beat_distance = pSourceBeatDistance ? pSourceBeatDistance->get() : 0;

    m_dInternalClockPosition = beat_distance * m_dSamplesPerBeat;
    m_pMasterBeatDistance->set(beat_distance);
    if (pSourceBeatDistance) {
        qDebug() << "Resetting clock beat distance to " << pRateControl->getGroup()
                 << m_dInternalClockPosition << " " << beat_distance;
    }
}

void EngineSync::updateSamplesPerBeat() {
    //to get samples per beat, do:
    //
    // samples   samples     60 seconds     minutes
    // ------- = -------  *  ----------  *  -------
    //   beat    second       1 minute       beats

    // that last term is 1 over bpm.
    double master_bpm = m_pMasterBpm->get();
    double sample_rate = m_pSampleRate->get();
    if (qFuzzyCompare(master_bpm, 0)) {
        m_dSamplesPerBeat = sample_rate;
        return;
    }
    m_dSamplesPerBeat = (sample_rate * 60.0) / master_bpm;
    if (m_dSamplesPerBeat <= 0) {
        qDebug() << "WARNING: Tried to set samples per beat <=0";
        m_dSamplesPerBeat = sample_rate;
    }
}

void EngineSync::process(int bufferSize) {
    // EngineMaster calls this function, it is used to keep track of the
    // clock (when there is no other master like a deck or MIDI) the
    // clock position is a double because we want to be precise, and beats may
    // not line up exactly with samples.

    if (m_sSyncSource != kMasterSyncGroup) {
        // We don't care, it will get set in setClockPosition.
        return;
    }

    m_dInternalClockPosition += bufferSize / 2; // stereo samples, so divide by 2

    // Can't use mod because we're in double land.
    if (m_dSamplesPerBeat <= 0) {
        qDebug() << "ERROR: Calculated <= 0 samples per beat which is impossible.  Forcibly "
                 << "setting to about 124 bpm at 44.1Khz.";
        m_dSamplesPerBeat = 21338;
    }
    while (m_dInternalClockPosition >= m_dSamplesPerBeat) {
        m_dInternalClockPosition -= m_dSamplesPerBeat;
    }

    m_pMasterBeatDistance->set(getInternalClockBeatDistance());
}

void EngineSync::setInternalClockPosition(double percent) {
    m_dInternalClockPosition = percent * m_dSamplesPerBeat;
}

EngineChannel* EngineSync::getMaster() const {
    return m_pChannelMaster ? m_pChannelMaster->getChannel() : NULL;
}

RateControl* EngineSync::getRateControlForGroup(const QString& group) {
    foreach (RateControl* pChannel, m_ratecontrols) {
        if (pChannel->getGroup() == group) {
            return pChannel;
        }
    }
    return NULL;
}
