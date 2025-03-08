#include "engine/controls/ratecontrol.h"

#include <QtDebug>

#include "control/controlobject.h"
#include "control/controlpotmeter.h"
#include "control/controlproxy.h"
#include "control/controlpushbutton.h"
#include "control/controlttrotary.h"
#include "engine/controls/bpmcontrol.h"
#include "engine/controls/enginecontrol.h"
#include "engine/positionscratchcontroller.h"
#include "moc_ratecontrol.cpp"
#include "util/rotary.h"
#include "vinylcontrol/defs_vinylcontrol.h"

namespace {
constexpr int kRateSensitivityMin = 100;
constexpr int kRateSensitivityMax = 2500;
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
          // Vinyl control
          m_pVCEnabled(ControlObject::getControl(ConfigKey(
                  getGroup(), QStringLiteral("vinylcontrol_enabled")))),
          m_pVCScratching(ControlObject::getControl(ConfigKey(
                  getGroup(), QStringLiteral("vinylcontrol_scratching")))),
          m_pVCMode(ControlObject::getControl(
                  ConfigKey(getGroup(), QStringLiteral("vinylcontrol_mode")))),
          m_syncMode(group, QStringLiteral("sync_mode")),
          m_slipEnabled(group, QStringLiteral("slip_enabled")),
          m_wrapAroundCount(0),
          m_jumpPos(mixxx::audio::FramePos()),
          m_targetPos(mixxx::audio::FramePos()),
          m_bTempStarted(false),
          m_tempRateRatio(0.0),
          m_dRateTempRampChange(0.0) {
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
            if (m_pVCScratching->toBool()) {
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
    m_pScratchController->notifySeek(position);
}
