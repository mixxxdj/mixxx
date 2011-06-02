/**
  * @file controllermanager.h
  * @author Sean Pappalardo spappalardo@mixxx.org
  * @date Sat Apr 30 2011
  * @brief Manages creation/enumeration/deletion of (non-MIDI) hardware controllers.
  */

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef CONTROLLERMANAGER_H
#define CONTROLLERMANAGER_H

#include "configobject.h"
#ifdef __HID__
    #include "hidenumerator.h"
#endif
#ifdef __OSC__
    #include "oscenumerator.h"
#endif

class Controller;

/** Manages creation/enumeration/deletion of (non-MIDI) hardware controllers. */
class ControllerManager : public QObject
{
    Q_OBJECT
    public:
        ControllerManager(ConfigObject<ConfigValue> * pConfig);
        ~ControllerManager();
        QList<Controller*> getControllerList(bool outputDevices=true, bool inputDevices=true);
        QList<QString> getPresetList(bool midi=false);
        int setupDevices();
        ConfigObject<ConfigValue>* getDeviceSettings() { return m_pDeviceSettings; };
    signals:
        void devicesChanged();
    private:
        QList<Controller*> m_controllers;
        ConfigObject<ConfigValue> *m_pDeviceSettings;
        ConfigObject<ConfigValue> *m_pConfig;
//         MidiEnumerator *m_pMIDIEnumerator;   // TODO
#ifdef __HID__
        HidEnumerator *m_pHIDEnumerator;
#endif
#ifdef __OSC__
        OscEnumerator *m_pOSCEnumerator;
#endif
    };
    
#endif  // CONTROLLERMANAGER_H