// bpmcontrol.cpp
// Created 7/5/2009 by RJ Ryan (rryan@mit.edu)

#include <QList>
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
        //m_pMasterSync(NULL),
        m_iSyncState(0),
        m_dSyncAdjustment(0.0),
        m_bUserTweakingSync(false),
        m_dUserOffset(0.0),
        m_tapFilter(this, filterLength, maxInterval),
        m_sGroup(_group) {
    m_pNumDecks = ControlObject::getControl(ConfigKey("[Master]", "num_decks"));

    m_pPlayButton = ControlObject::getControl(ConfigKey(_group, "play"));
    connect(m_pPlayButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlPlay(double)),
            Qt::DirectConnection);
            
    m_pQuantize = ControlObject::getControl(ConfigKey(_group, "quantize"));

    
    m_pRateSlider = ControlObject::getControl(ConfigKey(_group, "rate"));
    connect(m_pRateSlider, SIGNAL(valueChanged(double)),
            this, SLOT(slotRateChanged(double)),
            Qt::DirectConnection);
    connect(m_pRateSlider, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotRateChanged(double)),
            Qt::DirectConnection);

    m_pRateRange = ControlObject::getControl(ConfigKey(_group, "rateRange"));
    connect(m_pRateRange, SIGNAL(valueChanged(double)),
            this, SLOT(slotRateChanged(double)),
            Qt::DirectConnection);
    connect(m_pRateRange, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotRateChanged(double)),
            Qt::DirectConnection);

    m_pRateDir = ControlObject::getControl(ConfigKey(_group, "rate_dir"));
    connect(m_pRateDir, SIGNAL(valueChanged(double)),
            this, SLOT(slotRateChanged(double)),
            Qt::DirectConnection);
    connect(m_pRateDir, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotRateChanged(double)),
            Qt::DirectConnection);

    m_pLoopEnabled = ControlObject::getControl(
        ConfigKey(_group, "loop_enabled"));
    m_pLoopStartPosition = ControlObject::getControl(
        ConfigKey(_group, "loop_start_position"));
    m_pLoopEndPosition = ControlObject::getControl(
        ConfigKey(_group, "loop_end_position"));

    m_pFileBpm = new ControlObject(ConfigKey(_group, "file_bpm"));
    connect(m_pFileBpm, SIGNAL(valueChanged(double)),
            this, SLOT(slotFileBpmChanged(double)),
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
            
    m_pMasterBeatDistance = ControlObject::getControl(ConfigKey("[Master]","beat_distance"));
    connect(m_pMasterBeatDistance, SIGNAL(valueChangedFromEngine(double)),
                this, SLOT(slotMasterBeatDistanceChanged(double)),
                Qt::DirectConnection);
                
    m_pSyncMasterEnabled = ControlObject::getControl(ConfigKey(_group, "sync_master"));
    connect(m_pSyncMasterEnabled, SIGNAL(valueChanged(double)),
                this, SLOT(slotSyncMasterChanged(double)),
                Qt::DirectConnection);
                
    m_pSyncSlaveEnabled = ControlObject::getControl(ConfigKey(_group, "sync_slave"));
    connect(m_pSyncSlaveEnabled, SIGNAL(valueChanged(double)),
                this, SLOT(slotSyncSlaveChanged(double)),
                Qt::DirectConnection);
                
    m_pSyncMasterEnabled->set(FALSE);
    m_pSyncSlaveEnabled->set(FALSE);
    m_iSyncState = SYNC_NONE;
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
    return m_dFileBpm;
}

void BpmControl::slotFileBpmChanged(double bpm) {
    //qDebug() << this << "slotFileBpmChanged" << bpm;
    // Adjust the file-bpm with the current setting of the rate to get the
    // engine BPM.
    double dRate = 1.0 + m_pRateDir->get() * m_pRateRange->get() * m_pRateSlider->get();
    m_pEngineBpm->set(bpm * dRate);
    m_dFileBpm = bpm;
}

void BpmControl::slotSetEngineBpm(double bpm) {
    double filebpm = m_pFileBpm->get();

    if (filebpm != 0.0) {
        double newRate = bpm / filebpm - 1.0f;
        newRate = math_max(-1.0f, math_min(1.0f, newRate));
        m_dFileBpm = newRate;
        m_pRateSlider->set(newRate * m_pRateDir->get());
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
    m_pFileBpm->set(averageBpm);
    m_dFileBpm = averageBpm;
    slotFileBpmChanged(averageBpm);
}

void BpmControl::slotControlPlay(double v)
{
    if (v > 0.0)
    {
        if (m_pQuantize->get() > 0.0) 
        {
            qDebug() << m_sGroup << "we are quantizing so sync phase on play";
            syncPhase();
        }
    }
}

void BpmControl::slotControlBeatSyncPhase(double v) {
    if (!v)
        return;
    syncPhase();
}

void BpmControl::slotControlBeatSyncTempo(double v) {
    if (!v)
        return;
    syncTempo();
}

void BpmControl::slotControlBeatSync(double v) {
    if (!v)
        return;

    // If the player is playing, and adjusting its tempo succeeded, adjust its
    // phase so that it plays in sync.
    if (syncTempo() && m_pPlayButton->get() > 0) {
        syncPhase();
    }
}

void BpmControl::slotSyncMasterChanged(double state)
{
    if (state) {
        m_iSyncState = SYNC_MASTER;    
    } else {
        // For now, turning off master turns on slave mode
        m_iSyncState = SYNC_SLAVE;
    }
}

void BpmControl::slotSyncSlaveChanged(double state)
{
    if (state) {
        m_iSyncState = SYNC_SLAVE;    
    } else {
        // For now, turning off slave turns off syncing
        m_iSyncState = SYNC_NONE;
    }
}

bool BpmControl::syncTempo() {
    EngineBuffer* pOtherEngineBuffer = pickSyncTarget();

    if(!pOtherEngineBuffer) {
        return false;
    }

    double fThisBpm = m_pEngineBpm->get();
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
        // deck's file BPM. This gives us the playback rate that will produe an
        // effective BPM equivalent to the other decks.
        double fDesiredRate = fOtherBpm / fThisFileBpm;

        // Test if this buffer's bpm is the double of the other one, and adjust
        // the rate scale. I believe this is intended to account for our BPM
        // algorithm sometimes finding double or half BPMs. This avoids drastic
        // scales.

        float fFileBpmDelta = fabs(fThisFileBpm-fOtherFileBpm);
        if (fabs(fThisFileBpm*2.0 - fOtherFileBpm) < fFileBpmDelta) {
            fDesiredRate /= 2.0;
        } else if (fabs(fThisFileBpm - 2.0*fOtherFileBpm) < fFileBpmDelta) {
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
            fDesiredRate = fDesiredRate/(m_pRateRange->get() * m_pRateDir->get());

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

void BpmControl::userTweakingSync(bool tweakActive)
{
    //TODO XXX: this might be one loop off.  ie, user tweaks but we've already
    //calculated a new rate.  Then next time we pay attention to the tweak.
    //I think it might not matter though
    m_bUserTweakingSync = tweakActive;
}

void BpmControl::slotMasterBeatDistanceChanged(double master_distance)
{
    if (m_iSyncState != SYNC_SLAVE)
        return;
    
    if (m_pBeats == NULL)
    {
        //qDebug() << "null here too";
        return;
    }
    
    const double MAGIC_FUZZ = 0.01;
    const double MAGIC_FACTOR = 0.3; //the higher this is, the more we influence sync
    
    if (!m_pPlayButton->get()) {
        return;
    }
    
    //If we aren't quantized, don't worry about offset
    if (!m_pQuantize->get()) {
        m_dSyncAdjustment = 0;
        return;
    }
    
    
    double dThisPosition = getCurrentSample();
    
    double dPrevBeat = m_pBeats->findPrevBeat(dThisPosition); 
    double dNextBeat = m_pBeats->findNextBeat(dThisPosition);
    double beat_length = dNextBeat - dPrevBeat;
    if (fabs(beat_length) < 0.01)
    {
        //we are on a beat
        dNextBeat = m_pBeats->findNthBeat(dThisPosition, 2);
        beat_length = dNextBeat - dPrevBeat;
    }
    
    // my_distance is our percentage distance through the beat
    double my_distance = (dThisPosition - dPrevBeat) / beat_length;
    //double user_offset_percent = m_dUserOffset / beat_length;
    
    if (my_distance - m_dUserOffset < 0)
    {
        my_distance += 1.0;
    }
    
    // beat wraparound -- any other way to account for this?
    // if we're at .99% and the master is 0.1%, we are *not* 98% off!
    
    ////don't do anything if we're at the edges of the beat (wraparound issues)
    if (my_distance < 0.1 || my_distance > 0.9 ||
        master_distance < 0.1 || master_distance > 0.9)
            return;
    
    //XXX doublecheck math for applying the user offset here (almost certainly wrong)
    /*if (my_distance - m_dUserOffset > 0.9 && master_distance < 0.1)
    {
        qDebug() << "master wraparound";
        master_distance += 1.0;
    }
    else if (my_distance - m_dUserOffset < 0.1 && master_distance > 0.9)
    {
        my_distance += 1.0;
    }*/
    
    //e.g. if my position is .8, and theirs is .6
    //then sample_offset = beatlength * .8 - beatlength * .6
    //or, beatlength * (myposition - theirpercent)
    
    double percent_offset = my_distance - master_distance;
    double sample_offset = beat_length * percent_offset;
    
    /*qDebug() << "master beat distance:" << master_distance;
    qDebug() << "my     beat distance:" << my_distance;
    qDebug() << m_sGroup << sample_offset << m_dUserOffset;*/
    
    m_dSyncAdjustment = 0.0;
    
/*    if (m_dUserOffset != 0)
        qDebug() << m_sGroup << "tweak necessary?" << fabs(percent_offset - m_dUserOffset) << "offset val:" << m_dUserOffset << "cur offset" << percent_offset;
    else
        qDebug() << m_sGroup << "nouser tweak necessary?" << fabs(percent_offset - m_dUserOffset);*/
    
    if (m_bUserTweakingSync)
    {
        qDebug() << "user is tweaking sync, let's use their value" << sample_offset;
        m_dUserOffset = percent_offset;
        //don't do anything else, leave it
    } 
    else
    {
        if (fabs(percent_offset - m_dUserOffset) > MAGIC_FUZZ)
        {
            double error = percent_offset - m_dUserOffset;
            //qDebug() << "tweak to get back in sync" << percent_offset << m_dUserOffset << MAGIC_FUZZ;
            //qDebug() << "master" << master_distance << "mine" << my_distance << "diff" << percent_offset;
            m_dSyncAdjustment = (0 - error) * MAGIC_FACTOR;
            //qDebug() << m_sGroup << "tweaking...." << m_dSyncAdjustment;
            m_dSyncAdjustment = math_max(-0.2f, math_min(0.2f, m_dSyncAdjustment));
            //qDebug() << "clamped" << m_dSyncAdjustment;
        }
    }
}

double BpmControl::getSyncAdjustment()
{
    if (m_iSyncState != SYNC_SLAVE)
        return 0.0;
/*    if (m_dSyncAdjustment != 0)
        qDebug() << m_sGroup << "sync value" << m_dSyncAdjustment;*/
    return m_dSyncAdjustment;
}

double BpmControl::getBeatDistance()
{
    // returns absolute number of samples distance from current pos back to
    // previous beat
    if (m_pBeats == NULL)
    {
        //qDebug() << "no beats, returning 0";
        return 0;
    }
    double dThisPosition = getCurrentSample();
    double dPrevBeat = m_pBeats->findPrevBeat(dThisPosition); 
    double dNextBeat = m_pBeats->findNextBeat(dThisPosition);
    if (fabs(dNextBeat - dPrevBeat) < 0.01)
    {
        //we are on a beat
        return 0;
    }
    return (dThisPosition - dPrevBeat) / (dNextBeat - dPrevBeat);
}

bool BpmControl::syncPhase()
{
    double dThisPosition = getCurrentSample();
    double offset = getPhaseOffset();
    if (offset == 0.0)
    {
        qDebug() << m_sGroup << "got offset of zero so no sync";
        return false;
    }
        
    double dNewPlaypos = dThisPosition + offset;
    emit(seekAbs(dNewPlaypos));
    return true;
}

double BpmControl::getPhaseOffset()
{
    double dThisPosition = getCurrentSample();
    return getPhaseOffset(dThisPosition);
}

//when enginebuffer is seeking it wants the offset from the new position,
//not the current position
double BpmControl::getPhaseOffset(double reference_position)
{
    double dOtherBeatFraction;
    
    if (m_iSyncState == SYNC_SLAVE)
    {
        //if we're a slave, easy to get the other beat fraction
        dOtherBeatFraction = m_pMasterBeatDistance->get();
    }
    else
    {
        //if not, we have to figure it out
        EngineBuffer* pOtherEngineBuffer = pickSyncTarget();
        if (pOtherEngineBuffer == NULL) {
            return 0;
        }
            
        TrackPointer otherTrack = pOtherEngineBuffer->getLoadedTrack();
        BeatsPointer otherBeats = otherTrack ? otherTrack->getBeats() : BeatsPointer();

        // If either track does not have beats, then we can't adjust the phase.
        if (!otherBeats) {
            return 0;
        }
        
        double dOtherLength = ControlObject::getControl(
            ConfigKey(pOtherEngineBuffer->getGroup(), "track_samples"))->get();
        double dOtherPosition = dOtherLength * ControlObject::getControl(
            ConfigKey(pOtherEngineBuffer->getGroup(), "visual_playposition"))->get();

        double dOtherPrevBeat = otherBeats->findPrevBeat(dOtherPosition);
        double dOtherNextBeat = otherBeats->findNextBeat(dOtherPosition);

        if (dOtherPrevBeat == -1 || dOtherNextBeat == -1) {
            return 0;
        }

        // Protect against the case where we are sitting exactly on the beat.
        if (dOtherPrevBeat == dOtherNextBeat) {
            dOtherNextBeat = otherBeats->findNthBeat(dOtherPosition, 2);
        }
        
        double dOtherBeatLength = fabs(dOtherNextBeat - dOtherPrevBeat);
        dOtherBeatFraction = (dOtherPosition - dOtherPrevBeat) / dOtherBeatLength;
    }
    
     if (!m_pBeats) {
        return 0;
    }
    
    // Get the current position of both decks
    //qDebug() << m_sGroup << "starting with reference" << reference_position << "and adjusting" << m_dUserOffset;
    double dThisPosition = reference_position;
    double dThisPrevBeat = m_pBeats->findPrevBeat(dThisPosition);
    double dThisNextBeat = m_pBeats->findNextBeat(dThisPosition);

    if (dThisPrevBeat == -1 || dThisNextBeat == -1) {
        return 0;
    }

    // Protect against the case where we are sitting exactly on the beat.
    if (dThisPrevBeat == dThisNextBeat) {
        dThisNextBeat = m_pBeats->findNthBeat(dThisPosition, 2);
    }
    
    //qDebug() << "pseudo pos" << dThisPosition << "from ref" << reference_position << "and offset" << m_dUserOffset;
    
    double dThisBeatLength = fabs(dThisNextBeat - dThisPrevBeat);

    double dNewPlaypos;
    bool this_near_next = dThisNextBeat - dThisPosition <= dThisPosition - dThisPrevBeat;
    bool other_near_next = dOtherBeatFraction >= 0.5;

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
        dNewPlaypos = dThisPrevBeat + (dOtherBeatFraction + m_dUserOffset) * dThisBeatLength;
    } else if (this_near_next && !other_near_next) {
        dNewPlaypos = dThisNextBeat + (dOtherBeatFraction + m_dUserOffset) * dThisBeatLength;
    } else {  //!this_near_next && other_near_next
        dThisPrevBeat = m_pBeats->findNthBeat(dThisPosition, -2);
        dNewPlaypos = dThisPrevBeat + (dOtherBeatFraction + m_dUserOffset) * dThisBeatLength;
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

    return dNewPlaypos - dThisPosition;
}

void BpmControl::slotRateChanged(double) {
    //wait, always???  Are we hammering this??
    double dFileBpm = m_pFileBpm->get();
    slotFileBpmChanged(dFileBpm);
    m_dFileBpm = dFileBpm;
}

void BpmControl::trackLoaded(TrackPointer pTrack) {
    if (m_pTrack) {
        trackUnloaded(m_pTrack);
    }
    
    qDebug() << m_sGroup << "resetting user offset";
    m_dUserOffset = 0.0; //reset for new track

    if (pTrack) {
        m_pTrack = pTrack;
        m_pBeats = m_pTrack->getBeats();
        connect(m_pTrack.data(), SIGNAL(beatsUpdated()),
                this, SLOT(slotUpdatedTrackBeats()));
    }
}

void BpmControl::trackUnloaded(TrackPointer pTrack) {
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
