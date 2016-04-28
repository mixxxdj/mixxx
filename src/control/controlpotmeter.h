/***************************************************************************
                          controlpotmeter.h  -  description
                             -------------------
    begin                : Wed Feb 20 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONTROLPOTMETER_H
#define CONTROLPOTMETER_H

#include "preferences/usersettings.h"
#include "control/controlobject.h"

/**
  *@author Tue and Ken Haste Andersen
  */

class ControlPushButton;
class ControlProxy;

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

  private:
    ControlProxy* m_pControl;
    int m_stepCount;
    double m_smallStepCount;
};

class ControlPotmeter : public ControlObject {
    Q_OBJECT
  public:
    ControlPotmeter(ConfigKey key, double dMinValue = 0.0, double dMaxValue = 1.0,
                    bool allowOutOfBounds = false,
                    bool bIgnoreNops = true,
                    bool bTrack = false,
                    bool bPersist = false);
    virtual ~ControlPotmeter();

    // Sets the step count of the associated PushButtons.
    void setStepCount(int count);

    // Sets the small step count of the associated PushButtons.
    void setSmallStepCount(int count);

    // Sets the minimum and maximum allowed value. The control value is reset
    // when calling this method
    void setRange(double dMinValue, double dMaxValue, bool allowOutOfBounds);

  protected:
    bool m_bAllowOutOfBounds;
    PotmeterControls m_controls;
};

#endif
