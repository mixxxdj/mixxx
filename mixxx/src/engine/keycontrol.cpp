// bpmcontrol.cpp
// Created 7/5/2009 by RJ Ryan (rryan@mit.edu)

#include "controlobject.h"
#include "controlpushbutton.h"

#include "engine/enginebuffer.h"
#include "engine/keycontrol.h"
#include "controlpotmeter.h"

#include <QDebug>

const int minBpm = 30;
const int maxInterval = (int)(1000.*(60./(CSAMPLE)minBpm));
const int filterLength = 5;

KeyControl::KeyControl(const char* _group,
                       ConfigObject<ConfigValue>* _config) :
        EngineControl(_group, _config){
       // m_tapFilter(this, filterLength, maxInterval) {
    //m_pPlayButton = ControlObject::getControl(ConfigKey(_group, "play"));
   // m_pRateSlider = ControlObject::getControl(ConfigKey(_group, "rate"));
    m_pRateSlider = new ControlPotmeter(ConfigKey(_group, "keyrate"), -1.f, 1.f);
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

    m_pFileKey = new ControlObject(ConfigKey(_group, "file_key"));
    connect(m_pFileKey, SIGNAL(valueChanged(double)),
            this, SLOT(slotFileKeyChanged(double)),
            Qt::DirectConnection);

    m_pEngineKey = new ControlObject(ConfigKey(_group, "key"));
    connect(m_pEngineKey, SIGNAL(valueChanged(double)),
            this, SLOT(slotSetEngineKey(double)),
            Qt::DirectConnection);

    /*m_pButtonTap = new ControlPushButton(ConfigKey(_group, "bpm_tap"));
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
            Qt::DirectConnection);*/
}

KeyControl::~KeyControl() {
    delete m_pEngineKey;
    delete m_pFileKey;
    /*delete m_pButtonSync;
    delete m_pButtonSyncTempo;
    delete m_pButtonSyncPhase;
    delete m_pButtonTap;
    delete m_pTranslateBeats;*/
}

double KeyControl::getKey() {
    return m_pEngineKey->get();
    //return m_pKey;
}

void KeyControl::slotFileKeyChanged(double key) {
    //qDebug() << this << "slotFileBpmChanged" << bpm;
    // Adjust the file-bpm with the current setting of the rate to get the
    // engine BPM.
    double dRate = 1.0 + m_pRateDir->get() * m_pRateRange->get() * m_pRateSlider->get();
    m_pEngineKey->set(key * dRate);
    //qDebug()<<"1";
    //qDebug()<<m_pRateDir->get()<<" "<<m_pRateRange->get()<<" "<<m_pRateSlider->get();
    //qDebug()<<"2";
    //m_pEngineKey->set(key);
}

double KeyControl::getRawRate() {
    return m_pRateSlider->get() * m_pRateRange->get() * m_pRateDir->get();
}

void KeyControl::slotSetEngineKey(double key) {
    double filekey = m_pFileKey->get();
    //qDebug()<<"3";
    if (filekey != 0.0) {
        double newRate = key / filekey - 1.0f;
        newRate = math_max(-1.0f, math_min(1.0f, newRate));
        m_pRateSlider->set(newRate * m_pRateDir->get());
      //  qDebug()<<"4";
    }
}

/*void KeyControl::slotBpmTap(double v) {
    if (v > 0) {
        m_tapFilter.tap();
    }
}*/

/*void KeyControl::slotTapFilter(double averageLength, int numSamples) {
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
    slotFileBpmChanged(averageBpm);
}*/

/*void KeyControl::slotControlBeatSyncPhase(double v) {
    if (!v)
        return;
    syncPhase();
}

void KeyControl::slotControlBeatSyncTempo(double v) {
    if (!v)
        return;
    syncTempo();
}

void KeyControl::slotControlBeatSync(double v) {
    if (!v)
        return;

    // If the player is playing, and adjusting its tempo succeeded, adjust its
    // phase so that it plays in sync.
    if (syncTempo() && m_pPlayButton->get() > 0) {
        syncPhase();
    }
}

bool KeyControl::syncTempo() {
    EngineBuffer* pOtherEngineBuffer = getOtherEngineBuffer();

    if(!pOtherEngineBuffer)
        return false;

    double fThisKey  = m_pEngineKey->get();
    //double fThisRate = m_pRateDir->get() * m_pRateSlider->get() * m_pRateRange->get();
    double fThisFileKey = m_pFileKey->get();

    double fOtherKey = pOtherEngineBuffer->getKey();
    double fOtherRate = pOtherEngineBuffer->getRate();
    double fOtherFileKey = fOtherKey / (1.0 + fOtherRate);

    //qDebug() << "this" << "bpm" << fThisBpm << "filebpm" << fThisFileBpm << "rate" << fThisRate;
    //qDebug() << "other" << "bpm" << fOtherBpm << "filebpm" << fOtherFileBpm << "rate" << fOtherRate;

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

    if (fOtherKey > 0.0 && fThisKey > 0.0) {
        // The desired rate is the other decks effective rate divided by this
        // deck's file BPM. This gives us the playback rate that will produe an
        // effective BPM equivalent to the other decks.
        double fDesiredRate = fOtherKey / fThisFileKey;

        // Test if this buffer's bpm is the double of the other one, and adjust
        // the rate scale. I believe this is intended to account for our BPM
        // algorithm sometimes finding double or half BPMs. This avoids drastic
        // scales.

        float fFileKeyDelta = fabs(fThisFileKey-fOtherFileKey);
        if (fabs(fThisFileKey*2.0 - fOtherFileKey) < fFileKeyDelta) {
            fDesiredRate /= 2.0;
        } else if (fabs(fThisFileKey - 2.0*fOtherFileKey) < fFileKeyDelta) {
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

bool BpmControl::syncPhase() {
    EngineBuffer* pOtherEngineBuffer = getOtherEngineBuffer();
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
    double dOtherPosition = dOtherLength * ControlObject::getControl(
        ConfigKey(pOtherEngineBuffer->getGroup(), "visual_playposition"))->get();

    double dThisPrevBeat = m_pBeats->findPrevBeat(dThisPosition);
    double dThisNextBeat = m_pBeats->findNextBeat(dThisPosition);

    if (dThisPrevBeat == -1 || dThisNextBeat == -1) {
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

    emit(seekAbs(dNewPlaypos));
    return true;
}*/

void KeyControl::slotRateChanged(double) {
    double dFileKey = m_pFileKey->get();
    slotFileKeyChanged(dFileKey);
}

void KeyControl::trackLoaded(TrackPointer pTrack) {
    if (m_pTrack) {
        trackUnloaded(m_pTrack);
    }

    if (pTrack) {
        m_pTrack = pTrack;
        m_pKey = m_pTrack->getKey();
       // qDebug()<<m_pKey;
        connect(m_pTrack.data(), SIGNAL(keyUpdated()),
                this, SLOT(slotUpdatedTrackKey()));
    }
}

void KeyControl::trackUnloaded(TrackPointer pTrack) {
    if (m_pTrack) {
        disconnect(m_pTrack.data(), SIGNAL(keyUpdated()),
                   this, SLOT(slotUpdatedTrackKey()));
    }
    m_pTrack.clear();
    m_pKey=0;
}

void KeyControl::slotUpdatedTrackKey()
{
    if (m_pTrack) {
        m_pKey = m_pTrack->getKey();
        //qDebug()<<m_pKey;
    }
}

/*void KeyControl::slotBeatsTranslate(double v) {
    if (v > 0 && m_pBeats && (m_pBeats->getCapabilities() & Beats::BEATSCAP_TRANSLATE)) {
        double currentSample = getCurrentSample();
        double closestBeat = m_pBeats->findClosestBeat(currentSample);
        int delta = currentSample - closestBeat;
        if (delta % 2 != 0) {
            delta--;
        }
        m_pBeats->translate(delta);
    }
}*/

double KeyControl::convertKey(QString dValue)
{
    double key=0;
    if(dValue == "C") key=1;
    else if(dValue == "C#")key=2;
    else if(dValue == "D")key=3;
    else if(dValue == "D#")key=4;
    else if(dValue == "E")key=5;
    else if(dValue == "F")key=6;
    else if(dValue == "F#")key=7;
    else if(dValue == "G")key=8;
    else if(dValue == "G#")key=9;
    else if(dValue == "A")key=10;
    else if(dValue == "A#")key=11;
    else if(dValue == "B")key=12;
    else if(dValue == "c")key=13;
    else if(dValue == "c#")key=14;
    else if(dValue == "d")key=15;
    else if(dValue == "d#")key=16;
    else if(dValue == "e")key=17;
    else if(dValue == "f")key=18;
    else if(dValue == "f#")key=19;
    else if(dValue == "g")key=20;
    else if(dValue == "g#")key=21;
    else if(dValue == "a")key=22;
    else if(dValue == "a#")key=23;
    else if(dValue == "b")key=24;
    return key;
}

