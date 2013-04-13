// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2004
// Copyright: See COPYING file that comes with this distribution
#ifndef CONTROLOBJECTTHREADMAIN_H
#define CONTROLOBJECTTHREADMAIN_H

#include <QEvent>

#include "controlobjectthread.h"

class ControlObject;

// ControlObjectThreadMain is a variant of ControlObjectThread that should only
// ever have its methods called by the main thread. The benefit is that the
// valueChanged() signal is proxied to the main thread automatically and the
// get()/set()/add()/sub() methods are lock-free and performant relative to
// ControlObjectThread since COT requires using a mutex to maintain the
// integrity of the control value. If you create a COTM, you must make sure to
// only call its methods from the main thread.
class ControlObjectThreadMain : public ControlObjectThread {
    Q_OBJECT
  public:
    ControlObjectThreadMain(ControlObject *pControlObject, QObject* pParent=NULL);
    virtual ~ControlObjectThreadMain();
    /** Event filter */
    bool eventFilter(QObject *o, QEvent *e);
    /** Notify this object through events */
    virtual bool setExtern(double v);

    // No lock is needed for get()/add()/sub()/slotSet(). See class comment above.
    virtual inline double get() { return m_dValue; }
    virtual inline void add(double v) {
        // Would not be safe if m_dValue were not safe for use in the main
        // thread!
        slotSet(m_dValue + v);
    }
    virtual inline void sub(double v) {
        // Would not be safe if m_dValue were not safe for use in the main
        // thread!
        slotSet(m_dValue - v);
    }
  public slots:
    virtual inline void slotSet(double v) {
        m_dValue = v;
        updateControlObject(v);
    }
};

#endif
