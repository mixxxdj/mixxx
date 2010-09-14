//
// C++ Implementation: controlobjecthreadwidget.cpp
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <qapplication.h>
#include "controlobjectthreadwidget.h"
#include "controlobject.h"
#include "controlevent.h"

ControlObjectThreadWidget::ControlObjectThreadWidget(ControlObject * pControlObject) : ControlObjectThreadMain(pControlObject)
{
    // Initialize value
    m_dValue = m_pControlObject->getValueToWidget(m_pControlObject->get());
    emitValueChanged();
}

ControlObjectThreadWidget::~ControlObjectThreadWidget()
{
}

void ControlObjectThreadWidget::setWidget(QWidget * widget, bool connectValueFromWidget, bool connectValueToWidget, bool emitOnDownPress, Qt::ButtonState state)
{

    if (connectValueFromWidget) {
        if (emitOnDownPress)
        {
            if (state == Qt::NoButton)
                QApplication::connect(widget, SIGNAL(valueChangedDown(double)), this,   SLOT(slotSet(double)));
            else if (state == Qt::LeftButton)
                QApplication::connect(widget, SIGNAL(valueChangedLeftDown(double)), this,   SLOT(slotSet(double)));
            else if (state == Qt::RightButton)
                QApplication::connect(widget, SIGNAL(valueChangedRightDown(double)), this,   SLOT(slotSet(double)));
        }
        else
        {
            if (state == Qt::NoButton)
                QApplication::connect(widget, SIGNAL(valueChangedUp(double)), this,   SLOT(slotSet(double)));
            else if (state == Qt::LeftButton)
                QApplication::connect(widget, SIGNAL(valueChangedLeftUp(double)), this,   SLOT(slotSet(double)));
            else if (state == Qt::RightButton)
                QApplication::connect(widget, SIGNAL(valueChangedRightUp(double)), this,   SLOT(slotSet(double)));
        }
    }

    if (connectValueToWidget)
        QApplication::connect(this,   SIGNAL(valueChanged(double)),    widget, SLOT(setValue(double)));
    emitValueChanged();
}

void ControlObjectThreadWidget::setWidgetOnOff(QWidget * widget)
{
    QApplication::connect(this,   SIGNAL(valueChanged(double)),    widget, SLOT(setOnOff(double)));
    emit(valueChanged(m_dValue));
}

void ControlObjectThreadWidget::updateControlObject()
{
    m_pControlObject->queueFromThread(m_pControlObject->getValueFromWidget(get()), this);
}

bool ControlObjectThreadWidget::setExtern(double v)
{
    //qDebug() << "set extern widget";
    QApplication::postEvent(this, new ControlEvent(m_pControlObject->getValueToWidget(v)));
    return true;
}
