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


/**
@author Tue Haste Andersen
*/

class ControlObjectThreadWidget : public ControlObjectThreadMain {
    Q_OBJECT
  public:
    ControlObjectThreadWidget(const ConfigKey& key, QObject* pParent = NULL);
    ControlObjectThreadWidget(const char* g, const char* i, QObject* pParent = NULL);
    ControlObjectThreadWidget(const QString& g, const QString& i, QObject* pParent = NULL);
    virtual ~ControlObjectThreadWidget();

    virtual double get();

  public slots:
    virtual void set(double v);
};

#endif
