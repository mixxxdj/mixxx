/***************************************************************************
                   wabstractcontrol.h  -  Abstract Control Widget
                             -------------------
    copyright            : (C) 2007 by Wesley Stessens
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef WABSTRACTCONTROL_H
#define WABSTRACTCONTROL_H

#include <QObject>
#include "wwidget.h"

class WAbstractControl : public WWidget {
   Q_OBJECT
public:
    WAbstractControl(QWidget *parent=0, const char *name=0, float defaultValue=0.);
    ~WAbstractControl();
    /** Sets the default value to be used when resetting the value */
    void setDefaultValue(float value);
protected:
    /** Resets the widgets value */
    void reset();
    /** Default value to be used when resetting the knob */
    float m_fDefaultValue;
    /** True if right mouse button is pressed */
    bool m_bRightButtonPressed;
};

#endif
