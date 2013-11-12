// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2004
// Copyright: See COPYING file that comes with this distribution
#ifndef CONTROLOBJECTTHREADMAIN_H
#define CONTROLOBJECTTHREADMAIN_H

#include <QEvent>

#include "controlobjectthread.h"

// ControlObjectThreadMain is a variant of ControlObjectThread that should only
// ever have its methods called by the main thread. The benefit is that the
// valueChanged() signal is proxied to the main thread automatically and the
// get()/set() methods are lock-free and performant relative to
// ControlObjectThread since COT requires using a mutex to maintain the
// integrity of the control value. If you create a COTM, you must make sure to
// only call its methods from the main thread.
class ControlObjectThreadMain : public ControlObjectThread {
    Q_OBJECT
  public:
    ControlObjectThreadMain(const ConfigKey& key, QObject* pParent = NULL);
    ControlObjectThreadMain(const QString& g, const QString& i, QObject* pParent = NULL);
    ControlObjectThreadMain(const char* g, const char* i, QObject* pParent = NULL);

    bool eventFilter(QObject* o, QEvent* e);

  protected slots:
    // Receives the value from the master control, proxies it to the main
    // thread, and re-emits either valueChanged(double) or
    // valueChangedByThis(double) based on pSetter.
    virtual void slotValueChanged(double v, QObject* pSetter);

  private:
    void initialize();
};

#endif
