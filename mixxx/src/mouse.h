//
// C++ Interface: mouse
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef MOUSE_H
#define MOUSE_H

#include "rotary.h"
#include <qptrlist.h>

class QStringList;

/**
@author Tue Haste Andersen
*/
class Mouse : public Rotary
{
public:
    Mouse();
    ~Mouse();
    static void destroyAll();
    static QStringList getDeviceList();

protected:
    static QPtrList<Mouse> m_sqInstanceList;
};

#endif

