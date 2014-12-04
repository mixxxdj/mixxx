// ratecontrol.cpp
// Created 7/4/2009 by RJ Ryan (rryan@mit.edu)

#include "controlobject.h"
#include "controlpushbutton.h"
#include "controlpotmeter.h"
#include "controlttrotary.h"
#include "controlobjectslave.h"
#include "rotary.h"
#include "util/math.h"
#include "vinylcontrol/defs_vinylcontrol.h"

#include "engine/bpmcontrol.h"
#include "engine/enginecontrol.h"
#include "engine/ratecontrol.h"
#include "engine/positionscratchcontroller.h"

#include <QtDebug>

// Static default values for rate buttons (percents). Note that these are not
// actually used -- the preferences code sets the values that are stored in the
// user's configuration. These are just fail safe defaults.
double RateControl::m_dTemp = 4.00; //(eg. 4.00%)
double RateControl::m_dTempSmall = 2.00;
double RateControl::m_dPerm = 0.50;
double RateControl::m_dPermSmall = 0.05;

int RateControl::m_iRateRampSensitivity = 250;
const double RateControl::kWheelMultiplier = 40.0;
const double RateControl::kPausedJogMultiplier = 18.0;
enum RateControl::RATERAMP_MODE RateControl::m_eRateRampMode = RateControl::RATERAMP_STEP;

RateControl::RateControl(QString group,
                         ConfigObject<ConfigValue>* _config)
    : EngineControl(group, _config),
      m_pBpmControl(NULL),
      m_ePbCurrent(0),
      m_ePbPressed(0),
      m_bTempStarted(false),
      m_dTempRateChange(0.0),
      m_dRateTemp(0.0),
      m_eRampBackMode(RATERAMP_RAMPBACK_NONE),
      m_dRateTempRampbackChange(0.0) {
    m_pScratchController = new PositionScratchController(group);

    m_pRateDir = new ControlObject(ConfigKey(group, "rate_dir"));
    m_pRateRange = new ControlObject(ConfigKey(group, "rateRange"));
    // Allow rate slider to go out of bounds so that master sync rate
    // adjustments are not capped.
    m_pRateSlider = new ControlPotmeter(ConfigKey(group, "rate"),
                                        -1.0, 1.0, true);

    // Search rate. Rate used when searching in sound. This overrules the
    // playback rate
    m_pRateSearch = new ControlPotmeter(ConfigKey(group, "rateSearch"), -300., 300.);

    // Reverse button
    m_pReverseButton = new ControlPushButton(ConfigKey(group, "reverse"));
    m_pReverseButton->set(0);

    // Forward button
    m_pForwardButton = new ControlPushButton(ConfigKey(group, "fwd"));
    connect(m_pForwardButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlFastForward(double)),
            Qt::DirectConnection);
    m_pForwardButton->set(0);

    // Back button
    m_pBackButton = new ControlPushButton(ConfigKey(group, "back"));
    connect(m_pBackButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlFastBack(double)),
            Qt::DirectConnection);
    m_pBackButton->set(0);

    m_pReverseRollButton = new ControlPushButton(ConfigKey(group, "reverseroll"));
    connect(m_pReverseRollButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotReverseRollActivate(double)),
            Qt::DirectConnection);

    m_pSlipEnabled = new ControlObjectSlave(group, "slip_enabled", this);

    m_pVCEnabled = ControlObject::getControl(ConfigKey(getGroup(), "vinylcontrol_enabled"));
    m_pVCScratching = ControlObject::getControl(ConfigKey(getGroup(), "vinylcontrol_scratching"));
    m_pVCMode = ControlObject::getControl(ConfigKey(getGroup(), "vinylcontrol_mode"));
    m_pVCRate = ControlObject::getControl(ConfigKey(getGroup(), "vinylcontrol_rate"));

    // Permanent rate-change buttons
    buttonRatePermDown =
        new ControlPushButton(ConfigKey(group,"rate_perm_down"));
    connect(buttonRatePermDown, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlRatePermDown(double)),
            Qt::DirectConnection);

    buttonRatePermDownSmall =
        new ControlPushButton(ConfigKey(group,"rate_perm_down_small"));
    connect(buttonRatePermDownSmall, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlRatePermDownSmall(double)),
            Qt::DirectConnection);

    buttonRatePermUp =
        new ControlPushButton(ConfigKey(group,"rate_perm_up"));
    connect(buttonRatePermUp, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlRatePermUp(double)),
            Qt::DirectConnection);

    buttonRatePermUpSmall =
        new ControlPushButton(ConfigKey(group,"rate_perm_up_small"));
    connect(buttonRatePermUpSmall, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlRatePermUpSmall(double)),
            Qt::DirectConnection);

    // Temporary rate-change buttons
    buttonRateTempDown =
        new ControlPushButton(ConfigKey(group,"rate_temp_down"));
    connect(buttonRateTempDown, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlRateTempDown(double)),
            Qt::DirectConnection);

    buttonRateTempDownSmall =
        new ControlPushButton(ConfigKey(group,"rate_temp_down_small"));
    connect(buttonRateTempDownSmall, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlRateTempDownSmall(double)),
            Qt::DirectConnection);

    buttonRateTempUp =
        new ControlPushButton(ConfigKey(group,"rate_temp_up"));
    connect(buttonRateTempUp, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlRateTempUp(double)),
            Qt::DirectConnection);

    buttonRateTempUpSmall =
        new ControlPushButton(ConfigKey(group,"rate_temp_up_small"));
    connect(buttonRateTempUpSmall, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlRateTempUpSmall(double)),
            Qt::DirectConnection);

    // We need the sample rate so we can guesstimate something close
    // what latency is.
    m_pSampleRate = ControlObject::getControl(ConfigKey("[Master]","samplerate"));

    // Wheel to control playback position/speed
    m_pWheel = new ControlTTRotary(ConfigKey(group, "wheel"));

    // Scratch controller, this is an accumulator which is useful for
    // controllers that return individiual +1 or -1s, these get added up and
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

    // Update Internal Settings
    // Set Pitchbend Mode
    m_eRateRampMode = (RateControl::RATERAMP_MODE)
            getConfig()->getValueString(ConfigKey("[Controls]","RateRamp")).toInt();

    // Set the Sensitivity
    m_iRateRampSensitivity =
            getConfig()->getValueString(ConfigKey("[Controls]","RateRampSensitivity")).toInt();

    m_pSyncMode = new ControlObjectSlave(group, "sync_mode", this);
}

RateControl::~RateControl() {
    delete m_pRateSlider;
    delete m_pRateRange;
    delete m_pRateDir;
    delete m_pSyncMode;

    delete m_pRateSearch;

    delete m_pReverseButton;
    delete m_pReverseRollButton;
    delete m_pForwardButton;
    delete m_pBackButton;

    delete buttonRateTempDown;
    delete buttonRateTempDownSmall;
    delete buttonRateTempUp;
    delete buttonRateTempUpSmall;
    delete buttonRatePermDown;
    delete buttonRatePermDownSmall;
    delete buttonRatePermUp;
    delete buttonRatePermUpSmall;

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

void RateControl::setRateRamp(bool linearMode)
{
    m_eRateRampMode = linearMode ?
            RateControl::RATERAMP_LINEAR : RateControl::RATERAMP_STEP;
}

void RateControl::setRateRampSensitivity(int sense)
{
    // Reverse the actual sensitivity value passed.
    // That way the gui works in an intuitive manner.
    sense = RATE_SENSITIVITY_MAX - sense + RATE_SENSITIVITY_MIN;
    if (sense < RATE_SENSITIVITY_MIN) {
        m_iRateRampSensitivity = RATE_SENSITIVITY_MIN;
    } else if (sense > RATE_SENSITIVITY_MAX) {
        m_iRateRampSensitivity = RATE_SENSITIVITY_MAX;
    } else {
        m_iRateRampSensitivity = sense;
    }
}

void RateControl::setTemp(double v) {
    m_dTemp = v;
}

void RateControl::setTempSmall(double v) {
    m_dTempSmall = v;
}

void RateControl::setPerm(double v) {
    m_dPerm = v;
}

void RateControl::setPermSmall(double v) {
    m_dPermSmall = v;
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

void RateControl::slotControlFastForward(double v)
{
    //qDebug() << "slotControlFastForward(" << v << ")";
    if (v==0.)
        m_pRateSearch->set(0.);
    else
        m_pRateSearch->set(4.);
}

void RateControl::slotControlFastBack(double v)
{
    //qDebug() << "slotControlFastBack(" << v << ")";
    if (v==0.)
        m_pRateSearch->set(0.);
    else
        m_pRateSearch->set(-4.);
}

void RateControl::slotControlRatePermDown(double)
{
    // Adjusts temp rate down if button pressed
    if (buttonRatePermDown->get())
        m_pRateSlider->set(m_pRateSlider->get() -
                           m_pRateDir->get() * m_dPerm / (100. * m_pRateRange->get()));
}

void RateControl::slotControlRatePermDownSmall(double)
{
    // Adjusts temp rate down if button pressed
    if (buttonRatePermDownSmall->get())
        m_pRateSlider->set(m_pRateSlider->get() -
                           m_pRateDir->get() * m_dPermSmall / (100. * m_pRateRange->get()));
}

void RateControl::slotControlRatePermUp(double)
{
    // Adjusts temp rate up if button pressed
    if (buttonRatePermUp->get()) {
        m_pRateSlider->set(m_pRateSlider->get() +
                           m_pRateDir->get() * m_dPerm / (100. * m_pRateRange->get()));
    }
}

void RateControl::slotControlRatePermUpSmall(double)
{
    // Adjusts temp rate up if button pressed
    if (buttonRatePermUpSmall->get())
        m_pRateSlider->set(m_pRateSlider->get() +
                           m_pRateDir->get() * m_dPermSmall / (100. * m_pRateRange->get()));
}

void RateControl::slotControlRateTempDown(double)
{
    // Set the state of the Temporary button. Logic is handled in ::process()
    if (buttonRateTempDown->get() && !(m_ePbPressed & RateControl::RATERAMP_DOWN))
    {
        m_ePbPressed |= RateControl::RATERAMP_DOWN;
        m_ePbCurrent = RateControl::RATERAMP_DOWN;
    }
    else if (!buttonRateTempDown->get())
    {
        m_ePbPressed &= ~RateControl::RATERAMP_DOWN;
        m_ePbCurrent = m_ePbPressed;
    }
}

void RateControl::slotControlRateTempDownSmall(double)
{
    // Set the state of the Temporary button. Logic is handled in ::process()
    if (buttonRateTempDownSmall->get() && !(m_ePbPressed & RateControl::RATERAMP_DOWN))
    {
        m_ePbPressed |= RateControl::RATERAMP_DOWN;
        m_ePbCurrent = RateControl::RATERAMP_DOWN;
    }
    else if (!buttonRateTempDownSmall->get())
    {
        m_ePbPressed &= ~RateControl::RATERAMP_DOWN;
        m_ePbCurrent = m_ePbPressed;
    }
}

void RateControl::slotControlRateTempUp(double)
{
    // Set the state of the Temporary button. Logic is handled in ::process()
    if (buttonRateTempUp->get() && !(m_ePbPressed & RateControl::RATERAMP_UP))
    {
        m_ePbPressed |= RateControl::RATERAMP_UP;
        m_ePbCurrent = RateControl::RATERAMP_UP;
    }
    else if (!buttonRateTempUp->get())
    {
        m_ePbPressed &= ~RateControl::RATERAMP_UP;
        m_ePbCurrent = m_ePbPressed;
    }
}

void RateControl::slotControlRateTempUpSmall(double)
{
    // Set the state of the Temporary button. Logic is handled in ::process()
    if (buttonRateTempUpSmall->get() && !(m_ePbPressed & RateControl::RATERAMP_UP))
    {
        m_ePbPressed |= RateControl::RATERAMP_UP;
        m_ePbCurrent = RateControl::RATERAMP_UP;
    }
    else if (!buttonRateTempUpSmall->get())
    {
        m_ePbPressed &= ~RateControl::RATERAMP_UP;
        m_ePbCurrent = m_ePbPressed;
    }
}

void RateControl::trackLoaded(TrackPointer pTrack) {
    if (m_pTrack) {
        trackUnloaded(m_pTrack);
    }
    m_pTrack = pTrack;
}

void RateControl::trackUnloaded(TrackPointer pTrack) {
    Q_UNUSED(pTrack);
    m_pTrack.clear();
}

double RateControl::getRawRate() const {
    return m_pRateSlider->get() *
        m_pRateRange->get() *
        m_pRateDir->get();
}

double RateControl::getWheelFactor() const {
    return m_pWheel->get();
}

double RateControl::getJogFactor() const {
    // FIXME: Sensitivity should be configurable separately?
    const double jogSensitivity = 0.1;  // Nudges during playback
    double jogValue = m_pJog->get();

    // Since m_pJog is an accumulator, reset it since we've used its value.
    if(jogValue != 0.)
        m_pJog->set(0.);

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

double RateControl::calculateSpeed(double baserate, bool paused,
                                  int iSamplesPerBuffer,
                                  bool* reportScratching) {
    *reportScratching = false;
    double rate = (paused ? 0 : 1.0);
    double searching = m_pRateSearch->get();
    if (searching) {
        // If searching is in progress, it overrides everything else
        rate = searching;
    } else {
        double wheelFactor = getWheelFactor();
        double jogFactor = getJogFactor();
        bool bVinylControlEnabled = m_pVCEnabled && m_pVCEnabled->get() > 0.0;
        bool useScratch2Value = m_pScratch2Enable->get() != 0;

        // By default scratch2_enable is enough to determine if the user is
        // scratching or not. Moving platter controllers have to disable
        // "scratch2_indicates_scratching" if they are not scratching,
        // to allow things like key-lock.
        if (useScratch2Value && m_pScratch2Scratching->get()) {
            *reportScratching = true;
        }

        if (bVinylControlEnabled) {
            if (m_pVCScratching && m_pVCScratching->get() > 0.0) {
                *reportScratching = true;
            }
            rate = m_pVCRate->get();
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

                    rate = 1. + getRawRate() + getTempRate();
                    rate += wheelFactor;

                }
                rate += jogFactor;
            }
        }

        double currentSample = getCurrentSample();
        m_pScratchController->process(currentSample, rate, iSamplesPerBuffer, baserate);

        // If waveform scratch is enabled, override all other controls
        if (m_pScratchController->isEnabled()) {
            rate = m_pScratchController->getRate();
            *reportScratching = true;
        } else {
            // If master sync is on, respond to it -- but vinyl and scratch mode always override.
            if (getSyncMode() == SYNC_FOLLOWER && !paused &&
                !bVinylControlEnabled && !useScratch2Value) {
                if (m_pBpmControl == NULL) {
                    qDebug() << "ERROR: calculateRate m_pBpmControl is null during master sync";
                    return 1.0;
                }

                double userTweak = 0.0;
                if (!*reportScratching) {
                    // Only report user tweak if the user is not scratching.
                    userTweak = getTempRate() + wheelFactor + jogFactor;
                }
                rate = m_pBpmControl->calcSyncedRate(userTweak);
            }
            // If we are reversing (and not scratching,) flip the rate.  This is ok even when syncing.
            // Reverse with vinyl is only ok if absolute mode isn't on.
            int vcmode = m_pVCMode ? m_pVCMode->get() : MIXXX_VCMODE_ABSOLUTE;
            if (m_pReverseButton->get()
                    && !m_pScratch2Enable->get()
                    && (!bVinylControlEnabled || vcmode != MIXXX_VCMODE_ABSOLUTE)) {
                rate = -rate;
            }
        }
    }

    return rate;
}

double RateControl::process(const double rate,
                            const double currentSample,
                            const double totalSamples,
                            const int bufferSamples)
{
    Q_UNUSED(rate);
    Q_UNUSED(currentSample);
    Q_UNUSED(totalSamples);
    /*
     * Code to handle temporary rate change buttons.
     *
     * We support two behaviors, the standard ramped pitch bending
     * and pitch shift stepping, which is the old behavior.
     */

    /*
     * Initialize certain values necessary for pitchbending. Most of this
     * code should be handled inside a slot, but we'd need to connect to
     * the troublesome Latency ControlObject... Either the Master or Soundcard
     * one.
     */

    double latrate = ((double)bufferSamples / (double)m_pSampleRate->get());


    if ((m_ePbPressed) && (!m_bTempStarted)) {
        m_bTempStarted = true;

        if (m_eRateRampMode == RATERAMP_STEP) {
            // old temporary pitch shift behavior
            double range = m_pRateRange->get();

            // Avoid Division by Zero
            if (range == 0) {
                qDebug() << "Avoiding a Division by Zero in RATERAMP_STEP code";
                return kNoTrigger;
            }

            double change = m_pRateDir->get() * m_dTemp /
                                    (100. * range);
            double csmall = m_pRateDir->get() * m_dTempSmall /
                                    (100. * range);

            if (buttonRateTempUp->get())
                addRateTemp(change);
            else if (buttonRateTempDown->get())
                subRateTemp(change);
            else if (buttonRateTempUpSmall->get())
                addRateTemp(csmall);
            else if (buttonRateTempDownSmall->get())
                subRateTemp(csmall);
        }
        else
        {
            m_dTempRateChange = ((double)latrate / ((double)m_iRateRampSensitivity / 100.));

            if (m_eRampBackMode == RATERAMP_RAMPBACK_PERIOD)
                m_dRateTempRampbackChange = 0.0;
        }

    }

    if (m_eRateRampMode == RATERAMP_LINEAR) {

        if (m_ePbCurrent)
        {
            // apply ramped pitchbending
            if (m_ePbCurrent == RateControl::RATERAMP_UP) {
                addRateTemp(m_dTempRateChange);
            } else if (m_ePbCurrent == RateControl::RATERAMP_DOWN) {
                subRateTemp(m_dTempRateChange);
            }
        }
        else if ((m_bTempStarted) || ((m_eRampBackMode != RATERAMP_RAMPBACK_NONE) && (m_dRateTemp != 0.0)))
        {
            // No buttons pressed, so time to deinitialize
            m_bTempStarted = false;


            if ((m_eRampBackMode == RATERAMP_RAMPBACK_PERIOD) &&  (m_dRateTempRampbackChange == 0.0))
            {
                int period = 2;
                if (period)
                    m_dRateTempRampbackChange = fabs(m_dRateTemp / (double)period);
                else {
                    resetRateTemp();
                    return kNoTrigger;
                }

            }
            else if ((m_eRampBackMode != RATERAMP_RAMPBACK_NONE) && (m_dRateTempRampbackChange == 0.0))
            {

                if (fabs(m_dRateTemp) < m_dRateTempRampbackChange) {
                    resetRateTemp();
                } else if (m_dRateTemp > 0) {
                    subRateTemp(m_dRateTempRampbackChange);
                } else {
                    addRateTemp(m_dRateTempRampbackChange);
                }
            }
            else
                resetRateTemp();
        }
    }
    else if ((m_eRateRampMode == RATERAMP_STEP) && (m_bTempStarted))
    {
        if (!m_ePbCurrent) {
            m_bTempStarted = false;
            resetRateTemp();
        }
    }

    return kNoTrigger;
}

double RateControl::getTempRate() {
    return (m_pRateDir->get() * (m_dRateTemp * m_pRateRange->get()));
}

void RateControl::setRateTemp(double v)
{
    // Do not go backwards
    if ((1. + getRawRate() + v) < 0)
        return;

    m_dRateTemp = v;
    if (m_dRateTemp < -1.0) {
        m_dRateTemp = -1.0;
    } else if (m_dRateTemp > 1.0) {
        m_dRateTemp = 1.0;
    } else if (isnan(m_dRateTemp)) {
        m_dRateTemp = 0;
    }
}

void RateControl::addRateTemp(double v)
{
    setRateTemp(m_dRateTemp + v);
}

void RateControl::subRateTemp(double v)
{
    setRateTemp(m_dRateTemp - v);
}

void RateControl::resetRateTemp(void)
{
    setRateTemp(0.0);
}

void RateControl::notifySeek(double playPos) {
    m_pScratchController->notifySeek(playPos);
}
