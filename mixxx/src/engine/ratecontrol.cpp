// ratecontrol.cpp
// Created 7/4/2009 by RJ Ryan (rryan@mit.edu)

#include "controlobject.h"
#include "controlpushbutton.h"
#include "controlpotmeter.h"
#include "controlttrotary.h"
#include "rotary.h"

#include "engine/enginecontrol.h"
#include "engine/ratecontrol.h"

// Static default values for rate buttons
double RateControl::m_dTemp = 0.01;
double RateControl::m_dTempSmall = 0.001;
double RateControl::m_dPerm = 0.01;
double RateControl::m_dPermSmall = 0.001;

RateControl::RateControl(const char* _group,
                         ConfigObject<ConfigValue>* _config) :
    EngineControl(_group, _config),
    m_bTempPress(false),
    m_dOldRate(0.0f) {

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
    // Adjusts temp rate down if button pressed, otherwise set to 0.
    if (buttonRateTempDown->get() && !m_bTempPress)
    {
        m_bTempPress = true;
        m_dOldRate = m_pRateSlider->get();
        m_pRateSlider->sub(m_pRateDir->get() * m_dTemp / (100. * m_pRateRange->get()));
    }
    else if (!buttonRateTempDown->get())
    {
        m_bTempPress = false;
        m_pRateSlider->set(m_dOldRate);
    }
}

void RateControl::slotControlRateTempDownSmall(double)
{
    // Adjusts temp rate down if button pressed, otherwise set to 0.
    if (buttonRateTempDownSmall->get() && !m_bTempPress)
    {
        m_bTempPress = true;
        m_dOldRate = m_pRateSlider->get();
        m_pRateSlider->sub(m_pRateDir->get() * m_dTempSmall / (100. * m_pRateRange->get()));
    }
    else if (!buttonRateTempDownSmall->get())
    {
        m_bTempPress = false;
        m_pRateSlider->set(m_dOldRate);
    }
}

void RateControl::slotControlRateTempUp(double)
{
    // Adjusts temp rate up if button pressed, otherwise set to 0.
    if (buttonRateTempUp->get() && !m_bTempPress)
    {
        m_bTempPress = true;
        m_dOldRate = m_pRateSlider->get();
        m_pRateSlider->add(m_pRateDir->get() * m_dTemp / (100. * m_pRateRange->get()));
    }
    else if (!buttonRateTempUp->get())
    {
        m_bTempPress = false;
        m_pRateSlider->set(m_dOldRate);
    }
}

void RateControl::slotControlRateTempUpSmall(double)
{
    // Adjusts temp rate up if button pressed, otherwise set to 0.
    if (buttonRateTempUpSmall->get() && !m_bTempPress)
    {
        m_bTempPress = true;
        m_dOldRate = m_pRateSlider->get();
        m_pRateSlider->add(m_pRateDir->get() * m_dTempSmall / (100. * m_pRateRange->get()));
    }
    else if (!buttonRateTempUpSmall->get())
    {
        m_bTempPress = false;
        m_pRateSlider->set(m_dOldRate);
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
        if (scratchFactor == 1.)
            scratchFactor = 0;
        // Stopped. Wheel, jog and scratch controller all scrub through audio.
        rate = scratchFactor * jogFactor * baserate + wheelFactor/10.;
    } else {
        // The buffer is playing, so calculate the buffer rate.

        // There are three rate effects we apply: wheel, scratch, and jog.
        // Wheel: a linear additive effect
        // Scratch: a rate multiplier
        // Jog: a linear additive effect whose value is filtered

        rate = (1. + getRawRate()) * baserate;
        rate += wheelFactor/10.;
        rate *= scratchFactor;
        rate += jogFactor;

        // If we are reversing, flip the rate.
        if (m_pReverseButton->get()) {
            rate = -rate;
        }
    }

    return rate;
}

double RateControl::process(const double rate,
                            const double currentSample,
                            const double totalSamples,
                            const int iBufferSize) {
    return 0;
}
