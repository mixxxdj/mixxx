/**
* @file controllermanager.h
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

class Controller : public QObject
{
Q_OBJECT
    public:
        Controller();
        virtual ~Controller();  // Subclass should call close() at minimum.
        virtual int open() = 0;
        virtual int close() = 0;
        void startEngine();
        void stopEngine();
        bool isOpen() { return m_bIsOpen; };
        bool isOutputDevice() { return m_bIsOutputDevice; };
        bool isInputDevice() { return m_bIsInputDevice; };
        QString getName() { return m_sDeviceName; };
        virtual void send(unsigned char data[], unsigned int length);
        Q_INVOKABLE void send(QList<int> data, unsigned int length);
        void receive(const unsigned char data[], unsigned int length);
        bool debugging() { return m_bDebug; };

        void loadPreset(bool forceLoad=false);
        void loadPreset(QString path, bool forceLoad=false);
        void loadPreset(QDomElement root, bool forceLoad=false);

        void applyPreset();

    protected:
        /** Verbose device name, in format "[index]. [device name]". Suitable for display in GUI. */
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
        
    private:
        /** Adds a script file name and function prefix to the list to be loaded */
        void addScriptFile(QString filename, QString functionprefix);
        
        QList<QString> m_scriptFileNames;
        QList<QString> m_scriptFunctionPrefixes;
        ControllerEngine *m_pControllerEngine;
};

#endif
