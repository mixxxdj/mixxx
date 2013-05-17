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

bool ControlObjectThreadMain::eventFilter(QObject* o, QEvent* e) {
    // Handle events
    if (e && e->type() == MIXXXEVENT_CONTROL) {
        ControlEvent * ce = (ControlEvent *)e;
        emit(valueChanged(ce->value()));
    } else {
        return QObject::eventFilter(o,e);
    }
    return true;
}

void ControlObjectThreadMain::slotValueChanged(double, QObject* pSetter) {
    // The value argument to this function is in value space, but for some of
    // our subclasses (e.g. ControlObjectThreadWidget) emit valueChanged(double)
    // should emit in parameter space. So we emit the value of get() instead
    // which will get the correct value.

    // If we are already running in the main thread, then go ahead and update.
    if (QThread::currentThread() == QApplication::instance()->thread()) {
        if (pSetter != this) {
            emit(valueChanged(get()));
        } else {
            emit(valueChangedByThis(get()));
        }
    } else {
        // Otherwise, we have to post the event to the main thread event queue
        // and then catch the event via eventFilter.
        QApplication::postEvent(this, new ControlEvent(get()));
    }

}
