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

#include "preferences/usersettings.h"
#include "util/color/color.h"
#include "widget/wbasewidget.h"

class ControlProxy;

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
    explicit WWidget(QWidget *parent=nullptr, Qt::WindowFlags flags=nullptr);
    ~WWidget() override;

    Q_PROPERTY(double value READ getControlParameterDisplay);
    Q_PROPERTY(double backgroundColorRgb READ getBackgroundColorRgb WRITE
                    setBackgroundColorRgb);
    Q_PROPERTY(bool shouldHighlightBackgroundOnHover MEMBER
                    m_bShouldHighlightBackgroundOnHover);
    Q_PROPERTY(bool hasBackgroundColor READ hasBackgroundColor);
    Q_PROPERTY(bool backgroundIsDark READ backgroundIsDark);

    double getBackgroundColorRgb() const;
    void setBackgroundColorRgb(double rgb);
    bool hasBackgroundColor() const {
        return m_backgroundColorRgb >= 0;
    }
    bool backgroundIsDark() const {
        return m_bBackgroundIsDark;
    }

  protected:
    bool touchIsRightButton();
    bool event(QEvent* e) override;
    void setScaleFactor(double value) {
        m_scaleFactor = value;
    }
    double scaleFactor() {
        return m_scaleFactor;
    }

    enum Qt::MouseButton m_activeTouchButton;

  private:
    ControlProxy* m_pTouchShift;
    double m_scaleFactor;
    double m_backgroundColorRgb;
    bool m_bBackgroundIsDark;
    bool m_bShouldHighlightBackgroundOnHover;
};

#endif
