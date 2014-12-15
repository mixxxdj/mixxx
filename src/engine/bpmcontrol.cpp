// bpmcontrol.cpp
// Created 7/5/2009 by RJ Ryan (rryan@mit.edu)

#include <QStringList>

#include "controlobject.h"
#include "controlpushbutton.h"
#include "controllinpotmeter.h"

#include "engine/enginebuffer.h"
#include "engine/bpmcontrol.h"
#include "visualplayposition.h"
#include "engine/enginechannel.h"
#include "engine/enginemaster.h"
#include "controlobjectslave.h"
#include "util/math.h"

const int minBpm = 30;
const int maxInterval = (int)(1000.*(60./(CSAMPLE)minBpm));
const int filterLength = 5;
// The local_bpm is calculated forward and backward this number of beats, so
// the actual number of beats is this x2.
const int kLocalBpmSpan = 4;

BpmControl::BpmControl(QString group,
                       ConfigObject<ConfigValue>* _config) :
        EngineControl(group, _config),
        m_dPreviousSample(0),
        m_dSyncTargetBeatDistance(0.0),
        m_dSyncInstantaneousBpm(0.0),
        m_dLastSyncAdjustment(1.0),
        m_resetSyncAdjustment(false),
        m_dUserOffset(0.0),
        m_tapFilter(this, filterLength, maxInterval),
        m_sGroup(group) {
    m_pPlayButton = new ControlObjectSlave(group, "play", this);
    m_pPlayButton->connectValueChanged(SLOT(slotControlPlay(double)), Qt::DirectConnection);
    m_pReverseButton = new ControlObjectSlave(group, "reverse", this);
    m_pRateSlider = new ControlObjectSlave(group, "rate", this);
    m_pRateSlider->connectValueChanged(SLOT(slotAdjustRateSlider()), Qt::DirectConnection);
    m_pQuantize = ControlObject::getControl(group, "quantize");
    m_pRateRange = new ControlObjectSlave(group, "rateRange", this);
    m_pRateRange->connectValueChanged(SLOT(slotAdjustRateSlider()), Qt::DirectConnection);
    m_pRateDir = new ControlObjectSlave(group, "rate_dir", this);
    m_pRateDir->connectValueChanged(SLOT(slotAdjustRateSlider()), Qt::DirectConnection);

    m_pLoopEnabled = new ControlObjectSlave(group, "loop_enabled", this);
    m_pLoopStartPosition = new ControlObjectSlave(group, "loop_start_position", this);
    m_pLoopEndPosition = new ControlObjectSlave(group, "loop_end_position", this);

    m_pFileBpm = new ControlObject(ConfigKey(group, "file_bpm"));
    connect(m_pFileBpm, SIGNAL(valueChanged(double)),
            this, SLOT(slotFileBpmChanged(double)),
            Qt::DirectConnection);
    m_pLocalBpm = new ControlObject(ConfigKey(group, "local_bpm"));
    m_pAdjustBeatsFaster = new ControlPushButton(ConfigKey(group, "beats_adjust_faster"), false);
    connect(m_pAdjustBeatsFaster, SIGNAL(valueChanged(double)),
            this, SLOT(slotAdjustBeatsFaster(double)),
            Qt::DirectConnection);
    m_pAdjustBeatsSlower = new ControlPushButton(ConfigKey(group, "beats_adjust_slower"), false);
    connect(m_pAdjustBeatsSlower, SIGNAL(valueChanged(double)),
            this, SLOT(slotAdjustBeatsSlower(double)),
            Qt::DirectConnection);
    m_pTranslateBeatsEarlier = new ControlPushButton(ConfigKey(group, "beats_translate_earlier"), false);
    connect(m_pTranslateBeatsEarlier, SIGNAL(valueChanged(double)),
            this, SLOT(slotTranslateBeatsEarlier(double)),
            Qt::DirectConnection);
    m_pTranslateBeatsLater = new ControlPushButton(ConfigKey(group, "beats_translate_later"), false);
    connect(m_pTranslateBeatsLater, SIGNAL(valueChanged(double)),
            this, SLOT(slotTranslateBeatsLater(double)),
            Qt::DirectConnection);

    // Pick a wide range (1 to 200) and allow out of bounds sets. This lets you
    // map a soft-takeover MIDI knob to the BPM. This also creates bpm_up and
    // bpm_down controls.
    // bpm_up / bpm_down steps by 1
    // bpm_up_small / bpm_down_small steps by 0.1
    m_pEngineBpm = new ControlLinPotmeter(ConfigKey(group, "bpm"), 1, 200, 1, 0.1, true);
    connect(m_pEngineBpm, SIGNAL(valueChanged(double)),
            this, SLOT(slotSetEngineBpm(double)),
            Qt::DirectConnection);

    m_pButtonTap = new ControlPushButton(ConfigKey(group, "bpm_tap"));
    connect(m_pButtonTap, SIGNAL(valueChanged(double)),
            this, SLOT(slotBpmTap(double)),
            Qt::DirectConnection);

    // Beat sync (scale buffer tempo relative to tempo of other buffer)
    m_pButtonSync = new ControlPushButton(ConfigKey(group, "beatsync"));
    connect(m_pButtonSync, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlBeatSync(double)),
            Qt::DirectConnection);

    m_pButtonSyncPhase = new ControlPushButton(ConfigKey(group, "beatsync_phase"));
    connect(m_pButtonSyncPhase, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlBeatSyncPhase(double)),
            Qt::DirectConnection);

    m_pButtonSyncTempo = new ControlPushButton(ConfigKey(group, "beatsync_tempo"));
    connect(m_pButtonSyncTempo, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlBeatSyncTempo(double)),
            Qt::DirectConnection);

    m_pTranslateBeats = new ControlPushButton(ConfigKey(group, "beats_translate_curpos"));
    connect(m_pTranslateBeats, SIGNAL(valueChanged(double)),
            this, SLOT(slotBeatsTranslate(double)),
            Qt::DirectConnection);

    m_pBeatsTranslateMatchAlignment = new ControlPushButton(ConfigKey(group, "beats_translate_match_alignment"));
    connect(m_pBeatsTranslateMatchAlignment, SIGNAL(valueChanged(double)),
            this, SLOT(slotBeatsTranslateMatchAlignment(double)),
            Qt::DirectConnection);

    connect(&m_tapFilter, SIGNAL(tapped(double,int)),
            this, SLOT(slotTapFilter(double,int)),
            Qt::DirectConnection);

    // Measures distance from last beat in percentage: 0.5 = half-beat away.
    m_pThisBeatDistance = new ControlObjectSlave(group, "beat_distance", this);
    m_pSyncMode = ControlObject::getControl(ConfigKey(group, "sync_mode"));
}

BpmControl::~BpmControl() {
    delete m_pPlayButton;
    delete m_pRateSlider;
    delete m_pRateRange;
    delete m_pRateDir;
    delete m_pLoopEnabled;
    delete m_pLoopStartPosition;
    delete m_pLoopEndPosition;
    delete m_pFileBpm;
    delete m_pLocalBpm;
    delete m_pEngineBpm;
    delete m_pButtonTap;
    delete m_pButtonSync;
    delete m_pButtonSyncPhase;
    delete m_pButtonSyncTempo;
    delete m_pTranslateBeats;
    delete m_pTranslateBeatsEarlier;
    delete m_pTranslateBeatsLater;
    delete m_pThisBeatDistance;
    delete m_pAdjustBeatsFaster;
    delete m_pAdjustBeatsSlower;
}

double BpmControl::getBpm() const {
    return m_pEngineBpm->get();
}

void BpmControl::slotFileBpmChanged(double bpm) {
    Q_UNUSED(bpm);
    // Adjust the file-bpm with the current setting of the rate to get the
    // engine BPM. We only do this for SYNC_NONE decks because EngineSync will
    // set our BPM if the file BPM changes. See SyncControl::fileBpmChanged().
    if (m_pBeats) {
        m_pLocalBpm->set(m_pBeats->getBpmAroundPosition(getCurrentSample(),
                                                        kLocalBpmSpan));
    } else {
        m_pLocalBpm->set(bpm);
    }
    if (getSyncMode() == SYNC_NONE) {
        slotAdjustRateSlider();
    }
    resetSyncAdjustment();
}

void BpmControl::slotAdjustBeatsFaster(double v) {
    if (v > 0 && m_pBeats && (m_pBeats->getCapabilities() & Beats::BEATSCAP_SET)) {
        double new_bpm = math_min(200.0, m_pBeats->getBpm() + .01);
        m_pBeats->setBpm(new_bpm);
    }
}

void BpmControl::slotAdjustBeatsSlower(double v) {
    if (v > 0 && m_pBeats && (m_pBeats->getCapabilities() & Beats::BEATSCAP_SET)) {
        double new_bpm = math_max(10.0, m_pBeats->getBpm() - .01);
        m_pBeats->setBpm(new_bpm);
    }
}

void BpmControl::slotTranslateBeatsEarlier(double v) {
    if (v > 0 && m_pTrack && m_pBeats &&
            (m_pBeats->getCapabilities() & Beats::BEATSCAP_TRANSLATE)) {
        // TODO(rryan): TrackInfoObject::getSampleRate is possibly inaccurate!
        const int translate_dist = m_pTrack->getSampleRate() * -.01;
        m_pBeats->translate(translate_dist);
    }
}

void BpmControl::slotTranslateBeatsLater(double v) {
    if (v > 0 && m_pTrack && m_pBeats &&
            (m_pBeats->getCapabilities() & Beats::BEATSCAP_TRANSLATE)) {
        // TODO(rryan): TrackInfoObject::getSampleRate is possibly inaccurate!
        const int translate_dist = m_pTrack->getSampleRate() * .01;
        m_pBeats->translate(translate_dist);
    }
}

void BpmControl::slotSetEngineBpm(double bpm) {
    double localbpm = m_pLocalBpm->get();
    double ratedir = m_pRateDir->get();
    double raterange = m_pRateRange->get();

    if (localbpm && ratedir && raterange) {
        double newRate = bpm / localbpm;
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
    slotAdjustRateSlider();
}

void BpmControl::slotControlPlay(double v) {
    if (v > 0.0) {
        if (m_pQuantize->get() > 0.0) {
            getEngineBuffer()->requestSyncPhase();
        }
    }
}

void BpmControl::slotControlBeatSyncPhase(double v) {
    if (!v) return;
    getEngineBuffer()->requestSyncPhase();
}

void BpmControl::slotControlBeatSyncTempo(double v) {
    if (!v) return;
    syncTempo();
}

void BpmControl::slotControlBeatSync(double v) {
    if (!v) return;

    // If the player is playing, and adjusting its tempo succeeded, adjust its
    // phase so that it plays in sync.
    if (syncTempo() && m_pPlayButton->get() > 0) {
        getEngineBuffer()->requestSyncPhase();
    }
}

bool BpmControl::syncTempo() {
    EngineBuffer* pOtherEngineBuffer = pickSyncTarget();

    if (!pOtherEngineBuffer) {
        return false;
    }

    double fThisBpm = m_pEngineBpm->get();
    double fThisLocalBpm = m_pLocalBpm->get();

    double fOtherBpm = pOtherEngineBuffer->getBpm();
    double fOtherLocalBpm = pOtherEngineBuffer->getLocalBpm();

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
        double desiredRate = fOtherBpm / fThisLocalBpm;

        // Test if this buffer's bpm is the double of the other one, and adjust
        // the rate scale. I believe this is intended to account for our BPM
        // algorithm sometimes finding double or half BPMs. This avoids drastic
        // scales.

        float fFileBpmDelta = fabs(fThisLocalBpm - fOtherLocalBpm);
        if (fabs(fThisLocalBpm * 2.0 - fOtherLocalBpm) < fFileBpmDelta) {
            desiredRate /= 2.0;
        } else if (fabs(fThisLocalBpm - 2.0 * fOtherLocalBpm) < fFileBpmDelta) {
            desiredRate *= 2.0;
        }

        // Subtract the base 1.0, now fDesiredRate is the percentage
        // increase/decrease in playback rate, not the playback rate.
        double desiredRateShift = desiredRate - 1.0;

        // Ensure the rate is within resonable boundaries. Remember, this is the
        // percent to scale the rate, not the rate itself. If fDesiredRate was -1,
        // that would mean the deck would be completely stopped. If fDesiredRate
        // is 1, that means it is playing at 2x speed. This limit enforces that
        // we are scaled between 0.5x and 2x.
        if (desiredRateShift < 1.0 && desiredRateShift > -0.5)
        {
            m_pEngineBpm->set(m_pLocalBpm->get() * desiredRate);


            // Adjust the rateScale. We have to divide by the range and
            // direction to get the correct rateScale.
            double desiredRateSlider = desiredRateShift / (m_pRateRange->get() * m_pRateDir->get());
            // And finally, set the slider
            m_pRateSlider->set(desiredRateSlider);

            return true;
        }
    }
    return false;
}

// static
double BpmControl::shortestPercentageChange(const double& current_percentage,
                                            const double& target_percentage) {
    if (current_percentage == target_percentage) {
        return 0.0;
    } else if (current_percentage < target_percentage) {
        // Invariant: forwardDistance - backwardsDistance == 1.0

        // my: 0.01 target:0.99 forwards: 0.98
        // my: 0.25 target: 0.5 forwards: 0.25
        // my: 0.25 target: 0.75 forwards: 0.5
        // my: 0.98 target: 0.99 forwards: 0.01
        const double forwardDistance = target_percentage - current_percentage;

        // my: 0.01 target:0.99 backwards: -0.02
        // my: 0.25 target: 0.5 backwards: -0.75
        // my: 0.25 target: 0.75 backwards: -0.5
        // my: 0.98 target: 0.99 backwards: -0.99
        const double backwardsDistance = target_percentage - current_percentage - 1.0;

        return (fabs(forwardDistance) < fabs(backwardsDistance)) ?
                forwardDistance : backwardsDistance;
    } else { // current_percentage > target_percentage
        // Invariant: forwardDistance - backwardsDistance == 1.0

        // my: 0.99 target: 0.01 forwards: 0.02
        const double forwardDistance = 1.0 - current_percentage + target_percentage;

        // my: 0.99 target:0.01 backwards: -0.98
        const double backwardsDistance = target_percentage - current_percentage;

        return (fabs(forwardDistance) < fabs(backwardsDistance)) ?
                forwardDistance : backwardsDistance;
    }
}

double BpmControl::calcSyncedRate(double userTweak) {
    double rate = 1.0;
    // Don't know what to do if there's no bpm.
    if (m_pLocalBpm->get() != 0.0) {
        rate = m_dSyncInstantaneousBpm / m_pLocalBpm->get();
    }

    // If we are not quantized, or there are no beats, or we're master,
    // or we're in reverse, just return the rate as-is.
    if (!m_pQuantize->get() || getSyncMode() == SYNC_MASTER ||
            m_pBeats == NULL || m_pReverseButton->get()) {
        m_resetSyncAdjustment = true;
        return rate + userTweak;
    }

    // Now we need to get our beat distance so we can figure out how
    // out of phase we are.
    double dThisPosition = getCurrentSample();
    double dBeatLength;
    double my_percentage;

    if (!BpmControl::getBeatContext(m_pBeats, dThisPosition,
                                    NULL, NULL,
                                    &dBeatLength, &my_percentage, 0.01)) {
        m_resetSyncAdjustment = true;
        return rate + userTweak;
    }

    // Now that we have our beat distance we can also check how large the
    // current loop is.  If we are in a <1 beat loop, don't worry about offset.
    const bool loop_enabled = m_pLoopEnabled->get() > 0.0;
    const double loop_size = (m_pLoopEndPosition->get() -
                              m_pLoopStartPosition->get()) /
                              dBeatLength;
    if (loop_enabled && loop_size < 1.0 && loop_size > 0) {
        m_resetSyncAdjustment = true;
        return rate + userTweak;
    }

    // Now we have all we need to calculate the sync adjustment if any.
    double adjustment = calcSyncAdjustment(my_percentage, userTweak != 0.0);
    return (rate + userTweak) * adjustment;
}

double BpmControl::calcSyncAdjustment(double my_percentage, bool userTweakingSync) {
    if (m_resetSyncAdjustment) {
        m_resetSyncAdjustment = false;
        m_dLastSyncAdjustment = 1.0;
    }

    // Either shortest distance is directly to the master or backwards.

    // TODO(rryan): This is kind of backwards because we are measuring distance
    // from master to my percentage. All of the control code below is based on
    // this point of reference so I left it this way but I think we should think
    // about things in terms of "my percentage-offset setpoint" that the control
    // loop should aim to maintain.
    // TODO(rryan): All of this code is based on the assumption that a track
    // can't pass through multiple beats in one engine callback. Instead our
    // setpoint should be tracking the true offset in "samples traveled" rather
    // than modular 1.0 beat fractions. This will allow sync to work across loop
    // boundaries too.

    double master_percentage = m_dSyncTargetBeatDistance;
    double shortest_distance = shortestPercentageChange(
        master_percentage, my_percentage);

    /*qDebug() << m_sGroup << m_dUserOffset;
    qDebug() << "master beat distance:" << master_percentage;
    qDebug() << "my     beat distance:" << my_percentage;
    qDebug() << "error               :" << (shortest_distance - m_dUserOffset);
    qDebug() << "user offset         :" << m_dUserOffset;*/

    double adjustment = 1.0;

    if (userTweakingSync) {
        // Don't do anything else, leave it
        adjustment = 1.0;
        m_dUserOffset = shortest_distance;
    } else {
        double error = shortest_distance - m_dUserOffset;
        // Threshold above which we do sync adjustment.
        const double kErrorThreshold = 0.01;
        // Threshold above which sync is really, really bad, so much so that we
        // don't even know if we're ahead or behind.  This can occur when quantize was
        // off, but then it gets turned on.
        const double kTrainWreckThreshold = 0.2;
        const double kSyncAdjustmentCap = 0.05;
        if (fabs(error) > kTrainWreckThreshold) {
            // Assume poor reflexes (late button push) -- speed up to catch the other track.
            adjustment = 1.0 + kSyncAdjustmentCap;
        } else if (fabs(error) > kErrorThreshold) {
            // Proportional control constant. The higher this is, the more we
            // influence sync.
            const double kSyncAdjustmentProportional = 0.7;
            const double kSyncDeltaCap = 0.02;

            // TODO(owilliams): There are a lot of "1.0"s in this code -- can we eliminate them?
            const double adjust = 1.0 + (-error * kSyncAdjustmentProportional);
            // Cap the difference between the last adjustment and this one.
            double delta = adjust - m_dLastSyncAdjustment;
            delta = math_clamp(delta, -kSyncDeltaCap, kSyncDeltaCap);

            // Cap the adjustment between -kSyncAdjustmentCap and +kSyncAdjustmentCap
            adjustment = 1.0 + math_clamp(
                    m_dLastSyncAdjustment - 1.0 + delta,
                    -kSyncAdjustmentCap, kSyncAdjustmentCap);
        } else {
            // We are in sync, no adjustment needed.
            adjustment = 1.0;
        }
    }
    m_dLastSyncAdjustment = adjustment;
    return adjustment;
}

double BpmControl::getBeatDistance(double dThisPosition) const {
    double dBeatPercentage;
    // We have to adjust our reported beat distance by the user offset to
    // preserve comparisons of beat distances.  Specifically, this beat distance
    // is used in synccontrol to update the internal clock beat distance, and if
    // we don't adjust the reported distance the track will try to adjust
    // sync against itself.
    if (BpmControl::getBeatContext(m_pBeats, dThisPosition, NULL, NULL, NULL,
                                   &dBeatPercentage, 0.01)) {
        return dBeatPercentage - m_dUserOffset;
    }

    // if (getSyncMode() != SYNC_NONE && m_pQuantize->get()) {
    //     qWarning() << getGroup() << "No beatgrid but sync and quantize enabled.";
    // }
    return 0.0 - m_dUserOffset;
}

// static
bool BpmControl::getBeatContext(const BeatsPointer& pBeats,
                                const double dPosition,
                                double* dpPrevBeat,
                                double* dpNextBeat,
                                double* dpBeatLength,
                                double* dpBeatPercentage,
                                const double beatEpsilon) {
    if (!pBeats) {
        return false;
    }

    double dPrevBeat = pBeats->findPrevBeat(dPosition);
    double dNextBeat = pBeats->findNextBeat(dPosition);

    if (dPrevBeat == -1 || dNextBeat == -1) {
        return false;
    }

    if (fabs(dPrevBeat - dNextBeat) <= beatEpsilon) {
        dNextBeat = pBeats->findNthBeat(dPosition, 2);
    }

    if (dNextBeat == -1) {
        return false;
    }

    if (dpPrevBeat != NULL) {
        *dpPrevBeat = dPrevBeat;
    }

    if (dpNextBeat != NULL) {
        *dpNextBeat = dNextBeat;
    }

    double dBeatLength = dNextBeat - dPrevBeat;
    if (dpBeatLength != NULL) {
        *dpBeatLength = dBeatLength;
    }

    if (dpBeatPercentage != NULL) {
        *dpBeatPercentage = dBeatLength == 0.0 ? 0.0 :
                (dPosition - dPrevBeat) / dBeatLength;
        // Because findNext and findPrev have an epsilon built in, sometimes
        // the beat percentage is out of range.  Fix it.
        if (*dpBeatPercentage < 0) ++*dpBeatPercentage;
        if (*dpBeatPercentage > 1) --*dpBeatPercentage;
    }

    return true;
}

double BpmControl::getPhaseOffset(double dThisPosition) {
    // Without a beatgrid, we don't know the phase offset.
    if (!m_pBeats) {
        return 0;
    }
    // Master buffer is always in sync!
    if (getSyncMode() == SYNC_MASTER) {
        return 0;
    }

    // Get the current position of this deck.
    double dThisPrevBeat;
    double dThisNextBeat;
    double dThisBeatLength;
    if (!getBeatContext(m_pBeats, dThisPosition,
                        &dThisPrevBeat, &dThisNextBeat,
                        &dThisBeatLength, NULL)) {
        return 0;
    }

    double dOtherBeatFraction;
    if (getSyncMode() == SYNC_FOLLOWER) {
        // If we're a slave, it's easy to get the other beat fraction
        dOtherBeatFraction = m_dSyncTargetBeatDistance;
    } else {
        // If not, we have to figure it out
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
        double dOtherEnginePlayPos = pOtherEngineBuffer->getVisualPlayPos();
        double dOtherPosition = dOtherLength * dOtherEnginePlayPos;

        if (!BpmControl::getBeatContext(otherBeats, dOtherPosition,
                                        NULL, NULL, NULL, &dOtherBeatFraction)) {
            return 0.0;
        }
    }

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

    double dNewPlaypos = (dOtherBeatFraction + m_dUserOffset) * dThisBeatLength;
    if (this_near_next == other_near_next) {
        dNewPlaypos += dThisPrevBeat;
    } else if (this_near_next && !other_near_next) {
        dNewPlaypos += dThisNextBeat;
    } else {  //!this_near_next && other_near_next
        dThisPrevBeat = m_pBeats->findNthBeat(dThisPosition, -2);
        dNewPlaypos += dThisPrevBeat;
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

void BpmControl::slotAdjustRateSlider() {
    // Adjust playback bpm in response to a change in the rate slider.
    double dRate = 1.0 + m_pRateDir->get() * m_pRateRange->get() * m_pRateSlider->get();
    m_pEngineBpm->set(m_pLocalBpm->get() * dRate);
}

void BpmControl::trackLoaded(TrackPointer pTrack) {
    if (m_pTrack) {
        trackUnloaded(m_pTrack);
    }

    // reset for new track
    resetSyncAdjustment();

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
        m_pTrack.clear();
        m_pBeats.clear();
    }
    m_dUserOffset = 0.0;
    m_dLastSyncAdjustment = 1.0;
}

void BpmControl::slotUpdatedTrackBeats()
{
    if (m_pTrack) {
        resetSyncAdjustment();
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

void BpmControl::slotBeatsTranslateMatchAlignment(double v) {
    if (v > 0 && m_pBeats && (m_pBeats->getCapabilities() & Beats::BEATSCAP_TRANSLATE)) {
        // Must reset the user offset *before* calling getPhaseOffset(),
        // otherwise it will always return 0 if master sync is active.
        m_dUserOffset = 0.0;

        double offset = getPhaseOffset(getCurrentSample());
        m_pBeats->translate(-offset);
    }
}

void BpmControl::setCurrentSample(const double dCurrentSample, const double dTotalSamples) {
    m_dPreviousSample = dCurrentSample;
    EngineControl::setCurrentSample(dCurrentSample, dTotalSamples);
}

double BpmControl::process(const double dRate,
                           const double dCurrentSample,
                           const double dTotalSamples,
                           const int iBufferSize) {
    Q_UNUSED(dRate);
    Q_UNUSED(dCurrentSample);
    Q_UNUSED(dTotalSamples);
    Q_UNUSED(iBufferSize);
    return kNoTrigger;
}

double BpmControl::updateLocalBpm() {
    double prev_local_bpm = m_pLocalBpm->get();
    double local_bpm = 0;
    if (m_pBeats) {
        local_bpm = m_pBeats->getBpmAroundPosition(getCurrentSample(),
                                                   kLocalBpmSpan);
    } else {
        local_bpm = m_pFileBpm->get();
    }
    if (local_bpm != prev_local_bpm) {
        m_pLocalBpm->set(local_bpm);
        slotAdjustRateSlider();
    }
    return local_bpm;
}

double BpmControl::updateBeatDistance() {
    double beat_distance = getBeatDistance(m_dPreviousSample);
    m_pThisBeatDistance->set(beat_distance);
    if (getSyncMode() == SYNC_NONE) {
        m_dUserOffset = 0.0;
    }
    return beat_distance;
}

void BpmControl::setTargetBeatDistance(double beatDistance) {
    m_dSyncTargetBeatDistance = beatDistance;
}

void BpmControl::setInstantaneousBpm(double instantaneousBpm) {
    m_dSyncInstantaneousBpm = instantaneousBpm;
}

void BpmControl::resetSyncAdjustment() {
    // Immediately edit the beat distance to reflect the new reality.
    double new_distance = m_pThisBeatDistance->get() + m_dUserOffset;
    m_pThisBeatDistance->set(new_distance);
    m_dUserOffset = 0.0;
    m_resetSyncAdjustment = true;
}

void BpmControl::collectFeatures(GroupFeatureState* pGroupFeatures) const {
    double fileBpm = m_pFileBpm->get();
    if (fileBpm > 0) {
        pGroupFeatures->has_file_bpm = true;
        pGroupFeatures->file_bpm = fileBpm;
    }

    double bpm = m_pEngineBpm->get();
    if (bpm > 0) {
        pGroupFeatures->has_bpm = true;
        pGroupFeatures->bpm = bpm;
    }

    // Without a beatgrid we don't know any beat details.
    if (!m_pBeats) {
        return;
    }

    // Get the current position of this deck.
    double dThisPosition = getCurrentSample();
    double dThisPrevBeat;
    double dThisNextBeat;
    double dThisBeatLength;
    double dThisBeatFraction;
    if (getBeatContext(m_pBeats, dThisPosition,
                       &dThisPrevBeat, &dThisNextBeat,
                       &dThisBeatLength, &dThisBeatFraction)) {
        pGroupFeatures->has_prev_beat = true;
        pGroupFeatures->prev_beat = dThisPrevBeat;

        pGroupFeatures->has_next_beat = true;
        pGroupFeatures->next_beat = dThisNextBeat;

        pGroupFeatures->has_beat_length = true;
        pGroupFeatures->beat_length = dThisBeatLength;

        pGroupFeatures->has_beat_fraction = true;
        pGroupFeatures->beat_fraction = dThisBeatFraction;
    }
}
