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

#include "wnumber.h"
class ControlObject;

/**
@author Tue Haste Andersen
*/

class WNumberRate : public WNumber
{
    Q_OBJECT
public:
    WNumberRate(const char *group, QWidget *parent=0, const char *name=0);
    ~WNumberRate();
    void setValue(double dValue);

private:
    /** Pointer to control object for rate */
    ControlObject *m_pRateControl, *m_pRateDirControl;
};

#endif
