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

#include <qptrlist.h>
#include "input.h"

class QStringList;
class Rotary;
class ControlObject;

/**
@author Tue Haste Andersen
*/
class Mouse : public Input
{
public:
    Mouse();
    ~Mouse();
    static void destroyAll();
    static QStringList getDeviceList();

protected:
    static QPtrList<Mouse> m_sqInstanceList;
    /** Pointer to rotary filtering object */
    Rotary *m_pRotary;
    /** Pointer to control objects connected to the PowerMate */
    ControlObject *m_pControlObjectRotary;
};

#endif

