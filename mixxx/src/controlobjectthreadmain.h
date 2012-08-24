//
// C++ Interface: controlobjectthreadmain.h
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CONTROLOBJECTTHREADMAIN_H
#define CONTROLOBJECTTHREADMAIN_H

#include "controlobjectthread.h"
//Added by qt3to4:
#include <QEvent>

class ControlObject;

/**
@author Tue Haste Andersen
*/

class ControlObjectThreadMain : public ControlObjectThread
{
    Q_OBJECT
public:
    ControlObjectThreadMain(ControlObject *pControlObject, QObject* pParent=NULL);
    virtual ~ControlObjectThreadMain();
    /** Event filter */
    bool eventFilter(QObject *o, QEvent *e);
    /** Notify this object through events */
    bool setExtern(double v);
};

#endif
