// ratecontrol.h
// Created 7/4/2009 by RJ Ryan (rryan@mit.edu)

#ifndef RATECONTROL_H
#define RATECONTROL_H

#include <QObject>

#include "configobject.h"
#include "engine/enginecontrol.h"

const int RATE_TEMP_STEP = 100.;
const int RATE_TEMP_STEP_SMALL = RATE_TEMP_STEP * 10.;

class Rotary;
class ControlTTRotary;
class ControlObject;
class ControlPotmeter;
class ControlPushButton;

class RateControl : public EngineControl {
    Q_OBJECT
public:
    RateControl(const char* _group, const ConfigObject<ConfigValue>* _config);
    ~RateControl();
    double process(const double currentSample, const double totalSamples, 
                        const double iBufferSize);
    double calculateRate(double baserate, bool paused);
    double getRawRate();

    /** Set rate change when temp rate button is pressed */
    static void setTemp(double v);
    /** Set rate change when temp rate small button is pressed */
    static void setTempSmall(double v);
    /** Set rate change when perm rate button is pressed */
    static void setPerm(double v);
    /** Set rate change when perm rate small button is pressed */
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
    
    /** Values used when temp and perm rate buttons are pressed */
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

    /** Is true if a rate temp button is pressed */    
    bool m_bTempPress;
    /** Is true if a rate button was released */
    bool m_bTempRelease;
    /** Is true if a rate button is down */
    bool m_bTempDown;
    /** Set to the rate change used for rate temp */
    double m_dTempRateChange;

    /** Old playback rate. Stored in this variable while a temp pitch change
      * buttons is in effect. It does not work to just decrease the pitch slider
      * by the value it has been increased with when the temp button was
      * pressed, because there is a fixed limit on the range of the pitch
      * slider */
    double m_dOldRate;

};

#endif /* RATECONTROL_H */
