/**
* @file controller.h
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Sat Apr 30 2011
* @brief Base class representing a physical (or software) controller.
*
* This is a base class representing a physical (or software) controller.
*   It must be inherited by a class that implements it on some API.
*
*   Note that the subclass' destructor should call close() at a minimum.
*/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "controllerengine.h"
#include "controllerpresetfilehandler.h"

class Controller : public QObject {
Q_OBJECT
    friend class ControllerManager; // accesses lots of our stuff, but in the same thread
    friend class ControllerProcessor;   // so our timerEvent() can remain protected
    
    public:
        Controller();
        virtual ~Controller();  // Subclass should call close() at minimum.
        
        /** Returns the extension for the controller (type) preset files.
            This is used by the ControllerManager to display only relevant
            preset files for the controller (type.) */
        virtual QString presetExtension();
        inline QString defaultPreset();
        void setPreset(const ControllerPreset preset) { m_preset = preset; };
        ControllerPreset getPreset() { return m_preset; };
        virtual ControllerPresetFileHandler getFileHandler() {
            return ControllerPresetFileHandler();
        }
        
        bool isOpen() { return m_bIsOpen; };
        bool isOutputDevice() { return m_bIsOutputDevice; };
        bool isInputDevice() { return m_bIsInputDevice; };
        QString getName() { return m_sDeviceName; };
        bool debugging() { return m_bDebug; };
        
        void setPolling(bool needPolling) { m_bPolling = needPolling; };
        bool needPolling() { return m_bPolling; };

        

    protected:
        /** To be called in sub-class' open() functions after opening the
            device but before starting any input polling/processing. */
        void startEngine();
        /** To be called in sub-class' close() functions after stopping any
            input polling/processing but before closing the device. */
        void stopEngine();
        /** By default, this passes the event on to the engine.
            APIs that are not thread-safe or are only non-blocking should poll
            when 'poll' is true and pass the event on to the engine when not. */
        virtual void timerEvent(QTimerEvent *event, bool poll);
        Q_INVOKABLE void send(QList<int> data, unsigned int length);
        
        /** Verbose and unique device name suitable for display. */
        QString m_sDeviceName;
        /** Flag indicating if this device supports output (receiving data from Mixxx)*/
        bool m_bIsOutputDevice;
        /** Flag indicating if this device supports input (sending data to Mixxx)*/
        bool m_bIsInputDevice;
        /** Indicates whether or not the device has been opened for input/output. */
        bool m_bIsOpen;
        /** Specifies whether or not we should dump incoming data to the console at runtime. This is useful
            for end-user debugging and script-writing. */
        bool m_bDebug;

        ControllerEngine *m_pEngine;

    // Making these slots protected/private ensures that other parts of Mixxx
    //  can only signal them, preventing thread contention
    protected slots:
        /** Handles packets of raw bytes and passes them to an ".incomingData"
            script function that is assumed to exist.
            (Sub-classes may want to reimplement this if they have an alternate
            way of handling such data.) */
        virtual void receive(const QByteArray data);
        
        /** Initializes the controller engine */
        virtual void applyPreset();

    private slots:
        virtual int open() = 0;
        virtual int close() = 0;

    private:
        /** This must be reimplmented by sub-classes desiring to send raw bytes
            to a controller. */
        virtual void send(QByteArray data);

        ControllerPreset m_preset;

        bool m_bPolling;
};

#endif
