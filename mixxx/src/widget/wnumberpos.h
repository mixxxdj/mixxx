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
#ifndef WNUMBERPOS_H
#define WNUMBERPOS_H

#include "wnumber.h"

class ControlObjectThreadWidget;
class ControlObjectThreadMain;

/**
@author Tue Haste Andersen
*/

class WNumberPos : public WNumber
{
    Q_OBJECT
public:
    WNumberPos(const char *group, QWidget *parent=0);
    ~WNumberPos();
    void setValue(double dValue);
    /** Set if the display shows remaining time (true) or position (false) */
    void setRemain(bool bRemain);
public slots:
    void slotSetDuration(double dDuration);
  private slots:
    void slotSetRemain(double dRemain);
private:
    /** Duration in seconds */
    double m_dDuration;
    /** Old value set */
    double m_dOldValue;
    /** True if remaining content is being shown */
    bool m_bRemain;
    ControlObjectThreadMain* m_pShowDurationRemaining;
    /** Pointer to control object for rate and duration*/
    ControlObjectThreadWidget *m_pRateControl, *m_pRateDirControl, *m_pDurationControl;
};

#endif
