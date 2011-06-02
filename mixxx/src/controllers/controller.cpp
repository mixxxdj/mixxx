/***************************************************************************
                             controller.cpp
                           Controller Class
                           ----------------
    begin                : Sat Apr 30 2011
    copyright            : (C) 2011 Sean M. Pappalardo
    email                : spappalardo@mixxx.org

***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <qapplication.h>   // For command line arguments
#include "controller.h"

Controller::Controller() : QObject()
{
    m_bIsOutputDevice = false;
    m_bIsInputDevice = false;
    m_bIsOpen = false;

    // Get --midiDebug command line option
    QStringList commandLineArgs = QApplication::arguments();
    m_bDebug = commandLineArgs.contains("--controllerDebug", Qt::CaseInsensitive);
}

Controller::~Controller()
{
}

void Controller::startEngine()
{
    if (debugging()) qDebug() << "  Starting script engine";
    if (m_pScriptEngine != NULL) {
        qWarning() << "Controller: Script engine already exists! Restarting:";
        stopEngine();
    }
    m_pScriptEngine = new ControllerEngine(this);
}

void Controller::stopEngine()
{
    if (debugging()) qDebug() << "  Shutting down script engine";
    if (m_pScriptEngine == NULL) {
        qWarning() << "Controller::stopEngine(): No script engine exists!";
        return;
    }
    m_pScriptEngine->gracefulShutdown();
    delete m_pScriptEngine;
}

void Controller::send(QList<int> data, unsigned int length) {

    unsigned char * msg;
    msg = new unsigned char [length];

    for (unsigned int i=0; i<length; i++) {
        msg[i] = data.at(i);
//         qDebug() << "msg" << i << "=" << msg[i] << ", data=" << data.at(i);
    }

    send(msg,length);
    delete[] msg;
}

void Controller::send(unsigned char data[], unsigned int length) {
    qDebug() << "Error: data sending not yet implemented for this API or platform!";
}

void Controller::receive(const unsigned char data[], unsigned int length) {

    if (debugging()) {
        // Formatted packet display
        QString message = m_sDeviceName+": "+length+" bytes:\n";
        for(uint i=0; i<length; i++) {
            QString spacer=" ";
            if ((i+1) % 4 == 0) spacer="  ";
            if ((i+1) % 16 == 0) spacer="\n";
            message += QString("  %1%2")
                        .arg(data[i], 2, 16, QChar('0')).toUpper()
                        .arg(spacer);
        }
        qDebug()<< message;
    }
    
    QString function = m_sDeviceName + ".incomingData";

    if (!m_pScriptEngine->execute(function, data, length)) {
        qWarning() << "Controller: Invalid script function" << function;
    }
    return;
}
