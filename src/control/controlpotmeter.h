#pragma once

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "control/controlpushbutton.h"
#include "preferences/usersettings.h"

class PotmeterControls : public QObject {
    Q_OBJECT
  public:
    PotmeterControls(const ConfigKey& key);
    virtual ~PotmeterControls();

    void setStepCount(int count) {
        m_stepCount = count;
    }

    void setSmallStepCount(int count) {
        m_smallStepCount = count;
    }

    void addAlias(const ConfigKey& key);

  public slots:
    // Increases the value.
    void incValue(double);
    // Decreases the value.
    void decValue(double);
    // Increases the value by smaller step.
    void incSmallValue(double);
    // Decreases the value by smaller step.
    void decSmallValue(double);
    // Sets the value to 1.0.
    void setToOne(double);
    // Sets the value to -1.0.
    void setToMinusOne(double);
    // Sets the value to 0.0.
    void setToZero(double);
    // Sets the control to its default
    void setToDefault(double);
    // Toggles the value between 0.0 and 1.0.
    void toggleValue(double);
    // Toggles the value between -1.0 and 0.0.
    void toggleMinusValue(double);
    void setIsDefault(bool isDefault);

  private:
    ControlProxy m_control;
    ControlPushButton m_controlUp;
    ControlPushButton m_controlDown;
    ControlPushButton m_controlUpSmall;
    ControlPushButton m_controlDownSmall;
    ControlPushButton m_controlSetDefault;
    ControlPushButton m_controlSetZero;
    ControlPushButton m_controlSetOne;
    ControlPushButton m_controlSetMinusOne;
    ControlPushButton m_controlToggle;
    ControlPushButton m_controlMinusToggle;
    int m_stepCount;
    double m_smallStepCount;
};

class ControlPotmeter : public ControlObject {
    Q_OBJECT
  public:
    ControlPotmeter(const ConfigKey& key,
            double dMinValue = 0.0,
            double dMaxValue = 1.0,
            bool allowOutOfBounds = false,
            bool bIgnoreNops = true,
            bool bTrack = false,
            bool bPersist = false,
            double defaultValue = 0.0);
    ~ControlPotmeter() override = default;

    // Sets the step count of the associated PushButtons.
    void setStepCount(int count);

    // Sets the small step count of the associated PushButtons.
    void setSmallStepCount(int count);

    // Sets the minimum and maximum allowed value. The control value is reset
    // when calling this method
    void setRange(double dMinValue, double dMaxValue, bool allowOutOfBounds);

    void addAlias(const ConfigKey& key) {
        ControlObject::addAlias(key);
        m_controls.addAlias(key);
    };

  private slots:
    // Used to check if the current control value matches the default value.
    void privateValueChanged(double dValue, QObject* pSender);

  protected:
    bool m_bAllowOutOfBounds;
    PotmeterControls m_controls;
};
