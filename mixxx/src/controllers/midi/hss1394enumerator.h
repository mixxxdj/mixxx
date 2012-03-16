/**
* @file hss1394enumerator.h
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Thu 15 Mar 2012
* @brief This class handles discovery and enumeration of DJ controllers that appear under the HSS1394 cross-platform API.
*/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#ifndef HSS1394ENUMERATOR_H
#define HSS1394ENUMERATOR_H

#include "midienumerator.h"

class Hss1394Enumerator : public MidiEnumerator
{
    public:
        Hss1394Enumerator();
        ~Hss1394Enumerator();
        QList<Controller*> queryDevices();
    private:
        QList<Controller*> m_devices;
};

#endif