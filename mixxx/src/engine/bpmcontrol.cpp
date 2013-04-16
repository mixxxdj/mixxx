// bpmcontrol.cpp
// Created 7/5/2009 by RJ Ryan (rryan@mit.edu)

#include <QStringList>

#include "controlobject.h"
#include "controlpushbutton.h"

#include "engine/enginebuffer.h"
#include "engine/bpmcontrol.h"
#include "engine/enginechannel.h"
#include "engine/enginemaster.h"

const int minBpm = 30;
const int maxInterval = (int)(1000.*(60./(CSAMPLE)minBpm));
const int filterLength = 5;

BpmControl::BpmControl(const char* _group,
                       ConfigObject<ConfigValue>* _config) :
        EngineControl(_group, _config),
        m_tapFilter(this, filterLength, maxInterval) {
    m_pNumDecks = ControlObject::getControl(ConfigKey("[Master]", "num_decks"));

    m_pPlayButton = ControlObject::getControl(ConfigKey(_group, "play"));
    m_pRateSlider = ControlObject::getControl(ConfigKey(_group, "rate"));
    connect(m_pRateSlider, SIGNAL(valueChanged(double)),
            this, SLOT(slotAdjustBpm()),
            Qt::DirectConnection);
    connect(m_pRateSlider, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotAdjustBpm()),
            Qt::DirectConnection);

    m_pRateRange = ControlObject::getControl(ConfigKey(_group, "rateRange"));
    connect(m_pRateRange, SIGNAL(valueChanged(double)),
            this, SLOT(slotAdjustBpm()),
            Qt::DirectConnection);
    connect(m_pRateRange, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotAdjustBpm()),
            Qt::DirectConnection);

    m_pRateDir = ControlObject::getControl(ConfigKey(_group, "rate_dir"));
    connect(m_pRateDir, SIGNAL(valueChanged(double)),
            this, SLOT(slotAdjustBpm()),
            Qt::DirectConnection);
    connect(m_pRateDir, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotAdjustBpm()),
            Qt::DirectConnection);

    m_pLoopEnabled = ControlObject::getControl(
        ConfigKey(_group, "loop_enabled"));
    m_pLoopStartPosition = ControlObject::getControl(
        ConfigKey(_group, "loop_start_position"));
    m_pLoopEndPosition = ControlObject::getControl(
        ConfigKey(_group, "loop_end_position"));

    m_pFileBpm = new ControlObject(ConfigKey(_group, "file_bpm"));
    connect(m_pFileBpm, SIGNAL(valueChanged(double)),
            this, SLOT(slotAdjustBpm()),
            Qt::DirectConnection);

    m_pEngineBpm = new ControlObject(ConfigKey(_group, "bpm"));
    connect(m_pEngineBpm, SIGNAL(valueChanged(double)),
            this, SLOT(slotSetEngineBpm(double)),
            Qt::DirectConnection);

    m_pButtonTap = new ControlPushButton(ConfigKey(_group, "bpm_tap"));
    connect(m_pButtonTap, SIGNAL(valueChanged(double)),
            this, SLOT(slotBpmTap(double)),
            Qt::DirectConnection);

    // Beat sync (scale buffer tempo relative to tempo of other buffer)
    m_pButtonSync = new ControlPushButton(ConfigKey(_group, "beatsync"));
    connect(m_pButtonSync, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlBeatSync(double)),
            Qt::DirectConnection);

    m_pButtonSyncPhase = new ControlPushButton(ConfigKey(_group, "beatsync_phase"));
    connect(m_pButtonSyncPhase, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlBeatSyncPhase(double)),
            Qt::DirectConnection);

    m_pButtonSyncTempo = new ControlPushButton(ConfigKey(_group, "beatsync_tempo"));
    connect(m_pButtonSyncTempo, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlBeatSyncTempo(double)),
            Qt::DirectConnection);

    m_pTranslateBeats = new ControlPushButton(
        ConfigKey(_group, "beats_translate_curpos"));
    connect(m_pTranslateBeats, SIGNAL(valueChanged(double)),
            this, SLOT(slotBeatsTranslate(double)),
            Qt::DirectConnection);

    connect(&m_tapFilter, SIGNAL(tapped(double,int)),
            this, SLOT(slotTapFilter(double,int)),
            Qt::DirectConnection);
}

BpmControl::~BpmControl() {
    delete m_pEngineBpm;
    delete m_pFileBpm;
    delete m_pButtonSync;
    delete m_pButtonSyncTempo;
    delete m_pButtonSyncPhase;
    delete m_pButtonTap;
    delete m_pTranslateBeats;
}

double BpmControl::getBpm() {
    return m_pEngineBpm->get();
}

double BpmControl::getFileBpm() {
    return m_pFileBpm->get();
}

void BpmControl::slotSetEngineBpm(double bpm) {
    double filebpm = m_pFileBpm->get();
    double ratedir = m_pRateDir->get();
    double raterange = m_pRateRange->get();

    if (filebpm && ratedir && raterange) {
        double newRate = bpm / filebpm;
        m_pRateSlider->set((newRate - 1.0) / ratedir / raterange);
    }
}

void BpmControl::slotBpmTap(double v) {
    if (v > 0) {
        m_tapFilter.tap();
    }
}

void BpmControl::slotTapFilter(double averageLength, int numSamples) {
    // averageLength is the average interval in milliseconds tapped over
    // numSamples samples.  Have to convert to BPM now:

    if (averageLength <= 0)
        return;

    if (numSamples < 4)
        return;

    // (60 seconds per minute) * (1000 milliseconds per second) / (X millis per
    // beat) = Y beats/minute
    double averageBpm = 60.0 * 1000.0 / averageLength;
    double dRate = 1.0 + m_pRateDir->get() * m_pRateRange->get() * m_pRateSlider->get();
    m_pFileBpm->set(averageBpm / dRate);
    slotAdjustBpm(); 
}

void BpmControl::slotControlBeatSyncPhase(double v) {
    if (!v)
        return;
    EngineBuffer* pOtherEngineBuffer = pickSyncTarget();
    syncPhase(pOtherEngineBuffer);
}

void BpmControl::slotControlBeatSyncTempo(double v) {
    if (!v)
        return;
    EngineBuffer* pOtherEngineBuffer = pickSyncTarget();
    syncTempo(pOtherEngineBuffer);
}

void BpmControl::slotControlBeatSync(double v) {
    if (!v)
        return;

    // If the player is playing, and adjusting its tempo succeeded, adjust its
    // phase so that it plays in sync.
    EngineBuffer* pOtherEngineBuffer = pickSyncTarget();
    if (syncTempo(pOtherEngineBuffer) && m_pPlayButton->get() > 0) {
        syncPhase(pOtherEngineBuffer);
    }
}

bool BpmControl::syncTempo(EngineBuffer* pOtherEngineBuffer) {
    if (!pOtherEngineBuffer) {
        return false;
    }

    double fThisBpm  = m_pEngineBpm->get();
    double fThisFileBpm = m_pFileBpm->get();

    double fOtherBpm = pOtherEngineBuffer->getBpm();
    double fOtherFileBpm = pOtherEngineBuffer->getFileBpm();

    //qDebug() << "this" << "bpm" << fThisBpm << "filebpm" << fThisFileBpm;
    //qDebug() << "other" << "bpm" << fOtherBpm << "filebpm" << fOtherFileBpm;

    ////////////////////////////////////////////////////////////////////////////
    // Rough proof of how syncing works -- rryan 3/2011
    // ------------------------------------------------
    //
    // Let this and other denote this deck versus the sync-target deck.
    //
    // The goal is for this deck's effective BPM to equal the other decks.
    //
    // thisBpm = otherBpm
    //
    // The overall rate is the product of range, direction, and scale plus 1:
    //
    // rate = 1.0 + rateDir * rateRange * rateScale
    //
    // An effective BPM is the file-bpm times the rate:
    //
    // bpm = fileBpm * rate
    //
    // So our goal is to tweak thisRate such that this equation is true:
    //
    // thisFileBpm * (1.0 + thisRate) = otherFileBpm * (1.0 + otherRate)
    //
    // so rearrange this equation in terms of thisRate:
    //
    // thisRate = (otherFileBpm * (1.0 + otherRate)) / thisFileBpm - 1.0
    //
    // So the new rateScale to set is:
    //
    // thisRateScale = ((otherFileBpm * (1.0 + otherRate)) / thisFileBpm - 1.0) / (thisRateDir * thisRateRange)

    if (fOtherBpm > 0.0 && fThisBpm > 0.0) {
        // The desired rate is the other decks effective rate divided by this
        // deck's file BPM. This gives us the playback rate that will produce an
        // effective BPM equivalent to the other decks.
        double fDesiredRate = fOtherBpm / fThisFileBpm;

        // Test if this buffer's bpm is the double of the other one, and adjust
        // the rate scale. I believe this is intended to account for our BPM
        // algorithm sometimes finding double or half BPMs. This avoids drastic
        // scales.

        float fFileBpmDelta = fabs(fThisFileBpm - fOtherFileBpm);
        if (fabs(fThisFileBpm * 2.0 - fOtherFileBpm) < fFileBpmDelta) {
            fDesiredRate /= 2.0;
        } else if (fabs(fThisFileBpm - 2.0 * fOtherFileBpm) < fFileBpmDelta) {
            fDesiredRate *= 2.0;
        }

        // Subtract the base 1.0, now fDesiredRate is the percentage
        // increase/decrease in playback rate, not the playback rate.
        fDesiredRate -= 1.0;

        // Ensure the rate is within resonable boundaries. Remember, this is the
        // percent to scale the rate, not the rate itself. If fDesiredRate was -1,
        // that would mean the deck would be completely stopped. If fDesiredRate
        // is 1, that means it is playing at 2x speed. This limit enforces that
        // we are scaled between 0.5x and 2x.
        if (fDesiredRate < 1.0 && fDesiredRate > -0.5)
        {
            // Adjust the rateScale. We have to divide by the range and
            // direction to get the correct rateScale.
            fDesiredRate = fDesiredRate / (m_pRateRange->get() * m_pRateDir->get());

            // And finally, set the slider
            m_pRateSlider->set(fDesiredRate);
            return true;
        }
    }
    return false;
}

EngineBuffer* BpmControl::pickSyncTarget() {
    EngineMaster* pMaster = getEngineMaster();
    if (!pMaster) {
        return NULL;
    }
    QString group = getGroup();
    QStringList deckGroups;
    EngineBuffer* pFirstNonplayingDeck = NULL;

    for (int i = 0; i < m_pNumDecks->get(); ++i) {
        // TODO(XXX) format from PlayerManager
        QString deckGroup = QString("[Channel%1]").arg(i+1);
        if (deckGroup == group) {
            continue;
        }
        EngineChannel* pChannel = pMaster->getChannel(deckGroup);
        // Only consider channels that have a track loaded and are in the master
        // mix.
        if (pChannel && pChannel->isActive() && pChannel->isMaster()) {
            EngineBuffer* pBuffer = pChannel->getEngineBuffer();
            if (pBuffer && pBuffer->getBpm() > 0) {
                // If the deck is playing then go with it immediately.
                if (fabs(pBuffer->getRate()) > 0) {
                    return pBuffer;
                }
                // Otherwise hold out for a deck that might be playing but
                // remember the first deck that matched our criteria.
                if (pFirstNonplayingDeck == NULL) {
                    pFirstNonplayingDeck = pBuffer;
                }
            }
        }
    }
    // No playing decks have a BPM. Go with the first deck that was stopped but
    // had a BPM.
    return pFirstNonplayingDeck;
}

bool BpmControl::syncPhase(EngineBuffer* pOtherEngineBuffer) {
    if (!pOtherEngineBuffer) {
        return false;
    }
    TrackPointer otherTrack = pOtherEngineBuffer->getLoadedTrack();
    BeatsPointer otherBeats = otherTrack ? otherTrack->getBeats() : BeatsPointer();

    // If either track does not have beats, then we can't adjust the phase.
    if (!m_pBeats || !otherBeats) {
        return false;
    }

    // Get the file BPM of each song.
    //double dThisBpm = m_pBeats->getBpm();
    //double dOtherBpm = ControlObject::getControl(
    //ConfigKey(pOtherEngineBuffer->getGroup(), "file_bpm"))->get();

    // Get the current position of both decks
    double dThisPosition = getCurrentSample();
    double dOtherLength = ControlObject::getControl(
        ConfigKey(pOtherEngineBuffer->getGroup(), "track_samples"))->get();
    double dOtherEnginePlayPos = ControlObject::getControl(
        ConfigKey(pOtherEngineBuffer->getGroup(), "visual_playposition"))->get();
    double dOtherPosition = dOtherLength * dOtherEnginePlayPos;

    double dThisPrevBeat = m_pBeats->findPrevBeat(dThisPosition);
    double dThisNextBeat = m_pBeats->findNextBeat(dThisPosition);

    if (dThisPrevBeat == -1 || dThisNextBeat == -1 ||
            dOtherEnginePlayPos == -1 || dOtherLength == 0) {
        return false;
    }

    // Protect against the case where we are sitting exactly on the beat.
    if (dThisPrevBeat == dThisNextBeat) {
        dThisNextBeat = m_pBeats->findNthBeat(dThisPosition, 2);
    }

    double dOtherPrevBeat = otherBeats->findPrevBeat(dOtherPosition);
    double dOtherNextBeat = otherBeats->findNextBeat(dOtherPosition);

    if (dOtherPrevBeat == -1 || dOtherNextBeat == -1) {
        return false;
    }

    // Protect against the case where we are sitting exactly on the beat.
    if (dOtherPrevBeat == dOtherNextBeat) {
        dOtherNextBeat = otherBeats->findNthBeat(dOtherPosition, 2);
    }

    double dThisBeatLength = fabs(dThisNextBeat - dThisPrevBeat);
    double dOtherBeatLength = fabs(dOtherNextBeat - dOtherPrevBeat);
    double dOtherBeatFraction = (dOtherPosition - dOtherPrevBeat) / dOtherBeatLength;

    double dNewPlaypos;
    bool this_near_next = dThisNextBeat - dThisPosition <= dThisPosition - dThisPrevBeat;
    bool other_near_next = dOtherNextBeat - dOtherPosition <= dOtherPosition - dOtherPrevBeat;

    // We want our beat fraction to be identical to theirs.

    // If the two tracks have similar alignment, adjust phase is straight-
    // forward.  Use the same fraction for both beats, starting from the previous
    // beat.  But if This track is nearer to the next beat and the Other track
    // is nearer to the previous beat, use This Next beat as the starting point
    // for the phase. (ie, we pushed the sync button late).  If This track
    // is nearer to the previous beat, but the Other track is nearer to the
    // next beat, we pushed the sync button early so use the double-previous
    // beat as the basis for the adjustment.
    //
    // This makes way more sense when you're actually mixing.
    //
    // TODO(XXX) Revisit this logic once we move away from tempo-locked,
    // infinite beatgrids because the assumption that findNthBeat(-2) always
    // works will be wrong then.

    if (this_near_next == other_near_next) {
        dNewPlaypos = dThisPrevBeat + dOtherBeatFraction * dThisBeatLength;
    } else if (this_near_next && !other_near_next) {
        dNewPlaypos = dThisNextBeat + dOtherBeatFraction * dThisBeatLength;
    } else {  //!this_near_next && other_near_next
        dThisPrevBeat = m_pBeats->findNthBeat(dThisPosition, -2);
        dNewPlaypos = dThisPrevBeat + dOtherBeatFraction * dThisBeatLength;
    }

    // We might be seeking outside the loop.
    const bool loop_enabled = m_pLoopEnabled->get() > 0.0;
    const double loop_start_position = m_pLoopStartPosition->get();
    const double loop_end_position = m_pLoopEndPosition->get();

    // Cases for sanity:
    //
    // CASE 1
    // Two identical 1-beat loops, out of phase by X samples.
    // Other deck is at its loop start.
    // This deck is half way through. We want to jump forward X samples to the loop end point.
    //
    // Two identical 1-beat loop, out of phase by X samples.
    // Other deck is

    // If sync target is 50% through the beat,
    // If we are at the loop end point and hit sync, jump forward X samples.


    // TODO(rryan): Revise this with something that keeps a broader number of
    // cases in sync. This at least prevents breaking out of the loop.
    if (loop_enabled) {
        const double loop_length = loop_end_position - loop_start_position;
        if (loop_length <= 0.0) {
            return false;
        }

        // TODO(rryan): If loop_length is not a multiple of dThisBeatLength should
        // we bail and not sync phase?

        // Syncing to after the loop end.
        double end_delta = dNewPlaypos - loop_end_position;
        if (end_delta > 0) {
            int i = end_delta / loop_length;
            dNewPlaypos = loop_start_position + end_delta - i * loop_length;
        }

        // Syncing to before the loop beginning.
        double start_delta = loop_start_position - dNewPlaypos;
        if (start_delta > 0) {
            int i = start_delta / loop_length;
            dNewPlaypos = loop_end_position - start_delta + i * loop_length;
        }
    }

    emit(seekAbs(dNewPlaypos));
    return true;
}

void BpmControl::slotAdjustBpm() {
    // Emitted value is one of the control objects used below

    //qDebug() << this << "slotAdjustBpm"
    // Adjust the file-bpm with the current setting of the rate to get the
    // engine BPM.
    double dRate = 1.0 + m_pRateDir->get() * m_pRateRange->get() * m_pRateSlider->get();
    m_pEngineBpm->set(m_pFileBpm->get() * dRate);
}

void BpmControl::trackLoaded(TrackPointer pTrack) {
    if (m_pTrack) {
        trackUnloaded(m_pTrack);
    }

    if (pTrack) {
        m_pTrack = pTrack;
        m_pBeats = m_pTrack->getBeats();
        connect(m_pTrack.data(), SIGNAL(beatsUpdated()),
                this, SLOT(slotUpdatedTrackBeats()));
    }
}

void BpmControl::trackUnloaded(TrackPointer pTrack) {
    Q_UNUSED(pTrack);
    if (m_pTrack) {
        disconnect(m_pTrack.data(), SIGNAL(beatsUpdated()),
                   this, SLOT(slotUpdatedTrackBeats()));
    }
    m_pTrack.clear();
    m_pBeats.clear();
}

void BpmControl::slotUpdatedTrackBeats()
{
    if (m_pTrack) {
        m_pBeats = m_pTrack->getBeats();
    }
}

void BpmControl::slotBeatsTranslate(double v) {
    if (v > 0 && m_pBeats && (m_pBeats->getCapabilities() & Beats::BEATSCAP_TRANSLATE)) {
        double currentSample = getCurrentSample();
        double closestBeat = m_pBeats->findClosestBeat(currentSample);
        int delta = currentSample - closestBeat;
        if (delta % 2 != 0) {
            delta--;
        }
        m_pBeats->translate(delta);
    }
}
