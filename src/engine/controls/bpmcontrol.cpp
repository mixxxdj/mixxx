#include "engine/controls/bpmcontrol.h"

#include <QStringList>

#include "control/controllinpotmeter.h"
#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "control/controlpushbutton.h"
#include "engine/channels/enginechannel.h"
#include "engine/enginebuffer.h"
#include "engine/enginemaster.h"
#include "track/track.h"
#include "util/assert.h"
#include "util/duration.h"
#include "util/logger.h"
#include "util/math.h"
#include "waveform/visualplayposition.h"

namespace {
const mixxx::Logger kLogger("BpmControl");

constexpr double kBpmRangeMin = 1.0;
// TODO(XXX): Change to mixxx::Bpm::kValueMax? This would affect mappings!
constexpr double kBpmRangeMax = 200.0;
constexpr double kBpmRangeStep = 1.0;
constexpr double kBpmRangeSmallStep = 0.1;

constexpr double kBpmAdjustMin = kBpmRangeMin;
constexpr double kBpmAdjustStep = 0.01;

// Maximum allowed interval between beats (calculated from kBpmTapMin).
constexpr double kBpmTapMin = 30.0;
const mixxx::Duration kBpmTapMaxInterval = mixxx::Duration::fromMillis(
        static_cast<qint64>(1000.0 * (60.0 / kBpmTapMin)));
constexpr int kBpmTapFilterLength = 5;

// The local_bpm is calculated forward and backward this number of beats, so
// the actual number of beats is this x2.
constexpr int kLocalBpmSpan = 4;
constexpr SINT kSamplesPerFrame = 2;
}

BpmControl::BpmControl(QString group,
        UserSettingsPointer pConfig)
        : EngineControl(group, pConfig),
          m_tapFilter(this, kBpmTapFilterLength, kBpmTapMaxInterval),
          m_dSyncInstantaneousBpm(0.0),
          m_dLastSyncAdjustment(1.0),
          m_dUserTweakingSync(false) {
    m_dSyncTargetBeatDistance.setValue(0.0);
    m_dUserOffset.setValue(0.0);

    m_pPlayButton = new ControlProxy(group, "play", this);
    m_pReverseButton = new ControlProxy(group, "reverse", this);
    m_pRateRatio = new ControlProxy(group, "rate_ratio", this);
    m_pRateRatio->connectValueChanged(this, &BpmControl::slotUpdateEngineBpm,
                                      Qt::DirectConnection);

    m_pQuantize = ControlObject::getControl(group, "quantize");

    m_pPrevBeat.reset(new ControlProxy(group, "beat_prev"));
    m_pNextBeat.reset(new ControlProxy(group, "beat_next"));

    m_pLoopEnabled = new ControlProxy(group, "loop_enabled", this);
    m_pLoopStartPosition = new ControlProxy(group, "loop_start_position", this);
    m_pLoopEndPosition = new ControlProxy(group, "loop_end_position", this);

    m_pLocalBpm = new ControlObject(ConfigKey(group, "local_bpm"));
    m_pAdjustBeatsFaster = new ControlPushButton(ConfigKey(group, "beats_adjust_faster"), false);
    connect(m_pAdjustBeatsFaster, &ControlObject::valueChanged,
            this, &BpmControl::slotAdjustBeatsFaster,
            Qt::DirectConnection);
    m_pAdjustBeatsSlower = new ControlPushButton(ConfigKey(group, "beats_adjust_slower"), false);
    connect(m_pAdjustBeatsSlower, &ControlObject::valueChanged,
            this, &BpmControl::slotAdjustBeatsSlower,
            Qt::DirectConnection);
    m_pTranslateBeatsEarlier = new ControlPushButton(ConfigKey(group, "beats_translate_earlier"), false);
    connect(m_pTranslateBeatsEarlier, &ControlObject::valueChanged,
            this, &BpmControl::slotTranslateBeatsEarlier,
            Qt::DirectConnection);
    m_pTranslateBeatsLater = new ControlPushButton(ConfigKey(group, "beats_translate_later"), false);
    connect(m_pTranslateBeatsLater, &ControlObject::valueChanged,
            this, &BpmControl::slotTranslateBeatsLater,
            Qt::DirectConnection);

    // Pick a wide range (kBpmRangeMin to kBpmRangeMax) and allow out of bounds sets. This lets you
    // map a soft-takeover MIDI knob to the BPM. This also creates bpm_up and
    // bpm_down controls.
    // bpm_up / bpm_down steps by kBpmRangeStep
    // bpm_up_small / bpm_down_small steps by kBpmRangeSmallStep
    m_pEngineBpm = new ControlLinPotmeter(
            ConfigKey(group, "bpm"),
            kBpmRangeMin,
            kBpmRangeMax,
            kBpmRangeStep,
            kBpmRangeSmallStep,
            true);
    m_pEngineBpm->set(0.0);
    connect(m_pEngineBpm, &ControlObject::valueChanged,
            this, &BpmControl::slotUpdateRateSlider,
            Qt::DirectConnection);

    m_pButtonTap = new ControlPushButton(ConfigKey(group, "bpm_tap"));
    connect(m_pButtonTap, &ControlObject::valueChanged,
            this, &BpmControl::slotBpmTap,
            Qt::DirectConnection);

    // Beat sync (scale buffer tempo relative to tempo of other buffer)
    m_pButtonSync = new ControlPushButton(ConfigKey(group, "beatsync"));
    connect(m_pButtonSync, &ControlObject::valueChanged,
            this, &BpmControl::slotControlBeatSync,
            Qt::DirectConnection);

    m_pButtonSyncPhase = new ControlPushButton(ConfigKey(group, "beatsync_phase"));
    connect(m_pButtonSyncPhase, &ControlObject::valueChanged,
            this, &BpmControl::slotControlBeatSyncPhase,
            Qt::DirectConnection);

    m_pButtonSyncTempo = new ControlPushButton(ConfigKey(group, "beatsync_tempo"));
    connect(m_pButtonSyncTempo, &ControlObject::valueChanged,
            this, &BpmControl::slotControlBeatSyncTempo,
            Qt::DirectConnection);

    m_pTranslateBeats = new ControlPushButton(ConfigKey(group, "beats_translate_curpos"));
    connect(m_pTranslateBeats, &ControlObject::valueChanged,
            this, &BpmControl::slotBeatsTranslate,
            Qt::DirectConnection);

    m_pBeatsTranslateMatchAlignment = new ControlPushButton(ConfigKey(group, "beats_translate_match_alignment"));
    connect(m_pBeatsTranslateMatchAlignment, &ControlObject::valueChanged,
            this, &BpmControl::slotBeatsTranslateMatchAlignment,
            Qt::DirectConnection);

    connect(&m_tapFilter, &TapFilter::tapped,
            this, &BpmControl::slotTapFilter,
            Qt::DirectConnection);

    // Measures distance from last beat in percentage: 0.5 = half-beat away.
    m_pThisBeatDistance = new ControlProxy(group, "beat_distance", this);
    m_pSyncMode = new ControlProxy(group, "sync_mode", this);
}

BpmControl::~BpmControl() {
    delete m_pLocalBpm;
    delete m_pEngineBpm;
    delete m_pButtonTap;
    delete m_pButtonSync;
    delete m_pButtonSyncPhase;
    delete m_pButtonSyncTempo;
    delete m_pTranslateBeats;
    delete m_pBeatsTranslateMatchAlignment;
    delete m_pTranslateBeatsEarlier;
    delete m_pTranslateBeatsLater;
    delete m_pAdjustBeatsFaster;
    delete m_pAdjustBeatsSlower;
}

double BpmControl::getBpm() const {
    return m_pEngineBpm->get();
}

void BpmControl::slotAdjustBeatsFaster(double v) {
    mixxx::BeatsPointer pBeats = m_pBeats;
    if (v > 0 && pBeats && (pBeats->getCapabilities() & mixxx::Beats::BEATSCAP_SETBPM)) {
        double bpm = pBeats->getBpm();
        double adjustedBpm = bpm + kBpmAdjustStep;
        pBeats->setBpm(adjustedBpm);
    }
}

void BpmControl::slotAdjustBeatsSlower(double v) {
    mixxx::BeatsPointer pBeats = m_pBeats;
    if (v > 0 && pBeats && (pBeats->getCapabilities() & mixxx::Beats::BEATSCAP_SETBPM)) {
        double bpm = pBeats->getBpm();
        double adjustedBpm = math_max(kBpmAdjustMin, bpm - kBpmAdjustStep);
        pBeats->setBpm(adjustedBpm);
    }
}

void BpmControl::slotTranslateBeatsEarlier(double v) {
    mixxx::BeatsPointer pBeats = m_pBeats;
    if (v > 0 && pBeats &&
            (pBeats->getCapabilities() & mixxx::Beats::BEATSCAP_TRANSLATE)) {
        const double translate_dist = getSampleOfTrack().rate * -.01;
        pBeats->translate(translate_dist);
    }
}

void BpmControl::slotTranslateBeatsLater(double v) {
    mixxx::BeatsPointer pBeats = m_pBeats;
    if (v > 0 && pBeats &&
            (pBeats->getCapabilities() & mixxx::Beats::BEATSCAP_TRANSLATE)) {
        // TODO(rryan): Track::getSampleRate is possibly inaccurate!
        const double translate_dist = getSampleOfTrack().rate * .01;
        pBeats->translate(translate_dist);
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

    if (averageLength <= 0 || numSamples < 4) {
        return;
    }

    mixxx::BeatsPointer pBeats = m_pBeats;
    if (!pBeats) {
        return;
    }

    double rateRatio = m_pRateRatio->get();
    if (rateRatio == 0.0) {
        return;
    }

    // (60 seconds per minute) * (1000 milliseconds per second) / (X millis per
    // beat) = Y beats/minute
    double averageBpm = 60.0 * 1000.0 / averageLength / rateRatio;
    pBeats->setBpm(averageBpm);
}

void BpmControl::slotControlBeatSyncPhase(double value) {
    if (value == 0) {
        return;
    }

    if (isSynchronized()) {
        m_dUserOffset.setValue(0.0);
    }
    getEngineBuffer()->requestSyncPhase();
}

void BpmControl::slotControlBeatSyncTempo(double value) {
    if (value == 0) {
        return;
    }

    syncTempo();
}

void BpmControl::slotControlBeatSync(double value) {
    if (value == 0) {
        return;
    }

    if (!syncTempo()) {
        // syncTempo failed, nothing else to do
        return;
    }

    // Also sync phase if quantize is enabled.
    // this is used from controller scripts, where the latching behaviour of
    // the sync_enable CO cannot be used
    if (m_pPlayButton->toBool() && m_pQuantize->toBool()) {
        slotControlBeatSyncPhase(value);
    }
}

bool BpmControl::syncTempo() {
    if (getSyncMode() == SYNC_MASTER_EXPLICIT) {
        return false;
    }
    EngineBuffer* pOtherEngineBuffer = pickSyncTarget();

    if (!pOtherEngineBuffer) {
        return false;
    }

    double fThisBpm = m_pEngineBpm->get();
    double fThisLocalBpm = m_pLocalBpm->get();

    double fOtherBpm = pOtherEngineBuffer->getBpm();
    double fOtherLocalBpm = pOtherEngineBuffer->getLocalBpm();

    //qDebug() << "this" << "bpm" << fThisBpm << "filebpm" << fThisLocalBpm;
    //qDebug() << "other" << "bpm" << fOtherBpm << "filebpm" << fOtherLocalBpm;

    ////////////////////////////////////////////////////////////////////////////
    // Rough proof of how syncing works -- rryan 3/2011
    // ------------------------------------------------
    //
    // Let this and other denote this deck versus the sync-target deck.
    //
    // The goal is for this deck's effective BPM to equal the other decks.
    //
    // thisBpm = otherBpm
    ///
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

        double fFileBpmDelta = fabs(fThisLocalBpm - fOtherLocalBpm);
        if (fabs(fThisLocalBpm * 2.0 - fOtherLocalBpm) < fFileBpmDelta) {
            desiredRate /= 2.0;
        } else if (fabs(fThisLocalBpm - 2.0 * fOtherLocalBpm) < fFileBpmDelta) {
            desiredRate *= 2.0;
        }

        if (desiredRate < 2.0 && desiredRate > 0.5) {
            m_pEngineBpm->set(m_pLocalBpm->get() * desiredRate);
            m_pRateRatio->set(desiredRate);
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
    if (kLogger.traceEnabled()) {
        kLogger.trace() << getGroup() << "BpmControl::calcSyncedRate, tweak " << userTweak;
    }
    m_dUserTweakingSync = userTweak != 0.0;
    double rate = 1.0;
    // Don't know what to do if there's no bpm.
    if (m_pLocalBpm->toBool()) {
        rate = m_dSyncInstantaneousBpm / m_pLocalBpm->get();
    }

    // If we are not quantized, or there are no beats, or we're master,
    // or we're in reverse, just return the rate as-is.
    if (!m_pQuantize->toBool() || isMaster(getSyncMode()) ||
            !m_pBeats || m_pReverseButton->toBool()) {
        m_resetSyncAdjustment = true;
        return rate + userTweak;
    }

    const double dPrevBeat = m_pPrevBeat->get();
    const double dNextBeat = m_pNextBeat->get();

    if (dPrevBeat == -1 || dNextBeat == -1) {
        m_resetSyncAdjustment = true;
        return rate + userTweak;
    }

    double dBeatLength = dNextBeat - dPrevBeat;

    // Now that we have our beat distance we can also check how large the
    // current loop is.  If we are in a <1 beat loop, don't worry about offset.
    const bool loop_enabled = m_pLoopEnabled->toBool();
    const double loop_size = (m_pLoopEndPosition->get() -
                              m_pLoopStartPosition->get()) /
                              dBeatLength;
    if (loop_enabled && loop_size < 1.0 && loop_size > 0) {
        m_resetSyncAdjustment = true;
        return rate + userTweak;
    }

    // Now we have all we need to calculate the sync adjustment if any.
    double adjustment = calcSyncAdjustment(m_dUserTweakingSync);
    return (rate + userTweak) * adjustment;
}

double BpmControl::calcSyncAdjustment(bool userTweakingSync) {
    int resetSyncAdjustment = m_resetSyncAdjustment.fetchAndStoreRelaxed(0);
    if (resetSyncAdjustment) {
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

    double syncTargetBeatDistance = m_dSyncTargetBeatDistance.getValue();
    // We want the untweaked beat distance, so we have to add the offset here.
    double thisBeatDistance = m_pThisBeatDistance->get() + m_dUserOffset.getValue();
    double shortest_distance = shortestPercentageChange(
            syncTargetBeatDistance, thisBeatDistance);

    if (kLogger.traceEnabled()) {
        kLogger.trace() << m_group << "****************";
        kLogger.trace() << "master beat distance:" << syncTargetBeatDistance;
        kLogger.trace() << "my     beat distance:" << thisBeatDistance;
        kLogger.trace() << "user offset distance:" << m_dUserOffset.getValue();
        kLogger.trace() << "error               :"
                        << (shortest_distance - m_dUserOffset.getValue());
        kLogger.trace() << "user offset         :" << m_dUserOffset.getValue();
    }

    double adjustment = 1.0;

    if (userTweakingSync) {
        // Don't do anything else, leave it
        adjustment = 1.0;
        m_dUserOffset.setValue(shortest_distance);
    } else {
        double error = shortest_distance - m_dUserOffset.getValue();
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
    // We have to adjust our reported beat distance by the user offset to
    // preserve comparisons of beat distances.  Specifically, this beat distance
    // is used in synccontrol to update the internal clock beat distance, and if
    // we don't adjust the reported distance the track will try to adjust
    // sync against itself.
    if (kLogger.traceEnabled()) {
        kLogger.trace() << getGroup() << "BpmControl::getBeatDistance" << dThisPosition;
    }
    double dPrevBeat = m_pPrevBeat->get();
    double dNextBeat = m_pNextBeat->get();

    if (dPrevBeat == -1 || dNextBeat == -1) {
        return 0.0 - m_dUserOffset.getValue();
    }

    double dBeatLength = dNextBeat - dPrevBeat;
    double dBeatPercentage = dBeatLength == 0.0 ? 0.0 :
            (dThisPosition - dPrevBeat) / dBeatLength;
    // Because findNext and findPrev have an epsilon built in, sometimes
    // the beat percentage is out of range.  Fix it.
    if (dBeatPercentage < 0) ++dBeatPercentage;
    if (dBeatPercentage > 1) --dBeatPercentage;

    return dBeatPercentage - m_dUserOffset.getValue();
}

// static
bool BpmControl::getBeatContext(const mixxx::BeatsPointer& pBeats,
        const double dPosition,
        double* dpPrevBeat,
        double* dpNextBeat,
        double* dpBeatLength,
        double* dpBeatPercentage) {
    if (!pBeats) {
        return false;
    }

    double dPrevBeat;
    double dNextBeat;
    if (!pBeats->findPrevNextBeats(dPosition, &dPrevBeat, &dNextBeat)) {
        return false;
    }

    if (dpPrevBeat != NULL) {
        *dpPrevBeat = dPrevBeat;
    }

    if (dpNextBeat != NULL) {
        *dpNextBeat = dNextBeat;
    }

    return getBeatContextNoLookup(dPosition, dPrevBeat, dNextBeat,
                                  dpBeatLength, dpBeatPercentage);
}

// static
bool BpmControl::getBeatContextNoLookup(
                                const double dPosition,
                                const double dPrevBeat,
                                const double dNextBeat,
                                double* dpBeatLength,
                                double* dpBeatPercentage) {
    if (dPrevBeat == -1 || dNextBeat == -1) {
        return false;
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

double BpmControl::getNearestPositionInPhase(
        double dThisPosition, bool respectLoops, bool playing) {
    // Without a beatgrid, we don't know the phase offset.
    mixxx::BeatsPointer pBeats = m_pBeats;
    if (!pBeats) {
        return dThisPosition;
    }

    SyncMode syncMode = getSyncMode();

    // Explicit master buffer is always in sync!
    if (syncMode == SYNC_MASTER_EXPLICIT) {
        return dThisPosition;
    }

    // Get the current position of this deck.
    double dThisPrevBeat = m_pPrevBeat->get();
    double dThisNextBeat = m_pNextBeat->get();
    double dThisBeatLength;
    if (dThisPosition > dThisNextBeat || dThisPosition < dThisPrevBeat) {
        if (kLogger.traceEnabled()) {
            kLogger.trace() << "BpmControl::getNearestPositionInPhase out of date"
                            << dThisPosition << dThisNextBeat << dThisPrevBeat;
        }
        // This happens if dThisPosition is the target position of a requested
        // seek command
        if (!getBeatContext(pBeats, dThisPosition,
                            &dThisPrevBeat, &dThisNextBeat,
                            &dThisBeatLength, NULL)) {
            return dThisPosition;
        }
    } else {
        if (!getBeatContextNoLookup(dThisPosition,
                                    dThisPrevBeat, dThisNextBeat,
                                    &dThisBeatLength, NULL)) {
            return dThisPosition;
        }
    }

    double dOtherBeatFraction;
    if (syncMode == SYNC_FOLLOWER) {
        // If we're a follower, it's easy to get the other beat fraction
        dOtherBeatFraction = m_dSyncTargetBeatDistance.getValue();
    } else {
        // If not, we have to figure it out
        EngineBuffer* pOtherEngineBuffer = pickSyncTarget();
        if (playing) {
            if (!pOtherEngineBuffer || pOtherEngineBuffer->getSpeed() == 0.0) {
                // "this" track is playing, or just starting
                // only match phase if the sync target is playing as well
                // else use the previous phase of "this" track before the seek
                pOtherEngineBuffer = getEngineBuffer();
            }
        }

        if (!pOtherEngineBuffer) {
            // no suitable sync buffer found
            return dThisPosition;
        }

        TrackPointer otherTrack = pOtherEngineBuffer->getLoadedTrack();
        mixxx::BeatsPointer otherBeats =
                otherTrack ? otherTrack->getBeats() : mixxx::BeatsPointer();

        // If either track does not have beats, then we can't adjust the phase.
        if (!otherBeats) {
            return dThisPosition;
        }

        double dOtherPosition = pOtherEngineBuffer->getExactPlayPos();

        if (!BpmControl::getBeatContext(otherBeats,
                    dOtherPosition,
                    NULL,
                    NULL,
                    NULL,
                    &dOtherBeatFraction)) {
            return dThisPosition;
        }
    }

    bool this_near_next =
            dThisNextBeat - dThisPosition <= dThisPosition - dThisPrevBeat;
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

    double dNewPlaypos =
            (dOtherBeatFraction + m_dUserOffset.getValue()) * dThisBeatLength;
    if (this_near_next == other_near_next) {
        dNewPlaypos += dThisPrevBeat;
    } else if (this_near_next && !other_near_next) {
        dNewPlaypos += dThisNextBeat;
    } else { //!this_near_next && other_near_next
        dThisPrevBeat = pBeats->findNthBeat(dThisPosition, -2);
        dNewPlaypos += dThisPrevBeat;
    }

    if (respectLoops) {
        // We might be seeking outside the loop.
        const bool loop_enabled = m_pLoopEnabled->toBool();
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
        if (loop_enabled &&
                dThisPosition <= loop_end_position) {
            const double loop_length = loop_end_position - loop_start_position;
            const double end_delta = dNewPlaypos - loop_end_position;

            // Syncing to after the loop end.
            if (end_delta > 0 && loop_length > 0.0) {
                double i = end_delta / loop_length;
                dNewPlaypos = loop_start_position + end_delta - i * loop_length;

                // Move new position after loop jump into phase as well.
                // This is a recursive call, called only twice because of
                // respectLoops = false
                dNewPlaypos = getNearestPositionInPhase(dNewPlaypos, false, playing);
            }

            // Note: Syncing to before the loop beginning is allowed, because
            // loops are catching
        }
    }

    return dNewPlaypos;
}

double BpmControl::getBeatMatchPosition(
        double dThisPosition, bool respectLoops, bool playing) {
    // Without a beatgrid, we don't know the phase offset.
    if (!m_pBeats) {
        return dThisPosition;
    }
    // Explicit master buffer is always in sync!
    if (getSyncMode() == SYNC_MASTER_EXPLICIT) {
        return dThisPosition;
    }

    EngineBuffer* pOtherEngineBuffer = nullptr;
    // explicit master always syncs to itself, so keep it null
    if (getSyncMode() != SYNC_MASTER_EXPLICIT) {
        pOtherEngineBuffer = pickSyncTarget();
    }
    if (playing) {
        if (!pOtherEngineBuffer || pOtherEngineBuffer->getSpeed() == 0.0) {
            // "this" track is playing, or just starting
            // only match phase if the sync target is playing as well
            // else use the previous phase of "this" track before the seek
            pOtherEngineBuffer = getEngineBuffer();
        }
    } else if (!pOtherEngineBuffer) {
        return dThisPosition;
    }

    // Get the current position of this deck.
    double dThisPrevBeat = m_pPrevBeat->get();
    double dThisNextBeat = m_pNextBeat->get();
    double dThisBeatLength = -1;

    // Look up the next beat and beat length for the new position
    if (dThisNextBeat == -1 ||
            dThisPosition > dThisNextBeat ||
            (dThisPrevBeat != -1 && dThisPosition < dThisPrevBeat)) {
        if (kLogger.traceEnabled()) {
            kLogger.trace() << "BpmControl::getBeatMatchPosition out of date"
                            << dThisPrevBeat << dThisPosition << dThisNextBeat;
        }
        // This happens if dThisPosition is the target position of a requested
        // seek command.  Get new prev and next beats for the calculation.
        getBeatContext(
                m_pBeats,
                dThisPosition,
                &dThisPrevBeat,
                &dThisNextBeat,
                &dThisBeatLength,
                nullptr);
        // now we either have a useful next beat or there is none
        if (dThisNextBeat == -1) {
            // We can't match the next beat, give up.
            return dThisPosition;
        }
    } else {
        if (kLogger.traceEnabled()) {
            kLogger.trace() << "BpmControl::getBeatMatchPosition up to date"
                            << dThisPrevBeat << dThisPosition << dThisNextBeat;
        }
        // We are between the previous and next beats so we can try a standard
        // lookup of the beat length.
        getBeatContextNoLookup(
                dThisPosition,
                dThisPrevBeat,
                dThisNextBeat,
                &dThisBeatLength,
                nullptr);
    }

    TrackPointer otherTrack = pOtherEngineBuffer->getLoadedTrack();
    mixxx::BeatsPointer otherBeats = otherTrack ? otherTrack->getBeats() : mixxx::BeatsPointer();

    // If either track does not have beats, then we can't adjust the phase.
    if (!otherBeats) {
        return dThisPosition;
    }

    double dOtherPosition = pOtherEngineBuffer->getExactPlayPos();

    double dOtherPrevBeat = -1;
    double dOtherNextBeat = -1;
    double dOtherBeatLength = -1;
    double dOtherBeatFraction = -1;
    if (!BpmControl::getBeatContext(
                otherBeats,
                dOtherPosition,
                &dOtherPrevBeat,
                &dOtherNextBeat,
                &dOtherBeatLength,
                &dOtherBeatFraction)) {
        return dThisPosition;
    }

    if (dOtherBeatLength == -1 || dOtherBeatFraction == -1) {
        // the other Track has no usable beat info, do not seek.
        return dThisPosition;
    }

    double dThisSampleRate = m_pBeats->getSampleRate();
    double dThisRateRatio = m_pRateRatio->get();

    // Seek our next beat to the other next beat
    // This is the only thing we can do if the track has different BPM,
    // playing the next beat together.
    double thisDivSec = (dThisNextBeat - dThisPosition) /
            dThisSampleRate / dThisRateRatio;

    if (dOtherBeatFraction < 1.0 / 8) {
        // the user has probably pressed play too late, sync the previous beat
        dOtherBeatFraction += 1.0;
    }

    dOtherBeatFraction += m_dUserOffset.getValue();
    double otherDivSec = (1 - dOtherBeatFraction) *
            dOtherBeatLength / otherBeats->getSampleRate() / pOtherEngineBuffer->getRateRatio();

    // This matches the next beat in of both tracks.
    double seekMatch = (thisDivSec - otherDivSec) *
            dThisSampleRate * dThisRateRatio;

    if (dThisBeatLength > 0) {
        if (dThisBeatLength / 2 < seekMatch) {
            // seek to previous beat, because of shorter distance
            seekMatch -= dThisBeatLength;
        } else if (dThisBeatLength / 2 < -seekMatch) {
            // seek to beat after next, because of shorter distance
            seekMatch += dThisBeatLength;
        }
    }
    double dNewPlaypos = dThisPosition + seekMatch;

    if (respectLoops) {
        // We might be seeking outside the loop.
        const bool loop_enabled = m_pLoopEnabled->toBool();
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
        if (loop_enabled &&
                dThisPosition <= loop_end_position) {
            const double loop_length = loop_end_position - loop_start_position;
            const double end_delta = dNewPlaypos - loop_end_position;

            // Syncing to after the loop end.
            if (end_delta > 0 && loop_length > 0.0) {
                double i = end_delta / loop_length;
                dNewPlaypos = loop_start_position + end_delta - i * loop_length;

                // Move new position after loop jump into phase as well.
                // This is a recursive call, called only twice because of
                // respectLoops = false
                dNewPlaypos = getNearestPositionInPhase(dNewPlaypos, false, playing);
            }

            // Note: Syncing to before the loop beginning is allowed, because
            // loops are catching
        }
    }
    return dNewPlaypos;
}

double BpmControl::getPhaseOffset(double dThisPosition) {
    // This does not respect looping
    double dNewPlaypos = getNearestPositionInPhase(dThisPosition, false, false);
    return dNewPlaypos - dThisPosition;
}

void BpmControl::slotUpdateEngineBpm(double value) {
    Q_UNUSED(value);
    // Adjust playback bpm in response to a rate_ration update
    double dRate = m_pRateRatio->get();
    m_pEngineBpm->set(m_pLocalBpm->get() * dRate);
}

void BpmControl::slotUpdateRateSlider(double value) {
    Q_UNUSED(value);
    // Adjust rate slider position response to a change in rate range or m_pEngineBpm

    double localBpm = m_pLocalBpm->get();
    if (localBpm == 0.0) {
        return;
    }

    double dRateRatio = m_pEngineBpm->get() / localBpm;
    m_pRateRatio->set(dRateRatio);
}

void BpmControl::notifySeek(double dNewPlaypos) {
    EngineControl::notifySeek(dNewPlaypos);
    updateBeatDistance();
}

// called from an engine worker thread
void BpmControl::trackLoaded(TrackPointer pNewTrack) {
    mixxx::BeatsPointer pBeats;
    if (pNewTrack) {
        pBeats = pNewTrack->getBeats();
    }
    trackBeatsUpdated(pBeats);
}

void BpmControl::trackBeatsUpdated(mixxx::BeatsPointer pBeats) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << getGroup() << "BpmControl::trackBeatsUpdated"
                        << (pBeats ? pBeats->getBpm() : 0.0);
    }
    m_pBeats = pBeats;
    updateLocalBpm();
    resetSyncAdjustment();
}

void BpmControl::slotBeatsTranslate(double v) {
    mixxx::BeatsPointer pBeats = m_pBeats;
    if (v > 0 && pBeats && (pBeats->getCapabilities() & mixxx::Beats::BEATSCAP_TRANSLATE)) {
        double currentSample = getSampleOfTrack().current;
        double closestBeat = pBeats->findClosestBeat(currentSample);
        int delta = static_cast<int>(currentSample - closestBeat);
        if (delta % 2 != 0) {
            delta--;
        }
        pBeats->translate(delta);
    }
}

void BpmControl::slotBeatsTranslateMatchAlignment(double v) {
    mixxx::BeatsPointer pBeats = m_pBeats;
    if (v > 0 && pBeats && (pBeats->getCapabilities() & mixxx::Beats::BEATSCAP_TRANSLATE)) {
        // Must reset the user offset *before* calling getPhaseOffset(),
        // otherwise it will always return 0 if master sync is active.
        m_dUserOffset.setValue(0.0);

        double offset = getPhaseOffset(getSampleOfTrack().current);
        pBeats->translate(-offset);
    }
}

double BpmControl::updateLocalBpm() {
    double prev_local_bpm = m_pLocalBpm->get();
    double local_bpm = 0;
    mixxx::BeatsPointer pBeats = m_pBeats;
    if (pBeats) {
        local_bpm = pBeats->getBpmAroundPosition(
                getSampleOfTrack().current, kLocalBpmSpan);
        if (local_bpm == -1) {
            local_bpm = pBeats->getBpm();
        }
    }
    if (local_bpm != prev_local_bpm) {
        if (kLogger.traceEnabled()) {
            kLogger.trace() << getGroup() << "BpmControl::updateLocalBpm" << local_bpm;
        }
        m_pLocalBpm->set(local_bpm);
        slotUpdateEngineBpm();
    }
    return local_bpm;
}

double BpmControl::updateBeatDistance() {
    double beat_distance = getBeatDistance(getSampleOfTrack().current);
    m_pThisBeatDistance->set(beat_distance);
    if (!isSynchronized()) {
        m_dUserOffset.setValue(0.0);
    }
    if (kLogger.traceEnabled()) {
        kLogger.trace() << getGroup() << "BpmControl::updateBeatDistance" << beat_distance;
    }
    return beat_distance;
}

void BpmControl::setTargetBeatDistance(double beatDistance) {
    if (kLogger.traceEnabled()) {
        qDebug() << getGroup() << "BpmControl::setTargetBeatDistance:" << beatDistance;
    }
    m_dSyncTargetBeatDistance.setValue(beatDistance);
}

void BpmControl::setInstantaneousBpm(double instantaneousBpm) {
    m_dSyncInstantaneousBpm = instantaneousBpm;
}

void BpmControl::resetSyncAdjustment() {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << getGroup() << "BpmControl::resetSyncAdjustment";
    }
    // Immediately edit the beat distance to reflect the new reality.
    double new_distance = m_pThisBeatDistance->get() + m_dUserOffset.getValue();
    m_pThisBeatDistance->set(new_distance);
    m_dUserOffset.setValue(0.0);
    m_resetSyncAdjustment = true;
}

void BpmControl::collectFeatures(GroupFeatureState* pGroupFeatures) const {
    // Without a beatgrid we don't know any beat details.
    SampleOfTrack sot = getSampleOfTrack();
    if (sot.rate == 0 || !m_pBeats) {
        return;
    }

    // Get the current position of this deck.
    double dThisPrevBeat = m_pPrevBeat->get();
    double dThisNextBeat = m_pNextBeat->get();
    double dThisBeatLength;
    double dThisBeatFraction;
    if (getBeatContextNoLookup(sot.current,
            dThisPrevBeat, dThisNextBeat,
            &dThisBeatLength, &dThisBeatFraction)) {

        // Note: dThisBeatLength is fractional frames count * 2 (stereo samples)
	double sotPerSec = kSamplesPerFrame * sot.rate * m_pRateRatio->get();
        if (sotPerSec != 0.0) {
            pGroupFeatures->beat_length_sec = dThisBeatLength / sotPerSec;
            pGroupFeatures->has_beat_length_sec = true;
	} else {
            pGroupFeatures->has_beat_length_sec = false;
        }

        pGroupFeatures->has_beat_fraction = true;
        pGroupFeatures->beat_fraction = dThisBeatFraction;
    }
}

double BpmControl::getRateRatio() const {
    return m_pRateRatio->get();
}
