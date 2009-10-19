// ratecontrol.h
// Created 7/4/2009 by RJ Ryan (rryan@mit.edu)

#ifndef RATECONTROL_H
#define RATECONTROL_H

#include <QObject>

#include "configobject.h"
#include "engine/enginecontrol.h"

class Rotary;
class ControlTTRotary;
class ControlObject;
class ControlPotmeter;
class ControlPushButton;

// RateControl is an EngineControl that is in charge of managing the rate of
// playback of a given channel of audio in the Mixxx engine. Using input from
// various controls, RateControl will calculate the current rate.
class RateControl : public EngineControl {
    Q_OBJECT
public:
    RateControl(const char* _group, const ConfigObject<ConfigValue>* _config);
    ~RateControl();

    // Must be called during each callback of the audio thread so that
    // RateControl has a chance to update itself.
    double process(const double dRate,
                   const double currentSample,
                   const double totalSamples,
                   const int iBufferSize);
    // Returns the current engine rate.
    double calculateRate(double baserate, bool paused);
    double getRawRate();

    // Set rate change when temp rate button is pressed
    static void setTemp(double v);
    // Set rate change when temp rate small button is pressed
    static void setTempSmall(double v);
    // Set rate change when perm rate button is pressed
    static void setPerm(double v);
    // Set rate change when perm rate small button is pressed
    static void setPermSmall(double v);

public slots:
    void slotControlRatePermDown(double);
    void slotControlRatePermDownSmall(double);
    void slotControlRatePermUp(double);
    void slotControlRatePermUpSmall(double);
    void slotControlRateTempDown(double);
    void slotControlRateTempDownSmall(double);
    void slotControlRateTempUp(double);
    void slotControlRateTempUpSmall(double);
    void slotControlFastForward(double);
    void slotControlFastBack(double);
private:
    double getJogFactor();
    double getWheelFactor();
    double getScratchFactor();

    // Values used when temp and perm rate buttons are pressed
    static double m_dTemp, m_dTempSmall, m_dPerm, m_dPermSmall;

    ControlPushButton *buttonRateTempDown, *buttonRateTempDownSmall,
        *buttonRateTempUp, *buttonRateTempUpSmall;
    ControlPushButton *buttonRatePermDown, *buttonRatePermDownSmall,
        *buttonRatePermUp, *buttonRatePermUpSmall;
    ControlObject *m_pRateDir, *m_pRateRange;
    ControlPotmeter* m_pRateSlider;
    ControlPotmeter* m_pRateSearch;
    ControlPushButton* m_pReverseButton;
    ControlObject* m_pBackButton;
    ControlObject* m_pForwardButton;

    ControlTTRotary* m_pWheel;
    ControlTTRotary* m_pScratch;
    ControlObject* m_pJog;
    Rotary* m_pJogFilter;

    // Is true if a rate temp button is pressed
    bool m_bTempPress;

    // Old playback rate. Stored in this variable while a temp pitch change
    // buttons is in effect. It does not work to just decrease the pitch slider
    // by the value it has been increased with when the temp button was pressed,
    // because there is a fixed limit on the range of the pitch slider
    double m_dOldRate;

};

#endif /* RATECONTROL_H */
