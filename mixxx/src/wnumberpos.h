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
class ControlObject;

/**
@author Tue Haste Andersen
*/

class WNumberPos : public WNumber
{
    Q_OBJECT
public:
    WNumberPos(const char *group, QWidget *parent=0, const char *name=0);
    ~WNumberPos();
    void setDuration(int iDuration);
    void setValue(double dValue);
    /** Set if the display shows remaining time (true) or position (false) */
    void setRemain(bool bRemain);
private:
    /** Duration in seconds */
    int m_iDuration;
    /** True if remaining content is being shown */
    bool m_bRemain;
    /** Pointer to control object for rate */
    ControlObject *m_pRateControl, *m_pRateDirControl;
};

#endif
