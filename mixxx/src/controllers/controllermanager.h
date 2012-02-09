/**
  * @file controllermanager.h
  * @author Sean Pappalardo spappalardo@mixxx.org
  * @date Sat Apr 30 2011
  * @brief Manages creation/enumeration/deletion of hardware controllers.
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
// #include "midienumerator.h"  // TODO
#ifdef __HID__
    #include "hidenumerator.h"
#endif
#ifdef __OSC__
    #include "oscenumerator.h"
#endif

//Forward declaration(s)
class Controller;
class ControllerManager;

class ControllerProcessor : public QThread {
Q_OBJECT
    public:
        ControllerProcessor(ControllerManager *pManager);
        ~ControllerProcessor();
        void startPolling();
        void stopPolling();
        
        bool polling;
    protected:
        void timerEvent(QTimerEvent *event);
    private:
        int m_pollingTimerId;
        ControllerManager *m_pManager;
};

/** Manages enumeration/operation/deletion of hardware controllers. */
class ControllerManager : public QObject {
Q_OBJECT
    public:
        ControllerManager(ConfigObject<ConfigValue> * pConfig);
        ~ControllerManager();
        QList<Controller*> getControllers() { return m_controllers; };
        QList<Controller*> getControllerList(bool outputDevices=true, bool inputDevices=true);
        QList<QString> getPresetList(bool midi=false);
        ConfigObject<ConfigValue>* getDeviceSettings() { return m_pDeviceSettings; };
        void startThread();
        // Prevent other parts of Mixxx from having to manually connect to our slots
        void setUpDevices() { emit(requestSetUpDevices()); };
        void savePresets(bool onlyActive=false) { emit(requestSave(onlyActive)); };
        void shutdown() { emit(requestShutdown()); };
    signals:
        void devicesChanged();
        void requestSetUpDevices();
        void requestShutdown();
        void requestSave(bool onlyActive);
    public slots:
        void updateControllerList();
        /** Writes out presets for currently connected input devices */
        void slotSavePresets(bool onlyActive=false);
    protected:
        QList<Controller*> m_controllers;
    private slots:
        int slotSetUpDevices();
        void slotShutdown();
        
    private:
        ConfigObject<ConfigValue> *m_pDeviceSettings;
        ConfigObject<ConfigValue> *m_pConfig;
        ControllerProcessor *m_pProcessor;
        QMutex m_mControllerList;

//         MidiEnumerator *m_pMIDIEnumerator;   // TODO
#ifdef __HID__
        HidEnumerator *m_pHIDEnumerator;
#endif
#ifdef __OSC__
        OscEnumerator *m_pOSCEnumerator;
#endif
};
    
#endif  // CONTROLLERMANAGER_H