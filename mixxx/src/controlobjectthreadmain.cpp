//
// C++ Implementation: controlobjecthreadmainp.cpp
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <QApplication>
#include <QtDebug>
//Added by qt3to4:
#include <QEvent>
#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "controlevent.h"

ControlObjectThreadMain::ControlObjectThreadMain(ControlObject * pControlObject, QObject* pParent)
        : ControlObjectThread(pControlObject, pParent) {
    setObjectName("ControlObjectThreadMain");
    installEventFilter(this);
}

ControlObjectThreadMain::~ControlObjectThreadMain() {
}

bool ControlObjectThreadMain::eventFilter(QObject * o, QEvent * e)
{
    // Handle events
    if (e->type() == MIXXXEVENT_CONTROL)
    {
        ControlEvent * ce = (ControlEvent *)e;

        m_dataMutex.lock();
        m_dValue = ce->value();
        m_dataMutex.unlock();

        //qDebug() << "ControlEvent " << ce->value();
        emit(valueChanged(ce->value()));
    }
    else
    {
        // standard event processing
        return QObject::eventFilter(o,e);
    }
    return true;
}

bool ControlObjectThreadMain::setExtern(double v)
{
    //qDebug() << "set extern main";
    QApplication::postEvent(this, new ControlEvent(v));
    return true;
}
