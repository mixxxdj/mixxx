#include "engine/controls/bpmcontrol.h"

#include "control/controlencoder.h"
#include "control/controllinpotmeter.h"
#include "control/controlproxy.h"
#include "control/controlpushbutton.h"
#include "engine/effects/groupfeaturestate.h"
#include "engine/enginebuffer.h"
#include "moc_bpmcontrol.cpp"
#include "track/beatutils.h"
#include "track/track.h"
#include "util/duration.h"
#include "util/logger.h"
#include "util/math.h"

namespace {
const mixxx::Logger kLogger("BpmControl");

constexpr double kBpmRangeMin = 1.0;
// TODO(XXX): Change to mixxx::Bpm::kValueMax? This would affect mappings!
constexpr double kBpmRangeMax = 200.0;
constexpr double kBpmRangeStep = 1.0;
constexpr double kBpmRangeSmallStep = 0.1;

constexpr double kBpmAdjustMin = kBpmRangeMin;
constexpr double kBpmAdjustStep = 0.01;
constexpr double kBpmTapRounding = 1 / 12.0;

// Maximum allowed interval between beats (calculated from kBpmTapMin).
constexpr double kBpmTapMin = 30.0;
const mixxx::Duration kBpmTapMaxInterval = mixxx::Duration::fromMillis(
        static_cast<qint64>(1000.0 * (60.0 / kBpmTapMin)));
constexpr int kBpmTapFilterLength = 80;

// The local_bpm is calculated forward and backward this number of beats, so
// the actual number of beats is this x2.
constexpr int kLocalBpmSpan = 4;

// If we are 1 / 8.0 beat fraction near the previous beat we match that instead
// of the next beat.
constexpr double kPastBeatMatchThreshold = 1 / 8.0;

mixxx::Bpm averageBpmRoundedWithinRange(double averageLength, double rateRatio) {
    // (60 seconds per minute) * (1000 milliseconds per second) /
    //   (X millis per beat)
    // = Y beats/minute
    auto averageBpm = mixxx::Bpm(60.0 * 1000.0 / averageLength / rateRatio);
    averageBpm = BeatUtils::roundBpmWithinRange(averageBpm - kBpmTapRounding,
            averageBpm,
            averageBpm + kBpmTapRounding);
    return averageBpm;
}

} // namespace

BpmControl::BpmControl(const QString& group,
        UserSettingsPointer pConfig)
        : EngineControl(group, pConfig),
          m_pReverseButton(group, QStringLiteral("reverse")),
          m_pQuantize(group, QStringLiteral("quantize")),
          m_pNextBeat(group, QStringLiteral("beat_next")),
          m_pPrevBeat(group, QStringLiteral("beat_prev")),
          m_pLoopEnabled(group, QStringLiteral("loop_enabled")),
          m_pLoopStartPosition(group, QStringLiteral("loop_start_position")),
          m_pLoopEndPosition(group, QStringLiteral("loop_end_position")),
          // Measures distance from last beat in percentage: 0.5 = half-beat away.
          m_pThisBeatDistance(group, QStringLiteral("beat_distance")),
          m_pSyncMode(group, QStringLiteral("sync_mode")),
          m_bpmTapFilter(this, kBpmTapFilterLength, kBpmTapMaxInterval),
          m_tempoTapFilter(this, kBpmTapFilterLength, kBpmTapMaxInterval),
          m_dSyncInstantaneousBpm(0.0),
          m_dLastSyncAdjustment(1.0) {
    m_dSyncTargetBeatDistance.setValue(0.0);
    m_dUserOffset.setValue(0.0);

    m_pRateRatio = std::make_unique<ControlProxy>(group, "rate_ratio", this);
    m_pRateRatio->connectValueChanged(this, &BpmControl::slotUpdateEngineBpm,
                                      Qt::DirectConnection);

    m_pLocalBpm = std::make_unique<ControlObject>(ConfigKey(group, "local_bpm"));
    m_pAdjustBeatsFaster = std::make_unique<ControlPushButton>(
            ConfigKey(group, "beats_adjust_faster"), false);
    m_pAdjustBeatsFaster->setKbdRepeatable(true);
    connect(m_pAdjustBeatsFaster.get(),
            &ControlObject::valueChanged,
            this,
            &BpmControl::slotAdjustBeatsFaster,
            Qt::DirectConnection);
    m_pAdjustBeatsSlower = std::make_unique<ControlPushButton>(
            ConfigKey(group, "beats_adjust_slower"), false);
    m_pAdjustBeatsSlower->setKbdRepeatable(true);
    connect(m_pAdjustBeatsSlower.get(),
            &ControlObject::valueChanged,
            this,
            &BpmControl::slotAdjustBeatsSlower,
            Qt::DirectConnection);
    m_pTranslateBeatsEarlier = std::make_unique<ControlPushButton>(
            ConfigKey(group, "beats_translate_earlier"), false);
    m_pTranslateBeatsEarlier->setKbdRepeatable(true);
    connect(m_pTranslateBeatsEarlier.get(),
            &ControlObject::valueChanged,
            this,
            &BpmControl::slotTranslateBeatsEarlier,
            Qt::DirectConnection);
    m_pTranslateBeatsLater = std::make_unique<ControlPushButton>(
            ConfigKey(group, "beats_translate_later"), false);
    m_pTranslateBeatsLater->setKbdRepeatable(true);
    connect(m_pTranslateBeatsLater.get(),
            &ControlObject::valueChanged,
            this,
            &BpmControl::slotTranslateBeatsLater,
            Qt::DirectConnection);
    m_pTranslateBeatsMove = std::make_unique<ControlEncoder>(
            ConfigKey(group, "beats_translate_move"), false);
    connect(m_pTranslateBeatsMove.get(),
            &ControlObject::valueChanged,
            this,
            &BpmControl::slotTranslateBeatsMove,
            Qt::DirectConnection);

    m_pBeatsHalve = std::make_unique<ControlPushButton>(ConfigKey(group, "beats_set_halve"), false);
    connect(m_pBeatsHalve.get(),
            &ControlObject::valueChanged,
            this,
            [this](int value) {
                if (value > 0) {
                    slotScaleBpm(mixxx::Beats::BpmScale::Halve);
                }
            });
    m_pBeatsTwoThirds = std::make_unique<ControlPushButton>(
            ConfigKey(group, "beats_set_twothirds"), false);
    connect(m_pBeatsTwoThirds.get(),
            &ControlObject::valueChanged,
            this,
            [this](int value) {
                if (value > 0) {
                    slotScaleBpm(mixxx::Beats::BpmScale::TwoThirds);
                }
            });
    m_pBeatsThreeFourths = std::make_unique<ControlPushButton>(
            ConfigKey(group, "beats_set_threefourths"), false);
    connect(m_pBeatsThreeFourths.get(),
            &ControlObject::valueChanged,
            this,
            [this](int value) {
                if (value > 0) {
                    slotScaleBpm(mixxx::Beats::BpmScale::ThreeFourths);
                }
            });
    m_pBeatsFourThirds = std::make_unique<ControlPushButton>(
            ConfigKey(group, "beats_set_fourthirds"), false);
    connect(m_pBeatsFourThirds.get(),
            &ControlObject::valueChanged,
            this,
            [this](int value) {
                if (value > 0) {
                    slotScaleBpm(mixxx::Beats::BpmScale::FourThirds);
                }
            });
    m_pBeatsThreeHalves = std::make_unique<ControlPushButton>(
            ConfigKey(group, "beats_set_threehalves"), false);
    connect(m_pBeatsThreeHalves.get(),
            &ControlObject::valueChanged,
            this,
            [this](int value) {
                if (value > 0) {
                    slotScaleBpm(mixxx::Beats::BpmScale::ThreeHalves);
                }
            });
    m_pBeatsDouble = std::make_unique<ControlPushButton>(
            ConfigKey(group, "beats_set_double"), false);
    connect(m_pBeatsDouble.get(),
            &ControlObject::valueChanged,
            this,
            [this](int value) {
                if (value > 0) {
                    slotScaleBpm(mixxx::Beats::BpmScale::Double);
                }
            });

    // Pick a wide range (kBpmRangeMin to kBpmRangeMax) and allow out of bounds sets. This lets you
    // map a soft-takeover MIDI knob to the BPM. This also creates bpm_up and
    // bpm_down controls.
    // bpm_up / bpm_down steps by kBpmRangeStep
    // bpm_up_small / bpm_down_small steps by kBpmRangeSmallStep
    m_pEngineBpm = std::make_unique<ControlLinPotmeter>(
            ConfigKey(group, "bpm"),
            kBpmRangeMin,
            kBpmRangeMax,
            kBpmRangeStep,
            kBpmRangeSmallStep,
            true);
    m_pEngineBpm->set(0.0);
    connect(m_pEngineBpm.get(),
            &ControlObject::valueChanged,
            this,
            &BpmControl::slotUpdateRateSlider,
            Qt::DirectConnection);

    // Tap the (file) BPM
    m_pBpmTap = std::make_unique<ControlPushButton>(ConfigKey(group, "bpm_tap"));
    connect(m_pBpmTap.get(),
            &ControlObject::valueChanged,
            this,
            &BpmControl::slotBpmTap,
            Qt::DirectConnection);
    connect(&m_bpmTapFilter,
            &TapFilter::tapped,
            this,
            &BpmControl::slotBpmTapFilter,
            Qt::DirectConnection);

    // Tap the tempo (playback speed)
    m_pTempoTap = std::make_unique<ControlPushButton>(ConfigKey(group, "tempo_tap"));
    connect(m_pTempoTap.get(),
            &ControlObject::valueChanged,
            this,
            &BpmControl::slotTempoTap,
            Qt::DirectConnection);
    connect(&m_tempoTapFilter,
            &TapFilter::tapped,
            this,
            &BpmControl::slotTempoTapFilter,
            Qt::DirectConnection);

    m_pTranslateBeats = std::make_unique<ControlPushButton>(
            ConfigKey(group, "beats_translate_curpos"));
    connect(m_pTranslateBeats.get(),
            &ControlObject::valueChanged,
            this,
            &BpmControl::slotBeatsTranslate,
            Qt::DirectConnection);

    m_pBeatsTranslateMatchAlignment = std::make_unique<ControlPushButton>(
            ConfigKey(group, "beats_translate_match_alignment"));
    connect(m_pBeatsTranslateMatchAlignment.get(),
            &ControlObject::valueChanged,
            this,
            &BpmControl::slotBeatsTranslateMatchAlignment,
            Qt::DirectConnection);

    m_pBpmLock = std::make_unique<ControlPushButton>(
            ConfigKey(group, "bpmlock"), false);
    m_pBpmLock->setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_pBpmLock->connectValueChangeRequest(
            this,
            &BpmControl::slotToggleBpmLock,
            Qt::DirectConnection);

    m_pBeatsUndo = std::make_unique<ControlPushButton>(
            ConfigKey(group, "beats_undo_adjustment"));
    connect(m_pBeatsUndo.get(),
            &ControlObject::valueChanged,
            this,
            &BpmControl::slotBeatsUndoAdjustment,
            Qt::DirectConnection);

    m_pBeatsUndoPossible = std::make_unique<ControlObject>(
            ConfigKey(group, "beats_undo_possible"));
    m_pBeatsUndoPossible->setReadOnly();
}

mixxx::Bpm BpmControl::getBpm() const {
    return mixxx::Bpm(m_pEngineBpm->get());
}

void BpmControl::adjustBeatsBpm(double deltaBpm) {
    const TrackPointer pTrack = getEngineBuffer()->getLoadedTrack();
    if (!pTrack) {
        return;
    }
    const mixxx::BeatsPointer pBeats = pTrack->getBeats();
    if (!pBeats) {
        return;
    }

    const mixxx::Bpm bpm = pBeats->getBpmInRange(
            mixxx::audio::kStartFramePos, frameInfo().trackEndPosition);
    // FIXME: calling bpm.value() without checking bpm.isValid()
    const auto centerBpm = mixxx::Bpm(math_max(kBpmAdjustMin, bpm.value() + deltaBpm));
    mixxx::Bpm adjustedBpm = BeatUtils::roundBpmWithinRange(
            centerBpm - kBpmAdjustStep / 2, centerBpm, centerBpm + kBpmAdjustStep / 2);
    const auto newBeats = pBeats->trySetBpm(adjustedBpm);
    if (!newBeats) {
        return;
    }
    pTrack->trySetBeats(*newBeats);
}

void BpmControl::slotAdjustBeatsFaster(double v) {
    if (v <= 0) {
        return;
    }
    adjustBeatsBpm(kBpmAdjustStep);
}

void BpmControl::slotAdjustBeatsSlower(double v) {
    if (v <= 0) {
        return;
    }
    adjustBeatsBpm(-kBpmAdjustStep);
}

void BpmControl::slotTranslateBeatsEarlier(double v) {
    if (v <= 0) {
        return;
    }
    slotTranslateBeatsMove(-1);
}

void BpmControl::slotTranslateBeatsLater(double v) {
    if (v <= 0) {
        return;
    }
    slotTranslateBeatsMove(1);
}

void BpmControl::slotTranslateBeatsMove(double v) {
    v = std::round(v);
    if (v == 0) {
        return;
    }
    const TrackPointer pTrack = getEngineBuffer()->getLoadedTrack();
    if (!pTrack) {
        return;
    }
    const mixxx::BeatsPointer pBeats = pTrack->getBeats();
    if (pBeats) {
        // TODO(rryan): Track::frameInfo is possibly inaccurate!
        const double sampleOffset = frameInfo().sampleRate * v * 0.01;
        const mixxx::audio::FrameDiff_t frameOffset =
                sampleOffset / mixxx::kEngineChannelOutputCount;
        const auto translatedBeats = pBeats->tryTranslate(frameOffset);
        if (translatedBeats) {
            pTrack->trySetBeats(*translatedBeats);
        }
    }
}

void BpmControl::slotBeatsUndoAdjustment(double v) {
    if (v <= 0) {
        return;
    }
    const TrackPointer pTrack = getEngineBuffer()->getLoadedTrack();
    if (!pTrack) {
        return;
    }
    pTrack->undoBeatsChange();
    m_pBeatsUndoPossible->forceSet(pTrack->canUndoBeatsChange());
}

void BpmControl::slotBpmTap(double v) {
    if (v > 0) {
        m_bpmTapFilter.tap();
    }
}

void BpmControl::slotBpmTapFilter(double averageLength, int numSamples) {
    // averageLength is the average interval in milliseconds tapped over
    // numSamples samples.  Have to convert to BPM now:

    if (averageLength <= 0 || numSamples < 4) {
        return;
    }

    const TrackPointer pTrack = getEngineBuffer()->getLoadedTrack();
    if (!pTrack) {
        return;
    }
    const mixxx::BeatsPointer pBeats = pTrack->getBeats();
    if (!pBeats) {
        return;
    }
    double rateRatio = m_pRateRatio->get();
    if (rateRatio == 0.0) {
        return;
    }

    // (60 seconds per minute) * (1000 milliseconds per second) / (X millis per
    // beat) = Y beats/minute
    auto averageBpm = averageBpmRoundedWithinRange(averageLength, rateRatio);
    const auto newBeats = pBeats->trySetBpm(averageBpm);
    if (!newBeats) {
        return;
    }
    pTrack->trySetBeats(*newBeats);
}

void BpmControl::slotTempoTap(double v) {
    if (v > 0) {
        m_tempoTapFilter.tap();
    }
}

void BpmControl::slotTempoTapFilter(double averageLength, int numSamples) {
    // averageLength is the average interval in milliseconds tapped over
    // numSamples samples. Have to convert to BPM now:
    if (averageLength <= 0 || numSamples < 4) {
        return;
    }

    const TrackPointer pTrack = getEngineBuffer()->getLoadedTrack();
    if (!pTrack) {
        return;
    }

    auto averageBpm = averageBpmRoundedWithinRange(averageLength, 1.0);
    m_pEngineBpm->set(averageBpm.value());
    // NOTE(ronso0) When setting the control, m_pEngineBpm->valueChanged()
    // is not emitted (with Qt6) (it is when setting via a ControlProxy),
    // so call the slot directly.
    slotUpdateRateSlider(averageBpm.value());
}

void BpmControl::slotScaleBpm(mixxx::Beats::BpmScale bpmScale) {
    const TrackPointer pTrack = getEngineBuffer()->getLoadedTrack();
    if (!pTrack) {
        return;
    }
    const mixxx::BeatsPointer pBeats = pTrack->getBeats();
    if (!pBeats) {
        return;
    }
    const auto newBeats = pBeats->tryScale(bpmScale);
    if (!newBeats) {
        return;
    }
    pTrack->trySetBeats(*newBeats);
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
    double rate = 1.0;
    double localBpm = m_pLocalBpm->get();
    if (localBpm != 0.0) {
        rate = m_dSyncInstantaneousBpm / localBpm;
    }

    // If we are not quantized, or there are no beats, or we're leader,
    // or we're in reverse, just return the rate as-is.
    if (!m_pQuantize.toBool() || !m_pBeats || m_pReverseButton.toBool()) {
        m_resetSyncAdjustment = true;
        return rate + userTweak;
    }

    const auto prevBeatPosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pPrevBeat.get());
    const auto nextBeatPosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pNextBeat.get());

    if (!prevBeatPosition.isValid() || !nextBeatPosition.isValid()) {
        m_resetSyncAdjustment = true;
        return rate + userTweak;
    }

    const double beatLengthFrames = nextBeatPosition - prevBeatPosition;

    // Now that we have our beat distance we can also check how large the
    // current loop is.  If we are in a <1 beat loop, don't worry about offset.
    if (m_pLoopEnabled.toBool()) {
        const auto loopStartPosition =
                mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                        m_pLoopStartPosition.get());
        const auto loopEndPosition =
                mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                        m_pLoopEndPosition.get());
        // This should always be true when a loop is enabled, but we check it
        // anyway to prevent race conditions.
        if (loopStartPosition.isValid() && loopEndPosition.isValid()) {
            const auto loopSize = (loopEndPosition - loopStartPosition) / beatLengthFrames;
            if (loopSize < 1.0 && loopSize > 0) {
                m_resetSyncAdjustment = true;
                return rate + userTweak;
            }
        }
    }

    // Now we have all we need to calculate the sync adjustment if any.
    double adjustment = calcSyncAdjustment(userTweak != 0.0);
    // This can be used to detect pitch shift issues with cloned decks
    // DEBUG_ASSERT(((rate + userTweak) * adjustment) == 1);
    return (rate + userTweak) * adjustment;
}

double BpmControl::calcSyncAdjustment(bool userTweakingSync) {
    int resetSyncAdjustment = m_resetSyncAdjustment.fetchAndStoreRelaxed(0);
    if (resetSyncAdjustment) {
        m_dLastSyncAdjustment = 1.0;
    }

    // Either shortest distance is directly to the leader or backwards.

    // TODO(rryan): This is kind of backwards because we are measuring distance
    // from leader to my percentage. All of the control code below is based on
    // this point of reference so I left it this way but I think we should think
    // about things in terms of "my percentage-offset setpoint" that the control
    // loop should aim to maintain.
    // TODO(rryan): All of this code is based on the assumption that a track
    // can't pass through multiple beats in one engine callback. Instead our
    // setpoint should be tracking the true offset in "samples traveled" rather
    // than modular 1.0 beat fractions. This will allow sync to work across loop
    // boundaries too.

    const double syncTargetBeatDistance = m_dSyncTargetBeatDistance.getValue();
    const double thisBeatDistance = m_pThisBeatDistance.get();
    const double error = shortestPercentageChange(syncTargetBeatDistance, thisBeatDistance);
    const double curUserOffset = m_dUserOffset.getValue();

    double adjustment = 1.0;

    if (userTweakingSync) {
        // The user is actively adjusting the sync, so take the current error as their preferred
        // user offset.
        adjustment = 1.0;
        // When updating the user offset, make sure to remove the existing offset or else it
        // will get double-applied.
        m_dUserOffset.setValue(error + curUserOffset);
    } else {
        // Threshold above which we do sync adjustment.
        const double kErrorThreshold = 0.01;
        // Threshold above which sync is really, really bad, so much so that we
        // don't even know if we're ahead or behind.  This can occur when quantize was
        // off, but then it gets turned on.
        constexpr double kTrainWreckThreshold = 0.2;
        constexpr double kSyncAdjustmentCap = 0.05;
        if (fabs(error) > kTrainWreckThreshold) {
            // Assume poor reflexes (late button push) -- speed up to catch the other track.
            adjustment = 1.0 + kSyncAdjustmentCap;
        } else if (fabs(error) > kErrorThreshold) {
            // Proportional control constant. The higher this is, the more we
            // influence sync.
            constexpr double kSyncAdjustmentProportional = 0.7;
            constexpr double kSyncDeltaCap = 0.02;

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
    if (kLogger.traceEnabled() && adjustment - 1.0 > 0.01) {
        kLogger.trace() << m_group << "****************";
        kLogger.trace() << "target beat distance:" << syncTargetBeatDistance;
        kLogger.trace() << "my     beat distance:" << thisBeatDistance;
        kLogger.trace() << "user offset distance:" << curUserOffset;
        kLogger.trace() << "error               :" << error;
        kLogger.trace() << "adjustment          :" << adjustment;
    }
    return adjustment;
}

double BpmControl::getBeatDistance(mixxx::audio::FramePos thisPosition) const {
    // We have to adjust our reported beat distance by the user offset to
    // preserve comparisons of beat distances.  Specifically, this beat distance
    // is used in synccontrol to update the internal clock beat distance, and if
    // we don't adjust the reported distance the track will try to adjust
    // sync against itself.
    if (kLogger.traceEnabled()) {
        kLogger.trace() << getGroup()
                        << "BpmControl::getBeatDistance. thisPosition:"
                        << thisPosition;
    }
    mixxx::audio::FramePos prevBeatPosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pPrevBeat.get());
    mixxx::audio::FramePos nextBeatPosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pNextBeat.get());

    if (!prevBeatPosition.isValid() || !nextBeatPosition.isValid()) {
        return 0.0 - m_dUserOffset.getValue();
    }

    const mixxx::audio::FrameDiff_t beatLengthFrames = nextBeatPosition - prevBeatPosition;
    double beatPercentage = (beatLengthFrames == 0.0)
            ? 0.0
            : (thisPosition - prevBeatPosition) / beatLengthFrames;
    beatPercentage -= m_dUserOffset.getValue();
    // Because findNext and findPrev have an epsilon built in, and because the user
    // might have tweaked the offset, sometimes the beat percentage is out of range.
    // Fix it.
    if (beatPercentage < 0) {
        ++beatPercentage;
    }
    if (beatPercentage > 1) {
        --beatPercentage;
    }

    if (kLogger.traceEnabled()) {
        kLogger.trace() << getGroup() << "BpmControl::getBeatDistance. dBeatPercentage:"
                        << beatPercentage << "-  offset "
                        << m_dUserOffset.getValue() << " =  "
                        << (beatPercentage - m_dUserOffset.getValue());
    }

    return beatPercentage;
}

// static
bool BpmControl::getBeatContext(
        const mixxx::BeatsPointer& pBeats,
        mixxx::audio::FramePos position,
        mixxx::audio::FramePos* pPrevBeatPosition,
        mixxx::audio::FramePos* pNextBeatPosition,
        mixxx::audio::FrameDiff_t* pBeatLengthFrames,
        double* pBeatPercentage) {
    if (!pBeats) {
        return false;
    }

    mixxx::audio::FramePos prevBeatPosition;
    mixxx::audio::FramePos nextBeatPosition;
    if (!pBeats->findPrevNextBeats(position, &prevBeatPosition, &nextBeatPosition, false)) {
        return false;
    }

    if (pPrevBeatPosition != nullptr) {
        *pPrevBeatPosition = prevBeatPosition;
    }

    if (pNextBeatPosition != nullptr) {
        *pNextBeatPosition = nextBeatPosition;
    }

    return getBeatContextNoLookup(position,
            prevBeatPosition,
            nextBeatPosition,
            pBeatLengthFrames,
            pBeatPercentage);
}

// static
bool BpmControl::getBeatContextNoLookup(
        mixxx::audio::FramePos position,
        mixxx::audio::FramePos prevBeatPosition,
        mixxx::audio::FramePos nextBeatPosition,
        mixxx::audio::FrameDiff_t* pBeatLengthFrames,
        double* pBeatPercentage) {
    if (!prevBeatPosition.isValid() || !nextBeatPosition.isValid()) {
        return false;
    }

    const mixxx::audio::FrameDiff_t beatLengthFrames = nextBeatPosition - prevBeatPosition;
    if (pBeatLengthFrames != nullptr) {
        *pBeatLengthFrames = beatLengthFrames;
    }

    if (pBeatPercentage != nullptr) {
        *pBeatPercentage = (beatLengthFrames == 0.0)
                ? 0.0
                : (position - prevBeatPosition) / beatLengthFrames;
    }

    return true;
}

mixxx::audio::FramePos BpmControl::getNearestPositionInPhase(
        mixxx::audio::FramePos thisPosition, bool respectLoops, bool playing) {
    // Without a beatgrid, we don't know the phase offset.
    const mixxx::BeatsPointer pBeats = m_pBeats;
    if (!pBeats) {
        return thisPosition;
    }

    SyncMode syncMode = getSyncMode();

    // Get the current position of this deck.
    mixxx::audio::FramePos thisPrevBeatPosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pPrevBeat.get());
    mixxx::audio::FramePos thisNextBeatPosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pNextBeat.get());
    mixxx::audio::FrameDiff_t thisBeatLengthFrames;
    if (!thisPrevBeatPosition.isValid() || !thisNextBeatPosition.isValid() ||
            thisPosition > thisNextBeatPosition ||
            thisPosition < thisPrevBeatPosition) {
        if (kLogger.traceEnabled()) {
            kLogger.trace() << "BpmControl::getNearestPositionInPhase out of date"
                            << thisPosition << thisNextBeatPosition << thisPrevBeatPosition;
        }
        // This happens if dThisPosition is the target position of a requested
        // seek command
        if (!getBeatContext(pBeats,
                    thisPosition,
                    &thisPrevBeatPosition,
                    &thisNextBeatPosition,
                    &thisBeatLengthFrames,
                    nullptr)) {
            return thisPosition;
        }
    } else {
        if (!getBeatContextNoLookup(thisPosition,
                    thisPrevBeatPosition,
                    thisNextBeatPosition,
                    &thisBeatLengthFrames,
                    nullptr)) {
            return thisPosition;
        }
    }

    double otherBeatFraction;
    if (isFollower(syncMode)) {
        // If we're a follower, it's easy to get the other beat fraction
        otherBeatFraction = m_dSyncTargetBeatDistance.getValue();
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
            return thisPosition;
        }

        TrackPointer otherTrack = pOtherEngineBuffer->getLoadedTrack();
        mixxx::BeatsPointer otherBeats =
                otherTrack ? otherTrack->getBeats() : mixxx::BeatsPointer();

        // If either track does not have beats, then we can't adjust the phase.
        if (!otherBeats) {
            return thisPosition;
        }

        const auto otherPosition = pOtherEngineBuffer->getExactPlayPos();
        if (!BpmControl::getBeatContext(otherBeats,
                    otherPosition,
                    nullptr,
                    nullptr,
                    nullptr,
                    &otherBeatFraction)) {
            return thisPosition;
        }
    }

    bool thisNearNextBeat = (thisNextBeatPosition - thisPosition) <=
            (thisPosition - thisPrevBeatPosition);
    bool otherNearNextBeat = otherBeatFraction >= 0.5;

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

    mixxx::audio::FramePos newPlayPosition;
    if (thisNearNextBeat == otherNearNextBeat) {
        newPlayPosition = thisPrevBeatPosition;
    } else if (thisNearNextBeat && !otherNearNextBeat) {
        newPlayPosition = thisNextBeatPosition;
    } else { //!thisNearNextBeat && otherNearNextBeat
        thisPrevBeatPosition = pBeats->findNthBeat(thisPosition, -2);
        newPlayPosition = thisPrevBeatPosition;
    }
    newPlayPosition += (otherBeatFraction + m_dUserOffset.getValue()) * thisBeatLengthFrames;

    if (!respectLoops) {
        return newPlayPosition;
    }

    // We might be seeking outside the loop.
    const bool loopEnabled = m_pLoopEnabled.toBool();
    const auto loopStartPosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pLoopStartPosition.get());
    const auto loopEndPosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pLoopEndPosition.get());

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
    if (!loopEnabled || !loopStartPosition.isValid() ||
            !loopEndPosition.isValid() || thisPosition > loopEndPosition) {
        return newPlayPosition;
    }

    const mixxx::audio::FrameDiff_t loopLengthFrames = loopEndPosition - loopStartPosition;
    const mixxx::audio::FrameDiff_t endDeltaFrames = newPlayPosition - loopEndPosition;

    // Syncing to after the loop end.
    if (endDeltaFrames > 0 && loopLengthFrames > 0) {
        double i = endDeltaFrames / loopLengthFrames;
        newPlayPosition = loopStartPosition + endDeltaFrames - i * loopLengthFrames;

        // Move new position after loop jump into phase as well.
        // This is a recursive call, called only twice because of
        // respectLoops = false
        newPlayPosition = getNearestPositionInPhase(newPlayPosition, false, playing);
    }

    // Note: Syncing to before the loop beginning is allowed, because
    // loops are catching
    return newPlayPosition;
}

mixxx::audio::FramePos BpmControl::getBeatMatchPosition(
        mixxx::audio::FramePos thisPosition, bool respectLoops, bool playing) {
    // Without a beatgrid, we don't know the phase offset.
    if (!m_pBeats) {
        return thisPosition;
    }
    const double thisRateRatio = m_pRateRatio->get();
    if (thisRateRatio == 0.0) {
        // We can't continue without a rate.
        // This avoids also a division by zero in the following calculations
        return thisPosition;
    }

    EngineBuffer* pOtherEngineBuffer = pickSyncTarget();
    if (kLogger.traceEnabled()) {
        if (pOtherEngineBuffer) {
            kLogger.trace() << "BpmControl::getBeatMatchPosition sync target"
                            << pOtherEngineBuffer->getGroup();
        } else {
            kLogger.trace() << "BpmControl::getBeatMatchPosition no sync target found";
        }
    }
    if (playing) {
        if (!pOtherEngineBuffer || pOtherEngineBuffer->getSpeed() == 0.0) {
            // "this" track is playing, or just starting.
            // Only match phase if the sync target is playing as well,
            // otherwise use the previous phase of "this" track before the seek.
            // This occurs when the DJ does a quantized seek -- we preserve
            // the exact beat distance.
            pOtherEngineBuffer = getEngineBuffer();
        }
    } else if (!pOtherEngineBuffer) {
        return thisPosition;
    }

    // Get the current position of this deck.
    mixxx::audio::FramePos thisPrevBeatPosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pPrevBeat.get());
    mixxx::audio::FramePos thisNextBeatPosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pNextBeat.get());
    mixxx::audio::FrameDiff_t thisBeatLengthFrames;

    // Look up the next beat and beat length for the new position
    if (!thisNextBeatPosition.isValid() || !thisPrevBeatPosition.isValid() ||
            thisPosition > thisNextBeatPosition ||
            thisPosition < thisPrevBeatPosition) {
        if (kLogger.traceEnabled()) {
            kLogger.trace() << "BpmControl::getBeatMatchPosition out of date"
                            << thisPrevBeatPosition << thisPosition << thisNextBeatPosition;
        }
        // This happens if thisPosition is the target position of a requested
        // seek command.  Get new prev and next beats for the calculation.
        getBeatContext(
                m_pBeats,
                thisPosition,
                &thisPrevBeatPosition,
                &thisNextBeatPosition,
                &thisBeatLengthFrames,
                nullptr);
        // now we either have a useful next beat or there is none
        if (!thisNextBeatPosition.isValid()) {
            // We can't match the next beat, give up.
            return thisPosition;
        }
    } else {
        if (kLogger.traceEnabled()) {
            kLogger.trace() << "BpmControl::getBeatMatchPosition up to date"
                            << thisPrevBeatPosition << thisPosition << thisNextBeatPosition;
        }
        // We are between the previous and next beats so we can try a standard
        // lookup of the beat length.
        getBeatContextNoLookup(
                thisPosition,
                thisPrevBeatPosition,
                thisNextBeatPosition,
                &thisBeatLengthFrames,
                nullptr);
    }

    TrackPointer otherTrack = pOtherEngineBuffer->getLoadedTrack();
    mixxx::BeatsPointer otherBeats = otherTrack ? otherTrack->getBeats() : mixxx::BeatsPointer();

    // If either track does not have beats, then we can't adjust the phase.
    if (!otherBeats) {
        return thisPosition;
    }

    const mixxx::audio::FramePos otherPosition = pOtherEngineBuffer->getExactPlayPos();
    const mixxx::audio::SampleRate thisSampleRate = m_pBeats->getSampleRate();

    // Seek our next beat to the other next beat near our beat.
    // This is the only thing we can do if the track has different BPM,
    // playing the next beat together.

    // calculate framesTransposeFactor first to avoid rounding issues, because it is often 1
    double framesTransposeFactor = otherBeats->getSampleRate() /
            thisSampleRate * pOtherEngineBuffer->getRateRatio() / thisRateRatio;
    // subtract first to avoid a rounding issue, because lower double values
    // have a smaller minimum step width
    const mixxx::audio::FrameDiff_t otherToThisOffset =
            otherPosition - thisPosition * framesTransposeFactor;
    const mixxx::audio::FramePos otherPositionOfThisNextBeat =
            thisNextBeatPosition * framesTransposeFactor + otherToThisOffset;

    mixxx::audio::FramePos otherPrevBeatPosition;
    mixxx::audio::FramePos otherNextBeatPosition;
    mixxx::audio::FrameDiff_t otherBeatLengthFrames = -1;
    double otherBeatFraction = -1;
    if (!BpmControl::getBeatContext(
                otherBeats,
                otherPositionOfThisNextBeat,
                &otherPrevBeatPosition,
                &otherNextBeatPosition,
                &otherBeatLengthFrames,
                &otherBeatFraction)) {
        return thisPosition;
    }

    if (otherBeatLengthFrames == -1 || otherBeatFraction == -1) {
        // the other Track has no usable beat info, do not seek.
        return thisPosition;
    }

    // Offset the other deck's user offset, if any.
    otherBeatFraction -= pOtherEngineBuffer->getUserOffset();

    // We can either match the past beat with dOtherBeatFraction 1.0
    // or the next beat with dOtherBeatFraction 0.0
    // We prefer the next because this is what will be played,
    // unless we are close to the previous.
    // This happens if the user presses play too late.
    if (otherBeatFraction > 1.0 - kPastBeatMatchThreshold) {
        // match the past beat
        otherBeatFraction -= 1.0;
    }

    double otherDivSec2 = otherBeatFraction * otherBeatLengthFrames /
            otherBeats->getSampleRate() / pOtherEngineBuffer->getRateRatio();
    // Transform for this track
    double seekMatch = otherDivSec2 * thisSampleRate * thisRateRatio;

    if (thisBeatLengthFrames > 0) {
        // restore phase adjustment
        seekMatch += (thisBeatLengthFrames * m_dUserOffset.getValue());
        if (thisBeatLengthFrames / 2 < seekMatch) {
            // seek to previous beat, because of shorter distance
            seekMatch -= thisBeatLengthFrames;
        } else if (thisBeatLengthFrames / 2 < -seekMatch) {
            // seek to beat after next, because of shorter distance
            seekMatch += thisBeatLengthFrames;
        }
    }
    mixxx::audio::FramePos newPlayPosition = thisPosition + seekMatch;

    if (!respectLoops) {
        return newPlayPosition;
    }

    // We might be seeking outside the loop.
    const bool loopEnabled = m_pLoopEnabled.toBool();
    const auto loopStartPosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pLoopStartPosition.get());
    const auto loopEndPosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pLoopEndPosition.get());

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
    if (!loopEnabled || !loopStartPosition.isValid() ||
            !loopEndPosition.isValid() || thisPosition > loopEndPosition) {
        return newPlayPosition;
    }

    const mixxx::audio::FrameDiff_t loopLengthFrames = loopEndPosition - loopStartPosition;
    const mixxx::audio::FrameDiff_t endDeltaFrames = newPlayPosition - loopEndPosition;

    // Syncing to after the loop end.
    if (endDeltaFrames > 0 && loopLengthFrames > 0) {
        double i = endDeltaFrames / loopLengthFrames;
        newPlayPosition = loopStartPosition + endDeltaFrames - i * loopLengthFrames;

        // Move new position after loop jump into phase as well.
        // This is a recursive call, called only twice because of
        // respectLoops = false
        newPlayPosition = getNearestPositionInPhase(newPlayPosition, false, playing);
    }

    // Note: Syncing to before the loop beginning is allowed, because
    // loops are catching
    return newPlayPosition;
}

mixxx::audio::FrameDiff_t BpmControl::getPhaseOffset(mixxx::audio::FramePos thisPosition) {
    // This does not respect looping
    const mixxx::audio::FramePos position = getNearestPositionInPhase(thisPosition, false, false);
    return position - thisPosition;
}

void BpmControl::slotUpdateEngineBpm(double rateRatio) {
    // Adjust playback bpm in response to a rate_ratio update
    if (kLogger.traceEnabled()) {
        kLogger.trace() << getGroup() << "BpmControl::slotUpdateEngineBpm: local BPM:"
                        << m_pLocalBpm->get()
                        << "rate ratio:"
                        << rateRatio;
        // This can be used to detect pitch shift issues with cloned decks
        // DEBUG_ASSERT(getGroup() != "[Channel1]" || m_pRateRatio->get(); == 1);
    }
    m_pEngineBpm->set(m_pLocalBpm->get() * rateRatio);
}

void BpmControl::slotUpdateRateSlider(double value) {
    if (kLogger.traceEnabled()) {
        kLogger.trace() << getGroup() << "BpmControl::slotUpdateRateSlider"
                        << value;
    }
    // Adjust rate slider position in response to a change in rate range or m_pEngineBpm

    double localBpm = m_pLocalBpm->get();
    if (localBpm == 0.0) {
        return;
    }

    double dRateRatio = m_pEngineBpm->get() / localBpm;
    m_pRateRatio->set(dRateRatio);
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
                        << (pBeats ? pBeats->getBpmInRange(
                                             mixxx::audio::kStartFramePos,
                                             frameInfo().trackEndPosition)
                                   : mixxx::Bpm());
    }
    m_pBeats = pBeats;
    updateLocalBpm();
    resetSyncAdjustment();
    TrackPointer pTrack = getEngineBuffer()->getLoadedTrack();
    m_pBeatsUndoPossible->forceSet(pTrack ? pTrack->canUndoBeatsChange() : 0);
}

void BpmControl::trackBpmLockChanged(bool locked) {
    m_pBpmLock->setAndConfirm(locked);
    if (locked) {
        m_pBeatsUndoPossible->forceSet(0);
    } else {
        TrackPointer pTrack = getEngineBuffer()->getLoadedTrack();
        m_pBeatsUndoPossible->forceSet(pTrack ? pTrack->canUndoBeatsChange() : 0);
    }
}

void BpmControl::notifySeek(mixxx::audio::FramePos position) {
    updateBeatDistance(position);
}

void BpmControl::slotBeatsTranslate(double v) {
    if (v <= 0) {
        return;
    }
    TrackPointer pTrack = getEngineBuffer()->getLoadedTrack();
    if (!pTrack) {
        return;
    }
    const mixxx::BeatsPointer pBeats = pTrack->getBeats();
    if (pBeats) {
        const auto currentPosition = frameInfo().currentPosition.toLowerFrameBoundary();
        const auto closestBeat = pBeats->findClosestBeat(currentPosition);
        const mixxx::audio::FrameDiff_t frameOffset = currentPosition - closestBeat;
        const auto translatedBeats = pBeats->tryTranslate(frameOffset);
        if (translatedBeats) {
            pTrack->trySetBeats(*translatedBeats);
        }
    }
}

void BpmControl::slotBeatsTranslateMatchAlignment(double v) {
    if (v <= 0) {
        return;
    }
    TrackPointer pTrack = getEngineBuffer()->getLoadedTrack();
    if (!pTrack) {
        return;
    }
    const mixxx::BeatsPointer pBeats = pTrack->getBeats();
    if (pBeats) {
        // Must reset the user offset *before* calling getPhaseOffset(),
        // otherwise it will always return 0 if sync lock is active.
        m_dUserOffset.setValue(0.0);

        const mixxx::audio::FrameDiff_t frameOffset = -getPhaseOffset(frameInfo().currentPosition);
        const auto translatedBeats = pBeats->tryTranslate(frameOffset);
        if (translatedBeats) {
            pTrack->trySetBeats(*translatedBeats);
        }
    }
}

void BpmControl::slotToggleBpmLock(double v) {
    Q_UNUSED(v);
    const TrackPointer pTrack = getEngineBuffer()->getLoadedTrack();
    if (!pTrack) {
        return;
    }
    bool locked = pTrack->isBpmLocked();
    pTrack->setBpmLocked(!locked);
    // The pushbutton is updated in trackBpmLockChanged() via bpmLockChanged() signal.
}

mixxx::Bpm BpmControl::updateLocalBpm() {
    mixxx::Bpm prevLocalBpm = mixxx::Bpm(m_pLocalBpm->get());
    mixxx::Bpm localBpm;
    const mixxx::BeatsPointer pBeats = m_pBeats;
    const FrameInfo info = frameInfo();
    if (pBeats) {
        if (info.currentPosition.isValid() && info.currentPosition != kInitialPlayPosition) {
            localBpm = pBeats->getBpmAroundPosition(info.currentPosition, kLocalBpmSpan);
            if (!localBpm.isValid()) {
                localBpm = pBeats->getBpmInRange(
                        mixxx::audio::kStartFramePos, info.trackEndPosition);
            }
        }
    }
    if (localBpm != prevLocalBpm) {
        if (kLogger.traceEnabled()) {
            kLogger.trace() << getGroup() << "BpmControl::updateLocalBpm" << localBpm;
        }
        double localBpmValue = mixxx::Bpm::kValueUndefined;
        if (localBpm.isValid()) {
            localBpmValue = localBpm.value();
        }
        m_pLocalBpm->set(localBpmValue);
        slotUpdateEngineBpm(m_pRateRatio->get());
    }
    return localBpm;
}

double BpmControl::updateBeatDistance() {
    return updateBeatDistance(frameInfo().currentPosition);
}

double BpmControl::updateBeatDistance(mixxx::audio::FramePos playpos) {
    double beatDistance = getBeatDistance(playpos);
    m_pThisBeatDistance.set(beatDistance);
    if (!isSynchronized() && m_dUserOffset.getValue() != 0.0) {
        m_dUserOffset.setValue(0.0);
    }
    if (kLogger.traceEnabled()) {
        kLogger.trace() << getGroup() << "BpmControl::updateBeatDistance" << beatDistance;
    }
    return beatDistance;
}

void BpmControl::setTargetBeatDistance(double beatDistance) {
    if (kLogger.traceEnabled()) {
        qDebug() << getGroup() << "BpmControl::setTargetBeatDistance:" << beatDistance;
    }
    m_dSyncTargetBeatDistance.setValue(beatDistance);
}

void BpmControl::updateInstantaneousBpm(double instantaneousBpm) {
    m_dSyncInstantaneousBpm = instantaneousBpm;
}

void BpmControl::resetSyncAdjustment() {
    // Immediately edit the beat distance to reflect the new reality.
    double new_distance = m_pThisBeatDistance.get() + m_dUserOffset.getValue();
    if (kLogger.traceEnabled()) {
        kLogger.trace() << getGroup() << "BpmControl::resetSyncAdjustment: " << new_distance;
    }
    m_pThisBeatDistance.set(new_distance);
    m_dUserOffset.setValue(0.0);
    m_resetSyncAdjustment = true;
}

void BpmControl::collectFeatures(GroupFeatureState* pGroupFeatures, double speed) const {
    // Without a beatgrid we don't know any beat details.
    FrameInfo info = frameInfo();
    if (!info.sampleRate.isValid() || !m_pBeats) {
        return;
    }

    // Get the current position of this deck.
    mixxx::audio::FramePos prevBeatPosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pPrevBeat.get());
    mixxx::audio::FramePos nextBeatPosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pNextBeat.get());
    mixxx::audio::FrameDiff_t beatLengthFrames;
    double beatFraction;
    if (getBeatContextNoLookup(info.currentPosition,
                prevBeatPosition,
                nextBeatPosition,
                &beatLengthFrames,
                &beatFraction)) {
        const double rateRatio = m_pRateRatio->get();
        if (rateRatio != 0.0) {
            pGroupFeatures->beat_length = {beatLengthFrames / rateRatio, speed / rateRatio};
        }
        pGroupFeatures->beat_fraction_buffer_end = beatFraction;
    }
}

double BpmControl::getRateRatio() const {
    return m_pRateRatio->get();
}
