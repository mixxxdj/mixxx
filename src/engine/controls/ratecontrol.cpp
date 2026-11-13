#include "engine/controls/ratecontrol.h"

#include <QtDebug>
#include <algorithm>
#include <chrono>
#include <cmath>

#include "control/controlobject.h"
#include "control/controlpotmeter.h"
#include "control/controlproxy.h"
#include "control/controlpushbutton.h"
#include "control/controlttrotary.h"
#include "engine/controls/bpmcontrol.h"
#include "engine/controls/enginecontrol.h"
#include "engine/positionscratchcontroller.h"
#include "mixer/playermanager.h"
#include "moc_ratecontrol.cpp"
#include "util/rotary.h"
#include "vinylcontrol/defs_vinylcontrol.h"

namespace {
constexpr int kRateSensitivityMin = 100;
constexpr int kRateSensitivityMax = 2500;

using namespace std::chrono_literals;

// Approximate Technics SL-1210 brake time from 33 RPM to standstill.
// At 45 RPM the platter spins 1.35x faster with the same brake torque,
// so it takes 1.35x longer to stop.
constexpr std::chrono::duration<double> kSpindownTime33 = 800ms;
constexpr std::chrono::duration<double> kSpindownTime45 =
        kSpindownTime33 * 45.0 / (100.0 / 3.0);
// Power cut: no brake, the coasting platter is only slowed by bearing friction
// (constant) and drag (proportional to speed).
// kCoastTime33 is the time from 33 RPM to standstill.
// kCoastDragRatio = drag / friction: higher drops fast first and then
// crawls, lower is closer to a straight line.
// At 45 RPM the platter has more momentum, so the constant
// friction has less effect and it coasts longer.
constexpr std::chrono::duration<double> kCoastTime33 = 8s;
constexpr double kCoastDragRatio = 2.0;
// Drag and friction are rates (speed units per second), not durations
const double kCoastDrag = std::log(1.0 + kCoastDragRatio) / kCoastTime33.count();
const double kCoastFriction33 = kCoastDrag / kCoastDragRatio;
const double kCoastFriction45 = kCoastFriction33 * (100.0 / 3.0) / 45.0;
// Spin-up: motor torque (speed units per second), then soft saturation
// when the platter settles into quartz lock
constexpr double kSpinupTorque33 = 1.42;
constexpr double kSpinupTorque45 = kSpinupTorque33 * (100.0 / 3.0) / 45.0;
constexpr double kQuartzLockStart = 0.9;
constexpr double kQuartzLockGain = 2.0;
// Range of the spin-up / spin-down time settings, 1.0 = Technics SL-1210
constexpr double kMinRampTimeFactor = 0.25;
constexpr double kMaxRampTimeFactor = 4.0;

// RampRpmMode::Rpm78 is reserved and uses the 45 RPM values until implemented
double spinupTorque(RateControl::RampRpmMode rpm) {
    return rpm == RateControl::RampRpmMode::Rpm33 ? kSpinupTorque33 : kSpinupTorque45;
}

double coastFriction(RateControl::RampRpmMode rpm) {
    return rpm == RateControl::RampRpmMode::Rpm33 ? kCoastFriction33 : kCoastFriction45;
}

double brakeTime(RateControl::RampRpmMode rpm) {
    return (rpm == RateControl::RampRpmMode::Rpm33 ? kSpindownTime33 : kSpindownTime45)
            .count();
}
} // namespace

// Static default values for rate buttons (percents)
ControlValueAtomic<double> RateControl::m_dTemporaryRateChangeCoarse;
ControlValueAtomic<double> RateControl::m_dTemporaryRateChangeFine;
ControlValueAtomic<double> RateControl::m_dPermanentRateChangeCoarse;
ControlValueAtomic<double> RateControl::m_dPermanentRateChangeFine;
int RateControl::m_iRateRampSensitivity;
RateControl::RampMode RateControl::m_eRateRampMode;

const double RateControl::kWheelMultiplier = 40.0;
const double RateControl::kPausedJogMultiplier = 18.0;

RateControl::RateControl(const QString& group, UserSettingsPointer pConfig)
        : EngineControl(group, pConfig),
          m_pBpmControl(nullptr),
          m_pSampleRate(QStringLiteral("[App]"), QStringLiteral("samplerate")),
          m_pRateRatio(std::make_unique<ControlObject>(
                  ConfigKey(group, QStringLiteral("rate_ratio")),
                  true,
                  false,
                  false,
                  1.0)),
          m_pRateDir(std::make_unique<ControlObject>(
                  ConfigKey(group, QStringLiteral("rate_dir")))),
          m_pRateRange(std::make_unique<ControlPotmeter>(
                  ConfigKey(group, QStringLiteral("rateRange")), 0.01, 4.00)),
          // We need the sample rate so we can guesstimate something close
          // what latency is.
          // Allow rate slider to go out of bounds so that sync lock rate
          // adjustments are not capped.
          m_pRateSlider(std::make_unique<ControlPotmeter>(
                  ConfigKey(group, QStringLiteral("rate")), -1.0, 1.0, true)),
          // Search rate. Rate used when searching in sound. This overrules the
          // playback rate
          m_pRateSearch(std::make_unique<ControlPotmeter>(
                  ConfigKey(group, QStringLiteral("rateSearch")), -300., 300.)),
          // Temporary rate-change buttons
          m_pButtonRateTempDown(std::make_unique<ControlPushButton>(
                  ConfigKey(group, QStringLiteral("rate_temp_down")))),
          m_pButtonRateTempDownSmall(std::make_unique<ControlPushButton>(
                  ConfigKey(group, QStringLiteral("rate_temp_down_small")))),
          m_pButtonRateTempUp(std::make_unique<ControlPushButton>(
                  ConfigKey(group, QStringLiteral("rate_temp_up")))),
          m_pButtonRateTempUpSmall(std::make_unique<ControlPushButton>(
                  ConfigKey(group, QStringLiteral("rate_temp_up_small")))),
          // Permanent rate-change buttons
          m_pButtonRatePermDown(std::make_unique<ControlPushButton>(
                  ConfigKey(group, QStringLiteral("rate_perm_down")))),
          m_pButtonRatePermDownSmall(std::make_unique<ControlPushButton>(
                  ConfigKey(group, QStringLiteral("rate_perm_down_small")))),
          m_pButtonRatePermUp(std::make_unique<ControlPushButton>(
                  ConfigKey(group, QStringLiteral("rate_perm_up")))),
          m_pButtonRatePermUpSmall(std::make_unique<ControlPushButton>(
                  ConfigKey(group, QStringLiteral("rate_perm_up_small")))),
          // Reverse button
          m_pReverseButton(std::make_unique<ControlPushButton>(
                  ConfigKey(group, QStringLiteral("reverse")))),
          m_pReverseRollButton(std::make_unique<ControlPushButton>(
                  ConfigKey(group, QStringLiteral("reverseroll")))),
          m_pForwardButton(std::make_unique<ControlPushButton>(
                  ConfigKey(group, QStringLiteral("fwd")))),
          m_pBackButton(std::make_unique<ControlPushButton>(
                  ConfigKey(group, QStringLiteral("back")))),
          // Wheel to control playback position/speed
          m_pWheel(std::make_unique<ControlTTRotary>(
                  ConfigKey(group, QStringLiteral("wheel")))),
          // Scratch controller, this is an accumulator which is useful for
          // controllers that return individual +1 or -1s, these get added up
          // and cleared when we read
          m_pScratch2(std::make_unique<ControlObject>(
                  ConfigKey(group, QStringLiteral("scratch2")))),
          m_pScratch2Enable(std::make_unique<ControlPushButton>(
                  ConfigKey(group, QStringLiteral("scratch2_enable")))),
          m_pScratch2Scratching(std::make_unique<ControlPushButton>(ConfigKey(
                  group, QStringLiteral("scratch2_indicates_scratching")))),
          m_pScratchController(
                  std::make_unique<PositionScratchController>(group)),
          m_pJog(std::make_unique<ControlObject>(
                  ConfigKey(group, QStringLiteral("jog")))),
          // FIXME: The filter length should be dependent on sample rate/block size or something
          m_pJogFilter(std::make_unique<Rotary>(25)),
          m_pVCEnabled(nullptr),
          m_pVCScratching(nullptr),
          m_pVCMode(nullptr),
          m_syncMode(group, QStringLiteral("sync_mode")),
          m_slipEnabled(group, QStringLiteral("slip_enabled")),
          m_wrapAroundCount(0),
          m_jumpPos(mixxx::audio::FramePos()),
          m_targetPos(mixxx::audio::FramePos()),
          m_bTempStarted(false),
          m_tempRateRatio(0.0),
          m_dRateTempRampChange(0.0),
          m_platterSpeed(1.0),
          m_lastTargetRate(0.0),
          m_spinningDown(false),
          m_seekedSinceLastBuffer(false),
          m_prevPaused(true),
          m_latchedPlay(false),
          m_resetRampRequested(false),
          m_rampAllowed(PlayerManager::isDeckGroup(group)) {
    // Vinyl control COs are only created for main decks
    if (PlayerManager::isDeckGroup(getGroup())) {
        m_pVCEnabled = ControlObject::getControl(
                ConfigKey(getGroup(), QStringLiteral("vinylcontrol_enabled")),
                ControlFlag::NoAssertIfMissing);
        m_pVCScratching = ControlObject::getControl(
                ConfigKey(getGroup(), QStringLiteral("vinylcontrol_scratching")),
                ControlFlag::NoAssertIfMissing);
        m_pVCMode = ControlObject::getControl(
                ConfigKey(getGroup(), QStringLiteral("vinylcontrol_mode")),
                ControlFlag::NoAssertIfMissing);
    }
    // This is the resulting rate ratio that can be used for display or calculations.
    // The track original rate ratio is 1.
    connect(m_pRateRatio.get(),
            &ControlObject::valueChanged,
            this,
            &RateControl::slotRateRatioChanged,
            Qt::DirectConnection);

    connect(m_pRateDir.get(),
            &ControlObject::valueChanged,
            this,
            &RateControl::slotRateRangeChanged,
            Qt::DirectConnection);
    connect(m_pRateRange.get(),
            &ControlObject::valueChanged,
            this,
            &RateControl::slotRateRangeChanged,
            Qt::DirectConnection);

    connect(m_pRateSlider.get(),
            &ControlObject::valueChanged,
            this,
            &RateControl::slotRateSliderChanged,
            Qt::DirectConnection);

    m_pReverseButton->set(0);

    connect(m_pForwardButton.get(),
            &ControlObject::valueChanged,
            this,
            &RateControl::slotControlFastForward,
            Qt::DirectConnection);
    m_pForwardButton->set(0);

    connect(m_pBackButton.get(),
            &ControlObject::valueChanged,
            this,
            &RateControl::slotControlFastBack,
            Qt::DirectConnection);
    m_pBackButton->set(0);

    connect(m_pReverseRollButton.get(),
            &ControlObject::valueChanged,
            this,
            &RateControl::slotReverseRollActivate,
            Qt::DirectConnection);

    // Permanent rate-change buttons
    m_pButtonRatePermDown->setKbdRepeatable(true);
    connect(m_pButtonRatePermDown.get(),
            &ControlObject::valueChanged,
            this,
            &RateControl::slotControlRatePermDown,
            Qt::DirectConnection);

    m_pButtonRatePermDownSmall->setKbdRepeatable(true);
    connect(m_pButtonRatePermDownSmall.get(),
            &ControlObject::valueChanged,
            this,
            &RateControl::slotControlRatePermDownSmall,
            Qt::DirectConnection);

    m_pButtonRatePermUp->setKbdRepeatable(true);
    connect(m_pButtonRatePermUp.get(),
            &ControlObject::valueChanged,
            this,
            &RateControl::slotControlRatePermUp,
            Qt::DirectConnection);

    m_pButtonRatePermUpSmall->setKbdRepeatable(true);
    connect(m_pButtonRatePermUpSmall.get(),
            &ControlObject::valueChanged,
            this,
            &RateControl::slotControlRatePermUpSmall,
            Qt::DirectConnection);

    // Scratch enable toggle
    m_pScratch2Enable->set(0);

    // Enable by default, because it was always scratching before introducing
    // this control.
    m_pScratch2Scratching->set(1.0);

    //     // Update Internal Settings
    //     // Set Pitchbend Mode
    //     m_eRateRampMode = static_cast<RampMode>(
    //         getConfig()->getValue(ConfigKey("[Controls]","RateRamp"),
    //                               static_cast<int>(RampMode::Stepping)));

    //     // Set the Sensitivity
    //     m_iRateRampSensitivity =
    //             getConfig()->getValueString(ConfigKey("[Controls]","RateRampSensitivity")).toInt();

    // Physical spin-up / spin-down settings. DlgPrefDeck sets them for all
    // decks from the preferences, mappings can change them per deck.
    m_pSpinupEnabled = std::make_unique<ControlPushButton>(
            ConfigKey(group, QStringLiteral("spinup_enabled")));
    m_pSpinupEnabled->setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_pSpindownEnabled = std::make_unique<ControlPushButton>(
            ConfigKey(group, QStringLiteral("spindown_enabled")));
    m_pSpindownEnabled->setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_pSpindownMode = std::make_unique<ControlObject>(
            ConfigKey(group, QStringLiteral("spindown_mode")));
    m_pPlatterRpm = std::make_unique<ControlObject>(
            ConfigKey(group, QStringLiteral("platter_rpm")));
    m_pSpinupTime = std::make_unique<ControlObject>(
            ConfigKey(group, QStringLiteral("spinup_time")), true, false, false, 1.0);
    m_pSpindownTime = std::make_unique<ControlObject>(
            ConfigKey(group, QStringLiteral("spindown_time")), true, false, false, 1.0);
    m_rampRpm = RampRpmMode::Rpm33;
    m_rampDownMode = RampDownMode::Brake;
    m_rampTimeFactor = 1.0;
}

void RateControl::setBpmControl(BpmControl* bpmcontrol) {
    m_pBpmControl = bpmcontrol;
}

//static
void RateControl::setRateRampMode(RampMode mode) {
    m_eRateRampMode = mode;
}

//static
RateControl::RampMode RateControl::getRateRampMode() {
    return m_eRateRampMode;
}

//static
void RateControl::setRateRampSensitivity(int sense) {
    // Reverse the actual sensitivity value passed.
    // That way the gui works in an intuitive manner.
    sense = kRateSensitivityMax - sense + kRateSensitivityMin;
    if (sense < kRateSensitivityMin) {
        m_iRateRampSensitivity = kRateSensitivityMin;
    } else if (sense > kRateSensitivityMax) {
        m_iRateRampSensitivity = kRateSensitivityMax;
    } else {
        m_iRateRampSensitivity = sense;
    }
}

// static
double RateControl::spinUpTime(RampRpmMode rpm, double factor) {
    // Integrate the spin-up model of calculateSpeed() with a fine time step
    constexpr double kStep = 0.0005; // seconds
    const double torque = spinupTorque(rpm);
    double platterSpeed = 0.0;
    double time = 0.0;
    while (platterSpeed < 1.0 && time < 60.0) {
        platterSpeed += torque * kStep;
        if (platterSpeed > kQuartzLockStart) {
            platterSpeed += (1.0 - platterSpeed) * kQuartzLockGain * kStep;
        }
        time += kStep;
    }
    return time * factor;
}

// static
double RateControl::spinDownTime(RampRpmMode rpm, RampDownMode mode, double factor) {
    if (mode == RampDownMode::PowerCut) {
        // Solution of dv/dt = -(friction + drag * v) from 1 to 0
        return std::log(1.0 + kCoastDrag / coastFriction(rpm)) / kCoastDrag * factor;
    }
    return brakeTime(rpm) * factor;
}

//static
void RateControl::setTemporaryRateChangeCoarseAmount(double v) {
    m_dTemporaryRateChangeCoarse.setValue(v);
}

//static
void RateControl::setTemporaryRateChangeFineAmount(double v) {
    m_dTemporaryRateChangeFine.setValue(v);
}

//static
void RateControl::setPermanentRateChangeCoarseAmount(double v) {
    m_dPermanentRateChangeCoarse.setValue(v);
}

//static
void RateControl::setPermanentRateChangeFineAmount(double v) {
    m_dPermanentRateChangeFine.setValue(v);
}

//static
double RateControl::getTemporaryRateChangeCoarseAmount() {
    return m_dTemporaryRateChangeCoarse.getValue();
}

//static
double RateControl::getTemporaryRateChangeFineAmount() {
    return m_dTemporaryRateChangeFine.getValue();
}

//static
double RateControl::getPermanentRateChangeCoarseAmount() {
    return m_dPermanentRateChangeCoarse.getValue();
}

//static
double RateControl::getPermanentRateChangeFineAmount() {
    return m_dPermanentRateChangeFine.getValue();
}

void RateControl::slotRateRangeChanged(double) {
    // update RateSlider with the new Range value butdo not change m_pRateRatio
    slotRateRatioChanged(m_pRateRatio->get());
}

void RateControl::slotRateSliderChanged(double v) {
    double rateRatio = 1.0 + m_pRateDir->get() * m_pRateRange->get() * v;
    m_pRateRatio->set(rateRatio);
}

void RateControl::slotRateRatioChanged(double v) {
    double rateRange = m_pRateRange->get();
    if (rateRange > 0.0) {
        double newRate = m_pRateDir->get() * (v - 1) / rateRange;
        m_pRateSlider->set(newRate);
    } else {
        m_pRateSlider->set(0);
    }
}

void RateControl::slotReverseRollActivate(double v) {
    if (v > 0.0) {
        m_slipEnabled.set(1);
        m_pReverseButton->set(1);
    } else {
        m_pReverseButton->set(0);
        m_slipEnabled.set(0);
    }
}

void RateControl::slotControlFastForward(double v) {
    //qDebug() << "slotControlFastForward(" << v << ")";
    if (v > 0.0) {
        m_pRateSearch->set(4.0);
    } else {
        m_pRateSearch->set(0.0);
    }
}

void RateControl::slotControlFastBack(double v) {
    //qDebug() << "slotControlFastBack(" << v << ")";
    if (v > 0.0) {
        m_pRateSearch->set(-4.0);
    } else {
        m_pRateSearch->set(0.0);
    }
}

void RateControl::slotControlRatePermDown(double v) {
    // Adjusts temp rate down if button pressed
    if (v > 0.0) {
        m_pRateSlider->set(m_pRateSlider->get() -
                m_pRateDir->get() * m_dPermanentRateChangeCoarse.getValue() / (100 * m_pRateRange->get()));
        slotRateSliderChanged(m_pRateSlider->get());
    }
}

void RateControl::slotControlRatePermDownSmall(double v) {
    // Adjusts temp rate down if button pressed
    if (v > 0.0) {
        m_pRateSlider->set(m_pRateSlider->get() -
                m_pRateDir->get() * m_dPermanentRateChangeFine.getValue() / (100. * m_pRateRange->get()));
        slotRateSliderChanged(m_pRateSlider->get());
    }
}

void RateControl::slotControlRatePermUp(double v) {
    // Adjusts temp rate up if button pressed
    if (v > 0.0) {
        m_pRateSlider->set(m_pRateSlider->get() +
                m_pRateDir->get() * m_dPermanentRateChangeCoarse.getValue() / (100. * m_pRateRange->get()));
        slotRateSliderChanged(m_pRateSlider->get());
    }
}

void RateControl::slotControlRatePermUpSmall(double v) {
    // Adjusts temp rate up if button pressed
    if (v > 0.0) {
        m_pRateSlider->set(m_pRateSlider->get() +
                           m_pRateDir->get() * m_dPermanentRateChangeFine.getValue() / (100. * m_pRateRange->get()));
        slotRateSliderChanged(m_pRateSlider->get());
    }
}

double RateControl::getWheelFactor() const {
    return m_pWheel->get();
}

double RateControl::getJogFactor() const {
    // FIXME: Sensitivity should be configurable separately?
    constexpr double jogSensitivity = 0.1; // Nudges during playback
    double jogValue = m_pJog->get();

    // Since m_pJog is an accumulator, reset it since we've used its value.
    if (jogValue != 0.) {
        m_pJog->set(0.);
    }

    double jogValueFiltered = m_pJogFilter->filter(jogValue);
    double jogFactor = jogValueFiltered * jogSensitivity;

    if (util_isnan(jogValue) || util_isnan(jogFactor)) {
        jogFactor = 0.0;
    }

    return jogFactor;
}

SyncMode RateControl::getSyncMode() const {
    return syncModeFromDouble(m_syncMode.get());
}

double RateControl::calculateSpeed(double baserate,
        double speed,
        bool paused,
        std::size_t samplesPerBuffer,
        bool* pReportScratching,
        bool* pReportReverse) {
    *pReportScratching = false;
    *pReportReverse = false;

    processTempRate(samplesPerBuffer);

    double rate;
    const double searching = m_pRateSearch->get();
    if (searching != 0) {
        // If searching is in progress, it overrides everything else
        rate = searching;
    } else {
        double wheelFactor = getWheelFactor();
        double jogFactor = getJogFactor();
        bool bVinylControlEnabled = m_pVCEnabled && m_pVCEnabled->toBool();
        bool useScratch2Value = m_pScratch2Enable->toBool();

        // By default scratch2_enable is enough to determine if the user is
        // scratching or not. Moving platter controllers have to disable
        // "scratch2_indicates_scratching" if they are not scratching,
        // to allow things like key-lock.
        if (useScratch2Value && m_pScratch2Scratching->toBool()) {
            *pReportScratching = true;
        }

        if (bVinylControlEnabled) {
            if (m_pVCScratching && m_pVCScratching->toBool()) {
                *pReportScratching = true;
            }
            rate = speed;
        } else {
            double scratchFactor = m_pScratch2->get();
            // Don't trust values from m_pScratch2
            if (util_isnan(scratchFactor)) {
                scratchFactor = 0.0;
            }
            if (paused) {
                // Stopped. Wheel, jog and scratch controller all scrub through audio.
                if (useScratch2Value) {
                    rate = scratchFactor + jogFactor + wheelFactor * kWheelMultiplier;
                } else {
                    rate = jogFactor * kPausedJogMultiplier + wheelFactor;
                }
            } else {
                // The buffer is playing, so calculate the buffer rate.

                // There are four rate effects we apply: wheel, scratch, jog and temp.
                // Wheel:   a linear additive effect (no spring-back)
                // Scratch: a rate multiplier
                // Jog:     a linear additive effect whose value is filtered (springs back)
                // Temp:    pitch bend

                // New scratch behavior - overrides playback speed (and old behavior)
                if (useScratch2Value) {
                    rate = scratchFactor;
                } else {
                    // add temp rate, but don't go backwards
                    rate = math_max(speed + getTempRate(), 0.0);
                    rate += wheelFactor;
                }
                rate += jogFactor;
            }
        }

        double currentSample = frameInfo().currentPosition.toEngineSamplePos();
        // Let PositionScratchController also know if the play pos wrapped around
        // (beatloop or track repeat) so it can correctly interpret the sample position delta.
        m_pScratchController->process(currentSample,
                rate,
                samplesPerBuffer,
                baserate,
                m_wrapAroundCount,
                m_jumpPos,
                m_targetPos);
        // Reset count after use.
        m_wrapAroundCount = 0;

        // If waveform scratch is enabled, override all other controls
        if (m_pScratchController->isEnabled()) {
            rate = m_pScratchController->getRate();
            *pReportScratching = true;
        } else {
            // If sync lock is on, respond to it -- but vinyl and scratch mode always override.
            if (toSynchronized(getSyncMode()) && !paused &&
                    !bVinylControlEnabled && !useScratch2Value) {
                if (m_pBpmControl == nullptr) {
                    qDebug() << "ERROR: calculateRate m_pBpmControl is null during sync lock";
                    return 1.0;
                }

                double userTweak = 0.0;
                if (!*pReportScratching) {
                    // Only report user tweak if the user is not scratching.
                    userTweak = getTempRate() + wheelFactor + jogFactor;
                }
                rate = m_pBpmControl->calcSyncedRate(userTweak);
            }
            // If we are reversing (and not scratching,) flip the rate.  This is ok even when syncing.
            // Reverse with vinyl is only ok if absolute mode isn't on.
            int vcmode = m_pVCMode ? static_cast<int>(m_pVCMode->get()) : MIXXX_VCMODE_ABSOLUTE;
            // TODO(owen): Instead of just ignoring reverse mode, should we
            // disable absolute mode instead?
            if (m_pReverseButton->toBool() && !m_pScratch2Enable->toBool() &&
                    (!bVinylControlEnabled ||
                            vcmode != MIXXX_VCMODE_ABSOLUTE)) {
                rate = -rate;
                *pReportReverse = true;
            }
        }
    }

    if (!m_rampAllowed) {
        return rate;
    }

    if (m_resetRampRequested.exchange(false)) {
        m_platterSpeed = 1.0;
        m_spinningDown = false;
        m_latchedPlay = false;
        m_prevPaused = true;
    }

    // Physical SL-1210 spin-up / spin-down ramp
    // EngineBuffer passes paused = !play, and play is also 1 during cue
    // preview, so play can't tell a Play press from a cue preview.
    // play_latched (owned by CueControl) is 1 only for a latched play.
    const bool playLatched = m_pPlayLatched && m_pPlayLatched->toBool();
    const bool seeked = m_seekedSinceLastBuffer;
    m_seekedSinceLastBuffer = false;

    // Speed set by hand: scratching (spinny, waveform, controller platter),
    // jog/wheel scrubbing while paused, searching or vinyl control
    const bool manualSpeed = searching != 0 || *pReportScratching ||
            (paused && rate != 0.0) ||
            m_pScratch2Enable->toBool() ||
            (m_pVCEnabled && m_pVCEnabled->toBool());

    // play_latched drops to 0 slightly before play when pausing, so the
    // last playing buffer may already see 0. Remember it per play period.
    if (!paused && playLatched) {
        m_latchedPlay = true;
    }

    if (manualSpeed) {
        // Speed set by hand: cancel a running ramp
        m_platterSpeed = 1.0;
        m_spinningDown = false;
    } else if (!paused) {
        if (m_prevPaused) {
            if (playLatched && m_pSpinupEnabled->toBool()) {
                // Started by a latched Play: spin up. If the platter is still
                // spinning down, continue from its current speed.
                if (!m_spinningDown) {
                    m_platterSpeed = 0.0;
                }
                latchRampSettings(*m_pSpinupTime);
            } else {
                // Cue preview, or ramp up disabled: start instantly
                m_platterSpeed = 1.0;
            }
            m_spinningDown = false;
        } else if (seeked) {
            // qDebug() << "RateControl -> spin-up cut by seek, platter was" << m_platterSpeed;
            // Jump while playing (hotcue, cue preview, beatjump, ...):
            // a running spin-up is cut, playback continues at full speed
            m_platterSpeed = 1.0;
        }
    } else {
        if (m_pSpindownEnabled->toBool() && !m_prevPaused && m_latchedPlay && !seeked) {
            // A latched play was paused: spin down from the current speed
            m_spinningDown = true;
            latchRampSettings(*m_pSpindownTime);
        }
        if (seeked) {
            m_spinningDown = false;
        }
        if (!m_spinningDown) {
            m_platterSpeed = 1.0;
        }
    }
    if (paused) {
        m_latchedPlay = false;
    }
    m_prevPaused = paused;

    const double dt = static_cast<double>(samplesPerBuffer) / m_pSampleRate.get() / 2;

    if (m_spinningDown) {
        // Spin-down time setting stretches the curve, 1.0 = SL-1210
        const double downDt = dt / m_rampTimeFactor;
        if (m_rampDownMode == RampDownMode::PowerCut) {
            // Power cut: the platter coasts without brake
            m_platterSpeed -= (coastFriction(m_rampRpm) + kCoastDrag * m_platterSpeed) * downDt;
        } else {
            // Electronic brake: constant deceleration until standstill
            m_platterSpeed -= downDt / brakeTime(m_rampRpm);
        }
        if (m_platterSpeed <= 0.0) {
            m_platterSpeed = 1.0;
            m_spinningDown = false;
            return rate; // paused rate (0 unless jog scrubbing)
        }
        return m_lastTargetRate * m_platterSpeed;
    }

    if (m_platterSpeed < 1.0) {
        // Spin-up time setting stretches the curve, 1.0 = SL-1210
        const double upDt = dt / m_rampTimeFactor;
        m_platterSpeed += spinupTorque(m_rampRpm) * upDt;

        // Soft saturation: motor settling into quartz lock
        if (m_platterSpeed > kQuartzLockStart) {
            m_platterSpeed += (1.0 - m_platterSpeed) * kQuartzLockGain * upDt;
        }
        if (m_platterSpeed < 1.0) {
            // Like a real turntable, spin up to the pitched speed
            m_lastTargetRate = rate;
            return rate * m_platterSpeed;
        }
        m_platterSpeed = 1.0;
    }

    if (!paused) {
        m_lastTargetRate = rate;
    }
    return rate;
}

void RateControl::processTempRate(const std::size_t bufferSamples) {
    // Code to handle temporary rate change buttons.
    // We support two behaviors, the standard ramped pitch bending
    // and pitch shift stepping, which is the old behavior.

    RampDirection rampDirection = RampDirection::None;
    if (m_pButtonRateTempUp->toBool()) {
        rampDirection = RampDirection::Up;
    } else if (m_pButtonRateTempDown->toBool()) {
        rampDirection = RampDirection::Down;
    } else if (m_pButtonRateTempUpSmall->toBool()) {
        rampDirection = RampDirection::UpSmall;
    } else if (m_pButtonRateTempDownSmall->toBool()) {
        rampDirection = RampDirection::DownSmall;
    }

    if (rampDirection != RampDirection::None) {
        if (m_eRateRampMode == RampMode::Stepping) {
            if (!m_bTempStarted) {
                m_bTempStarted = true;
                // old temporary pitch shift behavior
                double change = m_dTemporaryRateChangeCoarse.getValue() / 100.0;
                double csmall = m_dTemporaryRateChangeFine.getValue() / 100.0;

                switch (rampDirection) {
                case RampDirection::Up:
                    setRateTemp(change);
                    break;
                case RampDirection::Down:
                    setRateTemp(-change);
                    break;
                case RampDirection::UpSmall:
                    setRateTemp(csmall);
                    break;
                case RampDirection::DownSmall:
                    setRateTemp(-csmall);
                    break;
                case RampDirection::None:
                default:
                    DEBUG_ASSERT(false);
                }
            }
        } else if (m_eRateRampMode == RampMode::Linear) {
            if (!m_bTempStarted) {
                m_bTempStarted = true;
                double latrate = bufferSamples / m_pSampleRate.get();
                m_dRateTempRampChange = latrate / (m_iRateRampSensitivity / 100.0);
            }

            switch (rampDirection) {
            case RampDirection::Up:
            case RampDirection::UpSmall:
                addRateTemp(m_dRateTempRampChange * m_pRateRange->get());
                break;
            case RampDirection::Down:
            case RampDirection::DownSmall:
                subRateTemp(m_dRateTempRampChange * m_pRateRange->get());
                break;
            case RampDirection::None:
            default:
                DEBUG_ASSERT(false);
            }
        }
    } else if (m_bTempStarted) {
        m_bTempStarted = false;
        resetRateTemp();
    }
}

double RateControl::getTempRate() {
    // qDebug() << m_tempRateRatio;
    return m_tempRateRatio;
}

void RateControl::setRateTemp(double v) {
    m_tempRateRatio = v;
    if (m_tempRateRatio < -1.0) {
        m_tempRateRatio = -1.0;
    } else if (m_tempRateRatio > 1.0) {
        m_tempRateRatio = 1.0;
    } else if (util_isnan(m_tempRateRatio)) {
        m_tempRateRatio = 0;
    }
}

void RateControl::addRateTemp(double v)
{
    setRateTemp(m_tempRateRatio + v);
}

void RateControl::subRateTemp(double v)
{
    setRateTemp(m_tempRateRatio - v);
}

void RateControl::resetRateTemp(void)
{
    setRateTemp(0.0);
}

bool RateControl::isReverseButtonPressed() {
    if (m_pReverseButton) {
        return m_pReverseButton->toBool();
    }
    return false;
}

void RateControl::notifyWrapAround(mixxx::audio::FramePos triggerPos,
        mixxx::audio::FramePos targetPos) {
    VERIFY_OR_DEBUG_ASSERT(triggerPos.isValid() && targetPos.isValid()) {
        m_wrapAroundCount = 0;
        // no need to reset the position, they're not used if count is 0.
        return;
    }
    m_wrapAroundCount++;
    m_jumpPos = triggerPos;
    m_targetPos = targetPos;
}

void RateControl::notifySeek(mixxx::audio::FramePos position) {
    // qDebug() << "RateControl::notifySeek" << getGroup() << position;
    m_pScratchController->notifySeek(position);
    // A pause combined with a jump (cue, start_stop, track load) must not
    // spin down from the new position
    m_seekedSinceLastBuffer = true;
}

void RateControl::resetPositionScratchController() {
    // Resets the scratch state to avoid engine freeze due to insanley high rate
    // reported on track load while scratching.
    // https://github.com/mixxxdj/mixxx/issues/15082
    m_pScratchController->reset();
    // Track changed: start the next track with a fresh motor state.
    // This runs in a worker thread, so calculateSpeed() applies it.
    requestRampReset();
}

void RateControl::setEngineMixer(EngineMixer* pEngineMixer) {
    EngineControl::setEngineMixer(pEngineMixer);
    // play_latched is owned by CueControl, which EngineBuffer creates after
    // RateControl. EngineBuffer calls setEngineMixer() once all controls
    // exist, outside the audio callback.
    m_pPlayLatched = std::make_unique<PollingControlProxy>(getGroup(),
            QStringLiteral("play_latched"),
            ControlFlag::AllowMissingOrInvalid);
}

void RateControl::requestRampReset() {
    m_resetRampRequested = true;
}

void RateControl::latchRampSettings(const ControlObject& timeControl) {
    // A running ramp keeps its settings, changes apply to the next one
    m_rampRpm = static_cast<RampRpmMode>(static_cast<int>(m_pPlatterRpm->get()));
    m_rampDownMode = static_cast<RampDownMode>(static_cast<int>(m_pSpindownMode->get()));
    m_rampTimeFactor = std::clamp(timeControl.get(), kMinRampTimeFactor, kMaxRampTimeFactor);
}
