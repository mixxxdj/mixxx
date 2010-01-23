// ratecontrol.cpp
// Created 7/4/2009 by RJ Ryan (rryan@mit.edu)

#include "controlobject.h"
#include "controlpushbutton.h"
#include "controlpotmeter.h"
#include "controlttrotary.h"
#include "rotary.h"

#include "engine/enginecontrol.h"
#include "engine/ratecontrol.h"

#ifdef _MSC_VER
#include <float.h>  // for _isnan() on VC++
#define isnan(x) _isnan(x)  // VC++ uses _isnan() instead of isnan()
#else
#include <math.h>  // for isnan() everywhere else
#endif

// Static default values for rate buttons
double RateControl::m_dTemp = 0.01;
double RateControl::m_dTempSmall = 0.001;
double RateControl::m_dPerm = 0.01;
double RateControl::m_dPermSmall = 0.001;

int RateControl::m_iRateRampSensitivity = 250;
enum RateControl::RATERAMP_MODE RateControl::m_eRateRampMode = RateControl::RATERAMP_STEP;

RateControl::RateControl(const char* _group,
                         ConfigObject<ConfigValue>* _config) :
    EngineControl(_group, _config),
    m_ePbCurrent(0),
    m_ePbPressed(0),
    m_bTempStarted(false),
    m_dTempRateChange(0.0),
    m_dRateTemp(0.0),
    m_eRampBackMode(RATERAMP_RAMPBACK_NONE),
    m_dRateTempRampbackChange(0.0),
    m_dOldRate(0.0f),
    m_pConfig(_config)
{
    m_pRateDir = new ControlObject(ConfigKey(_group, "rate_dir"));
    m_pRateRange = new ControlObject(ConfigKey(_group, "rateRange"));
    m_pRateSlider = new ControlPotmeter(ConfigKey(_group, "rate"), -1.f, 1.f);

    // Search rate. Rate used when searching in sound. This overrules the
    // playback rate
    m_pRateSearch = new ControlPotmeter(ConfigKey(_group, "rateSearch"), -300., 300.);

    // Reverse button
    m_pReverseButton = new ControlPushButton(ConfigKey(_group, "reverse"));
    m_pReverseButton->set(0);
    m_pReverseButton->setToggleButton(true);

    // Forward button
    m_pForwardButton = new ControlPushButton(ConfigKey(_group, "fwd"));
    connect(m_pForwardButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlFastForward(double)));
    m_pForwardButton->set(0);

    // Back button
    m_pBackButton = new ControlPushButton(ConfigKey(_group, "back"));
    connect(m_pBackButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlFastBack(double)));
    m_pBackButton->set(0);

    // Permanent rate-change buttons
    buttonRatePermDown =
        new ControlPushButton(ConfigKey(_group,"rate_perm_down"));
    connect(buttonRatePermDown, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlRatePermDown(double)));

    buttonRatePermDownSmall =
        new ControlPushButton(ConfigKey(_group,"rate_perm_down_small"));
    connect(buttonRatePermDownSmall, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlRatePermDownSmall(double)));

    buttonRatePermUp =
        new ControlPushButton(ConfigKey(_group,"rate_perm_up"));
    connect(buttonRatePermUp, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlRatePermUp(double)));

    buttonRatePermUpSmall =
        new ControlPushButton(ConfigKey(_group,"rate_perm_up_small"));
    connect(buttonRatePermUpSmall, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlRatePermUpSmall(double)));

    // Temporary rate-change buttons
    buttonRateTempDown =
        new ControlPushButton(ConfigKey(_group,"rate_temp_down"));
    connect(buttonRateTempDown, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlRateTempDown(double)));

    buttonRateTempDownSmall =
        new ControlPushButton(ConfigKey(_group,"rate_temp_down_small"));
    connect(buttonRateTempDownSmall, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlRateTempDownSmall(double)));

    buttonRateTempUp =
        new ControlPushButton(ConfigKey(_group,"rate_temp_up"));
    connect(buttonRateTempUp, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlRateTempUp(double)));

    buttonRateTempUpSmall =
        new ControlPushButton(ConfigKey(_group,"rate_temp_up_small"));
    connect(buttonRateTempUpSmall, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlRateTempUpSmall(double)));

    // We need the sample rate so we can guesstimate something close
    // what latency is.
    m_pSampleRate = ControlObject::getControl(ConfigKey("[Master]","samplerate"));

    // Wheel to control playback position/speed
    m_pWheel = new ControlTTRotary(ConfigKey(_group, "wheel"));

    // Scratch controller, this is an accumulator which is useful for
    // controllers that return individiual +1 or -1s, these get added up and
    // cleared when we read
    m_pScratch = new ControlTTRotary(ConfigKey(_group, "scratch"));

    m_pJog = new ControlObject(ConfigKey(_group, "jog"));
    m_pJogFilter = new Rotary();
    // FIXME: This should be dependent on sample rate/block size or something
    m_pJogFilter->setFilterLength(5);

    // Update Internal Settings
    // Set Pitchbend Mode
    m_eRateRampMode = (RateControl::RATERAMP_MODE)
        m_pConfig->getValueString(ConfigKey("[Controls]","RateRamp")).toInt();

    // Set the Sensitivity
    m_iRateRampSensitivity =
        m_pConfig->getValueString(ConfigKey("[Controls]","RateRampSensitivity")).toInt();

}

RateControl::~RateControl() {
    delete m_pRateSlider;
    delete m_pRateRange;
    delete m_pRateDir;

    delete m_pRateSearch;

    delete m_pReverseButton;
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
    delete m_pScratch;
    delete m_pJog;
    delete m_pJogFilter;
}

void RateControl::setRateRamp(bool linearMode)
{
    if ( linearMode )
        m_eRateRampMode = RateControl::RATERAMP_LINEAR;
    else
        m_eRateRampMode = RateControl::RATERAMP_STEP;
}

void RateControl::setRateRampSensitivity(int sense)
{
    // Reverse the actual sensitivity value passed.
    // That way the gui works in an intuitive manner.
    sense = RATE_SENSITIVITY_MAX - sense + RATE_SENSITIVITY_MIN;
    if ( sense < RATE_SENSITIVITY_MIN )
        m_iRateRampSensitivity = RATE_SENSITIVITY_MIN;
    else if ( sense > RATE_SENSITIVITY_MAX )
        m_iRateRampSensitivity = RATE_SENSITIVITY_MAX;
    else
        m_iRateRampSensitivity = sense;
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
        m_pRateSlider->sub(m_pRateDir->get() * m_dPerm / (100. * m_pRateRange->get()));
}

void RateControl::slotControlRatePermDownSmall(double)
{
    // Adjusts temp rate down if button pressed
    if (buttonRatePermDownSmall->get())
        m_pRateSlider->sub(m_pRateDir->get() * m_dPermSmall / (100. * m_pRateRange->get()));
}

void RateControl::slotControlRatePermUp(double)
{
    // Adjusts temp rate up if button pressed
    if (buttonRatePermUp->get())
        m_pRateSlider->add(m_pRateDir->get() * m_dPerm / (100. * m_pRateRange->get()));
}

void RateControl::slotControlRatePermUpSmall(double)
{
    // Adjusts temp rate up if button pressed
    if (buttonRatePermUpSmall->get())
        m_pRateSlider->add(m_pRateDir->get() * m_dPermSmall / (100. * m_pRateRange->get()));
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

double RateControl::getRawRate() {
    return m_pRateSlider->get() *
        m_pRateRange->get() *
        m_pRateDir->get();
}

double RateControl::getScratchFactor() {
    double scratchFactor = m_pScratch->get();
    if(!isnan(scratchFactor) && scratchFactor != 0.0f) {
        if (scratchFactor < 0.) {
            scratchFactor = scratchFactor - 1.0f;
        } else if (scratchFactor > 0.) {
            scratchFactor = scratchFactor + 1.0f;
        }
    } else {
        scratchFactor = 1.0f;
    }
    return scratchFactor;
}

double RateControl::getWheelFactor() {
    // Calculate wheel (experimental formula)
    return 40 * m_pWheel->get();
}

double RateControl::getJogFactor() {
    // FIXME: Sensitivity should be configurable separately?
    const double jogSensitivity = m_pRateRange->get();
    double jogValue = m_pJog->get();

    // Since m_pJog is an accumulator, reset it since we've used its value.
    if(jogValue != 0.)
        m_pJog->set(0.);

    double jogValueFiltered = m_pJogFilter->filter(jogValue);
    double jogFactor = jogValueFiltered * jogSensitivity;

    if (isnan(jogValue) || isnan(jogFactor)) {
        jogFactor = 0.0f;
    }

    return jogFactor;
}

double RateControl::calculateRate(double baserate, bool paused) {
    double rate = 0.0;
    double wheelFactor = getWheelFactor();
    double scratchFactor = getScratchFactor();
    double jogFactor = getJogFactor();
    bool searching = m_pRateSearch->get() != 0.;

    if (searching) {
        // If searching is in progress, it overrides the playback rate.
        rate = m_pRateSearch->get();
    } else if (paused) {
        // Stopped. Wheel, jog and scratch controller all scrub through audio.
        // Scratch is centered around 1.0, so subtract 1.0
        rate = scratchFactor - 1.0f + jogFactor + wheelFactor/10.;
    } else {
        // The buffer is playing, so calculate the buffer rate.

        // There are four rate effects we apply: wheel, scratch, jog and temp.
        // Wheel: a linear additive effect
        // Scratch: a rate multiplier
        // Jog: a linear additive effect whose value is filtered
        // Temp: pitch bend

        rate = 1. + getRawRate() + getTempRate();
        rate += wheelFactor/10.;
        rate *= scratchFactor;
        rate += jogFactor;

        // If we are reversing, flip the rate.
        if (m_pReverseButton->get()) {
            rate = -rate;
        }
    }

    // Scale the rate by the engine samplerate
    rate *= baserate;

    return rate;
}

double RateControl::process(const double rate,
                            const double currentSample,
                            const double totalSamples,
                            const int bufferSamples)
{
    /*
     * Code to handle temporary rate change buttons.
     *
     * We support two behaviours, the standard ramped pitch bending
     * and pitch shift stepping, which is the old behaviour.
     */

    /*
     * Initialize certain values necessary for pitchbending. Most of this
     * code should be handled inside a slot, but we'd need to connect to
     * the troublesome Latency ControlObject... Either the Master or Soundcard
     * one.
     */

    double latrate = ((double)bufferSamples / (double)m_pSampleRate->get());


    if ((m_ePbPressed) && (!m_bTempStarted))
    {
        m_bTempStarted = true;
        m_dOldRate = m_pRateSlider->get();


        if ( m_eRateRampMode == RATERAMP_STEP )
        {
            // old temporary pitch shift behaviour
            double range = m_pRateRange->get();

            // Avoid Division by Zero
            if (range == 0) {
                qDebug() << "Avoiding a Division by Zero in RATERAMP_STEP code";
                return m_dOldRate;
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

    if ((m_ePbCurrent) && (m_eRateRampMode == RATERAMP_LINEAR))
    {
        // apply ramped pitchbending
        if ( m_ePbCurrent == RateControl::RATERAMP_UP )
            addRateTemp(m_dTempRateChange);
        else if ( m_ePbCurrent == RateControl::RATERAMP_DOWN )
            subRateTemp(m_dTempRateChange);
    }
    else if ((!m_ePbCurrent) && (m_eRateRampMode == RATERAMP_STEP))
    {
        resetRateTemp();
    }
    else if ((m_bTempStarted) || ((m_eRampBackMode != RATERAMP_RAMPBACK_NONE) && (m_dRateTemp != 0.0)))
    {
        // No buttons pressed, so time to deinitialize
        m_bTempStarted = false;


        if ( m_eRateRampMode == RATERAMP_STEP )
            m_pRateSlider->set(m_dOldRate);
        else {

            if ((m_eRampBackMode == RATERAMP_RAMPBACK_PERIOD) &&  (m_dRateTempRampbackChange == 0.0 ))
            {
                int period = 2;
                if (period)
                    m_dRateTempRampbackChange = fabs(m_dRateTemp / (double)period);
                else {
                    resetRateTemp();
                    return 1;
                }

            }

            if ((m_eRampBackMode != RATERAMP_RAMPBACK_NONE))
            {

                if ( fabs(m_dRateTemp) < m_dRateTempRampbackChange)
                    resetRateTemp();
                else if ( m_dRateTemp > 0 )
                    subRateTemp(m_dRateTempRampbackChange);
                else
                    addRateTemp(m_dRateTempRampbackChange);
            }
            else
                resetRateTemp();
        }
    }

    return 1;
}

double RateControl::getTempRate() {
    return (m_pRateDir->get() * (m_dRateTemp * m_pRateRange->get()));
}

void RateControl::setRateTemp(double v)
{
    m_dRateTemp = v;
    if ( m_dRateTemp < -1.0 )
        m_dRateTemp = -1.0;
    else if ( m_dRateTemp > 1.0 )
        m_dRateTemp = 1.0;
    else if ( isnan(m_dRateTemp))
        m_dRateTemp = 0;
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
