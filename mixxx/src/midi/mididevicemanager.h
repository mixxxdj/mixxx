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
class MidiDevice;
class MidiLearnListener;
class MidiLearnProcessor;
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
        void closeDevices();
        void queryDevices();
        int setupDevices();
        ConfigObject<ConfigValue>* getDeviceSettings() { return m_pDeviceSettings; };
        void associateInputAndOutputDevices(MidiDevice* inputDevice, QString outputDeviceName);
    /*    MidiDevice* getPrimaryMidiDevice() { return m_pPrimaryMidiDevice; }; //HACK while our code still sucks
        void enableMidiLearn(DlgPrefMidiBindings* listener);
        void disableMidiLearn(); */ //Moving MIDI learning into MIDI device, a la 1.7.0
    signals:
        void devicesChanged();
    private:
        QList<MidiDevice*> m_devices;
        ConfigObject<ConfigValue> *m_pDeviceSettings;
        ConfigObject<ConfigValue> *m_pConfig;
    };
    
#endif
