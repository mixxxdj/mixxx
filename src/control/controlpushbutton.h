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

#include "control/controlobject.h"

/**
  *@author Tue and Ken Haste Andersen
  */

class ControlPushButton : public ControlObject {
    Q_OBJECT
  public:
    enum ButtonMode {
         PUSH = 0,
         TOGGLE,
         POWERWINDOW,
         LONGPRESSLATCHING,
         TRIGGER,
    };

    static QString buttonModeToString(int mode) {
        switch(mode) {
            case ControlPushButton::PUSH:
                return "PUSH";
            case ControlPushButton::TOGGLE:
                return "TOGGLE";
            case ControlPushButton::POWERWINDOW:
                return "POWERWINDOW";
            case ControlPushButton::LONGPRESSLATCHING:
                return "LONGPRESSLATCHING";
            case ControlPushButton::TRIGGER:
                return "TRIGGER";
            default:
                return "UNKNOWN";
        }
    }

    ControlPushButton(const ConfigKey& key, bool bPersist = false, double defaultValue = 0.0);
    virtual ~ControlPushButton();

    inline ButtonMode getButtonMode() const {
        return m_buttonMode;
    }
    void setButtonMode(enum ButtonMode mode);
    void setStates(int num_states);

  private:
    enum ButtonMode m_buttonMode;
    int m_iNoStates;
};

#endif
