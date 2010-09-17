//
// C++ Interface: controlobjecthreadwidget.h
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CONTROLOBJECTTHREADWIDGET_H
#define CONTROLOBJECTTHREADWIDGET_H

#include "controlobjectthreadmain.h"

class ControlObject;

/**
@author Tue Haste Andersen
*/

class ControlObjectThreadWidget : public ControlObjectThreadMain
{
    Q_OBJECT
public:
    ControlObjectThreadWidget(ControlObject *pControlObject);
    ~ControlObjectThreadWidget();
    /** Associates a QWidget with the ControlObject. */
    void setWidget(QWidget *widget,
                   bool connectValueFromWidget=true, bool connectValueToWidget = true,
                   bool emitOnDownPress=true, Qt::ButtonState state=Qt::NoButton);
    /** Associates a the enabled/disabled state of a widget with the state of a ControlObject. */
    void setWidgetOnOff(QWidget *widget);
    bool setExtern(double v);

private:
    void updateControlObject();
};

#endif
