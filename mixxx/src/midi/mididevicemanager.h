/**
  * @file mididevicemanager.h
  * @author Albert Santoni alberts@mixxx.org
  * @date Thu Dec 18 2008
  * @brief Manages creation/enumeration/deletion of MidiDevices.
  */

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef MIDIDEVICEMANAGER_H
#define MIDIDEVICEMANAGER_H

#include "configobject.h"
#include "portmidienumerator.h"
#ifdef __HSS1394__
    #include "hss1394enumerator.h"
#endif

class MidiDevice;
class DlgPrefMidiBindings;

/** Manages creation/enumeration/deletion of MidiDevices. */
class MidiDeviceManager : public QObject
{
    Q_OBJECT
    public:
        MidiDeviceManager(ConfigObject<ConfigValue> * pConfig);
        ~MidiDeviceManager();
        QList<MidiDevice*> getDeviceList(bool bOutputDevices=true, bool bInputDevice=true);
        QStringList getConfigList(QString path);
        void saveMappings(bool onlyActive=false);
        //void closeDevices();
        int setupDevices();
        //ConfigObject<ConfigValue>* getDeviceSettings() { return m_pDeviceSettings; };
        void associateInputAndOutputDevices(MidiDevice* inputDevice, QString outputDeviceName);
    signals:
        void devicesChanged();
    private:
        QList<MidiDevice*> m_devices;
        ConfigObject<ConfigValue> *m_pDeviceSettings;
        ConfigObject<ConfigValue> *m_pConfig;
        PortMidiEnumerator *m_pPMEnumerator;
#ifdef __HSS1394__
        Hss1394Enumerator *m_pHSSEnumerator;
#endif
    };
    
#endif
