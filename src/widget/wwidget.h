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
#include <QString>
#include <QDomNode>

#include "configobject.h"

/**
  * Abstract class used in widgets connected to ControlObjects. Derived
  * widgets can implement the signal and slot for manipulating the widgets
  * value. The widgets internal value should match that of a MIDI control.
  * The ControlObject can contain another mapping of the MIDI/Widget
  * value, but the mapping should always be done in the ControlObject.
  *
  *@author Tue & Ken Haste Andersen
  */

class WWidget : public QWidget  {
   Q_OBJECT
public:
    WWidget(QWidget *parent=0, Qt::WindowFlags flags=0);
    virtual ~WWidget();

    // Sometimes WWidget's compose a QWidget (like a label). This is used during
    // skin parsing to style and size the composed widget.
    virtual QWidget* getComposedWidget() { return NULL; }

    Q_PROPERTY(bool disabled READ isDisabled);
    Q_PROPERTY(double value READ getValue);

    bool isDisabled() const {
        return m_bOff;
    }

    double getValue() const {
        return m_value;
    }

  public slots:
    virtual void slotConnectedValueChanged(double value);
    void updateValue(double value);
    void setOnOff(double);

  private slots:
    void slotReEmitValueDown(double);
    void slotReEmitValueUp(double);

  signals:
    void valueReset();
    void valueChangedDown(double);
    void valueChangedUp(double);
    void valueChangedLeftDown(double);
    void valueChangedLeftUp(double);
    void valueChangedRightDown(double);
    void valueChangedRightUp(double);

  protected:
    // Value/state of widget
    double m_value;

  private:
    // Is true if widget is off
    bool m_bOff;
};

#endif
