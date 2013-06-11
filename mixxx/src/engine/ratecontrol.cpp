// ratecontrol.cpp
// Created 7/4/2009 by RJ Ryan (rryan@mit.edu)

#include "controlobject.h"
#include "controlpushbutton.h"
#include "controlpotmeter.h"
#include "controlttrotary.h"
#include "rotary.h"
#include "mathstuff.h"

#include "engine/enginecontrol.h"
#include "engine/ratecontrol.h"
#include "engine/positionscratchcontroller.h"

#include <QDebug>

// Static default values for rate buttons (percents)
double RateControl::m_dTemp = 4.00; //(eg. 4.00%)
double RateControl::m_dTempSmall = 1.00;
double RateControl::m_dPerm = 0.50;
double RateControl::m_dPermSmall = 0.05;

int RateControl::m_iRateRampSensitivity = 250;
enum RateControl::RATERAMP_MODE RateControl::m_eRateRampMode = RateControl::RATERAMP_STEP;

RateControl::RateControl(const char* _group,
                         ConfigObject<ConfigValue>* _config)
    : EngineControl(_group, _config),
      m_sGroup(_group),
      m_iSyncState(SYNC_NONE),
      m_bUserTweakingSync(false),
      m_bVinylControlEnabled(false),
      m_bVinylControlScratching(false),
      m_ePbCurrent(0),
      m_ePbPressed(0),
      m_bTempStarted(false),
      m_dTempRateChange(0.0),
      m_dRateTemp(0.0),
      m_eRampBackMode(RATERAMP_RAMPBACK_NONE),
      m_dRateTempRampbackChange(0.0) {
    m_pScratchController = new PositionScratchController(_group);

    m_pRateDir = new ControlObject(ConfigKey(_group, "rate_dir"));
    m_pRateRange = new ControlObject(ConfigKey(_group, "rateRange"));
    m_pRateSlider = new ControlPotmeter(ConfigKey(_group, "rate"), -1.f, 1.f);

    // Search rate. Rate used when searching in sound. This overrules the
    // playback rate
    m_pRateSearch = new ControlPotmeter(ConfigKey(_group, "rateSearch"), -300., 300.);

    // Reverse button
    m_pReverseButton = new ControlPushButton(ConfigKey(_group, "reverse"));
    m_pReverseButton->set(0);

    // Forward button
    m_pForwardButton = new ControlPushButton(ConfigKey(_group, "fwd"));
    connect(m_pForwardButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlFastForward(double)),
            Qt::DirectConnection);
    m_pForwardButton->set(0);

    // Back button
    m_pBackButton = new ControlPushButton(ConfigKey(_group, "back"));
    connect(m_pBackButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlFastBack(double)),
            Qt::DirectConnection);
    m_pBackButton->set(0);

    // Permanent rate-change buttons
    buttonRatePermDown =
        new ControlPushButton(ConfigKey(_group,"rate_perm_down"));
    connect(buttonRatePermDown, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlRatePermDown(double)),
            Qt::DirectConnection);

    buttonRatePermDownSmall =
        new ControlPushButton(ConfigKey(_group,"rate_perm_down_small"));
    connect(buttonRatePermDownSmall, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlRatePermDownSmall(double)),
            Qt::DirectConnection);

    buttonRatePermUp =
        new ControlPushButton(ConfigKey(_group,"rate_perm_up"));
    connect(buttonRatePermUp, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlRatePermUp(double)),
            Qt::DirectConnection);

    buttonRatePermUpSmall =
        new ControlPushButton(ConfigKey(_group,"rate_perm_up_small"));
    connect(buttonRatePermUpSmall, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlRatePermUpSmall(double)),
            Qt::DirectConnection);

    // Temporary rate-change buttons
    buttonRateTempDown =
        new ControlPushButton(ConfigKey(_group,"rate_temp_down"));
    connect(buttonRateTempDown, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlRateTempDown(double)),
            Qt::DirectConnection);

    buttonRateTempDownSmall =
        new ControlPushButton(ConfigKey(_group,"rate_temp_down_small"));
    connect(buttonRateTempDownSmall, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlRateTempDownSmall(double)),
            Qt::DirectConnection);

    buttonRateTempUp =
        new ControlPushButton(ConfigKey(_group,"rate_temp_up"));
    connect(buttonRateTempUp, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlRateTempUp(double)),
            Qt::DirectConnection);

    buttonRateTempUpSmall =
        new ControlPushButton(ConfigKey(_group,"rate_temp_up_small"));
    connect(buttonRateTempUpSmall, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlRateTempUpSmall(double)),
            Qt::DirectConnection);

    // We need the sample rate so we can guesstimate something close
    // what latency is.
    m_pSampleRate = ControlObject::getControl(ConfigKey("[Master]","samplerate"));

    // Wheel to control playback position/speed
    m_pWheel = new ControlTTRotary(ConfigKey(_group, "wheel"));

    // Scratch controller, this is an accumulator which is useful for
    // controllers that return individiual +1 or -1s, these get added up and
    // cleared when we read
    m_pScratch = new ControlTTRotary(ConfigKey(_group, "scratch2"));
    m_pOldScratch = new ControlTTRotary(ConfigKey(_group, "scratch"));  // Deprecated

    // Scratch enable toggle
    m_pScratchToggle = new ControlPushButton(ConfigKey(_group, "scratch2_enable"));
    m_pScratchToggle->set(0);

    m_pJog = new ControlObject(ConfigKey(_group, "jog"));
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

    m_pSyncMasterEnabled = new ControlPushButton(ConfigKey(_group, "sync_master"));
    m_pSyncMasterEnabled->setButtonMode(ControlPushButton::TOGGLE);
    connect(m_pSyncMasterEnabled, SIGNAL(valueChanged(double)),
                this, SLOT(slotSyncMasterChanged(double)),
                Qt::DirectConnection);

    m_pSyncSlaveEnabled = new ControlPushButton(ConfigKey(_group, "sync_slave"));
    m_pSyncSlaveEnabled->setButtonMode(ControlPushButton::TOGGLE);
    connect(m_pSyncSlaveEnabled, SIGNAL(valueChanged(double)),
                this, SLOT(slotSyncSlaveChanged(double)),
                Qt::DirectConnection);

    m_pSyncInternalEnabled = ControlObject::getControl(ConfigKey("[Master]", "sync_master"));
    connect(m_pSyncInternalEnabled, SIGNAL(valueChanged(double)),
                this, SLOT(slotSyncInternalChanged(double)),
                Qt::DirectConnection);

    m_pSyncState = new ControlObject(ConfigKey(_group, "sync_state"));
    connect(m_pSyncState, SIGNAL(valueChanged(double)),
                this, SLOT(slotSyncStateChanged(double)),
                Qt::DirectConnection);
    connect(m_pSyncState, SIGNAL(valueChangedFromEngine(double)),
                this, SLOT(slotSyncStateChanged(double)),
                Qt::DirectConnection);

    m_pSyncMasterEnabled->set(false);
    m_pSyncSlaveEnabled->set(false);
    m_iSyncState = SYNC_NONE;

#ifdef __VINYLCONTROL__
    m_pVCEnabled = ControlObject::getControl(ConfigKey(_group, "vinylcontrol_enabled"));
    // Throw a hissy fit if somebody moved us such that the vinylcontrol_enabled
    // control doesn't exist yet. This will blow up immediately, won't go unnoticed.
    Q_ASSERT(m_pVCEnabled);
    connect(m_pVCEnabled, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlVinyl(double)),
            Qt::DirectConnection);
    connect(m_pVCEnabled, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotControlVinyl(double)),
            Qt::DirectConnection);

    ControlObject* pVCScratching = ControlObject::getControl(ConfigKey(_group, "vinylcontrol_scratching"));
    // Throw a hissy fit if somebody moved us such that the vinylcontrol_enabled
    // control doesn't exist yet. This will blow up immediately, won't go unnoticed.
    Q_ASSERT(pVCScratching);
    connect(pVCScratching, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlVinylScratching(double)),
            Qt::DirectConnection);
    connect(pVCScratching, SIGNAL(valueChangedFromEngine(double)),
            this, SLOT(slotControlVinylScratching(double)),
            Qt::DirectConnection);
#endif
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
    delete m_pOldScratch;
    delete m_pScratchToggle;
    delete m_pJog;
    delete m_pJogFilter;
    delete m_pScratchController;
}

void RateControl::setEngineMaster(EngineMaster* pEngineMaster) {
    EngineControl::setEngineMaster(pEngineMaster);

    //TODO: should we only hook these up if we are a slave?  beat distance
    //is updated on every iteration so it's heavy
    m_pMasterBpm = ControlObject::getControl(ConfigKey("[Master]","sync_bpm"));
    connect(m_pMasterBpm, SIGNAL(valueChanged(double)),
                this, SLOT(slotMasterBpmChanged(double)),
                Qt::DirectConnection);
    connect(m_pMasterBpm, SIGNAL(valueChangedFromEngine(double)),
                this, SLOT(slotMasterBpmChanged(double)),
                Qt::DirectConnection);

    // We need this so we can sync to master sync
    m_pFileBpm = ControlObject::getControl(ConfigKey(m_sGroup, "file_bpm"));
    connect(m_pFileBpm, SIGNAL(valueChanged(double)),
            this, SLOT(slotFileBpmChanged(double)),
            Qt::DirectConnection);
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

void RateControl::slotFileBpmChanged(double bpm) {
    m_dFileBpm = bpm;
    slotMasterBpmChanged(m_pMasterBpm->get());
}

void RateControl::slotMasterBpmChanged(double syncbpm) {
    // Vinyl overrides
    if (m_bVinylControlEnabled) {
        return;
    }
    if (m_iSyncState == SYNC_SLAVE) {
        // if we're a slave, update the rate value -- we don't set anything here,
        // this comes into effect in the return from calculaterate
        //TODO: let's ignore x2, /2 issues for now
        //this is reproduced from bpmcontrol::syncTempo -- should break this out
        double dDesiredRate;
        if (m_dFileBpm == 0.0)
        {
            //XXX TODO: what to do about this case
            qDebug() << "Zero BPM, I guess we call the desired rate 1.0!";
            dDesiredRate = 1.0;
        } else {
            dDesiredRate = syncbpm / m_dFileBpm;
        }
        m_dSyncedRate = dDesiredRate;
        if (m_dSyncedRate != 0) {
            m_pRateSlider->set(((m_dSyncedRate - 1.0f) / m_pRateRange->get()) * m_pRateDir->get());
        } else {
            m_pRateSlider->set(0);
        }
    }
}

void RateControl::slotSyncMasterChanged(double state) {
    qDebug() << m_sGroup << "slot master changed";

    if (state) {
        if (m_iSyncState == SYNC_MASTER){
            qDebug() << "already master";
            return;
        }

        if (m_pTrack.isNull()) {
            qDebug() << m_sGroup << " no track loaded, can't be master";
            m_pSyncMasterEnabled->set(false);
            return;
        }

        qDebug() << m_sGroup << " setting ourselves as master";
        m_pSyncState->set(SYNC_MASTER);
    } else {
        // now, turning off master turns off sync mode
        if (m_iSyncState != SYNC_MASTER) {
            return;
        }
        //unset ourselves
        qDebug() << m_sGroup << "unsetting ourselves as master (now off)";
        m_pSyncState->set(SYNC_NONE);
    }
}

void RateControl::slotSyncSlaveChanged(double state) {
    //qDebug() << m_sGroup << "slot slave changed";
    if (state) {
        if (m_iSyncState == SYNC_SLAVE) {
            //qDebug() << "already slave";
            return;
        }
        if (m_pTrack.isNull()) {
            qDebug() << m_sGroup << " no track loaded, can't be slave";
            m_pSyncSlaveEnabled->set(false);
            return;
        }
        m_pSyncState->set(SYNC_SLAVE);
    } else {
        // For now, turning off slave turns off syncing
        m_pSyncState->set(SYNC_NONE);
    }
}

void RateControl::slotSyncInternalChanged(double state) {
    if (state) {
        if (m_iSyncState == SYNC_MASTER) {
            m_pSyncState->set(SYNC_SLAVE);
        }
    }
}

void RateControl::slotSyncStateChanged(double state) {
    double changed = m_iSyncState != state;
    m_iSyncState = state;
    if (changed) {
        slotSetStatuses();
    }
    m_iSyncState = state;
    if (state == SYNC_SLAVE) {
        slotMasterBpmChanged(m_pMasterBpm->get());
    }
}

void RateControl::slotSetStatuses() {
    switch (m_iSyncState) {
    case SYNC_NONE:
        m_pSyncMasterEnabled->set(false);
        m_pSyncSlaveEnabled->set(false);
        break;
    case SYNC_SLAVE:
        m_pSyncMasterEnabled->set(false);
        m_pSyncSlaveEnabled->set(true);
        break;
    case SYNC_MASTER:
        m_pSyncMasterEnabled->set(true);
        m_pSyncSlaveEnabled->set(false);
    }
}

void RateControl::trackLoaded(TrackPointer pTrack) {
    if (m_pTrack) {
        trackUnloaded(m_pTrack);
    }
    if (pTrack) {
        m_pTrack = pTrack;
    }
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
        jogFactor = 0.0f;
    }

    return jogFactor;
}

bool RateControl::getUserTweakingSync() const {
    return m_bUserTweakingSync;
}

double RateControl::calculateRate(double baserate, bool paused, int iSamplesPerBuffer,
                                  bool* isScratching) {
    double rate = (paused ? 0 : 1.0);

    double searching = m_pRateSearch->get();
    if (searching) {
        // If searching is in progress, it overrides everything else
        rate = searching;
    } else {


        double wheelFactor = getWheelFactor();
        double jogFactor = getJogFactor();
        bool scratchEnable = m_pScratchToggle->get() != 0 || m_bVinylControlEnabled;

        // if master sync is on, respond to it -- but vinyl always overrides
        if (m_iSyncState == SYNC_SLAVE && !paused && !m_bVinylControlEnabled)
        {
            rate = m_dSyncedRate;
            double userTweak = getTempRate() + wheelFactor + jogFactor;
            rate += userTweak;
            m_bUserTweakingSync = (userTweak != 0.0);

            m_pRateSlider->set(((rate - 1.0f) / m_pRateRange->get()) * m_pRateDir->get());
            return rate;
        }

        double scratchFactor = m_pScratch->get();
        // Don't trust values from m_pScratch
        if (isnan(scratchFactor)) {
            scratchFactor = 0.0;
        }

        // Old Scratch works without scratchEnable
        double oldScratchFactor = m_pOldScratch->get(); // Deprecated
        // Don't trust values from m_pScratch
        if (isnan(oldScratchFactor)) {
            oldScratchFactor = 0.0;
        }

        // If vinyl control is enabled and scratching then also set isScratching
        if (m_bVinylControlEnabled && m_bVinylControlScratching) {
            *isScratching = true;
        }

        if (paused) {
            // Stopped. Wheel, jog and scratch controller all scrub through audio.
            // New scratch behavior overrides old
            if (scratchEnable) {
                rate = scratchFactor + jogFactor + wheelFactor * 40.0;
            } else {
                // Just remove oldScratchFactor in future
                rate = oldScratchFactor + jogFactor * 18 + wheelFactor;
            }
        } else {
            // The buffer is playing, so calculate the buffer rate.

            // There are four rate effects we apply: wheel, scratch, jog and temp.
            // Wheel: a linear additive effect (no spring-back)
            // Scratch: a rate multiplier
            // Jog: a linear additive effect whose value is filtered (springs back)
            // Temp: pitch bend

            // New scratch behavior - overrides playback speed (and old behavior)
            if (scratchEnable) {
                rate = scratchFactor;
            } else {

                rate = 1. + getRawRate() + getTempRate();
                rate += wheelFactor;

                // Deprecated old scratch behavior
                if (oldScratchFactor < 0.) {
                    rate *= (oldScratchFactor - 1.);
                } else if (oldScratchFactor > 0.) {
                    rate *= (oldScratchFactor + 1.);
                }
            }

            rate += jogFactor;

            // If we are reversing (and not scratching,) flip the rate.
            if (!scratchEnable && m_pReverseButton->get()) {
                rate = -rate;
            }
        }

        double currentSample = getCurrentSample();
        m_pScratchController->process(currentSample, rate, iSamplesPerBuffer, baserate);

        // If waveform scratch is enabled, override all other controls
        if (m_pScratchController->isEnabled()) {
            rate = m_pScratchController->getRate();
            *isScratching = true;
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


        if ( m_eRateRampMode == RATERAMP_STEP )
        {
            // old temporary pitch shift behaviour
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
            if ( m_ePbCurrent == RateControl::RATERAMP_UP )
                addRateTemp(m_dTempRateChange);
            else if ( m_ePbCurrent == RateControl::RATERAMP_DOWN )
                subRateTemp(m_dTempRateChange);
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
    if (( 1. + getRawRate() + v ) < 0)
        return;

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

void RateControl::slotControlVinyl(double toggle) {
    m_bVinylControlEnabled = (bool)toggle;
}

void RateControl::slotControlVinylScratching(double toggle) {
    m_bVinylControlScratching = (bool)toggle;
}

void RateControl::notifySeek(double playPos) {
    m_pScratchController->notifySeek(playPos);
}
