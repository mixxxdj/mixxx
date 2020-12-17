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
#include "util/math.h"
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

RateControl::RateControl(const QString& group,
        UserSettingsPointer pConfig)
        : EngineControl(group, pConfig),
          m_pBpmControl(nullptr),
          m_bTempStarted(false),
          m_tempRateRatio(0.0),
          m_dRateTempRampChange(0.0) {
    m_pScratchController = new PositionScratchController(group);

    // This is the resulting rate ratio that can be used for display or calculations.
    // The track original rate ratio is 1.
    m_pRateRatio = new ControlObject(ConfigKey(group, "rate_ratio"),
                  true, false, false, 1.0);
    connect(m_pRateRatio, &ControlObject::valueChanged,
            this, &RateControl::slotRateRatioChanged,
            Qt::DirectConnection);

    m_pRateDir = new ControlObject(ConfigKey(group, "rate_dir"));
    connect(m_pRateDir, &ControlObject::valueChanged,
            this, &RateControl::slotRateRangeChanged,
            Qt::DirectConnection);
    m_pRateRange = new ControlPotmeter(
            ConfigKey(group, "rateRange"), 0.01, 4.00);
    connect(m_pRateRange, &ControlObject::valueChanged,
            this, &RateControl::slotRateRangeChanged,
            Qt::DirectConnection);

    // Allow rate slider to go out of bounds so that master sync rate
    // adjustments are not capped.
    m_pRateSlider = new ControlPotmeter(
            ConfigKey(group, "rate"), -1.0, 1.0, true);
    connect(m_pRateSlider, &ControlObject::valueChanged,
            this, &RateControl::slotRateSliderChanged,
            Qt::DirectConnection);

    // Search rate. Rate used when searching in sound. This overrules the
    // playback rate
    m_pRateSearch = new ControlPotmeter(ConfigKey(group, "rateSearch"), -300., 300.);

    // Reverse button
    m_pReverseButton = new ControlPushButton(ConfigKey(group, "reverse"));
    m_pReverseButton->set(0);

    // Forward button
    m_pForwardButton = new ControlPushButton(ConfigKey(group, "fwd"));
    connect(m_pForwardButton, &ControlObject::valueChanged,
            this, &RateControl::slotControlFastForward,
            Qt::DirectConnection);
    m_pForwardButton->set(0);

    // Back button
    m_pBackButton = new ControlPushButton(ConfigKey(group, "back"));
    connect(m_pBackButton, &ControlObject::valueChanged,
            this, &RateControl::slotControlFastBack,
            Qt::DirectConnection);
    m_pBackButton->set(0);

    m_pReverseRollButton = new ControlPushButton(ConfigKey(group, "reverseroll"));
    connect(m_pReverseRollButton, &ControlObject::valueChanged,
            this, &RateControl::slotReverseRollActivate,
            Qt::DirectConnection);

    m_pSlipEnabled = new ControlProxy(group, "slip_enabled", this);

    m_pVCEnabled = ControlObject::getControl(ConfigKey(getGroup(), "vinylcontrol_enabled"));
    m_pVCScratching = ControlObject::getControl(ConfigKey(getGroup(), "vinylcontrol_scratching"));
    m_pVCMode = ControlObject::getControl(ConfigKey(getGroup(), "vinylcontrol_mode"));

    // Permanent rate-change buttons
    m_pButtonRatePermDown =
        new ControlPushButton(ConfigKey(group,"rate_perm_down"));
    connect(m_pButtonRatePermDown, &ControlObject::valueChanged,
            this, &RateControl::slotControlRatePermDown,
            Qt::DirectConnection);

    m_pButtonRatePermDownSmall =
        new ControlPushButton(ConfigKey(group,"rate_perm_down_small"));
    connect(m_pButtonRatePermDownSmall, &ControlObject::valueChanged,
            this, &RateControl::slotControlRatePermDownSmall,
            Qt::DirectConnection);

    m_pButtonRatePermUp =
        new ControlPushButton(ConfigKey(group,"rate_perm_up"));
    connect(m_pButtonRatePermUp, &ControlObject::valueChanged,
            this, &RateControl::slotControlRatePermUp,
            Qt::DirectConnection);

    m_pButtonRatePermUpSmall =
        new ControlPushButton(ConfigKey(group,"rate_perm_up_small"));
    connect(m_pButtonRatePermUpSmall, &ControlObject::valueChanged,
            this, &RateControl::slotControlRatePermUpSmall,
            Qt::DirectConnection);

    // Temporary rate-change buttons
    m_pButtonRateTempDown =
        new ControlPushButton(ConfigKey(group,"rate_temp_down"));
    m_pButtonRateTempDownSmall =
        new ControlPushButton(ConfigKey(group,"rate_temp_down_small"));
    m_pButtonRateTempUp =
        new ControlPushButton(ConfigKey(group,"rate_temp_up"));
    m_pButtonRateTempUpSmall =
        new ControlPushButton(ConfigKey(group,"rate_temp_up_small"));

    // We need the sample rate so we can guesstimate something close
    // what latency is.
    m_pSampleRate = ControlObject::getControl(ConfigKey("[Master]","samplerate"));

    // Wheel to control playback position/speed
    m_pWheel = new ControlTTRotary(ConfigKey(group, "wheel"));

    // Scratch controller, this is an accumulator which is useful for
    // controllers that return individual +1 or -1s, these get added up and
    // cleared when we read
    m_pScratch2 = new ControlObject(ConfigKey(group, "scratch2"));

    // Scratch enable toggle
    m_pScratch2Enable = new ControlPushButton(ConfigKey(group, "scratch2_enable"));
    m_pScratch2Enable->set(0);

    m_pScratch2Scratching = new ControlPushButton(ConfigKey(group,
                                                            "scratch2_indicates_scratching"));
    // Enable by default, because it was always scratching before introducing
    // this control.
    m_pScratch2Scratching->set(1.0);


    m_pJog = new ControlObject(ConfigKey(group, "jog"));
    m_pJogFilter = new Rotary();
    // FIXME: This should be dependent on sample rate/block size or something
    m_pJogFilter->setFilterLength(25);

//     // Update Internal Settings
//     // Set Pitchbend Mode
//     m_eRateRampMode = static_cast<RampMode>(
//         getConfig()->getValue(ConfigKey("[Controls]","RateRamp"),
//                               static_cast<int>(RampMode::Stepping)));

//     // Set the Sensitivity
//     m_iRateRampSensitivity =
//             getConfig()->getValueString(ConfigKey("[Controls]","RateRampSensitivity")).toInt();

    m_pSyncMode = new ControlProxy(group, "sync_mode", this);
}

RateControl::~RateControl() {
    delete m_pRateRatio;
    delete m_pRateSlider;
    delete m_pRateRange;
    delete m_pRateDir;
    delete m_pSyncMode;

    delete m_pRateSearch;

    delete m_pReverseButton;
    delete m_pReverseRollButton;
    delete m_pForwardButton;
    delete m_pBackButton;

    delete m_pButtonRateTempDown;
    delete m_pButtonRateTempDownSmall;
    delete m_pButtonRateTempUp;
    delete m_pButtonRateTempUpSmall;
    delete m_pButtonRatePermDown;
    delete m_pButtonRatePermDownSmall;
    delete m_pButtonRatePermUp;
    delete m_pButtonRatePermUpSmall;

    delete m_pWheel;
    delete m_pScratch2;
    delete m_pScratch2Scratching;
    delete m_pScratch2Enable;
    delete m_pJog;
    delete m_pJogFilter;
    delete m_pScratchController;
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
        m_pSlipEnabled->set(1);
        m_pReverseButton->set(1);
    } else {
        m_pReverseButton->set(0);
        m_pSlipEnabled->set(0);
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
    const double jogSensitivity = 0.1;  // Nudges during playback
    double jogValue = m_pJog->get();

    // Since m_pJog is an accumulator, reset it since we've used its value.
    if (jogValue != 0.) {
        m_pJog->set(0.);
    }

    double jogValueFiltered = m_pJogFilter->filter(jogValue);
    double jogFactor = jogValueFiltered * jogSensitivity;

    if (isnan(jogValue) || isnan(jogFactor)) {
        jogFactor = 0.0;
    }

    return jogFactor;
}

SyncMode RateControl::getSyncMode() const {
    return syncModeFromDouble(m_pSyncMode->get());
}

double RateControl::calculateSpeed(double baserate, double speed, bool paused,
                                   int iSamplesPerBuffer,
                                   bool* pReportScratching,
                                   bool* pReportReverse) {
    *pReportScratching = false;
    *pReportReverse = false;

    processTempRate(iSamplesPerBuffer);

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
            if (isnan(scratchFactor)) {
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
                // Wheel: a linear additive effect (no spring-back)
                // Scratch: a rate multiplier
                // Jog: a linear additive effect whose value is filtered (springs back)
                // Temp: pitch bend

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

        double currentSample = getSampleOfTrack().current;
        m_pScratchController->process(currentSample, rate, iSamplesPerBuffer, baserate);

        // If waveform scratch is enabled, override all other controls
        if (m_pScratchController->isEnabled()) {
            rate = m_pScratchController->getRate();
            *pReportScratching = true;
        } else {
            // If master sync is on, respond to it -- but vinyl and scratch mode always override.
            if (getSyncMode() == SYNC_FOLLOWER && !paused &&
                    !bVinylControlEnabled && !useScratch2Value) {
                if (m_pBpmControl == nullptr) {
                    qDebug() << "ERROR: calculateRate m_pBpmControl is null during master sync";
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

void RateControl::processTempRate(const int bufferSamples) {
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
            if (rampDirection != RampDirection::None) {
                if (!m_bTempStarted) {
                    m_bTempStarted = true;
                    double latrate = ((double)bufferSamples / (double)m_pSampleRate->get());
                    m_dRateTempRampChange = (latrate / ((double)m_iRateRampSensitivity / 100.));
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
    } else if (isnan(m_tempRateRatio)) {
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

void RateControl::notifySeek(double playPos) {
    m_pScratchController->notifySeek(playPos);
    EngineControl::notifySeek(playPos);
}

bool RateControl::isReverseButtonPressed() {
    if (m_pReverseButton) {
        return m_pReverseButton->toBool();
    }
    return false;
}
