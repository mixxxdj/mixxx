// ratecontrol.h
// Created 7/4/2009 by RJ Ryan (rryan@mit.edu)

#ifndef RATECONTROL_H
#define RATECONTROL_H

#include <QObject>

#include "configobject.h"
#include "engine/enginecontrol.h"

const int RATE_TEMP_STEP = 500;
const int RATE_TEMP_STEP_SMALL = RATE_TEMP_STEP * 10.;
const int RATE_SENSITIVITY_MIN = 100;
const int RATE_SENSITIVITY_MAX = 2500;

class Rotary;
class ControlTTRotary;
class ControlObject;
class ControlPotmeter;
class ControlPushButton;
class PositionScratchController;

// RateControl is an EngineControl that is in charge of managing the rate of
// playback of a given channel of audio in the Mixxx engine. Using input from
// various controls, RateControl will calculate the current rate.
class RateControl : public EngineControl {
    Q_OBJECT
public:
    RateControl(const char* _group, ConfigObject<ConfigValue>* _config);
    virtual ~RateControl();

    // Must be called during each callback of the audio thread so that
    // RateControl has a chance to update itself.
    double process(const double dRate,
                   const double currentSample,
                   const double totalSamples,
                   const int bufferSamples);
    // Returns the current engine rate.
    double calculateRate(double baserate, bool paused, int iSamplesPerBuffer, bool* isScratching);
    double getRawRate();

    // Set rate change when temp rate button is pressed
    static void setTemp(double v);
    // Set rate change when temp rate small button is pressed
    static void setTempSmall(double v);
    // Set rate change when perm rate button is pressed
    static void setPerm(double v);
    // Set rate change when perm rate small button is pressed
    static void setPermSmall(double v);
    /** Set Rate Ramp Mode */
    static void setRateRamp(bool);
    /** Set Rate Ramp Sensitivity */
    static void setRateRampSensitivity(int);
    virtual void notifySeek(double dNewPlaypos);

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
    void slotControlVinyl(double);
    void slotControlVinylScratching(double);

  private:
    double getJogFactor();
    double getWheelFactor();

    /** Set rate change of the temporary pitch rate */
    void setRateTemp(double v);
    /** Add a value to the temporary pitch rate */
    void addRateTemp(double v);
    /** Subtract a value from the temporary pitch rate */
    void subRateTemp(double v);
    /** Reset the temporary pitch rate */
    void resetRateTemp(void);
    /** Get the 'Raw' Temp Rate */
    double getTempRate(void);

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
    ControlTTRotary* m_pOldScratch;
    PositionScratchController* m_pScratchController;

    ControlPushButton* m_pScratchToggle;
    ControlObject* m_pJog;
    Rotary* m_pJogFilter;

    ControlObject *m_pSampleRate;

    // Enumerations which hold the state of the pitchbend buttons.
    // These enumerations can be used like a bitmask.
    enum RATERAMP_DIRECTION {
        RATERAMP_NONE = 0,	// No buttons are held down
        RATERAMP_DOWN = 1,	// Down button is being held
        RATERAMP_UP = 2,	// Up button is being held
        RATERAMP_BOTH = 3	// Both buttons are being held down
    };

    // Rate ramping mode:
    //	RATERAMP_STEP: pitch takes a temporary step up/down a certain amount.
    //  RATERAMP_LINEAR: pitch moves up/down in a progresively linear fashion.
    enum RATERAMP_MODE {
        RATERAMP_STEP = 0,
        RATERAMP_LINEAR = 1
    };

    // This defines how the rate returns to normal. Currently unused.
    // Rate ramp back mode:
    //	RATERAMP_RAMPBACK_NONE: returns back to normal all at once.
    //	RATERAMP_RAMPBACK_SPEED: moves back in a linearly progresive manner.
    //	RATERAMP_RAMPBACK_PERIOD: returns to normal within a period of time.
    enum RATERAMP_RAMPBACK_MODE {
        RATERAMP_RAMPBACK_NONE,
        RATERAMP_RAMPBACK_SPEED,
        RATERAMP_RAMPBACK_PERIOD
    };

    /** Is vinyl control enabled? **/
    bool m_bVinylControlEnabled;
    bool m_bVinylControlScratching;

    // The current rate ramping direction. Only holds the last button pressed.
    int m_ePbCurrent;
    //  The rate ramping buttons which are currently being pressed.
    int m_ePbPressed;

    /** This is true if we've already started to ramp the rate */
    bool m_bTempStarted;
    /** Set to the rate change used for rate temp */
    double m_dTempRateChange;
    /** Set the Temporary Rate Change Mode */
    static enum RATERAMP_MODE m_eRateRampMode;
    /** The Rate Temp Sensitivity, the higher it is the slower it gets */
    static int m_iRateRampSensitivity;
    /** Temporary pitchrate, added to the permanent rate for calculateRate */
    double m_dRateTemp;
    /** */
    enum RATERAMP_RAMPBACK_MODE m_eRampBackMode;
    /** Return speed for temporary rate change */
    double m_dRateTempRampbackChange;

    /** Old playback rate. Stored in this variable while a temp pitch change
      * buttons is in effect. It does not work to just decrease the pitch slider
      * by the value it has been increased with when the temp button was
      * pressed, because there is a fixed limit on the range of the pitch
      * slider */
    double m_dOldRate;
};

#endif /* RATECONTROL_H */
