// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2004
// Copyright: See COPYING file that comes with this distribution

#include <QApplication>
#include <QtDebug>
#include <QEvent>
#include <QThread>

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
    if (e && e->type() == MIXXXEVENT_CONTROL) {
        ControlEvent * ce = (ControlEvent *)e;
        //qDebug() << "ControlEvent " << ce->value();
        m_dValue = ce->value();
        emit(valueChanged(ce->value()));
    } else {
        // standard event processing
        return QObject::eventFilter(o,e);
    }
    return true;
}

bool ControlObjectThreadMain::setExtern(double v)
{
    // If we are already running in the main thread, then go ahead and update.
    if (QThread::currentThread() == QApplication::instance()->thread()) {
        m_dValue = v;
        emit(valueChanged(v));
    } else {
        // Otherwise, we have to post the event to the main thread event queue
        // and then catch the event via eventFilter.
        QApplication::postEvent(this, new ControlEvent(v));
    }
    return true;
}
