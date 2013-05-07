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

#include <QWidget>

#include "controlobjectthreadmain.h"

class ControlObject;

/**
@author Tue Haste Andersen
*/

class ControlObjectThreadWidget : public ControlObjectThreadMain
{
    Q_OBJECT
public:

    enum EmitOption {
        EMIT_NEVER                = 0x00,
        EMIT_ON_PRESS             = 0x01,
        EMIT_ON_RELEASE           = 0x02,
        EMIT_ON_PRESS_AND_RELEASE = 0x03
    };

    ControlObjectThreadWidget(ControlObject *pControlObject, QObject* pParent=NULL);
    virtual ~ControlObjectThreadWidget();
    /** Associates a QWidget with the ControlObject. */
    void setWidget(QWidget *widget,
                   bool connectValueFromWidget=true, bool connectValueToWidget = true,
                   EmitOption emitOption=EMIT_ON_PRESS, Qt::MouseButton state=Qt::NoButton);
    /** Associates a the enabled/disabled state of a widget with the state of a ControlObject. */
    void setWidgetOnOff(QWidget *widget);
    bool setExtern(double v);

  private slots:
    void slotReset();

  private:
    virtual void updateControlObject(double v);
};

#endif
