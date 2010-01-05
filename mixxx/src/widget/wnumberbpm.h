/***************************************************************************
                          wnumberbpm.h  -  description
                             -------------------
    begin                : Wed Oct 31 2003
    copyright            : (C) 2003 by Tue Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef WNUMBERBPM_H
#define WNUMBERBPM_H

#include "wnumber.h"
class ControlObjectThreadMain;

/**
@author Tue Haste Andersen
*/
class WNumberBpm : public WNumber
{
    Q_OBJECT
public:
    WNumberBpm(const char *group, QWidget *parent = 0);
    ~WNumberBpm();

public slots:
    /** Sets the value */
    void setValue(double dValue);

private:
    /** Pointer to control object for rate */
    ControlObjectThreadMain *m_pRateControl, *m_pRateDirControl, *m_pRateRangeControl, *m_pBpmControl;
    static bool m_bScaleBpm;
};

#endif
