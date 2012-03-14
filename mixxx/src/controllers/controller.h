/**
* @file controller.h
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Sat Apr 30 2011
* @brief Base class representing a physical controller.
*
* This is a base class representing a physical controller.
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

class Controller : public QObject {
Q_OBJECT
    friend class ControllerManager; // accesses lots of our stuff, but in the same thread
    friend class ControllerProcessor;   // so our timerEvent() can remain protected
    
    public:
        Controller();
        virtual ~Controller();  // Subclass should call close() at minimum.
        bool isOpen() { return m_bIsOpen; };
        bool isOutputDevice() { return m_bIsOutputDevice; };
        bool isInputDevice() { return m_bIsInputDevice; };
        QString getName() { return m_sDeviceName; };
        bool debugging() { return m_bDebug; };

    protected:
        QDomElement loadPreset(QDomElement root, bool forceLoad=false);
        /** To be called in sub-class' open() functions after opening the
            device but before starting any input polling/processing. */
        virtual void startEngine();
        /** To be called in sub-class' close() functions after stopping any
            input polling/processing but before closing the device. */
        void stopEngine();
        /** By default, this passes the event on to the engine.
            APIs that are not thread-safe or are only non-blocking should poll
            when 'poll' is true and pass the event on to the engine when not. */
        virtual void timerEvent(QTimerEvent *event, bool poll);
        Q_INVOKABLE void send(QList<int> data, unsigned int length);
        /** ByteArray version */
        Q_INVOKABLE void sendBa(QByteArray data, unsigned int length);
        /** Updates the DOM with what script files are currently loaded.
            Sub-classes need to re-implement this (and call it first) if they
            need to add any other items. */
        virtual QDomDocument buildDomElement();
        
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

        QList<QString> m_scriptFileNames;

    // Making these slots protected/private ensures that other parts of Mixxx
    //  can only signal them, preventing thread contention
    protected slots:
        void receivePointer(unsigned char* data, unsigned int length);
        /** Initializes the controller engine */
        virtual void applyPreset();

    private slots:
        virtual int open() = 0;
        virtual int close() = 0;
        
        void loadPreset(bool forceLoad=false);
        void loadPreset(QString path, bool forceLoad=false);
        
    private:
        /** Handles packets of raw bytes and passes them to an ".incomingData"
            script function that is assumed to exist.
            (Sub-classes may want to reimplement this if they have an alternate
            way of handling such data.) */
        virtual void receive(const unsigned char data[], unsigned int length);
        /** This must be reimplmented by sub-classes desiring to send raw bytes
            to a controller. */
        virtual void send(unsigned char data[], unsigned int length);
        
        /** Adds a script file name and function prefix to the list to be loaded. */
        void addScriptFile(QString filename, QString functionprefix);
        /** Saves the current preset to the default device XML file. */
        void savePreset();
        /** Given a path, saves the current preset to an XML file. */
        void savePreset(QString path);

        ControllerEngine *m_pEngine;

        QList<QString> m_scriptFunctionPrefixes;
        
};

#endif
