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

#include "controlobject.h"
#include "controllers/midi/midimessage.h"
#include "defs.h"
#include <QTimer>

/**
  *@author Tue and Ken Haste Andersen
  */

class ControlPushButton : public ControlObject {
    Q_OBJECT
  public:
    enum ButtonMode {
         PUSH,
         TOGGLE,
         POWERWINDOW
    };
    static const int kPowerWindowTimeMillis;

    ControlPushButton(ConfigKey key);
    virtual ~ControlPushButton();

    inline ButtonMode getButtonMode() const {
        return m_buttonMode;
    }
    void setButtonMode(enum ButtonMode mode);
    void setStates(int num_states);

  protected:
    void setValueFromMidi(MidiOpCode o, double v);

  private:
    enum ButtonMode m_buttonMode;
    int m_iNoStates;
    QTimer m_pushTimer;
};

#endif
