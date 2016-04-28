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

class ControlProxy;

class WNumberRate : public WNumber {
    Q_OBJECT
  public:
    explicit WNumberRate(const char *group, QWidget *parent=nullptr);

  private slots:
    void setValue(double dValue) override;

  private:
    // Pointer to control objects for rate.
    ControlProxy* m_pRateControl;
    ControlProxy* m_pRateRangeControl;
    ControlProxy* m_pRateDirControl;
};

#endif
