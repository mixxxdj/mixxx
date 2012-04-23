/**
* @file hss1394enumerator.h
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Thu 15 Mar 2012
* @brief This class handles discovery and enumeration of DJ controllers that appear under the HSS1394 cross-platform API.
*/

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
