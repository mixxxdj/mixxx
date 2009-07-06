// RateControl.h
// Created 7/4/2009 by RJ Ryan (rryan@mit.edu)

#ifndef RATECONTROL_H
#define RATECONTROL_H

#include <QObject>

#include "configobject.h"
#include "engine/enginecontrol.h"

class ControlObject;
class ControlPushButton;

class RateControl : public EngineControl {
    
public:
    RateControl(const char* _group, const ConfigObject<ConfigValue>* _config);
    ~RateControl();
    double process(const double currentSample, const double totalSamples);

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
private:
    /** Values used when temp and perm rate buttons are pressed */
    static double m_dTemp, m_dTempSmall, m_dPerm, m_dPermSmall;
    
    ControlPushButton *buttonRateTempDown, *buttonRateTempDownSmall,
        *buttonRateTempUp, *buttonRateTempUpSmall;
    ControlPushButton *buttonRatePermDown, *buttonRatePermDownSmall,
        *buttonRatePermUp, *buttonRatePermUpSmall;
    ControlObject *m_pRateDir, *m_pRateRange, *m_pRateSlider;

    /** Is true if a rate temp button is pressed */    
    bool m_bTempPress;

    /** Old playback rate. Stored in this variable while a temp pitch change
      * buttons is in effect. It does not work to just decrease the pitch slider
      * by the value it has been increased with when the temp button was
      * pressed, because there is a fixed limit on the range of the pitch
      * slider */
    double m_dOldRate;

};

#endif /* RATECONTROL_H */
