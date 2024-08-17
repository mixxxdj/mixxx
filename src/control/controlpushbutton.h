/***************************************************************************
                          controlpushbutton.h  -  description
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

#ifndef CONTROLPUSHBUTTON_H
#define CONTROLPUSHBUTTON_H

#include "control/controlbuttonmode.h"
#include "control/controlobject.h"

/**
  *@author Tue and Ken Haste Andersen
  */

class ControlPushButton : public ControlObject {
    Q_OBJECT
  public:
    static QString buttonModeToString(ControlButtonMode mode) {
        switch(mode) {
        case ControlButtonMode::PUSH:
            return "PUSH";
        case ControlButtonMode::TOGGLE:
            return "TOGGLE";
        case ControlButtonMode::POWERWINDOW:
            return "POWERWINDOW";
        case ControlButtonMode::LONGPRESSLATCHING:
            return "LONGPRESSLATCHING";
        case ControlButtonMode::TRIGGER:
            return "TRIGGER";
        default:
            return "UNKNOWN";
        }
    }

    ControlPushButton(const ConfigKey& key, bool bPersist = false, double defaultValue = 0.0);
    virtual ~ControlPushButton();

    inline ControlButtonMode getButtonMode() const {
        return m_buttonMode;
    }
    void setButtonMode(ControlButtonMode mode);
    void setStates(int num_states);
    void setBehavior(ControlButtonMode mode, int num_states);

  private:
    void updateBehavior();
    enum ControlButtonMode m_buttonMode;
    int m_iNoStates;
};

#endif
