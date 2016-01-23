//
// C++ Interface: wnumberpos
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef WNUMBERRATE_H
#define WNUMBERRATE_H

#include "widget/wnumber.h"

class ControlObjectSlave;

class WNumberRate : public WNumber {
    Q_OBJECT
  public:
    WNumberRate(const char *group, QWidget *parent=0);
    virtual ~WNumberRate();

  private slots:
    void setValue(double dValue);

  private:
    // Pointer to control objects for rate.
    ControlObjectSlave* m_pRateControl;
    ControlObjectSlave* m_pRateRangeControl;
    ControlObjectSlave* m_pRateDirControl;
};

#endif
