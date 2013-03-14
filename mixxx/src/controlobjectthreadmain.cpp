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
}


