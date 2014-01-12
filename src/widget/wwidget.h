/***************************************************************************
                          wwidget.h  -  description
                             -------------------
    begin                : Wed Jun 18 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef WWIDGET_H
#define WWIDGET_H

#include <QWidget>
#include <QEvent>
#include <QString>

#include "configobject.h"
#include "widget/wbasewidget.h"

/**
  * Abstract class used in widgets connected to ControlObjects. Derived
  * widgets can implement the signal and slot for manipulating the widgets
  * value. The widgets internal value should match that of a MIDI control.
  * The ControlObject can contain another mapping of the MIDI/Widget
  * value, but the mapping should always be done in the ControlObject.
  *
  *@author Tue & Ken Haste Andersen
  */

class WWidget : public QWidget, public WBaseWidget {
   Q_OBJECT
public:
    WWidget(QWidget *parent=0, Qt::WindowFlags flags=0);
    virtual ~WWidget();

    Q_PROPERTY(bool controlDisabled READ controlDisabled);
    Q_PROPERTY(double value READ getControlParameterDisplay);

    virtual void onConnectedControlValueChanged(double value);

  protected:
    bool event(QEvent* pEvent);
};

#endif
