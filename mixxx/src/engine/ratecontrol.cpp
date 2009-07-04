/**
 * RateControl.cpp
 * 7/4/2009 -- rryan@mit.edu
 */

#include "controlobject.h"
#include "controlpushbutton.h"

#include "engine/ratecontrol.h"

// Static default values for rate buttons
double RateControl::m_dTemp = 0.01;
double RateControl::m_dTempSmall = 0.001;
double RateControl::m_dPerm = 0.01;
double RateControl::m_dPermSmall = 0.001;

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

RateControl::RateControl(const char* _group, ConfigObject<ConfigValue>* _config) :
    m_bTempPress(false),
    m_dOldRate(0.0f) {
    m_pRateDir = ControlObject::getControl(ConfigKey(_group, "rate_dir"));
    m_pRateRange = ControlObject::getControl(ConfigKey(_group, "rateRange"));
    m_pRateSlider = ControlObject::getControl(ConfigKey(_group, "rate"));
    
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

}

RateControl::~RateControl() {
    // Don't delete CO's that we don't own! (RateDir, RateRange, RateSlider)
    delete buttonRateTempDown;
    delete buttonRateTempDownSmall;
    delete buttonRateTempUp;
    delete buttonRateTempUpSmall;
    delete buttonRatePermDown;
    delete buttonRatePermDownSmall;
    delete buttonRatePermUp;
    delete buttonRatePermUpSmall;

    
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
