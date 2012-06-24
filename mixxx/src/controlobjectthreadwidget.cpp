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

#include <QtDebug>
#include <qapplication.h>
#include "controlobjectthreadwidget.h"
#include "controlobject.h"
#include "controlevent.h"

ControlObjectThreadWidget::ControlObjectThreadWidget(ControlObject * pControlObject, QObject* pParent)
        : ControlObjectThreadMain(pControlObject, pParent) {
    // Initialize value
    m_dValue = m_pControlObject->getValueToWidget(m_pControlObject->get());
    emitValueChanged();
}

ControlObjectThreadWidget::~ControlObjectThreadWidget() {
}

void ControlObjectThreadWidget::setWidget(QWidget * widget, bool connectValueFromWidget,
                                          bool connectValueToWidget,
                                          EmitOption emitOption, Qt::MouseButton state)
{

    if (connectValueFromWidget) {
        connect(widget, SIGNAL(valueReset()),
                this, SLOT(slotReset()));

        if (emitOption & EMIT_ON_PRESS) {
            if (state == Qt::NoButton)
                connect(widget, SIGNAL(valueChangedDown(double)),
                        this, SLOT(slotSet(double)));
            else if (state == Qt::LeftButton)
                connect(widget, SIGNAL(valueChangedLeftDown(double)),
                        this, SLOT(slotSet(double)));
            else if (state == Qt::RightButton)
                connect(widget, SIGNAL(valueChangedRightDown(double)),
                        this, SLOT(slotSet(double)));
        }

        if (emitOption & EMIT_ON_RELEASE) {
            if (state == Qt::NoButton)
                connect(widget, SIGNAL(valueChangedUp(double)),
                        this, SLOT(slotSet(double)));
            else if (state == Qt::LeftButton)
                connect(widget, SIGNAL(valueChangedLeftUp(double)),
                        this, SLOT(slotSet(double)));
            else if (state == Qt::RightButton)
                connect(widget, SIGNAL(valueChangedRightUp(double)),
                        this, SLOT(slotSet(double)));
        }
    }

    if (connectValueToWidget) {
        connect(this, SIGNAL(valueChanged(double)),
                widget, SLOT(setValue(double)));
    }
    emitValueChanged();
}

void ControlObjectThreadWidget::setWidgetOnOff(QWidget * widget)
{
    QApplication::connect(this,   SIGNAL(valueChanged(double)),    widget, SLOT(setOnOff(double)));
    emit(valueChanged(m_dValue));
}

void ControlObjectThreadWidget::slotReset() {
    double defaultValue = m_pControlObject->defaultValue();
    // defaultValue is a control value. slotSet needs to be set with a widget
    // value since widget value-changed signals connect to it. setExtern needs
    // to be called with a control value since it is triggered by control
    // updates.
    slotSet(m_pControlObject->getValueToWidget(defaultValue));
    setExtern(defaultValue);
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
