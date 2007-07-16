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

#include <q3ptrlist.h>
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
    static QStringList getMappings();
    void selectMapping(QString mapping);
    static void destroyAll();
    static QStringList getDeviceList();
    /** Start calibration measurement */
    void calibrateStart();
    /** End calibration measurement */
    double calibrateEnd();
    /** Set calibration */
    void setCalibration(double c);

protected:
    static Q3PtrList<Mouse> m_sqInstanceList;
    /** Pointer to rotary filtering object */
    Rotary *m_pRotary;
    /** Pointer to control objects connected to the PowerMate */
    ControlObject *m_pControlObjectRotary;
};

#endif

